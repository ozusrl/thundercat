#include "mkl.h"
#include "mkl.hpp"
//#include "mkl_spblas.h"
#include "spmvRegistry.h"

using namespace thundercat;
using namespace std;

const std::string MKL::name = "mkl";

REGISTER_METHOD(MKL)

void MKL::init(unsigned int numThreads) {
  CsrSpmvMethod::init(numThreads);
  
  mkl_set_num_threads_local(numThreads);
}

void MKL::spmv(double* __restrict v, double* __restrict w) {
  double  alpha = 1.0;
  double beta = 0.0;
  int *ptrb = csrMatrix->rowPtr;
  int *ptre = csrMatrix->rowPtr + 1;
  char trans[] = "N";
  char matdescra[] = "G__C";
  int mkl_n = csrMatrix->N;
  int mkl_m = csrMatrix->M;
  
  mkl_dcsrmv(trans, &mkl_n, &mkl_m, &alpha, matdescra,
             csrMatrix->values, csrMatrix->colIndices, ptrb, ptre, v, &beta, w);
}