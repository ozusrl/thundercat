#include "cusparse.hpp"
#include "spmvRegistry.h"
#include <cuda_runtime.h>


using namespace thundercat;
using namespace std;

const std::string Cusparse::name = "cusparse";
REGISTER_METHOD(Cusparse)


void Cusparse::preprocess(MMMatrix<VALUE_TYPE> &matrix) {
  cout<<"Cusparse::preprocess"<<endl;
}


void Cusparse::spmv(double* __restrict v, double* __restrict w) {
  cout << "Cusparse::spmv" << endl;
}
