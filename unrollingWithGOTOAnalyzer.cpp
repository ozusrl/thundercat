#include "unrollingWithGOTOAnalyzer.h"
#include "profiler.h"
#include <iostream>
#include <fstream>
#include <sstream>

extern unsigned int NUM_OF_THREADS;

using namespace spMVgen;
using namespace std;

///
/// UnrollingWithGOTOAnalyzer
///
UnrollingWithGOTOAnalyzer::UnrollingWithGOTOAnalyzer(Matrix *csrMatrix): CSRbyNZAnalyzer(csrMatrix) {
}

void UnrollingWithGOTOAnalyzer::analyzeMatrix() {
  vector<NZtoRowMap> *rowByNZLists = CSRbyNZAnalyzer::getRowByNZLists();

  for (auto &rowByNZList : *rowByNZLists) {
    maxRowLengths.push_back(rowByNZList.begin()->first);
  }
}

vector<unsigned long> *UnrollingWithGOTOAnalyzer::getMaxRowLengths() {
  if (maxRowLengths.size() == 0)
    analyzeMatrix();
  return &maxRowLengths;
}

