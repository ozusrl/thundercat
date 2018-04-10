#include "profiler.h"
#include "method.h"
#include <iostream>
#include <sstream>
#include <stdio.h>

using namespace thundercat;
using namespace asmjit;
using namespace std;

extern bool DUMP_OBJECT;

SpMVMethod::~SpMVMethod() {
}

void SpMVMethod::init(unsigned int numThreads) {
  this->numPartitions = numThreads;
}

bool SpMVMethod::isSpecializer() {
  return false;
}

void SpMVMethod::emitCode() {
  // By default, do nothing
}

//MATRIX SpMVMethod::getMethodSpecificMatrix() {
//  return matrix;
//}

void SpMVMethod::processMatrix(MATRIX matrix) {

  csrMatrix = matrix.toCSR();

  Profiler::recordTime("getStripeInfos", [this]() {
    stripeInfos = csrMatrix->getStripeInfos(numPartitions);
  });
  Profiler::recordTime("analyzeMatrix", [this]() {
    analyzeMatrix();
  });
  Profiler::recordTime("convertMatrix", [this]() {
    convertMatrix();
  });
}

void SpMVMethod::analyzeMatrix() {
  // Do nothing.
}

void SpMVMethod::convertMatrix() {
  // Do nothing.
}

void Specializer::init(unsigned int numThreads) {
  SpMVMethod::init(numThreads);

  codeHolders.clear();
  for (int i = 0; i < numThreads; i++) {
    codeHolders.push_back(new CodeHolder);
    codeHolders[i]->init(rt.getCodeInfo());
  }
  
  functions.resize(numThreads);
}

bool Specializer::isSpecializer() {
  return true;
}

void Specializer::emitCode() {
#pragma omp parallel for
  for (unsigned int i = 0; i < codeHolders.size(); i++) {
    emitMultByMFunction(i);
    codeHolders[i]->sync();
  }
  
  Profiler::recordTime("setMultByMFunctions", [this]() {
    for (unsigned int i = 0; i < codeHolders.size(); i++) {
      MultByMFun fn;
      asmjit::Error err = rt.add(&fn, codeHolders[i]);
      if (err) {
        std::cerr << "Problem occurred while adding function " << i << " to Runtime.\n";
        std::cerr << err;
        exit(1);
      }
      functions[i] = fn;
    }
  });
}

std::vector<CodeHolder*> *Specializer::getCodeHolders() {
  return &codeHolders;
}

void Specializer::spmv(double* __restrict v, double* __restrict w) {
#pragma omp parallel for
  for (unsigned j = 0; j < functions.size(); j++) {
    functions[j](v, w, csrMatrix->rowPtr, csrMatrix->colIndices, csrMatrix->values);
  }
}

///
/// LCSR Analyzer
///
vector<int> *RowByNZ::getRowIndices() {
  return &rowIndices;
}

void RowByNZ::addRowIndex(int index) {
  rowIndices.push_back(index);
}

void LCSRAnalyzer::analyzeMatrix(CSRMatrix<MATRIX_ELEMENT>& csrMatrix,
                                 std::vector<MatrixStripeInfo> *stripeInfos,
                                 std::vector<NZtoRowMap> &rowByNZLists) {
  rowByNZLists.resize(stripeInfos->size());
  
#pragma omp parallel for
  for (int threadIndex = 0; threadIndex < stripeInfos->size(); ++threadIndex) {
    auto &stripeInfo = stripeInfos->at(threadIndex);
    for (unsigned long rowIndex = stripeInfo.rowIndexBegin; rowIndex < stripeInfo.rowIndexEnd; ++rowIndex) {
      int rowStart = csrMatrix.rowPtr[rowIndex];
      int rowEnd = csrMatrix.rowPtr[rowIndex+1];
      int rowLength = rowEnd - rowStart;
      if (rowLength > 0) {
        rowByNZLists[threadIndex][rowLength].addRowIndex(rowIndex);
      }
    }
  }
}
