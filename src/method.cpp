#include "method.h"
#include "profiler.h"
#include <iostream>
#include <sstream>
#include <stdio.h>

using namespace thundercat;
using namespace asmjit;
using namespace std;

extern bool DUMP_OBJECT;

CsrSpmvMethod::~CsrSpmvMethod() {
}

void CsrSpmvMethod::init(unsigned int numThreads) {
  this->numPartitions = numThreads;
}

void CsrSpmvMethod::preprocess(MMMatrix<VALUE_TYPE>& matrix) {
  Profiler::recordTime("ConversionToCSR", [&]() {
      csrMatrix = matrix.toCSR();
  });

  stripeInfos = csrMatrix->getStripeInfos(numPartitions);

  Profiler::recordTime("Analysis", [&]() {
      analyzeMatrix();
  });

  Profiler::recordTime("Conversion", [&]() {
      convertMatrix();
  });
}

void CsrSpmvMethod::analyzeMatrix() {
  // Do nothing.
}

void CsrSpmvMethod::convertMatrix() {
  // Do nothing.
}

void Specializer::init(unsigned int numThreads) {
  CsrSpmvMethod::init(numThreads);

  codeHolders.clear();
  for (int i = 0; i < numThreads; i++) {
    codeHolders.push_back(new CodeHolder);
    codeHolders[i]->init(rt.getCodeInfo());
  }
  
  functions.resize(numThreads);
}

void Specializer::preprocess(MMMatrix<VALUE_TYPE>& matrix) {
  CsrSpmvMethod::preprocess(matrix);
  Profiler::recordTime("EmitCode", [&]() {
      emitCode();
  });
}

void Specializer::emitCode() {
#pragma omp parallel for
  for (unsigned int i = 0; i < codeHolders.size(); i++) {
    emitMultByMFunction(i);
    codeHolders[i]->sync();
  }

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
}

std::vector<CodeHolder*> *Specializer::getCodeHolders() {
  return &codeHolders;
}

void Specializer::spmv(double* __restrict v, double* __restrict w) {
#pragma omp parallel for
  for (unsigned j = 0; j < functions.size(); j++) {
    functions[j](v, w, matrix->rowPtr, matrix->colIndices, matrix->values);
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

void LCSRAnalyzer::analyzeMatrix(CSRMatrix<VALUE_TYPE>& csrMatrix,
                                 std::vector<MatrixStripeInfo> stripeInfos,
                                 std::vector<NZtoRowMap> &rowByNZLists) {
  rowByNZLists.resize(stripeInfos.size());
  
#pragma omp parallel for
  for (int threadIndex = 0; threadIndex < stripeInfos.size(); ++threadIndex) {
    auto &stripeInfo = stripeInfos.at(threadIndex);
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
