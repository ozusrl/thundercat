#include "viennacl.hpp"
#include "spmvRegistry.h"

using namespace thundercat;


const std::string ViennaCL::name = "viennacl";
REGISTER_METHOD(ViennaCL)

ViennaCL::~ViennaCL() {
  deleteViennaCLAdapter(adapter);
}

void ViennaCL::init(unsigned int numThreads) {
#ifdef VIENNACL_WITH_OPENMP
  omp_set_num_threads(numThreads);
#endif
}

void ViennaCL::preprocess(thundercat::MMMatrix<double> &matrix) {
  M = matrix.M;
  N = matrix.N;
  std::vector<std::map<int,double>> cpuMatrix(M);
  for(auto elem : matrix.getElements()) {
    cpuMatrix[elem.rowIndex][elem.colIndex] = elem.value;
  }

  adapter = newViennaCLAdapter(M, N);
  adapter->preprocess(cpuMatrix);
}

void ViennaCL::spmv(double *v, double *w) {
  std::vector<double> cpuX(v, v+M);
  std::vector<double> cpuY(w, w+N);

  adapter->spmv(cpuX, cpuY);
}