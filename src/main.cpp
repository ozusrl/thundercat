#include <iostream>
#include "spmvRegistry.h"
#include "parse_options.h"
#include "method.h"

#include "mmmatrix.hpp"
#include "profiler.h"

using namespace thundercat;

const int warmupIterations = 5;

void populateInputOutputVectors(VALUE_TYPE** input, VALUE_TYPE** output, unsigned int n, unsigned int m) {
  *input = new double[m];
  *output = new double[n];
  for(int i = 0; i < m; ++i) {
    (*input)[i] = i + 1;
  }
  for(int i = 0; i < n; ++i) {
    (*input)[i] = i + 1;
  }
}

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


  double *in, *out;

  populateInputOutputVectors(&in, &out, matrix->N, matrix->M);

  // Warm up
  Profiler::recordTime("Warm up", [&]() {
    for (int i = 0; i < warmupIterations; ++i) {
      method->spmv(in, out);
    }
  });

  // Do benchmark
  Profiler::recordTime("Spmv", [&]() {
    for (int i = 0; i < cliOptions->iters; ++i) {
      method->spmv(in, out);
    }
  });


  Profiler::print(cliOptions->iters, matrix->getElements().size());

  return 0;
}