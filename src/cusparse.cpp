#include "cusparse.hpp"
#include "spmvRegistry.h"
#include "profiler.h"

using namespace thundercat;
using namespace std;

REGISTER_METHOD(Cusparse, "cusparse")

Cusparse::~Cusparse() {
  deleteCusparseAdaptor(adaptor);
}
void Cusparse::init(unsigned int numThreads) {
  adaptor = newCusparseAdaptor();
  adaptor->init();
}

void Cusparse::preprocess(MMMatrix<VALUE_TYPE> &matrix) {

  csrMatrix = matrix.toCSR();
  adaptor->preprocess(csrMatrix->NZ, csrMatrix->M, csrMatrix->N, csrMatrix->rowPtr,
                     csrMatrix->colIndices, csrMatrix->values);
}

void Cusparse::spmv(double* __restrict v, double* __restrict w) {
  Profiler::recordSpmvOverhead("Copy input matrix to Device", [&]() {
      adaptor->setX(v);
  });

  adaptor->spmv();

  Profiler::recordSpmvOverhead("Copy output matrix to Host", [&]() {
    adaptor->getY(w);
  });
}
