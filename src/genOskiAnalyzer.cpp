#include "genOskiAnalyzer.h"
#include "profiler.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <bitset>

using namespace spMVgen;
using namespace std;

extern unsigned int NUM_OF_THREADS;

///
/// GenOskiAnalyzer
///
GenOSKIAnalyzer::GenOSKIAnalyzer(Matrix *csrMatrix, unsigned int b_r, unsigned int b_c):
  csrMatrix(csrMatrix), b_r(b_r), b_c(b_c) {
    
}

struct BlockInfo {
  bitset<32> pattern;
  vector<double> vals;
};

void GenOSKIAnalyzer::analyzeMatrix() {
  groupByBlockPatternMaps.resize(NUM_OF_THREADS);
  numBlocks.resize(NUM_OF_THREADS);
  
  // Split the matrix
  const vector<MatrixStripeInfo> &stripeInfos = csrMatrix->getStripeInfos();
  
#pragma omp parallel for
  for (int threadIndex = 0; threadIndex < NUM_OF_THREADS; ++threadIndex) {
    auto &stripeInfo = stripeInfos[threadIndex];
    map<int, BlockInfo> currentBlockRowPatternsAndElements;
    vector<BlockInfo> blockPatterns;
    blockPatterns.resize(csrMatrix->n / b_c + 1);
    vector<int> indicesOfDetectedBlockColumns;
  
    for (unsigned long rowIndex = stripeInfo.rowIndexBegin; rowIndex < stripeInfo.rowIndexEnd; ++rowIndex) {
      int rowStart = csrMatrix->rows[rowIndex];
      int rowEnd = csrMatrix->rows[rowIndex+1];
      
      for (int k = rowStart; k < rowEnd; ++k) {
        int col = csrMatrix->cols[k];
        int row = rowIndex;
        int blockCol = col/b_c;
        unsigned int elementPosition = (row % b_r) * b_c + (col % b_c);
        blockPatterns[blockCol].pattern.set(elementPosition);
        blockPatterns[blockCol].vals.reserve(b_r * b_c);
        blockPatterns[blockCol].vals.push_back(csrMatrix->vals[k]);
        indicesOfDetectedBlockColumns.push_back(blockCol);
      }
      if ((rowIndex % b_r) == b_r - 1 || rowIndex == stripeInfo.rowIndexEnd - 1) {
        for (auto &blockIndex : indicesOfDetectedBlockColumns) {
          if (blockPatterns[blockIndex].pattern.count() > 0) {
            auto patternInfo = &(groupByBlockPatternMaps[threadIndex][blockPatterns[blockIndex].pattern.to_ulong()]);
            blockPatterns[blockIndex].pattern.reset();
            
            patternInfo->first.push_back(pair<int,int>(rowIndex/b_r, blockIndex));
            vector<double> &vals = patternInfo->second;
            vals.insert(vals.end(), blockPatterns[blockIndex].vals.begin(), blockPatterns[blockIndex].vals.end());
            blockPatterns[blockIndex].vals.clear();
            numBlocks[threadIndex] += 1;
          }
        }
        indicesOfDetectedBlockColumns.clear();
      }      
    }
  }
}

vector<GroupByBlockPatternMap> *GenOSKIAnalyzer::getGroupByBlockPatternMaps() {
  if (groupByBlockPatternMaps.size() == 0)
    analyzeMatrix();
  return &groupByBlockPatternMaps;
}

vector<unsigned int> &GenOSKIAnalyzer::getNumBlocks() {
  return numBlocks;
}

