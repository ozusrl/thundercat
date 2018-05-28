#include "ZZ_CRS.hpp"
#include "method.h"
#include "zzCsr.hpp"
#include "spmvRegistry.h"

using namespace thundercat;
using namespace std;

const std::string ZzCrs::name = "zzcrs";
REGISTER_METHOD(ZzCrs)

ZzCrs::ZzCrs() {}

void ZzCrs::processMatrix(std::unique_ptr<MMMatrix<VALUE_TYPE>> matrix) {

  vector<Triplet<VALUE_TYPE>> triplets;

  auto elems = matrix->getElements();


  for (auto elem = elems.begin() ; elem != elems.end(); ++elem)
      triplets.push_back(Triplet<VALUE_TYPE>(elem->rowIndex, elem->colIndex, elem->value));


  yzelmanZzCrs = make_unique<ZZ_CRS<VALUE_TYPE>>(triplets, matrix->M, matrix->N, 0);
}

void ZzCrs::spmv(double* __restrict v, double* __restrict w) {
  yzelmanZzCrs->zax(v,w);
}




