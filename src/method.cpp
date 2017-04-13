#include "profiler.h"
#include "method.h"
#include <iostream>
#include <sstream>

extern unsigned int NUM_OF_THREADS;

using namespace spMVgen;
using namespace asmjit;

SpMVMethod::SpMVMethod(Matrix *csrMatrix) {
  this->csrMatrix = csrMatrix;
  this->matrix = csrMatrix;
}

SpMVMethod::~SpMVMethod() {
  
}

bool SpMVMethod::isSpecializer() {
  return false;
}

void SpMVMethod::emitCode() {
  // By default, do nothing
}

Matrix* SpMVMethod::getCustomMatrix() {
  return matrix;
}

void SpMVMethod::processMatrix() {
  START_TIME_PROFILE(getStripeInfos);
  stripeInfos = csrMatrix->getStripeInfos();
  END_TIME_PROFILE(getStripeInfos);
  START_TIME_PROFILE(analyzeMatrix);
  analyzeMatrix();
  END_TIME_PROFILE(analyzeMatrix);
  START_TIME_PROFILE(convertMatrix);
  convertMatrix();
  END_TIME_PROFILE(convertMatrix);
}

Specializer::Specializer(Matrix *csrMatrix):
SpMVMethod(csrMatrix) {
  for (int i = 0; i < NUM_OF_THREADS; i++) {
    codeHolders.push_back(new CodeHolder);
    codeHolders[i]->init(rt.getCodeInfo());
  }
}

bool Specializer::isSpecializer() {
  return true;
}

void Specializer::emitCode() {
#pragma omp parallel for
  for (unsigned int i = 0; i < NUM_OF_THREADS; i++) {
    emitMultByMFunction(i);
    codeHolders[i]->sync();
  }
}

std::vector<MultByMFun> Specializer::getMultByMFunctions() {
  std::vector<MultByMFun> fptrs;
  for (int i = 0; i < NUM_OF_THREADS; i++) {
    MultByMFun fn;
    asmjit::Error err = rt.add(&fn, codeHolders[i]);
    if (err) {
      std::cerr << "Problem occurred while adding function " << i << " to Runtime.\n";
      std::cerr << err;
      exit(1);
    }
    fptrs.push_back(fn);
  }
  return fptrs;
}

std::vector<CodeHolder*> *Specializer::getCodeHolders() {
  return &codeHolders;
}

static std::string newName(const std::string &str, int i) {
  std::ostringstream oss;
  oss << i;
  std::string name(str);
  return name + oss.str();
}
