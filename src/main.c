#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#define NOB_IMPLEMENTATION
#include <nob.h>
#undef NOB_IMPLEMENTATION
#define HT_IMPLEMENTATION
#include <ht.h>
#undef HT_IMPLEMENTATION

#define EXPR_AST_IMPLEMENTATION
#include "expr_ast.h"
#define EXPR_UTILS_IMPLEMENTATION
#include "expr_utils.h"
#define EXPR_PARSER_IMPLEMENTATION
#include "expr_parser.h"
#define EXPR_EVAL_IMPLEMENTATION
#include "expr_eval.h"

int main(int argc, char **argv) {
  UNUSED(ht__find_or_put);
  UNUSED(ht__find_and_delete);
  UNUSED(*ht__key);
  UNUSED(ht__next);
  UNUSED(ht__reset);
  UNUSED(ht__free);

  Nob_String_View source; {
    Nob_String_Builder sb = {0};
    for (int index = 1; index < argc; index += 1) {
      nob_sb_append_cstr(&sb, " ");
      nob_sb_append_cstr(&sb, argv[index]);
    }
    source = nob_sb_to_sv(sb);
    nob_sv_chop_left(&source, 1);
  }
  Expr_Parse_Result input = parse_expr(source);
  print_expr(input.expr);
  // dump_expr(input.expr);
  Expr_Env env={0};
  init_expr_global_env(&env);
  Expr_Value result = expr_eval(&env, (Expr_Value){.kind = EXPR_VALUE_KIND_EXPR, .expr = input.expr});
  print_expr(result);

  return 0;
}
