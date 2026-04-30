#include <libgen.h>
#include <stdlib.h>
#define NOB_IMPL
#include <nob.h>

typedef struct Test {
  const char *id;
  const char *relative_path;
  const char *absolute_path;
} Test;

typedef struct Tests {
  Test *items;
  size_t count;
  size_t capacity;
} Tests;

struct NoNob_Context_Storage {
  const char *root_path;
  const char *src_path;
  const char *syx_path;
  const char *tests_path;
  const char *outputs_path;
  Tests tests;
};

#define NONOB_IMPL
#include <nonob.h>

Test make_test(const char *relative_path, const char *absolute_path) {
  Test test = {0};
  size_t length = strlen(relative_path) - 2;
  char id[length + 1];
  for (size_t index = 0; index < length; index += 1) {
    id[index] = relative_path[2 + index] == '/' ? '_' : relative_path[2 + index];
  }
  id[length - 1] = 0;
  test.id = temp_strdup(id);
  test.relative_path = relative_path;
  test.absolute_path = absolute_path;
  return test;
}

bool find_tests(const char *directory, const char *base) {
  Nob_File_Paths files = {0};
  if (!nob_read_entire_dir(directory, &files)) return false;
  da_foreach(const char *, name, &files) {
    if (**name == '.') continue;
    const char *path = temp_sprintf("%s/%s", base, *name);
    const char *absolute_path = temp_sprintf("%s/%s", directory, *name);
    if (nob_get_file_type(absolute_path) == NOB_FILE_DIRECTORY) {
      if (!find_tests(absolute_path, path)) return false;
      continue;
    }
    if (!sv_ends_with_cstr(sv_from_cstr(path), ".spec.syx")) continue;
    da_append(&ctx.s->tests, make_test(path, absolute_path));
  }
  return true;
}

int main(int argc, char **argv) {
  UNUSED(ht__find_or_put);
  UNUSED(ht__find_and_delete);
  UNUSED(ht__reset);
  UNUSED(ht__free);

  nonob_initialize(argc, argv, .disable_ccjson = true);
  ctx.s->root_path = realpath(temp_sprintf("%s/..", ctx.exe_path), NULL);
  ctx.s->src_path = temp_sprintf("%s/src", ctx.s->root_path);
  ctx.s->syx_path = temp_sprintf("%s/build/syx", ctx.s->root_path);
  ctx.s->tests_path = temp_sprintf("%s/tests", ctx.s->root_path);
  ctx.s->outputs_path = temp_sprintf("%s/build/tests-outputs", ctx.s->root_path);
  nonob_parse_options();

  if (!find_tests(ctx.s->tests_path, ".")) goto fail;
  if (!nob_mkdir_if_not_exists(ctx.s->outputs_path)) goto fail;

  da_foreach(Test, test, &ctx.s->tests) {
    const char *output_path = temp_sprintf("%s/%s", ctx.s->outputs_path, test->id);
    printf(" %5s  %s\n", "RUN", test->relative_path);

    uint64_t timespend = nob_nanos_since_unspecified_epoch();
    nob_cmd_append(&ctx.cmd, ctx.s->syx_path);
    nob_cmd_append(&ctx.cmd, test->absolute_path);
    if (!nob_cmd_run(&ctx.cmd, .stdout_path = output_path)) goto fail;
    timespend = nob_nanos_since_unspecified_epoch() - timespend;
    printf(" %5s  %s (%.1Lfs)\n", "DONE", test->relative_path, (long double)timespend / NANOS_PER_SEC);
  }

  nonob_deinitialize();
  return 0;
fail:
  nonob_deinitialize();
  return 1;
}
