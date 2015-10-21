#ifndef _CSRbyNZ_ANALYZER_H_
#define _CSRbyNZ_ANALYZER_H_

#include "matrix.h"
#include <map>
#include <iostream>

namespace spMVgen {

  class RowByNZ {
  public:
    std::vector<int> *getRowIndices();
    void addRowIndex(int index);
    
  private:
    std::vector<int> rowIndices;
  };
  
  // To keep the map keys in ascending order, using the "greater" comparator.
  typedef std::map<unsigned long, RowByNZ, std::greater<unsigned long> > NZtoRowMap;
  
  class CSRbyNZAnalyzer {
  protected:
    Matrix *csrMatrix;
    std::vector<NZtoRowMap> rowByNZLists;
    
    void analyzeMatrix();

  public:
    CSRbyNZAnalyzer(Matrix *csrMatrix);
    std::vector<NZtoRowMap> *getRowByNZLists();
  };
}

#endif
