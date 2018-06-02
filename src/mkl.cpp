#include "method.h"

using namespace thundercat;
using namespace std;

const std::string MKL::name = "mkl";

#ifdef MKL_EXISTS

#include <mkl.h>
#include "spmvRegistry.h"
REGISTER_METHOD(MKL)

void MKL::init(unsigned int numThreads) {
  CsrSpmvMethod::init(numThreads);
  
  mkl_set_num_threads_local(numThreads);
}

void MKL::spmv(double* __restrict v, double* __restrict w) {
  double alpha = 1.0;
  double beta = 1.0;
  int *ptrb = csrMatrix->rowPtr;
  int *ptre = csrMatrix->rowPtr + 1;
  char trans[] = "N";
  char matdescra[] = "G__C";
  int mkl_n = csrMatrix->N;
  int mkl_m = csrMatrix->M;
  
  mkl_dcsrmv(trans, &mkl_n, &mkl_m, &alpha, matdescra,
             csrMatrix->values, csrMatrix->colIndices, ptrb, ptre, v, &beta, w);
}

#else

void MKL::spmv(double* __restrict v, double* __restrict w) {
  cerr << "MKL is not supported on this platform.\n";
  exit(1);
}

void MKL::init(unsigned int numThreads) {
  cerr << "MKL is not supported on this platform.\n";
  exit(1);
}

#endif
