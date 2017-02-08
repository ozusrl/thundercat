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
    void emitADDQInst(unsigned long offset, unsigned long baseRegister);
    void emitMOVQInst(unsigned baseRegisterTo, unsigned baseRegisterFrom);
    void emitJNEInst(long destinationAddress);
    void emitJMPInst(long destinationAddress);
    void emitDynamicJMPInst(unsigned baseRegister);
    void emitXOR32rrInst(unsigned registerFrom, unsigned registerTo);
    void emitCMP32riInst(unsigned registerTo, int imm);
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



    void emitVLDRArmInst(unsigned dest_d, unsigned base_r, int offset);
    void emitLDROffsetArmInst(unsigned dest_r, unsigned base_r, int offset);
    void emitLDRRegisterArmInst(unsigned dest_r, unsigned base_r, unsigned offset_register);
    void emitADDRegisterArmInst(unsigned dest_r, unsigned base1_r, unsigned base2_r, int scaler);
    void emitADDOffsetArmInst(unsigned dest_r, unsigned base1_r, int offset);
    void emitSUBOffsetArmInst(unsigned dest_r, unsigned base1_r, int offset);
    void emitVMULArmInst(unsigned dest_d, unsigned base1_d, unsigned base2_d);
    void emitVADDArmInst(unsigned dest_d, unsigned base1_d, unsigned base2_d);
    void emitVMOVI32ArmInst(unsigned dest_d, int value);
    void emitMOVArmInst(unsigned base_r, int value);
    void emitMOVWArmInst(unsigned base_r, int value);
    void emitVSTRArmInst(unsigned dest_d, unsigned base_r);
    void emitCMPRegisterArmInst(unsigned dest_r, unsigned base_r);
    void emitCMPOffsetArmInst(unsigned dest_r, int value, unsigned backup_r);
    void emitBNEArmInst(long destinationAddress);
    void emitPushArmInst();
    void emitPopArmInst();

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
