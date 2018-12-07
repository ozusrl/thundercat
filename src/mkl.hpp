#ifndef THUNDERCAT_MKL_HPP
#define THUNDERCAT_MKL_HPP

#include "method.h"
namespace thundercat {
  class MKL : public CsrSpmvMethod {
  public:
    virtual void init(unsigned int numThreads) final;

    virtual void spmv(double *__restrict v, double *__restrict w) final;
  };
}


#endif //THUNDERCAT_MKL_H
