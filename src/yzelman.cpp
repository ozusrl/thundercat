#include "method.h"
#include "yzelman.hpp"
#include "spmvRegistry.h"

using namespace thundercat;
using namespace std;

const std::string ZzCrs::name = "zzcrs";
REGISTER_METHOD(ZzCrs)

unique_ptr<ZZ_CRS<VALUE_TYPE>> ZzCrs::createUnderlying(std::vector<Triplet<VALUE_TYPE> > &input, int m, int n,
                                                      VALUE_TYPE zero) {
  return make_unique<ZZ_CRS<VALUE_TYPE>>(input, m, n, zero);
}


const std::string THilbert::name = "hilbert";
REGISTER_METHOD(THilbert)

unique_ptr<Hilbert<VALUE_TYPE>> THilbert::createUnderlying(std::vector<Triplet<VALUE_TYPE> > &input, int m, int n,
                                                       VALUE_TYPE zero) {
  return make_unique<Hilbert<VALUE_TYPE>>(input, m, n, zero);
}


const std::string Hts::name = "hts";
REGISTER_METHOD(Hts)

unique_ptr<HTS<VALUE_TYPE>> Hts::createUnderlying(std::vector<Triplet<VALUE_TYPE> > &input, int m, int n,
                                                           VALUE_TYPE zero) {
  return make_unique<HTS<VALUE_TYPE>>(input, m, n, zero);
}