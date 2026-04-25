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
#define make_expr_list(...) make_expr_list_opt((sizeof((Expr *[]){__VA_ARGS__}) / sizeof(Expr *)), ((Expr *[]){__VA_ARGS__}))
Exprs exprs_from_list(Expr *item);
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

void print_expr(Expr *expr);
void dump_expr(Expr *expr);

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
Exprs exprs_from_list(Expr *item) {
  Exprs exprs = {0};
  Expr *it = item;
  size_t size = 0;
  while (it->kind == EXPR_KIND_PAIR) {
    it = it->pair.right;
    size += 1;
  }
  da_reserve(&exprs, size + 1);
  it = item;
  while (it->kind == EXPR_KIND_PAIR) {
    da_append(&exprs, it->pair.left);
    it = it->pair.right;
  }
  if (it->kind != EXPR_KIND_NIL) {
    da_append(&exprs, it);
  } else {
    da_append(&exprs, NULL);
  }
  return exprs;
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

void _print_expr(Expr *expr, size_t level) {
  switch (expr->kind) {
  case EXPR_KIND_SYMBOL: {
    if (expr->symbol.guarded) {
      printf("|%s|", expr->symbol.name);
    } else {
      printf("%s", expr->symbol.name);
    }
  } break;
  case EXPR_KIND_NIL: {
    printf("()");
  } break;
  case EXPR_KIND_PAIR: {
    printf("(");
    Expr *it = expr;
    _print_expr(it->pair.left, level + 1);
    it = it->pair.right;
    if (it == NULL) UNREACHABLE("empty pair element");
    if (it->kind == EXPR_KIND_PAIR) {
      while (true) {
        printf(" ");
        _print_expr(it->pair.left, level + 1);
        it = it->pair.right;
        if (it == NULL) UNREACHABLE("empty pair element");
        if (it->kind != EXPR_KIND_PAIR) break;
      }
    }
    if (it->kind != EXPR_KIND_NIL) {
      printf(" . ");
      _print_expr(it->pair.right, level + 1);
    }
    printf(")");
  } break;
  case EXPR_KIND_QUOTE: {
    printf("'");
    _print_expr(expr->quote, level);
  } break;
  case EXPR_KIND_BOOL: {
    if (expr->boolean) printf("true");
    else printf("false");
  } break;
  case EXPR_KIND_INTEGER: {
    printf("%ld", expr->integer);
  } break;
  case EXPR_KIND_REAL: {
    printf("%f", expr->real);
  } break;
  case EXPR_KIND_STRING: {
    printf("\"%s\"", expr->string);
  } break;
  }
}
void print_expr(Expr *expr) {
  _print_expr(expr, 0);
  printf("\n");
}

void dump_expr_opt(Expr *expr, size_t current_indent, size_t next_indent) {
  if (current_indent) printf("%*c", (int)current_indent * 2, ' ');
  switch (expr->kind) {
  case EXPR_KIND_NIL: {
    printf("NIL\n");
  } break;
  case EXPR_KIND_SYMBOL: {
    if (expr->symbol.guarded) {
      printf("SYMBOL: |%s|\n", expr->symbol.name);
    } else {
      printf("SYMBOL: %s\n", expr->symbol.name);
    }
  } break;
  case EXPR_KIND_PAIR: {
    printf("PAIR:\n");
    dump_expr_opt(expr->pair.left, next_indent, next_indent + 1);
    dump_expr_opt(expr->pair.right, next_indent, next_indent + 1);
  } break;
  case EXPR_KIND_QUOTE: {
    printf("QUOTE: ");
    dump_expr_opt(expr->quote, 0, next_indent);
  } break;
  case EXPR_KIND_BOOL: {
    printf("BOOL: %s\n", expr->boolean ? "true" : "false");
  } break;
  case EXPR_KIND_INTEGER: {
    printf("INTEGER: %ld\n", expr->integer);
  } break;
  case EXPR_KIND_REAL: {
    printf("REAL: %f\n", expr->real);
  } break;
  case EXPR_KIND_STRING: {
    printf("STRING: \"%s\"\n", expr->string);
  } break;
  }
}
void dump_expr(Expr *expr) {
  dump_expr_opt(expr, 0, 1);
}

#endif // EXPR_AST_IMPLEMENTATION
