#include <iostream>
#include "spmvRegistry.h"
#include "parse_options.h"
#include "method.h"

#include "mmmatrix.hpp"
#include "profiler.h"

using namespace thundercat;

long long int deltaMicroseconds(
    std::chrono::steady_clock::time_point &t2,
    std::chrono::steady_clock::time_point &t1) {
  return std::chrono::duration_cast<std::chrono::microseconds>(t2 -t1).count();
}

int main(int argc, const char *argv[]) {

  auto cliOptions = parseCliOptions(argc, argv);
  
  auto method = SpmvMethodRegistry::instance().getMethod(cliOptions->method);

  auto readStart = std::chrono::high_resolution_clock::now();
  auto matrix = MMMatrix<VALUE_TYPE>::fromFile(cliOptions->mtxFile);

  auto initStart = std::chrono::high_resolution_clock::now();

  auto N =  matrix->N;
  method->init(cliOptions->threads);

  method->processMatrix(std::move(matrix));

  double *in = new double[N];
  double *out = new double[N];


  auto computeStart = std::chrono::high_resolution_clock::now();
  for (int i = 0; i <cliOptions->iters; ++i) {
    method->spmv(in, out);
  }
  auto computeEnd = std::chrono::high_resolution_clock::now();


  std::cout << std::endl <<
               "=====================  " << std::endl <<
               "Reading Matrix       : " << deltaMicroseconds(initStart, readStart) << " us" << std::endl <<
               "Initializing Matrix  : " << deltaMicroseconds(computeStart, initStart) << " us" << std::endl <<
               "Computed " << cliOptions->iters << " iterations:" <<std::endl <<
               "    Total            : " << deltaMicroseconds(computeEnd, computeStart) << " us" << std::endl <<
               "    Single Iteration : " << deltaMicroseconds(computeEnd, computeStart)/cliOptions->iters << " us" << std::endl;

  return 0;
}