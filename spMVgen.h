#ifndef SPMVGEN_H
#define SPMVGEN_H

#include "matrix.h"
#include "method.h"
#include "profiler.h"
#include <vector>
#include <map>
#include "llvm/MC/MCStreamer.h"
#include "llvm/Object/ObjectFile.h"

namespace spMVgen {
  class SpMVSpecializer {
  public:
    SpMVSpecializer(SpMVMethod *method);

    void specialize();

    std::vector<MultByMFun> &getMultByMFunctions();

  private:
    void generateTextAndDataSections();
    void setMCStreamer(llvm::MCStreamer *Str);
    void dumpAssemblyConstData();
    void dumpAssemblyData();
    void loadBuffer(llvm::object::ObjectFile *Buffer);

    std::vector<MultByMFun> multByMFunctions;
    SpMVMethod *method;
    llvm::MCStreamer *Str;
  };

}

#endif
