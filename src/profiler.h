#ifndef _PROFILING_H_
#define _PROFILING_H_

#include <sys/time.h>
#include <vector>
#include <string>

namespace spMVgen {
  class Profiler {
  public:
    static void addNewTiming(std::string, struct timeval);
    static void increaseTimingLevel();
    static void decreaseTimingLevel();
    static int timediff(struct timeval *res, struct timeval *x, struct timeval *y);
    static void print(unsigned int numIters);
  private:
    static int timingLevel;
    static std::vector<struct timeval> timingDiffs;
    static std::vector<std::string> timingNames;
    static std::vector<int> timingLevels;
  };
}

#define START_TIME_PROFILE(profileName) struct timeval time_##profileName##_Start, time_##profileName##_End; \
  struct timeval time_##profileName##_Diff;\
  Profiler::increaseTimingLevel();\
  gettimeofday(&time_##profileName##_Start, NULL);

#define END_TIME_PROFILE(profileName) gettimeofday(&time_##profileName##_End, NULL); \
  Profiler::timediff(&time_##profileName##_Diff, &time_##profileName##_End, &time_##profileName##_Start);\
  Profiler::addNewTiming(#profileName, time_##profileName##_Diff);	\
  Profiler::decreaseTimingLevel();

#endif
