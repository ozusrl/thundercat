#ifndef SPMVGEN_H
#define SPMVGEN_H

#include "matrix.h"
#include "method.h"
#include "profiler.h"
#include <vector>
#include <map>
#include "llvm/MC/MCStreamer.h"
#include "llvm/ExecutionEngine/ObjectBuffer.h"

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
    void loadBuffer(llvm::ObjectBuffer *Buffer);

    std::vector<MultByMFun> multByMFunctions;
    SpMVMethod *method;
    llvm::MCStreamer *Str;
  };

}

#endif
