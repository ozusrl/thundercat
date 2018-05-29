#include "ZZ_CRS.hpp"
#include "method.h"
#include "yzelman.hpp"
#include "spmvRegistry.h"

using namespace thundercat;
using namespace std;

const std::string ZzCrs2::name = "zzcrs2";
REGISTER_METHOD(ZzCrs2)

unique_ptr<ZZ_CRS<VALUE_TYPE>> ZzCrs2::createUnderlying(std::vector<Triplet<VALUE_TYPE> > &input, int m, int n,
                                                      VALUE_TYPE zero) {
  return make_unique<ZZ_CRS<VALUE_TYPE>>(input, m, n, zero);
}


const std::string THilbert::name = "hilbert";
REGISTER_METHOD(THilbert)

unique_ptr<Hilbert<VALUE_TYPE>> THilbert::createUnderlying(std::vector<Triplet<VALUE_TYPE> > &input, int m, int n,
                                                       VALUE_TYPE zero) {
  return make_unique<Hilbert<VALUE_TYPE>>(input, m, n, zero);
}