#include "profiler.h"
#include <iostream>
#include <iomanip>
#include <chrono>
using namespace thundercat;

int Profiler::timingLevel = 0;
std::vector<TimingInfo> Profiler::timingInfos;

void Profiler::recordTime(std::string description, std::function<void()> codeBlock) {
  timingLevel++;
  auto start = std::chrono::high_resolution_clock::now();
  codeBlock();
  auto end = std::chrono::high_resolution_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
  TimingInfo info;
  info.level = timingLevel;
  info.duration = duration;
  info.description = description;
  timingInfos.push_back(info);
  timingLevel--;
}

void Profiler::print(unsigned int numIters, unsigned int NNZ, unsigned int flopsPerNNZ) {
  for (auto &info : timingInfos) {
    std::cout << info.level << " ";
    std::cout << std::setw(10) << info.duration;
    std::cout << " usec.    " << info.description << std::endl;
  }
  auto &lastOne = timingInfos.back();
  auto x = timingInfos.back();

  std::cout << "0 " << std::setw(10) << lastOne.duration / (float)numIters << " usec.    perIteration" <<  std::endl;
  std::cout << "0 " << std::setw(10) << (flopsPerNNZ * NNZ) / (lastOne.duration / (float)numIters) / 1000 << " GFlops   perIteration" <<  std::endl;
  std::cout << "0 " << std::setw(10) << numIters << " times    iterated\n";

}
