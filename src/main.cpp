#include "profiler.h"
#include "spMVgen.h"
#include "svmAnalyzer.h"
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#ifdef __linux__
#include "omp.h"
#endif

using namespace spMVgen;
using namespace std;

bool __DEBUG__ = false;
bool DUMP_OBJECT = false;
bool DUMP_MATRIX = false;
bool MATRIX_STATS = false;
bool MKL_ENABLED = false;
bool PLAINCSR_ENABLED = false;
unsigned int NUM_OF_THREADS = 1;
int nthreads = 0;

// Caller of this method is responsible for destructing the 
// returned matrix. 
Matrix* readMatrixFromFile(string fileName) {
  ifstream mmFile(fileName.c_str());
  if (!mmFile.is_open()) {
    std::cerr << "Problem with file " << fileName << ".\n";
    exit(1);
  }
  string headerLine;
  // consume the comments until we reach the size info
  while (mmFile.good()) {
    getline (mmFile, headerLine);
    if (headerLine[0] != '%') break;
  }
  
  // Read N, M, NZ
  stringstream header(headerLine, ios_base::in);
  int n, m, nz;
  header >> n >> m >> nz;
  if (n != m) {
    std::cerr << "Only square matrices are accepted.\n";
    exit(1);
  }
  
  // Read rows, cols, vals
  MMMatrix matrix(n);
  int row; int col; double val;

  string line;
  for (int i = 0; i < nz; ++i) {
    getline(mmFile, line);
    stringstream linestream(line, ios_base::in);
    linestream >> row >> col;
    // pattern matrices do not contain val entry.
    // Such matrices are filled in with consecutive numbers.
    linestream >> val;
    if (linestream.fail()) 
      val = i+1;
    // adjust to zero index
    matrix.add(row-1, col-1, val);
  }
  mmFile.close();
  matrix.normalize();
  Matrix *csrMatrix = matrix.toCSRMatrix();
  return csrMatrix;
}

int main(int argc, const char *argv[]) {
  // Usage: spMVgen <matrixName> <specializerName> {-debug|-dump_object|-dump_matrix|-num_threads|-matrix_stats}
  // E.g: spMVgen matrices/fidap037 unfolding 
  // E.g: spMVgen matrices/fidap037 unfolding -debug
  // E.g: spMVgen matrices/fidap037 unfolding -dump_object
  // E.g: spMVgen matrices/fidap037 unfolding -num_threads 6
  string genOSKI("genOSKI");
  string genOSKI33("genOSKI33");
  string genOSKI44("genOSKI44");
  string genOSKI55("genOSKI55");
  string csrByNZ("CSRbyNZ");
  string unfolding("unfolding");
  string stencil("stencil");
  string unrollingWithGOTO("unrollingWithGOTO");
  string mkl("MKL");
  string plainCSR("PlainCSR");

  string debugFlag("-debug");
  string dumpObjFlag("-dump_object");
  string dumpMatrixFlag("-dump_matrix");
  string numThreadsFlag("-num_threads");
  string matrixStatsFlag("-matrix_stats");

  string matrixName(argv[1]);
  Matrix *csrMatrix = readMatrixFromFile(matrixName + ".mtx");
  
  char **argptr = (char**)&argv[2];

  // Read the specializer
  SpMVMethod *method = NULL;
  if(genOSKI.compare(*argptr) == 0) {
    int b_r = atoi(*(++argptr));
    int b_c = atoi(*(++argptr));
    method = new GenOSKI(csrMatrix, b_r, b_c);
  } else if(genOSKI33.compare(*argptr) == 0) {
    method = new GenOSKI(csrMatrix, 3, 3);
  } else if(genOSKI44.compare(*argptr) == 0) {
    method = new GenOSKI(csrMatrix, 4, 4);
  } else if(genOSKI55.compare(*argptr) == 0) {
    method = new GenOSKI(csrMatrix, 5, 5);
  } else if(unfolding.compare(*argptr) == 0) {
    method = new Unfolding(csrMatrix);
  } else if(csrByNZ.compare(*argptr) == 0) {
    method = new CSRbyNZ(csrMatrix);
  } else if(stencil.compare(*argptr) == 0) {
    method = new Stencil(csrMatrix);
  } else if(unrollingWithGOTO.compare(*argptr) == 0) {
    method = new UnrollingWithGOTO(csrMatrix);
  } else if(mkl.compare(*argptr) == 0) {
    method = new MKL(csrMatrix);
    MKL_ENABLED = true;
  } else if(plainCSR.compare(*argptr) == 0) {
    method = new PlainCSR(csrMatrix);
    PLAINCSR_ENABLED = true;
  } else {
    std::cout << "Method " << *argptr << " not found.\n";
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
        std::cout << "Number of threads must be >= 1.\n";
        exit(1);
      }
    } else {
      std::cout << "Unrecognized flag: " << *argptr << "\n";
      exit(1);
    } 
    argptr++;
  }


  if (DUMP_MATRIX) {
    Matrix *matrix = method->getMatrix();
    matrix->print();
    exit(0);
  }

  if (MATRIX_STATS) {
    SVMAnalyzer svmAnalyzer(csrMatrix);
    svmAnalyzer.printFeatures();
    exit(0);
  }

#ifdef __linux__
  if (!MKL_ENABLED) {
    omp_set_num_threads(NUM_OF_THREADS);
#pragma omp parallel
    {
#pragma omp master
      {
        nthreads = omp_get_num_threads();
      }
    }
    if (!__DEBUG__ && !DUMP_OBJECT)
      cout << "Num threads = " << nthreads << "\n";
  }
#endif
  
  vector<MultByMFun> fptrs;
  START_TIME_PROFILE(codeGeneration);
  if (MKL_ENABLED) {
    MKL *mklMethod = (MKL*)method;
    mklMethod->setNumOfThreads(NUM_OF_THREADS);
    fptrs = mklMethod->getMultByMFunctions();
  } else if (PLAINCSR_ENABLED) {
    fptrs = ((PlainCSR*)method)->getMultByMFunctions();
  } else {
    // Generate and run the specialized code
    SpMVSpecializer specializer(method);
    specializer.specialize();
    fptrs = specializer.getMultByMFunctions();
  }
  END_TIME_PROFILE(codeGeneration);
  
  unsigned long n = csrMatrix->n;
  unsigned long nz = csrMatrix->nz;
  double *v = new double[n];
  double *w = new double[n];
  for(int i = 0; i < n; ++i) {
    w[i] = 0;
  }
  for(int i = 0; i < n; ++i) {
    v[i] = i;
  }

  unsigned int ITERS;
  if (nz < 5000) {
    ITERS = 500000;
  } else if (nz < 10000) {
    ITERS = 200000;
  } else if (nz < 50000) {
    ITERS = 100000;
  } else if (nz < 100000) {
    ITERS = 50000;
  } else if (nz < 200000) {
    ITERS = 10000;
  } else if (nz < 1000000) {
    ITERS = 5000;
  } else if (nz < 2000000) {
    ITERS = 1000;
  } else if (nz < 3000000) {
    ITERS = 500;
  } else {
    ITERS = 200;
  }
  
  if (__DEBUG__) {
    ITERS = 1;
  }

  Matrix *matrix = method->getMatrix();
  START_TIME_PROFILE(multByM);
  if (fptrs.size() == 1) {
    for (int i=0; i < ITERS; i++) {
      fptrs[0](v, w, matrix->rows, matrix->cols, matrix->vals);
    }
  } else {    
    for (unsigned i=0; i < ITERS; i++) {
      #pragma omp parallel for
      for (unsigned j = 0; j < fptrs.size(); j++) {
        fptrs[j](v, w, matrix->rows, matrix->cols, matrix->vals);
      }
    }
  }
  END_TIME_PROFILE(multByM);

  if (__DEBUG__) {
    for(int i = 0; i < n; ++i) 
      printf("%g\n", w[i]);
  } else {
    Profiler::print(ITERS);
    std::cout << "0" << std::setw(10) << ITERS << " times    iterated\n";
  }

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

  return 0;
}

