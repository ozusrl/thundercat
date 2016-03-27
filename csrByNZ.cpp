#include "method.h"
#include "profiler.h"
#include "csrByNZAnalyzer.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include "lib/Target/X86/MCTargetDesc/X86BaseInfo.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCInstBuilder.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/MCObjectFileInfo.h"
#include "llvm/Target/TargetLoweringObjectFile.h"

#define SPMV

using namespace llvm;
using namespace spMVgen;
using namespace std;

extern unsigned int NUM_OF_THREADS;

class CSRbyNZCodeEmitter : public SpMVCodeEmitter {
private:
  NZtoRowMap *rowByNZs;
  unsigned long baseValsIndex, baseRowsIndex;
  
  void dumpSingleLoop(unsigned long numRows, unsigned long rowLength);
  
protected:
  virtual void dumpPushPopHeader();
  virtual void dumpPushPopFooter();
  
public:
  CSRbyNZCodeEmitter(NZtoRowMap *rowByNZs, unsigned long baseValsIndex,
                     unsigned long baseRowsIndex, llvm::MCStreamer *Str, unsigned int partitionIndex);
  
  void emit();
};


CSRbyNZ::CSRbyNZ(Matrix *csrMatrix):
  SpMVMethod(csrMatrix), analyzer(csrMatrix) {
  // do nothing
}

// Return a matrix to be used by CSRbyNZ
// rows: row indices, sorted by row lengths
// cols: indices of elements,
//       sorted according to the order used in rows array
// vals: values as usual,
//       sorted according to the order used in rows array
Matrix* CSRbyNZ::getMatrixForGeneration() {
  START_OPTIONAL_TIME_PROFILE(getCSRbyNZInfo);
  vector<NZtoRowMap> *rowByNZLists = analyzer.getRowByNZLists();
  END_OPTIONAL_TIME_PROFILE(getCSRbyNZInfo);
  
  START_OPTIONAL_TIME_PROFILE(matrixConversion);
  int *rows = new int[csrMatrix->n];
  int *cols = new int[csrMatrix->nz];
  double *vals = new double[csrMatrix->nz];

  vector<MatrixStripeInfo> &stripeInfos = csrMatrix->getStripeInfos();

#pragma omp parallel for
  for (int t = 0; t < NUM_OF_THREADS; ++t) {
    auto &rowByNZList = rowByNZLists->at(t);
    int *rowsPtr = rows + stripeInfos[t].rowIndexBegin;
    int *colsPtr = cols + stripeInfos[t].valIndexBegin;
    double *valsPtr = vals + stripeInfos[t].valIndexBegin;
    
    for (auto &rowByNZ : rowByNZList) {
      unsigned long rowLength = rowByNZ.first;
      for (int rowIndex : *(rowByNZ.second.getRowIndices())) {
        *rowsPtr++ = rowIndex;
        int k = csrMatrix->rows[rowIndex];
        for (int i = 0; i < rowLength; i++, k++) {
          *colsPtr++ = csrMatrix->cols[k];
          *valsPtr++ = csrMatrix->vals[k];
        }
      }
    }
  }
  END_OPTIONAL_TIME_PROFILE(matrixConversion);

  return new Matrix(rows, cols, vals, csrMatrix->n, csrMatrix->nz);
}

void CSRbyNZ::dumpAssemblyText() {
  START_OPTIONAL_TIME_PROFILE(getMatrix);
  this->getMatrix(); // this is done to measure the cost of matrix prep.
  vector<NZtoRowMap> *rowByNZLists = analyzer.getRowByNZLists(); // this has zero cost, because already calculated by getMatrix()
  vector<MatrixStripeInfo> &stripeInfos = csrMatrix->getStripeInfos();
  END_OPTIONAL_TIME_PROFILE(getMatrix);

  START_OPTIONAL_TIME_PROFILE(emitCode);
  // Set up code emitters
  vector<CSRbyNZCodeEmitter> codeEmitters;
  for (unsigned i = 0; i < rowByNZLists->size(); i++) {
    NZtoRowMap &rowByNZs = rowByNZLists->at(i);
    codeEmitters.push_back(CSRbyNZCodeEmitter(&rowByNZs, stripeInfos[i].valIndexBegin, stripeInfos[i].rowIndexBegin, Str, i));
  }
  
#pragma omp parallel for
  for (int threadIndex = 0; threadIndex < NUM_OF_THREADS; ++threadIndex) {
    codeEmitters[threadIndex].emit();
  }
  END_OPTIONAL_TIME_PROFILE(emitCode);
}


CSRbyNZCodeEmitter::CSRbyNZCodeEmitter(NZtoRowMap *rowByNZs, unsigned long baseValsIndex,
                                       unsigned long baseRowsIndex, llvm::MCStreamer *Str, unsigned int partitionIndex):
rowByNZs(rowByNZs), baseValsIndex(baseValsIndex), baseRowsIndex(baseRowsIndex) {
  this->DFOS = createNewDFOS(Str, partitionIndex);
}

void CSRbyNZCodeEmitter::emit() {
  dumpPushPopHeader();
  
  for (auto &rowByNZ : *rowByNZs) {
    unsigned long rowLength = rowByNZ.first;
    dumpSingleLoop(rowByNZ.second.getRowIndices()->size(), rowLength);
  }
  
  dumpPushPopFooter();
  emitRETInst();
}
  
void CSRbyNZCodeEmitter::dumpPushPopHeader() {
  // rows is in %rdx, cols is in %rcx, vals is in %r8
  emitPushPopInst(X86::PUSH64r,X86::R8);
  emitPushPopInst(X86::PUSH64r,X86::R9);
  emitPushPopInst(X86::PUSH64r,X86::R10);
  emitPushPopInst(X86::PUSH64r,X86::R11);
  emitPushPopInst(X86::PUSH64r,X86::RAX);
  emitPushPopInst(X86::PUSH64r,X86::RBX);
  emitPushPopInst(X86::PUSH64r,X86::RCX);
  emitPushPopInst(X86::PUSH64r,X86::RDX);
  
  emitLEAQInst(X86::RDX, X86::RDX, (int)(sizeof(int) * baseRowsIndex));
  emitLEAQInst(X86::RCX, X86::RCX, (int)(sizeof(int) * baseValsIndex));
  emitLEAQInst(X86::R8, X86::R8, (int)(sizeof(double) * baseValsIndex));
  // movq %r8, -8(%rbp)    <--- vals
  // movq %rcx, -16(%rbp)  <--- cols
  // movq %rdx, -24(%rbp)  <--- rows
}

void CSRbyNZCodeEmitter::dumpPushPopFooter() {
  emitPushPopInst(X86::POP64r, X86::RDX);
  emitPushPopInst(X86::POP64r, X86::RCX);
  emitPushPopInst(X86::POP64r, X86::RBX);
  emitPushPopInst(X86::POP64r, X86::RAX);
  emitPushPopInst(X86::POP64r, X86::R11);
  emitPushPopInst(X86::POP64r, X86::R10);
  emitPushPopInst(X86::POP64r, X86::R9);
  emitPushPopInst(X86::POP64r, X86::R8);
}

void CSRbyNZCodeEmitter::dumpSingleLoop(unsigned long numRows, unsigned long rowLength) {
  unsigned long labeledBlockBeginningOffset = 0;
  
  // xorl %r9d, %r9d
  emitXOR32rrInst(X86::R9D, X86::R9D);
  // xorl %ebx, %ebx
  emitXOR32rrInst(X86::EBX, X86::EBX);
  
  //.align 16, 0x90
  emitCodeAlignment(16);
  //.LBB0_1:
  labeledBlockBeginningOffset = DFOS->size();
  
  //xorps %xmm0, %xmm0
  emitRegInst(X86::XORPSrr, 0, 0);
#ifdef TEMPO_O0  
  //movslq -44(%rbp), %rcx <---- %rcx = a
  //movq -24(%rbp), %rdx   <---- %rdx = rows
  //movl (%rdx,%rcx,4), %eax 
  //movl %eax, -52(%rbp)   <---- row = rows[a]
  //movsd %xmm0, -64(%rbp) <---- sum
  emitMOVSDmrInst(0, -64, X86::RBP);
#endif

  // done for a single row
  for(int i = 0 ; i < rowLength ; i++){
#ifdef TEMPO_O1
    //movq "i", %rax
    emitMovImm(i, X86::RAX);
    //addq %r9, %rax
    emitADDrrInst(X86::R9, X86::RAX);
    //movsd (%r8,%rax,8), %xmm1
    emitMOVSDrmInst(0, X86::R8, X86::RAX, 8, 1);
    //movq "i", %rax
    emitMovImm(i, X86::RAX);
    //addq %r9, %rax
    emitADDrrInst(X86::R9, X86::RAX);
    //movslq (%rcx,%rax,4), %rax
    emitMOVSLQInst(X86::RAX, X86::RCX, X86::RAX, 4, 0);
    //mulsd (%rdi,%rax,8), %xmm1
    emitMULSDrmInst(0, X86::RDI, X86::RAX, 8, 1);
    //addsd %xmm1, %xmm0
    emitRegInst(X86::ADDSDrr, 1, 0);
#endif
#ifdef TEMPO_O0
    //movq "i", %rax
    emitMovImm(i, X86::RAX);
    //movl %eax, %ecx (89 c1)
    emitMovlInst(X86::EAX, X86::ECX);
    //movq "i", %rax
    emitMovImm(i, X86::RAX);
    //movl %eax, %edx (89 c2)
    emitMovlInst(X86::EAX, X86::EDX);
    //movsd -64(%rbp), %xmm0
    emitMOVSDrmInst(-64, X86::RBP, 0);
    //addl -48(%rbp), %edx
    emitAddlInst(-48, X86::RBP, X86::EDX);
    //movslq %edx, %rax
    //movq -8(%rbp), %rsi
    //movsd   (%rsi,%rax,8), %xmm1
    //addl    -48(%rbp), %ecx
    //movslq  %ecx, %rax
    //movq    -16(%rbp), %rsi
    //movslq  (%rsi,%rax,4), %rax
    //movq    -32(%rbp), %rsi
    //mulsd   (%rsi,%rax,8), %xmm1
    //addsd %xmm1, %xmm0
    emitRegInst(X86::ADDSDrr, 1, 0);
    //movsd   %xmm0, -64(%rbp)   
    emitMOVSDmrInst(0, -64, X86::RBP);
#endif
#ifdef SPMV
    //movslq "i*4"(%rcx,%r9,4), %rax
    emitMOVSLQInst(X86::RAX, X86::RCX, X86::R9, 4, i*4);
    //movsd "i*8"(%r8,%r9,8), %xmm1
    emitMOVSDrmInst(i*8, X86::R8, X86::R9, 8, 1);
    //mulsd (%rdi,%rax,8), %xmm1
    emitMULSDrmInst(0, X86::RDI, X86::RAX, 8, 1);
    //addsd %xmm1, %xmm0
    emitRegInst(X86::ADDSDrr, 1, 0);
#endif
  }

#ifdef TEMPO_O0
  //movslq -52(%rbp), %rcx
  //movq -40(%rbp), %rdx
  //addsd (%rdx,%rcx,8), %xmm0
  //movsd %xmm0, (%rdx,%rcx,8) <--- w[row] = sum
  //movl -44(%rbp), %eax
  //addl $1, %eax         
  //movl %eax, -44(%rbp)       <--- a+=1
  //movl -48(%rbp), %eax
  //addl rowLength, %eax
  //movl %eax, -48(%rbp)       <--- b+=length
  //cmpl lengthEnding, -44(%rbp)
  //jne .LBB0_1
  emitJNEInst(labeledBlockBeginningOffset);
else
  // movslq (%rdx,%rbx,4), %rax
  emitMOVSLQInst(X86::RAX, X86::RDX, X86::RBX, 4, 0);
  
  //addq $rowLength, %r9
  emitADDQInst(rowLength, X86::R9);
  
  //addq $1, %rbx
  emitADDQInst(1, X86::RBX);
  
  //addsd (%rsi,%rax,8), %xmm0
  emitADDSDrmInst(0, X86::RSI, X86::RAX, 8, 0);
  
  //cmpl numRows, %ebx
  emitCMP32riInst(X86::EBX, numRows);
  
  //movsd %xmm0, (%rsi,%rax,8)
  emitMOVSDmrInst(0, 0, X86::RSI, X86::RAX, 8);
  //jne .LBB0_1
  emitJNEInst(labeledBlockBeginningOffset);
  
  //addq $numRows*4, %rdx
  emitADDQInst(numRows*4, X86::RDX);
  
  //addq $numRows*rowLength*4, %rcx
  emitADDQInst(numRows*rowLength*4, X86::RCX);
  //addq $numRows*rowLength*8, %r8
  emitADDQInst(numRows*rowLength*8, X86::R8);
#endif
}
