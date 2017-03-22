#include "csrWithGOTOAnalyzer.h"
#include "profiler.h"
#include <iostream>
#include <fstream>
#include <sstream>

extern unsigned int NUM_OF_THREADS;

using namespace spMVgen;
using namespace std;

CSRWithGOTOAnalyzer::CSRWithGOTOAnalyzer(Matrix *csrMatrix): csrMatrix(csrMatrix) {
}

void CSRWithGOTOAnalyzer::analyzeMatrix() {
  maxRowLengths.resize(NUM_OF_THREADS);

  // Split the matrix
  const vector<MatrixStripeInfo> &stripeInfos = csrMatrix->getStripeInfos();

  #pragma omp parallel for
  for (int threadIndex = 0; threadIndex < NUM_OF_THREADS; ++threadIndex) {
    auto &stripeInfo = stripeInfos[threadIndex];
    int maxRowLength = 0;
    for (unsigned long rowIndex = stripeInfo.rowIndexBegin; rowIndex < stripeInfo.rowIndexEnd; ++rowIndex) {
      int rowStart = csrMatrix->rows[rowIndex];
      int rowEnd = csrMatrix->rows[rowIndex+1];
      int rowLength = rowEnd - rowStart;
      if (rowLength > maxRowLength) {
        maxRowLength = rowLength;
      }
    }
    maxRowLengths[threadIndex] = maxRowLength;
  }
}

vector<unsigned long> *CSRWithGOTOAnalyzer::getMaxRowLengths() {
  if (maxRowLengths.size() == 0) {
    analyzeMatrix();
  }
  return &maxRowLengths;
}

