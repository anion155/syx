#ifndef SYX_PARSER_H
#define SYX_PARSER_H

#include <ht.h>
#include <nob.h>
#include <rc.h>

#include "syx_value.h"
#include "syx_vector.h"

SyxV *parse_syxv(String_View *source);
SyxV_Vector *parse_syxvs(String_View *source);

typedef struct SyxV_Parser_Context {
  String_View source;
  String_View *it;
  String_View line;
  size_t linenumber;
} SyxV_Parser_Context;

struct SyxVs_Iterator {
  SyxV_Parser_Context ctx;
  SyxV_Vector syxvs;
};

SyxV ***parse__make_syxvs_for_each_iterator(const char *source_src);
bool parse__syxvs_for_each_next(SyxV ****syxvs, SyxV **expr);
#define parser_syxvs_for_each(name, source)                                       \
  for (                                                                           \
      SyxV *name = NULL, ***_ctx = parse__make_syxvs_for_each_iterator((source)); \
      parse__syxvs_for_each_next(&_ctx, &name);)
struct SyxVs_Iterator *parse__syxvs_for_each_iterator(SyxV ***syxvs);
#define parser_syxvs_for_each_iterator() parse__syxvs_for_each_iterator(_ctx)

#endif // SYX_PARSER_H
#if defined(SYX_PARSER_IMPL) && !defined(SYX_PARSER_IMPL_C)
#define SYX_PARSER_IMPL_C

#define SYX_VALUE_IMPL
#include "syx_value.h"
#define SYX_UTILS_IMPL
#include "syx_utils.h"

#define SYXV_TOKEN_LIST_START '('
#define SYXV_TOKEN_LIST_END ')'
#define SYXV_TOKEN_BOXED_START '['
#define SYXV_TOKEN_BOXED_END ']'
#define SYXV_TOKEN_QUOTES '\''
#define SYXV_TOKEN_COMMA ','
#define SYXV_TOKEN_DQUOTES '"'
#define SYXV_TOKEN_ESCAPE '\\'
#define SYXV_TOKEN_HASH '#'
#define SYXV_TOKEN_HYPHEN '-'
#define SYXV_TOKEN_DOT '.'
#define SYXV_TOKEN_DIGIT_0 '0'
#define SYXV_TOKEN_GUARD '|'
#define SYXV_TOKEN_COMMENT ';'

#define PARSER_ERROR(ctx, message, ...)            \
  do {                                             \
    rc_release_all(__VA_ARGS__);                   \
    SyxV *reason = make_syxv_string_cstr(message); \
    return make_syxv_thrown(NULL, reason);         \
  } while (0)

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
  if (!parse_fractions(ctx->it, &fractional_part)) PARSER_ERROR(ctx, "expected fractional number's fractional part start here");
  if (integer_part < 0) fractional_part = integer_part - fractional_part;
  else fractional_part = integer_part + fractional_part;
  return make_syxv_number_fractional(fractional_part);
}

SyxV *parse__syxv_integer(SyxV_Parser_Context *ctx) {
  if (!ctx->it->count) PARSER_ERROR(ctx, "expected number literal here");
  if (ctx->it->data[0] == '0' && ctx->it->count >= 3) {
    if ((ctx->it->data[1] == 'x' || ctx->it->data[1] == 'X')) TODO("implement hex number parser");
    if ((ctx->it->data[1] == 'o' || ctx->it->data[1] == 'O')) TODO("implement octal number parser");
    if ((ctx->it->data[1] == 'b' || ctx->it->data[1] == 'B')) TODO("implement binary number parser");
  }
  syx_integer_t value = 0;
  if (ctx->it->data[0] == SYXV_TOKEN_DOT) goto upgrade;
  if (!parse_integer(ctx->it, &value)) PARSER_ERROR(ctx, "expected number literal here");
  if (ctx->it->data[0] == SYXV_TOKEN_DOT) goto upgrade;
  return make_syxv_number_integer(value);
upgrade:
  return parse__syxv_fractional(ctx, value);
}

SyxV *parse__syxv_list(SyxV_Parser_Context *ctx, bool list_expected, char opening_parenthesis, char closing_parenthesis) {
  if (ctx->it->data[0] != opening_parenthesis) PARSER_ERROR(ctx, "expected ( symbol here");
  sv_chop_left(ctx->it, 1);
  SyxV_Vector *syxvs = rc_acquire(rc_malloc(sizeof(SyxV_Vector), .destructor = da_destructor));
  while (true) {
    String_View spaces = chop_spaces(ctx);
    if (ctx->it->count == 0) PARSER_ERROR(ctx, "unexpected s-expr end, ) was expected here", syxvs);
    if (ctx->it->data[0] == closing_parenthesis) break;
    if (syxvs->count && spaces.count == 0 && ctx->it->data[0] != SYXV_TOKEN_LIST_START) PARSER_ERROR(ctx, "space was expected here", syxvs);
    if (ctx->it->data[0] == SYXV_TOKEN_DOT && ctx->it->count > 1 && isspace(ctx->it->data[1])) {
      if (list_expected) PARSER_ERROR(ctx, "unexpected pair syntax", syxvs);
      sv_chop_left(ctx->it, 1);
      chop_spaces(ctx);
      SyxV *value = parse__syxv(ctx);
      syx_eval_early_exit(value, syxvs);
      da_append(syxvs, rc_acquire(value));
      chop_spaces(ctx);
      if (ctx->it->data[0] != closing_parenthesis) PARSER_ERROR(ctx, "expected list end here", syxvs);
      goto result;
    }
    SyxV *value = parse__syxv(ctx);
    syx_eval_early_exit(value, syxvs);
    da_append(syxvs, rc_acquire(value));
  }
  da_append(syxvs, NULL);
result:
  sv_chop_left(ctx->it, 1);
  SyxV *list = rc_acquire(make_syxv_list_opt(syxvs->count, syxvs->items));
  rc_release(syxvs);
  return rc_move(list);
}

SyxV *parse__syxv_quote(SyxV_Parser_Context *ctx) {
  if (ctx->it->data[0] != SYXV_TOKEN_QUOTES) PARSER_ERROR(ctx, "expected ' symbol here");
  sv_chop_left(ctx->it, 1);
  SyxV *value = parse__syxv(ctx);
  syx_eval_early_exit(value);
  return make_syxv_list(make_syxv_symbol_cstr("quote"), value, NULL);
}

SyxV *parse__syxv_unquote(SyxV_Parser_Context *ctx) {
  if (ctx->it->data[0] != SYXV_TOKEN_COMMA) PARSER_ERROR(ctx, "expected , symbol here");
  sv_chop_left(ctx->it, 1);
  SyxV *value = parse__syxv(ctx);
  syx_eval_early_exit(value);
  return make_syxv_list(make_syxv_symbol_cstr("unquote"), value, NULL);
}

SyxV *parse__syxv_string(SyxV_Parser_Context *ctx) {
  if (ctx->it->data[0] != SYXV_TOKEN_DQUOTES) PARSER_ERROR(ctx, "expected string literal start here");
  sv_chop_left(ctx->it, 1);
  size_t i = 0;
  while (ctx->it->data[i] != SYXV_TOKEN_DQUOTES) {
    if (ctx->it->data[i] == SYXV_TOKEN_ESCAPE) i += 1;
    i += 1;
    if (i >= ctx->it->count) PARSER_ERROR(ctx, "expected string literal end here");
  }
  String_View value = sv_from_parts(ctx->it->data, i);
  i += 1;
  ctx->it->count -= i;
  ctx->it->data += i;
  return make_syxv_string_sv(value);
}

SyxV *parse__syxv_guarded_symbol(SyxV_Parser_Context *ctx) {
  if (ctx->it->data[0] != SYXV_TOKEN_GUARD) PARSER_ERROR(ctx, "expected guarded symbol start here");
  sv_chop_left(ctx->it, 1);
  size_t i = 0;
  while (ctx->it->data[i] != SYXV_TOKEN_GUARD) {
    i += 1;
    if (i >= ctx->it->count) PARSER_ERROR(ctx, "expected guarded symbol end here");
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
  if (!name.count) PARSER_ERROR(ctx, "unexpected empty symbol name");
  return make_syxv_symbol(name);
}

typedef SyxV *(*SyxV_Dispatcher)(SyxV_Parser_Context *ctx);

SyxV *parse__dispatch_syxv_nil(SyxV_Parser_Context *ctx);
SyxV *parse__dispatch_syxv_true(SyxV_Parser_Context *ctx);
SyxV *parse__dispatch_syxv_false(SyxV_Parser_Context *ctx);
SyxV *parse__dispatch_syxv_vector(SyxV_Parser_Context *ctx);
SyxV *parse__dispatch_syxv_boxed(SyxV_Parser_Context *ctx);

define_constant(Ht(char, SyxV_Dispatcher), DISPATCH_TABLE) {
  DISPATCH_TABLE->hasheq = ht_mem_hasheq;
  *ht_put(DISPATCH_TABLE, 'n') = parse__dispatch_syxv_nil;
  *ht_put(DISPATCH_TABLE, 't') = parse__dispatch_syxv_true;
  *ht_put(DISPATCH_TABLE, 'f') = parse__dispatch_syxv_false;
  *ht_put(DISPATCH_TABLE, '(') = parse__dispatch_syxv_vector;
  *ht_put(DISPATCH_TABLE, '.') = parse__dispatch_syxv_boxed;
}

SyxV *parse__syxv_dispatch(SyxV_Parser_Context *ctx) {
  if (ctx->it->data[0] != SYXV_TOKEN_HASH) PARSER_ERROR(ctx, "expected hash symbol as dispatch start here");
  sv_chop_left(ctx->it, 1);
  SyxV_Dispatcher *dispatcher = ht_find(DISPATCH_TABLE(), ctx->it->data[0]);
  if (!dispatcher) PARSER_ERROR(ctx, temp_sprintf("no dispatcher found '%c'", ctx->it->data[0]));
  return (*dispatcher)(ctx);
}

SyxV *parse__syxv_nullable(SyxV_Parser_Context *ctx);

SyxV *parse__syxv_comment(SyxV_Parser_Context *ctx) {
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

SyxV *parse__syxv_nullable(SyxV_Parser_Context *ctx) {
  char current = ctx->it->data[0];
  if (current == SYXV_TOKEN_COMMENT) return parse__syxv_comment(ctx);
  if (current == SYXV_TOKEN_LIST_START) return parse__syxv_list(ctx, false, SYXV_TOKEN_LIST_START, SYXV_TOKEN_LIST_END);
  if (current == SYXV_TOKEN_QUOTES) return parse__syxv_quote(ctx);
  if (current == SYXV_TOKEN_COMMA) return parse__syxv_unquote(ctx);
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
  if (!expr) PARSER_ERROR(ctx, "expression expected");
  return expr;
}

SyxV *parse_syxv(String_View *source) {
  SyxV_Parser_Context ctx = {.source = *source, .it = source, .line = *source, .linenumber = 0};
  chop_spaces(&ctx);
  if (!ctx.it->count) PARSER_ERROR(ctx, "unexpected end of s-expression");
  return parse__syxv_nullable(&ctx);
}

SyxV_Vector *parse_syxvs(String_View *source) {
  SyxV_Parser_Context ctx = {.source = *source, .it = source, .line = *source, .linenumber = 0};
  SyxV_Vector *syxvs = rc_malloc(sizeof(SyxV_Vector), .destructor = da_destructor);
  assert(syxvs);
  syxvs->count = 0;
  syxvs->capacity = 0;
  syxvs->items = NULL;
  while (ctx.it->count) {
    chop_spaces(&ctx);
    if (!ctx.it->count) break;
    SyxV *expr = parse__syxv_nullable(&ctx);
    syx_eval_early_exit(expr, syxvs);
    if (expr) da_append(syxvs, rc_acquire(expr));
  }
  return syxvs;
}

void parse__syxvs_for_each_iterator_destructor(void *data) {
  struct SyxVs_Iterator *it = data;
  da_destructor(&it->syxvs);
  free(it->ctx.it);
}

SyxV ***parse__make_syxvs_for_each_iterator(const char *source_src) {
  struct SyxVs_Iterator *it = rc_malloc(sizeof(struct SyxVs_Iterator), .destructor = parse__syxvs_for_each_iterator_destructor);
  assert(it);
  rc_acquire(it);
  String_View source_sv = sv_from_cstr(source_src);
  String_View *source_it = malloc(sizeof(String_View));
  source_it->data = source_sv.data;
  source_it->count = source_sv.count;
  it->ctx = (SyxV_Parser_Context){.source = source_sv, .it = source_it, .line = source_sv, .linenumber = 0};
  it->syxvs = (SyxV_Vector){0};
  da_reserve(&it->syxvs, 1);
  return &it->syxvs.items;
}

bool parse__syxvs_for_each_next(SyxV ****_ctx, SyxV **expr) {
  struct SyxVs_Iterator *it = parse__syxvs_for_each_iterator(*_ctx);
  chop_spaces(&it->ctx);
  if (!it->ctx.it->count) {
    rc_release(it);
    return false;
  }
  SyxV *parsed = parse__syxv_nullable(&it->ctx);
  if (!parsed) return parse__syxvs_for_each_next(_ctx, expr);
  da_append(&it->syxvs, rc_acquire(parsed));
  (*_ctx) = &it->syxvs.items;
  (*expr) = parsed;
  return true;
}

struct SyxVs_Iterator *parse__syxvs_for_each_iterator(SyxV ***syxvs) {
  return (struct SyxVs_Iterator *)((SyxV_Parser_Context *)syxvs - 1);
}

SyxV *parse__dispatch_syxv_nil(SyxV_Parser_Context *ctx) {
  if (sv_starts_with(*ctx->it, sv_from_cstr("nil"))) sv_chop_left(ctx->it, 3);
  else if (sv_starts_with(*ctx->it, sv_from_cstr("null"))) sv_chop_left(ctx->it, 4);
  else sv_chop_left(ctx->it, 1);
  return make_syxv_nil();
}

SyxV *parse__dispatch_syxv_true(SyxV_Parser_Context *ctx) {
  if (sv_starts_with(*ctx->it, sv_from_cstr("true"))) sv_chop_left(ctx->it, 4);
  else sv_chop_left(ctx->it, 1);
  return make_syxv_bool(true);
}

SyxV *parse__dispatch_syxv_false(SyxV_Parser_Context *ctx) {
  if (sv_starts_with(*ctx->it, sv_from_cstr("false"))) sv_chop_left(ctx->it, 5);
  else sv_chop_left(ctx->it, 1);
  return make_syxv_bool(false);
}

SyxV *parse__dispatch_syxv_vector(SyxV_Parser_Context *ctx) {
  SyxV *arguments = parse__syxv_list(ctx, true, SYXV_TOKEN_LIST_START, SYXV_TOKEN_LIST_END);
  syx_eval_early_exit(arguments);
  return make_syxv_pair(make_syxv_symbol_cstr("new"), make_syxv_pair(make_syxv_symbol_cstr("vector"), arguments));
}

SyxV *parse__dispatch_syxv_boxed(SyxV_Parser_Context *ctx) {
  if (ctx->it->data[0] != SYXV_TOKEN_DOT) PARSER_ERROR(ctx, "expected . symbol here");
  sv_chop_left(ctx->it, 1);
  SyxV *constructor_symbol = parse__syxv_symbol(ctx);
  syx_eval_early_exit(constructor_symbol);
  rc_acquire(constructor_symbol);
  SyxV *arguments = parse__syxv_list(ctx, true, SYXV_TOKEN_LIST_START, SYXV_TOKEN_LIST_END);
  syx_eval_early_exit(arguments, constructor_symbol);
  return make_syxv_pair(make_syxv_symbol_cstr("new"), make_syxv_pair(rc_move(constructor_symbol), arguments));
}

#endif // SYX_PARSER_IMPL
