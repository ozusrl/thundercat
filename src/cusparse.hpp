#ifndef _THUNDERCAT_CUSPARSE
#define _THUNDERCAT_CUSPARSE


#include "method.h"

namespace thundercat {
  class Cusparse : public SpmvMethod {
   public:

      virtual void init(unsigned int numThreads) {};
      virtual ~Cusparse() {};
      virtual void preprocess(MMMatrix<VALUE_TYPE> &matrix);
      virtual void spmv(double* __restrict v, double* __restrict w);

      static const std::string name;
  };

}
#endif //_THUNDERCAT_CUSPARSE