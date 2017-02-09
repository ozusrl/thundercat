#include "method.h"
#include <iostream>
#include <sstream>
#if defined(__linux__) && defined(__x86_64__)
#include "lib/Target/X86/MCTargetDesc/X86BaseInfo.h"
#else
#include "lib/Target/ARM/MCTargetDesc/ARMBaseInfo.h"
#endif
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCInstBuilder.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCObjectFileInfo.h"

extern unsigned int NUM_OF_THREADS;

using namespace spMVgen;
using namespace llvm;

SpMVMethod::SpMVMethod(Matrix *csrMatrix) {
  this->matrix = NULL;
  this->csrMatrix = csrMatrix;
}

SpMVMethod::~SpMVMethod() {

}

Matrix* SpMVMethod::getMatrix() {
  if (matrix == NULL) {
    matrix = getMatrixForGeneration();
  }
  return matrix;
}

Matrix *SpMVMethod::getCSRMatrix() {
  return csrMatrix;
}

bool SpMVMethod::hasNonEmptyMatrix() {
  return csrMatrix->nz != 0;
}

void SpMVMethod::setMCStreamer(llvm::MCStreamer *Str) {
  this->Str = Str;
}

void SpMVMethod::dumpMultByMFunctions() {
  dumpAssemblyText();
//printf("%s %s END\n",__FILE__,__FUNCTION__);
}

llvm::SmallVectorImpl<char> *SpMVCodeEmitter::createNewDFOS(MCStreamer *Str, unsigned int index) {
  dumpMultByMHeader(Str, index);
  MCObjectStreamer *objstr = (MCObjectStreamer*)Str;
  MCDataFragment *DF = objstr->createDataFragment();
  llvm::SmallVectorImpl<char> *DFOS = &(DF->getContents());
  dumpMultByMFooter(Str);
  return DFOS;
}

static std::string newName(const std::string &str, int i) {
  std::ostringstream oss;
  oss << i;
  std::string name(str);
  return name + oss.str();
}

void SpMVCodeEmitter::dumpMultByMHeader(MCStreamer *Str, unsigned int index) {
  std::string uscore = "_";

  MCContext &Ctx = Str->getContext();
  // .globl multByM"index"
  Str->EmitSymbolAttribute(Ctx.GetOrCreateSymbol(StringRef(newName(MAIN_FUNCTION_NAME, index))), MCSA_Global);
  Str->EmitSymbolAttribute(Ctx.GetOrCreateSymbol(StringRef(newName(uscore + MAIN_FUNCTION_NAME, index))), MCSA_Global);
  // .align 4, 0x90
  Str->EmitCodeAlignment(16);
  // multByM"index":
  Str->EmitLabel(Ctx.GetOrCreateSymbol(StringRef(newName(MAIN_FUNCTION_NAME, index))));
  Str->EmitLabel(Ctx.GetOrCreateSymbol(StringRef(newName(uscore + MAIN_FUNCTION_NAME, index))));
  // .cfi_startproc
  Str->EmitCFIStartProc(true);
}

void SpMVCodeEmitter::dumpMultByMFooter(MCStreamer *Str) {
  Str->EmitCFIEndProc();
}

static bool isR8R15Register(unsigned registerNumber) {
#if defined(__linux__) && defined(__x86_64__)
  return registerNumber >= X86::R8 && registerNumber <= X86::R15;
#else
return false;
#endif
}

void SpMVCodeEmitter::emitRegInst(unsigned opCode, int XMMfrom, int XMMto) {
#if defined(__linux__) && defined(__x86_64__)
  unsigned char data[5];
  unsigned char *dataPtr = data;
  if (opCode != X86::XORPSrr)
    *(dataPtr++) = 0xf2;
  if (XMMfrom >= 8 && XMMto < 8) {
    *(dataPtr++) = 0x41;
  } else if (XMMfrom < 8 && XMMto >= 8) {
    *(dataPtr++) = 0x44;
  } else if (XMMfrom >= 8 && XMMto >= 8) {
    *(dataPtr++) = 0x45;
  }
  *(dataPtr++) = 0x0f;

  switch (opCode) {
  case X86::ADDSDrr: *(dataPtr++) = 0x58; break;
  case X86::SUBSDrr: *(dataPtr++) = 0x5c; break;
  case X86::XORPSrr: *(dataPtr++) = 0x57; break;
  default:
    std::cerr << "emitRegInst can be used for ADDSDrr, XORPSrr.\n";
    exit(1);
  }

  unsigned char regNumber = 0xc0 + (XMMfrom % 8) + (XMMto % 8) * 8;
  *(dataPtr++) = regNumber;

  DFOS->append(data, dataPtr);
#endif
}

unsigned char registerCode(unsigned reg) {
#if defined(__linux__) && defined(__x86_64__)
  if (isR8R15Register(reg)) {
    return (unsigned char)(reg - X86::R8);
  } else {
    switch (reg) {
    case X86::RIP:
    case X86::EAX:
    case X86::RAX: return 0;
    case X86::EBX:
    case X86::RBX: return 3;
    case X86::ECX:
    case X86::RCX: return 1;
    case X86::EDX:
    case X86::RDX: return 2;
    case X86::ESI:
    case X86::RSI: return 6;
    case X86::EDI:
    case X86::RDI: return 7;
    case X86::RBP: return 5;
    default:
      std::cerr << "Unsupported register in registerCode.\n";
      exit(1);
    }
  }
#endif
}

void SpMVCodeEmitter::emitMOVSLQInst(unsigned destinationRegister, unsigned baseRegister,
                                     int memOffset) {
  emitMOVSLQInst(destinationRegister, baseRegister, 0, 1, memOffset);
}
//  movslq "memOffset"(%baseRegister, %scaleRegister, scaleFactor), %destinationRegister
void SpMVCodeEmitter::emitMOVSLQInst(unsigned destinationRegister, unsigned baseRegister,
                                     unsigned scaleRegister, unsigned int scaleFactor,
                                     int memOffset) {
#if defined(__linux__) && defined(__x86_64__)
  if (scaleFactor != 1 && scaleFactor != 2 && scaleFactor != 4 && scaleFactor != 8) {
    std::cerr << "Scale factor can only be 1, 2, 4, or 8.\n";
    exit(1);
  }

  unsigned char data[8];
  unsigned char *dataPtr = data;

  //-------------------------------------
  *(dataPtr) = 0x48;
  if (isR8R15Register(destinationRegister)) {
    *(dataPtr) |= 0x0c;
  }
  if (isR8R15Register(baseRegister)) {
    *(dataPtr) |= 0x09;
  }
  if (isR8R15Register(scaleRegister)) {
    *(dataPtr) |= 0x0a;
  }
  //-------------------------------------
  dataPtr++;
  *(dataPtr++) = 0x63;
  //-------------------------------------
  *(dataPtr) = 0x00;
  if (scaleRegister > 0) {
    *(dataPtr) |= 0x04;
  }
  if (memOffset != 0 && memOffset < 128 && memOffset >= -128) {
    *(dataPtr) |= 0x40;
  } else if (memOffset < -128 || memOffset >= 128) {
    *(dataPtr) |= 0x80;
  }
  *(dataPtr) |= registerCode(destinationRegister) * 8;
  //-------------------------------------
  if (scaleRegister > 0) {
    dataPtr++;
    *(dataPtr) = registerCode(scaleRegister) * 8;
  }
  *(dataPtr) |= registerCode(baseRegister) * 1;

  if (scaleRegister > 0) {
    switch (scaleFactor) {
    case 1: *(dataPtr) |= 0x00; break;
    case 2: *(dataPtr) |= 0x40; break;
    case 4: *(dataPtr) |= 0x80; break;
    case 8: *(dataPtr) |= 0xc0; break;
    }
  } else if (baseRegister == X86::R12) {
    dataPtr++;
    *(dataPtr) = 0x24;
  }

  dataPtr++;
  //-------------------------------------
  if (memOffset != 0) {
    *(dataPtr++) = (unsigned char)memOffset;
  }
  if (memOffset >= 128 || memOffset < -128) {
    *(dataPtr++) = (unsigned char)(memOffset >> 8);
    *(dataPtr++) = (unsigned char)(memOffset >> 16);
    *(dataPtr++) = (unsigned char)(memOffset >> 24);
  }

  DFOS->append(data, dataPtr);
#endif
}

//  leaq "memoffset"(%baseRegisterFrom), %baseRegisterTo
void SpMVCodeEmitter::emitLEAQInst(unsigned baseRegisterFrom, unsigned baseRegisterTo, int memOffset){
  emitLEAQInst(baseRegisterFrom, baseRegisterTo, 0, memOffset);
}

//  leaq "memoffset"(%baseRegisterFrom, %scaleRegister), %baseRegisterTo
void SpMVCodeEmitter::emitLEAQInst(unsigned baseRegisterFrom, unsigned baseRegisterTo, unsigned scaleRegister, int memOffset){
#if defined(__linux__) && defined(__x86_64__)
  if (!(scaleRegister == 0 || scaleRegister == X86::R10)) {
    std::cerr << "Unsupported scaleRegister in emitLEAQInst.\n";
    exit(1);
  }
  unsigned char data[8];
  unsigned char *dataPtr = data;

  if(isR8R15Register(baseRegisterFrom) && isR8R15Register(baseRegisterTo))
    *(dataPtr) = 0x4d;
  else if (isR8R15Register(baseRegisterFrom))
    *(dataPtr) = 0x49;
  else if (isR8R15Register(baseRegisterTo))
    *(dataPtr) = 0x4c;
  else
    *(dataPtr) = 0x48;

  if (scaleRegister > 0) {
    *(dataPtr) |= 0x02;
  }
  dataPtr++;

  *(dataPtr++) = 0x8d;

  *(dataPtr) = 0x00;
  if (memOffset >= 128 || memOffset < -128)
    *(dataPtr) |= 0x80;
  else if (memOffset != 0)
    *(dataPtr) |= 0x40;

  if (scaleRegister > 0) {
    *(dataPtr) |= 0x14;
    dataPtr++;
    *(dataPtr) = 0x00;
  }

  *dataPtr |= registerCode(baseRegisterFrom);
  *dataPtr |= registerCode(baseRegisterTo) * 8;

  dataPtr++;

  // memOffset
  if (memOffset != 0) {
    *(dataPtr++) = (unsigned char)memOffset;
  }
  if (memOffset >= 128 || memOffset < -128) {
    *(dataPtr++) = (unsigned char)(memOffset >> 8);
    *(dataPtr++) = (unsigned char)(memOffset >> 16);
    *(dataPtr++) = (unsigned char)(memOffset >> 24);
  }

  DFOS->append(data, dataPtr);
#endif
}

// leaq (%rip), %REG
void SpMVCodeEmitter::emitLEAQ_RIP(unsigned toRegister, int memOffset) {
#if defined(__linux__) && defined(__x86_64__)
  unsigned char data[7];
  unsigned char *dataPtr = data;

  if (!(toRegister == X86::R8 || toRegister == X86::RAX || toRegister == X86::RDX)) {
    std::cerr << "Unsupported toRegister in emitLEAQ_RIP.\n";
    exit(1);
  }

  if (isR8R15Register(toRegister))
    *(dataPtr++) = 0x4c;
  else
    *(dataPtr++) = 0x48;

  *(dataPtr++) = 0x8d;

  switch (toRegister) {
    case X86::RAX: *(dataPtr++) = 0x05; break;
    case X86::RDX: *(dataPtr++) = 0x15; break;
    case X86::R8:  *(dataPtr++) = 0x05; break;
    default: std::cerr << "Unsupported toRegister in emitLEAQ_RIP.\n"; exit(1);
  }
  *(dataPtr++) = (unsigned char)(memOffset >>  0);
  *(dataPtr++) = (unsigned char)(memOffset >>  8);
  *(dataPtr++) = (unsigned char)(memOffset >> 16);
  *(dataPtr++) = (unsigned char)(memOffset >> 24);

  DFOS->append(data, dataPtr);
#endif
}

void SpMVCodeEmitter::emitPushPopInst(unsigned opCode, unsigned baseRegister){
#if defined(__linux__) && defined(__x86_64__)
  unsigned char data[4];
  unsigned char *dataPtr = data;

  if(isR8R15Register(baseRegister)){
    *(dataPtr++) = 0x41;
  }
  *(dataPtr) = 0x50 | registerCode(baseRegister);

  if(opCode == X86::POP64r) {
    *(dataPtr) |= 0x8;
  }

  dataPtr++;

  DFOS->append(data, dataPtr);
#endif
}

// emits for both ADD64ri32 & ADD64ri8
void SpMVCodeEmitter::emitADDQInst(unsigned long offset, unsigned long baseRegister){
  unsigned char data[7];
  unsigned char *dataPtr = data;

  //  1st byte
  if (isR8R15Register(baseRegister)) {
    *(dataPtr++) = 0x49;
  } else {
    *(dataPtr++) = 0x48;
  }

  //  2nd byte
  if (offset < 128) {
    *(dataPtr++) = 0x83;
  } else {
    *(dataPtr++) = 0x81;
  }

  //  3rd byte
  *(dataPtr++) = 0xc0 + registerCode(baseRegister);

  // memOffset
  if (offset != 0) {
    *(dataPtr++) = (unsigned char) offset;
  }

  if (offset >= 128) {
    *(dataPtr++) = (unsigned char) (offset >> 8);
    *(dataPtr++) = (unsigned char) (offset >> 16);
    *(dataPtr++) = (unsigned char) (offset >> 24);
  }

  DFOS->append(data, dataPtr);
}

void SpMVCodeEmitter::emitMOVQInst(unsigned baseRegisterTo, unsigned baseRegisterFrom){
  unsigned char data[7];
  unsigned char *dataPtr = data;

  //  1st byte
  if (!isR8R15Register(baseRegisterFrom)) {
    if(!isR8R15Register(baseRegisterTo))
      *(dataPtr++) = 0x48;
    else
      *(dataPtr++) = 0x49;
  } else {
    if (!isR8R15Register(baseRegisterTo))
      *(dataPtr++) = 0x4c;
  }

  //  2nd byte
  *(dataPtr++) = 0x89;

  //  3rd byte
  *(dataPtr) = 0xc0;

  unsigned char offset1 = registerCode(baseRegisterTo);
  unsigned char offset2 = registerCode(baseRegisterFrom);

  *(dataPtr++) += offset1 + 8 * offset2;

  DFOS->append(data, dataPtr);
}

void SpMVCodeEmitter::emitJNEInst(long destinationAddress){
  long offset = (long)destinationAddress - (long)DFOS->size();
  if (offset >= 0) {
    std::cerr << "Only backwards jumps are handled in emitJNEInst.\n";
    exit(-1);
  }

  if (offset >= (-128 + 2)) {
    offset -= 2;
  } else {
    offset -= 6;
  }

  unsigned char data[6];
  unsigned char *dataPtr = data;
  if (offset < 128 && offset >= -128) {
    *(dataPtr++) = 0x75;
    *(dataPtr++) = (unsigned char) offset;
  } else {
    *(dataPtr++) = 0x0f;
    *(dataPtr++) = 0x85;
    *(dataPtr++) = (unsigned char) offset;
    *(dataPtr++) = (unsigned char) (offset >> 8);
    *(dataPtr++) = (unsigned char) (offset >> 16);
    *(dataPtr++) = (unsigned char) (offset >> 24);
  }

  DFOS->append(data, dataPtr);
}

void SpMVCodeEmitter::emitJMPInst(long numBytesToJump){
  if (numBytesToJump < 0) {
    std::cerr << "Only forward jumps are handled in emitJMPInst.\n";
    exit(-1);
  }

  unsigned char data[5];
  unsigned char *dataPtr = data;
  if (numBytesToJump < 128) {
    *(dataPtr++) = 0xeb;
    *(dataPtr++) = (unsigned char) numBytesToJump;
  } else {
    *(dataPtr++) = 0xe9;
    *(dataPtr++) = (unsigned char) numBytesToJump;
    *(dataPtr++) = (unsigned char) (numBytesToJump >> 8);
    *(dataPtr++) = (unsigned char) (numBytesToJump >> 16);
    *(dataPtr++) = (unsigned char) (numBytesToJump >> 24);
  }

  DFOS->append(data, dataPtr);
}

void SpMVCodeEmitter::emitDynamicJMPInst(unsigned baseRegister){
#if defined(__linux__) && defined(__x86_64__)
  if (baseRegister != X86::RDX) {
    std::cerr << "Only RDX is supported in emitDynamicJMPInst.\n";
    exit(-1);
  }

  unsigned char data[2];
  unsigned char *dataPtr = data;
  *(dataPtr++) = 0xff;
  *(dataPtr++) = 0xe2;

  DFOS->append(data, dataPtr);
#endif
}

void SpMVCodeEmitter::emitXOR32rrInst(unsigned registerFrom, unsigned registerTo) {
#if defined(__linux__) && defined(__x86_64__)
  if (registerFrom != registerTo) {
    std::cerr << "Source and destination registers must be the same in emitXOR32rr.\n";
    exit(-1);
  }

  unsigned char data[3];
  unsigned char *dataPtr = data;

  if (registerFrom >= X86::R8D && registerFrom <= X86::R15D) {
    *(dataPtr++) = 0x45;
  }

  *(dataPtr++) = 0x31;
  switch (registerFrom) {
    case X86::EAX: *(dataPtr++) = 0xc0; break;
    case X86::EBX: *(dataPtr++) = 0xdb; break;
    case X86::ECX: *(dataPtr++) = 0xc9; break;
    case X86::R9D: *(dataPtr++) = 0xc9; break;
    case X86::R11D: *(dataPtr++) = 0xdb; break;
    default: std::cerr << "Unhandled register in emitXOR32rr.\n"; exit(-1);
  }

  DFOS->append(data, dataPtr);
#endif
}

void SpMVCodeEmitter::emitCMP32riInst(unsigned registerTo, int imm) {
#if defined(__linux__) && defined(__x86_64__)
  unsigned char data[7];
  unsigned char *dataPtr = data;
  if (registerTo == X86::R11D) {
    *(dataPtr++) = 0x41;
  }
  *(dataPtr++) = 0x81;
  if (registerTo == X86::R11D) {
    *(dataPtr++) = 0xfb;
  } else {
    *(dataPtr++) = 0xf8 + registerCode(registerTo);
  }

  *(dataPtr++) = (unsigned char) imm;
  *(dataPtr++) = (unsigned char) (imm >> 8);
  *(dataPtr++) = (unsigned char) (imm >> 16);
  *(dataPtr++) = (unsigned char) (imm >> 24);

  DFOS->append(data, dataPtr);
#endif
}

void SpMVCodeEmitter::emitNOP4() {
  unsigned char data[4];
  unsigned char *dataPtr = data;
  *(dataPtr++) = 0x0F;
  *(dataPtr++) = 0x1F;
  *(dataPtr++) = 0x40;
  *(dataPtr++) = 0x00;
  DFOS->append(data, dataPtr);
}

void SpMVCodeEmitter::emitIntValue(int n) {
  unsigned char data[4];
  unsigned char *dataPtr = data;
  *(dataPtr++) = (unsigned char) n;
  *(dataPtr++) = (unsigned char) (n >> 8);
  *(dataPtr++) = (unsigned char) (n >> 16);
  *(dataPtr++) = (unsigned char) (n >> 24);
  DFOS->append(data, dataPtr);
}

void SpMVCodeEmitter::emitDoubleValue(double d) {
  uint64_t n = *((uint64_t *)&d);
  unsigned char data[8];
  unsigned char *dataPtr = data;
  *(dataPtr++) = (unsigned char) n;
  *(dataPtr++) = (unsigned char) (n >> 8);
  *(dataPtr++) = (unsigned char) (n >> 16);
  *(dataPtr++) = (unsigned char) (n >> 24);
  *(dataPtr++) = (unsigned char) (n >> 32);
  *(dataPtr++) = (unsigned char) (n >> 40);
  *(dataPtr++) = (unsigned char) (n >> 48);
  *(dataPtr++) = (unsigned char) (n >> 56);
  DFOS->append(data, dataPtr);
}

void SpMVCodeEmitter::emitCodeAlignment(unsigned int alignment) {
  unsigned numBytesToEmit = (alignment - (DFOS->size() % alignment)) % alignment;

  unsigned char data[15];
  unsigned char *dataPtr = data;

  if (numBytesToEmit == 1) {
    *(dataPtr++) = 0x90;
  } else if (numBytesToEmit == 2) {
    *(dataPtr++) = 0x66;
    *(dataPtr++) = 0x90;
  } else if (numBytesToEmit > 2) {
    if (numBytesToEmit == 6 || numBytesToEmit == 9) {
      *(dataPtr++) = 0x66;
    }
    if (numBytesToEmit >= 9) {
      for (int i=0; i < numBytesToEmit-9; i++) {
        *(dataPtr++) = 0x66;
      }
    }
    if (numBytesToEmit >= 10) {
      *(dataPtr++) = 0x2E;
    }
    *(dataPtr++) = 0x0F;
    *(dataPtr++) = 0x1F;

    if (numBytesToEmit == 4) {
      *(dataPtr++) = 0x40;
    } else if (numBytesToEmit == 5 || numBytesToEmit == 6) {
      *(dataPtr++) = 0x44;
      *(dataPtr++) = 0x00;
    } else if (numBytesToEmit == 7) {
      *(dataPtr++) = 0x80;
      *(dataPtr++) = 0x00;
      *(dataPtr++) = 0x00;
      *(dataPtr++) = 0x00;
    } else if (numBytesToEmit >= 8) {
      *(dataPtr++) = 0x84;
      *(dataPtr++) = 0x00;
      *(dataPtr++) = 0x00;
      *(dataPtr++) = 0x00;
      *(dataPtr++) = 0x00;
    }
    *(dataPtr++) = 0x00;
  }
  DFOS->append(data, dataPtr);
}

void SpMVCodeEmitter::emitRETInst() {
#if defined(__linux__) && defined(__x86_64__)
  unsigned char data[1];
  unsigned char *dataPtr = data;
  *(dataPtr++) = 0xc3;
  DFOS->append(data, dataPtr);
#endif
}

// addsd offset(%baseRegister), %xmmN
void SpMVCodeEmitter::emitADDSDrmInst(int offset, unsigned baseRegister, unsigned xmmRegisterNumber) {
#if defined(__linux__) && defined(__x86_64__)
  emitMemInst(X86::ADDSDrm, offset, baseRegister, 0, 1, xmmRegisterNumber);
#endif
}

// addsd offset(%baseRegister, %scaleRegister, scaleFactor), %xmmN
void SpMVCodeEmitter::emitADDSDrmInst(int offset, unsigned baseRegister, unsigned scaleRegister, unsigned scaleFactor, unsigned xmmRegisterNumber) {
#if defined(__linux__) && defined(__x86_64__)
  emitMemInst(X86::ADDSDrm, offset, baseRegister, scaleRegister, scaleFactor, xmmRegisterNumber);
#endif
}

// mulsd offset(%baseRegister), %xmmN
void SpMVCodeEmitter::emitMULSDrmInst(int offset, unsigned baseRegister, unsigned xmmRegisterNumber) {
#if defined(__linux__) && defined(__x86_64__)
  emitMemInst(X86::MULSDrm, offset, baseRegister, 0, 1, xmmRegisterNumber);
#endif
}

// mulsd offset(%baseRegister, %scaleRegister, scaleFactor), %xmmN
void SpMVCodeEmitter::emitMULSDrmInst(int offset, unsigned baseRegister, unsigned scaleRegister, unsigned scaleFactor, unsigned xmmRegisterNumber) {
#if defined(__linux__) && defined(__x86_64__)
  emitMemInst(X86::MULSDrm, offset, baseRegister, scaleRegister, scaleFactor, xmmRegisterNumber);
#endif
}

// movsd offset(%baseRegister), %xmmN
void SpMVCodeEmitter::emitMOVSDrmInst(int offset, unsigned baseRegister, unsigned xmmRegisterNumber) {
#if defined(__linux__) && defined(__x86_64__)
  emitMemInst(X86::MOVSDrm, offset, baseRegister, 0, 1, xmmRegisterNumber);
#endif
}

// movsd offset(%baseRegister, %scaleRegister, scaleFactor), %xmmN
void SpMVCodeEmitter::emitMOVSDrmInst(int offset, unsigned baseRegister, unsigned scaleRegister, unsigned scaleFactor, unsigned xmmRegisterNumber) {
#if defined(__linux__) && defined(__x86_64__)
  emitMemInst(X86::MOVSDrm, offset, baseRegister, scaleRegister, scaleFactor, xmmRegisterNumber);
#endif
}

// movsd %xmmN, offset(%baseRegister)
void SpMVCodeEmitter::emitMOVSDmrInst(unsigned xmmRegisterNumber, int offset, unsigned baseRegister) {
#if defined(__linux__) && defined(__x86_64__)
  emitMemInst(X86::MOVSDmr, offset, baseRegister, 0, 1, xmmRegisterNumber);
#endif
}

// movsd %xmmN, offset(%baseRegister, %scaleRegister, scaleFactor)
void SpMVCodeEmitter::emitMOVSDmrInst(unsigned xmmRegisterNumber, int offset, unsigned baseRegister, unsigned scaleRegister, unsigned scaleFactor) {
#if defined(__linux__) && defined(__x86_64__)
  emitMemInst(X86::MOVSDmr, offset, baseRegister, scaleRegister, scaleFactor, xmmRegisterNumber);
#endif
}

// mul/add/movsd offset(%baseRegister), %xmmN
void SpMVCodeEmitter::emitMemInst(unsigned opcode, int offset, unsigned baseRegister, unsigned scaleRegister, unsigned scaleFactor, unsigned xmmRegisterNumber) {
#if defined(__linux__) && defined(__x86_64__)
  if (scaleFactor != 1 && scaleFactor != 2 && scaleFactor != 4 && scaleFactor != 8) {
    std::cerr << "Scale factor can only be 1, 2, 4, or 8.\n";
    exit(1);
  }

  unsigned char data[10];
  unsigned char *dataPtr = data;
  //----------------------------------------------------------
  *(dataPtr++) = 0xf2;
  //----------------------------------------------------------
  unsigned char extraByte = 0x00;
  if (isR8R15Register(baseRegister)) {
    extraByte |= 0x41;
  }
  if (isR8R15Register(scaleRegister)) {
    extraByte |= 0x42;
  }
  if (xmmRegisterNumber >= 8) {
    extraByte |= 0x44;
  }
  if (extraByte != 0x00) {
    *(dataPtr++) = extraByte;
  }
  //----------------------------------------------------------
  *(dataPtr++) = 0x0f;
  //----------------------------------------------------------
  switch (opcode) {
  case X86::MULSDrm: *(dataPtr++) = 0x59; break;
  case X86::ADDSDrm: *(dataPtr++) = 0x58; break;
  case X86::MOVSDrm: *(dataPtr++) = 0x10; break;
  case X86::MOVSDmr: *(dataPtr++) = 0x11; break;
  default:
    std::cerr << "emitMemInst supports MULSDrm, ADDSDrm, MOVSDrm, MOVSDmr only.\n";
    exit(1);
  }
  //----------------------------------------------------------
  unsigned char memOffsetIndicator = 0x00;
  if (offset != 0) {
    if (offset < 128 && offset >= -128) {
      memOffsetIndicator += 0x40;
    } else {
      memOffsetIndicator += 0x80;
    }
  }

  if (scaleRegister == 0) {
    *(dataPtr) = memOffsetIndicator;
    *(dataPtr) += (xmmRegisterNumber % 8) * 8;
    *(dataPtr) += registerCode(baseRegister);
    dataPtr++;
  } else {
    *(dataPtr) = 0x04;
    *(dataPtr) += memOffsetIndicator;
    *(dataPtr) += (xmmRegisterNumber % 8) * 8;
    dataPtr++;
    *(dataPtr) = registerCode(baseRegister);
    *(dataPtr) |= (registerCode(scaleRegister) << 3);
    switch (scaleFactor) {
    case 1: *(dataPtr) |= 0x00; break;
    case 2: *(dataPtr) |= 0x40; break;
    case 4: *(dataPtr) |= 0x80; break;
    case 8: *(dataPtr) |= 0xc0; break;
    }
    dataPtr++;
  }
  //----------------------------------------------------------
  if (offset != 0) {
    if (offset < 128 && offset >= -128) {
      *(dataPtr++) = (unsigned char) offset;
    } else {
      *(dataPtr++) = (unsigned char) offset;
      *(dataPtr++) = (unsigned char) (offset >> 8);
      *(dataPtr++) = (unsigned char) (offset >> 16);
      *(dataPtr++) = (unsigned char) (offset >> 24);
    }
  }
  DFOS->append(data, dataPtr);
#endif
}


void SpMVCodeEmitter::emitFPNegation(unsigned xmmRegisterNumber) {
  unsigned char data[5];
  unsigned char *dataPtr = data;

  // *(dataPtr++) = 0x66;

  if (xmmRegisterNumber >= 8) {
    *(dataPtr++) = 0x44;
  }
  *(dataPtr++) = 0x0f;
  *(dataPtr++) = 0x57;
  *(dataPtr++) = 0x01 | (xmmRegisterNumber%8)*8;

  DFOS->append(data, dataPtr);
}

unsigned rotateLeft(unsigned n, unsigned times) {
  return ((n << times) | (n >> (32 - times)));
}

unsigned rotateRight(unsigned n, unsigned times) {
  return ((n >> times) | (n << (32 - times)));
}

bool encodeAsARMImmediate(int n, unsigned &result) {
  if (n < 0) {
    std::cerr << "Negative numbers cannot be encoded yet.\n";
    exit(1);
  }
  unsigned i, m, number; 
  number = (unsigned)n;

  for (unsigned i = 0; i < 16; i++) {
    unsigned m = rotateLeft(number, i * 2);
    if (m < 256) {
      result = (i << 8) | m;
      return true;
    }
  }

  return false;
}

int absoluteValueOf(int value) {
  return (value < 0) ? (-1 * value) : value;
}

//          vldr    d17, [r2, i*8]
void SpMVCodeEmitter::emitVLDRArmInst(unsigned dest_d, unsigned base_r, int offset)
{
  if (offset % 4 != 0 || offset >= 1024 || offset <= -1024) {
    std::cerr << "Cannot handle offset " << offset << " in VLDR.\n";
    exit(1);
  } 

  unsigned char data[4];
  unsigned char *dataPtr = data;
  unsigned base = base_r - ARM::R0;
  unsigned dest = dest_d - ARM::D16;

  unsigned sign = offset < 0 ? 0x50 : 0xd0;
  if (offset < 0)
    offset = -offset;

  *(dataPtr++) = 0xFF & (offset >> 2);
  *(dataPtr++) = (dest << 4) | 0x0b;
  *(dataPtr++) = sign | (base & 0x0f);
  *(dataPtr++) = 0xed;
  DFOS->append(data, dataPtr);
}

//              ldr     r5, [r1, i*4]
void SpMVCodeEmitter::emitLDRRegisterArmInst(unsigned dest_r, unsigned base_r, unsigned offset_register)
{
  //           ldr     r5, [r1, i*4]
  //e5915000  // 0
  //e5915004
  //e5915008

  //ldr     r5, [r0, r3] ?
  //e7905003

  unsigned char data[4];
  unsigned char *dataPtr = data;
  unsigned dest = dest_r - ARM::R0;
  unsigned base = base_r - ARM::R0;
  unsigned offset = offset_register - ARM::R0;
  *(dataPtr++) = 0x00 | (offset&0x0F);
  *(dataPtr++) = ((dest<<4)&0xF0);
  *(dataPtr++) = 0x90 | (base&0x0F);
  *(dataPtr++) = 0xe7;

  DFOS->append(data, dataPtr);
}

void SpMVCodeEmitter::emitLDROffsetArmInst(unsigned dest_r, unsigned base_r, int offset)
{
  if (offset < 0 || offset >= 4096) {
    std::cerr << "Cannot handle offset " << offset << " in LDROffset.\n";
    exit(1);
  }

  //           ldr     r5, [r1, i*4]
  //0x12 0x70 0x94 0xe5  ldr	r7, [r4, #18]
  unsigned char data[4];
  unsigned char *dataPtr = data;
  unsigned dest = dest_r - ARM::R0;
  unsigned base = base_r - ARM::R0;
  if (base_r == ARM::SP)
    base = 0x0d;

  *(dataPtr++) = 0xFF & offset;
  *(dataPtr++) = ((dest << 4) & 0xF0) | ((offset >> 8) & 0x0F);
  *(dataPtr++) = 0x90 | (base & 0x0F);
  *(dataPtr++) = 0xe5;

  DFOS->append(data, dataPtr);
}


//     add     r5, lr, r5, lsl #3
void SpMVCodeEmitter::emitADDRegisterArmInst(unsigned dest_r, unsigned base1_r, unsigned base2_r, int scaler)
{
  //e08e5185
  unsigned char data[4];
  unsigned char *dataPtr = data;

  //add     r5, r4, r5, lsl #3  ?
  //e0845185
  //e08e5005 	add	r5, lr, r5
  //0x82 0x41 0x8e 0xe0 add	r4, lr, r2, lsl #3
  //0x82 0x51 0x8e 0xe0  add	r5, lr, r2, lsl #3
  //0x85 0x51 0x8e 0xe0 add	r5, lr, r5, lsl #3
  //0x05 0x50 0x8e 0xe0 add	r5, lr, r5
  //0x05 0x50 0x8e 0xe0 add	r5, lr, r5
  //0x05 0x51 0x8e 0xe0  add	r5, lr, r5, lsl #2
  //0x05 0x50 0x8e 0xe0  add	r5, lr, r5
  //0x85 0x50 0x8e 0xe0  add	r5, lr, r5, lsl #1
  //0xc5 0x50 0x8e 0xe0  add	r5, lr, r5, asr #1
  //0x05 0x52 0x8e 0xe0  add	r5, lr, r5, lsl #4
  //0x85 0x52 0x8e 0xe0  add	r5, lr, r5, lsl #5
  //0x05 0x52 0x8e 0xe0  add	r5, lr, r5, lsl #4
  //0x05 0x53 0x8e 0xe0 	add	r5, lr, r5, lsl #6

  unsigned dest = dest_r - ARM::R0;
  unsigned base1;
  if (base1_r >= ARM::R0)
    base1 = base1_r - ARM::R0;
  else {
    if(base1_r == ARM::SP) 
      base1 = 0xd;
    else if(base1_r == ARM::LR) 
      base1 = 0xe;
    else if(base1_r == ARM::PC) 
      base1 = 0xf;
  }

  unsigned base2;
  if (base2_r >= ARM::R0)
    base2 = base2_r - ARM::R0;
  else {
    if(base2_r == ARM::SP) 
      base2 = 0xd;
    else if(base2_r == ARM::LR) 
      base2 = 0xe;
    else if(base2_r == ARM::PC) 
      base2 = 0xf;
  }

  unsigned char scaler_odd = (scaler%2)==1 ? 0x80 : 0x00;

  *(dataPtr++) = scaler_odd | (base2&0x0F);
  *(dataPtr++) = (scaler>>1) | ((dest<<4)&0xF0);
  *(dataPtr++) = 0x80 | (base1&0x0F);
  *(dataPtr++) = 0xe0;
  DFOS->append(data, dataPtr);
}

//     add     r5, lr, #offset
void SpMVCodeEmitter::emitADDOffsetArmInst(unsigned dest_r, unsigned base_r, int offset)
{ 
  if (offset == 0 && dest_r == base_r) return;

  unsigned encodedOffset = 0;
  if (!encodeAsARMImmediate(offset, encodedOffset)) {
    // Emit more than one instruction to handle this case 
    emitADDOffsetArmInst(dest_r, base_r, offset & 0x000000FF);
    emitADDOffsetArmInst(dest_r, dest_r, offset & 0x0000FF00);
    emitADDOffsetArmInst(dest_r, dest_r, offset & 0x00FF0000);
    emitADDOffsetArmInst(dest_r, dest_r, offset & 0xFF000000);
    return;
  }

  unsigned char data[4];
  unsigned char *dataPtr = data;

  unsigned dest = dest_r - ARM::R0;
  unsigned base = base_r - ARM::R0;

  *(dataPtr++) = 0xFF & encodedOffset;
  *(dataPtr++) = ((dest << 4) & 0xF0) | ((encodedOffset >> 8) & 0x0F);
  *(dataPtr++) = 0x80 | (base & 0x0F);
  *(dataPtr++) = 0xe2;
  DFOS->append(data, dataPtr);
}

void SpMVCodeEmitter::emitSUBOffsetArmInst(unsigned dest_r, unsigned base_r, int offset)
{
  if (offset == 0 && dest_r == base_r) return;

  unsigned encodedOffset = 0;
  if (!encodeAsARMImmediate(offset, encodedOffset)) {
    // Emit more than one instruction to handle this case
    emitSUBOffsetArmInst(dest_r, base_r, offset & 0x000000FF);
    emitSUBOffsetArmInst(dest_r, dest_r, offset & 0x0000FF00);
    emitSUBOffsetArmInst(dest_r, dest_r, offset & 0x00FF0000);
    emitSUBOffsetArmInst(dest_r, dest_r, offset & 0xFF000000);
    return;
  }
  
  unsigned char data[4];
  unsigned char *dataPtr = data;
  
  unsigned dest = dest_r - ARM::R0;
  unsigned base = base_r - ARM::R0;
  
  *(dataPtr++) = 0xFF & encodedOffset;
  *(dataPtr++) = ((dest << 4) & 0xF0) | ((encodedOffset >> 8) & 0x0F);
  *(dataPtr++) = 0x40 | (base & 0x0F);
  *(dataPtr++) = 0xe2;
  DFOS->append(data, dataPtr);
}


//     vmul.f64        d17, d17, d20
void SpMVCodeEmitter::emitVMULArmInst(unsigned dest_d, unsigned base1_d, unsigned base2_d)
{
  //     vmul.f64        d17, d17, d20
  //ee611ba4
  unsigned char data[4];
  unsigned char *dataPtr = data;
  unsigned dest = dest_d - ARM::D16;
  unsigned base1 = base1_d - ARM::D16;
  unsigned base2 = base2_d - ARM::D16;

  *(dataPtr++) = 0xa0 | (base2 & 0x0f);
  *(dataPtr++) = 0x0b | ((dest << 4) & 0xf0);
  *(dataPtr++) = 0x60 | (base1 & 0x0f);
  *(dataPtr++) = 0xee;
  DFOS->append(data, dataPtr);
}

//          vadd.f64        d16, d16, d17
void SpMVCodeEmitter::emitVADDArmInst(unsigned dest_d, unsigned base1_d, unsigned base2_d)
{
  //vadd.f64        d16, d16, d17
  //ee700ba1
  //vadd.f64        d16, d18, d16
  //ee720ba0
  unsigned char data[4];
  unsigned char *dataPtr = data;
  unsigned dest = dest_d - ARM::D16;
  unsigned base1 = base1_d - ARM::D16;
  unsigned base2 = base2_d - ARM::D16;

  *(dataPtr++) = 0xa0 | (base2&0x0f);
  *(dataPtr++) = 0x0b | ((dest<<4) & 0xf0);
  *(dataPtr++) = 0x70 | (base1&0x0f);
  *(dataPtr++) = 0xee;
  DFOS->append(data, dataPtr);
}

//vmov.i32        d16, #0x0
void SpMVCodeEmitter::emitVMOVI32ArmInst(unsigned dest_d, int value)
{
  if (value >= 256) {
    std::cerr << "Cannot have value >= 256 in VMOVI32 yet.\n";
    exit(1);
  } 
  //vmov.i32        d16, #0x0
  //f2c00010
  unsigned char data[4];
  unsigned char *dataPtr = data;
  unsigned dest = dest_d - ARM::D16;
  *(dataPtr++) = 0x10;
  *(dataPtr++) = ((dest<<4) & 0xf0);
  *(dataPtr++) = 0xc0;
  *(dataPtr++) = 0xf2;

  DFOS->append(data, dataPtr);
}

// mov r3, #1234
void SpMVCodeEmitter::emitMOVArmInst(unsigned base_r, int value)
{
  unsigned encodedValue = 0;
  if (!encodeAsARMImmediate(value, encodedValue)) {
    if (value >= 0 && value < (1 << 16)) {
      // try 16-bit mover
      emitMOVWArmInst(base_r, value);
      return;
    } else {
      // Emit more than one instruction to handle this case 
      unsigned partOne   = value & 0x000000FF;
      unsigned partTwo   = value & 0x0000FF00;
      unsigned partThree = value & 0x00FF0000;
      unsigned partFour  = value & 0xFF000000;
      emitMOVArmInst(base_r, partOne);
      emitADDOffsetArmInst(base_r, base_r, partTwo);
      emitADDOffsetArmInst(base_r, base_r, partThree);
      emitADDOffsetArmInst(base_r, base_r, partFour);
      return;
    }
  }

  unsigned char data[4];
  unsigned char *dataPtr = data;
  unsigned base = base_r - ARM::R0;

  *(dataPtr++) = 0xFF & encodedValue;
  *(dataPtr++) = ((base << 4) & 0xF0) | ((encodedValue >> 8) & 0x0F);
  *(dataPtr++) = 0xa0;
  *(dataPtr++) = 0xe3;

  DFOS->append(data, dataPtr);
}

// movw r3, #1234
void SpMVCodeEmitter::emitMOVWArmInst(unsigned base_r, int value)
{
  if (value < 0 || value >= (1 << 16)) {
    // try rotation encoding.
    emitMOVArmInst(base_r, value);
    return;
  } 

  unsigned char data[4];
  unsigned char *dataPtr = data;
  unsigned base = base_r - ARM::R0;
  *(dataPtr++) = value & 0xFF;
  *(dataPtr++) = (base << 4) & 0xF0 | ((value >> 8) & 0x0F);
  *(dataPtr++) = (value >> 12) & 0x0F;
  *(dataPtr++) = 0xe3;

  DFOS->append(data, dataPtr);
}

//vstr    d16, [r5]
void SpMVCodeEmitter::emitVSTRArmInst(unsigned dest_d, unsigned base_r)
{
  //vstr    d16, [r5]
  //edc50b00
  unsigned char data[4];
  unsigned char *dataPtr = data;
  unsigned dest = dest_d - ARM::D16;
  unsigned base = base_r - ARM::R0;
  *(dataPtr++) = 0x00;
  *(dataPtr++) = 0x0b | ((dest<<4)&0xf0);
  *(dataPtr++) = 0xc0 | (base&0x0f);
  *(dataPtr++) = 0xed;

  DFOS->append(data, dataPtr);
}
 
void SpMVCodeEmitter::emitCMPRegisterArmInst(unsigned dest_r, unsigned base_r)
{
  unsigned char data[4];
  unsigned char *dataPtr = data;
  unsigned dest = dest_r - ARM::R0;
  unsigned base = base_r - ARM::R0;

  *(dataPtr++) = 0x00 | (base) & 0x0F;
  *(dataPtr++) = 0x00;
  *(dataPtr++) = 0x50 | (dest&0x0f);
  *(dataPtr++) = 0xe1;

  DFOS->append(data, dataPtr);
}

void SpMVCodeEmitter::emitCMPOffsetArmInst(unsigned dest_r, int value, unsigned backup_r)
{
  unsigned encodedOffset = 0;
  if (!encodeAsARMImmediate(value, encodedOffset)) {
    emitMOVWArmInst(backup_r, value);
    emitCMPRegisterArmInst(dest_r, backup_r);
    return;
  }

  unsigned char data[4];
  unsigned char *dataPtr = data;
  unsigned dest = dest_r - ARM::R0;

  *(dataPtr++) = 0xFF & encodedOffset;
  *(dataPtr++) = 0x00 | ((encodedOffset >> 8) & 0x0F);
  *(dataPtr++) = 0x50 | (dest&0x0f);
  *(dataPtr++) = 0xe3;

  DFOS->append(data, dataPtr);
}

//bne     .LBB0_1
void SpMVCodeEmitter::emitBNEArmInst(long destinationAddress)
{
  int target = destinationAddress - DFOS->size() - 8;
  if (target % 4 != 0) {
    std::cerr << "Can jump to a word-address only. Target address " << target << " is bad for BNE.\n";
    exit(1);
  }
  target = target >> 2;
  if (absoluteValueOf(target) >= (1 << 24)) {
    std::cerr << "Cannot have target (" << target << ") larger than 24 bits.\n";
    exit(1);
  } 
  unsigned char data[4];
  unsigned char *dataPtr = data;

  *(dataPtr++) = 0xFF & target;
  *(dataPtr++) = 0xFF & (target >> 8);
  *(dataPtr++) = 0xFF & (target >> 16);
  *(dataPtr++) = 0x1a;

  DFOS->append(data, dataPtr);

  //bne     .LBB0_1
  //  88:   1affffe3        bne     1c <_Z4spmvPiS_Pd+0x1c>
}

void SpMVCodeEmitter::emitPushArmInst()
{
  unsigned char data[4];
  unsigned char *dataPtr = data; 
  *(dataPtr++) = 0xf0;
  *(dataPtr++) = 0x4b;
  *(dataPtr++) = 0x2d;
  *(dataPtr++) = 0xe9;

  DFOS->append(data, dataPtr);
}

void SpMVCodeEmitter::emitPopArmInst()
{
  //0x00 0x80 0xbd 0xe8    ldm	sp!, {pc}
  unsigned char data[4];
  unsigned char *dataPtr = data;
	 
  *(dataPtr++) = 0xf0;
  *(dataPtr++) = 0x8b;
  *(dataPtr++) = 0xbd;
  *(dataPtr++) = 0xe8;

  DFOS->append(data, dataPtr);
}

