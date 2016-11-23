#include "method.h"

#ifdef __linux__
#ifdef __x86_64__
#include <mkl.h>

int mkl_n = 0;

void mkl_multByM(double *v, double *w, int *rows, int *cols, double *vals) {
  double alpha = 1.0;
  double beta = 1.0;
  int *ptrb = rows;
  int *ptre = rows+1;
  char trans[] = "N";
  char matdescra[] = "G__C";
  mkl_dcsrmv(trans, &mkl_n, &mkl_n, &alpha, matdescra, vals, cols, ptrb, ptre, v, &beta, w);
}
#endif
#endif

using namespace spMVgen;
using namespace std;

MKL::MKL(Matrix *csrMatrix):
  SpMVMethod(csrMatrix) {
}

Matrix* MKL::getMatrixForGeneration() {
  return csrMatrix;
}

void MKL::dumpAssemblyText() {
  cerr << "MKL method does not generate code.\n";
  exit(1);  
}

void MKL::setNumOfThreads(unsigned int num) {
#ifdef __linux__
#ifdef __x86_64__
  mkl_set_num_threads_local(num);
#else
  cerr << "MKL is not supported on this platform.\n";
  exit(1);
#endif
#else
  cerr << "MKL is not supported on this platform.\n";
  exit(1);
#endif
}

std::vector<MultByMFun> MKL::getMultByMFunctions() {
#ifdef __linux__
#ifdef __x86_64__
  mkl_n = (int)csrMatrix->n;
  std::vector<MultByMFun> fptrs;
  fptrs.push_back(&mkl_multByM);
  return fptrs;
#else
  cerr << "MKL is not supported on this platform.\n";
  exit(1);
#endif
#else
  cerr << "MKL is not supported on this platform.\n";
  exit(1);
#endif
}

