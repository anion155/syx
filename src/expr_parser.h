#ifndef EXPR_PARSER_H
#define EXPR_PARSER_H
#include <nob.h>
#include <ht.h>
#include "expr_ast.h"

typedef struct Expr_Parse_Result {
  Expr *expr;
  Nob_String_View end;
} Expr_Parse_Result;
Expr_Parse_Result parse_expr(Nob_String_View source);

#endif // EXPR_PARSER_H

#ifdef EXPR_PARSER_IMPLEMENTATION
#undef EXPR_PARSER_IMPLEMENTATION

#include "expr_utils.h"

typedef struct {
  Nob_String_View source;
  Nob_String_View it;
  Nob_String_View line;
  size_t linenumber;
} ExprParserContext;

#define EXPR_TOKEN_LIST_START '('
#define EXPR_TOKEN_LIST_END   ')'
#define EXPR_TOKEN_QUOTES     '\''
#define EXPR_TOKEN_DQUOTES    '"'
#define EXPR_TOKEN_ESCAPE     '\\'
#define EXPR_TOKEN_HYPHEN     '-'
#define EXPR_TOKEN_DOT        '.'
#define EXPR_TOKEN_DIGIT_0    '0'
#define EXPR_TOKEN_GUARD      '|'

int islineend(int c) {
  return (c == '\n' || c == '\r');
}
int issymbol_special(int c) {
  return (
       c == '#' || c == '?' || c == '@' || c == '!' || c == '$' || c == '+' || c == '-' || c == '*' || c == '/'
    || c == '=' || c == '<' || c == '>'
  );
}
int issymbol(int c) {
  return c == EXPR_TOKEN_HYPHEN || issymbol_special(c) || isalnum(c);
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

Expr *parse__expr_string(ExprParserContext *ctx) {
  if (ctx->it.data[0] != EXPR_TOKEN_DQUOTES) UNREACHABLE("expected string literal start here");
  nob_sv_chop_left(&ctx->it, 1);
  size_t i = 0;
  while (ctx->it.data[i] != EXPR_TOKEN_DQUOTES) {
    if (ctx->it.data[i] == EXPR_TOKEN_ESCAPE) i += 1;
    i += 1;
    if (i >= ctx->it.count) UNREACHABLE("expected string literal end here");
  }
  Nob_String_View value = nob_sv_from_parts(ctx->it.data, i);
  i += 1;
  ctx->it.count -= i;
  ctx->it.data  += i;
  return make_expr_string(strndup(value.data, value.count));
}
Expr *parse__expr_real(ExprParserContext *ctx, expr_int_t integer_part) {
  expr_real_t fractional_part = parse_fractions(&ctx->it);
  if (integer_part < 0) fractional_part = integer_part - fractional_part;
  else fractional_part = integer_part + fractional_part;
  return make_expr_real(fractional_part);
}
Expr *parse__expr_integer(ExprParserContext *ctx) {
  if (!ctx->it.count) UNREACHABLE("expected number literal here");
  expr_int_t value = 0;
  if (ctx->it.data[0] == EXPR_TOKEN_DOT) goto upgrade;
  value = parse_integer(&ctx->it);
  if (ctx->it.data[0] == EXPR_TOKEN_DOT) goto upgrade;
  return make_expr_integer(value);
upgrade:
  return parse__expr_real(ctx, value);
}
Expr *parse__expr_guarded_symbol(ExprParserContext *ctx) {
  if (ctx->it.data[0] != EXPR_TOKEN_GUARD) UNREACHABLE("expected guarded symbol start here");
  nob_sv_chop_left(&ctx->it, 1);
  size_t i = 0;
  while (ctx->it.data[i] != EXPR_TOKEN_GUARD) {
    i += 1;
    if (i >= ctx->it.count) UNREACHABLE("expected guarded symbol end here");
  }
  Nob_String_View name = nob_sv_from_parts(ctx->it.data, i);
  i += 1;
  ctx->it.count -= i;
  ctx->it.data  += i;
  return make_expr_symbol(strndup(name.data, name.count), true);
}
Expr *parse__expr_symbol(ExprParserContext *ctx) {
  if (ctx->it.data[0] == EXPR_TOKEN_GUARD) return parse__expr_guarded_symbol(ctx);
  Nob_String_View name = nob_sv_chop_while(&ctx->it, issymbol);
  if (!name.count) UNREACHABLE("unexpected empty symbol name");
  return make_expr_symbol(strndup(name.data, name.count), false);
}
Expr *parse__expr(ExprParserContext *ctx);
typedef struct {
  Expr **items;
  size_t count;
  size_t capacity;
} ExprList;
Expr *parse__expr_list(ExprParserContext *ctx) {
  if (ctx->it.data[0] != EXPR_TOKEN_LIST_START) UNREACHABLE("expected ( symbol here");
  nob_sv_chop_left(&ctx->it, 1);
  ExprList exprs = {0};
  while (true) {
    Nob_String_View spaces = nob_sv_chop_while(&ctx->it, isspace);
    update_line(ctx);
    if (ctx->it.count == 0) UNREACHABLE("unexpected s-expr end, ) was expected here");
    if (ctx->it.data[0] == EXPR_TOKEN_LIST_END) break;
    if (exprs.count && spaces.count == 0) UNREACHABLE("space was expected here");
    if (ctx->it.data[0] == EXPR_TOKEN_DOT && isspace(ctx->it.data[1])) {
      nob_sv_chop_left(&ctx->it, 1);
      spaces = nob_sv_chop_while(&ctx->it, isspace);
      update_line(ctx);
      da_append(&exprs, parse__expr(ctx));
      if (ctx->it.data[0] != EXPR_TOKEN_LIST_END) UNREACHABLE("expected list end here");
      nob_sv_chop_left(&ctx->it, 1);
      return make_expr_list_opt(exprs.count, exprs.items);
    }
    da_append(&exprs, parse__expr(ctx));
  }
  nob_sv_chop_left(&ctx->it, 1);
  da_append(&exprs, NULL);
  return make_expr_list_opt(exprs.count, exprs.items);
}
Expr *parse__expr_quote(ExprParserContext *ctx) {
  if (ctx->it.data[0] != EXPR_TOKEN_QUOTES) UNREACHABLE("expected ( symbol here");
  nob_sv_chop_left(&ctx->it, 1);
  return make_expr_quote(parse__expr(ctx));
}
Expr *parse__expr(ExprParserContext *ctx) {
  char current = ctx->it.data[0];
  if (current == EXPR_TOKEN_LIST_START) return parse__expr_list(ctx);
  if (current == EXPR_TOKEN_QUOTES) return parse__expr_quote(ctx);
  if (current == EXPR_TOKEN_DQUOTES) return parse__expr_string(ctx);
  if (current == EXPR_TOKEN_HYPHEN) {
    if (isspace(ctx->it.data[1])) return parse__expr_symbol(ctx);
    return parse__expr_integer(ctx);
  }
  if (isdigit(current)) return parse__expr_integer(ctx);
  if (current == EXPR_TOKEN_DOT) return parse__expr_real(ctx, 0);
  return parse__expr_symbol(ctx);
}
Expr_Parse_Result parse_expr(Nob_String_View source) {
  ExprParserContext ctx = {.source = source, .it = source, .line = source, .linenumber = 0};
  nob_sv_chop_while(&ctx.it, isspace);
  if (!ctx.it.count) UNREACHABLE("unexpected end of s-expression");
  update_line(&ctx);
  Expr *expr = parse__expr(&ctx);
  nob_sv_chop_while(&ctx.it, isspace);
  return (Expr_Parse_Result){.expr = expr, .end = ctx.it};
}

#endif // EXPR_PARSER_IMPLEMENTATION
