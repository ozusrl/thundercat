#include "csrByNZAnalyzer.h"
#include "profiler.h"
#include <iostream>
#include <fstream>
#include <sstream>

extern unsigned int NUM_OF_THREADS;

using namespace spMVgen;
using namespace std;

vector<int> *RowByNZ::getRowIndices() {
  return &rowIndices;
}

void RowByNZ::addRowIndex(int index) {
  rowIndices.push_back(index);
}

///
/// CSRbyNZAnalyzer
///
CSRbyNZAnalyzer::CSRbyNZAnalyzer(Matrix *csrMatrix): csrMatrix(csrMatrix) {
}

void CSRbyNZAnalyzer::analyzeMatrix() {
  rowByNZLists.resize(NUM_OF_THREADS);
  
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
        rowByNZLists[threadIndex][rowLength].addRowIndex(rowIndex);
      }
    }
  }
}

vector<NZtoRowMap> *CSRbyNZAnalyzer::getRowByNZLists() {
  if (rowByNZLists.size() == 0) {
    analyzeMatrix();
  }

  return &rowByNZLists;
}

