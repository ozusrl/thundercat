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

using namespace spMVgen;
using namespace std;
using namespace llvm;

extern unsigned int NUM_OF_THREADS;

class CSRWithGOTOCodeEmitter : SpMVCodeEmitter {
public:
  CSRWithGOTOCodeEmitter(unsigned long maxRowLength, unsigned long baseValsIndex, unsigned long baseRowsIndex,
                         llvm::MCStreamer *Str, unsigned int partitionIndex);
  
  void emit();

protected:
  virtual void dumpPushPopHeader();
  virtual void dumpPushPopFooter();
  
private:
  unsigned long maxRowLength;
  unsigned long baseValsIndex, baseRowsIndex;
};


CSRWithGOTO::CSRWithGOTO(Matrix *csrMatrix):
SpMVMethod(csrMatrix), analyzer(csrMatrix) {
}

Matrix* CSRWithGOTO::getMatrixForGeneration() {
  START_OPTIONAL_TIME_PROFILE(getMaxRowLengthsInfo);
  vector<unsigned long> *maxRowLengths = analyzer.getMaxRowLengths();
  END_OPTIONAL_TIME_PROFILE(getMaxRowLengthsInfo);
  
  START_OPTIONAL_TIME_PROFILE(matrixConversion);
  // Do nothing. We use the original CSR format
  END_OPTIONAL_TIME_PROFILE(matrixConversion);

  return csrMatrix;
}

void CSRWithGOTO::dumpAssemblyText() {
  START_OPTIONAL_TIME_PROFILE(getMatrix);
  this->getMatrix(); // this is done to measure the cost of matrix prep.
  vector<unsigned long> *maxRowLengths = analyzer.getMaxRowLengths();
  vector<MatrixStripeInfo> &stripeInfos = csrMatrix->getStripeInfos();
  END_OPTIONAL_TIME_PROFILE(getMatrix);

  START_OPTIONAL_TIME_PROFILE(emitCode);
  // Set up code emitters
  vector<CSRWithGOTOCodeEmitter> codeEmitters;
  for (unsigned i = 0; i < maxRowLengths->size(); i++) {
    unsigned long maxRowLength = maxRowLengths->at(i);
    codeEmitters.push_back(CSRWithGOTOCodeEmitter(maxRowLength, stripeInfos[i].valIndexBegin, stripeInfos[i].rowIndexBegin, Str, i));
  }
  
#pragma omp parallel for
  for (int threadIndex = 0; threadIndex < NUM_OF_THREADS; ++threadIndex) {
    codeEmitters[threadIndex].emit();
  }
  END_OPTIONAL_TIME_PROFILE(emitCode);
}

CSRWithGOTOCodeEmitter::CSRWithGOTOCodeEmitter(unsigned long maxRowLength, unsigned long baseValsIndex, unsigned long baseRowsIndex,
                                               llvm::MCStreamer *Str, unsigned int partitionIndex):
maxRowLength(maxRowLength), baseValsIndex(baseValsIndex), baseRowsIndex(baseRowsIndex) {
  this->DFOS = createNewDFOS(Str, partitionIndex);
}


void CSRWithGOTOCodeEmitter::emit() {
  dumpPushPopHeader();
  
  // xorl %eax, %eax
  emitXOR32rrInst(X86::EAX, X86::EAX);
  
  // leaq "4*numPreviousRows"(%r11), %r11
  emitLEAQInst(X86::R11, X86::R11, 2 * (int)baseRowsIndex * sizeof(int));
  // leaq "numPreviousElements"(%rax), %rax
  emitLEAQInst(X86::RAX, X86::RAX, (int)baseValsIndex);
  // xorps %xmm0, %xmm0
  emitRegInst(X86::XORPSrr, 0, 0);
  
  //.align 16, 0x90
  emitCodeAlignment(16);
  
  for (int i = 0; i < maxRowLength; ++i) {
    // movslq (%r9,%rax,4), %rbx ## cols[k]
    emitMOVSLQInst(X86::RBX, X86::R9, X86::RAX, 4, 0);
    // movsd (%rdi,%rbx,8), %xmm1 ## v[cols[k]]
    emitMOVSDrmInst(0, X86::RDI, X86::RBX, 8, 1);
    // mulsd (%r8,%rax,8), %xmm1 ## ...  *  vals[k]
    emitMULSDrmInst(0, X86::R8, X86::RAX, 8, 1);
    // addsd %xmm1, %xmm0
    emitRegInst(X86::ADDSDrr, 1, 0);
    // addq $"1", %rax
    emitADDQInst(1 , X86::RAX);
  }
  
  emitLEAQ_RIP(X86::RDX, 0);
  
  // Move the next row index to rcx
  // movslq (%r11), %rcx
  emitMOVSLQInst(X86::RCX, X86::R11, 0);
  // Move the num bytes to jump into r10
  // movslq "sizeof(int)"(%r11), %r10
  emitMOVSLQInst(X86::R10, X86::R11, sizeof(int));
  // Add to w[r]
  //addsd (%rsi,%rcx,8), %xmm0
  emitADDSDrmInst(0, X86::RSI, X86::RCX, 8, 0);
  //movsd %xmm0, (%rsi,%rcx,8)
  emitMOVSDmrInst(0, 0, X86::RSI, X86::RCX, 8);
  // xorps %xmm0, %xmm0
  emitRegInst(X86::XORPSrr, 0, 0);
  // leaq "2*sizeof(int)"(%r11), %r11
  emitLEAQInst(X86::R11, X86::R11, 2*sizeof(int));
  // leaq (%rdx,%r10), %rdx
  emitLEAQInst(X86::RDX, X86::RDX, X86::R10, 0);
  // jmp *%rdx
  emitDynamicJMPInst(X86::RDX);
  
  dumpPushPopFooter();
}

void CSRWithGOTOCodeEmitter::dumpPushPopHeader() {
  // rows is in %rdx, cols is in %rcx, vals is in %r8
  emitPushPopInst(X86::PUSH64r,X86::R9);
  emitPushPopInst(X86::PUSH64r,X86::R10);
  emitPushPopInst(X86::PUSH64r,X86::R11);
  
  emitLEAQInst(X86::RCX, X86::R9, 0); // using %r9 for cols
  emitLEAQInst(X86::RDX, X86::R11, 0); // using %r11 for rows
  
  emitPushPopInst(X86::PUSH64r,X86::RAX);
  emitPushPopInst(X86::PUSH64r,X86::RCX);
  emitPushPopInst(X86::PUSH64r,X86::RDX);
  emitPushPopInst(X86::PUSH64r,X86::RBX);
}

void CSRWithGOTOCodeEmitter::dumpPushPopFooter() {
  emitPushPopInst(X86::POP64r,X86::RBX);
  emitPushPopInst(X86::POP64r,X86::RDX);
  emitPushPopInst(X86::POP64r,X86::RCX);
  emitPushPopInst(X86::POP64r,X86::RAX);
  emitPushPopInst(X86::POP64r,X86::R11);
  emitPushPopInst(X86::POP64r,X86::R10);
  emitPushPopInst(X86::POP64r,X86::R9);
  emitRETInst(); // We shouldn't have had to do this here. Strange...
}


