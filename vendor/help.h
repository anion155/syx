#include "./flag.h"
#include "./nob.h"

void usage(FILE *stream) {
  fprintf(stream, "usage: ./nob [<options>] [--]\n");
  fprintf(stream, "options:\n");
  flag_print_options(stream);
}

void flag_parse_with_help(int *argc, char ***argv) {
  bool *flag_help = flag_bool("help", false, "Print this help to stdout and exit with 0");
  if (!flag_parse(*argc, *argv)) {
    usage(stderr);
    flag_print_error(stderr);
    exit(1);
  }
  if (*flag_help) {
    usage(stdout);
    exit(0);
  }
  *argc = flag_rest_argc();
  *argv = flag_rest_argv();
}
