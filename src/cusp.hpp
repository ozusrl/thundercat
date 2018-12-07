#ifndef THUNDERCAT_CUSP_HPP
#define THUNDERCAT_CUSP_HPP

#include "method.h"


namespace thundercat {

  class CuspAdapter {
  public:
      void preprocess(int m, int n, int nnz, int * rowPtr, int * colIndx, double * values);
      void setX(double *);
      void getY(double *);
      void spmv();
  };

  CuspAdapter* newCuspAdapter();
  void deleteCuspAdapter(CuspAdapter* handle);

  class Cusp : public SpmvMethod {
  public:
    virtual void init(unsigned int numThreads){};
    virtual ~Cusp();
    virtual void preprocess(MMMatrix<VALUE_TYPE>& matrix);
    virtual void spmv(double* __restrict v, double* __restrict w);

  private:
    std::unique_ptr<CSRMatrix<VALUE_TYPE>> csrMatrix;
    CuspAdapter* adapter;

  };
}
#endif //THUNDERCAT_CUSP_HPP
