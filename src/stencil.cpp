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
#include "llvm/MC/MCObjectFileInfo.h"

using namespace spMVgen;
using namespace std;
using namespace llvm;

extern unsigned int NUM_OF_THREADS;

class StencilCodeEmitter : public SpMVCodeEmitter {
private:
  StencilToRowMap *stencils;
  unsigned long baseValsIndex;
  unsigned long baseRowsIndex;
  
  void dumpStencilAssemblyText(const StencilPattern &pattern,
                               const RowIndices &rowIndices);
protected:
  virtual void dumpPushPopHeader();
  virtual void dumpPushPopFooter();
  
public:
  StencilCodeEmitter(StencilToRowMap *stencils,
                     unsigned long baseValsIndex, unsigned long baseRowsIndex,
                     llvm::MCStreamer *Str, unsigned int partitionIndex);
  
  void emit();
};

Stencil::Stencil(Matrix *csrMatrix):
SpMVMethod(csrMatrix), analyzer(csrMatrix) {
  
}

Matrix* Stencil::getMatrixForGeneration() {
  START_OPTIONAL_TIME_PROFILE(getStencilInfo);
  vector<StencilToRowMap> *stencilLists = analyzer.getStencilLists();
  END_OPTIONAL_TIME_PROFILE(getStencilInfo);

  START_OPTIONAL_TIME_PROFILE(matrixConversion);
  double *vals = new double[csrMatrix->nz];
  int *rows = new int[csrMatrix->n];
  
  vector<MatrixStripeInfo> &stripeInfos = csrMatrix->getStripeInfos();
  
#pragma omp parallel for
  for (unsigned int t = 0; t < NUM_OF_THREADS; ++t) {
    auto &stencils = stencilLists->at(t);
    double *valPtr = vals + stripeInfos[t].valIndexBegin;
    int *rowPtr = rows + stripeInfos[t].rowIndexBegin;
    
    for (auto &stencilInfo : stencils) {
      // build vals array
      for (auto rowIndex: stencilInfo.second) {
        for (int k = csrMatrix->rows[rowIndex]; k < csrMatrix->rows[rowIndex+1]; ++k) {
          *valPtr++ = csrMatrix->vals[k];
        }
      }
      // build rows array
      if (stencilInfo.second.size() > 1) {
        for (auto rowIndex: stencilInfo.second) {
          *rowPtr++ = rowIndex;
        }
      }
    }
  }
  END_OPTIONAL_TIME_PROFILE(matrixConversion);
  
  Matrix *result = new Matrix(rows, NULL, vals, csrMatrix->n, csrMatrix->nz);
  result->numRows = csrMatrix->n;
  result->numCols = 0;
  result->numVals = csrMatrix->nz;
  return result;
}

void Stencil::dumpAssemblyText() {
  START_OPTIONAL_TIME_PROFILE(getMatrix);
  this->getMatrix(); // this is done to measure the cost of matrix prep.
  vector<StencilToRowMap> *stencilLists = analyzer.getStencilLists(); // this has zero cost, because already calculated by getMatrix()
  vector<MatrixStripeInfo> &stripeInfos = csrMatrix->getStripeInfos();
  END_OPTIONAL_TIME_PROFILE(getMatrix);

  START_OPTIONAL_TIME_PROFILE(emitCode);
  // Set up code emitters
  vector<StencilCodeEmitter> codeEmitters;
  for (unsigned i = 0; i < stencilLists->size(); i++) {
    auto &stencilList = stencilLists->at(i);
    codeEmitters.push_back(StencilCodeEmitter(&stencilList, stripeInfos[i].valIndexBegin, stripeInfos[i].rowIndexBegin, Str, i));
  }
  
#pragma omp parallel for
  for (int threadIndex = 0; threadIndex < NUM_OF_THREADS; ++threadIndex) {
    codeEmitters[threadIndex].emit();
  }
  END_OPTIONAL_TIME_PROFILE(emitCode);
}

StencilCodeEmitter::StencilCodeEmitter(StencilToRowMap *stencils, unsigned long baseValsIndex,
                                       unsigned long baseRowsIndex, llvm::MCStreamer *Str, unsigned int partitionIndex):
stencils(stencils), baseValsIndex(baseValsIndex), baseRowsIndex(baseRowsIndex) {
  this->DFOS = createNewDFOS(Str, partitionIndex);
}

void StencilCodeEmitter::emit() {
  dumpPushPopHeader();
  
  for (auto &stencilInfo : *stencils) {
    dumpStencilAssemblyText(stencilInfo.first, stencilInfo.second);
  }
  
  dumpPushPopFooter();
  emitRETInst();
}

void StencilCodeEmitter::dumpPushPopHeader() {
  // rows is in %rdx, cols is in %rcx, vals is in %r8
  emitPushPopInst(X86::PUSH64r,X86::RBX);
  emitLEAQInst(X86::R8, X86::RBX, sizeof(double) * baseValsIndex); // using %rbx for vals
  
  emitPushPopInst(X86::PUSH64r,X86::R8);
  emitLEAQInst(X86::RDX, X86::R8, sizeof(int) * baseRowsIndex); // using %r8 for rows
  
  emitPushPopInst(X86::PUSH64r,X86::R11);
  emitPushPopInst(X86::PUSH64r,X86::RDX);
}

void StencilCodeEmitter::dumpPushPopFooter() {
  emitPushPopInst(X86::POP64r, X86::RDX);
  emitPushPopInst(X86::POP64r, X86::R11);
  emitPushPopInst(X86::POP64r, X86::R8);
  emitPushPopInst(X86::POP64r, X86::RBX);
}

void StencilCodeEmitter::dumpStencilAssemblyText(const StencilPattern &stencil,
                                                 const RowIndices &rowIndices) {
  unsigned long popularity = rowIndices.size();
  unsigned long stencilSize = stencil.size();
  if(stencilSize == 0 || popularity == 0) return;
  
  int row = rowIndices[0];
  unsigned long labeledBlockBeginningOffset = 0;
  
  if (popularity == 1) {
    //  movsd  "8*(row+stencil[0])"(%rdi), %xmm1
    emitMOVSDrmInst((8*(row+stencil[0])), X86::RDI, 1);
  } else {
    //  xorl %r11d, %r11d
    emitXOR32rrInst(X86::R11D, X86::R11D);
    //  .align 4, 0x90
    emitCodeAlignment(16);
    //  LBB_"row":
    labeledBlockBeginningOffset = DFOS->size();
    
    //  movslq (%r11,%r8), %rdx
    emitMOVSLQInst(X86::RDX, X86::R11, X86::R8, 1, 0);
    //  movsd "8*(stencil[0])"(%rdi,%rdx,8), %xmm1
    emitMOVSDrmInst((8*(stencil[0])), X86::RDI, X86::RDX, 8, 1);
  }
  
  //  mulsd (%RBX), %xmm1
  emitMULSDrmInst(0, X86::RBX, 1);
  
  for(int i = 1; i < stencilSize; ++i) {
    if (popularity == 1) {
      //  movsd "8*(row+stencil[i])"(%rdi), %xmm0
      emitMOVSDrmInst((8*(row+stencil[i])), X86::RDI, 0);
    } else {
      //  movsd "8*(stencil[i])"(%rdi,%rdx,8), %xmm0
      emitMOVSDrmInst((8*(stencil[i])), X86::RDI, X86::RDX, 8, 0);
    }
    //  mulsd "8*(i)"(%RBX), %xmm0
    emitMULSDrmInst(8*(i), X86::RBX, 0);
    //  addsd %xmm0, %xmm1
    emitRegInst(X86::ADDSDrr, 0, 1);
  }
  if (popularity > 1) {
    //  addsd (%rsi,%rdx,8), %xmm1
    emitADDSDrmInst(0, X86::RSI, X86::RDX, 8, 1);
    //  addq $"sizeof(int)", %r11
    emitADDQInst(sizeof(int), X86::R11);
    //  movsd %xmm1, (%rsi,%rdx,8)
    emitMOVSDmrInst(1, 0, X86::RSI, X86::RDX, 8);
  }
  
  //  leaq "sizeof(double)*stencilSize"(%RBX), %RBX
  emitLEAQInst(X86::RBX, X86::RBX, (int)(sizeof(double)*stencilSize));
  
  if (popularity == 1) {
    //  addsd "sizeof(double)*row"(%rsi), %xmm1
    emitADDSDrmInst((sizeof(double)*row), X86::RSI, 1);
    //  movsd %xmm1, "sizeof(double)*row"(%rsi)
    emitMOVSDmrInst(1, (sizeof(double)*row), X86::RSI);
  } else {
    //  cmpl $"popularity*sizeof(int)", %r11d
    emitCMP32riInst(X86::R11D, (int)popularity*sizeof(int));
    //  jne LBB_"row"
    emitJNEInst(labeledBlockBeginningOffset);
    
    //  leaq "sizeof(int)*popularity"(%r8), %r8
    emitLEAQInst(X86::R8, X86::R8, (int)(sizeof(int)*popularity));
  }
}


