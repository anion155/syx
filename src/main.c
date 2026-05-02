#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef __APPLE__
#  include <editline/readline.h>
#else
#  include <readline/history.h>
#  include <readline/readline.h>
#endif

#define NOB_IMPL
#include <nob.h>
#define FLAG_IMPL
#include <flag.h>
#define HT_IMPL
#include <ht.h>
#define RC_IMPL
#include <rc.h>

#define SYX_PARSER_IMPL
#include "syx_parser.h"
#define SYX_VALUE_IMPL
#include "syx_value.h"
#define SYX_EVAL_IMPL
#include "syx_eval.h"

#define SYXV_EXIT_QUIT_STORAGE "SYXV_EXIT_QUIT_STORAGE"

// typedef enum Syx_Run_Verbose {
//   SYX_RUN_VERBOSE_QUITE = 0,
//   SYX_RUN_VERBOSE_LAST_RESULT,
//   SYX_RUN_VERBOSE_EVERY_RESULT,
//   SYX_RUN_VERBOSE_ALL,
// } Syx_Run_Verbose;

typedef struct Syx_Run_Context {
  Syx_Env *global_env;
  // Syx_Run_Verbose verbose;
  bool opt_xtrace;
  bool opt_print;
} Syx_Run_Context;

Syx_Run_Context ctx = {0};

define_constant(Ht(const char *, bool *), ctx_options) {
  ctx_options->hasheq = ht_cstr_hasheq;
  *ht_put(ctx_options, "x") = &ctx.opt_xtrace;
  *ht_put(ctx_options, "p") = &ctx.opt_print;
}

int run_syx(const char *source_cstr) {
  SyxVs *results = rc_acquire(rc_alloc(sizeof(SyxVs), da_destructor));
  parser_syxvs_for_each(value, source_cstr) {
    if (ctx.opt_xtrace) {
      printf("+");
      print_syxv(value);
      printf("\n");
    }
    SyxV *result = syx_eval(ctx.global_env, value);
    if (ctx.opt_xtrace) {
      print_syxv(result);
      printf("\n");
    }
    da_append(results, rc_acquire(result));
    if (syx_env_lookup_get(ctx.global_env, SYXV_EXIT_QUIT_STORAGE) != NULL) break;
  }
  if (!ctx.opt_xtrace && ctx.opt_print && results->count) {
    print_syxv(results->items[results->count - 1]);
    printf("\n");
  }
  rc_release(results);
  SyxV *quit = syx_env_lookup_get(ctx.global_env, SYXV_EXIT_QUIT_STORAGE);
  if (!quit) return -1;
  return syx_convert_to_integer_v(ctx.global_env, quit);
}

SyxV *eval_quit(Syx_Env *env, SyxV *arguments) {
  SyxV *result = syxv_list_next(&arguments);
  if (result->kind == SYXV_KIND_NIL) result = make_syxv_integer(0);
  syx_env_define_cstr(syx_env_global(env), SYXV_EXIT_QUIT_STORAGE, result);
  return NULL;
}

SyxV *eval_setopt(Syx_Env *env, SyxV *arguments) {
  SyxV *name = syxv_list_next(&arguments);
  if (name->kind != SYXV_KIND_SYMBOL) RUNTIME_ERROR("option name expected", env);
  bool **option = ht_find(ctx_options(), name->symbol.name);
  if (option == NULL) RUNTIME_ERROR("option not found", env);
  SyxV *value = syx_eval(env, syxv_list_next(&arguments));
  (**option) = syx_convert_to_bool_v(env, value);
  return NULL;
}

void usage(FILE *stream) {
  fprintf(stream, "usage: %s [options] [file]\n", flag_program_name());
  fprintf(stream, "modes:\n");
  fprintf(stream, "  %-16s  %s\n", "syx", "Starts the interactive REPL.");
  fprintf(stream, "  %-16s  %s\n", "syx <file.syx>", "Executes the specified script file.");
  fprintf(stream, "  %-16s  %s\n", "syx -c \"expr\"", "Executes the provided syx string.");
  fprintf(stream, "options:\n");
  flag_print_options(stream);
}

int main(int argc, char **argv) {
  srand(time(NULL));

  bool *opt_xtrace = flag_bool("x", false, "Print every expression before evaluation");
  bool *opt_print = flag_bool("p", false, "Print result of last evaluation");
  Flag_List *commands = flag_list("c", "Commands to run");
  bool *help = flag_bool("h", false, "Show this help message");
  if (!flag_parse(argc, argv)) {
    flag_print_error(stderr);
    usage(stderr);
    exit(1);
  }
  if (*help) {
    usage(stdout);
    exit(0);
  }
  argc = flag_rest_argc();
  argv = flag_rest_argv();

  if (commands->count) ctx.opt_print = true;
  else if (argc == 1) ctx.opt_print = false;
  else ctx.opt_print = true;

  if (*opt_xtrace) ctx.opt_xtrace = *opt_xtrace;
  else if (*opt_print) ctx.opt_print = true;

  Syx_Env *global_env = rc_acquire(make_global_syx_env());
  ctx.global_env = global_env;

  syx_env_define_cstr(global_env, "quit", make_syxv_builtin(NULL, eval_quit));
  syx_env_define_cstr(global_env, "setopt", make_syxv_specialf(NULL, eval_setopt));

  int result = 0;
  if (commands->count) {
    String_Builder sb = {0};
    da_foreach(const char *, command, commands) sb_append_cstr(&sb, *command);
    int run_result = run_syx(sb_to_sv(sb).data);
    if (run_result >= 0) nob_return_defer(run_result);
  } else if (argc == 1) {
    String_Builder sb = {0};
    if (!nob_read_entire_file(argv[0], &sb)) UNREACHABLE("Failed to read file");
    String_View script = sb_to_sv(sb);
    int run_result = run_syx(script.data);
    if (run_result >= 0) nob_return_defer(run_result);
  } else {
    printf("Syx Language REPL\n");
    char *line_ptr;
    while ((line_ptr = readline("> ")) != NULL) {
      if (*line_ptr) add_history(line_ptr);
      int run_result = run_syx(line_ptr);
      free(line_ptr);
      if (run_result >= 0) nob_return_defer(run_result);
    }
  }

defer:
  rc_release(global_env);
  return result;
}
