#include "method.h"

using namespace thundercat;
using namespace std;

#ifdef MKL_EXISTS

#include <mkl.h>

void MKL::init(Matrix *csrMatrix, unsigned int numThreads) {
  SpMVMethod::init(csrMatrix, numThreads);
  
  mkl_set_num_threads_local(numThreads);
}

void MKL::spmv(double* __restrict v, double* __restrict w) {
  double alpha = 1.0;
  double beta = 1.0;
  int *ptrb = matrix->rows;
  int *ptre = matrix->rows + 1;
  char trans[] = "N";
  char matdescra[] = "G__C";
  int mkl_n = matrix->n;
  
  mkl_dcsrmv(trans, &mkl_n, &mkl_n, &alpha, matdescra,
             matrix->vals, matrix->cols, ptrb, ptre, v, &beta, w);
}

#else

void MKL::spmv(double* __restrict v, double* __restrict w) {
  cerr << "MKL is not supported on this platform.\n";
  exit(1);
}

void MKL::init(Matrix *csrMatrix, unsigned int numThreads) {
  cerr << "MKL is not supported on this platform.\n";
  exit(1);
}

#endif
