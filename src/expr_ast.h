#ifndef EXPR_AST_H
#define EXPR_AST_H
#include <stdlib.h>

typedef enum {
  EXPR_KIND_SYMBOL,
  EXPR_KIND_LIST,
  EXPR_KIND_BOOL,
  EXPR_KIND_INTEGER,
  EXPR_KIND_REAL,
  EXPR_KIND_STRING,
} Expr_Kind;
typedef bool expr_bool_t;
typedef long int expr_int_t;
typedef double expr_real_t;
typedef char * expr_string_t;

struct ExprSymbol {
  const char *name;
  bool guarded;
};

typedef struct Expr Expr;
struct ExprList {
  Expr **items;
  size_t count;
};
struct Expr {
  Expr_Kind kind;
  union {
    struct ExprSymbol symbol;
    struct ExprList list;
    expr_bool_t boolean;
    expr_int_t integer;
    expr_real_t real;
    expr_string_t string;
  };
};

Expr *make_expr(Expr_Kind kind);
Expr *make_expr_symbol(const char *symbol, bool guarded);
Expr *make_expr_list_opt(size_t count, Expr **items);
#define make_expr_list(...) make_expr_list_opt((sizeof((Expr *[]){__VA_ARGS__}) / sizeof(Expr *)), ((Expr *[]){__VA_ARGS__}))
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

#ifdef EXPR_AST_IMPLEMENTATION
#undef EXPR_AST_IMPLEMENTATION
#include <stdio.h>
#include <nob.h>

Expr *make_expr(Expr_Kind kind) {
  Expr *expr = (Expr *)malloc(sizeof(Expr));
  expr->kind = kind;
  return expr;
}
Expr *make_expr_symbol(const char *symbol, bool guarded) {
  Expr *expr = make_expr(EXPR_KIND_SYMBOL);
  expr->symbol.name = symbol;
  expr->symbol.guarded = guarded;
  return expr;
}
Expr *make_expr_list_opt(size_t count, Expr **items) {
  Expr *expr = make_expr(EXPR_KIND_LIST);
  expr->list.count = count;
  expr->list.items = items;
  return expr;
}
Expr *make_expr_bool(expr_bool_t value) {
  Expr *expr = make_expr(EXPR_KIND_BOOL);
  expr->integer = value;
  return expr;
}
Expr *make_expr_integer(expr_int_t value) {
  Expr *expr = make_expr(EXPR_KIND_INTEGER);
  expr->integer = value;
  return expr;
}
Expr *make_expr_real(expr_real_t value) {
  Expr *expr = make_expr(EXPR_KIND_REAL);
  expr->integer = value;
  return expr;
}
Expr *make_expr_string(expr_string_t value) {
  Expr *expr = make_expr(EXPR_KIND_STRING);
  expr->string = value;
  return expr;
}

void _print_expr(Expr *expr, int level) {
  switch (expr->kind) {
  case EXPR_KIND_SYMBOL: {
    if (expr->symbol.guarded) {
      printf("|%s|", expr->symbol.name);
    } else {
      printf("%s", expr->symbol.name);
    }
  } break;
  case EXPR_KIND_LIST: {
    printf("(");
    size_t index = 0;
    size_t count = expr->list.count - 1;
    if (expr->list.count > 1) _print_expr(expr->list.items[index++], level + 1);
    while (index < count) {
      printf(" ");
      _print_expr(expr->list.items[index], level + 1);
      index += 1;
    }
    if (expr->list.items[index]) {
      printf(" . ");
      _print_expr(expr->list.items[index], level + 1);
    }
    printf(")");
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
  default: UNREACHABLE("unknown expr kind");
  }
}
void print_expr(Expr *expr) {
  _print_expr(expr, 0);
  printf("\n");
}

void dump_expr(Expr *expr, int level) {
  printf("%*c", level * 2, ' ');
  switch (expr->kind) {
  case EXPR_KIND_SYMBOL: {
    if (expr->symbol.guarded) {
      printf("SYMBOL: |%s|\n", expr->symbol.name);
    } else {
      printf("SYMBOL: %s\n", expr->symbol.name);
    }
  } break;
  case EXPR_KIND_LIST: {
    printf("LIST:\n");
    for (size_t index = 0; index < expr->list.count; index += 1) {
      Expr *it = expr->list.items[index];
      if (it) dump_expr(it, level + 1);
      else printf("%*cNIL\n", (level + 1) * 2, ' ');
    }
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
  default: UNREACHABLE("unknown expr kind");
  }
}

#endif // EXPR_AST_IMPLEMENTATION
#endif // EXPR_AST_H
