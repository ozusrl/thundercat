#include "method.h"
#ifdef __linux__
#include "omp.h"
#endif

using namespace spMVgen;
using namespace std;

extern unsigned int NUM_OF_THREADS;

vector<MatrixStripeInfo> stripeInfos;

void plainCSR_multByM(double *v, double *w, int *rows, int *cols, double *vals) {
  
#ifdef __linux__
  int thread_id = omp_get_thread_num();
#else
  int thread_id = 0;
#endif
  for (int i = stripeInfos[thread_id].rowIndexBegin; i < stripeInfos[thread_id].rowIndexEnd; i++) {
    double ww = 0.0;
    for (int k = rows[i]; k < rows[i+1]; k++) {
      ww += vals[k] * v[cols[k]];
    }
    w[i] += ww;
  }
}

PlainCSR::PlainCSR(Matrix *csrMatrix):
  SpMVMethod(csrMatrix) {
}

Matrix* PlainCSR::getMatrixForGeneration() {
  stripeInfos = csrMatrix->getStripeInfos();
  
  return csrMatrix;
}

void PlainCSR::dumpAssemblyText() {
  cerr << "PlainCSR method does not generate code.\n";
  exit(1);  
}

std::vector<MultByMFun> PlainCSR::getMultByMFunctions() {
  std::vector<MultByMFun> fptrs;
  for (unsigned int i = 0; i < NUM_OF_THREADS; ++i) {
    fptrs.push_back(&plainCSR_multByM);
  }
  return fptrs;
}

