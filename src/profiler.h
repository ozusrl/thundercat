#ifndef _PROFILING_H_
#define _PROFILING_H_

#include <sys/time.h>
#include <vector>
#include <string>
#include <functional>

namespace thundercat {
  struct TimingInfo {
    long long duration;
    int level;
    std::string description;
  };
  
  class Profiler {
  public:
    static void recordTime(std::string description, std::function<void()> codeBlock);
    static void print(unsigned int numIters);
  private:
    static int timingLevel;
    static std::vector<TimingInfo> timingInfos;
  };
}

#endif
