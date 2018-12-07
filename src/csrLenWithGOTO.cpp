#include "specializers.h"
#include "spmvRegistry.h"
#include <iostream>

using namespace thundercat;
using namespace std;
using namespace asmjit;
using namespace x86;

/// This method is a variation of CSRWithGOTO where
/// in the rows array we pre-compute and store the number
/// of bytes to jump backwards.
/// This way, we do not have to compute the distance dynamically.
/// Matrix values are not reordered. Only the rows array changes.

REGISTER_METHOD(CSRLenWithGOTO, "csrlenwithgoto")

///
/// Analysis
///
void CSRLenWithGOTO::analyzeMatrix() {
  maxRowLengths.resize(stripeInfos->size());
  
#pragma omp parallel for
  for (int threadIndex = 0; threadIndex < stripeInfos->size(); ++threadIndex) {
    auto &stripeInfo = stripeInfos->at(threadIndex);
    int maxRowLength = 0;
    for (unsigned long rowIndex = stripeInfo.rowIndexBegin; rowIndex < stripeInfo.rowIndexEnd; ++rowIndex) {
      int rowStart = csrMatrix->rowPtr[rowIndex];
      int rowEnd = csrMatrix->rowPtr[rowIndex+1];
      int rowLength = rowEnd - rowStart;
      if (rowLength > maxRowLength) {
        maxRowLength = rowLength;
      }
    }
    maxRowLengths[threadIndex] = maxRowLength;
  }
}

///
/// CSRWithGOTO
///

void CSRLenWithGOTO::convertMatrix() {
  int *rows = new int[csrMatrix->N + stripeInfos->size()]; // 1 terminating slot for each stripe
  int *cols = csrMatrix->colIndices;
  double *vals = csrMatrix->values;

  // newly created matrix will take the ownership of colIndices and values, set these two to NULL to avoid double free
  // problem.
  csrMatrix->colIndices = NULL;
  csrMatrix->values = NULL;
  
#pragma omp parallel for
  for (int t = 0; t < stripeInfos->size(); ++t) {
    int i;
    for (i = stripeInfos->at(t).rowIndexBegin; i < stripeInfos->at(t).rowIndexEnd; i++) {
      int length = csrMatrix->rowPtr[i + 1] - csrMatrix->rowPtr[i];
      rows[i + t] = -(length * 22);
    }
    rows[i + t] = 5 + 5 + 3 + 3 + 4 + 7 + 3 + 3;
  }

  matrix = std::make_unique<CSRMatrix<VALUE_TYPE>>(rows, cols, vals, csrMatrix->N, csrMatrix->M, csrMatrix->NZ);
}

///
/// CSRLenWithGOTOCodeEmitter:
/// Helper class to avoid having to pass several parameters
///
class CSRLenWithGOTOCodeEmitter {
public:
  CSRLenWithGOTOCodeEmitter(X86Assembler *assembler,
                            unsigned long maxRowLength,
                            unsigned long baseValsIndex,
                            unsigned long baseRowsIndex) {
    this->assembler = assembler;
    this->maxRowLength = maxRowLength;
    this->baseValsIndex = baseValsIndex;
    this->baseRowsIndex = baseRowsIndex;
  }
  
  void emit(int stripeIndex);
  
private:
  X86Assembler *assembler;
  unsigned long maxRowLength;
  unsigned long baseValsIndex;
  unsigned long baseRowsIndex;

  void emitHeader(int stripeIndex);
  
  void emitFooter();
  
  void emitMainLoop();
};

void CSRLenWithGOTO::emitMultByMFunction(unsigned int index) {
  X86Assembler assembler(codeHolders[index]);
  unsigned int maxRowLength = maxRowLengths.at(index);
  int N = stripeInfos->at(index).rowIndexEnd - stripeInfos->at(index).rowIndexBegin;
  CSRLenWithGOTOCodeEmitter emitter(&assembler,
                                 maxRowLength,
                                 stripeInfos->at(index).valIndexBegin,
                                 stripeInfos->at(index).rowIndexBegin);
  emitter.emit(index);
}

void CSRLenWithGOTOCodeEmitter::emit(int stripeIndex) {
  emitHeader(stripeIndex);
  emitMainLoop();
  emitFooter();
}

void CSRLenWithGOTOCodeEmitter::emitHeader(int stripeIndex) {
  // rows is in %rdx, cols is in %rcx, vals is in %r8
  assembler->push(r8);
  assembler->push(r9);
  assembler->push(r10);
  assembler->push(r11);
  assembler->push(rax);
  assembler->push(rcx);
  assembler->push(rdx);
  assembler->push(rbx);
  assembler->push(rsi);

  assembler->lea(r8, ptr(r8, (int)baseValsIndex * sizeof(double)));
  assembler->lea(r9, ptr(rcx, (int)baseValsIndex * sizeof(int))); // using %r9 for cols
  assembler->lea(r11, ptr(rdx, ((int)baseRowsIndex + stripeIndex) * sizeof(int))); // using %r11 for rows
  assembler->lea(rsi, ptr(rsi, (int)baseRowsIndex * sizeof(double)));
}

void CSRLenWithGOTOCodeEmitter::emitFooter() {
  assembler->pop(rsi);
  assembler->pop(rbx);
  assembler->pop(rdx);
  assembler->pop(rcx);
  assembler->pop(rax);
  assembler->pop(r11);
  assembler->pop(r10);
  assembler->pop(r9);
  assembler->pop(r8);
  assembler->ret();
}

void CSRLenWithGOTOCodeEmitter::emitMainLoop() {
  // xorl %eax, %eax
  assembler->xor_(eax, eax);
  // xorl %edx, %edx
  assembler->xor_(edx, edx); // row counter
  
  Label end = assembler->newLabel();
  assembler->jmp(end);
  
  Label loopStart = assembler->newLabel();
  assembler->bind(loopStart);
  for (int i = 0; i < maxRowLength; ++i) {
    // movslq (%r9,%rax,4), %rbx ## cols[k]
    assembler->movsxd(rbx, ptr(r9, rax, 2));
    // movsd (%r8,%rax,8), %xmm1 ## ...  *  vals[k]
    assembler->movsd(xmm1, ptr(r8, rax, 3));
    // addq $"1", %rax
    assembler->inc(rax);
    // mulsd (%rdi,%rbx,8), %xmm1 ## v[cols[k]]
    assembler->mulsd(xmm1, ptr(rdi, rbx, 3));
    // addsd %xmm1, %xmm0
    assembler->addsd(xmm0, xmm1);
  }
  Label loopEnd = assembler->newLabel();
  assembler->bind(loopEnd);
  
  // Add to w[r]
  //addsd (%rsi,%rdx,8), %xmm0
  assembler->addsd(xmm0, ptr(rsi, rdx, 3));
  //movsd %xmm0, (%rsi,%rdx,8)
  assembler->movsd(ptr(rsi, rdx, 3), xmm0);
  // addq $"1", %rdx
  assembler->inc(rdx);
  
  assembler->bind(end); // This is the destination of the very first jump.
  // xorps %xmm0, %xmm0
  assembler->xorps(xmm0, xmm0);

  // Load the jump distance from rows
  assembler->movsxd(rbx, ptr(r11, rdx, 2));
  
  Label marker = assembler->newLabel();
  assembler->bind(marker);
  int loopEndOffset = assembler->getCode()->getLabelOffset(loopEnd);
  int loopPrologue = assembler->getCode()->getLabelOffset(marker) - loopEndOffset;
  
  assembler->lea(r10, ptr(rip, -(7 + loopPrologue))); // length of lea is 7
  assembler->add(r10, rbx); // r10 = r10 + rbx.
  // jmp *%r10
  assembler->jmp(r10);
}

