#include <iostream>
#include "docopt.h"
#include "parse_options.h"

static const char USAGE[] =
R"(OzU SRL SpMV Benchmarking.
  Usage:
    spmv_benchmarking <mtxFile> <method>
      [--threads=<num>] [--debug] [--iters=<count>] [--dump-output]
      [--warmups=<count>]
    spmv_benchmarking (-h | --help)
    spmv_benchmarking --version
  Options:
    -h --help                     Show this screen.
    --version                     Show version.
    --debug                       Turn debug mode on.
    --dump-output                 Dump output vector to <method>.out
    --threads=<num>               Number of threads to use [default: 1].
    --iters=<count>               Number of iterations for benchmarking [default: 10].
    --warmups=<count>             Number of warmup iterations before benchmarking [default: 5].

)";

void dumpOptions(std::unique_ptr<CliOptions>& options) {
  std::cout << "Options:" << std::endl <<
            "========" << std::endl <<
            "matrix file: " << options->mtxFile << std::endl <<
            "method     : " << options->method << std::endl <<
            "threads    : " << options->threads << std::endl <<
            "iters      : " << options->iters << std::endl <<
            "warmups    : " << options->warmups << std::endl <<
            "debug      : " << (options->debug ? "true" : "false" )<< std::endl <<
            "dump output: " << (options->dumpOutput ? "true" : "false" )<< std::endl <<
            "==================" << std::endl << std::endl;

}

std::unique_ptr<CliOptions> parseCliOptions(int argc, const char * argv[]) {

  std::map<std::string, docopt::value> args
      = docopt::docopt(USAGE, { argv + 1, argv + argc }, true, "SpTRSV 0.1");

  std::unique_ptr<CliOptions> options(new CliOptions {
      args["<mtxFile>"].asString(),
      args["<method>"].asString(),
      args["--threads"].asLong(),
      args["--iters"].asLong(),
      args["--warmups"].asLong(),
      args["--debug"].asBool(),
      args["--dump-output"].asBool()
  });

  dumpOptions(options);

  return options;

}