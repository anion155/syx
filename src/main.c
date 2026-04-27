#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

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

#define SYXV_EXIT_RESULT_STORAGE "SYXV_EXIT_RESULT_STORAGE"

int run(Syx_Env *env, char *source_cstr) {
  SExprs *input = parse_sexprs(source_cstr);
  da_foreach(SExpr *, input_expr, input) {
    // dump_sexpr(*line); printf("\n");
    // printf("> "); print_sexpr(*input_expr); printf("\n");
    SyxV *result = syx_eval(env, (SyxV *)*input_expr);
    print_syxv(result); printf("\n");
    if (syx_env_lookup(env, SYXV_EXIT_RESULT_STORAGE) != NULL) break;
  }
  SyxV **result = syx_env_lookup(env, SYXV_EXIT_RESULT_STORAGE);
  if (!result) return -1;
  // TODO: support conversion
  if ((*result)->kind != SYXV_KIND_INTEGER) return -1;
  if ((*result)->integer < 0) return -1;
  return (*result)->integer;
}

SyxV *eval_quit(Syx_Env *env, Syx_Arguments arguments) {
  SyxV *result = arguments.count >= 1 ? arguments.items[0] : make_syxv_integer(0);
  syx_env_put(syx_env_global(env), SYXV_EXIT_RESULT_STORAGE, result);
  return NULL;
}

int main(int argc, char **argv) {
  UNUSED(ht__find_or_put);
  UNUSED(ht__find_and_delete);
  UNUSED(*ht__key);
  UNUSED(ht__next);
  UNUSED(ht__reset);
  UNUSED(ht__free);

  char **command = flag_str("c", NULL, "Commands to run");
  if (!flag_parse(argc, argv)) {
    // usage(stderr);
    flag_print_error(stderr);
    exit(1);
  }

  Syx_Env *env = make_global_syx_env();
  syx_env_put(env, "quit", make_syxv_builtin("quit", eval_quit));

  if (*command) {
    int result = run(env, *command);
    return result >= 0 ? result : 0;
  }

  printf("Syx Language REPL\n");
  char line[4096];
  while (1) {
    printf("> ");
    fflush(stdout);
    if (!fgets(line, sizeof(line), stdin)) break;
    int result = run(env, line);
    if (result >= 0) return result;
  }

  return 0;
}
