#ifndef SPMV_BENCHMARKING_PARSE_OPTIONS_H
#define SPMV_BENCHMARKING_PARSE_OPTIONS_H

#include <string>
#include <memory>
struct CliOptions {
    const std::string mtxFile;
    const std::string method;
    const long threads;
    const long iters;
    const long warmups;
    const bool debug;
    const bool dumpOutput;

};

std::unique_ptr<CliOptions> parseCliOptions(int argc, const char *argv[]);

#endif //SPMV_BENCHMARKING_PARSE_OPTIONS_H
