#include <iostream>
#include "spmvRegistry.h"
#include "parse_options.h"
#include "method.h"

#include "mmmatrix.hpp"
#include "profiler.h"

#include <fstream>

using namespace thundercat;

void populateInputOutputVectors(VALUE_TYPE** input,  unsigned int m, VALUE_TYPE** output, unsigned int n) {
  *input = new double[m];
  *output = new double[n];
  for(int i = 0; i < m; ++i) {
    (*input)[i] = i + 1;
  }
  for(int i = 0; i < n; ++i) {
    (*output)[i] = 0;
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

  populateInputOutputVectors(&in, matrix->M, &out, matrix->N);

  // Warm up
  Profiler::recordTime("Warm up", [&]() {
    for (int i = 0; i < cliOptions->warmups; ++i) {
      method->spmv(in, out);
    }
  });

  // Do benchmark
  Profiler::recordSpmv([&]() {
    for (int i = 0; i < cliOptions->iters; ++i) {
      method->spmv(in, out);
    }
  });


  Profiler::print(cliOptions->iters, matrix->getElements().size());

  if (cliOptions->dumpOutput){
    std::ofstream myfile;
    myfile.open (cliOptions->method + ".out");
    for(int i = 0; i < matrix->N; i++) {
      myfile << out[i] << std::endl;
    }
    myfile.close();
  }

  return 0;
}