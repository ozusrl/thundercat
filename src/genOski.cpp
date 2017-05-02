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

class GenOSKICodeEmitter : public SpMVCodeEmitter {
private:
  GroupByBlockPatternMap *patternMap;
  unsigned int b_r, b_c;
  unsigned long baseValsIndex, baseBlockIndex;
  
  void dumpForLoops();
  
protected:
  virtual void dumpPushPopHeader();
  virtual void dumpPushPopFooter();
  
public:
  GenOSKICodeEmitter(GroupByBlockPatternMap *patternMap,
                     unsigned long baseValsIndex,
                     unsigned long baseBlockIndex,
                     unsigned int b_r,
                     unsigned int b_c,
                     llvm::MCStreamer *Str, unsigned int partitionIndex);
  void emit();
};


GenOSKI::GenOSKI(Matrix *csrMatrix, unsigned b_r, unsigned b_c):
  SpMVMethod(csrMatrix), b_r(b_r), b_c(b_c), analyzer(csrMatrix, b_r, b_c)
{
}

Matrix* GenOSKI::getMatrixForGeneration() {
  START_OPTIONAL_TIME_PROFILE(getGenOSKIInfo);
  vector<GroupByBlockPatternMap> *blockPatternLists = analyzer.getGroupByBlockPatternMaps();
  END_OPTIONAL_TIME_PROFILE(getGenOSKIInfo);

  START_OPTIONAL_TIME_PROFILE(matrixConversion);
  vector<unsigned int> &numBlocks = analyzer.getNumBlocks();
  unsigned int numTotalBlocks = 0;
  vector<unsigned int> blockBaseIndices;
  for (auto n : numBlocks) {
    blockBaseIndices.push_back(numTotalBlocks);
    numTotalBlocks += n;
  }
  
  int *rows = new int[numTotalBlocks];
  int *cols = new int[numTotalBlocks];
  double *vals = new double[csrMatrix->nz];
  
  vector<MatrixStripeInfo> &stripeInfos = csrMatrix->getStripeInfos();
  
#pragma omp parallel for
  for (int t = 0; t < NUM_OF_THREADS; ++t) {
    auto &blockPatterns = blockPatternLists->at(t);
    int *rowsPtr = rows + blockBaseIndices[t];
    int *colsPtr = cols + blockBaseIndices[t];
    double *valsPtr = vals + stripeInfos[t].valIndexBegin;
    
    //Build rows cols vals for the new Matrix
    for (auto &patternInfo : blockPatterns) {
      for (auto &blockIdx : patternInfo.second.first) {
        *rowsPtr++ = blockIdx.first * b_r;
        *colsPtr++ = blockIdx.second * b_c;
      }
      for (auto &val : patternInfo.second.second) {
        *valsPtr++ = val;
      }
    }
  }
  END_OPTIONAL_TIME_PROFILE(matrixConversion);

  Matrix *result = new Matrix(rows, cols, vals, csrMatrix->n, csrMatrix->nz);
  result->numRows = numTotalBlocks;
  result->numCols = numTotalBlocks;
  result->numVals = csrMatrix->nz;
  return result;
}

void GenOSKI::dumpAssemblyText() {
  START_OPTIONAL_TIME_PROFILE(getMatrix);
  this->getMatrix(); // Only for benchmarking purposes.
  vector<GroupByBlockPatternMap> *patternMaps = analyzer.getGroupByBlockPatternMaps();
  vector<MatrixStripeInfo> &stripeInfos = csrMatrix->getStripeInfos();
  END_OPTIONAL_TIME_PROFILE(getMatrix);

  START_OPTIONAL_TIME_PROFILE(emitCode);
  vector<unsigned int> &numBlocks = analyzer.getNumBlocks();
  unsigned int numTotalBlocks = 0;
  vector<unsigned int> blockBaseIndices;
  for (auto n : numBlocks) {
    blockBaseIndices.push_back(numTotalBlocks);
    numTotalBlocks += n;
  }
  
  // Set up code emitters
  vector<GenOSKICodeEmitter> codeEmitters;
  for (unsigned i = 0; i < patternMaps->size(); i++) {
    auto &patternMap = patternMaps->at(i);
    codeEmitters.push_back(GenOSKICodeEmitter(&patternMap, stripeInfos[i].valIndexBegin, blockBaseIndices[i], b_r, b_c, Str, i));
  }
  
#pragma omp parallel for
  for (int threadIndex = 0; threadIndex < NUM_OF_THREADS; ++threadIndex) {
    codeEmitters[threadIndex].emit();
  }
  END_OPTIONAL_TIME_PROFILE(emitCode);
}

GenOSKICodeEmitter::GenOSKICodeEmitter(GroupByBlockPatternMap *patternMap,
                                       unsigned long baseValsIndex,
                                       unsigned long baseBlockIndex,
                                       unsigned int b_r,
                                       unsigned int b_c,
                                       llvm::MCStreamer *Str, unsigned int partitionIndex):
patternMap(patternMap), baseValsIndex(baseValsIndex), baseBlockIndex(baseBlockIndex), b_r(b_r), b_c(b_c) {
  this->DFOS = createNewDFOS(Str, partitionIndex);
}

void GenOSKICodeEmitter::emit() {
  dumpPushPopHeader();
  
  dumpForLoops();
  
  dumpPushPopFooter();
}

void GenOSKICodeEmitter::dumpPushPopHeader() {
  emitPushArmInst();
  emitLDROffsetArmInst(ARM::R7, ARM::SP, 32); // load vals into R7
  emitADDOffsetArmInst(ARM::R2, ARM::R2, (int)(sizeof(int) * baseBlockIndex));
  emitADDOffsetArmInst(ARM::R3, ARM::R3, (int)(sizeof(int) * baseBlockIndex));
  emitADDOffsetArmInst(ARM::R7, ARM::R7, (int)(sizeof(double) * baseValsIndex));
}

void GenOSKICodeEmitter::dumpPushPopFooter() {
  emitPopArmInst();
}

void GenOSKICodeEmitter::dumpForLoops() {
  
  int size = b_r * b_c;
  for (auto &pattern : *patternMap) {
  
    emitEORArmInst(ARM::R8, ARM::R8, ARM::R8);
    emitARMCodeAlignment(32);
    unsigned long labeledBlockBeginningOffset = DFOS->size();
    
    emitLDMArmInst(ARM::R2, ARM::R4, ARM::R4);
    emitADDRegisterArmInst(ARM::R4, ARM::R1, ARM::R4, 3); //www  
    emitLDMArmInst(ARM::R3, ARM::R5, ARM::R5);
    emitADDRegisterArmInst(ARM::R5, ARM::R0, ARM::R5, 3); //VV
    
    bitset<32> patternBits(pattern.first);
    unsigned int numBlocks = pattern.second.first.size();
    int nz = patternBits.count(); // nz elements per pattern
    
    //startingMMElements simulation <row, Cols>
    map<int, vector<int> > patternLocs;
    for (int j = 0; j < size; ++j) {
      if (patternBits[j] == 1) {
        patternLocs[j / b_c].push_back(j % b_c);
      }
    }
    
    int bb = 0;
    for (auto &nz : patternLocs) {
      int row = nz.first;
      vector<int> cols = nz.second;
      vector<int>::iterator colsIt = cols.begin(), colsEnd = cols.end();

      emitVLDRArmInst(ARM::D18, ARM::R4, row * sizeof(double));

      while (colsIt != colsEnd) {
        emitVLDRArmInst(ARM::D16, ARM::R5, (*colsIt++) * sizeof(double));
        emitVLDRArmInst(ARM::D17, ARM::R7, (bb++) * sizeof(double));
        emitVMLAArmInst(ARM::D18, ARM::D16, ARM::D17);
      }

      emitVSTRArmInst(ARM::D18, ARM::R4, row * sizeof(double));
    }

    emitADDOffsetArmInst(ARM::R7, ARM::R7, sizeof(double) * bb);
    emitADDOffsetArmInst(ARM::R8, ARM::R8, sizeof(int));
    emitCMPOffsetArmInst(ARM::R8, numBlocks * sizeof(int), ARM::R9);
    emitBNEArmInst(labeledBlockBeginningOffset);  
  } 
}


