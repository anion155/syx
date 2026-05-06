#ifndef TESTS_H
#define TESTS_H

#include <cli.h>
#include <ht.h>
#include <nob.h>
#include <stdlib.h>

typedef struct Test {
  const char *kind;
  const char *id;
  const char *relative_path;
  const char *absolute_path;
} Test;

typedef struct Tests {
  Test *items;
  size_t count;
  size_t capacity;
} Tests;

typedef enum Test_Result_Status {
  TEST_RESULT_STATUS_PASS = -1,
  TEST_RESULT_STATUS_NONE = 0,
  TEST_RESULT_STATUS_FAIL,
} Test_Result_Status;

typedef struct Test_Result {
  const char *id;
  const char *kind;
  int status;
  const char *error_str;
  uint64_t elapsed;
} Test_Result;

typedef struct Test_Results {
  const char *kind;
  Test_Result **items;
  size_t count;
  size_t capacity;
  size_t passed;
  size_t failed;
  uint64_t elapsed;
} Test_Results;

typedef enum Tests_Runner_Tags {
  TESTS_RUNNER_TAG_SNAPSHOT = 0b00001,
  TESTS_RUNNER_TAG_COMPILLER = 0b00010,
} Tests_Runner_Tags;

typedef struct Tests_Context Tests_Context;
typedef struct Tests_Runner Tests_Runner;

struct Tests_Runner {
  Tests_Runner_Tags tags;
  Test_Result *(*create_result)(Tests_Context *ctx, const Tests_Runner *runner);
  void (*free_result)(Test_Result *result);
  void (*run_test)(Tests_Context *ctx, const Tests_Runner *runner, Test *test, Test_Result *result);
};

typedef Ht(const char *, Tests_Runner *, Tests_Runners) Tests_Runners;

struct Tests_Context {
  Nob_Cmd cmd;
  cli_term_size terminal_size;
  const char *build_path;
  Tests_Runners runners;
};

Test make_test(const char *relative_path, const char *absolute_path);
Tests_Context make_tests_context_opt(Tests_Context opt);
#define make_tests_context(...) make_tests_context_opt((Tests_Context){__VA_ARGS__})
void tests_register_runner(Tests_Context *ctx, const char *name, Tests_Runner *runner);
Test_Result *run_test(Tests_Context *ctx, Test test);
Test_Results run_tests(Tests_Context *ctx, Tests tests);

typedef struct Tests_Snapshot_Runner {
  Tests_Runner base;
  void (*runner)(Tests_Context *ctx, Test *test, const char *sout_path, const char *serr_path);
  bool save_snapshots;
} Tests_Snapshot_Runner;

typedef struct Test_Snapshot_Result {
  Test_Result base;
  Nob_String_View source_code;
  Nob_String_View sout_result;
  Nob_String_View sout_diff;
  Nob_String_View serr_result;
  Nob_String_View serr_diff;
} Test_Snapshot_Result;

typedef enum Test_Snapshot_Result_Status {
  TEST_SNAPSHOT_RESULT_STATUS_UPDATED = TEST_RESULT_STATUS_PASS - 1,
  TEST_SNAPSHOT_RESULT_STATUS_DIFF_sout = TEST_RESULT_STATUS_FAIL + 1,
  TEST_SNAPSHOT_RESULT_STATUS_DIFF_serr,
  TEST_SNAPSHOT_RESULT_STATUS_EMPTY_sout,
  TEST_SNAPSHOT_RESULT_STATUS_EMPTY_serr,
} Test_Snapshot_Result_Status;

void init_tests_snapshot_runner(Tests_Snapshot_Runner *runner);

#endif // TESTS_H
#if defined(TESTS_IMPL) && !defined(TESTS_IMPL_C)
#define TESTS_IMPL_C

Test make_test(const char *relative_path, const char *absolute_path) {
  Test test = {0};
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

Tests_Context make_tests_context_opt(Tests_Context opt) {
  return (Tests_Context){
      .cmd = {0},
      .terminal_size = !opt.terminal_size.cols || !opt.terminal_size.rows ? cli_get_window_size() : opt.terminal_size,
      .build_path = opt.build_path,
      .runners = {0},
  };
}

void tests_register_runner(Tests_Context *ctx, const char *name, Tests_Runner *runner) {
  *ht_put(&ctx->runners, name) = runner;
}

void tests_snapshot_print_new(Tests_Context *ctx, Nob_String_View output);
void tests_snapshot_print_diff(Tests_Context *ctx, Nob_String_View diff_output);

#define TAG_UPDT CLI_BOLD CLI_BG_MAGENTA CLI_FG_BLACK " UPDT " CLI_RESET
#define TAG_NEW_ CLI_BOLD CLI_BG_CYAN CLI_FG_BLACK " NEW_ " CLI_RESET
#define TAG_PASS CLI_BOLD CLI_BG_GREEN CLI_FG_BLACK " PASS " CLI_RESET
#define TAG_FAIL CLI_BOLD CLI_BG_RED CLI_FG_BLACK " FAIL " CLI_RESET
#define TAG_RUNS CLI_BOLD CLI_BG_YELLOW CLI_FG_BLACK " RUNS " CLI_RESET
#define TAG_TIME CLI_DIM "(%.2Lfs)" CLI_RESET

#define ICON_FAIL CLI_FG_WHITE "●" CLI_RESET
#define ICON_NEW CLI_FG_WHITE "◎" CLI_RESET

Test_Result *run_test(Tests_Context *ctx, Test test) {
  printf("\r " TAG_RUNS "  %s\n", test.relative_path);
  fflush(stdout);

  Tests_Runner *runner = *ht_find(&ctx->runners, test.kind);

  Test_Result *result = runner->create_result(ctx, runner);
  result->id = test.id;
  result->kind = test.kind;
  result->status = TEST_RESULT_STATUS_NONE;

  runner->run_test(ctx, runner, &test, result);
  long double seconds = (long double)result->elapsed / (long double)NANOS_PER_SEC;

  printf(CLI_MOVE_CURSOR_UP(1));
  if (result->status == TEST_RESULT_STATUS_NONE) {
    printf("\r " TAG_FAIL "  %s " TAG_TIME "\n", test.relative_path, seconds);
    printf("   " ICON_FAIL " Failed to run test\n");
  } else if (runner->tags & TESTS_RUNNER_TAG_SNAPSHOT) {
    Test_Snapshot_Result *snapshot = (Test_Snapshot_Result *)result;
    switch (result->status) {
      case TEST_SNAPSHOT_RESULT_STATUS_UPDATED: {
        printf("\r " TAG_UPDT "  %s " TAG_TIME "\n", test.relative_path, seconds);
      } break;
      case TEST_SNAPSHOT_RESULT_STATUS_DIFF_sout: {
        printf("\r " TAG_FAIL "  %s " TAG_TIME "\n", test.relative_path, seconds);
        printf("   " ICON_FAIL " > sout difference\n");
        tests_snapshot_print_diff(ctx, snapshot->sout_diff);
      } break;
      case TEST_SNAPSHOT_RESULT_STATUS_DIFF_serr: {
        printf("\r " TAG_FAIL "  %s " TAG_TIME "\n", test.relative_path, seconds);
        printf("   " ICON_FAIL " > serr difference\n");
        tests_snapshot_print_diff(ctx, snapshot->sout_diff);
      } break;
      case TEST_SNAPSHOT_RESULT_STATUS_EMPTY_sout: {
        printf("\r " TAG_FAIL "  %s " TAG_TIME "\n", test.relative_path, seconds);
        printf("   " ICON_NEW " %s\n", result->error_str);
        printf("   Expected sout in path: '%s'\n", temp_sprintf("%s.sout", test.absolute_path));
        tests_snapshot_print_new(ctx, snapshot->sout_result);
      } break;
      case TEST_SNAPSHOT_RESULT_STATUS_EMPTY_serr: {
        printf("\r " TAG_FAIL "  %s " TAG_TIME "\n", test.relative_path, seconds);
        printf("   " ICON_NEW " %s\n", result->error_str);
        printf("   Expected serr in path: '%s'\n", temp_sprintf("%s.serr", test.absolute_path));
        tests_snapshot_print_new(ctx, snapshot->serr_result);
      } break;
      default: {
        if (result->status < 0) {
          printf("\r " TAG_PASS "  %s " TAG_TIME "\n", test.relative_path, seconds);
        } else {
          printf("\r " TAG_FAIL "  %s " TAG_TIME "\n", test.relative_path, seconds);
          printf("   " ICON_FAIL " Error: %s\n", result->error_str);
        }
      }
    }
  } else if (result->status < 0) {
    printf("\r " TAG_PASS "  %s " TAG_TIME "\n", test.relative_path, seconds);
  } else {
    printf("\r " TAG_FAIL "  %s " TAG_TIME "\n", test.relative_path, seconds);
    printf("   " ICON_FAIL " Error: %s\n", result->error_str);
  }
  fflush(stdout);
  return result;
}

Test_Results run_tests(Tests_Context *ctx, Tests tests) {
  nob_mkdir_if_not_exists(ctx->build_path);

  Nob_Log_Level log_level = nob_minimal_log_level;
  nob_minimal_log_level = NOB_NO_LOGS;

  Test_Results results = {0};
  results.elapsed = nob_nanos_since_unspecified_epoch();
  da_foreach(Test, test, &tests) {
    Test_Result *result = run_test(ctx, *test);
    da_append(&results, result);
    if (result->status < 0) results.passed += 1;
  }
  results.elapsed = nob_nanos_since_unspecified_epoch() - results.elapsed;
  results.failed = results.count - results.passed;

  nob_minimal_log_level = log_level;

  printf("\n");
  printf("%-11s ", CLI_BOLD "Tests:" CLI_RESET);
  if (results.failed) {
    printf(CLI_FG_RED "%zu failed, " CLI_RESET, results.failed);
  } else {
    printf("%d failed, ", 0);
  }
  printf("%zu passed, %zu total\n", results.passed, results.count);
  printf("%-11s %.2Lfs\n", CLI_BOLD "Time:" CLI_RESET, (long double)results.elapsed / (long double)NANOS_PER_SEC);

  return results;
}

Test_Result *tests_snapshot_runner_create_result(Tests_Context *ctx, const Tests_Runner *runner);
void tests_snapshot_runner_free_result(Test_Result *_result);
void tests_snapshot_runner_run_test(Tests_Context *ctx, const Tests_Runner *_runner, Test *test, Test_Result *base_result);

void init_tests_snapshot_runner(Tests_Snapshot_Runner *runner) {
  runner->base.tags = TESTS_RUNNER_TAG_SNAPSHOT;
  runner->base.create_result = tests_snapshot_runner_create_result;
  runner->base.free_result = tests_snapshot_runner_free_result;
  runner->base.run_test = tests_snapshot_runner_run_test;
}

bool run_diff(Tests_Context *ctx, const char *expected_path, const char *actual_path, String_View *diff_output);

Test_Result *tests_snapshot_runner_create_result(Tests_Context *ctx, const Tests_Runner *runner) {
  UNUSED(ctx);
  UNUSED(runner);
  Test_Snapshot_Result *result = malloc(sizeof(Test_Snapshot_Result));
  return &result->base;
}

void tests_snapshot_runner_free_result(Test_Result *_result) {
  Test_Snapshot_Result *result = (Test_Snapshot_Result *)_result;
  if (result->source_code.data) free((void *)result->source_code.data);
  if (result->sout_result.data) free((void *)result->sout_result.data);
  if (result->sout_diff.data) free((void *)result->sout_diff.data);
  if (result->serr_result.data) free((void *)result->serr_result.data);
  if (result->serr_diff.data) free((void *)result->serr_diff.data);
}

void tests_snapshot_runner_run_test(Tests_Context *ctx, const Tests_Runner *_runner, Test *test, Test_Result *base_result) {
  Tests_Snapshot_Runner *runner = (Tests_Snapshot_Runner *)_runner;
  Test_Snapshot_Result *result = (Test_Snapshot_Result *)base_result;
  const char *sout_path = temp_sprintf("%s/%s.sout", ctx->build_path, test->id);
  const char *serr_path = temp_sprintf("%s/%s.serr", ctx->build_path, test->id);

  String_Builder source_code_sb = {0};
  if (!nob_read_entire_file(test->absolute_path, &source_code_sb)) {
    base_result->error_str = "Failed to read source code";
    base_result->status = TEST_RESULT_STATUS_FAIL;
    return;
  }
  result->source_code = sb_to_sv(source_code_sb);

  base_result->elapsed = nanos_since_unspecified_epoch();
  runner->runner(ctx, test, sout_path, serr_path);
  base_result->elapsed = nanos_since_unspecified_epoch() - base_result->elapsed;
  base_result->status = TEST_RESULT_STATUS_PASS;

  String_Builder sout_sb = {0};
  if (!nob_read_entire_file(sout_path, &sout_sb)) {
    base_result->error_str = "Failed to read test's sout";
    base_result->status = TEST_RESULT_STATUS_FAIL;
    return;
  }
  result->sout_result = sb_to_sv(sout_sb);

  String_Builder serr_sb = {0};
  if (!nob_read_entire_file(serr_path, &serr_sb)) {
    base_result->error_str = "Failed to read test's serr";
    base_result->status = TEST_RESULT_STATUS_FAIL;
    return;
  }
  result->serr_result = sb_to_sv(serr_sb);

#define diff_output(name)                                                                              \
  do {                                                                                                 \
    const char *snapshot_path = temp_sprintf("%s." STRINGIFY(name), test->absolute_path);              \
    if (!nob_file_exists(snapshot_path)) {                                                             \
      if (result->name##_result.count) {                                                               \
        if (!runner->save_snapshots) {                                                                 \
          base_result->error_str = STRINGIFY(name) " snapshot not found";                              \
          base_result->status = TEST_SNAPSHOT_RESULT_STATUS_EMPTY_##name;                              \
          return;                                                                                      \
        }                                                                                              \
        nob_write_entire_file(snapshot_path, result->name##_result.data, result->name##_result.count); \
        base_result->status = TEST_SNAPSHOT_RESULT_STATUS_UPDATED;                                     \
      }                                                                                                \
    } else if (run_diff(ctx, snapshot_path, name##_path, &result->name##_diff)) {                      \
      if (!runner->save_snapshots) {                                                                   \
        base_result->error_str = STRINGIFY(name) " does not match snapshot";                           \
        base_result->status = TEST_SNAPSHOT_RESULT_STATUS_DIFF_##name;                                 \
        return;                                                                                        \
      }                                                                                                \
      nob_write_entire_file(snapshot_path, result->name##_result.data, result->name##_result.count);   \
      base_result->status = TEST_SNAPSHOT_RESULT_STATUS_UPDATED;                                       \
    }                                                                                                  \
  } while (0)

  diff_output(sout);
  diff_output(serr);
#undef diff_output
}

bool run_diff(Tests_Context *ctx, const char *expected_path, const char *actual_path, String_View *diff_output) {
  const char *diff_path = temp_sprintf("%s.diff", actual_path);
  nob_cmd_append(&ctx->cmd, "git", "diff", "--no-index", "--unified=1", expected_path, actual_path);
  // FIXME: check exit code
  nob_cmd_run(&ctx->cmd, .stdout_path = diff_path);
  String_Builder sb = {0};
  nob_read_entire_file(diff_path, &sb);
  *diff_output = sb_to_sv(sb);
  return !!diff_output->count;
}

void tests_snapshot_print_new(Tests_Context *ctx, Nob_String_View output) {
  Nob_String_View it = output;
  int width = ctx->terminal_size.cols - 3 - 1 - 3 - 3;
  size_t line_number = 1;
  while (it.count) {
    Nob_String_View line = sv_chop_by_delim(&it, '\n');
    printf("   %3zu | %.*s\n", line_number++, MIN((int)line.count, width), line.data);
  }
}

void tests_snapshot_print_diff(Tests_Context *ctx, Nob_String_View diff_output) {
  Nob_String_View it = diff_output;
  int width = ctx->terminal_size.cols - 3 - 1 - 3 - 3;
  while (it.count) {
    Nob_String_View line = sv_chop_by_delim(&it, '\n');
    if (strncmp(line.data, "---", 3) == 0) {
    } else if (strncmp(line.data, "+++", 3) == 0) {
    } else if (strncmp(line.data, "@@", 2) == 0) {
      printf("      | ...\n");
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
          printf("%3d %3d | %.*s\n", old_start + old_ind++, new_start + new_ind++, MIN((int)line.count - 1, width), line.data + 1);
        } else if (line.data[0] == '-') {
          old_len -= 1;
          printf("%3d    " CLI_FG_RED "-" CLI_RESET "| " CLI_FG_RED "%.*s" CLI_RESET "\n", old_start + old_ind++, (int)MIN((int)line.count - 1, width), line.data + 1);
        } else if (line.data[0] == '+') {
          new_len -= 1;
          printf("    %3d" CLI_FG_GREEN "+" CLI_RESET "| " CLI_FG_GREEN "%.*s" CLI_RESET "\n", new_start + new_ind++, (int)MIN((int)line.count - 1, width), line.data + 1);
        }
      }
    }
  }
}

#endif // TESTS_IMPL
