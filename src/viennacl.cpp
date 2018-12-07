#include "viennacl.hpp"
#include "spmvRegistry.h"
#include "profiler.h"
#include <iostream>
#include <fstream>

using namespace thundercat;


REGISTER_METHOD(ViennaCL, "viennacl")

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


  Profiler::recordSpmvOverhead("Copy input matrix to Device", [&]() {
      std::vector<double> cpuX(v, v+M);
      adapter->setX(cpuX);
  });

  adapter->spmv();

  Profiler::recordSpmvOverhead("Copy output matrix to Host", [&]() {
      std::vector<double> cpuY(w, w+N);
      adapter->getY(cpuY);
      memcpy(w, cpuY.data(), N * sizeof(double));
  });

}