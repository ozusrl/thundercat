#include "method.h"
#include "profiler.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include "lib/Target/X86/MCTargetDesc/X86BaseInfo.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCInstBuilder.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/MCObjectFileInfo.h"

using namespace spMVgen;
using namespace std;
using namespace llvm;

extern unsigned int NUM_OF_THREADS;

class GenOSKICodeEmitter : public SpMVCodeEmitter {
private:
  GroupByBlockPatternMap *patternMap;
  unsigned int b_r, b_c;
  unsigned long baseValsIndex, baseBlockIndex;
  
  void dumpForLoops();
  
protected:
  virtual void dumpPushPopHeader();
  virtual void dumpPushPopFooter();
  
public:
  GenOSKICodeEmitter(GroupByBlockPatternMap *patternMap,
                     unsigned long baseValsIndex,
                     unsigned long baseBlockIndex,
                     unsigned int b_r,
                     unsigned int b_c,
                     llvm::MCStreamer *Str, unsigned int partitionIndex);
  void emit();
};


GenOSKI::GenOSKI(Matrix *csrMatrix, unsigned b_r, unsigned b_c):
  SpMVMethod(csrMatrix), b_r(b_r), b_c(b_c), analyzer(csrMatrix, b_r, b_c)
{
}

Matrix* GenOSKI::getMatrixForGeneration() {
  START_OPTIONAL_TIME_PROFILE(getGenOSKIInfo);
  vector<GroupByBlockPatternMap> *blockPatternLists = analyzer.getGroupByBlockPatternMaps();
  END_OPTIONAL_TIME_PROFILE(getGenOSKIInfo);

  START_OPTIONAL_TIME_PROFILE(matrixConversion);
  vector<unsigned int> &numBlocks = analyzer.getNumBlocks();
  unsigned int numTotalBlocks = 0;
  vector<unsigned int> blockBaseIndices;
  for (auto n : numBlocks) {
    blockBaseIndices.push_back(numTotalBlocks);
    numTotalBlocks += n;
  }
  
  int *rows = new int[numTotalBlocks];
  int *cols = new int[numTotalBlocks];
  double *vals = new double[csrMatrix->nz];
  
  vector<MatrixStripeInfo> &stripeInfos = csrMatrix->getStripeInfos();
  
#pragma omp parallel for
  for (int t = 0; t < NUM_OF_THREADS; ++t) {
    auto &blockPatterns = blockPatternLists->at(t);
    int *rowsPtr = rows + blockBaseIndices[t];
    int *colsPtr = cols + blockBaseIndices[t];
    double *valsPtr = vals + stripeInfos[t].valIndexBegin;
    
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
  END_OPTIONAL_TIME_PROFILE(matrixConversion);

  Matrix *result = new Matrix(rows, cols, vals, csrMatrix->n, csrMatrix->nz);
  result->numRows = numTotalBlocks;
  result->numCols = numTotalBlocks;
  result->numVals = csrMatrix->nz;
  return result;
}

void GenOSKI::dumpAssemblyText() {
  START_OPTIONAL_TIME_PROFILE(getMatrix);
  this->getMatrix(); // Only for benchmarking purposes.
  vector<GroupByBlockPatternMap> *patternMaps = analyzer.getGroupByBlockPatternMaps();
  vector<MatrixStripeInfo> &stripeInfos = csrMatrix->getStripeInfos();
  END_OPTIONAL_TIME_PROFILE(getMatrix);

  START_OPTIONAL_TIME_PROFILE(emitCode);
  vector<unsigned int> &numBlocks = analyzer.getNumBlocks();
  unsigned int numTotalBlocks = 0;
  vector<unsigned int> blockBaseIndices;
  for (auto n : numBlocks) {
    blockBaseIndices.push_back(numTotalBlocks);
    numTotalBlocks += n;
  }
  
  // Set up code emitters
  vector<GenOSKICodeEmitter> codeEmitters;
  for (unsigned i = 0; i < patternMaps->size(); i++) {
    auto &patternMap = patternMaps->at(i);
    codeEmitters.push_back(GenOSKICodeEmitter(&patternMap, stripeInfos[i].valIndexBegin, blockBaseIndices[i], b_r, b_c, Str, i));
  }
  
#pragma omp parallel for
  for (int threadIndex = 0; threadIndex < NUM_OF_THREADS; ++threadIndex) {
    codeEmitters[threadIndex].emit();
  }
  END_OPTIONAL_TIME_PROFILE(emitCode);
}

GenOSKICodeEmitter::GenOSKICodeEmitter(GroupByBlockPatternMap *patternMap,
                                       unsigned long baseValsIndex,
                                       unsigned long baseBlockIndex,
                                       unsigned int b_r,
                                       unsigned int b_c,
                                       llvm::MCStreamer *Str, unsigned int partitionIndex):
patternMap(patternMap), baseValsIndex(baseValsIndex), baseBlockIndex(baseBlockIndex), b_r(b_r), b_c(b_c) {
  this->DFOS = createNewDFOS(Str, partitionIndex);
}

void GenOSKICodeEmitter::emit() {
  dumpPushPopHeader();
  
  dumpForLoops();
  
  dumpPushPopFooter();
  emitRETInst();
}

void GenOSKICodeEmitter::dumpPushPopHeader() {
  // rows is in %rdx, cols is in %rcx, vals is in %r8
  emitPushPopInst(X86::PUSH64r,X86::R11);
  emitLEAQInst(X86::R8, X86::R11, (int)(sizeof(double) * baseValsIndex)); // using %r11 for vals
  
  emitPushPopInst(X86::PUSH64r,X86::R8);
  emitLEAQInst(X86::RDX, X86::R8, (int)(sizeof(int) * baseBlockIndex)); // using %r8 for rows
  
  emitPushPopInst(X86::PUSH64r,X86::R9);
  emitLEAQInst(X86::RCX, X86::R9, (int)(sizeof(int) * baseBlockIndex)); // using %r9 for cols
  
  emitPushPopInst(X86::PUSH64r,X86::RAX);
  emitPushPopInst(X86::PUSH64r,X86::RCX);
  emitPushPopInst(X86::PUSH64r,X86::RDX);
  emitPushPopInst(X86::PUSH64r,X86::RBX);
}

void GenOSKICodeEmitter::dumpPushPopFooter() {
  emitPushPopInst(X86::POP64r,X86::RBX);
  emitPushPopInst(X86::POP64r,X86::RDX);
  emitPushPopInst(X86::POP64r,X86::RCX);
  emitPushPopInst(X86::POP64r,X86::RAX);
  emitPushPopInst(X86::POP64r,X86::R11);
  emitPushPopInst(X86::POP64r,X86::R9);
  emitPushPopInst(X86::POP64r,X86::R8);
}

void GenOSKICodeEmitter::dumpForLoops() {
  // xorl %ecx, %ecx
  emitXOR32rrInst(X86::ECX, X86::ECX);
  
  int size = b_r * b_c;
  for (auto &pattern : *patternMap) {
    // xorl %eax, %eax
    emitXOR32rrInst(X86::EAX, X86::EAX);
    // xorl %ebx, %ebx
    emitXOR32rrInst(X86::EBX, X86::EBX);
    //.align 16, 0x90
    emitCodeAlignment(16);
    
    //Create label for each pattern
    unsigned long labeledBlockBeginningOffset = DFOS->size();
    
    //xorps %xmm0, %xmm0
    emitRegInst(X86::XORPSrr, 0, 0);
    
    // movslq (%r9,%rax,4), %rcx ## cols1[a]
    emitMOVSLQInst(X86::RCX, X86::R9, X86::RAX, 4, 0);
    // movslq (%r8,%rax,4), %rdx ## rows1[a]
    emitMOVSLQInst(X86::RDX, X86::R8, X86::RAX, 4, 0);
    
    
    bitset<32> patternBits(pattern.first);
    unsigned int numBlocks = pattern.second.first.size();
    int nz = patternBits.count(); // nz elements per pattern
    
    //startingMMElements simulation <row, Cols>
    map<int, vector<int> > patternLocs;
    for (int j = 0; j < size; ++j) {
      if (patternBits[j] == 1) {
        patternLocs[j / b_c].push_back(j % b_c);
      }
    }
    
    int bb = 0;
    for (auto &nz : patternLocs) {
      int row = nz.first;
      vector<int> cols = nz.second;
      vector<int>::iterator colsIt = cols.begin(), colsEnd = cols.end();
      // movsd "col*8"(%rdi,%rcx,8), %xmm0 ## v + cols1[a] + col = vv[col]
      emitMOVSDrmInst((*colsIt++)*sizeof(double), X86::RDI, X86::RCX, 8, 0);
      // mulsd "b*8"(%r11,%rbx,8), %xmm0      ## vv[col] * mvalues1[b + some k]
      emitMULSDrmInst((bb++)*sizeof(double), X86::R11, 0);
      
      if (cols.size() > 1) {
        for (; colsIt != colsEnd; ++colsIt) {
          // movsd "col*8"(%rdi,%rcx,8), %xmm1 ## v + cols1[a] + col = vv[col]
          emitMOVSDrmInst((*colsIt)*sizeof(double), X86::RDI, X86::RCX, 8, 1);
          // mulsd "b*8"(%r11,%rbx, 8), %xmm1      ## vv[col] * mvalues1[b + some k]
          emitMULSDrmInst((bb++)*sizeof(double), X86::R11, 1);
          // addsd %xmm1, %xmm0
          emitRegInst(X86::ADDSDrr, 1, 0);
        }
      }
      
      // addsd "row*8"(%rsi,%rdx,8), %xmm0
      emitADDSDrmInst(row*8, X86::RSI, X86::RDX, 8, 0);
      // movsd %xmm0, "row*8"(%rsi, %rdx, 8)
      emitMOVSDmrInst(0, row*8, X86::RSI, X86::RDX, 8);
    }
    emitLEAQInst(X86::R11, X86::R11, sizeof(double)*bb);
    
    // addq $1, %rax
    emitADDQInst(1, X86::RAX);
    // cmpl $"numBlocks", %eax
    emitCMP32riInst(X86::EAX, numBlocks);
    // jne LBB*_*
    emitJNEInst(labeledBlockBeginningOffset);
    
    emitLEAQInst(X86::R8, X86::R8, sizeof(int)*numBlocks);
    emitLEAQInst(X86::R9, X86::R9, sizeof(int)*numBlocks);
  }
}

