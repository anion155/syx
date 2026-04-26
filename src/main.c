#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#define NOB_IMPL
#include <nob.h>
#define HT_IMPL
#include <ht.h>
#define RC_IMPL
#include <rc.h>

#define EXPR_PRINT_IMPL
#include "expr_print.h"
#define EXPR_PARSER_IMPL
#include "expr_parser.h"
// #define EXPR_EVAL_IMPLEMENTATION
// #include "expr_eval.h"

int main(int argc, char **argv) {
  UNUSED(ht__find_or_put);
  UNUSED(ht__find_and_delete);
  UNUSED(*ht__key);
  UNUSED(ht__next);
  UNUSED(ht__reset);
  UNUSED(ht__free);

  String_View source; {
    String_Builder sb = {0};
    for (int index = 1; index < argc; index += 1) {
      sb_append_cstr(&sb, " ");
      sb_append_cstr(&sb, argv[index]);
    }
    source = sb_to_sv(sb);
    sv_chop_left(&source, 1);
  }
  Expr *input = parse_expr(&source);
  // dump_expr(input); printf("\n");
  print_expr(input); printf("\n");
  // Expr_Env env={0};
  // expr_global_env_init(&env);
  // Expr_Value result = expr_eval(&env, (Expr_Value){.kind = EXPR_VALUE_KIND_EXPR, .expr = input.expr});
  // printf("= "); print_expr_value(result); printf("\n");

  return 0;
}
