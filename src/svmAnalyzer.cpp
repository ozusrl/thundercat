#include "svmAnalyzer.h"
#include "profiler.h"
#include <iostream>
#include <set>
#include <unordered_set>
#include <bitset>

using namespace spMVgen;
using namespace std;

SVMAnalyzer::SVMAnalyzer(Matrix *matrix): matrix(matrix) {
  
}

void SVMAnalyzer::printFeatures() {
  const bool EARLY_EXIT_ENABLED = true;
  
  vector<bitset<32> > blockPatterns44;
  vector<bitset<32> > blockPatterns55;
  blockPatterns44.resize(matrix->n / 4 + 1);
  blockPatterns55.resize(matrix->n / 5 + 1);
  START_TIME_PROFILE(svmAnalysis)
  unsigned long numNonEmptyRows = 0;
  set<unsigned int> nzGroups;
  unordered_set<double> distinctValues;
  map<vector<int>, unsigned int> stencils;
  vector<int> indicesOfBlockColumnsThatExist4;
  vector<int> indicesOfBlockColumnsThatExist5;
  map<unsigned long, unsigned int> patterns44;
  map<unsigned long, unsigned int> patterns55;
  const unsigned long distinctValueLimit = 5000;
  const unsigned long stencilCountLimit = EARLY_EXIT_ENABLED ? 2000 : (matrix->n + 1);
  const unsigned long patternCountLimit = EARLY_EXIT_ENABLED ? 5000 : (matrix->n * matrix->n);

  unsigned long numElementsAnalyzedForStencil = 0;
  unsigned long numElementsAnalyzedForGenOSKI44 = 0;
  unsigned long numElementsAnalyzedForGenOSKI55 = 0;
  
  // Find the matrix partition that covers the most elements
  const vector<MatrixStripeInfo> &stripeInfos = matrix->getStripeInfos();
  unsigned int stripeIndexWithMaxCoverage = 0;
  for (unsigned int i = 0; i < stripeInfos.size(); ++i) {
    unsigned long maxCoverage = (stripeInfos[stripeIndexWithMaxCoverage].valIndexEnd - stripeInfos[stripeIndexWithMaxCoverage].valIndexBegin);
    if ((stripeInfos[i].valIndexEnd - stripeInfos[i].valIndexBegin) > maxCoverage) {
      stripeIndexWithMaxCoverage = i;
    }
  }
  const MatrixStripeInfo *stripeInfo = &(stripeInfos[stripeIndexWithMaxCoverage]);
  
  for (unsigned i = stripeInfo->rowIndexBegin; i < stripeInfo->rowIndexEnd; ++i) {
    int rowStart = matrix->rows[i];
    int rowEnd = matrix->rows[i+1];
    unsigned int rowLength = rowEnd - rowStart;
    vector<int> stencil;
    int blockRow4 = i/4;
    int blockRow5 = i/5;

    if (rowLength > 0) {
      numNonEmptyRows++;
    }
    nzGroups.insert(rowLength);

    for (int k = matrix->rows[i]; k < matrix->rows[i+1]; ++k) {
      if (distinctValues.size() < distinctValueLimit) {
        distinctValues.insert(matrix->vals[k]);
      }
      if (stencils.size() < stencilCountLimit) {
        numElementsAnalyzedForStencil++;
        stencil.push_back(matrix->cols[k] - i);
      }
      if (patterns44.size() < patternCountLimit) {
        numElementsAnalyzedForGenOSKI44++;
        int col = matrix->cols[k];
        int row = i;
        int blockCol = col/4;
        unsigned int elementPosition = (row % 4) * 4 + (col % 4);
        blockPatterns44[blockCol].set(elementPosition);
        indicesOfBlockColumnsThatExist4.push_back(blockCol);
      }
      if (patterns55.size() < patternCountLimit) {
        numElementsAnalyzedForGenOSKI55++;
        int col = matrix->cols[k];
        int row = i;
        int blockCol = col/5;
        unsigned int elementPosition = (row % 5) * 5 + (col % 5);
        blockPatterns55[blockCol].set(elementPosition);
        indicesOfBlockColumnsThatExist5.push_back(blockCol);
      }
    }
    if (stencil.size() > 0 && stencils.size() < stencilCountLimit) {
      stencils[stencil] += 1;
    }
    if (patterns44.size() < patternCountLimit && ((i % 4 == 3) || i == stripeInfo->rowIndexEnd - 1)) {
      for (auto &blockIndex : indicesOfBlockColumnsThatExist4) {
        if (blockPatterns44[blockIndex].count() > 0) {
          patterns44[blockPatterns44[blockIndex].to_ulong()] += 1;
          blockPatterns44[blockIndex].reset();
        }
      }
      indicesOfBlockColumnsThatExist4.clear();
    }
    if (patterns55.size() < patternCountLimit && ((i % 5 == 4) || i == stripeInfo->rowIndexEnd - 1)) {
      for (auto &blockIndex : indicesOfBlockColumnsThatExist5) {
        if (blockPatterns55[blockIndex].count() > 0) {
          patterns55[blockPatterns55[blockIndex].to_ulong()] += 1;
          blockPatterns55[blockIndex].reset();
        }
      }
      indicesOfBlockColumnsThatExist5.clear();
    }
  }
  
  unsigned long numSingleRowStencils = 0;
  unsigned long numMultiRowStencils = 0;
  unsigned long sumOfSingleRowStencilLengths = 0;
  unsigned long sumOfMultiRowStencilLengths = 0;
  
  unsigned long numRowsCoveredByMultiRowStencils = 0;
  unsigned long numElementsCoveredByGoodStencils = 0;
  for (auto &stencilInfo: stencils) {
    if (stencilInfo.second == 1) {
      numSingleRowStencils++;
      sumOfSingleRowStencilLengths += stencilInfo.first.size();
    } else {
      numMultiRowStencils++;
      sumOfMultiRowStencilLengths += stencilInfo.first.size();
      numRowsCoveredByMultiRowStencils += stencilInfo.second;
      if (stencilInfo.first.size() > 3 && (stencilInfo.first.size() * stencilInfo.second) >= 1000) {
        numElementsCoveredByGoodStencils += stencilInfo.first.size() * stencilInfo.second;
      }
    }
  }

  unsigned long sumOfNZLengths = 0;
  for (auto length : nzGroups) {
    sumOfNZLengths += length;
  }
  
  unsigned long sumOfPatternLengths44 = 0;
  unsigned long sumOfPatternLengths55 = 0;
  unsigned long numBlocks44 = 0;
  unsigned long numBlocks55 = 0;
  unsigned long numElementsCoveredByGoodPatterns44 = 0;
  unsigned long numElementsCoveredByGoodPatterns55 = 0;
  for (auto &patternInfo : patterns44) {
    bitset<32> bits(patternInfo.first);
    sumOfPatternLengths44 += bits.count();
    numBlocks44 += patternInfo.second;
    if (bits.count() > 3 && patternInfo.second >= 1000) { // at least 4 elements and 1000 blocks.
      numElementsCoveredByGoodPatterns44 += bits.count() * patternInfo.second;
    }
  }
  for (auto &patternInfo : patterns55) {
    bitset<32> bits(patternInfo.first);
    sumOfPatternLengths55 += bits.count();
    numBlocks55 += patternInfo.second;
    if (bits.count() > 3 && patternInfo.second >= 1000) { // at least 4 elements and 1000 blocks.
      numElementsCoveredByGoodPatterns55 += bits.count() * patternInfo.second;
    }
  }
  
  END_TIME_PROFILE(svmAnalysis)
  
  cout << "svmAnalysisTime: " << (time_svmAnalysis_Diff.tv_sec*1000000)+time_svmAnalysis_Diff.tv_usec << "\n";
  
  // General
  cout << (stripeInfo->rowIndexEnd - stripeInfo->rowIndexBegin) << " ";
  cout << (stripeInfo->valIndexEnd - stripeInfo->valIndexBegin) << " ";
  cout << numNonEmptyRows << " ";
  
  // CSRbyNZ
  cout << nzGroups.size() << " ";
  cout << sumOfNZLengths << " ";
  cout << numNonEmptyRows / (double)nzGroups.size() << " ";
  cout << sumOfNZLengths / (double)nzGroups.size() << " ";
  
  // Stencil
  cout << numSingleRowStencils << " ";
  cout << numMultiRowStencils << " ";

  cout << sumOfSingleRowStencilLengths << " ";
  cout << sumOfMultiRowStencilLengths << " ";
  cout << (numMultiRowStencils  == 0 ? 0 : (numRowsCoveredByMultiRowStencils  / (double)numMultiRowStencils)) << " ";

  cout << (numSingleRowStencils == 0 ? 0 : (sumOfSingleRowStencilLengths / (double)numSingleRowStencils)) << " ";
  cout << (numMultiRowStencils  == 0 ? 0 : (sumOfMultiRowStencilLengths  / (double)numMultiRowStencils)) << " ";

  cout << numElementsCoveredByGoodStencils / (double)numElementsAnalyzedForStencil << " ";
  
  // Unfolding
  cout << distinctValues.size() << " ";

  // GenOSKI44
  cout << patterns44.size() << " ";

  cout << sumOfPatternLengths44 << " ";
  cout << numBlocks44 << " ";
  cout << numBlocks44 / (double)patterns44.size() << " ";
  
  cout << sumOfPatternLengths44 / (double)patterns44.size() << " ";

  cout << numElementsCoveredByGoodPatterns44 / (double)numElementsAnalyzedForGenOSKI44 << " ";

  // GenOSKI55
  cout << patterns55.size() << " ";
  
  cout << sumOfPatternLengths55 << " ";
  cout << numBlocks55 << " ";
  cout << numBlocks55 / (double)patterns55.size() << " ";
  
  cout << sumOfPatternLengths55 / (double)patterns55.size() << " ";
  
  cout << numElementsCoveredByGoodPatterns55 / (double)numElementsAnalyzedForGenOSKI55 << " ";
  
  // NZ per row
  cout << (stripeInfo->valIndexEnd - stripeInfo->valIndexBegin) / (double)(stripeInfo->rowIndexEnd - stripeInfo->rowIndexBegin) << " ";
  
  cout << "\n";
}


