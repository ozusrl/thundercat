#ifndef _THUNDERCAT_VIENNACL_HPP
#define _THUNDERCAT_VIENNACL_HPP

#include "method.h"
#include "viennacl/compressed_matrix.hpp"
#include "viennacl/vector.hpp"

namespace thundercat {
  class ViennaCL : public SpmvMethod {

  public:
    virtual void init(unsigned int numThreads);

    virtual void preprocess(MMMatrix<VALUE_TYPE>& matrix);

    virtual void spmv(double* __restrict v, double* __restrict w);

    static const std::string name;

  private:
      viennacl::compressed_matrix<VALUE_TYPE,16> A;
      int M;
      int N;
      int NNZ;

  };
}

#endif //_THUNDERCAT_VIENNACL_HPP
