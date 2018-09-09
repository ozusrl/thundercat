#include "cusparse.hpp"
#include "spmvRegistry.h"


using namespace thundercat;
using namespace std;

const std::string Cusparse::name = "cusparse";
REGISTER_METHOD(Cusparse)

void cusparse_spmv();

void Cusparse::preprocess(MMMatrix<VALUE_TYPE> &matrix) {
  cout<<"Cusparse::preprocess"<<endl;
}


void Cusparse::spmv(double* __restrict v, double* __restrict w) {
  cout << "Cusparse::spmv" << endl;
  cusparse_spmv();
}
