#include "cusp.hpp"
#include "spmvRegistry.h"

using namespace thundercat;

const std::string Cusp::name = "cusp";
REGISTER_METHOD(Cusp)

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
  adapter->spmv(v, w);
}