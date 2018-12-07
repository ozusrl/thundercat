#ifndef _THUNDERCAT_VIENNACL_HPP
#define _THUNDERCAT_VIENNACL_HPP

#include "method.h"
#include <vector>
#include <map>

namespace thundercat {

  // Forward declare ViennaCL Adapter class. Implemantions of this class will
  // ViennaCL backend specific.
  class ViennaCLAdapter {
  public:
      ViennaCLAdapter();
      void preprocess(std::vector<std::map<int,double> > &cpuMatrix);
      void setX(std::vector<double> &cpuX);
      void getY(std::vector<double> &cpuY);
      void spmv();
  };

  ViennaCLAdapter * newViennaCLAdapter(int M, int N);
  void deleteViennaCLAdapter(ViennaCLAdapter* handle);

  class ViennaCL : public SpmvMethod {

  public:

    ~ViennaCL();
    virtual void init(unsigned int numThreads);

    virtual void preprocess(MMMatrix<VALUE_TYPE>& matrix);

    virtual void spmv(double* __restrict v, double* __restrict w);

  private:
      ViennaCLAdapter* adapter;
      int M;
      int N;
  };
}

#endif //_THUNDERCAT_VIENNACL_HPP
