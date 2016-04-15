#include "stencilAnalyzer.h"
#include "profiler.h"
#include <iostream>
#include <fstream>
#include <sstream>

extern unsigned int NUM_OF_THREADS;

using namespace spMVgen;
using namespace std;


StencilAnalyzer::StencilAnalyzer(Matrix *csrMatrix): csrMatrix(csrMatrix) {
}

void StencilAnalyzer::analyzeMatrix() {
  stencilLists.resize(NUM_OF_THREADS);
  
  // Split the matrix
  const vector<MatrixStripeInfo> &stripeInfos = csrMatrix->getStripeInfos();
  
#pragma omp parallel for
  for (int threadIndex = 0; threadIndex < NUM_OF_THREADS; ++threadIndex) {
    auto &stripeInfo = stripeInfos[threadIndex];
    for (unsigned long rowIndex = stripeInfo.rowIndexBegin; rowIndex < stripeInfo.rowIndexEnd; ++rowIndex) {
      int rowStart = csrMatrix->rows[rowIndex];
      int rowEnd = csrMatrix->rows[rowIndex+1];
      int rowLength = rowEnd - rowStart;
      
      if (rowLength > 0) {
        StencilPattern pattern;
        for (int k = rowStart; k < rowEnd; ++k) {
          pattern.push_back(csrMatrix->cols[k] - (int)rowIndex);
        }
        stencilLists[threadIndex][pattern].push_back((int)rowIndex);
      }
    }
  }
}

vector<StencilToRowMap> *StencilAnalyzer::getStencilLists() {
  if (stencilLists.size() == 0) {
    analyzeMatrix();
  }
  return &stencilLists;
}

