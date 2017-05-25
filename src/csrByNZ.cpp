#include "method.h"
#include "profiler.h"
#include <iostream>
#include <fstream>
#include <sstream>

using namespace thundercat;
using namespace std;
using namespace asmjit;
using namespace x86;

///
/// Analysis
///
void CSRbyNZ::analyzeMatrix() {
  LCSRAnalyzer analyzer;
  analyzer.analyzeMatrix(csrMatrix, stripeInfos, rowByNZLists);
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
  int *rows = new int[csrMatrix->n];
  int *cols = new int[csrMatrix->nz];
  double *vals = new double[csrMatrix->nz];

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
        int k = csrMatrix->rows[rowIndex];
        for (int i = 0; i < rowLength; i++, k++) {
          *colsPtr++ = csrMatrix->cols[k];
          *valsPtr++ = csrMatrix->vals[k];
        }
      }
    }
  }

  matrix = new Matrix(rows, cols, vals, csrMatrix->n, csrMatrix->nz);
}

///
/// CSRbyNZCodeEmitter:
/// Helper class to avoid having to pass several parameters
///
class CSRbyNZCodeEmitter {
public:
  CSRbyNZCodeEmitter(X86Compiler *compiler,
                     NZtoRowMap *rowByNZs,
                     unsigned long baseValsIndex,
                     unsigned long baseRowsIndex) {
    this->compiler = compiler;
    this->rowByNZs = rowByNZs;
    this->baseValsIndex = baseValsIndex;
    this->baseRowsIndex = baseRowsIndex;

    vReg = compiler->newIntPtr("v");
    wReg = compiler->newIntPtr("w");
    rowsReg = compiler->newIntPtr("rows");
    colsReg = compiler->newIntPtr("cols");
    valsReg = compiler->newIntPtr("vals");
    compiler->setArg(0, vReg);
    compiler->setArg(1, wReg);
    compiler->setArg(2, rowsReg);
    compiler->setArg(3, colsReg);
    compiler->setArg(4, valsReg);
  }

  void emit();
  
private:
  X86Compiler *compiler;
  X86Gp vReg;
  X86Gp wReg;
  X86Gp rowsReg;
  X86Gp colsReg;
  X86Gp valsReg;
  NZtoRowMap *rowByNZs;
  unsigned long baseValsIndex;
  unsigned long baseRowsIndex;
  
  void emitHeader();
  
  void emitFooter();
  
  void emitSingleLoop(unsigned long numRows, unsigned long rowLength);
};

void CSRbyNZ::emitMultByMFunction(unsigned int index) {
  X86Compiler compiler(codeHolders[index]);
  // (v, w, rows, cols, vals)
  compiler.addFunc(FuncSignature5<void, double*, double*, int*, int*, double*>());

  NZtoRowMap &rowByNZs = rowByNZLists.at(index);
  CSRbyNZCodeEmitter emitter(&compiler,
                             &rowByNZs,
                             stripeInfos->at(index).valIndexBegin,
                             stripeInfos->at(index).rowIndexBegin);
  emitter.emit();
  compiler.endFunc();
  compiler.finalize();
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
  compiler->lea(rowsReg, ptr(rowsReg, (int)(sizeof(int) * baseRowsIndex)));
  compiler->lea(colsReg, ptr(colsReg, (int)(sizeof(int) * baseValsIndex)));
  compiler->lea(valsReg, ptr(valsReg, (int)(sizeof(double) * baseValsIndex)));
}

void CSRbyNZCodeEmitter::emitFooter() {
  compiler->ret();
}

void CSRbyNZCodeEmitter::emitSingleLoop(unsigned long numRows,
                                        unsigned long rowLength) {
  X86Gp aReg = compiler->newI32("a");
  X86Gp bReg = compiler->newI32("b");
  X86Gp colReg = compiler->newI32("col");
  X86Gp rowReg = compiler->newI32("row");
  X86Gp valReg = compiler->newUInt64("val");
  X86Xmm sumReg = compiler->newXmm("sum");

  compiler->xor_(bReg, bReg);
  compiler->xor_(aReg, aReg);
  
  compiler->align(kAlignCode, 16);
  Label loopBegin = compiler->newLabel();
  compiler->bind(loopBegin);
  
  //xorps %xmm0, %xmm0
  compiler->xorps(sumReg, sumReg);
  
  // done for a single row
  for(int i = 0 ; i < rowLength ; i++){
    X86Xmm multReg = compiler->newXmm("mult");
    //movslq "i*4"(%rcx,%r9,4), %rax
    compiler->movsxd(colReg, ptr(colsReg, bReg, 2, i * sizeof(int)));
    //movsd "i*8"(%r8,%r9,8), %xmm1
    compiler->movsd(multReg, ptr(valsReg, bReg, 3, i * sizeof(double)));
    //mulsd (%rdi,%rax,8), %xmm1
    compiler->mulsd(multReg, ptr(vReg, colReg, 3));
    //addsd %xmm1, %xmm0
    compiler->addsd(sumReg, multReg);
  }
  
  // movslq (%rdx,%rbx,4), %rax
  compiler->movsxd(rowReg, ptr(rowsReg, aReg, 2));
  //addq $rowLength, %r9
  compiler->add(bReg, (unsigned int)rowLength);
  //addq $1, %rbx
  compiler->inc(aReg);
  //addsd (%rsi,%rax,8), %xmm0
  compiler->addsd(sumReg, ptr(wReg, rowReg, 3));
  //cmpl numRows, %ebx
  compiler->cmp(aReg, (unsigned int)numRows);
  //movsd %xmm0, (%rsi,%rax,8)
  compiler->movsd(ptr(wReg, rowReg, 3), sumReg);
  //jne .LBB0_1
  compiler->jne(loopBegin);
  
  //addq $numRows*4, %rdx
  compiler->add(rowsReg, (unsigned int)(numRows * sizeof(int)));
  //addq $numRows*rowLength*4, %rcx
  compiler->add(colsReg, (unsigned int)(numRows * rowLength * sizeof(int)));
  //addq $numRows*rowLength*8, %r8
  compiler->add(valsReg, (unsigned int)(numRows * rowLength * sizeof(double)));
}
