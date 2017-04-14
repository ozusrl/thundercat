#include "method.h"
#include <iostream>

using namespace thundercat;
using namespace std;
using namespace asmjit;
using namespace x86;

///
/// Analysis
///
void CSRWithGOTO::analyzeMatrix() {
  maxRowLengths.resize(stripeInfos->size());
  
#pragma omp parallel for
  for (int threadIndex = 0; threadIndex < stripeInfos->size(); ++threadIndex) {
    auto &stripeInfo = stripeInfos->at(threadIndex);
    int maxRowLength = 0;
    for (unsigned long rowIndex = stripeInfo.rowIndexBegin; rowIndex < stripeInfo.rowIndexEnd; ++rowIndex) {
      int rowStart = csrMatrix->rows[rowIndex];
      int rowEnd = csrMatrix->rows[rowIndex+1];
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

void CSRWithGOTO::convertMatrix() {
  // We use the original CSR format
  matrix = csrMatrix;
}

///
/// CSRWithGOTOCodeEmitter:
/// Helper class to avoid having to pass several parameters
///
class CSRWithGOTOCodeEmitter {
public:
  CSRWithGOTOCodeEmitter(X86Assembler *assembler,
                         unsigned long maxRowLength,
                         unsigned long baseValsIndex,
                         unsigned long baseRowsIndex,
                         int N) {
    this->assembler = assembler;
    this->maxRowLength = maxRowLength;
    this->baseValsIndex = baseValsIndex;
    this->baseRowsIndex = baseRowsIndex;
    this->N = N;
  }
  
  void emit();
  
private:
  X86Assembler *assembler;
  unsigned long maxRowLength;
  unsigned long baseValsIndex;
  unsigned long baseRowsIndex;
  int N;

  void emitHeader();
  
  void emitFooter();
  
  void emitMainLoop();
};

void CSRWithGOTO::emitMultByMFunction(unsigned int index) {
  X86Assembler assembler(codeHolders[index]);
  unsigned int maxRowLength = maxRowLengths.at(index);
  int N = stripeInfos->at(index).rowIndexEnd - stripeInfos->at(index).rowIndexBegin;
  CSRWithGOTOCodeEmitter emitter(&assembler,
                                 maxRowLength,
                                 stripeInfos->at(index).valIndexBegin,
                                 stripeInfos->at(index).rowIndexBegin,
                                 N);
  emitter.emit();
}

void CSRWithGOTOCodeEmitter::emit() {
  emitHeader();
  emitMainLoop();
  emitFooter();
}

void CSRWithGOTOCodeEmitter::emitHeader() {
  // rows is in %rdx, cols is in %rcx, vals is in %r8
  assembler->push(r9);
  assembler->push(r10);
  assembler->push(r11);
  assembler->lea(r9, ptr(rcx)); // using %r9 for cols
  assembler->lea(r11, ptr(rdx)); // using %r11 for rows
  assembler->push(rax);
  assembler->push(rcx);
  assembler->push(rdx);
  assembler->push(rbx);
}

void CSRWithGOTOCodeEmitter::emitFooter() {
  assembler->pop(rbx);
  assembler->pop(rdx);
  assembler->pop(rcx);
  assembler->pop(rax);
  assembler->pop(r11);
  assembler->pop(r10);
  assembler->pop(r9);
  assembler->ret();
}

void CSRWithGOTOCodeEmitter::emitMainLoop() {
  // leaq "4*numPreviousRows"(%r11), %r11
  assembler->lea(r11, ptr(r11, (int)baseRowsIndex * sizeof(int)));
  // leaq "8*numPreviousRows"(%rsi), %rsi
  assembler->lea(r8, ptr(r8, (int)baseRowsIndex * sizeof(double)));
  // leaq "numPreviousElements"(%r9), %r9
  assembler->lea(r9, ptr(r9, (int)baseValsIndex * sizeof(int)));
  // leaq "numPreviousElements"(%r8), %r8
  assembler->lea(r8, ptr(r8, (int)baseValsIndex * sizeof(double)));
  
  // xorl %eax, %eax
  assembler->xor_(eax, eax);
  // Keep rows[i] in rcx, rows[i+1] in rdx. Index through %rbx.
  // xorl %edx, %edx
  assembler->xor_(edx, edx); // row counter
  // movslq (%r11), %rcx
  assembler->movsxd(rcx, ptr(r11, 0));
  
  Label end = assembler->newLabel();
  assembler->jmp(end);
  Label loopStart = assembler->newLabel();
  assembler->bind(loopStart);
  
  for (int i = 0; i < maxRowLength; ++i) {
    // movslq (%r9,%rax,4), %rbx ## cols[k]
    assembler->movsxd(rbx, ptr(r9, rax, 2));
    // movsd (%rdi,%rbx,8), %xmm1 ## v[cols[k]]
    assembler->movsd(xmm1, ptr(rdi, rbx, 3));
    // mulsd (%r8,%rax,8), %xmm1 ## ...  *  vals[k]
    assembler->mulsd(xmm1, ptr(r8, rax, 3));
    // addsd %xmm1, %xmm0
    assembler->addsd(xmm0, xmm1);
    // addq $"1", %rax
    assembler->inc(rax);
  }
  Label loopEnd = assembler->newLabel();
  assembler->bind(loopEnd);
  // Add to w[r]
  //addsd -8(%rsi,%rdx,8), %xmm0
  assembler->addsd(xmm0, ptr(rsi, rdx, 3, -8));
  //movsd %xmm0, -8(%rsi,%rdx,8)
  assembler->movsd(ptr(rsi, rdx, 3, -8), xmm0);
  
  assembler->bind(end); // This is the destination of the very first jump.
  // addq $"1", %rdx
  assembler->inc(rdx);
  // compare %rbx and N to check for the end of the loop
  assembler->cmp(rdx, N);
  Label veryEnd = assembler->newLabel();
  assembler->jg(veryEnd);
  
  // Move the next row index to rbx
  assembler->movsxd(rbx, ptr(r11, rdx, 2));
  // Find row length, store in rcx
  assembler->sub(rcx, rbx); // rcx = rcx - rbx. This is a negative number.
  // mulq $PER_ELEMENT_CODE_LENGTH, %rcx
  int loopStartOffset = assembler->getCode()->getLabelOffset(loopStart);
  int loopEndOffset = assembler->getCode()->getLabelOffset(loopEnd);
  int loopLength = loopEndOffset - loopStartOffset;
  if (loopLength % maxRowLength != 0) {
    std::cerr << "Unexpected loop length in CSRWithGOTO.\n";
    exit(1);
  }
  int perElementCodeLength = loopLength / maxRowLength;
  assembler->imul(rcx, perElementCodeLength);
  
  Label backJump = assembler->newLabel();
  assembler->bind(backJump);
  int backJumpOffset = assembler->getCode()->getLabelOffset(backJump) + 7;
  // The next leaq instruction is 7 bytes long.
  assembler->lea(r10, ptr(rip, loopEndOffset - backJumpOffset));
  
  assembler->add(r10, rcx); // r10 = r10 - rcx.
  // Move the next row value to %rcx for the next iteration
  assembler->lea(rcx, ptr(rbx));
  // xorps %xmm0, %xmm0
  assembler->xorps(xmm0, xmm0);
  // jmp *%r10
  assembler->jmp(r10);
  
  assembler->bind(veryEnd);
}

