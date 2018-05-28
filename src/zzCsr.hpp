#ifndef _THUNDERCAT_ZZ_CRS_HPP
#include "method.h"
//#include "ZZ_CRS.hpp"

namespace thundercat {

  class ZzCrs : public SpmvMethod {

  public:
      static const std::string name;

      ZzCrs();

      virtual void init(unsigned int numThreads) {};

      virtual ~ZzCrs() {};

      virtual bool isSpecializer() {return false;};

      virtual void emitCode() {};

      virtual void preprocess(MMMatrix<VALUE_TYPE>& matrix);

      virtual void spmv(double* __restrict v, double* __restrict w);

  private:
      std::unique_ptr<ZZ_CRS<VALUE_TYPE>> yzelmanZzCrs;
  };
}

#endif