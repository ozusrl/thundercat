#ifndef _SVM_ANALYZER_H_
#define _SVM_ANALYZER_H_

#include "matrix.h"
#include <iostream>

namespace thundercat {
  class SVMAnalyzer {
  public:
    SVMAnalyzer(Matrix *matrix);
    
    void printFeatures();
    
  private:
    Matrix *matrix;
  };
}

#endif
