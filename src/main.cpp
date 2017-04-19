#include "profiler.h"
#include "svmAnalyzer.h"
#include "method.h"
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <stdio.h>
#ifdef OPENMP_EXISTS
#include "omp.h"
#endif

using namespace thundercat;
using namespace std;
using namespace asmjit;

bool __DEBUG__ = false;
bool DUMP_OBJECT = false;
bool DUMP_MATRIX = false;
bool MATRIX_STATS = false;
bool MKL_ENABLED = false; // TODO: Get rid of this
unsigned int NUM_OF_THREADS = 1;
Matrix *csrMatrix;
SpMVMethod *method;
vector<MultByMFun> fptrs;
double *vVector;
double *wVector;


void parseCommandLineArguments(int argc, const char *argv[]);
void setParallelism();
void dumpMatrixIfRequested();
void doSVMAnalysisIfRequested();
void registerLoggersIfRequested();
void generateFunctions();
void dumpObjectIfRequested();
void populateInputOutputVectors();
void benchmark();
void cleanup();

int main(int argc, const char *argv[]) {
  parseCommandLineArguments(argc, argv);
  setParallelism();
  method->init(csrMatrix, NUM_OF_THREADS);
  dumpMatrixIfRequested();
  doSVMAnalysisIfRequested();
  registerLoggersIfRequested();
  generateFunctions();
  populateInputOutputVectors();
  benchmark();
  cleanup();
  return 0;
}

void parseCommandLineArguments(int argc, const char *argv[]) {
  // Usage: thundercat <matrixName> <specializerName> {-debug|-dump_object|-dump_matrix|-num_threads|-matrix_stats}
  // E.g: thundercat matrices/fidap037 CSRbyNZ
  // E.g: thundercat matrices/fidap037 RowPattern -debug
  // E.g: thundercat matrices/fidap037 GenOSKI44 -dump_object
  // E.g: thundercat matrices/fidap037 PlainCSR -num_threads 6
  string genOSKI("GenOSKI");
  string genOSKI33("GenOSKI33");
  string genOSKI44("GenOSKI44");
  string genOSKI55("GenOSKI55");
  string csrByNZ("CSRbyNZ");
  string unfolding("Unfolding");
  string rowPattern("RowPattern");
  string unrollingWithGOTO("UnrollingWithGOTO");
  string csrWithGOTO("CSRWithGOTO");
  string mkl("MKL");
  string plainCSR("PlainCSR");
  
  string debugFlag("-debug");
  string dumpObjFlag("-dump_object");
  string dumpMatrixFlag("-dump_matrix");
  string numThreadsFlag("-num_threads");
  string matrixStatsFlag("-matrix_stats");
  
  string matrixName(argv[1]);
  csrMatrix = Matrix::readMatrixFromFile(matrixName + ".mtx");
  
  char **argptr = (char**)&argv[2];
  
  // Read the specializer
  if(genOSKI.compare(*argptr) == 0) {
    int b_r = atoi(*(++argptr));
    int b_c = atoi(*(++argptr));
    method = new GenOSKI(b_r, b_c);
  } else if(genOSKI33.compare(*argptr) == 0) {
    method = new GenOSKI(3, 3);
  } else if(genOSKI44.compare(*argptr) == 0) {
    method = new GenOSKI(4, 4);
  } else if(genOSKI55.compare(*argptr) == 0) {
    method = new GenOSKI(5, 5);
  } else if(unfolding.compare(*argptr) == 0) {
    method = new Unfolding();
  } else if(csrByNZ.compare(*argptr) == 0) {
    method = new CSRbyNZ();
  } else if(rowPattern.compare(*argptr) == 0) {
    method = new RowPattern();
  } else if(unrollingWithGOTO.compare(*argptr) == 0) {
    method = new UnrollingWithGOTO();
  } else if(csrWithGOTO.compare(*argptr) == 0) {
    method = new CSRWithGOTO();
  } else if(mkl.compare(*argptr) == 0) {
    method = new MKL();
    MKL_ENABLED = true;
  } else if(plainCSR.compare(*argptr) == 0) {
    method = new PlainCSR();
  } else {
    std::cerr << "Method " << *argptr << " not found.\n";
    exit(1);
  }
  argptr++;
  
  while (argc > argptr - (char**)&argv[0]) { // the optional flag exists
    if (debugFlag.compare(*argptr) == 0)
      __DEBUG__ = true;
    else if (dumpObjFlag.compare(*argptr) == 0)
      DUMP_OBJECT = true;
    else if (dumpMatrixFlag.compare(*argptr) == 0)
      DUMP_MATRIX = true;
    else if (matrixStatsFlag.compare(*argptr) == 0)
      MATRIX_STATS = true;
    else if (numThreadsFlag.compare(*argptr) == 0) {
      NUM_OF_THREADS = atoi(*(++argptr));
      if (NUM_OF_THREADS < 1) {
        std::cerr << "Number of threads must be >= 1.\n";
        exit(1);
      }
    } else {
      std::cerr << "Unrecognized flag: " << *argptr << "\n";
      exit(1);
    }
    argptr++;
  }
}

void setParallelism() {
#ifdef OPENMP_EXISTS
  omp_set_num_threads(NUM_OF_THREADS);
  int nthreads = -1;
#pragma omp parallel
  {
#pragma omp master
    {
      nthreads = omp_get_num_threads();
    }
  }
  if (!__DEBUG__ && !DUMP_OBJECT)
    cout << "Num threads = " << nthreads << "\n";
#endif
}

void dumpMatrixIfRequested() {
  if (DUMP_MATRIX) {
    Matrix *matrix = method->getMethodSpecificMatrix();
    matrix->print();
    exit(0);
  }
}

void doSVMAnalysisIfRequested() {
  if (MATRIX_STATS) {
    SVMAnalyzer svmAnalyzer(csrMatrix);
    svmAnalyzer.printFeatures();
    exit(0);
  }
}

void registerLoggersIfRequested() {
  if (DUMP_OBJECT && method->isSpecializer()) {
    Specializer *specializer = (Specializer*)method;
    auto codeHolders = specializer->getCodeHolders();
    for (int i = 0; i < codeHolders->size(); i++) {
      auto codeHolder = codeHolders->at(i);
      FileLogger *logger = new FileLogger();
      //codeLoggers.push_back(logger);
      std::string fileName("generated_");
      fileName.append(std::to_string(i));
      logger->setStream(fopen(fileName.c_str(), "w"));
      logger->addOptions(Logger::kOptionBinaryForm);
      codeHolder->setLogger(logger);
    }
  }
}

void generateFunctions() {
  Profiler::recordTime("processMatrix", []() {
    method->processMatrix();
  });
  Profiler::recordTime("emitCode", []() {
    method->emitCode();
  });
}

void populateInputOutputVectors() {
  unsigned long n = csrMatrix->n;
  unsigned long nz = csrMatrix->nz;
  vVector = new double[n];
  wVector = new double[n];
  for(int i = 0; i < n; ++i) {
    vVector[i] = i + 1;
    wVector[i] = i + 1;
  }
}

unsigned int getNumIterations() {
  if (__DEBUG__) {
    return 1;
  }
  if (DUMP_OBJECT) {
    return 0;
  }

  unsigned long nz = csrMatrix->nz;

  if (nz < 5000) {
    return 500000;
  } else if (nz < 10000) {
    return 200000;
  } else if (nz < 50000) {
    return 100000;
  } else if (nz < 100000) {
    return 50000;
  } else if (nz < 200000) {
    return 10000;
  } else if (nz < 1000000) {
    return 5000;
  } else if (nz < 2000000) {
    return 1000;
  } else if (nz < 3000000) {
    return 500;
  } else if (nz < 5000000) {
    return 200;
  } else if (nz < 8000000) {
    return 100;
  } else if (nz < 12000000) {
    return 50;
  } else {
    return 100;
  }
}

void benchmark() {
  unsigned int ITERS = getNumIterations();
  unsigned long n = csrMatrix->n;
  
  Profiler::recordTime("multByM", [ITERS]() {
    for (unsigned i=0; i < ITERS; i++) {
      method->spmv(vVector, wVector);
    }
  });
  
  if (__DEBUG__) {
    for(int i = 0; i < n; ++i)
      printf("%g\n", wVector[i]);
  } else {
    Profiler::print(ITERS);
  }
}

void cleanup() {
  /* Normally, a cleanup as follows is the client's responsibility.
     Because we're aliasing some pointers in the returned matrices
     for Unfolding, MKL, and PlainCSR, these deallocations would cause
     double free's. Can be fixed by using shared pointers.
   
   delete csrMatrix;
   delete matrix;
   delete[] v;
   delete[] w;
   delete method;
  */
}
