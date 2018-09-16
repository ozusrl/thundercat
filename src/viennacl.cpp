#include "viennacl.hpp"
#include "viennacl/linalg/prod.hpp"
#include "viennacl/vector.hpp"
#include "viennacl/linalg/prod.hpp"
#include "spmvRegistry.h"

using namespace thundercat;


const std::string ViennaCL::name = "viennacl";
REGISTER_METHOD(ViennaCL)

void ViennaCL::init(unsigned int numThreads) {

}

void ViennaCL::preprocess(thundercat::MMMatrix<double> &matrix) {
  M = matrix.M;
  N = matrix.N;
  std::vector<std::map<int,double>> cpuMatrix(M);
  for(auto elem : matrix.getElements()) {
    cpuMatrix[elem.rowIndex][elem.colIndex] = elem.value;
  }

  viennacl::copy(cpuMatrix, A);

}

void ViennaCL::spmv(double *v, double *w) {
  std::vector<double> cpuX(v, v+M);
  std::vector<double> cpuY(w, w+N);

  viennacl::vector<double> x;
  viennacl::copy(cpuX, x);

  viennacl::vector<double> y = viennacl::linalg::prod(A, x);

  viennacl:copy(y, cpuY);
}