#include "profiler.h"
#include <iostream>
#include <iomanip>
#include <chrono>
using namespace spMVgen;

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

void Profiler::print(unsigned int numIters) {
  for (auto &info : timingInfos) {
    std::cout << info.level << " ";
    std::cout << std::setw(10) << info.duration;
    std::cout << " usec.    " << info.description << "\n";
  }
  auto &lastOne = timingInfos.back();
  std::cout << "0 " << std::setw(10) << lastOne.duration / (double)numIters << " usec.    perIteration\n";
  std::cout << "0 " << std::setw(10) << numIters << " times    iterated\n";
}
