#ifndef _UNFOLDING_ANALYZER_H_
#define _UNFOLDING_ANALYZER_H_

#include "matrix.h"
#include <iostream>

#define DISTINCT_VALUE_COUNT_LIMIT 5000

namespace spMVgen {
  
  class UnfoldingAnalyzer {
  private:
    std::vector<std::map<double, unsigned long> > valToIndexMaps;
    std::vector<std::vector<double> > distinctValueLists;
    Matrix *csrMatrix;

    void analyzeMatrix();
    
  public:
    UnfoldingAnalyzer(Matrix *csrMatrix);
    std::vector<std::map<double, unsigned long> > *getValToIndexMaps();
    std::vector<std::vector<double> > *getValues();
    bool hasFewDistinctValues();
  };
}

#endif
