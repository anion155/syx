#include "vendor/nob.h"

#include <stdbool.h>

struct NoNob_Context_Storage {
  const char *src_path;
  const char *vendor_path;
  const char *build_path;
  const char *syx_path;
  const char *tests_path;

  bool build_debug;
  bool build_sanitizer;
  bool run_leaks;
  bool run_watch;
};

#define NONOB_IMPL
#include "./vendor/nonob.h"

void command_build_init(NoNob_Command *command) {
  flag_c_bool_var(command->flags, &ctx.s->build_debug, "g", false, "Build with debug symbols");
  flag_c_bool_var(command->flags, &ctx.s->build_sanitizer, "sanitize", false, "Enable compiler memory leak detection");
}

bool command_build_run() {
  if (!nob_mkdir_if_not_exists(ctx.s->build_path)) return false;

  nob_cc(&ctx.cmd);
  nob_cc_flags(&ctx.cmd);
  nob_cmd_append(&ctx.cmd, temp_sprintf("-I%s", ctx.s->src_path));
  nob_cmd_append(&ctx.cmd, temp_sprintf("-I%s", ctx.s->vendor_path));
  if (ctx.s->build_sanitizer) nob_cmd_append(&ctx.cmd, "-ggdb", "-fsanitize=address");
  else if (ctx.s->build_debug) nob_cmd_append(&ctx.cmd, "-ggdb");
  nob_cc_inputs(&ctx.cmd, "-std=c23");
  nob_cc_inputs(&ctx.cmd, temp_sprintf("%s/main.c", ctx.s->src_path));
  nob_cc_output(&ctx.cmd, ctx.s->syx_path);
#ifdef __APPLE__
  nob_cmd_append(&ctx.cmd, "-ledit");
#else
  nob_cmd_append(&cmd, "-lreadline");
#endif
  nonob_append_cmd_to_ccjson();
  if (!nob_cmd_run(&ctx.cmd)) return false;

  return true;
}

void command_run_init(NoNob_Command *command) {
  command_build_init(command);
  flag_c_bool_var(command->flags, &ctx.s->run_watch, "w", false, "Re run app on input files changes");
  flag_c_bool_var(command->flags, &ctx.s->run_leaks, "leaks", false, "Run with platform memory leak detector");
}

int watch_and_rebuild() { TODO("watch_and_rebuild"); }

bool command_run_run() {
  if (ctx.s->run_leaks && ctx.s->build_sanitizer) {
    fprintf(stderr, "ERROR: flags -leaks and -sanitize are incompatible with each other\n");
    NoNob_Command *run_command = ht_find(&ctx.commands, "run");
    if (ctx.usage) ctx.usage(stderr, run_command);
    else nonob_default_usage(stderr, run_command);
    exit(1);
  }
  if (ctx.s->run_watch) return watch_and_rebuild();
  if (ctx.s->run_leaks) ctx.s->build_debug = true;
  if (!command_build_run()) return false;

#if defined(__APPLE__)
  if (ctx.s->run_leaks) nob_cmd_append(&ctx.cmd, "leaks", "--atExit", "--");
#elif defined(__linux__)
  if (ctx.s->run_memory_test) nob_cmd_append(&ctx.cmd, "valgrind", "--leak-check=full");
#endif
  nob_cmd_append(&ctx.cmd, ctx.s->syx_path);
  if (ctx.argc) nob_da_append_many(&ctx.cmd, ctx.argv, ctx.argc);
  if (!nob_cmd_run(&ctx.cmd)) return false;

  return true;
}

void command_debug_init(NoNob_Command *command) {}

bool command_debug_run() {
  ctx.s->build_debug = true;
  if (!command_build_run()) return false;

  nob_cmd_append(&ctx.cmd, "lldb");
  nob_cmd_append(&ctx.cmd, ctx.s->syx_path);
  nob_cmd_append(&ctx.cmd, "--");
  if (ctx.argc > 0 && strcmp(ctx.argv[0], "--") == 0) nob_shift(ctx.argv, ctx.argc);
  if (ctx.argc) nob_da_append_many(&ctx.cmd, ctx.argv, ctx.argc);
  if (!nob_cmd_run(&ctx.cmd)) return false;

  return true;
}

#define command_tests_init NULL

bool command_tests_run() {
  if (!command_build_run()) return false;

  nob_cc(&ctx.cmd);
  nob_cc_flags(&ctx.cmd);
  nob_cmd_append(&ctx.cmd, temp_sprintf("-I%s", ctx.s->vendor_path));
  nob_cmd_append(&ctx.cmd, "-std=c23");
  nob_cmd_append(&ctx.cmd, "-ggdb");
  const char *tests_c = temp_sprintf("%s/tests.c", ctx.exe_path);
  nob_cc_inputs(&ctx.cmd, tests_c);
  nob_cc_output(&ctx.cmd, ctx.s->tests_path);
  nonob_append_cmd_to_ccjson();
  Nob_File_Paths deps = {0};
  da_append(&deps, tests_c);
  da_append(&deps, temp_sprintf("%s/nonob.h", ctx.s->vendor_path));
  da_append(&deps, temp_sprintf("%s/tests.h", ctx.s->vendor_path));
  nonob_append_cmd_to_ccjson();
  if (nob_needs_rebuild(ctx.s->tests_path, deps.items, deps.count)) {
    if (!nob_cmd_run(&ctx.cmd)) return false;
  } else {
    ctx.cmd.count = 0;
  }

  nob_cmd_append(&ctx.cmd, ctx.s->tests_path);
  if (ctx.argc > 0 && strcmp(ctx.argv[0], "--") == 0) nob_shift(ctx.argv, ctx.argc);
  if (ctx.argc) nob_da_append_many(&ctx.cmd, ctx.argv, ctx.argc);
  if (!nob_cmd_run(&ctx.cmd)) return false;

  return true;
}

bool delete_recursively(const char *file_path) {
  if (!nob_file_exists(file_path)) return true;
  if (nob_get_file_type(file_path) != NOB_FILE_DIRECTORY) return nob_delete_file(file_path);
  Nob_File_Paths children = {0};
  nob_read_entire_dir(file_path, &children);
  bool success = true;
  da_foreach(const char *, child, &children) {
    if (strcmp(*child, ".") == 0) continue;
    if (strcmp(*child, "..") == 0) continue;
    success = delete_recursively(temp_sprintf("%s/%s", file_path, *child)) && success;
  }
  if (!nob_delete_file(file_path)) return false;
  return success;
}

#define command_clean_init NULL

bool command_clean_run() {
  delete_recursively(ctx.s->build_path);
  nob_delete_file(temp_sprintf("%s/nob.old", ctx.exe_path));
  nob_delete_file(temp_sprintf("%s/compile_commands.json", ctx.exe_path));
  return true;
}

#define command_playground_init NULL

bool command_playground_run() {
  nob_cc(&ctx.cmd);
  nob_cc_flags(&ctx.cmd);
  nob_cmd_append(&ctx.cmd, temp_sprintf("-I%s", ctx.s->src_path));
  nob_cmd_append(&ctx.cmd, temp_sprintf("-I%s", ctx.s->vendor_path));
  nob_cmd_append(&ctx.cmd, "-ggdb");
  nob_cc_inputs(&ctx.cmd, "-std=c23");
  nob_cc_inputs(&ctx.cmd, temp_sprintf("%s/playground.c", ctx.s->src_path));
  nob_cc_output(&ctx.cmd, temp_sprintf("%s/playground", ctx.s->build_path));
  nonob_append_cmd_to_ccjson();
  if (!nob_cmd_run(&ctx.cmd)) return false;

  nob_cmd_append(&ctx.cmd, temp_sprintf("%s/playground", ctx.s->build_path));
  if (!nob_cmd_run(&ctx.cmd)) {
    nob_cmd_append(&ctx.cmd, "lldb");
    nob_cmd_append(&ctx.cmd, temp_sprintf("%s/playground", ctx.s->build_path));
    nob_cmd_run(&ctx.cmd);
    return false;
  }

  return true;
}

int main(int argc, char **argv) {
  NOB_GO_REBUILD_URSELF_PLUS(argc, argv, "./vendor/nonob.h");
  nonob_initialize(argc, argv);

  ctx.s->src_path = temp_sprintf("%s/src", ctx.exe_path);
  ctx.s->vendor_path = temp_sprintf("%s/vendor", ctx.exe_path);
  ctx.s->build_path = temp_sprintf("%s/build", ctx.exe_path);
  ctx.s->syx_path = temp_sprintf("%s/syx", ctx.s->build_path);
  ctx.s->tests_path = temp_sprintf("%s/tests", ctx.s->build_path);

  bool *clear = flag_bool("c", false, "Clear terminal before running");
  nonob_define_command(build, "Build project");
  nonob_define_command(run, "Run project, with arguments provided after '--'");
  nonob_define_command(debug, "Run project with lldb");
  nonob_define_command(tests, "Run tests");
  nonob_define_command(clean, "Clean artifacts");
  nonob_define_command(playground, "Playground env");
  nonob_parse_options();
  if (*clear) {
    nob_cmd_append(&ctx.cmd, "clear");
    if (!nob_cmd_run(&ctx.cmd)) goto fail;
  }
  if (!nonob_run_command("build")) goto fail;

  nonob_deinitialize();
  return 0;
fail:
  nonob_deinitialize();
  return 1;
}
