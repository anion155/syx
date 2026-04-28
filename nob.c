#include <stdbool.h>

struct ContextPaths {
  const char *src;
  const char *vendor;
  const char *build;
  const char *output;
};

struct ContextFlags {
  bool run;
  bool debug;
  bool watch;
};

#define NONOB_IMPL
#include "./vendor/nonob.h"

int watch_and_rebuild();

int main(int argc, char **argv) {
  NOB_GO_REBUILD_URSELF(argc, argv);
  ctx.argc = argc;
  ctx.argv = argv;
  initialize_context();

  ctx.paths.src = temp_sprintf("%s/src", ctx.root);
  ctx.paths.vendor = temp_sprintf("%s/vendor", ctx.root);
  ctx.paths.build = temp_sprintf("%s/build", ctx.root);
  ctx.paths.output = temp_sprintf("%s/syx", ctx.paths.build);

  flag_bool_var(&ctx.flags.run, "run", false, "Run result, when provided arguments after '--'");
  flag_bool_var(&ctx.flags.debug, "g", false, "Build with debugger support");
  flag_bool_var(&ctx.flags.watch, "w", false, "Watch for changes and rebuild");
  flags_parse();

  if (ctx.flags.watch) return watch_and_rebuild();

  if (!nob_mkdir_if_not_exists(ctx.paths.build)) goto fail;

  nob_cc(&ctx.cmd);
  nob_cc_flags(&ctx.cmd);
  nob_cmd_append(&ctx.cmd, temp_sprintf("-I%s", ctx.paths.src));
  nob_cmd_append(&ctx.cmd, temp_sprintf("-I%s", ctx.paths.vendor));
  if (ctx.flags.debug) nob_cmd_append(&ctx.cmd, "-ggdb");
  nob_cc_inputs(&ctx.cmd, "-std=c23");
#ifdef __APPLE__
  nob_cmd_append(&ctx.cmd, "-ledit");
#else
  nob_cmd_append(&cmd, "-lreadline");
#endif
  nob_cc_inputs(&ctx.cmd, temp_sprintf("%s/main.c", ctx.paths.src));
  nob_cc_output(&ctx.cmd, ctx.paths.output);
  append_cmd_to_cdb();
  if (!nob_cmd_run(&ctx.cmd)) goto fail;

  if (ctx.flags.run) {
    if (ctx.flags.debug) nob_cmd_append(&ctx.cmd, "lldb");
    nob_cmd_append(&ctx.cmd, ctx.paths.output);
    if (ctx.flags.debug) nob_cmd_append(&ctx.cmd, "--");
    if (ctx.argc) nob_da_append_many(&ctx.cmd, ctx.argv, ctx.argc);
    if (!nob_cmd_run(&ctx.cmd)) goto fail;
  }

  deinitialize_context();
  return 0;
fail:
  deinitialize_context();
  return 1;
}

int watch_and_rebuild() {
  TODO("watch_and_rebuild");
  // printf("Watching for changes...\n");

  // while (1) {
  //   Nob_File_Paths paths = {0};
  //   nob_read_entire_dir(ctx.paths.src, &paths);
  //   da_foreach(const char *, path, &paths) {
  //     path;
  //   }

  //   while (!nob_needs_rebuild(temp_sprintf("", ctx.paths.build, ctx.paths.output), paths.items, paths.count)) {
  //     usleep(200000);
  //   }
  //   printf("\nChange detected! Running...\n");
  // }
}
