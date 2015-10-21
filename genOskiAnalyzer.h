#ifndef _GENOSKI_ANALYZER_H_
#define _GENOSKI_ANALYZER_H_

#include "matrix.h"
#include <iostream>
#include <bitset>

namespace spMVgen {
  // This is a map from pattern to block indices and values
  typedef std::map<unsigned long, std::pair<std::vector<std::pair<int, int> >, std::vector<double> > > GroupByBlockPatternMap;

  class GenOSKIAnalyzer {
  private:
    unsigned int b_r, b_c;

    std::vector<unsigned int> numBlocks;
    
    Matrix *csrMatrix;
    
    std::vector<GroupByBlockPatternMap> groupByBlockPatternMaps;

    void analyzeMatrix();
    
  public:
    GenOSKIAnalyzer(Matrix *matrix, unsigned int b_r, unsigned int b_c);

    std::vector<GroupByBlockPatternMap> *getGroupByBlockPatternMaps();

    std::vector<unsigned int> &getNumBlocks();
  };
}

#endif
