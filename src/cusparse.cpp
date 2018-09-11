#include "cusparse.hpp"
#include "spmvRegistry.h"


using namespace thundercat;
using namespace std;

const std::string Cusparse::name = "cusparse";
REGISTER_METHOD(Cusparse)


void Cusparse::init(unsigned int numThreads) {
  wrapper.init();
}

void Cusparse::preprocess(MMMatrix<VALUE_TYPE> &matrix) {

  csrMatrix = matrix.toCSR();
  wrapper.preprocess(csrMatrix->NZ, csrMatrix->M, csrMatrix->N, csrMatrix->rowPtr,
                     csrMatrix->colIndices, csrMatrix->values);

}


void Cusparse::spmv(double* __restrict v, double* __restrict w) {
  wrapper.spmv(v, w);
}
