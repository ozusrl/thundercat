#include "specializers.h"
#include "spmvRegistry.h"
#include <iostream>

using namespace thundercat;
using namespace std;
using namespace asmjit;
using namespace x86;


REGISTER_METHOD(UnrollingWithGOTO, "unrollingwithgoto")

///
/// Analysis is inherited from CSRbyNZ.
///

///
/// UnrollingWithGOTO
///
void UnrollingWithGOTO::convertMatrix() {
  int *rows = new int[2 * csrMatrix->N]; // keeps the row index and num bytes to jump back
  int *cols = new int[csrMatrix->NZ];
  double *vals = new double[csrMatrix->NZ];
  
#pragma omp parallel for
  for (int t = 0; t < stripeInfos->size(); ++t) {
    auto &rowByNZList = rowByNZLists.at(t);
    int *rowsPtr = rows + stripeInfos->at(t).rowIndexBegin * 2;
    int *colsPtr = cols + stripeInfos->at(t).valIndexBegin;
    double *valsPtr = vals + stripeInfos->at(t).valIndexBegin;

    int rowCount = 0;
    for (auto &rowByNZ : rowByNZList) {
      unsigned long rowLength = rowByNZ.first;
      for (int rowIndex : *(rowByNZ.second.getRowIndices())) {
        *rowsPtr++ = rowIndex;
        if (rowCount != 0) {
          *(rowsPtr-2) = rowLength * -(4 + 6 + 3 + 5 + 4) - 7;
        }
        rowsPtr++;
        int k = csrMatrix->rowPtr[rowIndex];
        for (int i = 0; i < rowLength; i++, k++) {
          *colsPtr++ = csrMatrix->colIndices[k];
          *valsPtr++ = csrMatrix->values[k];
        }
        rowCount++;
      }
    }
    *(rowsPtr-1) = 3 + 4 + 5 + 5 + 3 + 4 + 4 + 2; // last row jumps forward
  }

  matrix = std::make_unique<CSRMatrix<VALUE_TYPE>>(rows, cols, vals, csrMatrix->N, csrMatrix->M, csrMatrix->NZ);
}

///
/// UnrollingWithGOTOCodeEmitter:
/// Helper class to avoid having to pass several parameters
///
class UnrollingWithGOTOCodeEmitter {
public:
  UnrollingWithGOTOCodeEmitter(X86Assembler *assembler,
                               NZtoRowMap *rowByNZs,
                               unsigned long baseValsIndex,
                               unsigned long baseRowsIndex) {
    this->assembler = assembler;
    this->rowByNZs = rowByNZs;
    this->baseValsIndex = baseValsIndex;
    this->baseRowsIndex = baseRowsIndex;
  }
  
  void emit();
  
private:
  X86Assembler *assembler;
  NZtoRowMap *rowByNZs;
  unsigned long baseValsIndex;
  unsigned long baseRowsIndex;
  
  void emitHeader();
  
  void emitFooter();
  
  void emitMainLoop(int maxRowLength);
};

void UnrollingWithGOTO::emitMultByMFunction(unsigned int index) {
  X86Assembler assembler(codeHolders[index]);
  NZtoRowMap &rowByNZs = rowByNZLists.at(index);
  UnrollingWithGOTOCodeEmitter emitter(&assembler,
                                       &rowByNZs,
                                       stripeInfos->at(index).valIndexBegin,
                                       stripeInfos->at(index).rowIndexBegin);
  emitter.emit();
}


void UnrollingWithGOTOCodeEmitter::emit() {
  emitHeader();
  
  // This relies on the fact that rows are sorted in ascending order by length
  int maxRowLength = rowByNZs->begin()->first;
  emitMainLoop(maxRowLength);
  
  emitFooter();
}

void UnrollingWithGOTOCodeEmitter::emitHeader() {
  // rows is in %rdx, cols is in %rcx, vals is in %r8
  assembler->push(r9);
  assembler->push(r10);
  assembler->push(r11);

  assembler->lea(r9, ptr(rcx));  // using %r9 for cols
  assembler->lea(r11, ptr(rdx, 2 * baseRowsIndex * sizeof(int)));  // using %r11 for rows
  
  assembler->push(rax);
  assembler->push(rcx);
  assembler->push(rdx);
  assembler->push(rbx);

  // xorl %eax, %eax
  assembler->xor_(eax, eax);
  assembler->lea(rax, ptr(rax, baseValsIndex));
}

void UnrollingWithGOTOCodeEmitter::emitFooter() {
  assembler->pop(rbx);
  assembler->pop(rdx);
  assembler->pop(rcx);
  assembler->pop(rax);
  assembler->pop(r11);
  assembler->pop(r10);
  assembler->pop(r9);
  assembler->ret();
}

void UnrollingWithGOTOCodeEmitter::emitMainLoop(int maxRowLength) {
  // xorps %xmm0, %xmm0
  assembler->xorps(xmm0, xmm0);
  
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

  assembler->lea(rdx, ptr(rip));
  
  // Move the next row index to rcx
  // movslq (%r11), %rcx
  assembler->movsxd(rcx, ptr(r11));
  // Move the num bytes to jump into r10
  // movslq "sizeof(int)"(%r11), %r10
  assembler->movsxd(r10, ptr(r11, sizeof(int)));
  // Add to w[r]
  //addsd (%rsi,%rcx,8), %xmm0
  assembler->addsd(xmm0, ptr(rsi, rcx, 3));
  //movsd %xmm0, (%rsi,%rcx,8)
  assembler->movsd(ptr(rsi, rcx, 3), xmm0);
  // xorps %xmm0, %xmm0
  assembler->xorps(xmm0, xmm0);
  // leaq "2*sizeof(int)"(%r11), %r11
  assembler->lea(r11, ptr(r11, 2 * sizeof(int)));
  // leaq (%rdx,%r10), %rdx
  assembler->lea(rdx, ptr(rdx, r10));
  // jmp *%rdx
  assembler->jmp(rdx);
}

