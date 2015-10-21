#include "profiler.h"
#include <iostream>
#include <iomanip>
using namespace spMVgen;

int Profiler::timingLevel = 0;
std::vector<struct timeval> Profiler::timingDiffs;
std::vector<std::string> Profiler::timingNames;
std::vector<int> Profiler::timingLevels;

void Profiler::addNewTiming(std::string name, struct timeval delta) {
  timingDiffs.push_back(delta);
  timingNames.push_back(name);
  timingLevels.push_back(timingLevel);
}

void Profiler::increaseTimingLevel() {
  timingLevel++;
}

void Profiler::decreaseTimingLevel() {
  timingLevel--;
}

void Profiler::print(unsigned int numIters) {
  for (int i=0; i < timingDiffs.size(); i++) {
    std::cout << timingLevels[i];
    std::cout << std::setw(10) << (timingDiffs[i].tv_sec*1000000)+timingDiffs[i].tv_usec;
    std::cout << " usec.    " << timingNames[i] << "\n";
  }
  struct timeval lastOne = timingDiffs.back();
  std::cout << "0 " << std::setw(10) << ((lastOne.tv_sec*1000000)+lastOne.tv_usec) / (double)numIters << " usec.    perIteration\n";
}

int Profiler::timediff(struct timeval *res, struct timeval *x, struct timeval *y) {
   /* Perform the carry for the later subtraction by updating y. */
   if (x->tv_usec < y->tv_usec) {
      int nsec = (y->tv_usec - x->tv_usec) / 1000000 + 1;
      y->tv_usec -= 1000000 * nsec;
      y->tv_sec += nsec;
   }
   if (x->tv_usec - y->tv_usec > 1000000) {
      int nsec = (x->tv_usec - y->tv_usec) / 1000000;
      y->tv_usec += 1000000 * nsec;
      y->tv_sec -= nsec;
   }
     
   /* Compute the time remaining to wait.
      tv_usec is certainly positive. */
   res->tv_sec = x->tv_sec - y->tv_sec;
   res->tv_usec = x->tv_usec - y->tv_usec;
   
   /* Return 1 if result is negative. */
   return x->tv_sec < y->tv_sec;
} // end timediff //

