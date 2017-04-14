#ifndef _CSR_WITH_GOTO_ANALYZER_H_
#define _CSR_WITH_GOTO_ANALYZER_H_

#include "matrix.h"
#include <iostream>

namespace thundercat {
  
  class CSRWithGOTOAnalyzer {
  private:
    Matrix *csrMatrix;
    std::vector<unsigned long> maxRowLengths;

    void analyzeMatrix();
    
  public:
    CSRWithGOTOAnalyzer(Matrix *csrMatrix);
    
    std::vector<unsigned long> *getMaxRowLengths();
  };
}

#endif
