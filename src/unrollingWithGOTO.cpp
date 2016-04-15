#include "method.h"
#include "profiler.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include "lib/Target/X86/MCTargetDesc/X86BaseInfo.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCInstBuilder.h"
#include "llvm/MC/MCStreamer.h"

using namespace spMVgen;
using namespace std;
using namespace llvm;

extern unsigned int NUM_OF_THREADS;

class UnrollingWithGOTOCodeEmitter : SpMVCodeEmitter {
public:
  UnrollingWithGOTOCodeEmitter(unsigned long maxRowLength, unsigned long baseValsIndex, unsigned long baseRowsIndex,
                               llvm::MCStreamer *Str, unsigned int partitionIndex);
  
  void emit();

protected:
  virtual void dumpPushPopHeader();
  virtual void dumpPushPopFooter();
  
private:
  unsigned long maxRowLength;
  unsigned long baseValsIndex, baseRowsIndex;
};


UnrollingWithGOTO::UnrollingWithGOTO(Matrix *csrMatrix):
SpMVMethod(csrMatrix), analyzer(csrMatrix) {
}

Matrix* UnrollingWithGOTO::getMatrixForGeneration() {
  START_OPTIONAL_TIME_PROFILE(getCSRbyNZInfo);
  vector<NZtoRowMap> *rowByNZLists = analyzer.getRowByNZLists();
  END_OPTIONAL_TIME_PROFILE(getCSRbyNZInfo);
  
  START_OPTIONAL_TIME_PROFILE(matrixConversion);
  int *rows = new int[2 * csrMatrix->n]; // keeps the row index and num bytes to jump back
  int *cols = new int[csrMatrix->nz];
  double *vals = new double[csrMatrix->nz];
  
  vector<MatrixStripeInfo> &stripeInfos = csrMatrix->getStripeInfos();
  
#pragma omp parallel for
  for (int t = 0; t < NUM_OF_THREADS; ++t) {
    auto &rowByNZList = rowByNZLists->at(t);
    int *rowsPtr = rows + stripeInfos[t].rowIndexBegin * 2;
    int *colsPtr = cols + stripeInfos[t].valIndexBegin;
    double *valsPtr = vals + stripeInfos[t].valIndexBegin;

    int rowCount = 0;
    for (auto &rowByNZ : rowByNZList) {
      unsigned long rowLength = rowByNZ.first;
      for (int rowIndex : *(rowByNZ.second.getRowIndices())) {
        *rowsPtr++ = rowIndex;
        if (rowCount != 0) {
          *(rowsPtr-2) = rowLength * -(4 + 5 + 6 + 4 + 4) - 7;
        }
        rowsPtr++;
        int k = csrMatrix->rows[rowIndex];
        for (int i = 0; i < rowLength; i++, k++) {
          *colsPtr++ = csrMatrix->cols[k];
          *valsPtr++ = csrMatrix->vals[k];
        }
        rowCount++;
      }
    }
    *(rowsPtr-1) = 3 + 4 + 5 + 5 + 3 + 4 + 4 + 2; // last row jumps forward
  }
  END_OPTIONAL_TIME_PROFILE(matrixConversion);

  Matrix *result = new Matrix(rows, cols, vals, csrMatrix->n, csrMatrix->nz);
  result->numRows = 2 * csrMatrix->n;
  result->numCols = csrMatrix->nz;
  result->numVals = csrMatrix->nz;
  return result;
}

void UnrollingWithGOTO::dumpAssemblyText() {
  START_OPTIONAL_TIME_PROFILE(getMatrix);
  this->getMatrix(); // this is done to measure the cost of matrix prep.
  vector<unsigned long> *maxRowLengths = analyzer.getMaxRowLengths();
  vector<MatrixStripeInfo> &stripeInfos = csrMatrix->getStripeInfos();
  END_OPTIONAL_TIME_PROFILE(getMatrix);

  START_OPTIONAL_TIME_PROFILE(emitCode);
  // Set up code emitters
  vector<UnrollingWithGOTOCodeEmitter> codeEmitters;
  for (unsigned i = 0; i < maxRowLengths->size(); i++) {
    unsigned long maxRowLength = maxRowLengths->at(i);
    codeEmitters.push_back(UnrollingWithGOTOCodeEmitter(maxRowLength, stripeInfos[i].valIndexBegin, stripeInfos[i].rowIndexBegin, Str, i));
  }
  
#pragma omp parallel for
  for (int threadIndex = 0; threadIndex < NUM_OF_THREADS; ++threadIndex) {
    codeEmitters[threadIndex].emit();
  }
  END_OPTIONAL_TIME_PROFILE(emitCode);
}

UnrollingWithGOTOCodeEmitter::UnrollingWithGOTOCodeEmitter(unsigned long maxRowLength, unsigned long baseValsIndex, unsigned long baseRowsIndex,
                                                           llvm::MCStreamer *Str, unsigned int partitionIndex):
maxRowLength(maxRowLength), baseValsIndex(baseValsIndex), baseRowsIndex(baseRowsIndex) {
  this->DFOS = createNewDFOS(Str, partitionIndex);
}


void UnrollingWithGOTOCodeEmitter::emit() {
  dumpPushPopHeader();
  
  // xorl %eax, %eax
  emitXOR32rrInst(X86::EAX, X86::EAX);
  
  // leaq "4*numPreviousRows"(%r11), %r11
  emitLEAQInst(X86::R11, X86::R11, 2 * (int)baseRowsIndex * sizeof(int));
  // leaq "numPreviousElements"(%rax), %rax
  emitLEAQInst(X86::RAX, X86::RAX, (int)baseValsIndex);
  // xorps %xmm0, %xmm0
  emitRegInst(X86::XORPSrr, 0, 0);
  
  //.align 16, 0x90
  emitCodeAlignment(16);
  
  for (int i = 0; i < maxRowLength; ++i) {
    // movslq (%r9,%rax,4), %rbx ## cols[k]
    emitMOVSLQInst(X86::RBX, X86::R9, X86::RAX, 4, 0);
    // movsd (%rdi,%rbx,8), %xmm1 ## v[cols[k]]
    emitMOVSDrmInst(0, X86::RDI, X86::RBX, 8, 1);
    // mulsd (%r8,%rax,8), %xmm1 ## ...  *  vals[k]
    emitMULSDrmInst(0, X86::R8, X86::RAX, 8, 1);
    // addsd %xmm1, %xmm0
    emitRegInst(X86::ADDSDrr, 1, 0);
    // addq $"1", %rax
    emitADDQInst(1 , X86::RAX);
  }
  
  emitLEAQ_RIP(X86::RDX, 0);
  
  // Move the next row index to rcx
  // movslq (%r11), %rcx
  emitMOVSLQInst(X86::RCX, X86::R11, 0);
  // Move the num bytes to jump into r10
  // movslq "sizeof(int)"(%r11), %r10
  emitMOVSLQInst(X86::R10, X86::R11, sizeof(int));
  // Add to w[r]
  //addsd (%rsi,%rcx,8), %xmm0
  emitADDSDrmInst(0, X86::RSI, X86::RCX, 8, 0);
  //movsd %xmm0, (%rsi,%rcx,8)
  emitMOVSDmrInst(0, 0, X86::RSI, X86::RCX, 8);
  // xorps %xmm0, %xmm0
  emitRegInst(X86::XORPSrr, 0, 0);
  // leaq "2*sizeof(int)"(%r11), %r11
  emitLEAQInst(X86::R11, X86::R11, 2*sizeof(int));
  // leaq (%rdx,%r10), %rdx
  emitLEAQInst(X86::RDX, X86::RDX, X86::R10, 0);
  // jmp *%rdx
  emitDynamicJMPInst(X86::RDX);
  
  dumpPushPopFooter();
}

void UnrollingWithGOTOCodeEmitter::dumpPushPopHeader() {
  // rows is in %rdx, cols is in %rcx, vals is in %r8
  emitPushPopInst(X86::PUSH64r,X86::R9);
  emitPushPopInst(X86::PUSH64r,X86::R10);
  emitPushPopInst(X86::PUSH64r,X86::R11);
  
  emitLEAQInst(X86::RCX, X86::R9, 0); // using %r9 for cols
  emitLEAQInst(X86::RDX, X86::R11, 0); // using %r11 for rows
  
  emitPushPopInst(X86::PUSH64r,X86::RAX);
  emitPushPopInst(X86::PUSH64r,X86::RCX);
  emitPushPopInst(X86::PUSH64r,X86::RDX);
  emitPushPopInst(X86::PUSH64r,X86::RBX);
}

void UnrollingWithGOTOCodeEmitter::dumpPushPopFooter() {
  emitPushPopInst(X86::POP64r,X86::RBX);
  emitPushPopInst(X86::POP64r,X86::RDX);
  emitPushPopInst(X86::POP64r,X86::RCX);
  emitPushPopInst(X86::POP64r,X86::RAX);
  emitPushPopInst(X86::POP64r,X86::R11);
  emitPushPopInst(X86::POP64r,X86::R10);
  emitPushPopInst(X86::POP64r,X86::R9);
  emitRETInst(); // We shouldn't have had to do this here. Strange...
}


