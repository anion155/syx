#include "./flag.h"
#include "./nob.h"

#pragma once

bool define_args();
bool build_main(int argc, char **argv);

void usage(FILE *stream) {
  fprintf(stream, "usage: ./nob [<options>] [--]\n");
  fprintf(stream, "options:\n");
  flag_print_options(stream);
}

int main(int argc, char **argv) {
  NOB_GO_REBUILD_URSELF(argc, argv);

  bool *flag_help = flag_bool("help", false, "Print this help to stdout and exit with 0");
  if (!define_args()) return 1;
  if (!flag_parse(argc, argv)) {
    usage(stderr);
    flag_print_error(stderr);
    return 1;
  }
  if (*flag_help) {
    usage(stdout);
    return 0;
  }
  argc = flag_rest_argc();
  argv = flag_rest_argv();

  if (!build_main(argc, argv)) return 1;
  return 0;
}
