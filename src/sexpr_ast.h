#ifndef SEXPR_AST_H
#define SEXPR_AST_H

#include "sexpr_utils.h"
#include <ht.h>
#include <nob.h>
#include <rc.h>
#include <stdlib.h>

typedef enum : unsigned int {
  SEXPR_KIND_NIL,
  SEXPR_KIND_SYMBOL,
  SEXPR_KIND_PAIR,
  SEXPR_KIND_BOOL,
  SEXPR_KIND_INTEGER,
  SEXPR_KIND_REAL,
  SEXPR_KIND_STRING,
  SEXPR_KIND_QUOTE,
} SExpr_Kind;

typedef struct SExpr_Symbol {
  char *name;
  bool guarded;
} SExpr_Symbol;

typedef struct SExpr SExpr;

struct SExpr {
  SExpr_Kind kind;

  union {
    SExpr_Symbol symbol;

    struct Expr_Pair {
      SExpr *left;
      SExpr *right;
    } pair;

    sexpr_bool_t boolean;
    sexpr_int_t integer;
    sexpr_real_t real;
    sexpr_string_t string;
    SExpr *quote;
  };
};

void sexpr_destructor(void *data);
SExpr *make_sexpr_nil();
SExpr *make_sexpr_symbol(String_View symbol);
SExpr *make_sexpr_symbol_n(const char *symbol, size_t size);
SExpr *make_sexpr_symbol_ctrs(const char *symbol);
SExpr *make_sexpr_pair(SExpr *left, SExpr *right);
SExpr *make_sexpr_bool(sexpr_bool_t value);
SExpr *make_sexpr_integer(sexpr_int_t value);
SExpr *make_sexpr_real(sexpr_real_t value);
SExpr *make_sexpr_string(String_View value);
SExpr *make_sexpr_string_n(const sexpr_string_t value, size_t size);
SExpr *make_sexpr_string_cstr(const sexpr_string_t value);
SExpr *make_sexpr_quote(SExpr *quote);

#define make_sexpr_value(value)               \
  _Generic(value,                             \
      sexpr_bool_t: make_sexpr_bool(value),   \
      sexpr_int_t: make_sexpr_integer(value), \
      sexpr_real_t: make_sexpr_real(value),   \
      sexpr_string_t: make_sexpr_string(value))

SExpr *make_sexpr_list_opt(size_t count, SExpr **items);
#define make_sexpr_list(...)                              \
  make_sexpr_list_opt(                                    \
      sizeof((SExpr *[]){__VA_ARGS__}) / sizeof(SExpr *), \
      (SExpr *[]){__VA_ARGS__})

void fprint_sexpr(FILE *f, SExpr *expr);
#define print_sexpr(expr) fprint_sexpr(stdout, (expr))

void fdump__sexpr(FILE *f, SExpr *expr, size_t current_indent, size_t next_indent);
#define fdump_sexpr(f, expr, ...) fdump__sexpr((f), (expr), WITH_TWO_DEFAULTS(0, 2, __VA_ARGS__))
#define dump_sexpr(expr, ...) fdump__sexpr(stdout, (expr), WITH_TWO_DEFAULTS(0, 2, __VA_ARGS__))

#endif // SEXPR_AST_H

#if defined(SEXPR_AST_IMPL) && !defined(SEXPR_AST_IMPL_C)
#define SEXPR_AST_IMPL_C

#include <nob.h>
#include <stdio.h>
#define SEXPR_UTILS_IMPL
#include "sexpr_utils.h"

void sexpr_destructor(void *data) {
  SExpr *expr = data;
  switch (expr->kind) {
    case SEXPR_KIND_NIL: break;
    case SEXPR_KIND_SYMBOL: rc_release(expr->symbol.name); break;
    case SEXPR_KIND_PAIR:
      rc_release(expr->pair.left);
      rc_release(expr->pair.right);
      break;
    case SEXPR_KIND_BOOL: break;
    case SEXPR_KIND_INTEGER: break;
    case SEXPR_KIND_REAL: break;
    case SEXPR_KIND_STRING: free(expr->string); break;
    case SEXPR_KIND_QUOTE: rc_release(expr->quote); break;
  }
}

SExpr *make_sexpr(SExpr_Kind kind) {
  SExpr *expr = rc_alloc(sizeof(SExpr), sexpr_destructor);
  expr->kind = kind;
  return expr;
}

SExpr *SEXPR_NIL = NULL;
SExpr *SEXPR_TRUE = NULL;
SExpr *SEXPR_FALSE = NULL;
typedef Ht(String_View, SExpr *, SExpr_Constants) SExpr_Constants;
SExpr_Constants SEXPR_CONSTANTS = {.hasheq = ht_sv_hasheq};

SExpr_Constants *sexpr_constants() {
  if (SEXPR_CONSTANTS.count) return &SEXPR_CONSTANTS;
  SEXPR_NIL = make_sexpr(SEXPR_KIND_NIL);
  SEXPR_TRUE = make_sexpr(SEXPR_KIND_BOOL);
  SEXPR_TRUE->boolean = true;
  SEXPR_FALSE = make_sexpr(SEXPR_KIND_BOOL);
  SEXPR_FALSE->boolean = false;
  *ht_put(&SEXPR_CONSTANTS, sv_from_cstr("nil")) = rc_acquire(SEXPR_NIL);
  *ht_put(&SEXPR_CONSTANTS, sv_from_cstr("true")) = rc_acquire(SEXPR_TRUE);
  *ht_put(&SEXPR_CONSTANTS, sv_from_cstr("#t")) = rc_acquire(SEXPR_TRUE);
  *ht_put(&SEXPR_CONSTANTS, sv_from_cstr("false")) = rc_acquire(SEXPR_FALSE);
  *ht_put(&SEXPR_CONSTANTS, sv_from_cstr("#f")) = rc_acquire(SEXPR_FALSE);
  return &SEXPR_CONSTANTS;
}

SExpr *make_sexpr_nil() {
  sexpr_constants();
  return SEXPR_NIL;
}

SExpr *make_sexpr_symbol(String_View symbol) {
  SExpr **constant = ht_find(sexpr_constants(), symbol);
  if (constant != NULL) return *constant;
  bool guarded = false;
  for (String_View it = symbol; it.count; sv_chop_left(&it, 1)) {
    if (issymbol(*it.data)) continue;
    guarded = true;
    break;
  }
  SExpr *expr = make_sexpr(SEXPR_KIND_SYMBOL);
  expr->symbol.name = rc_acquire(rc_manage_strndup(symbol.data, symbol.count));
  expr->symbol.guarded = guarded;
  return expr;
}

SExpr *make_sexpr_symbol_n(const char *symbol, size_t size) {
  return make_sexpr_symbol((String_View){.data = symbol, .count = size});
}

SExpr *make_sexpr_symbol_ctrs(const char *symbol) {
  return make_sexpr_symbol(sv_from_cstr(symbol));
}

SExpr *make_sexpr_pair(SExpr *left, SExpr *right) {
  SExpr *expr = make_sexpr(SEXPR_KIND_PAIR);
  expr->pair.left = left ? rc_acquire(left) : NULL;
  expr->pair.right = right ? rc_acquire(right) : NULL;
  return expr;
}

SExpr *make_sexpr_bool(sexpr_bool_t value) {
  sexpr_constants();
  return value ? SEXPR_TRUE : SEXPR_FALSE;
}

SExpr *make_sexpr_integer(sexpr_int_t value) {
  SExpr *expr = make_sexpr(SEXPR_KIND_INTEGER);
  expr->integer = value;
  return expr;
}

SExpr *make_sexpr_real(sexpr_real_t value) {
  SExpr *expr = make_sexpr(SEXPR_KIND_REAL);
  expr->real = value;
  return expr;
}

SExpr *make_sexpr_string(String_View value) {
  SExpr *expr = make_sexpr(SEXPR_KIND_STRING);
  expr->string = strndup(value.data, value.count);
  return expr;
}

SExpr *make_sexpr_string_n(const sexpr_string_t value, size_t size) {
  return make_sexpr_string((String_View){.data = value, .count = size});
}

SExpr *make_sexpr_string_cstr(const sexpr_string_t value) {
  return make_sexpr_string(sv_from_cstr(value));
}

SExpr *make_sexpr_quote(SExpr *quote) {
  SExpr *expr = make_sexpr(SEXPR_KIND_QUOTE);
  expr->quote = quote ? rc_acquire(quote) : NULL;
  return expr;
}

SExpr *make_sexpr_list_opt(size_t count, SExpr **items) {
  if (!count) UNREACHABLE("empty list array must contain [NULL]");
  sexpr_constants();
  SExpr *expr = items[count - 1] == NULL ? SEXPR_NIL : items[count - 1];
  for (ssize_t index = (ssize_t)count - 2; index >= 0; index -= 1) {
    expr = make_sexpr_pair(items[index], expr);
  }
  return expr;
}

void fprint_sexpr(FILE *f, SExpr *expr) {
  switch (expr->kind) {
    case SEXPR_KIND_SYMBOL: {
      if (expr->symbol.guarded) {
        fprintf(f, "|%s|", expr->symbol.name);
      } else {
        fprintf(f, "%s", expr->symbol.name);
      }
    } break;
    case SEXPR_KIND_NIL: {
      fprintf(f, "()");
    } break;
    case SEXPR_KIND_PAIR: {
      fprintf(f, "(");
      SExpr *it = expr;
      SExpr *last_left = it->pair.left;
      if (last_left->kind == SEXPR_KIND_NIL) fprintf(f, "nil");
      else fprint_sexpr(f, last_left);
      it = it->pair.right;
      while (it->kind == SEXPR_KIND_PAIR) {
        fprintf(f, " ");
        last_left = it->pair.left;
        fprint_sexpr(f, last_left);
        it = it->pair.right;
      }
      if (it->kind != SEXPR_KIND_NIL) {
        fprintf(f, " . ");
        fprint_sexpr(f, it);
      } else if (last_left->kind == SEXPR_KIND_NIL) {
        fprintf(f, " . nil");
      }
      fprintf(f, ")");
    } break;
    case SEXPR_KIND_QUOTE: {
      fprintf(f, "'");
      fprint_sexpr(f, expr->quote);
    } break;
    case SEXPR_KIND_BOOL: {
      if (expr->boolean) fprintf(f, "true");
      else fprintf(f, "false");
    } break;
    case SEXPR_KIND_INTEGER: {
      String_View sv = stringify_int(expr->integer);
      fprintf(f, SV_Fmt, SV_Arg(sv));
    } break;
    case SEXPR_KIND_REAL: {
      String_View sv = stringify_real(expr->real);
      fprintf(f, SV_Fmt, SV_Arg(sv));
    } break;
    case SEXPR_KIND_STRING: {
      fprintf(f, "\"%s\"", expr->string);
    } break;
  }
}

void fdump__sexpr(FILE *f, SExpr *expr, size_t current_indent, size_t next_indent) {
  if (current_indent) fprintf(f, "%*c", (int)current_indent, ' ');
  switch (expr->kind) {
    case SEXPR_KIND_NIL: {
      fprintf(f, "NIL\n");
    } break;
    case SEXPR_KIND_SYMBOL: {
      if (expr->symbol.guarded) {
        fprintf(f, "SYMBOL: |%s|\n", expr->symbol.name);
      } else {
        fprintf(f, "SYMBOL: %s\n", expr->symbol.name);
      }
    } break;
    case SEXPR_KIND_PAIR: {
      fprintf(f, "PAIR:\n");
      fdump__sexpr(f, expr->pair.left, next_indent, next_indent + 2);
      fdump__sexpr(f, expr->pair.right, next_indent, next_indent + 2);
    } break;
    case SEXPR_KIND_QUOTE: {
      fprintf(f, "QUOTE: ");
      fdump__sexpr(f, expr->quote, 0, next_indent);
    } break;
    case SEXPR_KIND_BOOL: {
      fprintf(f, "BOOL: %s\n", expr->boolean ? "true" : "false");
    } break;
    case SEXPR_KIND_INTEGER: {
      fprintf(f, "INTEGER: %lld\n", expr->integer);
    } break;
    case SEXPR_KIND_REAL: {
      fprintf(f, "REAL: %Lf\n", expr->real);
    } break;
    case SEXPR_KIND_STRING: {
      fprintf(f, "STRING: \"%s\"\n", expr->string);
    } break;
  }
}

#endif // SEXPR_AST_IMPL
