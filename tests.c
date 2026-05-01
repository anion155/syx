#include <libgen.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#define NOB_IMPL
#include <nob.h>

typedef struct Test_File {
  const char *id;
  const char *relative_path;
  const char *absolute_path;
} Test_File;

typedef struct Test_Files {
  Test_File *items;
  size_t count;
  size_t capacity;
} Test_Files;

struct NoNob_Context_Storage {
  const char *root_path;
  const char *src_path;
  const char *syx_path;
  const char *tests_path;
  const char *outputs_path;
  Test_Files tests;
  bool update_snapshots_flag;
  struct winsize window;
};

#define NONOB_IMPL
#include <nonob.h>

Test_File make_test(const char *relative_path, const char *absolute_path) {
  Test_File test = {0};
  size_t length = strlen(relative_path) - 2;
  char id[length + 1];
  for (size_t index = 0; index < length; index += 1) {
    id[index] = relative_path[2 + index] == '/' ? '_' : relative_path[2 + index];
  }
  id[length] = 0;
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

typedef enum Test_Result_Status {
  TEST_RESULT_STATUS_UPDATE = -3,
  TEST_RESULT_STATUS_NEW = -2,
  TEST_RESULT_STATUS_PASS = -1,
  TEST_RESULT_STATUS_FAIL,
  TEST_RESULT_STATUS_DIFF,
  TEST_RESULT_STATUS_EMPTY,
} Test_Result_Status;

typedef struct Test_Result {
  const char *id;
  Test_Result_Status status;
  uint64_t elapsed;

  union {
    const char *reason;
    String_View output;
    String_View snapshot;
    String_View source_code;
    String_View diff;
  };
} Test_Result;

typedef struct Test_Results {
  Test_Result *items;
  size_t count;
  size_t capacity;
} Test_Results;

#define ESC "\x1b["

#define RESET ESC "0m"
#define BOLD ESC "1m"
#define DIM ESC "2m"
#define F_RED ESC "31m"
#define F_GREEN ESC "32m"
#define F_YELLOW ESC "33m"
#define F_BLACK ESC "38;2;0;0;0m"
#define F_WHITE ESC "37m"
#define BG_RED ESC "41m"
#define BG_GREEN ESC "42m"
#define BG_YELLOW ESC "43m"
#define BG_BLUE ESC "44m"
#define BG_MAGENTA ESC "45m"
#define BG_CYAN ESC "46m"

#define MOVE_CURSOR_UP(lines) ESC STRINGIFY(lines) "A"

#define TAG_UPD BOLD BG_MAGENTA F_BLACK " UPD " RESET
#define TAG_NEW BOLD BG_CYAN F_BLACK " NEW " RESET
#define TAG_PASS BOLD BG_GREEN F_BLACK " PASS " RESET
#define TAG_FAIL BOLD BG_RED F_BLACK " FAIL " RESET
#define TAG_RUNS BOLD BG_YELLOW F_BLACK " RUNS " RESET
#define TAG_TIME DIM "(%.2Lfs)" RESET

#define ICON_FAIL F_WHITE "●" RESET
#define ICON_NEW F_WHITE "◎" RESET

#define MIN(a, b) ((a) < (b) ? (a) : (b))

Test_Result run_test(Test_File test) {
  const char *output_path = temp_sprintf("%s/%s.output", ctx.s->outputs_path, test.id);
  const char *error_path = temp_sprintf("%s/%s.error", ctx.s->outputs_path, test.id);
  const char *snapshot_path = temp_sprintf("%s.snapshot", test.absolute_path);
  fprintf(stdout, "\r " TAG_RUNS "  %s\n", test.relative_path);
  fflush(stdout);

  Test_Result result = {.id = test.id, .status = TEST_RESULT_STATUS_PASS};
#define to_fail(_status, _reason) \
  do {                            \
    result.status = (_status);    \
    result.reason = (_reason);    \
    goto finish;                  \
  } while (0)
  String_Builder source_sb = {0};
  if (!nob_read_entire_file(test.absolute_path, &source_sb)) to_fail(TEST_RESULT_STATUS_FAIL, "Failed to read source code");
  String_View source_code = sb_to_sv(source_sb);

  result.elapsed = nob_nanos_since_unspecified_epoch();
  nob_cmd_append(&ctx.cmd, ctx.s->syx_path, "-x", test.absolute_path);
  if (!nob_cmd_run(&ctx.cmd, .stdout_path = output_path, .stderr_path = error_path)) {
    result.elapsed = nob_nanos_since_unspecified_epoch() - result.elapsed;
    to_fail(TEST_RESULT_STATUS_FAIL, "Failed to run test");
  }
  result.elapsed = nob_nanos_since_unspecified_epoch() - result.elapsed;
  long double seconds = (long double)result.elapsed / (long double)NANOS_PER_SEC;

  if (!nob_file_exists(output_path)) to_fail(TEST_RESULT_STATUS_FAIL, "Test runner failed to store output");
  String_Builder output_sb = {0};
  if (!nob_read_entire_file(output_path, &output_sb)) to_fail(TEST_RESULT_STATUS_FAIL, "Failed to read test's output");
  String_View output = sb_to_sv(output_sb);
  if (!output.count) to_fail(TEST_RESULT_STATUS_FAIL, "Output is empty");

  if (!nob_file_exists(snapshot_path)) {
    if (!ctx.s->update_snapshots_flag) to_fail(TEST_RESULT_STATUS_EMPTY, "Snapshot does not exists, to store current snapshot use `-update` option");
    nob_write_entire_file(snapshot_path, output.data, output.count);
    result.status = TEST_RESULT_STATUS_NEW;
  }
  String_Builder snapshot_sb = {0};
  if (!nob_read_entire_file(snapshot_path, &snapshot_sb)) to_fail(TEST_RESULT_STATUS_FAIL, "Failed to read test's snapshot");
  String_View snapshot = sb_to_sv(snapshot_sb);
  if (!snapshot.count) to_fail(TEST_RESULT_STATUS_FAIL, "Snapshot is empty");

  if (!sv_eq(output, snapshot)) {
    if (!ctx.s->update_snapshots_flag) to_fail(TEST_RESULT_STATUS_DIFF, "Output does not match snapshot");
    nob_write_entire_file(snapshot_path, output.data, output.count);
    result.status = TEST_RESULT_STATUS_UPDATE;
  }
  result.output = output;
  result.snapshot = snapshot;
  result.source_code = source_code;

finish:
  fprintf(stdout, MOVE_CURSOR_UP(1));
  switch (result.status) {
    case TEST_RESULT_STATUS_UPDATE: {
      fprintf(stdout, "\r " TAG_UPD "  %s " TAG_TIME "\n", test.relative_path, seconds);
    } break;
    case TEST_RESULT_STATUS_NEW: {
      fprintf(stdout, "\r " TAG_NEW "  %s " TAG_TIME "\n", test.relative_path, seconds);
    } break;
    case TEST_RESULT_STATUS_PASS: {
      fprintf(stdout, "\r " TAG_PASS "  %s " TAG_TIME "\n", test.relative_path, seconds);
    } break;
    case TEST_RESULT_STATUS_FAIL: {
      fprintf(stdout, "\r " TAG_FAIL "  %s " TAG_TIME "\n", test.relative_path, seconds);
      fprintf(stdout, "   " ICON_FAIL " Error: %s\n", result.reason);
    } break;
    case TEST_RESULT_STATUS_DIFF: {
      fprintf(stdout, "\r " TAG_FAIL "  %s " TAG_TIME "\n", test.relative_path, seconds);
      fprintf(stdout, "   " ICON_FAIL " > should\n");

      const char *diff_path = temp_sprintf("%s/%s.diff", ctx.s->outputs_path, test.id);
      nob_cmd_append(&ctx.cmd, "git", "diff", "--no-index", "--unified=1", snapshot_path, output_path);
      nob_cmd_run(&ctx.cmd, .stdout_path = diff_path);
      String_Builder diff_sb = {0};
      nob_read_entire_file(diff_path, &diff_sb);
      String_View it = sb_to_sv(diff_sb);
      int width = ctx.s->window.ws_col - 3 - 1 - 3 - 3;
      while (it.count) {
        String_View line = sv_chop_by_delim(&it, '\n');
        if (strncmp(line.data, "---", 3) == 0) {
        } else if (strncmp(line.data, "+++", 3) == 0) {
        } else if (strncmp(line.data, "@@", 2) == 0) {
          fprintf(stdout, "      | ...\n");
          int old_start, old_len, new_start, new_len, old_ind = 0, new_ind = 0;
          char *p = strchr(line.data, '-');
          if (p) {
            old_start = strtol(p + 1, &p, 10);
            if (*p == ',') old_len = strtol(p + 1, &p, 10);
          }
          p = strchr(p, '+');
          if (p) {
            new_start = strtol(p + 1, &p, 10);
            if (*p == ',') new_len = strtol(p + 1, &p, 10);
          }

          while (it.count && (old_len || new_len)) {
            line = sv_chop_by_delim(&it, '\n');
            if (line.data[0] == ' ') {
              old_len -= 1;
              new_len -= 1;
              fprintf(stdout, "%3d %3d | %.*s\n", old_start + old_ind++, new_start + new_ind++, MIN((int)line.count - 1, width), line.data + 1);
            } else if (line.data[0] == '-') {
              old_len -= 1;
              fprintf(stdout, "%3d    " F_RED "-" RESET "| " F_RED "%.*s" RESET "\n", old_start + old_ind++, (int)MIN((int)line.count - 1, width), line.data + 1);
            } else if (line.data[0] == '+') {
              new_len -= 1;
              fprintf(stdout, "    %3d" F_GREEN "+" RESET "| " F_GREEN "%.*s" RESET "\n", new_start + new_ind++, (int)MIN((int)line.count - 1, width), line.data + 1);
            }
          }
        }
      }

      fprintf(stdout, "\n");
    } break;
    case TEST_RESULT_STATUS_EMPTY: {
      fprintf(stdout, "\r " TAG_FAIL "  %s " TAG_TIME "\n", test.relative_path, seconds);
      fprintf(stdout, "   " ICON_NEW " %s\n", result.reason);
    } break;
  }
  fflush(stdout);
  return result;
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

  if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ctx.s->window) != 0) {
    ctx.s->window.ws_row = 40;
    ctx.s->window.ws_col = 80;
  }

  flag_bool_var(&ctx.s->update_snapshots_flag, "update", false, "Update snapshots");
  nonob_parse_options();

  if (!find_tests(ctx.s->tests_path, ".")) goto fail;
  if (!nob_mkdir_if_not_exists(ctx.s->outputs_path)) goto fail;

  nob_minimal_log_level = NOB_NO_LOGS;
  Test_Results results = {0};
  size_t passed = 0;
  uint64_t elapsed = nob_nanos_since_unspecified_epoch();
  da_foreach(Test_File, test, &ctx.s->tests) {
    Test_Result result = run_test(*test);
    da_append(&results, result);
    if (result.status < 0) passed += 1;
  }
  elapsed = nob_nanos_since_unspecified_epoch() - elapsed;
  fprintf(stdout, "\n");
  fprintf(stdout, "%-11s " F_RED "%zu failed" RESET ", %zu passed, %zu total\n", BOLD "Tests:" RESET, passed, results.count - passed, results.count);
  fprintf(stdout, "%-11s %.2Lfs\n", BOLD "Time:" RESET, (long double)elapsed / (long double)NANOS_PER_SEC);
  nob_minimal_log_level = NOB_INFO;

  nonob_deinitialize();
  return 0;
fail:
  nonob_deinitialize();
  return 1;
}
