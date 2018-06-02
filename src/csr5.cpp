#include "method.h"
#include "csr5.hpp"
#include "spmvRegistry.h"

using namespace thundercat;
using namespace std;

const std::string Csr5::name = "csr5";
REGISTER_METHOD(Csr5)



void Csr5::init(unsigned int numThreads){
   mNumThreads = numThreads;
}

Csr5::~Csr5() {
  underlying->destroy();
}

void Csr5::preprocess(MMMatrix<VALUE_TYPE>& matrix){
  auto csr = matrix.toCSR();
   underlying = make_unique<anonymouslibHandle<int, unsigned int, VALUE_TYPE>>(csr->M, csr->N);
  underlying->inputCSR(csr->NZ, csr->rowPtr, csr->colIndices, csr->values);
  underlying->setSigma(ANONYMOUSLIB_CSR5_SIGMA);
  underlying->asCSR5();
}

void Csr5::spmv(double* __restrict v, double* __restrict w){
  underlying->setX(v);
  underlying->spmv(1.0 /*alpha */, w);
}
