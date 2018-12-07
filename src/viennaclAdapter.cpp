#include "viennaclAdapter.hpp"

using namespace thundercat;

ViennaCLAdapter::ViennaCLAdapter(int M,int N):A(M, N), x(M), y(N) {}

void ViennaCLAdapter::preprocess(std::vector< std::map<int,double> > &cpuMatrix) {
  viennacl::copy(cpuMatrix, A);
}

void ViennaCLAdapter::setX(std::vector<double> &cpuX) {
  viennacl::copy(cpuX, x);
}

void ViennaCLAdapter::getY(std::vector<double> &cpuY) {
  viennacl::copy(y, cpuY);
}

void ViennaCLAdapter::spmv() {
  y  += viennacl::linalg::prod(A, x);
  viennacl::backend::finish();
}

ViennaCLAdapter * thundercat::newViennaCLAdapter(int M, int N) {
  return new ViennaCLAdapter(M, N);
}
void thundercat::deleteViennaCLAdapter(ViennaCLAdapter* handle) {
  delete handle;
}