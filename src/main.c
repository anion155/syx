#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#define NOB_IMPLEMENTATION
#include <nob.h>
#undef NOB_IMPLEMENTATION

#define EXPR_AST_IMPLEMENTATION
#include "expr_ast.h"
#define EXPR_PARSER_IMPLEMENTATION
#include "expr_parser.h"

typedef struct ExprRunContext {
} ExprRunContext;
Expr *expr_eval(Expr *input, ExprRunContext *ctx) {
  TODO("e-expression eval");
  // if (input->kind != EXPR_PAIR) return input;
  // if (input->pair.left->kind != EXPR_SYMBOL) return input;
  // const char *fn_name = input->pair.left->symbol.name;
  // TODO("function eval is not implemented");
}

int main(int argc, char **argv) {
  Nob_String_View source; {
    Nob_String_Builder sb = {0};
    for (int index = 1; index < argc; index += 1) {
      nob_sb_append_cstr(&sb, " ");
      nob_sb_append_cstr(&sb, argv[index]);
    }
    source = nob_sb_to_sv(sb);
    nob_sv_chop_left(&source, 1);
  }
  Expr *input = parse_expr(source);
  dump_expr(input, 0);
  print_expr(input);

  return 0;
}
