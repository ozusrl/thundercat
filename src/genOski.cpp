#include "specializers.h"
#include "spmvRegistry.h"
#include <iostream>
#include <bitset>

using namespace thundercat;
using namespace std;
using namespace asmjit;
using namespace x86;

///
/// Analysis
///
struct BlockInfo {
  bitset<32> pattern;
  vector<double> vals;
};

void GenOSKI::analyzeMatrix() {
  groupByBlockPatternMaps.resize(stripeInfos->size());
  numBlocks.resize(stripeInfos->size());
  
#pragma omp parallel for
  for (int threadIndex = 0; threadIndex < stripeInfos->size(); ++threadIndex) {
    auto &stripeInfo = stripeInfos->at(threadIndex);
    map<int, BlockInfo> currentBlockRowPatternsAndElements;
    vector<BlockInfo> blockPatterns;
    blockPatterns.resize(csrMatrix->M / b_c + 1);
    vector<int> indicesOfDetectedBlockColumns;
    
    for (unsigned long rowIndex = stripeInfo.rowIndexBegin; rowIndex < stripeInfo.rowIndexEnd; ++rowIndex) {
      int rowStart = csrMatrix->rowPtr[rowIndex];
      int rowEnd = csrMatrix->rowPtr[rowIndex+1];
      
      for (int k = rowStart; k < rowEnd; ++k) {
        int col = csrMatrix->colIndices[k];
        int row = rowIndex;
        int blockCol = col/b_c;
        unsigned int elementPosition = (row % b_r) * b_c + (col % b_c);
        blockPatterns[blockCol].pattern.set(elementPosition);
        blockPatterns[blockCol].vals.reserve(b_r * b_c);
        blockPatterns[blockCol].vals.push_back(csrMatrix->values[k]);
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


///
/// GenOSKI
///

GenOSKI::GenOSKI(unsigned int b_r, unsigned int b_c) {
  this->b_r = b_r;
  this->b_c = b_c;
}

void GenOSKI::convertMatrix() {
  unsigned int numTotalBlocks = 0;
  vector<unsigned int> blockBaseIndices;
  for (auto n : numBlocks) {
    blockBaseIndices.push_back(numTotalBlocks);
    numTotalBlocks += n;
  }
  
  int *rows = new int[numTotalBlocks];
  int *cols = new int[numTotalBlocks];
  double *vals = new double[csrMatrix->NZ];
  
#pragma omp parallel for
  for (int t = 0; t < stripeInfos->size(); ++t) {
    auto &blockPatterns = groupByBlockPatternMaps.at(t);
    int *rowsPtr = rows + blockBaseIndices[t];
    int *colsPtr = cols + blockBaseIndices[t];
    double *valsPtr = vals + stripeInfos->at(t).valIndexBegin;
    
    //Build rows cols vals for the new Matrix
    for (auto &patternInfo : blockPatterns) {
      for (auto &blockIdx : patternInfo.second.first) {
        *rowsPtr++ = blockIdx.first * b_r;
        *colsPtr++ = blockIdx.second * b_c;
      }
      for (auto &val : patternInfo.second.second) {
        *valsPtr++ = val;
      }
    }
  }

  matrix = std::make_unique<CSRMatrix<VALUE_TYPE>>(rows, cols, vals, csrMatrix->N, csrMatrix->M, csrMatrix->NZ);
}

///
/// GenOSKICodeEmitter:
/// Helper class to avoid having to pass several parameters
///
class GenOSKICodeEmitter {
public:
  GenOSKICodeEmitter(X86Assembler *assembler,
                     GroupByBlockPatternMap *patternMap,
                     unsigned long baseValsIndex,
                     unsigned long baseBlockIndex,
                     unsigned int b_r,
                     unsigned int b_c) {
    this->assembler = assembler;
    this->patternMap = patternMap;
    this->baseValsIndex = baseValsIndex;
    this->baseBlockIndex = baseBlockIndex;
    this->b_r = b_r;
    this->b_c = b_c;
  }
  
  void emit();
  
private:
  X86Assembler *assembler;
  GroupByBlockPatternMap *patternMap;
  unsigned long baseValsIndex;
  unsigned long baseBlockIndex;
  unsigned int b_r;
  unsigned int b_c;
  
  void emitHeader();
  
  void emitFooter();
  
  void emitSingleLoop(bitset<32> &patternBits, unsigned int numBlocks);
};

void GenOSKI::emitMultByMFunction(unsigned int index) {
  unsigned int numTotalBlocks = 0;
  vector<unsigned int> blockBaseIndices;
  for (auto n : numBlocks) {
    blockBaseIndices.push_back(numTotalBlocks);
    numTotalBlocks += n;
  }
  
  X86Assembler assembler(codeHolders[index]);
  GroupByBlockPatternMap &patternMap = groupByBlockPatternMaps.at(index);
  GenOSKICodeEmitter emitter(&assembler,
                             &patternMap,
                             stripeInfos->at(index).valIndexBegin,
                             blockBaseIndices[index],
                             b_r,
                             b_c);
  emitter.emit();
}

void GenOSKICodeEmitter::emit() {
  emitHeader();
  
  for (auto &pattern : *patternMap) {
    bitset<32> patternBits(pattern.first);
    unsigned int numBlocks = pattern.second.first.size();
    emitSingleLoop(patternBits, numBlocks);
  }
  
  emitFooter();
}

void GenOSKICodeEmitter::emitHeader() {
  // rows is in %rdx, cols is in %rcx, vals is in %r8
  assembler->push(r11);
  assembler->lea(r11, ptr(r8, (int)(sizeof(double) * baseValsIndex))); // using %r11 for vals
  assembler->push(r8);
  assembler->lea(r8, ptr(rdx, (int)(sizeof(int) * baseBlockIndex))); // using %r8 for rows
  assembler->push(r9);
  assembler->lea(r9, ptr(rcx, (int)(sizeof(int) * baseBlockIndex))); // using %r9 for cols
  
  assembler->push(rax);
  assembler->push(rcx);
  assembler->push(rdx);
  assembler->push(rbx);

  // xorl %ecx, %ecx
  assembler->xor_(ecx, ecx);
}

void GenOSKICodeEmitter::emitFooter() {
  assembler->pop(rbx);
  assembler->pop(rdx);
  assembler->pop(rcx);
  assembler->pop(rax);
  assembler->pop(r11);
  assembler->pop(r9);
  assembler->pop(r8);
  assembler->ret();
}

void GenOSKICodeEmitter::emitSingleLoop(bitset<32> &patternBits, unsigned int numBlocks) {
  int size = b_r * b_c;
  // xorl %eax, %eax
  assembler->xor_(eax, eax);
  // xorl %ebx, %ebx
  assembler->xor_(ebx, ebx);
  //.align 16, 0x90
  assembler->align(kAlignCode, 16);
    
  //Create label for each pattern
  Label loopStart = assembler->newLabel();
  assembler->bind(loopStart);
  
  //xorps %xmm0, %xmm0
  assembler->xorps(xmm0, xmm0);
  // movslq (%r9,%rax,4), %rcx ## cols1[a]
  assembler->movsxd(rcx, ptr(r9, rax, 2));
  // movslq (%r8,%rax,4), %rdx ## rows1[a]
  assembler->movsxd(rdx, ptr(r8, rax, 2));
  
  int nz = patternBits.count(); // nz elements per pattern
  //startingMMElements simulation <row, Cols>
  map<int, vector<int> > patternLocs;
  for (int j = 0; j < size; ++j)
    if (patternBits[j] == 1)
      patternLocs[j / b_c].push_back(j % b_c);
  
  int bb = 0;
  for (auto &nz : patternLocs) {
    int row = nz.first;
    vector<int> cols = nz.second;
    vector<int>::iterator colsIt = cols.begin(), colsEnd = cols.end();
    // movsd "col*8"(%rdi,%rcx,8), %xmm0 ## v + cols1[a] + col = vv[col]
    assembler->movsd(xmm0, ptr(rdi, rcx, 3, (*colsIt++) * sizeof(double)));
    // mulsd "b*8"(%r11,%rbx,8), %xmm0      ## vv[col] * mvalues1[b + some k]
    assembler->mulsd(xmm0, ptr(r11, (bb++) * sizeof(double)));
      
    if (cols.size() > 1) {
      for (; colsIt != colsEnd; ++colsIt) {
        // movsd "col*8"(%rdi,%rcx,8), %xmm1 ## v + cols1[a] + col = vv[col]
        assembler->movsd(xmm1, ptr(rdi, rcx, 3, (*colsIt) * sizeof(double)));
        // mulsd "b*8"(%r11,%rbx, 8), %xmm1      ## vv[col] * mvalues1[b + some k]
        assembler->mulsd(xmm1, ptr(r11, (bb++) * sizeof(double)));
        // addsd %xmm1, %xmm0
        assembler->addsd(xmm0, xmm1);
      }
    }
      
    // addsd "row*8"(%rsi,%rdx,8), %xmm0
    assembler->addsd(xmm0, ptr(rsi, rdx, 3, row * 8));
    // movsd %xmm0, "row*8"(%rsi, %rdx, 8)
    assembler->movsd(ptr(rsi, rdx, 3, row * 8), xmm0);
  }
  assembler->lea(r11, ptr(r11, sizeof(double) * bb));
  // addq $1, %rax
  assembler->inc(rax);
  // cmpl $"numBlocks", %eax
  assembler->cmp(eax, numBlocks);
  // jne LBB*_*
  assembler->jne(loopStart);
  
  assembler->lea(r8, ptr(r8, sizeof(int) * numBlocks));
  assembler->lea(r9, ptr(r9, sizeof(int) * numBlocks));
}


// TODO: GenOski requires extra b_r and b_c params. We don't have support for this in registry yet, therefore we define
// TODO: GenOSKI33 and GenOSKI44 as separate methods

REGISTER_METHOD(GenOSKI33, "genoski33")
REGISTER_METHOD(GenOSKI44, "genoski44")

GenOSKI33::GenOSKI33()
    : GenOSKI(3,3) {

}

GenOSKI44::GenOSKI44()
    : GenOSKI(4,4){

}
