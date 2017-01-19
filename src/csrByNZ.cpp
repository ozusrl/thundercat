#include "method.h"
#include "profiler.h"
#include "csrByNZAnalyzer.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include "lib/Target/ARM/MCTargetDesc/ARMBaseInfo.h"

#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCInstBuilder.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/MCObjectFileInfo.h"

using namespace llvm;
using namespace spMVgen;
using namespace std;

extern unsigned int NUM_OF_THREADS;

class CSRbyNZCodeEmitter : public SpMVCodeEmitter {
private:
  NZtoRowMap *rowByNZs;
  unsigned long baseValsIndex, baseRowsIndex;
  
  void dumpSingleLoop(unsigned long numRows, unsigned long rowLength);
  
protected:
  virtual void dumpPushPopHeader();
  virtual void dumpPushPopFooter();
  
public:
  CSRbyNZCodeEmitter(NZtoRowMap *rowByNZs, unsigned long baseValsIndex,
                     unsigned long baseRowsIndex, llvm::MCStreamer *Str, unsigned int partitionIndex);
  
  void emit();
};


CSRbyNZ::CSRbyNZ(Matrix *csrMatrix):
  SpMVMethod(csrMatrix), analyzer(csrMatrix) {
  // do nothing
}

// Return a matrix to be used by CSRbyNZ
// rows: row indices, sorted by row lengths
// cols: indices of elements,
//       sorted according to the order used in rows array
// vals: values as usual,
//       sorted according to the order used in rows array
Matrix* CSRbyNZ::getMatrixForGeneration() {
  START_OPTIONAL_TIME_PROFILE(getCSRbyNZInfo);
  vector<NZtoRowMap> *rowByNZLists = analyzer.getRowByNZLists();
  END_OPTIONAL_TIME_PROFILE(getCSRbyNZInfo);
  
  START_OPTIONAL_TIME_PROFILE(matrixConversion);
  int *rows = new int[csrMatrix->n];
  int *cols = new int[csrMatrix->nz];
  double *vals = new double[csrMatrix->nz];

  vector<MatrixStripeInfo> &stripeInfos = csrMatrix->getStripeInfos();

#pragma omp parallel for
  for (int t = 0; t < NUM_OF_THREADS; ++t) {
    auto &rowByNZList = rowByNZLists->at(t);
    int *rowsPtr = rows + stripeInfos[t].rowIndexBegin;
    int *colsPtr = cols + stripeInfos[t].valIndexBegin;
    double *valsPtr = vals + stripeInfos[t].valIndexBegin;
    
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
  END_OPTIONAL_TIME_PROFILE(matrixConversion);

  return new Matrix(rows, cols, vals, csrMatrix->n, csrMatrix->nz);
}

void CSRbyNZ::dumpAssemblyText() {
  START_OPTIONAL_TIME_PROFILE(getMatrix);
  this->getMatrix(); // this is done to measure the cost of matrix prep.
  vector<NZtoRowMap> *rowByNZLists = analyzer.getRowByNZLists(); // this has zero cost, because already calculated by getMatrix()
  vector<MatrixStripeInfo> &stripeInfos = csrMatrix->getStripeInfos();
  END_OPTIONAL_TIME_PROFILE(getMatrix);

  START_OPTIONAL_TIME_PROFILE(emitCode);
  // Set up code emitters
  vector<CSRbyNZCodeEmitter> codeEmitters;
  for (unsigned i = 0; i < rowByNZLists->size(); i++) {
    NZtoRowMap &rowByNZs = rowByNZLists->at(i);
    codeEmitters.push_back(CSRbyNZCodeEmitter(&rowByNZs, stripeInfos[i].valIndexBegin, stripeInfos[i].rowIndexBegin, Str, i));
  }
  
#pragma omp parallel for
  for (int threadIndex = 0; threadIndex < NUM_OF_THREADS; ++threadIndex) {
    codeEmitters[threadIndex].emit();
  }
  END_OPTIONAL_TIME_PROFILE(emitCode);
}


CSRbyNZCodeEmitter::CSRbyNZCodeEmitter(NZtoRowMap *rowByNZs, unsigned long baseValsIndex,
                                       unsigned long baseRowsIndex, llvm::MCStreamer *Str, unsigned int partitionIndex):
rowByNZs(rowByNZs), baseValsIndex(baseValsIndex), baseRowsIndex(baseRowsIndex) {
  this->DFOS = createNewDFOS(Str, partitionIndex);
}

void CSRbyNZCodeEmitter::emit() {
  dumpPushPopHeader();
  
  for (auto &rowByNZ : *rowByNZs) {
    unsigned long rowLength = rowByNZ.first;
    dumpSingleLoop(rowByNZ.second.getRowIndices()->size(), rowLength);
  }
  
  dumpPushPopFooter();
}
  
void CSRbyNZCodeEmitter::dumpPushPopHeader() {
  emitPushArmInst();

  emitLDROffsetArmInst(ARM::R7, ARM::SP, 8); // load vals into R7
}

void CSRbyNZCodeEmitter::dumpPushPopFooter() {
  emitPopArmInst();
}

void CSRbyNZCodeEmitter::dumpSingleLoop(unsigned long numRows, unsigned long rowLength) {
  // v is in R0, w is in R1, rows is in R2, cols is in R3, vals is in R7 

  emitMOVArmInst(ARM::R8, 0x0); // loop counter 'a'
  emitMOVWArmInst(ARM::R9, numRows * sizeof(int)); // loop limit
  
  unsigned long labeledBlockBeginningOffset = DFOS->size();
  emitVMOVI32ArmInst(ARM::D16, 0x0);
  for (int i = 0 ; i < rowLength ; i++) {
    emitLDROffsetArmInst(ARM::R5, ARM::R3, i); // What if i >= 256?
    emitVLDRArmInst(ARM::D17, ARM::R7, i); // What if i >= 256?
    emitADDRegisterArmInst(ARM::R5, ARM::R0, ARM::R5, 3);
    emitVLDRArmInst(ARM::D20, ARM::R5, 0x0);
    emitVMULArmInst(ARM::D17, ARM::D17, ARM::D20);
    emitVADDArmInst(ARM::D16, ARM::D16, ARM::D17);
  }
  
  emitLDRRegisterArmInst(ARM::R5, ARM::R2, ARM::R8);
  emitADDRegisterArmInst(ARM::R5, ARM::R1, ARM::R5, 3);
  
  emitVLDRArmInst(ARM::D18, ARM::R5, 0x0); // load w[row] into D18
  emitVADDArmInst(ARM::D18, ARM::D18, ARM::D16);
  emitVSTRArmInst(ARM::D18, ARM::R5);
  
  emitADDOffsetArmInst(ARM::R8, ARM::R8, sizeof(int)); 
  emitCMPRegisterArmInst(ARM::R8, ARM::R9);
  emitBNEArmInst(labeledBlockBeginningOffset);
  
  emitADDRegisterArmInst(ARM::R2, ARM::R2, ARM::R8, 0); // R8's value at this point is "numRows * sizeof(int)"
  emitADDOffsetArmInst(ARM::R3, ARM::R3, numRows * rowLength * sizeof(int));
  emitADDOffsetArmInst(ARM::R7, ARM::R7, numRows * rowLength * sizeof(double));
}
