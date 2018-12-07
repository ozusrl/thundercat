#include "cusp.hpp"
#include "spmvRegistry.h"
#include "profiler.h"

using namespace thundercat;

REGISTER_METHOD(Cusp, "cusp")

Cusp::~Cusp() {
  deleteCuspAdapter(adapter);
}

void Cusp::preprocess(thundercat::MMMatrix<double> &matrix) {
  csrMatrix = matrix.toCSR();
  adapter = newCuspAdapter();
  adapter->preprocess(csrMatrix->M, csrMatrix->N, csrMatrix->NZ,
                      csrMatrix->rowPtr, csrMatrix->colIndices, csrMatrix->values);
}

void Cusp::spmv(double *v, double *w) {
  Profiler::recordSpmvOverhead("Copy input matrix to Device", [&]() {
      adapter->setX(v);
  });

  adapter->spmv();

  Profiler::recordSpmvOverhead("Copy output matrix to Host", [&]() {
      adapter->getY(w);
  });
}