#ifndef SYX_PARSER_H
#define SYX_PARSER_H

#include <ht.h>
#include <nob.h>
#include <rc.h>

#include "syx_value.h"

SyxV *parse_syxv(String_View *source);
SyxVs *parse_syxvs(String_View *source);

typedef struct SyxV_Parser_Context {
  String_View source;
  String_View *it;
  String_View line;
  size_t linenumber;
} SyxV_Parser_Context;

struct SyxVs_Iterator {
  SyxV_Parser_Context ctx;
  SyxVs syxvs;
};

SyxV ***parser__make_syxvs_for_each_iterator(const char *source_src);
bool parser__syxvs_for_each_next(SyxV ****syxvs, SyxV **expr);
#define parser_syxvs_for_each(name, source)                                        \
  for (                                                                            \
      SyxV *name = NULL, ***_ctx = parser__make_syxvs_for_each_iterator((source)); \
      parser__syxvs_for_each_next(&_ctx, &name);)
struct SyxVs_Iterator *parser__syxvs_for_each_iterator(SyxV ***syxvs);
#define parser_syxvs_for_each_iterator() parser__syxvs_for_each_iterator(_ctx)

#endif // SYX_PARSER_H
#if defined(SYX_PARSER_IMPL) && !defined(SYX_PARSER_IMPL_C)
#define SYX_PARSER_IMPL_C

#define SYX_VALUE_IMPL
#include "syx_value.h"
#define SYX_UTILS_IMPL
#include "syx_utils.h"

#define SYXV_TOKEN_LIST_START '('
#define SYXV_TOKEN_LIST_END ')'
#define SYXV_TOKEN_QUOTES '\''
#define SYXV_TOKEN_DQUOTES '"'
#define SYXV_TOKEN_ESCAPE '\\'
#define SYXV_TOKEN_HASH '#'
#define SYXV_TOKEN_HYPHEN '-'
#define SYXV_TOKEN_DOT '.'
#define SYXV_TOKEN_DIGIT_0 '0'
#define SYXV_TOKEN_GUARD '|'
#define SYXV_TOKEN_COMMENT ';'

#define PARSER_ERROR(message, ctx) UNREACHABLE((UNUSED(ctx), (message)))

String_View chop_spaces(SyxV_Parser_Context *ctx) {
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

SyxV *parse__syxv(SyxV_Parser_Context *ctx);

SyxV *parse__syxv_fractional(SyxV_Parser_Context *ctx, syx_integer_t integer_part) {
  syx_fractional_t fractional_part = 0;
  if (!parse_fractions(ctx->it, &fractional_part)) PARSER_ERROR("expected fractional number's fractional part start here", ctx);
  if (integer_part < 0) fractional_part = integer_part - fractional_part;
  else fractional_part = integer_part + fractional_part;
  return make_syxv_fractional(fractional_part);
}

SyxV *parse__syxv_integer(SyxV_Parser_Context *ctx) {
  if (!ctx->it->count) PARSER_ERROR("expected number literal here", ctx);
  if (ctx->it->data[0] == '0' && ctx->it->count >= 3) {
    if ((ctx->it->data[1] == 'x' || ctx->it->data[1] == 'X')) TODO("implement hex number parser");
    if ((ctx->it->data[1] == 'o' || ctx->it->data[1] == 'O')) TODO("implement octal number parser");
    if ((ctx->it->data[1] == 'b' || ctx->it->data[1] == 'B')) TODO("implement binary number parser");
  }
  syx_integer_t value = 0;
  if (ctx->it->data[0] == SYXV_TOKEN_DOT) goto upgrade;
  if (!parse_integer(ctx->it, &value)) PARSER_ERROR("expected number literal here", ctx);
  if (ctx->it->data[0] == SYXV_TOKEN_DOT) goto upgrade;
  return make_syxv_integer(value);
upgrade:
  return parse__syxv_fractional(ctx, value);
}

SyxV *parse__syxv_list(SyxV_Parser_Context *ctx) {
  if (ctx->it->data[0] != SYXV_TOKEN_LIST_START) PARSER_ERROR("expected ( symbol here", ctx);
  sv_chop_left(ctx->it, 1);
  SyxVs syxvs = {0};
  while (true) {
    String_View spaces = chop_spaces(ctx);
    if (ctx->it->count == 0) PARSER_ERROR("unexpected s-expr end, ) was expected here", ctx);
    if (ctx->it->data[0] == SYXV_TOKEN_LIST_END) break;
    if (syxvs.count && spaces.count == 0 && ctx->it->data[0] != SYXV_TOKEN_LIST_START) PARSER_ERROR("space was expected here", ctx);
    if (ctx->it->data[0] == SYXV_TOKEN_DOT && ctx->it->count > 1 && isspace(ctx->it->data[1])) {
      sv_chop_left(ctx->it, 1);
      chop_spaces(ctx);
      da_append(&syxvs, parse__syxv(ctx));
      chop_spaces(ctx);
      if (ctx->it->data[0] != SYXV_TOKEN_LIST_END) PARSER_ERROR("expected list end here", ctx);
      goto result;
    }
    da_append(&syxvs, parse__syxv(ctx));
  }
  da_append(&syxvs, NULL);
result:
  sv_chop_left(ctx->it, 1);
  SyxV *list = make_syxv_list_opt(syxvs.count, syxvs.items);
  da_free(syxvs);
  return list;
}

SyxV *parse__syxv_quote(SyxV_Parser_Context *ctx) {
  if (ctx->it->data[0] != SYXV_TOKEN_QUOTES) PARSER_ERROR("expected ( symbol here", ctx);
  sv_chop_left(ctx->it, 1);
  return make_syxv_quote(parse__syxv(ctx));
}

SyxV *parse__syxv_string(SyxV_Parser_Context *ctx) {
  if (ctx->it->data[0] != SYXV_TOKEN_DQUOTES) PARSER_ERROR("expected string literal start here", ctx);
  sv_chop_left(ctx->it, 1);
  size_t i = 0;
  while (ctx->it->data[i] != SYXV_TOKEN_DQUOTES) {
    if (ctx->it->data[i] == SYXV_TOKEN_ESCAPE) i += 1;
    i += 1;
    if (i >= ctx->it->count) PARSER_ERROR("expected string literal end here", ctx);
  }
  String_View value = sv_from_parts(ctx->it->data, i);
  i += 1;
  ctx->it->count -= i;
  ctx->it->data += i;
  return make_syxv_string_sv(value);
}

SyxV *parse__syxv_guarded_symbol(SyxV_Parser_Context *ctx) {
  if (ctx->it->data[0] != SYXV_TOKEN_GUARD) PARSER_ERROR("expected guarded symbol start here", ctx);
  sv_chop_left(ctx->it, 1);
  size_t i = 0;
  while (ctx->it->data[i] != SYXV_TOKEN_GUARD) {
    i += 1;
    if (i >= ctx->it->count) PARSER_ERROR("expected guarded symbol end here", ctx);
  }
  String_View name = sv_from_parts(ctx->it->data, i);
  i += 1;
  ctx->it->count -= i;
  ctx->it->data += i;
  return make_syxv_symbol(name);
}

SyxV *parse__syxv_symbol(SyxV_Parser_Context *ctx) {
  if (ctx->it->data[0] == SYXV_TOKEN_GUARD) return parse__syxv_guarded_symbol(ctx);
  String_View name = sv_chop_while(ctx->it, issymbol);
  if (!name.count) PARSER_ERROR("unexpected empty symbol name", ctx);
  return make_syxv_symbol(name);
}

typedef SyxV *(*SyxV_Dispatcher)(SyxV_Parser_Context *ctx);

SyxV *parser__dispatch_syxv_nil(SyxV_Parser_Context *ctx);
SyxV *parser__dispatch_syxv_true(SyxV_Parser_Context *ctx);
SyxV *parser__dispatch_syxv_false(SyxV_Parser_Context *ctx);

define_constant(Ht(char, SyxV_Dispatcher), DISPATCH_TABLE) {
  DISPATCH_TABLE->hasheq = ht_mem_hasheq;
  *ht_put(DISPATCH_TABLE, 'n') = parser__dispatch_syxv_nil;
  *ht_put(DISPATCH_TABLE, 't') = parser__dispatch_syxv_true;
  *ht_put(DISPATCH_TABLE, 'f') = parser__dispatch_syxv_false;
}

SyxV *parse__syxv_dispatch(SyxV_Parser_Context *ctx) {
  if (ctx->it->data[0] != SYXV_TOKEN_HASH) PARSER_ERROR("expected hash symbol as dispatch start here", ctx);
  sv_chop_left(ctx->it, 1);
  SyxV_Dispatcher *dispatcher = ht_find(DISPATCH_TABLE(), ctx->it->data[0]);
  if (!dispatcher) PARSER_ERROR(temp_sprintf("no dispatcher found '%c'", ctx->it->data[0]), ctx);
  sv_chop_left(ctx->it, 1);
  return (*dispatcher)(ctx);
}

SyxV *parse__syxv_nullable(SyxV_Parser_Context *ctx) {
  char current = ctx->it->data[0];
  if (current == SYXV_TOKEN_COMMENT) {
    size_t i = 0;
    while (i < ctx->it->count && !islineend(ctx->it->data[i])) i += 1;
    ctx->it->count -= i;
    ctx->it->data += i;
    if (!ctx->it->count) return NULL;
    sv_chop_left(ctx->it, 1);
    if (!ctx->it->count) return NULL;
    chop_spaces(ctx);
    if (!ctx->it->count) return NULL;
    return parse__syxv_nullable(ctx);
  }
  if (current == SYXV_TOKEN_LIST_START) return parse__syxv_list(ctx);
  if (current == SYXV_TOKEN_QUOTES) return parse__syxv_quote(ctx);
  if (current == SYXV_TOKEN_DQUOTES) return parse__syxv_string(ctx);
  if (current == SYXV_TOKEN_HASH) return parse__syxv_dispatch(ctx);
  if (current == SYXV_TOKEN_HYPHEN) {
    if (ctx->it->count > 1 && isspace(ctx->it->data[1])) return parse__syxv_symbol(ctx);
    return parse__syxv_integer(ctx);
  }
  if (isdigit(current)) return parse__syxv_integer(ctx);
  if (current == SYXV_TOKEN_DOT) return parse__syxv_fractional(ctx, 0);
  return parse__syxv_symbol(ctx);
}

SyxV *parse__syxv(SyxV_Parser_Context *ctx) {
  SyxV *expr = parse__syxv_nullable(ctx);
  if (!expr) PARSER_ERROR("expression expected", ctx);
  return expr;
}

SyxV *parse_syxv(String_View *source) {
  SyxV_Parser_Context ctx = {.source = *source, .it = source, .line = *source, .linenumber = 0};
  chop_spaces(&ctx);
  if (!ctx.it->count) PARSER_ERROR("unexpected end of s-expression", ctx);
  return parse__syxv_nullable(&ctx);
}

SyxVs *parse_syxvs(String_View *source) {
  SyxV_Parser_Context ctx = {.source = *source, .it = source, .line = *source, .linenumber = 0};
  SyxVs *syxvs = rc_alloc(sizeof(SyxVs), da_destructor);
  syxvs->count = 0;
  syxvs->capacity = 0;
  syxvs->items = NULL;
  while (ctx.it->count) {
    chop_spaces(&ctx);
    if (!ctx.it->count) break;
    SyxV *expr = parse__syxv_nullable(&ctx);
    if (expr) da_append(syxvs, rc_acquire(expr));
  }
  return syxvs;
}

void parser__syxvs_for_each_iterator_destructor(void *data) {
  struct SyxVs_Iterator *it = data;
  da_destructor(&it->syxvs);
  free(it->ctx.it);
}

SyxV ***parser__make_syxvs_for_each_iterator(const char *source_src) {
  struct SyxVs_Iterator *it = rc_acquire(rc_alloc(sizeof(struct SyxVs_Iterator), parser__syxvs_for_each_iterator_destructor));
  String_View source_sv = sv_from_cstr(source_src);
  String_View *source_it = malloc(sizeof(String_View));
  source_it->data = source_sv.data;
  source_it->count = source_sv.count;
  it->ctx = (SyxV_Parser_Context){.source = source_sv, .it = source_it, .line = source_sv, .linenumber = 0};
  it->syxvs = (SyxVs){0};
  da_reserve(&it->syxvs, 1);
  return &it->syxvs.items;
}

bool parser__syxvs_for_each_next(SyxV ****_ctx, SyxV **expr) {
  struct SyxVs_Iterator *it = parser__syxvs_for_each_iterator(*_ctx);
  chop_spaces(&it->ctx);
  if (!it->ctx.it->count) {
    rc_release(it);
    return false;
  }
  SyxV *parsed = parse__syxv_nullable(&it->ctx);
  if (!parsed) return parser__syxvs_for_each_next(_ctx, expr);
  da_append(&it->syxvs, rc_acquire(parsed));
  (*_ctx) = &it->syxvs.items;
  (*expr) = parsed;
  return true;
}

struct SyxVs_Iterator *parser__syxvs_for_each_iterator(SyxV ***syxvs) {
  return (struct SyxVs_Iterator *)((SyxV_Parser_Context *)syxvs - 1);
}

SyxV *parser__dispatch_syxv_nil(SyxV_Parser_Context *ctx) {
  if (sv_starts_with(*ctx->it, sv_from_cstr("il"))) sv_chop_left(ctx->it, 2);
  else if (sv_starts_with(*ctx->it, sv_from_cstr("ull"))) sv_chop_left(ctx->it, 3);
  return make_syxv_nil();
}

SyxV *parser__dispatch_syxv_true(SyxV_Parser_Context *ctx) {
  if (sv_starts_with(*ctx->it, sv_from_cstr("rue"))) sv_chop_left(ctx->it, 3);
  return make_syxv_bool(true);
}

SyxV *parser__dispatch_syxv_false(SyxV_Parser_Context *ctx) {
  if (sv_starts_with(*ctx->it, sv_from_cstr("alse"))) sv_chop_left(ctx->it, 4);
  return make_syxv_bool(false);
}

#endif // SYX_PARSER_IMPL
