#include <libgen.h>
#include <stdlib.h>

#define CLI_IMPL
#include <cli.h>
#define NOB_IMPL
#include <nob.h>
#define TESTS_IMPL
#include <tests.h>

struct NoNob_Context_Storage {
  const char *root_path;
  const char *src_path;
  const char *syx_path;
  const char *tests_path;
  const char *outputs_path;
};

#define NONOB_IMPL
#include <nonob.h>

#define SYX_RUNNER_NAME "syx"

void syx_runner_runner(Tests_Context *tests_ctx, Test *test, const char *sout_path, const char *serr_path) {
  UNUSED(tests_ctx);
  nob_cmd_append(&ctx.cmd, ctx.s->syx_path, test->absolute_path);
  nob_cmd_run(&ctx.cmd, .stdout_path = sout_path, .stderr_path = serr_path);
}

bool find_tests_recursively(Tests *tests, const char *directory, const char *base) {
  Nob_File_Paths files = {0};
  if (!nob_read_entire_dir(directory, &files)) return false;
  da_foreach(const char *, name, &files) {
    if (**name == '.') continue;
    const char *path = temp_sprintf("%s/%s", base, *name);
    const char *absolute_path = temp_sprintf("%s/%s", directory, *name);
    if (nob_get_file_type(absolute_path) == NOB_FILE_DIRECTORY) {
      if (!find_tests_recursively(tests, absolute_path, path)) return false;
      continue;
    }
    if (!sv_ends_with_cstr(sv_from_cstr(path), ".syx")) continue;
    Test test = make_test(path, absolute_path);
    test.kind = SYX_RUNNER_NAME;
    da_append(tests, test);
  }
  return true;
}

int main(int argc, char **argv) {
  nonob_initialize(argc, argv, .disable_ccjson = true);
  ctx.s->root_path = realpath(temp_sprintf("%s/..", ctx.exe_path), NULL);
  ctx.s->src_path = temp_sprintf("%s/src", ctx.s->root_path);
  ctx.s->syx_path = temp_sprintf("%s/build/syx", ctx.s->root_path);
  ctx.s->tests_path = temp_sprintf("%s/tests", ctx.s->root_path);
  ctx.s->outputs_path = temp_sprintf("%s/build/tests-outputs", ctx.s->root_path);

  bool *save_snapshots = flag_bool("update", false, "Update snapshots");
  nonob_parse_options();

  Tests_Context tests_ctx = make_tests_context(.build_path = ctx.s->outputs_path);

  Tests_Snapshot_Runner syx_runner = {.runner = syx_runner_runner, .save_snapshots = *save_snapshots};
  init_tests_snapshot_runner(&syx_runner);
  tests_register_runner(&tests_ctx, "syx", &syx_runner.base);

  Tests tests = {0};
  if (!find_tests_recursively(&tests, ctx.s->tests_path, ".")) goto fail;

  run_tests(&tests_ctx, tests);

  nonob_deinitialize();
  return 0;
fail:
  nonob_deinitialize();
  return 1;
}
