#ifndef SEXPR_PARSER_H
#define SEXPR_PARSER_H

#include <ht.h>
#include <nob.h>
#include <rc.h>

#include "sexpr_ast.h"

typedef struct SExprs {
  SExpr **items;
  size_t count;
  size_t capacity;
} SExprs;

SExpr *parse_sexpr(String_View *source);
SExprs *parse_sexprs(String_View *source);

typedef struct SExpr_Parser_Context {
  String_View source;
  String_View *it;
  String_View line;
  size_t linenumber;
} SExpr_Parser_Context;

struct Sexprs_Iterator {
  SExpr_Parser_Context ctx;
  SExprs exprs;
};

SExpr ***parser__make_sexprs_for_each_iterator(const char *source_src);
bool parser__sexprs_for_each_next(SExpr ****exprs, SExpr **expr);
SExpr_Parser_Context *parser__sexprs_for_each_ctx(SExpr **exprs);
#define parser_sexprs_for_each(name, source)                                         \
  for (                                                                              \
      SExpr *name = NULL, ***_ctx = parser__make_sexprs_for_each_iterator((source)); \
      parser__sexprs_for_each_next(&_ctx, &name);)
#define parser_sexprs_for_each_ctx() parser__sexprs_for_each_ctx(exprs)

#endif // SEXPR_PARSER_H

#if defined(SEXPR_PARSER_IMPL) && !defined(SEXPR_PARSER_IMPL_C)
#define SEXPR_PARSER_IMPL_C

#define SEXPR_AST_IMPL
#include "sexpr_ast.h"
#define SYX_UTILS_IMPL
#include "syx_utils.h"

#define SEXPR_TOKEN_LIST_START '('
#define SEXPR_TOKEN_LIST_END ')'
#define SEXPR_TOKEN_QUOTES '\''
#define SEXPR_TOKEN_DQUOTES '"'
#define SEXPR_TOKEN_ESCAPE '\\'
#define SEXPR_TOKEN_HYPHEN '-'
#define SEXPR_TOKEN_DOT '.'
#define SEXPR_TOKEN_DIGIT_0 '0'
#define SEXPR_TOKEN_GUARD '|'
#define SEXPR_TOKEN_COMMENT ';'

#define PARSER_ERROR(message, ctx) UNREACHABLE((UNUSED(ctx), (message)))

String_View chop_spaces(SExpr_Parser_Context *ctx) {
  String_View spaces = sv_chop_while(ctx->it, isspace);
  const char *line_end = ctx->line.data + ctx->line.count;
  while ((ctx->it->data - line_end) > 0) {
    const char *source_end = ctx->source.data + ctx->source.count;
    String_View line = {.data = line_end, .count = source_end - line_end};
    ctx->line = sv_chop_while(&line, islineend);
    ctx->linenumber += 1;
    line_end = ctx->line.data + ctx->line.count;
  }
  return spaces;
}

SExpr *parse__sexpr(SExpr_Parser_Context *ctx);

SExpr *parse__sexpr_fractional(SExpr_Parser_Context *ctx, syx_integer_t integer_part) {
  syx_fractional_t fractional_part = 0;
  if (!parse_fractions(ctx->it, &fractional_part)) PARSER_ERROR("expected fractional number's fractional part start here", ctx);
  if (integer_part < 0) fractional_part = integer_part - fractional_part;
  else fractional_part = integer_part + fractional_part;
  return make_sexpr_fractional(fractional_part);
}

SExpr *parse__sexpr_integer(SExpr_Parser_Context *ctx) {
  if (!ctx->it->count) PARSER_ERROR("expected number literal here", ctx);
  syx_integer_t value = 0;
  if (ctx->it->data[0] == SEXPR_TOKEN_DOT) goto upgrade;
  if (!parse_integer(ctx->it, &value)) PARSER_ERROR("expected number literal here", ctx);
  if (ctx->it->data[0] == SEXPR_TOKEN_DOT) goto upgrade;
  return make_sexpr_integer(value);
upgrade:
  return parse__sexpr_fractional(ctx, value);
}

SExpr *parse__sexpr_list(SExpr_Parser_Context *ctx) {
  if (ctx->it->data[0] != SEXPR_TOKEN_LIST_START) PARSER_ERROR("expected ( symbol here", ctx);
  sv_chop_left(ctx->it, 1);
  SExprs exprs = {0};
  while (true) {
    String_View spaces = chop_spaces(ctx);
    if (ctx->it->count == 0) PARSER_ERROR("unexpected s-expr end, ) was expected here", ctx);
    if (ctx->it->data[0] == SEXPR_TOKEN_LIST_END) break;
    if (exprs.count && spaces.count == 0) PARSER_ERROR("space was expected here", ctx);
    if (ctx->it->data[0] == SEXPR_TOKEN_DOT && isspace(ctx->it->data[1])) {
      sv_chop_left(ctx->it, 1);
      spaces = chop_spaces(ctx);
      da_append(&exprs, parse__sexpr(ctx));
      if (ctx->it->data[0] != SEXPR_TOKEN_LIST_END) PARSER_ERROR("expected list end here", ctx);
      sv_chop_left(ctx->it, 1);
      goto result;
    }
    da_append(&exprs, parse__sexpr(ctx));
  }
  sv_chop_left(ctx->it, 1);
  da_append(&exprs, NULL);
result:
  SExpr *list = make_sexpr_list_opt(exprs.count, exprs.items);
  da_free(exprs);
  return list;
}

SExpr *parse__sexpr_quote(SExpr_Parser_Context *ctx) {
  if (ctx->it->data[0] != SEXPR_TOKEN_QUOTES) PARSER_ERROR("expected ( symbol here", ctx);
  sv_chop_left(ctx->it, 1);
  return make_sexpr_quote(parse__sexpr(ctx));
}

SExpr *parse__sexpr_string(SExpr_Parser_Context *ctx) {
  if (ctx->it->data[0] != SEXPR_TOKEN_DQUOTES) PARSER_ERROR("expected string literal start here", ctx);
  sv_chop_left(ctx->it, 1);
  size_t i = 0;
  while (ctx->it->data[i] != SEXPR_TOKEN_DQUOTES) {
    if (ctx->it->data[i] == SEXPR_TOKEN_ESCAPE) i += 1;
    i += 1;
    if (i >= ctx->it->count) PARSER_ERROR("expected string literal end here", ctx);
  }
  String_View value = sv_from_parts(ctx->it->data, i);
  i += 1;
  ctx->it->count -= i;
  ctx->it->data += i;
  return make_sexpr_string(value);
}

SExpr *parse__sexpr_guarded_symbol(SExpr_Parser_Context *ctx) {
  if (ctx->it->data[0] != SEXPR_TOKEN_GUARD) PARSER_ERROR("expected guarded symbol start here", ctx);
  sv_chop_left(ctx->it, 1);
  size_t i = 0;
  while (ctx->it->data[i] != SEXPR_TOKEN_GUARD) {
    i += 1;
    if (i >= ctx->it->count) PARSER_ERROR("expected guarded symbol end here", ctx);
  }
  String_View name = sv_from_parts(ctx->it->data, i);
  i += 1;
  ctx->it->count -= i;
  ctx->it->data += i;
  return make_sexpr_symbol(name);
}

SExpr *parse__sexpr_symbol(SExpr_Parser_Context *ctx) {
  if (ctx->it->data[0] == SEXPR_TOKEN_GUARD) return parse__sexpr_guarded_symbol(ctx);
  String_View name = sv_chop_while(ctx->it, issymbol);
  if (!name.count) PARSER_ERROR("unexpected empty symbol name", ctx);
  return make_sexpr_symbol(name);
}

SExpr *parse__sexpr_nullable(SExpr_Parser_Context *ctx) {
  char current = ctx->it->data[0];
  if (current == SEXPR_TOKEN_COMMENT) {
    size_t i = 0;
    while (i < ctx->it->count && !islineend(ctx->it->data[i])) i += 1;
    i += 1;
    ctx->it->count -= i;
    ctx->it->data += i;
    current = ctx->it->data[0];
  }
  if (!ctx->it->count) return NULL;
  if (current == SEXPR_TOKEN_LIST_START) return parse__sexpr_list(ctx);
  if (current == SEXPR_TOKEN_QUOTES) return parse__sexpr_quote(ctx);
  if (current == SEXPR_TOKEN_DQUOTES) return parse__sexpr_string(ctx);
  if (current == SEXPR_TOKEN_HYPHEN) {
    if (isspace(ctx->it->data[1])) return parse__sexpr_symbol(ctx);
    return parse__sexpr_integer(ctx);
  }
  if (isdigit(current)) return parse__sexpr_integer(ctx);
  if (current == SEXPR_TOKEN_DOT) return parse__sexpr_fractional(ctx, 0);
  return parse__sexpr_symbol(ctx);
}

SExpr *parse__sexpr(SExpr_Parser_Context *ctx) {
  SExpr *expr = parse__sexpr_nullable(ctx);
  if (!expr) PARSER_ERROR("expression expected", ctx);
  return expr;
}

SExpr *parse_sexpr(String_View *source) {
  SExpr_Parser_Context ctx = {.source = *source, .it = source, .line = *source, .linenumber = 0};
  chop_spaces(&ctx);
  if (!ctx.it->count) PARSER_ERROR("unexpected end of s-expression", ctx);
  return parse__sexpr_nullable(&ctx);
}

SExprs *parse_sexprs(String_View *source) {
  SExpr_Parser_Context ctx = {.source = *source, .it = source, .line = *source, .linenumber = 0};
  SExprs *exprs = rc_alloc(sizeof(SExprs), da_destructor);
  while (ctx.it->count) {
    chop_spaces(&ctx);
    if (!ctx.it->count) break;
    SExpr *expr = parse__sexpr_nullable(&ctx);
    if (expr) da_append(exprs, rc_acquire(expr));
  }
  return exprs;
}

void parser__sexprs_for_each_iterator_destructor(void *data) {
  struct Sexprs_Iterator *it = data;
  free(it->ctx.it);
  da_destructor(&it->exprs);
}

SExpr ***parser__make_sexprs_for_each_iterator(const char *source_src) {
  struct Sexprs_Iterator *it = rc_acquire(rc_alloc(sizeof(struct Sexprs_Iterator), parser__sexprs_for_each_iterator_destructor));
  String_View source_it = sv_from_cstr(source_src);
  it->ctx.source = source_it;
  it->ctx.it = malloc(sizeof(String_View));
  it->ctx.it->data = source_it.data;
  it->ctx.it->count = source_it.count;
  it->ctx.line = source_it;
  it->ctx.linenumber = 0;
  da_reserve(&it->exprs, 1);
  return &it->exprs.items;
}

bool parser__sexprs_for_each_next(SExpr ****_ctx, SExpr **expr) {
  struct Sexprs_Iterator *it = (struct Sexprs_Iterator *)((SExpr_Parser_Context *)(*_ctx) - 1);
  chop_spaces(&it->ctx);
  if (!it->ctx.it->count) {
    rc_release(it);
    return false;
  }
  SExpr *parsed = parse__sexpr_nullable(&it->ctx);
  if (!parsed) return parser__sexprs_for_each_next(_ctx, expr);
  da_append(&it->exprs, rc_acquire(parsed));
  (*_ctx) = &it->exprs.items;
  (*expr) = parsed;
  return true;
}

SExpr_Parser_Context *parser__sexprs_for_each_ctx(SExpr **exprs) {
  struct Sexprs_Iterator *it = (struct Sexprs_Iterator *)((SExpr_Parser_Context *)exprs - 1);
  return &it->ctx;
}

#endif // SEXPR_PARSER_IMPL
