#include "method.h"
#include "profiler.h"
#include <iostream>
#include <sstream>
#include <stdio.h>

using namespace thundercat;
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