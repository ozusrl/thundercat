#include "method.h"
#include "csr5avx512.hpp"
#include "spmvRegistry.h"
#include <mm_malloc.h>

using namespace thundercat;
using namespace std;

const std::string Csr5avx512::name = "csr5avx512";
REGISTER_METHOD(Csr5avx512)



void Csr5avx512::init(unsigned int numThreads){
  mNumThreads = numThreads;
}

Csr5avx512::~Csr5avx512() {
  underlying->destroy();
  free(csrRowPtr);
  free(csrColIdx);
  free(csrVal);
}

void Csr5avx512::preprocess(MMMatrix<VALUE_TYPE>& matrix){
  auto csr = matrix.toCSR();

  csrRowPtr = (int *)_mm_malloc((csr->N + 1) * sizeof(int), ANONYMOUSLIB_X86_CACHELINE);
  csrColIdx = (int *)_mm_malloc(csr->NZ * sizeof(int), ANONYMOUSLIB_X86_CACHELINE);
  csrVal    = (VALUE_TYPE *)_mm_malloc(csr->NZ * sizeof(VALUE_TYPE), ANONYMOUSLIB_X86_CACHELINE);
  memcpy(csrRowPtr, csr->rowPtr, (csr->N + 1) * sizeof(int));
  memcpy(csrColIdx, csr->colIndices, (csr->NZ) * sizeof(int));
  memcpy(csrVal, csr->values, (csr->NZ) * sizeof(int));

  underlying = make_unique<anonymouslibHandle<int, unsigned int, VALUE_TYPE>>(csr->M, csr->N);
  underlying->inputCSR(csr->NZ, csrRowPtr, csrColIdx, csrVal);
  underlying->setSigma(ANONYMOUSLIB_CSR5_SIGMA);
  underlying->asCSR5();
}

void Csr5avx512::spmv(double* __restrict v, double* __restrict w){
  if (isXSet == false) {
    underlying->setX(v);
    isXSet = true;
  }
  underlying->spmv(1.0 /*alpha */, w);
}
