#include "specializers.h"
#include "spmvRegistry.h"
#include <iostream>
#include <set>

using namespace thundercat;
using namespace std;
using namespace asmjit;
using namespace x86;

#define DISTINCT_VALUE_COUNT_LIMIT 5000
#define REGISTER_LIMIT 7
#define LIMIT_TO_DO_LEAQ 120

REGISTER_METHOD(Unfolding, "unfolding")

///
/// Analysis
///
void Unfolding::analyzeMatrix() {
  valToIndexMaps.resize(stripeInfos->size());
  distinctValueLists.resize(stripeInfos->size());
  
  bool earlyExit = false;
  
#pragma omp parallel for
  for (int threadIndex = 0; threadIndex < stripeInfos->size(); ++threadIndex) {
    auto &stripeInfo = stripeInfos->at(threadIndex);
    for (unsigned long rowIndex = stripeInfo.rowIndexBegin; !earlyExit && rowIndex < stripeInfo.rowIndexEnd; ++rowIndex) {
      int rowStart = csrMatrix->rowPtr[rowIndex];
      int rowEnd = csrMatrix->rowPtr[rowIndex+1];
      
      for (int k = rowStart; k < rowEnd; ++k) {
        double val = csrMatrix->values[k];
        if (valToIndexMaps[threadIndex].count(val) == 0) {
          unsigned long valueIndex = distinctValueLists[threadIndex].size();
          valToIndexMaps[threadIndex][val] = valueIndex;
          distinctValueLists[threadIndex].push_back(val);
        }
      }
      
      if (valToIndexMaps[threadIndex].size() >= DISTINCT_VALUE_COUNT_LIMIT) {
        earlyExit = true;
      }
    }
  }
}

bool Unfolding::hasFewDistinctValues() {
  for (auto &map : valToIndexMaps) {
    if (map.size() >= DISTINCT_VALUE_COUNT_LIMIT) {
      return false;
    }
  }
  return true;
}

///
/// Unfolding
///
void Unfolding::convertMatrix() {
  // The hack below is used when negating
  // FP values
  unsigned long *cols = new unsigned long[2];   // Yes, this is unsigned long!
  cols[0] = 0x8000000000000000;
  cols[1] = 0x0000000000000000;
  
  double *values = NULL;
  unsigned long numVals = 0;
  if (hasFewDistinctValues()) {
    for (auto &partitionValues : distinctValueLists) {
      numVals += partitionValues.size();
    }
    values = new double[numVals];
    double *valPtr = values;
    for (auto &partitionValues : distinctValueLists) {
      memcpy(valPtr, partitionValues.data(), partitionValues.size() * sizeof(double));
      valPtr += partitionValues.size();
    }
  } else {
    values = new VALUE_TYPE[csrMatrix->NZ];
    memcpy(values, csrMatrix->values, sizeof(VALUE_TYPE) * csrMatrix->NZ);
  }
  
  matrix = std::make_unique<CSRMatrix<VALUE_TYPE>>((int  *)NULL, (int*)cols, values, csrMatrix->N, csrMatrix->M, csrMatrix->NZ);
}

///
/// UnfoldingCodeEmitter:
/// Helper class to avoid having to pass several parameters
///
class UnfoldingCodeEmitter {
public:
  UnfoldingCodeEmitter(X86Assembler *assembler,
                       CSRMatrix<VALUE_TYPE> *csrMatrix,
                       MatrixStripeInfo *stripeInfo) {
    this->assembler = assembler;
    this->csrMatrix = csrMatrix;
    this->stripeInfo = stripeInfo;
    this->numWPointerShiftings = 0;
  }

  virtual void emit();
  
protected:
  X86Assembler *assembler;
  CSRMatrix<VALUE_TYPE> *csrMatrix;
  MatrixStripeInfo *stripeInfo;
  unsigned long numWPointerShiftings;
  
  void emitRow(int rowIndex);
  virtual void emitPartialRow(vector<vector<int> > &partitions, unsigned partitionIndex, bool haveToAccumulateResult);
  void emitRowConclusion(int rowIndex, bool hadToSplitRow);
  void emitRegisterReduce(vector<bool> &signs);

  virtual void partitionRowElements(int rowIndex, vector<vector<int> > &partitions);

  virtual void emitHeader();
  virtual void emitValsPointerAdjustment();
  virtual void emitFooter();
  
private:
  unsigned long getValIndexAndShiftValsPointer(int elementIndex);
};

//--------------------------------------------------------------------------------------
class UnfoldingWithDistinctValuesCodeEmitter : public UnfoldingCodeEmitter {
public:
  UnfoldingWithDistinctValuesCodeEmitter(X86Assembler *assembler,
                                         CSRMatrix<VALUE_TYPE> *csrMatrix,
                                         MatrixStripeInfo *stripeInfo,
                                         map<double, unsigned long> *valToIndexMap,
                                         unsigned long baseValsIndex) :
  UnfoldingCodeEmitter(assembler, csrMatrix, stripeInfo) {
    this->valToIndexMap = valToIndexMap;
    this->baseValsIndex = baseValsIndex;
  }

private:
  std::map<double, unsigned long> *valToIndexMap;
  unsigned long baseValsIndex;
  
protected:
  virtual void emitValsPointerAdjustment();
  virtual void emitPartialRow(vector<vector<int> > &partitions, unsigned partitionIndex, bool haveToAccumulateResult);
  virtual void partitionRowElements(int rowIndex, vector<vector<int> > &partitions);
};


///
///
///
void Unfolding::emitMultByMFunction(unsigned int index) {
  X86Assembler assembler(codeHolders[index]);
  auto &stripeInfo = stripeInfos->at(index);
  if (!hasFewDistinctValues()) {
    UnfoldingCodeEmitter emitter(&assembler, csrMatrix.get(), &stripeInfo);
    emitter.emit();
  } else {
    unsigned long baseValsIndex = 0;
    for (unsigned i = 0; i < index; i++) {
      baseValsIndex += valToIndexMaps[i].size();
    }

    UnfoldingWithDistinctValuesCodeEmitter emitter(&assembler,
                                                   csrMatrix.get(),
                                                   &stripeInfo,
                                                   &(valToIndexMaps[index]),
                                                   baseValsIndex);
    emitter.emit();
  }
}

void UnfoldingCodeEmitter::emit() {
  emitHeader();

  for (int row = stripeInfo->rowIndexBegin; row < stripeInfo->rowIndexEnd; ++row) {
    emitRow(row);
  }
  
  emitFooter();
}

void UnfoldingCodeEmitter::emitHeader() {
  // rows is in %rdx, cols is in %rcx, vals is in %r8
  assembler->push(rdx);
  assembler->push(rsi);
  
  emitValsPointerAdjustment();

  int firstRow = stripeInfo->rowIndexBegin;
  assembler->lea(rsi, ptr(rsi, (firstRow * sizeof(double)) / LIMIT_TO_DO_LEAQ * LIMIT_TO_DO_LEAQ));
  numWPointerShiftings = (firstRow * sizeof(double)) / LIMIT_TO_DO_LEAQ;
}

void UnfoldingCodeEmitter::emitValsPointerAdjustment() {
  // Create a copy of vals pointer (R8) in RDX.
  //  leaq "offset"(%r8), %rdx
  assembler->lea(rdx, ptr(r8, (stripeInfo->valIndexBegin * sizeof(double)) / LIMIT_TO_DO_LEAQ * LIMIT_TO_DO_LEAQ));
}

void UnfoldingWithDistinctValuesCodeEmitter::emitValsPointerAdjustment() {
  // Create a copy of vals pointer (R8) in RDX.
  //  leaq "offset"(%r8), %rdx
  assembler->lea(rdx, ptr(r8, baseValsIndex * sizeof(double)));
}

void UnfoldingCodeEmitter::emitFooter() {
  assembler->pop(rsi);
  assembler->pop(rdx);
  assembler->ret();
}

void UnfoldingCodeEmitter::emitRow(int rowIndex) {
  int rowLength = csrMatrix->rowPtr[rowIndex+1] - csrMatrix->rowPtr[rowIndex];
  if (rowLength == 0) return;
  
  vector<vector<int> > partitions;
  partitionRowElements(rowIndex, partitions);

  bool haveToUseAccumulator = partitions.size() > 1;
  
  if (haveToUseAccumulator) { // reset the accumulation register
    //  xorps %xmm15, %xmm15
    assembler->xorps(xmm(REGISTER_LIMIT), xmm(REGISTER_LIMIT));
  }
  // First, do partial sums as much as needed
  for (unsigned partitionIndex = 0; partitionIndex < partitions.size(); partitionIndex++) {
    emitPartialRow(partitions, partitionIndex, haveToUseAccumulator);
  }
  
  // Finally, write the result to the output vector
  emitRowConclusion(rowIndex, haveToUseAccumulator);
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
  splitElements(pair<int, int>(csrMatrix->rowPtr[rowIndex], csrMatrix->rowPtr[rowIndex+1]), partitions, REGISTER_LIMIT);
}

void UnfoldingCodeEmitter::emitPartialRow(vector<vector<int> > &partitions,
                                          unsigned partitionIndex,
                                          bool haveToAccumulateResult) {
  vector<int> &elements = partitions[partitionIndex];
  // Move vector elements to registers
  unsigned int vRegIndex = 0;
  for (auto eltIndex : elements) {
    int colIndex = csrMatrix->colIndices[eltIndex];
    //  movsd "sizeof(double)*colIndex"(%rdi), %xmm"vRegIndex"
    assembler->movsd(xmm(vRegIndex), ptr(rdi, sizeof(double) * colIndex));
    vRegIndex++;
  }
  
  // Multiply matrix values with vector elements
  vRegIndex = 0;
  for (auto eltIndex : elements) {
    double value = csrMatrix->values[eltIndex];
    unsigned long valIndex = getValIndexAndShiftValsPointer(eltIndex);
    if (value == -1.0) {
      // do nothing
    } else if (value == 1.0) {
      // do nothing
    } else {
      // mulsd "valIndex"(%rdx), %xmm"vRegIndex"
      assembler->mulsd(xmm(vRegIndex), ptr(rdx, valIndex));
    }
    vRegIndex++;
  }
  
  vector<bool> signs;
  for (auto eltIndex: elements) {
      signs.push_back(csrMatrix->values[eltIndex] != -1);
  }
  emitRegisterReduce(signs);
  
  if (haveToAccumulateResult) {
    if (!signs[0]) {
      //  subsd %xmm0, %xmm15
      assembler->subsd(xmm(REGISTER_LIMIT), xmm0);
    } else {
      //  addsd %xmm0, %xmm15
      assembler->addsd(xmm(REGISTER_LIMIT), xmm0);
    }
  } else if (!signs[0]) {
    // negate the register
    // xorpd (%rcx), %xmm0
    assembler->xorpd(xmm0, ptr(rcx));
  }
}

unsigned long UnfoldingCodeEmitter::getValIndexAndShiftValsPointer(int elementIndex) {
  unsigned long valIndex = sizeof(double) * elementIndex;
  valIndex -= ((stripeInfo->valIndexBegin * sizeof(double)) / LIMIT_TO_DO_LEAQ * LIMIT_TO_DO_LEAQ);
  if (valIndex % LIMIT_TO_DO_LEAQ == 0 && valIndex != 0) {
    //  leaq "LIMIT_TO_DO_LEAQ"(%rdx), %rdx
    assembler->lea(rdx, ptr(rdx, LIMIT_TO_DO_LEAQ));
  }
  valIndex = valIndex % LIMIT_TO_DO_LEAQ;
  return valIndex;
}

void UnfoldingWithDistinctValuesCodeEmitter::partitionRowElements(int rowIndex, vector<vector<int> > &partitions) {
  // Find distinct values of this row
  set<double> distinctValues;
  for (int k = csrMatrix->rowPtr[rowIndex]; k < csrMatrix->rowPtr[rowIndex+1]; ++k) {
    distinctValues.insert(csrMatrix->values[k]);
  }
  
  // For each distinct value, group the MMElements that have that distinct val
  // and partition each group.
  for (double distinctVal : distinctValues) {
    vector<int> distinctValIndices;
    for (int k = csrMatrix->rowPtr[rowIndex]; k < csrMatrix->rowPtr[rowIndex+1]; ++k) {
      if (csrMatrix->values[k] == distinctVal)
        distinctValIndices.push_back(k);
    }
    
    vector<vector<int> > partitionsForDistinctVal;
    splitElements(distinctValIndices.data(), distinctValIndices.size(), partitionsForDistinctVal, 2 * REGISTER_LIMIT);
    partitions.insert(partitions.end(), partitionsForDistinctVal.begin(), partitionsForDistinctVal.end());
  }
}

// The goal here is to emit code that will do something like
// 7*(v[1] + v[5] + v[9])
void UnfoldingWithDistinctValuesCodeEmitter::emitPartialRow(vector<vector<int> > &partitions, unsigned partitionIndex, bool haveToAccumulateResult) {
  vector<int> &elements = partitions[partitionIndex];
  // Move vector elements to registers
  unsigned int iterIndex = 0;
  unsigned int vRegIndex = 0;
  
  // Check if this is the *first* partition for a particular distinct value.
  // If so, emit a move instruction, otherwise we continue to add on xmm0
  bool isFirstPartition = partitionIndex == 0;
  double val = csrMatrix->values[elements[0]];
  double prevVal = isFirstPartition ? 0 : csrMatrix->values[partitions[partitionIndex-1][0]];
  
  for (; iterIndex < (elements.size() + 1) / 2; iterIndex++, vRegIndex++) {
    int colIndex = csrMatrix->colIndices[elements[iterIndex]];
    if (iterIndex == 0 && !(isFirstPartition || val != prevVal)) {
      //  addsd "sizeof(double)*colIndex"(%rdi), %xmm"vRegIndex"
      assembler->addsd(xmm(vRegIndex), ptr(rdi, sizeof(double) * colIndex));
    } else {
      //  movsd "sizeof(double)*colIndex"(%rdi), %xmm"vRegIndex"
      assembler->movsd(xmm(vRegIndex), ptr(rdi, sizeof(double) * colIndex));
    }
  }
  vRegIndex = 0;
  for (; iterIndex < elements.size(); iterIndex++, vRegIndex++) {
    int colIndex = csrMatrix->colIndices[elements[iterIndex]];
    //  addsd "sizeof(double)*colIndex"(%rdi), %xmm"vRegIndex"
    assembler->addsd(xmm(vRegIndex), ptr(rdi, sizeof(double) * colIndex));
  }
  
  vector<bool> signs;
  signs.resize((elements.size() + 1) / 2, true);
  emitRegisterReduce(signs);
  
  // Check if this is the last partition for a particular distinct value.
  // If so, emit the multiplication with the accumulated sum of vector elements
  bool isLastPartition = partitionIndex == partitions.size() - 1;
  double nextVal = isLastPartition ? 0 : csrMatrix->values[partitions[partitionIndex+1][0]];
  if (isLastPartition || val != nextVal) {
    unsigned long valIndex = sizeof(double) * valToIndexMap->at(val);
    if (val == -1.0) {
      if (haveToAccumulateResult) {
        //  subsd %xmm0, %xmm15
        assembler->subsd(xmm(REGISTER_LIMIT), xmm0);
      } else {
        assembler->xorpd(xmm0, ptr(rcx)); // FP negation
      }
    } else if (val == 1.0) {
      if (haveToAccumulateResult) {
        //  addsd %xmm0, %xmm15
        assembler->addsd(xmm(REGISTER_LIMIT), xmm0);
      }
    } else {
      // mulsd "valIndex"(%rdx), %xmm"0"
      assembler->mulsd(xmm0, ptr(rdx, valIndex));
      if (haveToAccumulateResult) {
        //  addsd %xmm0, %xmm15
        assembler->addsd(xmm(REGISTER_LIMIT), xmm0);
      }
    }
  }
}

void UnfoldingCodeEmitter::emitRowConclusion(int rowIndex, bool hadToSplitRow) {
  int registerThatStoresFinalResult = hadToSplitRow ? REGISTER_LIMIT : 0;
  
  unsigned int numRequiredShiftings = (sizeof(double)*rowIndex) / LIMIT_TO_DO_LEAQ;
  if (numRequiredShiftings - numWPointerShiftings == 1) {
    assembler->lea(rsi, ptr(rsi, LIMIT_TO_DO_LEAQ));
    numWPointerShiftings++;
  } else if (numRequiredShiftings - numWPointerShiftings > 1) {
    int offset = (numRequiredShiftings - numWPointerShiftings) * LIMIT_TO_DO_LEAQ;
    assembler->lea(rsi, ptr(rsi, offset));
    numWPointerShiftings = numRequiredShiftings;
  }
  
  int rsiOffset = (sizeof(double)*rowIndex) % LIMIT_TO_DO_LEAQ;
  //  addsd "sizeof(double)*rowIndex"(%rsi), %xmm"0|REGISTER_LIMIT"
  assembler->addsd(xmm(registerThatStoresFinalResult), ptr(rsi, rsiOffset));
  //  movsd %xmm0, "sizeof(double)*rowIndex"(%rsi)
  assembler->movsd(ptr(rsi, rsiOffset), xmm(registerThatStoresFinalResult));
}

void UnfoldingCodeEmitter::emitRegisterReduce(vector<bool> &signs) {
  unsigned long numElements = signs.size();
  for(int inc=1; inc <= 8; inc *= 2) {
    for(int i=0; i+inc < numElements; i+=inc*2) {
      if (signs[i] == signs[i+inc]) {
        //  addsd %xmm(i+inc), %xmm(i)
        assembler->addsd(xmm(i), xmm(i + inc));
      } else {
        //  subsd %xmm(i+inc), %xmm(i)
        assembler->subsd(xmm(i), xmm(i + inc));
      }
    }
  }
}

