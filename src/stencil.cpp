#include "method.h"
#include "profiler.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include "lib/Target/ARM/MCTargetDesc/ARMBaseInfo.h"
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
}

void StencilCodeEmitter::dumpPushPopHeader() {
  emitPushArmInst();
  emitLDROffsetArmInst(ARM::R7, ARM::SP, 32); // load vals into R7
}

void StencilCodeEmitter::dumpPushPopFooter() {
  emitPopArmInst();
}

#define LDR_IMM_LIMIT 128

void StencilCodeEmitter::dumpStencilAssemblyText(const StencilPattern &stencil,
                                                 const RowIndices &rowIndices) {
  unsigned long popularity = rowIndices.size();
  unsigned long stencilSize = stencil.size();
  if(stencilSize == 0 || popularity == 0) return;
  
  if (popularity > 1) {
    emitMOVArmInst(ARM::R8, 0x0);
  }
  unsigned long labeledBlockBeginningOffset = DFOS->size();
  if (popularity > 1) {
    emitLDRRegisterArmInst(ARM::R6, ARM::R2, ARM::R8);
    emitADDRegisterArmInst(ARM::R5, ARM::R0, ARM::R6, 0x3);
  } else {
    emitADDOffsetArmInst(ARM::R5, ARM::R0, sizeof(double) * rowIndices[0]);
  }
  emitVMOVI32ArmInst(ARM::D16, 0x0);
  unsigned numShiftings = 0;
  int vPtrPositionRelativeToDiagonal = 0;

  for (int i = 0; i < stencilSize; i++) {
    if (i % LDR_IMM_LIMIT == 0 && i != 0) {
      emitADDOffsetArmInst(ARM::R7, ARM::R7, LDR_IMM_LIMIT * sizeof(double));
      numShiftings++;
    }
    if (abs(vPtrPositionRelativeToDiagonal - stencil[i]) >= LDR_IMM_LIMIT) {
      // re-adjust the pointer.
      // -1 to stay in left-edge of the (-1024,1024) range of allowable VLDR offset
      int adjustment = stencil[i] - vPtrPositionRelativeToDiagonal + LDR_IMM_LIMIT - 1;
      if (adjustment >= 0) {
	emitADDOffsetArmInst(ARM::R5, ARM::R5, adjustment * sizeof(double));
      } else {
	emitSUBOffsetArmInst(ARM::R5, ARM::R5, 0 - adjustment * sizeof(double));
      }
      vPtrPositionRelativeToDiagonal += adjustment;
    }

    emitVLDRArmInst(ARM::D18, ARM::R5, (stencil[i] - vPtrPositionRelativeToDiagonal) * sizeof(double));
    emitVLDRArmInst(ARM::D17, ARM::R7, (i % LDR_IMM_LIMIT) * sizeof(double));
    emitVMULArmInst(ARM::D17, ARM::D17, ARM::D18);
    emitVADDArmInst(ARM::D16, ARM::D16, ARM::D17);
  }
  
  if (popularity > 1) {
    emitADDRegisterArmInst(ARM::R5, ARM::R1, ARM::R6, 3);
    emitADDOffsetArmInst(ARM::R8, ARM::R8, sizeof(int));
    emitVLDRArmInst(ARM::D18, ARM::R5, 0); // load w[row] into D18
    emitADDOffsetArmInst(ARM::R7, ARM::R7, (stencilSize - numShiftings * LDR_IMM_LIMIT) * sizeof(double));
    emitCMPOffsetArmInst(ARM::R8, (int)popularity * sizeof(int), ARM::R9);
    emitVADDArmInst(ARM::D18, ARM::D18, ARM::D16);
    emitVSTRArmInst(ARM::D18, ARM::R5);
    emitBNEArmInst(labeledBlockBeginningOffset);
    emitADDRegisterArmInst(ARM::R2, ARM::R2, ARM::R8, 0); // R8's value at this point is "numRows * sizeof(int)"
  } else {
    emitADDOffsetArmInst(ARM::R5, ARM::R1, rowIndices[0] * sizeof(double));
    emitVLDRArmInst(ARM::D18, ARM::R5, 0); // load w[row] into D18
    emitADDOffsetArmInst(ARM::R7, ARM::R7, (stencilSize - numShiftings * LDR_IMM_LIMIT) * sizeof(double));
    emitVADDArmInst(ARM::D18, ARM::D18, ARM::D16);
    emitVSTRArmInst(ARM::D18, ARM::R5);
  }
}
