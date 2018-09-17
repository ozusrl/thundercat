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
    void spmv(std::vector<double> &cpuX, std::vector<double> &cpuY);

  private:
    viennacl::compressed_matrix<double,16> A;

  };

  ViennaCLAdapter * newViennaCLAdapter(int M, int N);
  void deleteViennaCLAdapter(ViennaCLAdapter* handle);

}
#endif //_THUNDERCAT_VIENNACUDACLADAPTER_HPP
