#ifndef EXPR_VALUE_H
#define EXPR_VALUE_H

#include <nob.h>
#include <ht.h>
#include <rc.h>
#include <magic.h>
#include "expr_ast.h"
#include "expr_print.h"

typedef struct Expr_Value Expr_Value;
typedef struct Expr_Env Expr_Env;

typedef struct Expr_Arguments {
  Expr_Value *items;
  size_t count;
  size_t capacity;
} Expr_Arguments;
Expr_Arguments expr_arguments(Expr *expr);
typedef Expr_Value (*Expr_Evaluator)(Expr_Env *env, Expr_Arguments arguments);

typedef struct Expr_Special_Form {
  char *name;
  Expr_Evaluator eval;
} Expr_Special_Form;

typedef struct Expr_Builtin {
  char *name;
  Expr_Evaluator eval;
} Expr_Builtin;

typedef struct Expr_Closure {
  char *name;
  Expr_Env *env;
  Expr_Value *arguments;
  Expr_Value *body;
} Expr_Closure;
void fprint__closure(FILE *f, Expr_Closure *closure, size_t indent);
#define fprint_closure(f, closure, ...) fprint_closure((f), (closure), WITH_DEFAULT(0, __VA_ARGS))
#define print_closure(closure) fprint_closure(stdout, (closure), WITH_DEFAULT(0, __VA_ARGS))

typedef enum : unsigned int {
  EXPR_VALUE_KIND_NIL = EXPR_KIND_NIL,
  EXPR_VALUE_KIND_SYMBOL = EXPR_KIND_SYMBOL,
  EXPR_VALUE_KIND_PAIR = EXPR_KIND_PAIR,
  EXPR_VALUE_KIND_BOOL = EXPR_KIND_BOOL,
  EXPR_VALUE_KIND_INTEGER = EXPR_KIND_INTEGER,
  EXPR_VALUE_KIND_REAL = EXPR_KIND_REAL,
  EXPR_VALUE_KIND_STRING = EXPR_KIND_STRING,
  EXPR_VALUE_KIND_QUOTE = EXPR_KIND_QUOTE,
  EXPR_VALUE_KIND_SPECIAL_FORM,
  EXPR_VALUE_KIND_BUILTIN,
  EXPR_VALUE_KIND_CLOSURE,
} Expr_Value_Kind;

struct Expr_Value {
  Expr_Value_Kind kind;
  union {
    Expr_Symbol symbol;
    struct Expr_Value_Pair {
      Expr_Value *left;
      Expr_Value *right;
    } pair;
    expr_bool_t boolean;
    expr_int_t integer;
    expr_real_t real;
    expr_string_t string;
    Expr_Value *quote;
    Expr_Special_Form special;
    Expr_Builtin builtin;
    Expr_Closure closure;
  };
};
void expr_value_destructor(void *data);
Expr_Value *expr_to_value(Expr *expr);

void fprint__expr_value(FILE *f, Expr_Value *value, size_t indent);
#define fprint_expr_value(f, value, ...) fprint__expr_value((f), (value), WITH_DEFAULT(0, __VA_ARGS__))
#define print_expr_value(value, ...) fprint__expr_value(stdout, (value), WITH_DEFAULT(0, __VA_ARGS__))

typedef Ht(const char *, Expr_Value *, Expr_Env_Symbols) Expr_Env_Symbols;
typedef struct Expr_Env Expr_Env;
struct Expr_Env {
  Expr_Env *parent;
  Expr_Env_Symbols symbols;
};

#endif // EXPR_VALUE_H

#if defined(EXPR_VALUE_IMPL) && !defined(EXPR_VALUE_IMPL_C)
#define EXPR_VALUE_IMPL_C

#define EXPR_PRINT_IMPL
#include "expr_print.h"

void expr_value_destructor(void *data) {
  Expr_Value *value = data;
  switch (value->kind) {
    case EXPR_VALUE_KIND_NIL: break;
    case EXPR_VALUE_KIND_SYMBOL: rc_release(value->symbol.name); break;
    case EXPR_VALUE_KIND_PAIR: rc_release(value->pair.left); rc_release(value->pair.right); break;
    case EXPR_VALUE_KIND_BOOL: break;
    case EXPR_VALUE_KIND_INTEGER: break;
    case EXPR_VALUE_KIND_REAL: break;
    case EXPR_VALUE_KIND_STRING: rc_release(value->string); break;
    case EXPR_VALUE_KIND_QUOTE: rc_release(value->quote); break;
    case EXPR_VALUE_KIND_SPECIAL_FORM: break;
    case EXPR_VALUE_KIND_BUILTIN: break;
    case EXPR_VALUE_KIND_CLOSURE: {
      rc_release(value->closure.name);
      rc_release(value->closure.env);
      rc_release(value->closure.arguments);
      rc_release(value->closure.body);
    } break;
  }
}

Expr_Value *expr_to_value(Expr *expr) {
  Expr_Value *value = rc_alloc(sizeof(Expr_Value), expr_value_destructor);
  memcpy(value, expr, sizeof(Expr));
  switch (expr->kind) {
    case EXPR_KIND_NIL: break;
    case EXPR_KIND_SYMBOL: value->symbol.name = rc_acquire(expr->symbol.name); break;
    case EXPR_KIND_PAIR: {
      value->pair.left = expr_to_value(expr->pair.left);
      value->pair.right = expr_to_value(expr->pair.right);
    } break;
    case EXPR_KIND_BOOL: break;
    case EXPR_KIND_INTEGER: break;
    case EXPR_KIND_REAL: break;
    case EXPR_KIND_STRING: value->string = rc_acquire(expr->string); break;
    case EXPR_KIND_QUOTE: value->quote = expr_to_value(expr->quote); break;
  }
  return value;
}

void fprint__expr_value(FILE *f, Expr_Value *value, size_t indent) {
  switch (value->kind) {
    case EXPR_VALUE_KIND_NIL:
    case EXPR_VALUE_KIND_SYMBOL:
    case EXPR_VALUE_KIND_PAIR:
    case EXPR_VALUE_KIND_BOOL:
    case EXPR_VALUE_KIND_INTEGER:
    case EXPR_VALUE_KIND_REAL:
    case EXPR_VALUE_KIND_STRING:
    case EXPR_VALUE_KIND_QUOTE: return fprint__expr_value(f, value, indent + 2);
    case EXPR_VALUE_KIND_SPECIAL_FORM: fprintf(f, "?<%s>", value->special.name); break;
    case EXPR_VALUE_KIND_BUILTIN: fprintf(f, "!<%s>", value->builtin.name); break;
    case EXPR_VALUE_KIND_CLOSURE: fprint__closure(f, &value->closure, indent + 2); break;
  }
}

void fprint__closure(FILE *f, Expr_Closure *closure, size_t indent) {
  if (indent) fprintf(f, "%*c", (int)indent, ' ');
  fprintf(f, "(#<%s>", closure->name);
  fprint_expr_value(f, closure->arguments);
  fprintf(f, "\n");
  fprintf(f, "%*c", (int)indent + 2 + 8, ' ');
  fprint_expr_value(f, closure->body);
  fprintf(f, ")");
}

#endif // EXPR_VALUE_IMPL
