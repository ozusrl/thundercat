#ifndef _PROFILING_H_
#define _PROFILING_H_

#include <sys/time.h>
#include <vector>
#include <string>
#include <functional>
#include <map>

namespace thundercat {
  struct TimingInfo {
    long long duration;
    int level;
    std::string description;
  };
  
  class Profiler {
  public:
    static void recordTime(std::string description, std::function<void()> codeBlock);
    static void recordSpmv(std::function<void()> codeBlock);
    static void recordSpmvOverhead(std::string description, std::function<void()> codeBlock);
    static void print(unsigned int numIters, unsigned int NNZ, unsigned int flopPerNNZ = 2);
  private:
    static bool startedSpmv;
    static int timingLevel;
    static std::vector<TimingInfo> timingInfos;
    static std::map<std::string,TimingInfo> spmvOverheads;
    static TimingInfo spmvTiming;

    static long long measure(std::function<void()> codeBlock);
  };
}

#endif
