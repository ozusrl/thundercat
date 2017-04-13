#include "method.h"

using namespace spMVgen;
using namespace std;

int mkl_n = 0;

#ifdef MKL_EXISTS

#include <mkl.h>

void mkl_multByM(double *v, double *w, int *rows, int *cols, double *vals) {
  double alpha = 1.0;
  double beta = 1.0;
  int *ptrb = rows;
  int *ptre = rows+1;
  char trans[] = "N";
  char matdescra[] = "G__C";
  mkl_dcsrmv(trans, &mkl_n, &mkl_n, &alpha, matdescra, vals, cols, ptrb, ptre, v, &beta, w);
}

std::vector<MultByMFun> MKL::getMultByMFunctions() {
  std::vector<MultByMFun> fptrs;
  fptrs.push_back(&mkl_multByM);
  return fptrs;
}


void MKL::init(Matrix *csrMatrix, unsigned int numThreads) {
  this->csrMatrix = csrMatrix;
  this->matrix = csrMatrix;
  
  mkl_set_num_threads_local(numThreads);
}

#else

std::vector<MultByMFun> MKL::getMultByMFunctions() {
  std::vector<MultByMFun> fptrs;
  return fptrs;
}

void MKL::init(Matrix *csrMatrix, unsigned int numThreads) {
  cerr << "MKL is not supported on this platform.\n";
  exit(1);
}

#endif

void MKL::analyzeMatrix() {
  mkl_n = (int)csrMatrix->n;
}

void MKL::convertMatrix() {
  // do nothing
}
