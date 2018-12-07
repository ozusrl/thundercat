#include "specializers.h"
#include "spmvRegistry.h"
#include <iostream>

using namespace thundercat;
using namespace std;
using namespace asmjit;
using namespace x86;



REGISTER_METHOD(RowPattern, "rowpattern")
///
/// Analysis
///
void RowPattern::analyzeMatrix() {
  patternInfos.resize(stripeInfos->size());
  
#pragma omp parallel for
  for (int threadIndex = 0; threadIndex < stripeInfos->size(); ++threadIndex) {
    auto &stripeInfo = stripeInfos->at(threadIndex);
    for (unsigned long rowIndex = stripeInfo.rowIndexBegin; rowIndex < stripeInfo.rowIndexEnd; ++rowIndex) {
      int rowStart = csrMatrix->rowPtr[rowIndex];
      int rowEnd = csrMatrix->rowPtr[rowIndex+1];
      int rowLength = rowEnd - rowStart;
      
      if (rowLength > 0) {
        vector<int> pattern;
        for (int k = rowStart; k < rowEnd; ++k) {
          pattern.push_back(csrMatrix->colIndices[k] - (int)rowIndex);
        }
        patternInfos[threadIndex][pattern].push_back((int)rowIndex);
      }
    }
  }
}

///
/// RowPattern
///
void RowPattern::convertMatrix() {
  double *vals = new double[csrMatrix->NZ];
  int *rows = new int[csrMatrix->N];
  
#pragma omp parallel for
  for (unsigned int t = 0; t < stripeInfos->size(); ++t) {
    auto &stencils = patternInfos.at(t);
    double *valPtr = vals + stripeInfos->at(t).valIndexBegin;
    int *rowPtr = rows + stripeInfos->at(t).rowIndexBegin;
    
    for (auto &stencilInfo : stencils) {
      // build vals array
      for (auto rowIndex: stencilInfo.second) {
        for (int k = csrMatrix->rowPtr[rowIndex]; k < csrMatrix->rowPtr[rowIndex+1]; ++k) {
          *valPtr++ = csrMatrix->values[k];
        }
      }
      // build rows array
      if (stencilInfo.second.size() > 1) {
        for (auto rowIndex: stencilInfo.second) {
          *rowPtr++ = rowIndex;
        }
      }
    }
  }
  
  matrix = std::make_unique<CSRMatrix<VALUE_TYPE>>(rows, (int *) NULL, vals, csrMatrix->N, csrMatrix->M, csrMatrix->NZ);
}

///
/// RowPatternCodeEmitter:
/// Helper class to avoid having to pass several parameters
///
class RowPatternCodeEmitter {
public:
  RowPatternCodeEmitter(X86Assembler *assembler,
                        RowPatternInfo *patternInfo,
                        unsigned long baseValsIndex,
                        unsigned long baseRowsIndex) {
    this->assembler = assembler;
    this->patternInfo = patternInfo;
    this->baseValsIndex = baseValsIndex;
    this->baseRowsIndex = baseRowsIndex;
  }
  
  void emit();
  
private:
  X86Assembler *assembler;
  RowPatternInfo *patternInfo;
  unsigned long baseValsIndex;
  unsigned long baseRowsIndex;
  
  void emitHeader();
  
  void emitFooter();
  
  void emitSingleLoop(const vector<int> &pattern, const vector<int> &rowIndices);
};

void RowPattern::emitMultByMFunction(unsigned int index) {
  X86Assembler assembler(codeHolders[index]);
  RowPatternInfo &patternInfo = patternInfos.at(index);
  RowPatternCodeEmitter emitter(&assembler,
                                &patternInfo,
                                stripeInfos->at(index).valIndexBegin,
                                stripeInfos->at(index).rowIndexBegin);
  emitter.emit();
}

void RowPatternCodeEmitter::emit() {
  emitHeader();
  
  for (auto &info : *patternInfo) {
    emitSingleLoop(info.first, info.second);
  }
  
  emitFooter();
}

void RowPatternCodeEmitter::emitHeader() {
  // rows is in %rdx, cols is in %rcx, vals is in %r8
  assembler->push(rbx);
  assembler->lea(rbx, ptr(r8, sizeof(double) * baseValsIndex)); // using %rbx for vals
  assembler->push(r8);
  assembler->lea(r8, ptr(rdx, sizeof(int) * baseRowsIndex)); // using %r8 for rows
  assembler->push(r11);
  assembler->push(rdx);
}

void RowPatternCodeEmitter::emitFooter() {
  assembler->pop(rdx);
  assembler->pop(r11);
  assembler->pop(r8);
  assembler->pop(rbx);
  assembler->ret();
}

void RowPatternCodeEmitter::emitSingleLoop(const vector<int> &pattern,
                                           const vector<int> &rowIndices) {
  unsigned long popularity = rowIndices.size();
  unsigned long patternSize = pattern.size();
  if(patternSize == 0 || popularity == 0) return;
  
  int row = rowIndices[0];
  Label loopStart;
  
  if (popularity == 1) {
    //  movsd  "8*(row+stencil[0])"(%rdi), %xmm1
    assembler->movsd(xmm1, ptr(rdi, 8 * (row + pattern[0])));
  } else {
    //  xorl %r11d, %r11d
    assembler->xor_(r11d, r11d);
    //  .align 4, 0x90
    assembler->align(kAlignCode, 16);
    loopStart = assembler->newLabel();
    assembler->bind(loopStart);
    
    //  movslq (%r11,%r8), %rdx
    assembler->movsxd(rdx, ptr(r11, r8));
    //  movsd "8*(stencil[0])"(%rdi,%rdx,8), %xmm1
    assembler->movsd(xmm1, ptr(rdi, rdx, 3, 8 * pattern[0]));
  }
  
  //  mulsd (%RBX), %xmm1
  assembler->mulsd(xmm1, ptr(rbx));
  
  for(int i = 1; i < patternSize; ++i) {
    if (popularity == 1) {
      //  movsd "8*(row+stencil[i])"(%rdi), %xmm0
      assembler->movsd(xmm0, ptr(rdi, 8 * (row + pattern[i])));
    } else {
      //  movsd "8*(stencil[i])"(%rdi,%rdx,8), %xmm0
      assembler->movsd(xmm0, ptr(rdi, rdx, 3, 8 * pattern[i]));
    }
    //  mulsd "8*(i)"(%RBX), %xmm0
    assembler->mulsd(xmm0, ptr(rbx, 8 * i));
    //  addsd %xmm0, %xmm1
    assembler->addsd(xmm1, xmm0);
  }
  
  if (popularity > 1) {
    //  addsd (%rsi,%rdx,8), %xmm1
    assembler->addsd(xmm1, ptr(rsi, rdx, 3, 0));
    //  addq $"sizeof(int)", %r11
    assembler->add(r11, (int)sizeof(int));
    //  movsd %xmm1, (%rsi,%rdx,8)
    assembler->movsd(ptr(rsi, rdx, 3, 0), xmm1);
  }
  //  leaq "sizeof(double)*stencilSize"(%RBX), %RBX
  assembler->lea(rbx, ptr(rbx, (sizeof(double) * patternSize)));
  
  if (popularity == 1) {
    //  addsd "sizeof(double)*row"(%rsi), %xmm1
    assembler->addsd(xmm1, ptr(rsi, sizeof(double) * row));
    //  movsd %xmm1, "sizeof(double)*row"(%rsi)
    assembler->movsd(ptr(rsi, sizeof(double) * row), xmm1);
  } else {
    //  cmpl $"popularity*sizeof(int)", %r11d
    assembler->cmp(r11d, (int)(popularity * sizeof(int)));
    //  jne LBB_"row"
    assembler->jne(loopStart);
    //  leaq "sizeof(int)*popularity"(%r8), %r8
    assembler->lea(r8, ptr(r8, sizeof(int) * popularity));
  }
}


