#include "cusparse.hpp"
#include "spmvRegistry.h"


using namespace thundercat;
using namespace std;

const std::string Cusparse::name = "cusparse";
REGISTER_METHOD(Cusparse)

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
  adaptor->spmv(v, w);
}
