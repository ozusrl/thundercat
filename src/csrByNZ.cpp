#include "specializers.h"
#include "profiler.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include "spmvRegistry.h"

using namespace thundercat;
using namespace std;
using namespace asmjit;
using namespace x86;

REGISTER_METHOD(CSRbyNZ, "csrbynz")

///
/// Analysis
///
void CSRbyNZ::analyzeMatrix() {
  LCSRAnalyzer analyzer;
  analyzer.analyzeMatrix(*csrMatrix, *stripeInfos, rowByNZLists);
}

///
/// CSRbyNZ
///

// Return a matrix to be used by CSRbyNZ
// rows: row indices, sorted by row lengths
// cols: indices of elements,
//       sorted according to the order used in rows array
// vals: values as usual,
//       sorted according to the order used in rows array
void CSRbyNZ::convertMatrix() {
  int *rows = new int[csrMatrix->N];
  int *cols = new int[csrMatrix->NZ];
  double *vals = new double[csrMatrix->NZ];

#pragma omp parallel for
  for (int t = 0; t < rowByNZLists.size(); ++t) {
    auto &rowByNZList = rowByNZLists.at(t);
    int *rowsPtr = rows + stripeInfos->at(t).rowIndexBegin;
    int *colsPtr = cols + stripeInfos->at(t).valIndexBegin;
    double *valsPtr = vals + stripeInfos->at(t).valIndexBegin;
    
    for (auto &rowByNZ : rowByNZList) {
      unsigned long rowLength = rowByNZ.first;
      for (int rowIndex : *(rowByNZ.second.getRowIndices())) {
        *rowsPtr++ = rowIndex;
        int k = csrMatrix->rowPtr[rowIndex];
        for (int i = 0; i < rowLength; i++, k++) {
          *colsPtr++ = csrMatrix->colIndices[k];
          *valsPtr++ = csrMatrix->values[k];
        }
      }
    }
  }

  matrix = std::make_unique<CSRMatrix<VALUE_TYPE>>(rows, cols, vals, csrMatrix->N, csrMatrix->M, csrMatrix->NZ);
}

///
/// CSRbyNZCodeEmitter:
/// Helper class to avoid having to pass several parameters
///
class CSRbyNZCodeEmitter {
public:
  CSRbyNZCodeEmitter(X86Assembler *assembler,
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
  
  void emitSingleLoop(unsigned long numRows, unsigned long rowLength);
};

void CSRbyNZ::emitMultByMFunction(unsigned int index) {
  X86Assembler assembler(codeHolders[index]);
  NZtoRowMap &rowByNZs = rowByNZLists.at(index);
  CSRbyNZCodeEmitter emitter(&assembler,
                             &rowByNZs,
                             stripeInfos->at(index).valIndexBegin,
                             stripeInfos->at(index).rowIndexBegin);
  emitter.emit();
}


void CSRbyNZCodeEmitter::emit() {
  emitHeader();
  
  for (auto &rowByNZ : *rowByNZs) {
    unsigned long rowLength = rowByNZ.first;
    emitSingleLoop(rowByNZ.second.getRowIndices()->size(), rowLength);
  }
  
  emitFooter();
}
  
void CSRbyNZCodeEmitter::emitHeader() {
  // rows is in %rdx, cols is in %rcx, vals is in %r8
  assembler->push(r8);
  assembler->push(r9);
  assembler->push(r10);
  assembler->push(r11);
  assembler->push(rax);
  assembler->push(rbx);
  assembler->push(rcx);
  assembler->push(rdx);

  assembler->lea(rdx, ptr(rdx, (int)(sizeof(int) * baseRowsIndex)));
  assembler->lea(rcx, ptr(rcx, (int)(sizeof(int) * baseValsIndex)));
  assembler->lea(r8, ptr(r8, (int)(sizeof(double) * baseValsIndex)));
}

void CSRbyNZCodeEmitter::emitFooter() {
  assembler->pop(rdx);
  assembler->pop(rcx);
  assembler->pop(rbx);
  assembler->pop(rax);
  assembler->pop(r11);
  assembler->pop(r10);
  assembler->pop(r9);
  assembler->pop(r8);
  assembler->ret();
}

void CSRbyNZCodeEmitter::emitSingleLoop(unsigned long numRows,
                                        unsigned long rowLength) {
  assembler->xor_(r9d, r9d);
  assembler->xor_(ebx, ebx);

  assembler->align(kAlignCode, 16);
  Label loopBegin = assembler->newLabel();
  assembler->bind(loopBegin);
  
  //xorps %xmm0, %xmm0
  assembler->xorps(xmm0, xmm0);
  
  // done for a single row
  for(int i = 0 ; i < rowLength ; i++){
    //movslq "i*4"(%rcx,%r9,4), %rax
    assembler->movsxd(rax, ptr(rcx, r9, 2, i * sizeof(int)));
    //movsd "i*8"(%r8,%r9,8), %xmm1
    assembler->movsd(xmm1, ptr(r8, r9, 3, i * sizeof(double)));
    //mulsd (%rdi,%rax,8), %xmm1
    assembler->mulsd(xmm1, ptr(rdi, rax, 3));
    //addsd %xmm1, %xmm0
    assembler->addsd(xmm0, xmm1);
  }
  
  // movslq (%rdx,%rbx,4), %rax
  assembler->movsxd(rax, ptr(rdx, rbx, 2));
  //addq $rowLength, %r9
  assembler->add(r9, (unsigned int)rowLength);
  //addq $1, %rbx
  assembler->inc(rbx);
  //addsd (%rsi,%rax,8), %xmm0
  assembler->addsd(xmm0, ptr(rsi, rax, 3));
  //cmpl numRows, %ebx
  assembler->cmp(ebx, (unsigned int)numRows);
  //movsd %xmm0, (%rsi,%rax,8)
  assembler->movsd(ptr(rsi, rax, 3), xmm0);
  //jne .LBB0_1
  assembler->jne(loopBegin);
  
  //addq $numRows*4, %rdx
  assembler->add(rdx, (unsigned int)(numRows * sizeof(int)));
  //addq $numRows*rowLength*4, %rcx
  assembler->add(rcx, (unsigned int)(numRows * rowLength * sizeof(int)));
  //addq $numRows*rowLength*8, %r8
  assembler->add(r8, (unsigned int)(numRows * rowLength * sizeof(double)));
}
