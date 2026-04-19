#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#define NOB_IMPLEMENTATION
#include <nob.h>

typedef enum {
  EXPR_NIL,
  EXPR_SYMBOL,
  EXPR_PAIR,
  EXPR_LITERAL_NUMBER_INTEGER,
  EXPR_LITERAL_NUMBER_REAL,
  EXPR_LITERAL_STRING,
} Expr_Kind;
typedef struct Expr Expr;
typedef long int expr_int_t;
typedef double expr_real_t;
typedef char * expr_string_t;
struct ExprSymbol {
  const char *name;
  bool guarded;
};
struct ExprPair {
  Expr *left;
  Expr *right;
};
struct Expr {
  Expr_Kind kind;
  union {
    struct ExprSymbol symbol;
    struct ExprPair pair;
    expr_int_t integer;
    expr_real_t real;
    expr_string_t string;
  };
};

Expr *make_expr(Expr_Kind kind) {
  Expr *expr = malloc(sizeof(Expr));
  expr->kind = kind;
  return expr;
}
Expr *make_expr_nil() {
  return make_expr(EXPR_NIL);
}
Expr *make_expr_symbol(const char *symbol, bool guarded) {
  Expr *expr = make_expr(EXPR_SYMBOL);
  expr->symbol.name = symbol;
  expr->symbol.guarded = guarded;
  return expr;
}
Expr *make_expr_pair(Expr *left, Expr *right) {
  Expr *expr = make_expr(EXPR_PAIR);
  expr->pair.left = left;
  expr->pair.right = right;
  return expr;
}
Expr *make_expr_list_opt(size_t size, Expr **exprs) {
  Expr *list = make_expr_pair(NULL, make_expr_nil());
  for (int index = size - 1; index >= 1; index -= 1) {
    list->pair.left = exprs[index];
    list = make_expr_pair(NULL, list);
  }
  list->pair.left = exprs[0];
  return list;
}
#define make_expr_list(...) make_expr_list_opt((sizeof((Expr *[]){__VA_ARGS__}) / sizeof(Expr *)), ((Expr *[]){__VA_ARGS__}))
Expr *make_expr_literal_integer(expr_int_t value) {
  Expr *expr = make_expr(EXPR_LITERAL_NUMBER_INTEGER);
  expr->integer = value;
  return expr;
}
#define make_expr_int(value) make_expr_literal_integer(value)
Expr *make_expr_literal_real(expr_real_t value) {
  Expr *expr = make_expr(EXPR_LITERAL_NUMBER_REAL);
  expr->real = value;
  return expr;
}
#define make_expr_real(value) make_expr_literal_real(value)
Expr *make_expr_literal_string(expr_string_t value) {
  Expr *expr = make_expr(EXPR_LITERAL_STRING);
  expr->string = value;
  return expr;
}
#define make_expr_str(value) make_expr_literal_string(value)
#define make_expr_value(value)                       \
  _Generic(value,                                    \
    expr_int_t:    make_expr_literal_integer(value), \
    expr_real_t:   make_expr_literal_real(value),    \
    expr_string_t: make_expr_literal_string(value)   \
  )

#define EXPR_TOKEN_LIST_START '('
#define EXPR_TOKEN_LIST_END   ')'
#define EXPR_TOKEN_QUOTES     '"'
#define EXPR_TOKEN_ESCAPE     '\\'
#define EXPR_TOKEN_HYPHEN     '-'
#define EXPR_TOKEN_DOT        '.'
#define EXPR_TOKEN_DIGIT_0    '0'
#define EXPR_TOKEN_GUARD      '|'

int islineend(int c) {
  return (c == '\n' || c == '\r');
}
int issymbol_special(int c) {
  return c == '?' || c == '@' || c == '!' || c == '$';
}
int issymbol(int c) {
  return c == EXPR_TOKEN_HYPHEN || issymbol_special(c) || isalnum(c);
}

typedef struct {
  Nob_String_View source;
  Nob_String_View it;
  Nob_String_View line;
  size_t linenumber;
} ExprParserContext;
void update_line(ExprParserContext *ctx) {
  // const char *line_end = ctx->line.data + ctx->line.count;
  // while ((ctx->it.data - line_end) > 0) {
  //   const char *source_end = ctx->source.data + ctx->source.count;
  //   Nob_String_View line = { .data = line_end, .count = source_end - line_end };
  //   ctx->line = nob_sv_chop_while(&line, islineend);
  //   ctx->linenumber += 1;
  //   line_end = ctx->line.data + ctx->line.count;
  // }
}

Expr *_parse_expr_string(ExprParserContext *ctx) {
  if (*ctx->it.data != EXPR_TOKEN_QUOTES) NOB_UNREACHABLE("expected string literal start here");
  size_t i = 0;
  while (ctx->it.data[i] != EXPR_TOKEN_QUOTES) {
    if (ctx->it.data[i] == EXPR_TOKEN_ESCAPE) i += 1;
    i += 1;
    if (i >= ctx->it.count) NOB_UNREACHABLE("expected string literal end here");
  }
  Nob_String_View value = nob_sv_from_parts(ctx->it.data, i);
  i += 1;
  ctx->it.count -= i;
  ctx->it.data  += i;
  Expr *expr = make_expr(EXPR_LITERAL_STRING);
  expr->string = strdup(temp_sv_to_cstr(value));
  return expr;
}
Expr *_parse_expr_real(ExprParserContext *ctx, expr_int_t integer_part, bool is_negative);
Expr *_parse_expr_integer(ExprParserContext *ctx) {
  bool is_negative = *ctx->it.data == EXPR_TOKEN_HYPHEN;
  if (is_negative) nob_sv_chop_left(&ctx->it, 1);
  expr_int_t value = 0;
  size_t i = 0;
  if (!ctx->it.count) NOB_UNREACHABLE("expected number literal here");
  if (ctx->it.data[i] == EXPR_TOKEN_DOT) return _parse_expr_real(ctx, value, is_negative);
  while (isdigit(ctx->it.data[i])) {
    value = value * 10 + (ctx->it.data[i] - EXPR_TOKEN_DIGIT_0);
    i += 1;
    if (i >= ctx->it.count) break;
    if (ctx->it.data[i] == EXPR_TOKEN_DOT) return _parse_expr_real(ctx, value, is_negative);
  }
  ctx->it.count -= i;
  ctx->it.data  += i;
  Expr *expr = make_expr(EXPR_LITERAL_NUMBER_INTEGER);
  if (is_negative) value *= -1;
  expr->integer = value;
  return expr;
}
Expr *_parse_expr_real(ExprParserContext *ctx, expr_int_t integer_part, bool is_negative) {
  if (*ctx->it.data != EXPR_TOKEN_DOT) NOB_UNREACHABLE("expected real number's fractional part start here");
  nob_sv_chop_left(&ctx->it, 1);
  expr_int_t frac_part = 0;
  expr_int_t exponent = 1;
  size_t i = 0;
  while (i < ctx->it.count && isdigit(ctx->it.data[i])) {
    frac_part = frac_part * 10 + (ctx->it.data[i] - EXPR_TOKEN_DIGIT_0);
    exponent *= 10;
    i += 1;
  }
  ctx->it.count -= i;
  ctx->it.data  += i;
  Expr *expr = make_expr(EXPR_LITERAL_NUMBER_REAL);
  expr->real = (expr_real_t)integer_part + (expr_real_t)frac_part / (expr_real_t)exponent;
  if (is_negative) expr->real *= -1;
  return expr;
}
Expr *_parse_expr_guarded_symbol(ExprParserContext *ctx) {
  if (*ctx->it.data != EXPR_TOKEN_GUARD) NOB_UNREACHABLE("expected guarded symbol start here");
  nob_sv_chop_left(&ctx->it, 1);
  size_t i = 0;
  while (ctx->it.data[i] != EXPR_TOKEN_GUARD) {
    i += 1;
    if (i >= ctx->it.count) NOB_UNREACHABLE("expected guarded symbol end here");
  }
  Nob_String_View name = nob_sv_from_parts(ctx->it.data, i);
  i += 1;
  ctx->it.count -= i;
  ctx->it.data  += i;
  Expr *expr = make_expr(EXPR_SYMBOL);
  expr->symbol.name = temp_sv_to_cstr(name);
  expr->symbol.guarded = true;
  return expr;
}
Expr *_parse_expr_symbol(ExprParserContext *ctx) {
  if (*ctx->it.data == EXPR_TOKEN_GUARD) return _parse_expr_guarded_symbol(ctx);
  Nob_String_View name = nob_sv_chop_while(&ctx->it, issymbol);
  Expr *expr = make_expr(EXPR_SYMBOL);
  expr->symbol.name = temp_sv_to_cstr(name);
  expr->symbol.guarded = false;
  return expr;
}
Expr *_parse_expr(ExprParserContext *ctx);
Expr *_parse_expr_list(ExprParserContext *ctx) {
  if (*ctx->it.data != EXPR_TOKEN_LIST_START) NOB_UNREACHABLE("expected ( symbol here");
  nob_sv_chop_left(&ctx->it, 1);
  Expr *list = make_expr_nil();
  Expr **last = &list;
  while (true) {
    Nob_String_View spaces = nob_sv_chop_while(&ctx->it, isspace);
    update_line(ctx);
    if (ctx->it.count == 0) NOB_UNREACHABLE("unexpected s-expr end, ) was expected here");
    if (*ctx->it.data == EXPR_TOKEN_LIST_END) break;
    if (list->kind != EXPR_NIL && spaces.count == 0) NOB_UNREACHABLE("space was expected here");
    *last = make_expr_pair(_parse_expr(ctx), *last);
    last = &(*last)->pair.right;
  }
  nob_sv_chop_left(&ctx->it, 1);
  return list;
}
Expr *_parse_expr(ExprParserContext *ctx) {
  char curent = *ctx->it.data;
  if (curent == EXPR_TOKEN_LIST_START) return _parse_expr_list(ctx);
  if (curent == EXPR_TOKEN_QUOTES) return _parse_expr_string(ctx);
  if (curent == EXPR_TOKEN_HYPHEN || isdigit(curent)) return _parse_expr_integer(ctx);
  if (curent == EXPR_TOKEN_DOT) return _parse_expr_real(ctx, 0, false);
  return _parse_expr_symbol(ctx);
}
Expr *parse_expr(Nob_String_View source) {
  ExprParserContext ctx = {.source = source, .it = source, .line = {.data = source.data, .count = 0}, .linenumber = 0};
  nob_sv_chop_while(&ctx.it, isspace);
  update_line(&ctx);
  Expr *expr = _parse_expr_list(&ctx);
  nob_sv_chop_while(&ctx.it, isspace);
  if (ctx.it.count) {
    // Nob_String_View line = start;
    // nob_sv_chop_while(&line, islineend);
    // source->data - start.data
    // NOB_UNREACHABLE(temp_sprintf("unexpected end of s-expression:\n  '"SV_Fmt"'\n  %*c", SV_Arg(start), ));
    NOB_UNREACHABLE("unexpected end of s-expression");
  }
  return expr;
}

void print_expr(Expr *expr, int level) {
  printf("%*c", level * 2, ' ');
  switch (expr->kind) {
  case EXPR_NIL: {
    printf("NIL\n");
  } break;
  case EXPR_SYMBOL: {
    if (expr->symbol.guarded) {
      printf("SYMBOL: |%s|\n", expr->symbol.name);
    } else {
      printf("SYMBOL: %s\n", expr->symbol.name);
    }
  } break;
  case EXPR_LITERAL_NUMBER_INTEGER: {
    printf("INTEGER: %ld\n", expr->integer);
  } break;
  case EXPR_LITERAL_NUMBER_REAL: {
    printf("REAL: %f\n", expr->real);
  } break;
  case EXPR_LITERAL_STRING: {
    printf("STRING: '%s'\n", expr->string);
  } break;
  case EXPR_PAIR: {
    printf("PAIR:\n");
    print_expr(expr->pair.left, level + 1);
    print_expr(expr->pair.right, level + 1);
  } break;
  default:
    break;
  }
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
  // Expr *input = make_expr_list(
  //   make_expr_int(1),
  //   make_expr_int(2),
  //   make_expr_int(3),
  // );
  print_expr(input, 0);

  return 0;
}
