#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef __APPLE__
#include <editline/readline.h>
#else
#include <readline/history.h>
#include <readline/readline.h>
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

int run(Syx_Env *env, char *source_cstr) {
  SExprs *input = rc_acquire(parse_sexprs(source_cstr));
  da_foreach(SExpr *, input_expr, input) {
    SyxV *result = rc_acquire(syx_eval(env, (SyxV *)*input_expr));
    print_syxv(result);
    printf("\n");
    if (syx_env_lookup_get(env, SYXV_EXIT_QUIT_STORAGE) != NULL) break;
    rc_release(result);
  }
  rc_release(input);
  SyxV *quit = syx_env_lookup_get(env, SYXV_EXIT_QUIT_STORAGE);
  if (!quit) return -1;
  return syx_convert_to_integer_v(env, quit);
}

SyxV *eval_quit(Syx_Env *env, SyxV *arguments) {
  SyxV *result = syxv_list_next_safe(&arguments);
  if (result->kind == SYXV_KIND_NIL) result = make_syxv_integer(0);
  syx_env_define_cstr(syx_env_global(env), SYXV_EXIT_QUIT_STORAGE, result);
  return NULL;
}

int main(int argc, char **argv) {
  UNUSED(ht__find_or_put);
  UNUSED(ht__find_and_delete);
  UNUSED(*ht__key);
  UNUSED(ht__next);
  UNUSED(ht__reset);
  UNUSED(ht__free);
  srand(time(NULL));

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
    int result = run(global_env, *command);
    rc_release(global_env);
    return result >= 0 ? result : 0;
  }

  printf("Syx Language REPL\n");
  char *line_ptr;
  while ((line_ptr = readline("> ")) != NULL) {
    if (*line_ptr) add_history(line_ptr);
    int result = run(global_env, line_ptr);
    free(line_ptr);
    if (result >= 0) return result;
  }
  rc_release(global_env);

  return 0;
}
