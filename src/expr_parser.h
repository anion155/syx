#ifndef EXPR_PARSER_H
#define EXPR_PARSER_H

#include <nob.h>
#include <ht.h>
#include "expr_ast.h"

Expr *parse_expr(String_View *source);

typedef struct Exprs {
  Expr **items;
  size_t count;
  size_t capacity;
} Exprs;
Exprs parse_exprs(String_View *source);

#endif // EXPR_PARSER_H

#if defined(EXPR_PARSER_IMPL) && !defined(EXPR_PARSER_IMPL_C)
#define EXPR_PARSER_IMPL_C

#define EXPR_AST_IMPL
#include "expr_ast.h"
#define EXPR_UTILS_IMPL
#include "expr_utils.h"

typedef struct {
  String_View source;
  String_View *it;
  String_View line;
  size_t linenumber;
} Expr_Parser_Context;
#define PARSER_ERROR(message, ctx) UNREACHABLE((message))

String_View chop_spaces(Expr_Parser_Context *ctx) {
  String_View spaces = sv_chop_while(ctx->it, isspace);
  const char *line_end = ctx->line.data + ctx->line.count;
  while ((ctx->it->data - line_end) > 0) {
    const char *source_end = ctx->source.data + ctx->source.count;
    String_View line = { .data = line_end, .count = source_end - line_end };
    ctx->line = sv_chop_while(&line, islineend);
    ctx->linenumber += 1;
    line_end = ctx->line.data + ctx->line.count;
  }
  return spaces;
}

Expr *parse__expr(Expr_Parser_Context *ctx);
Expr *parse__expr_real(Expr_Parser_Context *ctx, expr_int_t integer_part) {
  expr_real_t fractional_part = 0;
  if (!parse_fractions(ctx->it, &fractional_part)) PARSER_ERROR("expected real number's fractional part start here", ctx);
  if (integer_part < 0) fractional_part = integer_part - fractional_part;
  else fractional_part = integer_part + fractional_part;
  return make_expr_real(fractional_part);
}
Expr *parse__expr_integer(Expr_Parser_Context *ctx) {
  if (!ctx->it->count) PARSER_ERROR("expected number literal here", ctx);
  expr_int_t value = 0;
  if (ctx->it->data[0] == EXPR_TOKEN_DOT) goto upgrade;
  if (!parse_integer(ctx->it, &value)) PARSER_ERROR("expected number literal here", ctx);
  if (ctx->it->data[0] == EXPR_TOKEN_DOT) goto upgrade;
  return make_expr_integer(value);
upgrade:
  return parse__expr_real(ctx, value);
}
Expr *parse__expr_list(Expr_Parser_Context *ctx) {
  if (ctx->it->data[0] != EXPR_TOKEN_LIST_START) PARSER_ERROR("expected ( symbol here", ctx);
  sv_chop_left(ctx->it, 1);
  Exprs exprs = {0};
  while (true) {
    String_View spaces = chop_spaces(ctx);
    if (ctx->it->count == 0) PARSER_ERROR("unexpected s-expr end, ) was expected here", ctx);
    if (ctx->it->data[0] == EXPR_TOKEN_LIST_END) break;
    if (exprs.count && spaces.count == 0) PARSER_ERROR("space was expected here", ctx);
    if (ctx->it->data[0] == EXPR_TOKEN_DOT && isspace(ctx->it->data[1])) {
      sv_chop_left(ctx->it, 1);
      spaces = chop_spaces(ctx);
      da_append(&exprs, parse__expr(ctx));
      if (ctx->it->data[0] != EXPR_TOKEN_LIST_END) PARSER_ERROR("expected list end here", ctx);
      sv_chop_left(ctx->it, 1);
      goto result;
    }
    da_append(&exprs, parse__expr(ctx));
  }
  sv_chop_left(ctx->it, 1);
  da_append(&exprs, NULL);
result:
  Expr *list = make_expr_list_opt(exprs.count, exprs.items);
  da_free(exprs);
  return list;
}
Expr *parse__expr_quote(Expr_Parser_Context *ctx) {
  if (ctx->it->data[0] != EXPR_TOKEN_QUOTES) PARSER_ERROR("expected ( symbol here", ctx);
  sv_chop_left(ctx->it, 1);
  return make_expr_quote(parse__expr(ctx));
}
Expr *parse__expr_string(Expr_Parser_Context *ctx) {
  if (ctx->it->data[0] != EXPR_TOKEN_DQUOTES) PARSER_ERROR("expected string literal start here", ctx);
  sv_chop_left(ctx->it, 1);
  size_t i = 0;
  while (ctx->it->data[i] != EXPR_TOKEN_DQUOTES) {
    if (ctx->it->data[i] == EXPR_TOKEN_ESCAPE) i += 1;
    i += 1;
    if (i >= ctx->it->count) PARSER_ERROR("expected string literal end here", ctx);
  }
  String_View value = sv_from_parts(ctx->it->data, i);
  i += 1;
  ctx->it->count -= i;
  ctx->it->data  += i;
  return make_expr_string(value);
}
Expr *parse__expr_guarded_symbol(Expr_Parser_Context *ctx) {
  if (ctx->it->data[0] != EXPR_TOKEN_GUARD) PARSER_ERROR("expected guarded symbol start here", ctx);
  sv_chop_left(ctx->it, 1);
  size_t i = 0;
  while (ctx->it->data[i] != EXPR_TOKEN_GUARD) {
    i += 1;
    if (i >= ctx->it->count) PARSER_ERROR("expected guarded symbol end here", ctx);
  }
  String_View name = sv_from_parts(ctx->it->data, i);
  i += 1;
  ctx->it->count -= i;
  ctx->it->data  += i;
  return make_expr_symbol(name);
}
Expr *parse__expr_symbol(Expr_Parser_Context *ctx) {
  if (ctx->it->data[0] == EXPR_TOKEN_GUARD) return parse__expr_guarded_symbol(ctx);
  String_View name = sv_chop_while(ctx->it, issymbol);
  if (!name.count) PARSER_ERROR("unexpected empty symbol name", ctx);
  return make_expr_symbol(name);
}
Expr *parse__expr(Expr_Parser_Context *ctx) {
  char current = ctx->it->data[0];
  if (current == EXPR_TOKEN_LIST_START) return parse__expr_list(ctx);
  if (current == EXPR_TOKEN_QUOTES) return parse__expr_quote(ctx);
  if (current == EXPR_TOKEN_DQUOTES) return parse__expr_string(ctx);
  if (current == EXPR_TOKEN_HYPHEN) {
    if (isspace(ctx->it->data[1])) return parse__expr_symbol(ctx);
    return parse__expr_integer(ctx);
  }
  if (isdigit(current)) return parse__expr_integer(ctx);
  if (current == EXPR_TOKEN_DOT) return parse__expr_real(ctx, 0);
  return parse__expr_symbol(ctx);
}
Expr *parse_expr(String_View *source) {
  Expr_Parser_Context ctx = {.source = *source, .it = source, .line = *source, .linenumber = 0};
  chop_spaces(&ctx);
  if (!ctx.it->count) PARSER_ERROR("unexpected end of s-expression", ctx);
  return parse__expr(&ctx);
}

Exprs parse_exprs(String_View *source) {
  Expr_Parser_Context ctx = {.source = *source, .it = source, .line = *source, .linenumber = 0};
  Exprs exprs = {0};
  while (source->count) {
    chop_spaces(&ctx);
    if (!ctx.it->count) break;
    da_append(&exprs, parse__expr(&ctx));
  }
  return exprs;
}

#endif // EXPR_PARSER_IMPL
