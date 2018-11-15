#include "profiler.h"
#include <iostream>
#include <iomanip>
#include <chrono>
using namespace thundercat;

int Profiler::timingLevel = 0;
bool Profiler::startedSpmv = false;
TimingInfo Profiler::spmvTiming;
std::vector<TimingInfo> Profiler::timingInfos;
std::map<std::string, TimingInfo> Profiler::spmvOverheads;

long long  Profiler::measure(std::function<void()> codeBlock) {
  auto start = std::chrono::high_resolution_clock::now();
  codeBlock();
  auto end = std::chrono::high_resolution_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
  return duration;
}

void Profiler::recordSpmv(std::function<void()> codeBlock) {
  if (startedSpmv == false) {
    startedSpmv = true;
    auto duration = measure(codeBlock);
    spmvTiming.description = "Spmv";
    spmvTiming.duration = duration;
    spmvTiming.level = 1;
  } else {
    std::cerr << "Attempted call Profiler::recordSpmv second time" << std::endl;
    exit(2);
  }

}
void Profiler::recordTime(std::string description, std::function<void()> codeBlock) {
  timingLevel++;
  auto duration = measure(codeBlock);
  TimingInfo info;
  info.level = timingLevel;
  info.duration = duration;
  info.description = description;
  timingInfos.push_back(info);
  timingLevel--;
}

void Profiler::recordSpmvOverhead(std::string description, std::function<void()> codeBlock) {
  if (startedSpmv) {
    auto duration = measure(codeBlock);

    auto info = spmvOverheads[description];

    if(description.compare(info.description)) {
      info.description = description;
      info.level = timingLevel + 1;
      info.duration = duration;

    } else {
      info.duration += duration;
    }
    spmvOverheads[description] = info;
  }
}

void Profiler::print(unsigned int numIters, unsigned int NNZ, unsigned int flopsPerNNZ) {
  timingInfos.push_back(spmvTiming);
  for (auto &info : timingInfos) {
    std::cout << info.level << " ";
    std::cout << std::setw(10) << info.duration;
    std::cout << " usec.    " << info.description << std::endl;
  }

  long long totalSpmvOverhead = 0;

  if (spmvOverheads.size()) {
    std::cout << std::endl <<"Spmv Overheads: " << std::endl;
    for (auto &entry : spmvOverheads) {
      auto info = entry.second;
      std::cout << std::setw(12) << info.duration;
      std::cout << " usec.    " << info.description<< std::endl;
      totalSpmvOverhead += info.duration;
    }
  }

  std::cout << std::endl;
  long long  netSpmvDuration = spmvTiming.duration - totalSpmvOverhead;

  if (spmvOverheads.size()) {
    std::cout << std::setw(12) << netSpmvDuration;
    std::cout << " usec.    " << spmvTiming.description << " w/o Overheads" << std::endl;
  }
  float durationPerIteration = netSpmvDuration / numIters;

  std::cout << std::setw(12) << durationPerIteration << " usec.    perIteration" <<  std::endl;
  std::cout << std::setw(12) << (flopsPerNNZ * NNZ) / durationPerIteration / 1000 << " GFlops   perIteration" <<  std::endl;
  std::cout << std::setw(12) << numIters << " times    iterated\n";

}
