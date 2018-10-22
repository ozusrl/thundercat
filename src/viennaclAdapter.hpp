#ifndef _THUNDERCAT_VIENNACUDACLADAPTER_HPP
#define _THUNDERCAT_VIENNACUDACLADAPTER_HPP

#include <vector>
#include <map>
#include "viennacl/vector.hpp"
#include "viennacl/compressed_matrix.hpp"
#include "viennacl/linalg/prod.hpp"

namespace thundercat {
  class ViennaCLAdapter {

  public:
    ViennaCLAdapter(int M, int N);
    void preprocess(std::vector< std::map<int,double> > &cpuMatrix);
    void setX(std::vector<double> &cpuX);
    void getY(std::vector<double> &cpuY);
    void spmv();

  private:
    viennacl::compressed_matrix<double> A;
    viennacl::vector<double> x;
    viennacl::vector<double> y;

  };

  ViennaCLAdapter * newViennaCLAdapter(int M, int N);
  void deleteViennaCLAdapter(ViennaCLAdapter* handle);

}
#endif //_THUNDERCAT_VIENNACUDACLADAPTER_HPP
