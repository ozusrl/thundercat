#ifndef _STENCIL_ANALYZER_H_
#define _STENCIL_ANALYZER_H_

#include "matrix.h"
#include <iostream>

namespace spMVgen {

  typedef std::vector<int> StencilPattern;
  typedef std::vector<int> RowIndices;
  
  typedef std::map<StencilPattern, RowIndices > StencilToRowMap;
  
  class StencilAnalyzer {
  private:
    Matrix *csrMatrix;
    std::vector<StencilToRowMap> stencilLists;
    
    void analyzeMatrix();
    
  public:
    StencilAnalyzer(Matrix *csrMatrix);
    std::vector<StencilToRowMap> *getStencilLists();
  };
}

#endif
