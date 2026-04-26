#ifndef EXPR_AST_H
#define EXPR_AST_H

#include <stdlib.h>
#include <nob.h>
#include <ht.h>
#include <rc.h>
#include "expr_utils.h"

typedef enum : unsigned int {
  EXPR_KIND_NIL,
  EXPR_KIND_SYMBOL,
  EXPR_KIND_PAIR,
  EXPR_KIND_BOOL,
  EXPR_KIND_INTEGER,
  EXPR_KIND_REAL,
  EXPR_KIND_STRING,
  EXPR_KIND_QUOTE,
} Expr_Kind;

typedef struct Expr_Symbol {
  char *name;
  bool guarded;
} Expr_Symbol;

typedef struct Expr Expr;
struct Expr {
  Expr_Kind kind;
  union {
    Expr_Symbol symbol;
    struct Expr_Pair {
      Expr *left;
      Expr *right;
    } pair;
    expr_bool_t boolean;
    expr_int_t integer;
    expr_real_t real;
    expr_string_t string;
    Expr *quote;
  };
};

#define UNREACHABLE_EXPR(message, ...) \
  (fprintf(stderr, "%s:%d: UNREACHABLE: %s\n", __FILE__, __LINE__, (message)), abort() __VA_OPT__(,) __VA_ARGS__)
#define expr_list_for_each(first, value) for (      \
  Expr *value##_it = (first),                       \
    *value = value##_it->kind == EXPR_KIND_PAIR     \
      ? value##_it->pair.left                       \
      : UNREACHABLE_EXPR("malformed list", NULL);   \
  value##_it->kind != EXPR_KIND_NIL;                \
  value##_it = value##_it->kind == EXPR_KIND_PAIR   \
    ? value##_it->pair.right                        \
    : EXPR_NIL,                                     \
  value = value##_it->kind == EXPR_KIND_PAIR        \
    ? value##_it->pair.left                         \
    : value##_it->kind == EXPR_KIND_NIL             \
      ? NULL                                        \
      : UNREACHABLE_EXPR("malformed list", NULL)    \
)

void expr_destructor(void *data);
Expr *make_expr_nil();
Expr *make_expr_symbol(String_View symbol);
Expr *make_expr_symbol_n(char *symbol, size_t size);
Expr *make_expr_symbol_ctrs(char *symbol);
Expr *make_expr_pair(Expr *left, Expr *right);
Expr *make_expr_bool(expr_bool_t value);
Expr *make_expr_integer(expr_int_t value);
Expr *make_expr_real(expr_real_t value);
Expr *make_expr_string(String_View value);
Expr *make_expr_string_n(expr_string_t value, size_t size);
Expr *make_expr_string_cstr(expr_string_t value);
Expr *make_expr_quote(Expr *quote);

#define make_expr_value(value)               \
  _Generic(value,                            \
    expr_bool_t:   make_expr_bool(value),    \
    expr_int_t:    make_expr_integer(value), \
    expr_real_t:   make_expr_real(value),    \
    expr_string_t: make_expr_string(value)   \
  )

Expr *make_expr_list_opt(size_t count, Expr **items);
#define make_expr_list(...)                           \
  make_expr_list_opt(                                 \
    sizeof((Expr *[]){__VA_ARGS__}) / sizeof(Expr *), \
    (Expr *[]){__VA_ARGS__}                           \
  )

#endif // EXPR_AST_H

#if defined(EXPR_AST_IMPL) && !defined(EXPR_AST_IMPL_C)
#define EXPR_AST_IMPL_C

#include <stdio.h>
#include <nob.h>
#define EXPR_UTILS_IMPL
#include "expr_utils.h"

void expr_destructor(void *data) {
  Expr *expr = data;
  switch (expr->kind) {
    case EXPR_KIND_NIL: break;
    case EXPR_KIND_SYMBOL: rc_release(expr->symbol.name); break;
    case EXPR_KIND_PAIR: rc_release(expr->pair.left); rc_release(expr->pair.right); break;
    case EXPR_KIND_BOOL: break;
    case EXPR_KIND_INTEGER: break;
    case EXPR_KIND_REAL: break;
    case EXPR_KIND_STRING: rc_release(expr->string); break;
    case EXPR_KIND_QUOTE: rc_release(expr->quote); break;
  }
}

Expr *make_expr(Expr_Kind kind) {
  Expr *expr = rc_alloc(sizeof(Expr), expr_destructor);
  expr->kind = kind;
  return expr;
}

Expr *EXPR_NIL = NULL;
Expr *EXPR_TRUE = NULL;
Expr *EXPR_FALSE = NULL;
typedef Ht(String_View, Expr *, Expr_Constants) Expr_Constants;
Expr_Constants EXPR_CONSTANTS = {.hasheq = ht_sv_hasheq};
Expr_Constants *expr_constants() {
  if (EXPR_CONSTANTS.count) return &EXPR_CONSTANTS;
  EXPR_NIL = make_expr(EXPR_KIND_NIL);
  EXPR_TRUE = make_expr(EXPR_KIND_BOOL); EXPR_TRUE->boolean = true;
  EXPR_FALSE = make_expr(EXPR_KIND_BOOL); EXPR_FALSE->boolean = false;
  *ht_put(&EXPR_CONSTANTS, sv_from_cstr("nil")) = rc_acquire(EXPR_NIL);
  *ht_put(&EXPR_CONSTANTS, sv_from_cstr("true")) = rc_acquire(EXPR_TRUE);
  *ht_put(&EXPR_CONSTANTS, sv_from_cstr("#t")) = rc_acquire(EXPR_TRUE);
  *ht_put(&EXPR_CONSTANTS, sv_from_cstr("false")) = rc_acquire(EXPR_FALSE);
  *ht_put(&EXPR_CONSTANTS, sv_from_cstr("#f")) = rc_acquire(EXPR_FALSE);
  return &EXPR_CONSTANTS;
}

Expr *make_expr_nil() {
  expr_constants();
  return EXPR_NIL;
}
Expr *make_expr_symbol(String_View symbol) {
  Expr **constant = ht_find(expr_constants(), symbol);
  if (constant != NULL) return *constant;
  bool guarded = false;
  for (String_View it = symbol; it.count; sv_chop_left(&it, 1)) {
    if (issymbol(*it.data)) continue;
    guarded = true;
    break;
  }
  Expr *expr = make_expr(EXPR_KIND_SYMBOL);
  char *name = rc_manage_copy(symbol.data, (symbol.count + 1) * sizeof(char));
  name[symbol.count] = 0;
  expr->symbol.name = name;
  expr->symbol.guarded = guarded;
  return expr;
}
Expr *make_expr_symbol_n(char *symbol, size_t size) {
  return make_expr_symbol((String_View){.data = symbol, .count = size});
}
Expr *make_expr_symbol_ctrs(char *symbol) {
  return make_expr_symbol(sv_from_cstr(symbol));
}
Expr *make_expr_pair(Expr *left, Expr *right) {
  Expr *expr = make_expr(EXPR_KIND_PAIR);
  expr->pair.left = rc_acquire(left);
  expr->pair.right = rc_acquire(right);
  return expr;
}
Expr *make_expr_bool(expr_bool_t value) {
  expr_constants();
  return value ? EXPR_TRUE : EXPR_FALSE;
}
Expr *make_expr_integer(expr_int_t value) {
  Expr *expr = make_expr(EXPR_KIND_INTEGER);
  expr->integer = value;
  return expr;
}
Expr *make_expr_real(expr_real_t value) {
  Expr *expr = make_expr(EXPR_KIND_REAL);
  expr->real = value;
  return expr;
}
Expr *make_expr_string(String_View value) {
  Expr *expr = make_expr(EXPR_KIND_STRING);
  char *string = rc_manage_copy(value.data, (value.count + 1) * sizeof(char));
  string[value.count] = 0;
  expr->string = string;
  return expr;
}
Expr *make_expr_string_n(expr_string_t value, size_t size) {
  return make_expr_string((String_View){.data = value, .count = size});
}
Expr *make_expr_string_cstr(expr_string_t value) {
  return make_expr_string(sv_from_cstr(value));
}
Expr *make_expr_quote(Expr *quote) {
  Expr *expr = make_expr(EXPR_KIND_QUOTE);
  expr->quote = rc_acquire(quote);
  return expr;
}

Expr *make_expr_list_opt(size_t count, Expr **items) {
  if (!count) UNREACHABLE("empty list array must contain [NULL]");
  Expr *expr = items[count - 1] == NULL ? EXPR_NIL : items[count - 1];
  for (ssize_t index = (ssize_t)count - 2; index >= 0; index -= 1) {
    expr = make_expr_pair(items[index], expr);
  }
  return expr;
}

#endif // EXPR_AST_IMPL
