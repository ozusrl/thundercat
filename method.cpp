#include "method.h"
#include <iostream>
#include <sstream>
#include "lib/Target/X86/MCTargetDesc/X86BaseInfo.h"
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
  return registerNumber >= X86::R8 && registerNumber <= X86::R15;
}

void SpMVCodeEmitter::emitRegInst(unsigned opCode, int XMMfrom, int XMMto) {
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
}

unsigned char registerCode(unsigned reg) {
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
}

void SpMVCodeEmitter::emitMOVSLQInst(unsigned destinationRegister, unsigned baseRegister, 
                                     int memOffset) {
  emitMOVSLQInst(destinationRegister, baseRegister, 0, 1, memOffset);
}
//  movslq "memOffset"(%baseRegister, %scaleRegister, scaleFactor), %destinationRegister
void SpMVCodeEmitter::emitMOVSLQInst(unsigned destinationRegister, unsigned baseRegister, 
                                     unsigned scaleRegister, unsigned int scaleFactor, 
                                     int memOffset) {
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
}

//  leaq "memoffset"(%baseRegisterFrom), %baseRegisterTo
void SpMVCodeEmitter::emitLEAQInst(unsigned baseRegisterFrom, unsigned baseRegisterTo, int memOffset){
  emitLEAQInst(baseRegisterFrom, baseRegisterTo, 0, memOffset);
}

//  leaq "memoffset"(%baseRegisterFrom, %scaleRegister), %baseRegisterTo
void SpMVCodeEmitter::emitLEAQInst(unsigned baseRegisterFrom, unsigned baseRegisterTo, unsigned scaleRegister, int memOffset){
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
}

// leaq (%rip), %REG
void SpMVCodeEmitter::emitLEAQ_RIP(unsigned toRegister, int memOffset) {
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
}

void SpMVCodeEmitter::emitPushPopInst(unsigned opCode, unsigned baseRegister){
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
  if (baseRegister != X86::RDX) {
    std::cerr << "Only RDX is supported in emitDynamicJMPInst.\n";
    exit(-1);
  }
  
  unsigned char data[2];
  unsigned char *dataPtr = data;
  *(dataPtr++) = 0xff;
  *(dataPtr++) = 0xe2;
  
  DFOS->append(data, dataPtr);
}

void SpMVCodeEmitter::emitXOR32rrInst(unsigned registerFrom, unsigned registerTo) {
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
}

void SpMVCodeEmitter::emitCMP32riInst(unsigned registerTo, int imm) {
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
  unsigned char data[1];
  unsigned char *dataPtr = data;
  *(dataPtr++) = 0xc3;
  DFOS->append(data, dataPtr);
}

// addsd offset(%baseRegister), %xmmN
void SpMVCodeEmitter::emitADDSDrmInst(int offset, unsigned baseRegister, unsigned xmmRegisterNumber) {
  emitMemInst(X86::ADDSDrm, offset, baseRegister, 0, 1, xmmRegisterNumber);
}

// addsd offset(%baseRegister, %scaleRegister, scaleFactor), %xmmN
void SpMVCodeEmitter::emitADDSDrmInst(int offset, unsigned baseRegister, unsigned scaleRegister, unsigned scaleFactor, unsigned xmmRegisterNumber) {
  emitMemInst(X86::ADDSDrm, offset, baseRegister, scaleRegister, scaleFactor, xmmRegisterNumber);
}

// mulsd offset(%baseRegister), %xmmN
void SpMVCodeEmitter::emitMULSDrmInst(int offset, unsigned baseRegister, unsigned xmmRegisterNumber) {
  emitMemInst(X86::MULSDrm, offset, baseRegister, 0, 1, xmmRegisterNumber);
}

// mulsd offset(%baseRegister, %scaleRegister, scaleFactor), %xmmN
void SpMVCodeEmitter::emitMULSDrmInst(int offset, unsigned baseRegister, unsigned scaleRegister, unsigned scaleFactor, unsigned xmmRegisterNumber) {
  emitMemInst(X86::MULSDrm, offset, baseRegister, scaleRegister, scaleFactor, xmmRegisterNumber);
}

// movsd offset(%baseRegister), %xmmN
void SpMVCodeEmitter::emitMOVSDrmInst(int offset, unsigned baseRegister, unsigned xmmRegisterNumber) {
  emitMemInst(X86::MOVSDrm, offset, baseRegister, 0, 1, xmmRegisterNumber);
}

// movsd offset(%baseRegister, %scaleRegister, scaleFactor), %xmmN
void SpMVCodeEmitter::emitMOVSDrmInst(int offset, unsigned baseRegister, unsigned scaleRegister, unsigned scaleFactor, unsigned xmmRegisterNumber) {
  emitMemInst(X86::MOVSDrm, offset, baseRegister, scaleRegister, scaleFactor, xmmRegisterNumber);
}

// movsd %xmmN, offset(%baseRegister)
void SpMVCodeEmitter::emitMOVSDmrInst(unsigned xmmRegisterNumber, int offset, unsigned baseRegister) {
  emitMemInst(X86::MOVSDmr, offset, baseRegister, 0, 1, xmmRegisterNumber);
}

// movsd %xmmN, offset(%baseRegister, %scaleRegister, scaleFactor)
void SpMVCodeEmitter::emitMOVSDmrInst(unsigned xmmRegisterNumber, int offset, unsigned baseRegister, unsigned scaleRegister, unsigned scaleFactor) {
  emitMemInst(X86::MOVSDmr, offset, baseRegister, scaleRegister, scaleFactor, xmmRegisterNumber);
}

// mul/add/movsd offset(%baseRegister), %xmmN
void SpMVCodeEmitter::emitMemInst(unsigned opcode, int offset, unsigned baseRegister, unsigned scaleRegister, unsigned scaleFactor, unsigned xmmRegisterNumber) {
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


