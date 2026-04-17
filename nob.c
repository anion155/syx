#define FLAG_IMPLEMENTATION
#include "./vendor/flag.h"
#undef FLAG_IMPLEMENTATION
#define NOB_IMPLEMENTATION
#include "./vendor/nob.h"
#undef NOB_IMPLEMENTATION
#include "./vendor/nob_main.h"

#define SRC_DIR "./src"
#define VENDOR_DIR "./vendor"
#define BUILD_DIR "./build"

Nob_Cmd cmd = {0};
bool *flag_run;
bool *flag_debug;

bool define_args() {
  flag_run   = flag_bool("run", false,  "Run result, when provided arguments after '--'");
  flag_debug = flag_bool("g", false,    "Build with debugger support");
  return true;
}

bool build_main(int argc, char **argv) {
  if (!nob_mkdir_if_not_exists(BUILD_DIR)) return false;

  nob_cc(&cmd);
  nob_cc_flags(&cmd);
  #if defined(__APPLE__)
    nob_cmd_append(&cmd, "-framework", "Cocoa", "-framework", "AudioToolbox");
  #else
    nob_cmd_append(&cmd, "-lX11", "-lasound");
  #endif
  nob_cmd_append(&cmd, "-I"SRC_DIR);
  nob_cmd_append(&cmd, "-I"VENDOR_DIR);
  if (*flag_debug) nob_cmd_append(&cmd, "-g");
  nob_cc_inputs(&cmd, SRC_DIR"/main.c");
  nob_cc_output(&cmd, BUILD_DIR"/bogo");
  if (!nob_cmd_run_opt(&cmd, (Nob_Cmd_Opt){})) return false;

  if (*flag_run) {
    nob_cmd_append(&cmd, BUILD_DIR"/bogo");
    nob_da_append_many(&cmd, argv, argc);
    if (!nob_cmd_run(&cmd)) return false;
  }

  return true;
}
