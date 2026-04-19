#ifndef EXPR_PARSER_H
#define EXPR_PARSER_H
#include <nob.h>
#include "expr_ast.h"

Expr *parse_expr(Nob_String_View source);

#ifdef EXPR_PARSER_IMPLEMENTATION
#undef EXPR_PARSER_IMPLEMENTATION

typedef struct {
  Nob_String_View source;
  Nob_String_View it;
  Nob_String_View line;
  size_t linenumber;
} ExprParserContext;

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
  return c == EXPR_TOKEN_HYPHEN || isspecial(c) || isalnum(c);
}

void update_line(ExprParserContext *ctx) {
  const char *line_end = ctx->line.data + ctx->line.count;
  while ((ctx->it.data - line_end) > 0) {
    const char *source_end = ctx->source.data + ctx->source.count;
    Nob_String_View line = { .data = line_end, .count = source_end - line_end };
    ctx->line = nob_sv_chop_while(&line, islineend);
    ctx->linenumber += 1;
    line_end = ctx->line.data + ctx->line.count;
  }
}

Expr *_parse_expr_string(ExprParserContext *ctx) {
  if (*ctx->it.data != EXPR_TOKEN_QUOTES) NOB_UNREACHABLE("expected string literal start here");
  nob_sv_chop_left(&ctx->it, 1);
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
  Expr *expr = make_expr(EXPR_KIND_STRING);
  expr->string = strdup(temp_sv_to_cstr(value));
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
  if (i == 0) NOB_UNREACHABLE("expected real number's fractional part start here");
  ctx->it.count -= i;
  ctx->it.data  += i;
  Expr *expr = make_expr(EXPR_KIND_REAL);
  expr->real = (expr_real_t)integer_part + (expr_real_t)frac_part / (expr_real_t)exponent;
  if (is_negative) expr->real *= -1;
  return expr;
}
Expr *_parse_expr_integer(ExprParserContext *ctx) {
  bool is_negative = *ctx->it.data == EXPR_TOKEN_HYPHEN;
  if (is_negative) nob_sv_chop_left(&ctx->it, 1);
  expr_int_t value = 0;
  size_t i = 0;
  if (!ctx->it.count) NOB_UNREACHABLE("expected number literal here");
  #define update_to_real_if_needed() if (ctx->it.data[i] == EXPR_TOKEN_DOT) { \
    ctx->it.count -= i; \
    ctx->it.data  += i; \
    return _parse_expr_real(ctx, value, is_negative); \
  }
  update_to_real_if_needed();
  while (isdigit(ctx->it.data[i])) {
    value = value * 10 + (ctx->it.data[i] - EXPR_TOKEN_DIGIT_0);
    i += 1;
    if (i >= ctx->it.count) break;
    update_to_real_if_needed();
  }
  ctx->it.count -= i;
  ctx->it.data  += i;
  Expr *expr = make_expr(EXPR_KIND_INTEGER);
  if (is_negative) value *= -1;
  expr->integer = value;
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
  Expr *expr = make_expr(EXPR_KIND_SYMBOL);
  expr->symbol.name = temp_sv_to_cstr(name);
  expr->symbol.guarded = true;
  return expr;
}
Expr *_parse_expr_symbol(ExprParserContext *ctx) {
  if (*ctx->it.data == EXPR_TOKEN_GUARD) return _parse_expr_guarded_symbol(ctx);
  Nob_String_View name = nob_sv_chop_while(&ctx->it, issymbol);
  if (!name.count) UNREACHABLE("unexpected empty symbol name");
  Expr *expr = make_expr(EXPR_KIND_SYMBOL);
  expr->symbol.name = temp_sv_to_cstr(name);
  expr->symbol.guarded = false;
  return expr;
}
Expr *_parse_expr(ExprParserContext *ctx);
typedef struct {
  Expr **items;
  size_t count;
  size_t capacity;
} ExprList;
Expr *_parse_expr_list(ExprParserContext *ctx) {
  if (*ctx->it.data != EXPR_TOKEN_LIST_START) NOB_UNREACHABLE("expected ( symbol here");
  nob_sv_chop_left(&ctx->it, 1);
  ExprList exprs = {0};
  while (true) {
    Nob_String_View spaces = nob_sv_chop_while(&ctx->it, isspace);
    update_line(ctx);
    if (ctx->it.count == 0) NOB_UNREACHABLE("unexpected s-expr end, ) was expected here");
    if (*ctx->it.data == EXPR_TOKEN_LIST_END) break;
    if (*ctx->it.data == EXPR_TOKEN_DOT && isspace(ctx->it.data[1])) {
      TODO("implement last list element set");
    }
    if (exprs.count && spaces.count == 0) NOB_UNREACHABLE("space was expected here");
    da_append(&exprs, _parse_expr(ctx));
  }
  nob_sv_chop_left(&ctx->it, 1);
  da_append(&exprs, NULL);
  return make_expr_list_opt(exprs.count, exprs.items);
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
  ExprParserContext ctx = {.source = source, .it = source, .line = source, .linenumber = 0};
  nob_sv_chop_while(&ctx.it, isspace);
  if (!ctx.it.count) UNREACHABLE("unexpected end of s-expression");
  update_line(&ctx);
  Expr *expr = _parse_expr(&ctx);
  nob_sv_chop_while(&ctx.it, isspace);
  if (ctx.it.count) NOB_UNREACHABLE("unexpected end of s-expression");
  return expr;
}

#endif // EXPR_PARSER_IMPLEMENTATION
#endif // EXPR_PARSER_H
