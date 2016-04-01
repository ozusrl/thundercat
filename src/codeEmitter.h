#ifndef _SPMVLIB_CODE_EMITTER_H_
#define _SPMVLIB_CODE_EMITTER_H_

#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/MCObjectStreamer.h"
#include <string>

namespace spMVgen {
  class SpMVCodeEmitter {
  protected:
    void emitRegInst(unsigned opCode, int XMMfrom, int XMMto);
    void emitPushPopInst(unsigned opCode, unsigned baseRegister);
    void emitLEAQInst(unsigned baseRegisterFrom, unsigned baseRegisterTo, int memOffset);
    void emitLEAQInst(unsigned baseRegisterFrom, unsigned baseRegisterTo, unsigned scaleRegister, int memOffset);
    void emitLEAQ_RIP(unsigned toRegister, int memOffset);
    void emitMOVSLQInst(unsigned destinationRegister, unsigned baseRegister,
                        unsigned scaleRegister, unsigned int scaleFactor, 
                        int memOffset);
    void emitMOVSLQInst(unsigned destinationRegister, unsigned baseRegister,
                        int memOffset);
    void emitMOVSLQ32Inst(unsigned sourceRegister, unsigned destRegister);
    void emitADDQInst(unsigned long offset, unsigned long baseRegister);
    void emitMOVQInst(unsigned baseRegisterTo, unsigned baseRegisterFrom);
    void emitMOVQOffsetInst(int offset, unsigned baseRegisterTo, unsigned baseRegiserFrom);
    void emitMOVLOffsetInst(int offset, unsigned baseRegisterTo, unsigned baseRegisterFrom);
    void emitMOVQToStackInst(int offset, unsigned baseRegisterFrom);
    void emitMOVLImmInst(int offset, unsigned baseRegisterTo, int imm);

    void emitJNEInst(long destinationAddress);
    void emitJMPInst(long destinationAddress);
    void emitDynamicJMPInst(unsigned baseRegister);
    void emitXOR32rrInst(unsigned registerFrom, unsigned registerTo);
    void emitCMP32riInst(unsigned registerTo, int imm);
    void emitCMP32riInst(int offset, unsigned registerTo, int imm);

    void emitNOP4();
    void emitRETInst();
    void emitIntValue(int n);
    void emitDoubleValue(double n);
    void emitCodeAlignment(unsigned alignment);

    void emitADDSDrmInst(int offset, unsigned baseRegister, unsigned xmmRegisterNumber);
    void emitADDSDrmInst(int offset, unsigned baseRegister, unsigned scaleRegister, unsigned scaleFactor, unsigned xmmRegisterNumber);

    void emitMULSDrmInst(int offset, unsigned baseRegister, unsigned xmmRegisterNumber);
    void emitMULSDrmInst(int offset, unsigned baseRegister, unsigned scaleRegister, unsigned scaleFactor, unsigned xmmRegisterNumber);

    void emitMOVSDrmInst(int offset, unsigned baseRegister, unsigned xmmRegisterNumber);
    void emitMOVSDrmInst(int offset, unsigned baseRegister, unsigned scaleRegister, unsigned scaleFactor, unsigned xmmRegisterNumber);

    void emitMOVSDmrInst(unsigned xmmRegisterNumber, int offset, unsigned baseRegister);
    void emitMOVSDmrInst(unsigned xmmRegisterNumber, int offset, unsigned baseRegister, unsigned scaleRegister, unsigned scaleFactor);

    void emitFPNegation(unsigned xmmRegisterNumber);
    void emitMovImm(int value, unsigned destRegister);
    void emitMovlInst(unsigned sourceRegister, unsigned destRegister);
    void emitADDrrInst(unsigned sourceRegister, unsigned destRegister);
    void emitAddlInst(int offset, unsigned sourceRegister, unsigned destRegister);
    void emitAddlImmInst(unsigned long offset,  unsigned destRegister);

  protected:
    llvm::SmallVectorImpl<char> *DFOS;

    virtual void dumpPushPopHeader() = 0;
    virtual void dumpPushPopFooter() = 0;

    static llvm::SmallVectorImpl<char> *createNewDFOS(llvm::MCStreamer *Str, unsigned int index);

  private:
    static void dumpMultByMHeader(llvm::MCStreamer *Str, unsigned int index);
    static void dumpMultByMFooter(llvm::MCStreamer *Str);
    void emitMemInst(unsigned opcode, int offset, unsigned baseRegister, unsigned scaleRegister, unsigned scaleFactor, unsigned xmmRegisterNumber);
  };
}

#endif
