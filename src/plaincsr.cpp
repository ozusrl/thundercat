#include "method.h"
#ifdef OMP_EXISTS
#include "omp.h"
#endif

using namespace thundercat;
using namespace std;

static vector<MatrixStripeInfo> *stripes;
static unsigned int numThreads = 1;

void plainCSR_multByM(double *v, double *w, int *rows, int *cols, double *vals) {
#pragma omp parallel for
  for (unsigned int threadIndex = 0; threadIndex < numThreads; threadIndex++) {
    int rowIndexBegin = stripes->at(threadIndex).rowIndexBegin;
    int rowIndexEnd = stripes->at(threadIndex).rowIndexEnd;
    for (int i = rowIndexBegin; i < rowIndexEnd; i++) {
      double ww = 0.0;
      for (int k = rows[i]; k < rows[i+1]; k++) {
        ww += vals[k] * v[cols[k]];
      }
      w[i] += ww;
    }
  }
}

void PlainCSR::analyzeMatrix() {
  stripes = this->stripeInfos;
  numThreads = this->numPartitions;
}

void PlainCSR::convertMatrix() {
  // Do nothing. We use the CSR format.
}

std::vector<MultByMFun> PlainCSR::getMultByMFunctions() {
  std::vector<MultByMFun> fptrs;
  fptrs.push_back(&plainCSR_multByM);
  return fptrs;
}

