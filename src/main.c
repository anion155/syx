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

#define SEXPR_AST_IMPL
#include "sexpr_ast.h"
#define SEXPR_PARSER_IMPL
#include "sexpr_parser.h"
#define SYX_VALUE_IMPL
#include "syx_value.h"
#define SYX_EVAL_IMPL
#include "syx_eval.h"

#define SYXV_EXIT_QUIT_STORAGE "SYXV_EXIT_QUIT_STORAGE"

struct SyxVs {
  SyxV **items;
  size_t count;
  size_t capacity;
};

int run_syx(Syx_Env *env, char *source_cstr, bool print_result) {
  SExprs *input = rc_acquire(parse_sexprs(source_cstr));
  struct SyxVs *results = rc_acquire(rc_alloc(sizeof(SExprs), da_destructor));
  da_foreach(SExpr *, expr, input) {
    SyxV *result = rc_acquire(syx_eval(env, (SyxV *)*expr));
    if (print_result) {
      print_syxv(result);
      printf("\n");
    }
    da_append(results, result);
    if (syx_env_lookup_get(env, SYXV_EXIT_QUIT_STORAGE) != NULL) break;
  }
  rc_release(input);
  rc_release(results);
  SyxV *quit = syx_env_lookup_get(env, SYXV_EXIT_QUIT_STORAGE);
  if (!quit) return -1;
  return syx_convert_to_integer_v(env, quit);
}

SyxV *eval_quit(Syx_Env *env, SyxV *arguments) {
  SyxV *result = syxv_list_next(&arguments);
  if (result->kind == SYXV_KIND_NIL) result = make_syxv_integer(0);
  syx_env_define_cstr(syx_env_global(env), SYXV_EXIT_QUIT_STORAGE, result);
  return NULL;
}

int main(int argc, char **argv) {
  UNUSED(ht__find_and_delete);
  UNUSED(ht__reset);
  srand(time(NULL));

  bool *print_result = flag_bool("p", false, "Print results of evaluation");
  char **command = flag_str("c", NULL, "Commands to run");
  if (!flag_parse(argc, argv)) {
    // usage(stderr);
    flag_print_error(stderr);
    exit(1);
  }
  argc = flag_rest_argc();
  argv = flag_rest_argv();

  Syx_Env *global_env = rc_acquire(make_global_syx_env());
  syx_env_define_cstr(global_env, "quit", make_syxv_builtin("quit", eval_quit));

  if (*command) {
    int result = run_syx(global_env, *command, *print_result);
    rc_release(global_env);
    return result >= 0 ? result : 0;
  }

  printf("Syx Language REPL\n");
  char *line_ptr;
  while ((line_ptr = readline("> ")) != NULL) {
    if (*line_ptr) add_history(line_ptr);
    int result = run_syx(global_env, line_ptr, *print_result);
    free(line_ptr);
    if (result >= 0) return result;
  }
  rc_release(global_env);

  return 0;
}
