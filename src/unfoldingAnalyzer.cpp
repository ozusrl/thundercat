#include "unfoldingAnalyzer.h"
#include "profiler.h"
#include <iostream>
#include <fstream>
#include <sstream>

extern unsigned int NUM_OF_THREADS;

using namespace spMVgen;
using namespace std;

///
/// UnfoldingAnalyzer
///
UnfoldingAnalyzer::UnfoldingAnalyzer(Matrix *csrMatrix): csrMatrix(csrMatrix){
  
}

void UnfoldingAnalyzer::analyzeMatrix() {
  valToIndexMaps.resize(NUM_OF_THREADS);
  distinctValueLists.resize(NUM_OF_THREADS);
  
  // Split the matrix
  const vector<MatrixStripeInfo> &stripeInfos = csrMatrix->getStripeInfos();
  bool earlyExit = false;
  
#pragma omp parallel for
  for (int threadIndex = 0; threadIndex < NUM_OF_THREADS; ++threadIndex) {
    auto &stripeInfo = stripeInfos[threadIndex];
    for (unsigned long rowIndex = stripeInfo.rowIndexBegin; !earlyExit && rowIndex < stripeInfo.rowIndexEnd; ++rowIndex) {
      int rowStart = csrMatrix->rows[rowIndex];
      int rowEnd = csrMatrix->rows[rowIndex+1];
      
      for (int k = rowStart; k < rowEnd; ++k) {
        double val = csrMatrix->vals[k];
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

vector<map<double, unsigned long> > *UnfoldingAnalyzer::getValToIndexMaps() {
  if (distinctValueLists.size() == 0 || valToIndexMaps.size() == 0) {
    analyzeMatrix();
  }
  return &valToIndexMaps;
}

vector<vector<double> > *UnfoldingAnalyzer::getValues() {
  if (distinctValueLists.size() == 0 || valToIndexMaps.size() == 0) {
    analyzeMatrix();
  }
  return &distinctValueLists;
}

bool UnfoldingAnalyzer::hasFewDistinctValues() {
  for (auto &map : valToIndexMaps) {
    if (map.size() >= DISTINCT_VALUE_COUNT_LIMIT) {
      return false;
    }
  }
  return true;
}



