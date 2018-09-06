#include "method.h"
#include "csr5avx2.hpp"
#include "spmvRegistry.h"
#include <mm_malloc.h>

using namespace thundercat;
using namespace std;

const std::string Csr5avx2::name = "csr5avx2";
REGISTER_METHOD(Csr5avx2)



void Csr5avx2::init(unsigned int numThreads){
  mNumThreads = numThreads;
}

Csr5avx2::~Csr5avx2() {
  underlying->destroy();
//  free(csrRowPtr);
//  free(csrColIdx);
//  free(csrVal);
}

void Csr5avx2::preprocess(MMMatrix<VALUE_TYPE>& matrix){
  csr = matrix.toCSR();
//  auto csr = matrix.toCSR();

//  csrRowPtr = (int *)_mm_malloc((csr->N + 1) * sizeof(int), ANONYMOUSLIB_X86_CACHELINE);
//  csrColIdx = (int *)_mm_malloc(csr->NZ * sizeof(int), ANONYMOUSLIB_X86_CACHELINE);
//  csrVal    = (VALUE_TYPE *)_mm_malloc(csr->NZ * sizeof(VALUE_TYPE), ANONYMOUSLIB_X86_CACHELINE);
//  memcpy(csrRowPtr, csr->rowPtr, (csr->N + 1) * sizeof(int));
//  memcpy(csrColIdx, csr->colIndices, (csr->NZ) * sizeof(int));
//  memcpy(csrVal, csr->values, (csr->NZ) * sizeof(int));

  underlying = make_unique<anonymouslibHandle<int, unsigned int, VALUE_TYPE>>(csr->M, csr->N);
  underlying->inputCSR(csr->NZ, csr->rowPtr, csr->colIndices, csr->values);
//  underlying->inputCSR(csr->NZ, csrRowPtr, csrColIdx, csrVal);
  underlying->setSigma(ANONYMOUSLIB_CSR5_SIGMA);
  underlying->asCSR5();
}

void Csr5avx2::spmv(double* __restrict v, double* __restrict w){
  if (isXSet == false) {
    underlying->setX(v);
    isXSet = true;
  }
  underlying->spmv(1.0 /*alpha */, w);
}
