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

#define REGISTER_LIMIT 7
#define LIMIT_TO_DO_LEAQ 120

class UnfoldingCodeEmitter : public SpMVCodeEmitter {
public:
  UnfoldingCodeEmitter(Matrix *csrMatrix,
                       MatrixStripeInfo *stripeInfo,
                       llvm::MCStreamer *Str, unsigned int partitionIndex);
  virtual void emit();
  
protected:
  unsigned long numWPointerShiftings;
  
  void dumpRow(int rowIndex);
  virtual void dumpPartialRow(vector<vector<int> > &partitions, unsigned partitionIndex, bool haveToAccumulateResult);
  void dumpRowConclusion(int rowIndex, bool hadToSplitRow);
  void dumpRegisterReduce(vector<bool> &signs);

  virtual void partitionRowElements(int rowIndex, vector<vector<int> > &partitions);

  virtual void dumpPushPopHeader();
  virtual void dumpValsPointerAdjustment();
  virtual void dumpPushPopFooter();
  
  Matrix *csrMatrix;
  MatrixStripeInfo *stripeInfo;
  
private:
  unsigned long getValIndexAndShiftValsPointer(int elementIndex);
};

//--------------------------------------------------------------------------------------

class UnfoldingWithDistinctValuesCodeEmitter : public UnfoldingCodeEmitter {
  friend class UnfoldingCodeEmitter;
public:
  UnfoldingWithDistinctValuesCodeEmitter(Matrix *csrMatrix,
                                         MatrixStripeInfo *stripeInfo,
                                         map<double, unsigned long> *valToIndexMap,
                                         unsigned long baseValsIndex,
                                         llvm::MCStreamer *Str, unsigned int partitionIndex);
private:
  std::map<double, unsigned long> *valToIndexMap;
  unsigned long baseValsIndex;
  
protected:
  virtual void dumpValsPointerAdjustment();
  virtual void dumpPartialRow(vector<vector<int> > &partitions, unsigned partitionIndex, bool haveToAccumulateResult);
  virtual void partitionRowElements(int rowIndex, vector<vector<int> > &partitions);
};

Unfolding::Unfolding(Matrix *csrMatrix):
  SpMVMethod(csrMatrix), analyzer(csrMatrix) {
}

Matrix* Unfolding::getMatrixForGeneration() {
  START_OPTIONAL_TIME_PROFILE(getUnfoldingInfo);
  const auto valueLists = analyzer.getValues();
  END_OPTIONAL_TIME_PROFILE(getUnfoldingInfo);
  
  START_OPTIONAL_TIME_PROFILE(matrixConversion)
  // The hack below is used when negating
  // FP values
  unsigned long *cols = new unsigned long[2];   // Yes, this is unsigned long!
  cols[0] = 0x8000000000000000;
  cols[1] = 0x0000000000000000;

  double *values = NULL;
  unsigned long numVals = 0;
  if (analyzer.hasFewDistinctValues()) {
    for (auto &partitionValues : *valueLists) {
      numVals += partitionValues.size();
    }
    values = new double[numVals];
    double *valPtr = values;
    for (auto &partitionValues : *valueLists) {
      memcpy(valPtr, partitionValues.data(), partitionValues.size() * sizeof(double));
      valPtr += partitionValues.size();
    }
  } else {
    values = csrMatrix->vals;
    numVals = csrMatrix->nz;
  }
  END_OPTIONAL_TIME_PROFILE(matrixConversion)

  Matrix *result = new Matrix(NULL, (int*)cols, values, csrMatrix->n, csrMatrix->nz);
  result->numRows = 0;
  result->numCols = 2*sizeof(unsigned long)/sizeof(int);
  result->numVals = numVals;
  return result;
}

void Unfolding::dumpAssemblyText() {
  START_OPTIONAL_TIME_PROFILE(getMatrix);
  this->getMatrix(); // this is done to measure the cost of matrix prep.
  auto valToIndexMaps = analyzer.getValToIndexMaps(); // this has zero cost, because already calculated by getMatrix()
  END_OPTIONAL_TIME_PROFILE(getMatrix);

  START_OPTIONAL_TIME_PROFILE(emitCode);
  vector<MatrixStripeInfo> &stripeInfos = csrMatrix->getStripeInfos();
  
  // Set up code emitters
  vector<UnfoldingCodeEmitter*> codeEmitters;
  unsigned long baseValsIndex = 0;
  for (unsigned i = 0; i < stripeInfos.size(); i++) {
    auto &stripeInfo = stripeInfos.at(i);
    if (analyzer.hasFewDistinctValues()) {
      codeEmitters.push_back(new UnfoldingWithDistinctValuesCodeEmitter(csrMatrix, &stripeInfo, &(valToIndexMaps->at(i)), baseValsIndex, Str, i));
    } else {
      codeEmitters.push_back(new UnfoldingCodeEmitter(csrMatrix, &stripeInfo, Str, i));
    }
    baseValsIndex += valToIndexMaps->at(i).size();
  }
  
#pragma omp parallel for
  for (int threadIndex = 0; threadIndex < NUM_OF_THREADS; ++threadIndex) {
    codeEmitters[threadIndex]->emit();
  }

  for (UnfoldingCodeEmitter *emitter : codeEmitters) {
    delete emitter;
  }
  END_OPTIONAL_TIME_PROFILE(emitCode);
}


UnfoldingCodeEmitter::UnfoldingCodeEmitter(Matrix *csrMatrix,
                                           MatrixStripeInfo *stripeInfo,
                                           llvm::MCStreamer *Str, unsigned int partitionIndex):
csrMatrix(csrMatrix), stripeInfo(stripeInfo) {
  this->DFOS = createNewDFOS(Str, partitionIndex);
  numWPointerShiftings = 0;
}

UnfoldingWithDistinctValuesCodeEmitter::UnfoldingWithDistinctValuesCodeEmitter(Matrix *csrMatrix,
                                                                               MatrixStripeInfo *stripeInfo,
                                                                               map<double, unsigned long> *valToIndexMap,
                                                                               unsigned long baseValsIndex,
                                                                               llvm::MCStreamer *Str, unsigned int partitionIndex):
UnfoldingCodeEmitter(csrMatrix, stripeInfo, Str, partitionIndex), valToIndexMap(valToIndexMap), baseValsIndex(baseValsIndex) {
  
}

void UnfoldingCodeEmitter::emit() {
  dumpPushPopHeader();

  for (int rowIndex = stripeInfo->rowIndexBegin; rowIndex < stripeInfo->rowIndexEnd; ++rowIndex) {
    dumpRow(rowIndex);
  }
  
  dumpPushPopFooter();
  emitRETInst();
}

void UnfoldingCodeEmitter::dumpPushPopHeader() {
  // rows is in %rdx, cols is in %rcx, vals is in %r8
  emitPushPopInst(X86::PUSH64r, X86::RDX);
  emitPushPopInst(X86::PUSH64r, X86::RSI);
  
  dumpValsPointerAdjustment();

  int firstRow = stripeInfo->rowIndexBegin;
  emitLEAQInst(X86::RSI, X86::RSI, (firstRow * sizeof(double)) / LIMIT_TO_DO_LEAQ * LIMIT_TO_DO_LEAQ);
  numWPointerShiftings = (firstRow * sizeof(double)) / LIMIT_TO_DO_LEAQ;
}

void UnfoldingCodeEmitter::dumpValsPointerAdjustment() {
  // Create a copy of vals pointer (R8) in RDX.
  //  leaq "offset"(%r8), %rdx
  emitLEAQInst(X86::R8, X86::RDX, (stripeInfo->valIndexBegin * sizeof(double)) / LIMIT_TO_DO_LEAQ * LIMIT_TO_DO_LEAQ);
}

void UnfoldingWithDistinctValuesCodeEmitter::dumpValsPointerAdjustment() {
  // Create a copy of vals pointer (R8) in RDX.
  //  leaq "offset"(%r8), %rdx
  emitLEAQInst(X86::R8, X86::RDX, baseValsIndex * sizeof(double));
}

void UnfoldingCodeEmitter::dumpPushPopFooter() {
  emitPushPopInst(X86::POP64r, X86::RSI);
  emitPushPopInst(X86::POP64r, X86::RDX);
}

void UnfoldingCodeEmitter::dumpRow(int rowIndex) {
  int rowLength = csrMatrix->rows[rowIndex+1] - csrMatrix->rows[rowIndex];
  if (rowLength == 0) return;
  
  vector<vector<int> > partitions;
  partitionRowElements(rowIndex, partitions);

  bool haveToUseAccumulator = partitions.size() > 1;
  
  if (haveToUseAccumulator) { // reset the accumulation register
    //  xorps %xmm15, %xmm15
    emitRegInst(X86::XORPSrr, REGISTER_LIMIT, REGISTER_LIMIT);
  }
  // First, do partial sums as much as needed
  for (unsigned partitionIndex = 0; partitionIndex < partitions.size(); partitionIndex++) {
    dumpPartialRow(partitions, partitionIndex, haveToUseAccumulator);
  }
  
  // Finally, write the result to the output vector
  dumpRowConclusion(rowIndex, haveToUseAccumulator);
}

static void splitElements(pair<int, int> range, vector<vector<int> > &partitions, unsigned int chunkSize) {
  int length = (range.second - range.first);
  unsigned long numPartitions = length / chunkSize;
  if (length % chunkSize != 0)
    numPartitions += 1;
  partitions.resize(numPartitions);
  
  for (unsigned int i = 0; i < length; ++i) {
    unsigned int partitionIndex = i / chunkSize;
    partitions[partitionIndex].push_back(range.first + i);
  }
}

static void splitElements(int *elements, int length, vector<vector<int> > &partitions, unsigned int chunkSize) {
  unsigned long numPartitions = length / chunkSize;
  if (length % chunkSize != 0)
    numPartitions += 1;
  partitions.resize(numPartitions);
  
  for (unsigned int i = 0; i < length; ++i) {
    unsigned int partitionIndex = i / chunkSize;
    partitions[partitionIndex].push_back(elements[i]);
  }
}

void UnfoldingCodeEmitter::partitionRowElements(int rowIndex, vector<vector<int> > &partitions) {
  splitElements(pair<int, int>(csrMatrix->rows[rowIndex], csrMatrix->rows[rowIndex+1]), partitions, REGISTER_LIMIT);
}

void UnfoldingCodeEmitter::dumpPartialRow(vector<vector<int> > &partitions, unsigned partitionIndex, bool haveToAccumulateResult) {
  vector<int> &elements = partitions[partitionIndex];
  // Move vector elements to registers
  unsigned int vRegIndex = 0;
  for (auto eltIndex : elements) {
    int colIndex = csrMatrix->cols[eltIndex];
    //  movsd "sizeof(double)*colIndex"(%rdi), %xmm"vRegIndex"
    emitMOVSDrmInst(sizeof(double)*colIndex, X86::RDI, vRegIndex);
    vRegIndex++;
  }
  
  // Multiply matrix values with vector elements
  vRegIndex = 0;
  for (auto eltIndex : elements) {
    double value = csrMatrix->vals[eltIndex];
    unsigned long valIndex = getValIndexAndShiftValsPointer(eltIndex);
    if (value == -1.0) {
      // do nothing
    } else if (value == 1.0) {
      // do nothing
    } else {
      // mulsd "valIndex"(%rdx), %xmm"vRegIndex"
      emitMULSDrmInst((int)valIndex, X86::RDX, vRegIndex);
    }
    vRegIndex++;
  }
  
  vector<bool> signs;
  for (auto eltIndex: elements) {
      signs.push_back(csrMatrix->vals[eltIndex] != -1);
  }
  dumpRegisterReduce(signs);
  
  if (haveToAccumulateResult) {
    if (!signs[0]) {
      //  subsd %xmm0, %xmm15
      emitRegInst(X86::SUBSDrr, 0, REGISTER_LIMIT);
    } else {
      //  addsd %xmm0, %xmm15
      emitRegInst(X86::ADDSDrr, 0, REGISTER_LIMIT);
    }
  } else if (!signs[0]) {
    // negate the register
    // xorpd (%rcx), %xmm0
    emitFPNegation(0);
  }
}

unsigned long UnfoldingCodeEmitter::getValIndexAndShiftValsPointer(int elementIndex) {
  unsigned long valIndex = sizeof(double) * elementIndex;
  valIndex -= ((stripeInfo->valIndexBegin * sizeof(double)) / LIMIT_TO_DO_LEAQ * LIMIT_TO_DO_LEAQ);
  if (valIndex % LIMIT_TO_DO_LEAQ == 0 && valIndex != 0) {
    //  leaq "LIMIT_TO_DO_LEAQ"(%rdx), %rdx
    emitLEAQInst(X86::RDX, X86::RDX, LIMIT_TO_DO_LEAQ);
  }
  valIndex = valIndex % LIMIT_TO_DO_LEAQ;
  return valIndex;
}

void UnfoldingWithDistinctValuesCodeEmitter::partitionRowElements(int rowIndex, vector<vector<int> > &partitions) {
  // Find distinct values of this row
  set<double> distinctValues;
  for (int k = csrMatrix->rows[rowIndex]; k < csrMatrix->rows[rowIndex+1]; ++k) {
    distinctValues.insert(csrMatrix->vals[k]);
  }
  
  // For each distinct value, group the MMElements that have that distinct val
  // and partition each group.
  for (double distinctVal : distinctValues) {
    vector<int> distinctValIndices;
    for (int k = csrMatrix->rows[rowIndex]; k < csrMatrix->rows[rowIndex+1]; ++k) {
      if (csrMatrix->vals[k] == distinctVal)
        distinctValIndices.push_back(k);
    }
    
    vector<vector<int> > partitionsForDistinctVal;
    splitElements(distinctValIndices.data(), distinctValIndices.size(), partitionsForDistinctVal, 2 * REGISTER_LIMIT);
    partitions.insert(partitions.end(), partitionsForDistinctVal.begin(), partitionsForDistinctVal.end());
  }
}

// The goal here is to emit code that will do something like
// 7*(v[1] + v[5] + v[9])
void UnfoldingWithDistinctValuesCodeEmitter::dumpPartialRow(vector<vector<int> > &partitions, unsigned partitionIndex, bool haveToAccumulateResult) {
  vector<int> &elements = partitions[partitionIndex];
  // Move vector elements to registers
  unsigned int iterIndex = 0;
  unsigned int vRegIndex = 0;
  
  // Check if this is the *first* partition for a particular distinct value.
  // If so, emit a move instruction, otherwise we continue to add on xmm0
  bool isFirstPartition = partitionIndex == 0;
  double val = csrMatrix->vals[elements[0]];
  double prevVal = isFirstPartition ? 0 : csrMatrix->vals[partitions[partitionIndex-1][0]];
  
  for (; iterIndex < (elements.size() + 1) / 2; iterIndex++, vRegIndex++) {
    int colIndex = csrMatrix->cols[elements[iterIndex]];
    if (iterIndex == 0 && !(isFirstPartition || val != prevVal)) {
      //  addsd "sizeof(double)*colIndex"(%rdi), %xmm"vRegIndex"
      emitADDSDrmInst(sizeof(double)*colIndex, X86::RDI, vRegIndex);
    } else {
      //  movsd "sizeof(double)*colIndex"(%rdi), %xmm"vRegIndex"
      emitMOVSDrmInst(sizeof(double)*colIndex, X86::RDI, vRegIndex);
    }
  }
  vRegIndex = 0;
  for (; iterIndex < elements.size(); iterIndex++, vRegIndex++) {
    int colIndex = csrMatrix->cols[elements[iterIndex]];
    //  addsd "sizeof(double)*colIndex"(%rdi), %xmm"vRegIndex"
    emitADDSDrmInst(sizeof(double)*colIndex, X86::RDI, vRegIndex);
  }
  
  vector<bool> signs;
  signs.resize((elements.size() + 1) / 2, true);
  dumpRegisterReduce(signs);
  
  // Check if this is the last partition for a particular distinct value.
  // If so, emit the multiplication with the accumulated sum of vector elements
  bool isLastPartition = partitionIndex == partitions.size() - 1;
  double nextVal = isLastPartition ? 0 : csrMatrix->vals[partitions[partitionIndex+1][0]];
  if (isLastPartition || val != nextVal) {
    unsigned long valIndex = sizeof(double) * valToIndexMap->at(val);
    if (val == -1.0) {
      if (haveToAccumulateResult) {
        //  subsd %xmm0, %xmm15
        emitRegInst(X86::SUBSDrr, 0, REGISTER_LIMIT);
      } else {
        emitFPNegation(0);
      }
    } else if (val == 1.0) {
      if (haveToAccumulateResult) {
        //  addsd %xmm0, %xmm15
        emitRegInst(X86::ADDSDrr, 0, REGISTER_LIMIT);
      }
    } else {
      // mulsd "valIndex"(%rdx), %xmm"0"
      emitMULSDrmInst((int)valIndex, X86::RDX, 0);
      if (haveToAccumulateResult) {
        //  addsd %xmm0, %xmm15
        emitRegInst(X86::ADDSDrr, 0, REGISTER_LIMIT);
      }
    }
  }
}

void UnfoldingCodeEmitter::dumpRowConclusion(int rowIndex, bool hadToSplitRow) {
  int registerThatStoresFinalResult = hadToSplitRow ? REGISTER_LIMIT : 0;
  
  unsigned int numRequiredShiftings = (sizeof(double)*rowIndex) / LIMIT_TO_DO_LEAQ;
  if (numRequiredShiftings - numWPointerShiftings == 1) {
    emitLEAQInst(X86::RSI, X86::RSI, LIMIT_TO_DO_LEAQ);
    numWPointerShiftings++;
  } else if (numRequiredShiftings - numWPointerShiftings > 1) {
    emitLEAQInst(X86::RSI, X86::RSI, (numRequiredShiftings - numWPointerShiftings)*LIMIT_TO_DO_LEAQ);
    numWPointerShiftings = numRequiredShiftings;
  }
  
  int rsiOffset = (sizeof(double)*rowIndex) % LIMIT_TO_DO_LEAQ;
  //  addsd "sizeof(double)*rowIndex"(%rsi), %xmm"0|REGISTER_LIMIT"
  emitADDSDrmInst(rsiOffset, X86::RSI, registerThatStoresFinalResult);
  //  movsd %xmm0, "sizeof(double)*rowIndex"(%rsi)
  emitMOVSDmrInst(registerThatStoresFinalResult, rsiOffset, X86::RSI);
}

void UnfoldingCodeEmitter::dumpRegisterReduce(vector<bool> &signs) {
  unsigned long numElements = signs.size();
  for(int inc=1; inc <= 8; inc *= 2) {
    for(int i=0; i+inc < numElements; i+=inc*2) {
      if (signs[i] == signs[i+inc]) {
        //  addsd %xmm(i+inc), %xmm(i)
        emitRegInst(X86::ADDSDrr, i+inc, i);
      } else {
        //  subsd %xmm(i+inc), %xmm(i)
        emitRegInst(X86::SUBSDrr, i+inc, i);
      }
    }
  }
}

