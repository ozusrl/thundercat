#include <iostream>
#include "spmvRegistry.h"
#include "parse_options.h"
#include "method.h"

#include "mmmatrix.hpp"
#include "profiler.h"

using namespace thundercat;

int main(int argc, const char *argv[]) {

  auto cliOptions = parseCliOptions(argc, argv);
  
  auto method = SpmvMethodRegistry::instance().getMethod(cliOptions->method);


  // Read input file
  std::unique_ptr<MMMatrix<VALUE_TYPE>> matrix;
  Profiler::recordTime("ReadInputFile", [&]() {
      matrix = MMMatrix<VALUE_TYPE>::fromFile(cliOptions->mtxFile);
  });


  // Method initialisation
  Profiler::recordTime("Init", [&]() {
      method->init(cliOptions->threads);
  });

  Profiler::recordTime("Preprocess", [&]() {
      method->preprocess(*matrix);
  });


  // Prepare input & output vectors
  auto N =  matrix->N;
  double *in = new double[N];
  double *out = new double[N];


  // Do benchmark
  Profiler::recordTime("Spmv", [&]() {
    for (int i = 0; i < cliOptions->iters; ++i) {
      method->spmv(in, out);
    }
  });


  Profiler::print(cliOptions->iters);

  return 0;
}