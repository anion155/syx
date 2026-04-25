#ifndef EXPR_AST_H
#define EXPR_AST_H
#include <stdlib.h>
#include <ht.h>

typedef enum {
  EXPR_KIND_NIL,
  EXPR_KIND_SYMBOL,
  EXPR_KIND_PAIR,
  EXPR_KIND_QUOTE,
  EXPR_KIND_BOOL,
  EXPR_KIND_INTEGER,
  EXPR_KIND_REAL,
  EXPR_KIND_STRING,
} Expr_Kind;
typedef bool expr_bool_t;
typedef long int expr_int_t;
typedef double expr_real_t;
typedef const char * expr_string_t;

typedef struct Expr_Symbol {
  const char *name;
  bool guarded;
} Expr_Symbol;

typedef struct Expr Expr;
typedef struct Expr_Pair {
  Expr *left;
  Expr *right;
} Expr_Pair;
struct Expr {
  Expr_Kind kind;
  union {
    Expr_Symbol symbol;
    Expr_Pair pair;
    Expr *quote;
    expr_bool_t boolean;
    expr_int_t integer;
    expr_real_t real;
    expr_string_t string;
  };
};

typedef struct Exprs {
  Expr **items;
  size_t count;
  size_t capacity;
} Exprs;

Expr EXPR_NIL;
Expr EXPR_TRUE;
Expr EXPR_FALSE;
typedef Ht(const char *, Expr *, Expr_Constants) Expr_Constants;
Expr_Constants EXPR_CONSTANTS;

Expr *make_expr(Expr_Kind kind);
Expr *make_expr_symbol(const char *symbol, bool guarded);
Expr *make_expr_list_opt(size_t count, Expr **items);
#define make_expr_list(...)                           \
  make_expr_list_opt(                                 \
    sizeof((Expr *[]){__VA_ARGS__}) / sizeof(Expr *), \
    (Expr *[]){__VA_ARGS__}                           \
  )
#define UNREACHABLE_EXPR(message, as) \
  (fprintf(stderr, "%s:%d: UNREACHABLE: %s\n", __FILE__, __LINE__, (message)), abort(), as)
#define expr_list_for_each(first, value) for (      \
  Expr *value##_it = (first),                       \
    *value = value##_it->kind == EXPR_KIND_PAIR     \
      ? value##_it->pair.left                       \
      : UNREACHABLE_EXPR("malformed list", NULL);   \
  value##_it->kind != EXPR_KIND_NIL;                \
  value##_it = value##_it->kind == EXPR_KIND_PAIR   \
    ? value##_it->pair.right                        \
    : &EXPR_NIL,                                    \
  value = value##_it->kind == EXPR_KIND_PAIR        \
    ? value##_it->pair.left                         \
    : value##_it->kind == EXPR_KIND_NIL             \
      ? NULL                                        \
      : UNREACHABLE_EXPR("malformed list", NULL)    \
)
Expr *make_expr_quote(Expr *quote);
Expr *make_expr_bool(expr_bool_t value);
Expr *make_expr_integer(expr_int_t value);
Expr *make_expr_real(expr_real_t value);
Expr *make_expr_string(expr_string_t value);
#define make_expr_value(value)               \
  _Generic(value,                            \
    expr_bool_t:    make_expr_bool(value),   \
    expr_int_t:    make_expr_integer(value), \
    expr_real_t:   make_expr_real(value),    \
    expr_string_t: make_expr_string(value)   \
  )

void fprint_expr(FILE *f, Expr *expr);
#define print_expr(expr) fprint_expr(stdout, (expr))
void fdump_expr_opt(FILE *f, Expr *expr, size_t current_indent, size_t next_indent);
#define dump_expr_opt(expr, current_indent, next_indent) fdump_expr_opt(stdout, (expr), (current_indent), (next_indent))
#define fdump_expr(f, expr) fdump_expr_opt((f), (expr), 0, 1)
#define dump_expr(expr) dump_expr_opt((expr), 0, 1)

#endif // EXPR_AST_H

#ifdef EXPR_AST_IMPLEMENTATION
#undef EXPR_AST_IMPLEMENTATION

#include <stdio.h>
#include <nob.h>
#include "expr_parser.h"

Expr EXPR_NIL = {.kind = EXPR_KIND_NIL};
Expr EXPR_TRUE = {.kind = EXPR_KIND_BOOL, .boolean = true};
Expr EXPR_FALSE = {.kind = EXPR_KIND_BOOL, .boolean = false};
Expr_Constants EXPR_CONSTANTS = {.hasheq = ht_cstr_hasheq};

Expr_Constants *expr_constants() {
  if (EXPR_CONSTANTS.count) return &EXPR_CONSTANTS;
  *ht_put(&EXPR_CONSTANTS, "nil") = &EXPR_NIL;
  *ht_put(&EXPR_CONSTANTS, "true") = &EXPR_TRUE;
  *ht_put(&EXPR_CONSTANTS, "#t") = &EXPR_TRUE;
  *ht_put(&EXPR_CONSTANTS, "false") = &EXPR_FALSE;
  *ht_put(&EXPR_CONSTANTS, "#f") = &EXPR_FALSE;
  return &EXPR_CONSTANTS;
}

Expr *make_expr(Expr_Kind kind) {
  Expr *expr = (Expr *)malloc(sizeof(Expr));
  expr->kind = kind;
  return expr;
}
Expr *make_expr_symbol(const char *symbol, bool guarded) {
  Expr **constant = ht_find(expr_constants(), symbol);
  if (constant != NULL) {
    // free(symbol);
    return *constant;
  }

  Expr *expr = make_expr(EXPR_KIND_SYMBOL);
  expr->symbol.name = symbol;
  expr->symbol.guarded = guarded;
  return expr;
}
Expr *make_expr_pair(Expr *left, Expr *right) {
  Expr *expr = make_expr(EXPR_KIND_PAIR);
  expr->pair.left = left;
  expr->pair.right = right;
  return expr;
}
Expr *make_expr_list_opt(size_t count, Expr **items) {
  if (!count) UNREACHABLE("empty list array must contain [NULL]");
  Expr *expr = items[count - 1] == NULL ? &EXPR_NIL : items[count - 1];
  for (ssize_t index = (ssize_t)count - 2; index >= 0; index -= 1) {
    expr = make_expr_pair(items[index], expr);
  }
  return expr;
}
Expr *make_expr_quote(Expr *quote) {
  Expr *expr = make_expr(EXPR_KIND_QUOTE);
  expr->quote = quote;
  return expr;
}
Expr *make_expr_bool(expr_bool_t value) {
  return value ? &EXPR_TRUE : &EXPR_FALSE;
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
Expr *make_expr_string(expr_string_t value) {
  Expr *expr = make_expr(EXPR_KIND_STRING);
  expr->string = value;
  return expr;
}

void fprint_expr(FILE *f, Expr *expr) {
  switch (expr->kind) {
    case EXPR_KIND_SYMBOL: {
      if (expr->symbol.guarded) {
        fprintf(f, "|%s|", expr->symbol.name);
      } else {
        fprintf(f, "%s", expr->symbol.name);
      }
    } break;
    case EXPR_KIND_NIL: {
      fprintf(f, "()");
    } break;
    case EXPR_KIND_PAIR: {
      fprintf(f, "(");
      Expr *it = expr;
      fprint_expr(f, it->pair.left);
      it = it->pair.right;
      if (it == NULL) UNREACHABLE("empty pair element");
      if (it->kind == EXPR_KIND_PAIR) {
        while (true) {
          fprintf(f, " ");
          fprint_expr(f, it->pair.left);
          it = it->pair.right;
          if (it == NULL) UNREACHABLE("empty pair element");
          if (it->kind != EXPR_KIND_PAIR) break;
        }
      }
      if (it->kind != EXPR_KIND_NIL) {
        fprintf(f, " . ");
        fprint_expr(f, it->pair.right);
      }
      fprintf(f, ")");
    } break;
    case EXPR_KIND_QUOTE: {
      fprintf(f, "'");
      fprint_expr(f, expr->quote);
    } break;
    case EXPR_KIND_BOOL: {
      if (expr->boolean) fprintf(f, "true");
      else fprintf(f, "false");
    } break;
    case EXPR_KIND_INTEGER: {
      fprintf(f, "%ld", expr->integer);
    } break;
    case EXPR_KIND_REAL: {
      fprintf(f, "%f", expr->real);
    } break;
    case EXPR_KIND_STRING: {
      fprintf(f, "\"%s\"", expr->string);
    } break;
  }
}

void fdump_expr_opt(FILE *f, Expr *expr, size_t current_indent, size_t next_indent) {
  if (current_indent) fprintf(f, "%*c", (int)current_indent, ' ');
  switch (expr->kind) {
    case EXPR_KIND_NIL: {
      fprintf(f, "NIL\n");
    } break;
    case EXPR_KIND_SYMBOL: {
      if (expr->symbol.guarded) {
        fprintf(f, "SYMBOL: |%s|\n", expr->symbol.name);
      } else {
        fprintf(f, "SYMBOL: %s\n", expr->symbol.name);
      }
    } break;
    case EXPR_KIND_PAIR: {
      fprintf(f, "PAIR:\n");
      fdump_expr_opt(f, expr->pair.left, next_indent, next_indent + 2);
      fdump_expr_opt(f, expr->pair.right, next_indent, next_indent + 2);
    } break;
    case EXPR_KIND_QUOTE: {
      fprintf(f, "QUOTE: ");
      fdump_expr_opt(f, expr->quote, 0, next_indent);
    } break;
    case EXPR_KIND_BOOL: {
      fprintf(f, "BOOL: %s\n", expr->boolean ? "true" : "false");
    } break;
    case EXPR_KIND_INTEGER: {
      fprintf(f, "INTEGER: %ld\n", expr->integer);
    } break;
    case EXPR_KIND_REAL: {
      fprintf(f, "REAL: %f\n", expr->real);
    } break;
    case EXPR_KIND_STRING: {
      fprintf(f, "STRING: \"%s\"\n", expr->string);
    } break;
  }
}

#endif // EXPR_AST_IMPLEMENTATION
