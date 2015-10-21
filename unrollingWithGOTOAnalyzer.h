#ifndef _UNROLLING_WITH_GOTO_ANALYZER_H_
#define _UNROLLING_WITH_GOTO_ANALYZER_H_

#include "matrix.h"
#include "csrByNZAnalyzer.h"
#include <iostream>

namespace spMVgen {
  
  class UnrollingWithGOTOAnalyzer : public CSRbyNZAnalyzer {
  private:
    std::vector<unsigned long> maxRowLengths;

    void analyzeMatrix();
    
  public:
    UnrollingWithGOTOAnalyzer(Matrix *csrMatrix);
    
    std::vector<unsigned long> *getMaxRowLengths();
  };
}

#endif
