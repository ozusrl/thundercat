#include "spMVgen.h"
#include "method.h"
#include <iostream>
#include <sstream>
#include "llvm/MC/MCAsmBackend.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/MC/MCCodeEmitter.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCInstBuilder.h"
#include "llvm/MC/MCInstPrinter.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCObjectFileInfo.h"
#include "llvm/MC/MCParser/AsmLexer.h"
#include "llvm/MC/MCParser/MCAsmLexer.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCSectionMachO.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/MC/MCTargetAsmParser.h"
#include "llvm/MC/SubtargetFeature.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/FileUtilities.h"
#include "llvm/Support/FormattedStream.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/PrettyStackTrace.h"
#include "llvm/Support/Signals.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/ToolOutputFile.h"
#include "lib/Target/X86/X86TargetObjectFile.h"
#include "llvm/Support/DynamicLibrary.h"
#include "llvm/ExecutionEngine/SectionMemoryManager.h"
#include "llvm/ExecutionEngine/RuntimeDyld.h"
#include "llvm/ExecutionEngine/ObjectMemoryBuffer.h"

using namespace llvm;
using namespace llvm::object;
using namespace std;
using namespace spMVgen;

extern bool __DEBUG__;
extern bool DUMP_OBJECT;
extern unsigned int NUM_OF_THREADS;

SpMVSpecializer::SpMVSpecializer(SpMVMethod *method) {
  this->method = method;
}

////////////////////////////////////////////////////////////////////////////
// Taken from llvm-rtdyld.cpp
////////////////////////////////////////////////////////////////////////////

// A trivial memory manager that doesn't do anything fancy, just uses the
// support library allocation routines directly.
class TrivialMemoryManager : public RTDyldMemoryManager {
public:
  SmallVector<sys::MemoryBlock, 16> FunctionMemory;
  SmallVector<sys::MemoryBlock, 16> DataMemory;

  uint8_t *allocateCodeSection(uintptr_t Size, unsigned Alignment,
                               unsigned SectionID,
                               StringRef SectionName) override;
  uint8_t *allocateDataSection(uintptr_t Size, unsigned Alignment,
                               unsigned SectionID, StringRef SectionName,
                               bool IsReadOnly) override;

  void *getPointerToNamedFunction(const std::string &Name,
                                  bool AbortOnFailure = true) override {
    return nullptr;
  }

  bool finalizeMemory(std::string *ErrMsg) override { return false; }

  // Invalidate instruction cache for sections with execute permissions.
  // Some platforms with separate data cache and instruction cache require
  // explicit cache flush, otherwise JIT code manipulations (like resolved
  // relocations) will get to the data cache but not to the instruction cache.
  virtual void invalidateInstructionCache();

  void addDummySymbol(const std::string &Name, uint64_t Addr) {
    DummyExterns[Name] = Addr;
  }

  RuntimeDyld::SymbolInfo findSymbol(const std::string &Name) override {
    auto I = DummyExterns.find(Name);

    if (I != DummyExterns.end())
      return RuntimeDyld::SymbolInfo(I->second, JITSymbolFlags::Exported);

    return RTDyldMemoryManager::findSymbol(Name);
  }

  void registerEHFrames(uint8_t *Addr, uint64_t LoadAddr,
                        size_t Size) override {}
  void deregisterEHFrames(uint8_t *Addr, uint64_t LoadAddr,
                          size_t Size) override {}
private:
  std::map<std::string, uint64_t> DummyExterns;
};

uint8_t *TrivialMemoryManager::allocateCodeSection(uintptr_t Size,
                                                   unsigned Alignment,
                                                   unsigned SectionID,
                                                   StringRef SectionName) {
  sys::MemoryBlock MB = sys::Memory::AllocateRWX(Size, nullptr, nullptr);
  FunctionMemory.push_back(MB);
  return (uint8_t*)MB.base();
}

uint8_t *TrivialMemoryManager::allocateDataSection(uintptr_t Size,
                                                   unsigned Alignment,
                                                   unsigned SectionID,
                                                   StringRef SectionName,
                                                   bool IsReadOnly) {
  sys::MemoryBlock MB = sys::Memory::AllocateRWX(Size, nullptr, nullptr);
  DataMemory.push_back(MB);
  return (uint8_t*)MB.base();
}

void TrivialMemoryManager::invalidateInstructionCache() {
  for (int i = 0, e = FunctionMemory.size(); i != e; ++i)
    sys::Memory::InvalidateInstructionCache(FunctionMemory[i].base(),
                                            FunctionMemory[i].size());

  for (int i = 0, e = DataMemory.size(); i != e; ++i)
    sys::Memory::InvalidateInstructionCache(DataMemory[i].base(),
                                            DataMemory[i].size());
}
////////////////////////////////////////////////////////////////////////////

static string newName(const string &str, int i) {
  std::ostringstream oss;
  oss << i;
  string name(str);
  return name + oss.str();
}

string newStagedName(const string &str) {
  return newName(str, 0);
}

// adapted from llvm-rtdyld.cpp
void SpMVSpecializer::loadBuffer(ObjectFile *Buffer) {
  // Instantiate a dynamic linker.
  TrivialMemoryManager *MemMgr = new TrivialMemoryManager;
  RuntimeDyld *Dyld = new RuntimeDyld(*MemMgr, *MemMgr);

  // FIXME: Preserve buffers until resolveRelocations time to work around a bug
  //        in RuntimeDyldELF.
  // This fixme should be fixed ASAP. This is a very brittle workaround.
  std::vector<std::unique_ptr<MemoryBuffer>> InputBuffers;

  
  // Load the input memory buffer.
  Dyld->loadObject(*Buffer);
  if (Dyld->hasError()) {
    cerr << "Dyld error:" << Dyld->getErrorString().str() << "\n";
    exit(1);
  }

  // Resolve all the relocations we can.
  Dyld->resolveRelocations();
  // Clear instruction cache before code will be executed.
  MemMgr->invalidateInstructionCache();
  // FIXME: Error out if there are unresolved relocations.

  int i = 0;
  for (int i = 0; i < NUM_OF_THREADS; ++i) {
    void *multByMFunction = Dyld->getSymbolLocalAddress(newName(MAIN_FUNCTION_NAME, i));
    if (multByMFunction == 0) {
      cerr << "multByMFunction " << i << " not found.\n"; 
      exit(1);
    }
    multByMFunctions.push_back((MultByMFun)multByMFunction);
  }

  // Invalidate the instruction cache for each loaded function.
  for (unsigned i = 0, e = (unsigned)MemMgr->FunctionMemory.size(); i != e; ++i) {
    sys::MemoryBlock &Data = MemMgr->FunctionMemory[i];
    // Make sure the memory is executable.
    string ErrorStr;
    sys::Memory::InvalidateInstructionCache(Data.base(), Data.size());
    if (!sys::Memory::setExecutable(Data, &ErrorStr)) {
      cerr << "unable to mark function executable: '" << ErrorStr << "'\n";
      exit(1);
    }
  }
}

TargetLoweringObjectFile *getMCObjectFileInfo(Triple &TheTriple) {
  TargetLoweringObjectFile *mcObjectFileInfo = NULL;
  if(TheTriple.getArch() == Triple::x86_64 && TheTriple.getOS() == Triple::Darwin) {
    mcObjectFileInfo = new X86_64MachoTargetObjectFile();
  } else if(TheTriple.getArch() == Triple::x86_64 && TheTriple.getOS() == Triple::Linux) {
    mcObjectFileInfo = new X86ELFTargetObjectFile();
  } else {
    cerr << "Only X86_64 on Linux or MacOS is supported.\n";
    exit(1);
  }
  return mcObjectFileInfo;
}
  

vector<MultByMFun> &SpMVSpecializer::getMultByMFunctions() {
  return multByMFunctions;
}

// Taken from include/llvm/MC/MCTargetOptionsCommandFlags.h
static inline MCTargetOptions InitMCTargetOptions() {
  MCTargetOptions Options;
  Options.SanitizeAddress = false;
  Options.MCRelaxAll = false;
  Options.DwarfVersion = 0;
  Options.ShowMCInst = false;
  Options.ABIName = "";
  return Options;
}

// This method is adapted from llvm-mc.cpp
// TODO: Memory management.
void SpMVSpecializer::specialize() {
  // Initialize targets and assembly printers/parsers.
  
  llvm::InitializeAllTargetInfos();
  llvm::InitializeAllTargetMCs();
  llvm::InitializeAllAsmParsers();
  llvm::InitializeAllDisassemblers();
  
  // Figure out the target triple.
  Triple TheTriple(Triple::normalize(sys::getDefaultTargetTriple()));
  // Get the target specific parser.
  string Error;
  const Target *TheTarget = TargetRegistry::lookupTarget(/*ArchName*/ "", TheTriple, Error);
  if (!TheTarget) {
    cerr << "Target cannot be found: " << Error;
    exit(1);
  }
  string TripleName = TheTriple.getTriple();
  
  SourceMgr SrcMgr;
  SrcMgr.AddNewSourceBuffer(MemoryBuffer::getMemBuffer(""), SMLoc());
  
  std::unique_ptr<MCRegisterInfo> MRI(TheTarget->createMCRegInfo(TripleName));
  assert(MRI && "Unable to create target register info!");
  
  std::unique_ptr<MCAsmInfo> MAI(TheTarget->createMCAsmInfo(*MRI, TripleName));
  assert(MAI && "Unable to create target asm info!");

  // FIXME: This is not pretty. MCContext has a ptr to MCObjectFileInfo and
  // MCObjectFileInfo needs a MCContext reference in order to initialize itself.
  MCObjectFileInfo MOFI;
  MCContext Ctx(MAI.get(), MRI.get(), &MOFI, &SrcMgr);
  MOFI.InitMCObjectFileInfo(TheTriple, Reloc::Default, CodeModel::Default, Ctx);

  SmallVector<char, 1024*16> *smallVector = new SmallVector<char, 1024*16>();
  raw_svector_ostream svectorOS(*smallVector);
  {
    buffer_ostream BOS(svectorOS);
    std::unique_ptr<MCStreamer> Str;
    
    std::unique_ptr<MCInstrInfo> MCII(TheTarget->createMCInstrInfo());
    std::unique_ptr<MCSubtargetInfo> STI(TheTarget->createMCSubtargetInfo(TripleName, /*MCPU*/ "", /*FeaturesStr*/ ""));


    Ctx.setUseNamesOnTempLabels(false);
    MCCodeEmitter *CE = TheTarget->createMCCodeEmitter(*MCII, *MRI, Ctx);
    MCAsmBackend *MAB = TheTarget->createMCAsmBackend(*MRI, TripleName, /*MCPU*/ "");
    Str.reset(TheTarget->createMCObjectStreamer(TheTriple, Ctx, *MAB,
                                                BOS, CE, *STI, /*RelaxAll*/false,
                                                /*DWARFMustBeAtTheEnd*/false));
    
    setMCStreamer(&*Str);

    Str->SwitchSection(Ctx.getObjectFileInfo()->getTextSection());

    generateTextAndDataSections();

    std::unique_ptr<MCAsmParser> Parser(createMCAsmParser(SrcMgr, Ctx, *Str, *MAI));
    std::unique_ptr<MCTargetAsmParser> TAP(TheTarget->createMCAsmParser(*STI, *Parser, *MCII, InitMCTargetOptions()));
    if (!TAP) {
      cerr << "Could not create TAP: this target does not support assembly parsing.\n";
      exit(1);
    }
    
    Parser->setTargetParser(*TAP);
    Parser->Run(/*NoInitialTextSection*/ true);
    
    svectorOS.flush();
  }
  
  if (DUMP_OBJECT) {
    cout << svectorOS.str().str();
    exit(1);
  }
  
  std::unique_ptr<MemoryBuffer> Buffer(MemoryBuffer::getMemBuffer(svectorOS.str()));
  ErrorOr<std::unique_ptr<ObjectFile>> objectBuffer(ObjectFile::createObjectFile(Buffer->getMemBufferRef()));
  loadBuffer(objectBuffer.get().get());
}

void SpMVSpecializer::setMCStreamer(llvm::MCStreamer *Str) {
  this->Str = Str;
  method->setMCStreamer(Str);
}

void SpMVSpecializer::generateTextAndDataSections() {
  method->dumpMultByMFunctions();
  dumpAssemblyConstData();
}

void SpMVSpecializer::dumpAssemblyConstData() {
  MCContext &Ctx = Str->getContext();
  //  .section .rodata
  Str->SwitchSection(Ctx.getObjectFileInfo()->getDataSection());  
  // .globl n
  Str->EmitSymbolAttribute(Ctx.getOrCreateSymbol(StringRef("n")), MCSA_Global);
  Str->EmitSymbolAttribute(Ctx.getOrCreateSymbol(StringRef("_n")), MCSA_Global);
  // .globl nz
  Str->EmitSymbolAttribute(Ctx.getOrCreateSymbol(StringRef("nz")), MCSA_Global);
  Str->EmitSymbolAttribute(Ctx.getOrCreateSymbol(StringRef("_nz")), MCSA_Global);
  // .align 4
  Str->EmitCodeAlignment(4);
  // n:
  Str->EmitLabel(Ctx.getOrCreateSymbol(StringRef("n")));
  Str->EmitLabel(Ctx.getOrCreateSymbol(StringRef("_n")));
  // .long "N"
  Str->EmitIntValue(method->getCSRMatrix()->n, sizeof(int));
  // .align 4
  Str->EmitCodeAlignment(4);
  // nz:
  Str->EmitLabel(Ctx.getOrCreateSymbol(StringRef("nz")));
  Str->EmitLabel(Ctx.getOrCreateSymbol(StringRef("_nz")));
  // .long "NZ"
  Str->EmitIntValue(method->getCSRMatrix()->nz, sizeof(unsigned long));
}
