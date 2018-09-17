#include "viennaclAdapter.hpp"

using namespace thundercat;

ViennaCLAdapter::ViennaCLAdapter(int M,int N):A(M, N) {}

void ViennaCLAdapter::preprocess(std::vector< std::map<int,double> > &cpuMatrix) {
  viennacl::copy(cpuMatrix, A);
}

void ViennaCLAdapter::spmv(std::vector<double> &cpuX, std::vector<double> &cpuY) {
  viennacl::vector<double> x;
  viennacl::copy(cpuX, x);

  viennacl::vector<double> y= viennacl::linalg::prod(A, x);

  viennacl::copy(y, cpuY);
}

ViennaCLAdapter * thundercat::newViennaCLAdapter(int M, int N) {
  return new ViennaCLAdapter(M, N);
}
void thundercat::deleteViennaCLAdapter(ViennaCLAdapter* handle) {
  delete handle;
}