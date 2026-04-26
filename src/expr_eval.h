#ifndef EXPR_EVAL_H
#define EXPR_EVAL_H

#include <nob.h>
#include <ht.h>
#include "expr_ast.h"

typedef struct Expr_Env Expr_Env;
typedef struct Expr_Value Expr_Value;

typedef struct Expr_Arguments {
  Expr_Value *items;
  size_t count;
  size_t capacity;
} Expr_Arguments;
Expr_Arguments expr_arguments(Expr *expr);

typedef Expr_Value (*Expr_Evaluator)(Expr_Env *env, Expr_Arguments arguments);

typedef struct Expr_Closure {
  Expr_Env *env;
  Expr *arguments;
  Expr *body;
} Expr_Closure;
void fprint_closure_opt(FILE *f, Expr_Closure *closure, size_t indent);
#define print_closure_opt(closure, indent) fprint_closure_opt(stdout, (closure), (indent))
#define fprint_closure(f, closure) fprint_closure_opt(f, (closure), 0)
#define print_closure(closure) print_closure_opt((closure), 0)

typedef enum {
  EXPR_VALUE_KIND_EXPR,
  EXPR_VALUE_KIND_SPECIAL_FORM,
  EXPR_VALUE_KIND_BUILTIN,
  EXPR_VALUE_KIND_CLOSURE,
} Expr_Value_Kind;
struct Expr_Value {
  Expr_Value_Kind kind;
  union {
    Expr *expr;
    Expr_Evaluator special;
    Expr_Evaluator builtin;
    Expr_Closure closure;
  };
};
void fprint_expr_value_opt(FILE * f, Expr_Value value, size_t indent);
#define print_expr_value_opt(value, indent) fprint_expr_value_opt(stdout, (value), (indent))
#define fprint_expr_value(f, value) fprint_expr_value_opt((f), (value), 0)
#define print_expr_value(value) print_expr_value_opt((value), 0)

typedef Ht(const char *, Expr_Value, Expr_Env_Symbols) Expr_Env_Symbols;
struct Expr_Env {
  Expr_Env *parent;
  Expr_Env_Symbols symbols;
};

void expr_env_put_symbol(Expr_Env *env, const char *name, Expr_Value value);
void expr_global_env_init(Expr_Env *env);
Expr_Env make_expr_env(Expr_Env *parent);
void free_expr_env(Expr_Env *env);
Expr_Value *expr_env_lookup(Expr_Env *env, const char *name);

void expr_eval_arguments(Expr_Env *env, Exprs *arguments);
Expr_Value expr_eval_special_form(Expr_Env *env, Expr_Evaluator evaluator, Expr *arguments);
Expr_Value expr_eval_builtin(Expr_Env *env, Expr_Evaluator evaluator, Expr *arguments);
Expr_Value expr_eval_closure(Expr_Env *env, Expr_Closure *closure, Expr *arguments);
Expr_Value expr_eval(Expr_Env *env, Expr_Value input);

#endif // EXPR_EVAL_H

#if defined(EXPR_EVAL_IMPLEMENTATION) && !defined(EXPR_EVAL_IMPLEMENTATION_C)
#define EXPR_EVAL_IMPLEMENTATION_C

#include "expr_utils.h"
#define EXPR_EVAL_SPECIAL_FORMS_IMPLEMENTATION
#include "expr_eval_special_forms.h"
#define EXPR_EVAL_ARITHMETIC_IMPLEMENTATION
#include "expr_eval_arithmetic.h"
#define EXPR_EVAL_COMPARISON_IMPLEMENTATION
#include "expr_eval_comparison.h"
#define EXPR_EVAL_LIST_IMPLEMENTATION
#include "expr_eval_list.h"
#define EXPR_EVAL_TYPE_PREDICATES_IMPLEMENTATION
#include "expr_eval_type_predicates.h"
#define EXPR_EVAL_STRING_IMPLEMENTATION
#include "expr_eval_string.h"
#define EXPR_EVAL_TYPE_CONVERSION_IMPLEMENTATION
#include "expr_eval_type_conversion.h"
#define EXPR_EVAL_EQUALITY_IMPLEMENTATION
#include "expr_eval_equality.h"
#define EXPR_EVAL_IO_IMPLEMENTATION
#include "expr_eval_io.h"

void fprint_closure_opt(FILE *f, Expr_Closure *closure, size_t indent) {
  if (indent) fprintf(f, "%*c", (int)indent, ' ');
  fprintf(f, "(lambda ");
  fprint_expr(f, closure->arguments);
  fprintf(f, "\n");
  fprintf(f, "%*c", (int)indent + 2 + 8, ' ');
  fprint_expr(f, closure->body);
  fprintf(f, ")");
}

void fprint_expr_value_opt(FILE *f, Expr_Value value, size_t indent) {
  switch (value.kind) {
    case EXPR_VALUE_KIND_EXPR: fprint_expr(f, value.expr); break;
    case EXPR_VALUE_KIND_SPECIAL_FORM: UNREACHABLE("can not print special form");
    case EXPR_VALUE_KIND_BUILTIN: UNREACHABLE("can not print builtin");
    case EXPR_VALUE_KIND_CLOSURE: fprint_closure_opt(f, &value.closure, indent); break;
  }
}

void expr_env_put_symbol(Expr_Env *env, const char *name, Expr_Value value) {
  Expr_Value *item = expr_env_lookup(env, name);
  if (item != NULL) {
    switch (item->kind) {
      case EXPR_VALUE_KIND_EXPR: break;
      case EXPR_VALUE_KIND_SPECIAL_FORM: UNREACHABLE("trying to redefine special form");
      case EXPR_VALUE_KIND_BUILTIN: UNREACHABLE("trying to redefine builtin");
      case EXPR_VALUE_KIND_CLOSURE: break;
    }
  } else {
    item = ht_put(&env->symbols, name);
  }
  *item = value;
}
Expr_Env make_expr_env(Expr_Env *parent) {
  return (Expr_Env){.parent = parent, .symbols = {.hasheq = ht_cstr_hasheq}};
}
void free_expr_env(Expr_Env *env) {
  ht_free(&env->symbols);
}
Expr_Value *expr_env_lookup(Expr_Env *env, const char *name) {
  Expr_Value *item = NULL;
  while (env != NULL && item == NULL) {
    item = ht_find(&env->symbols, name);
    env = env->parent;
  }
  return item;
}

Expr_Arguments expr_arguments(Expr *expr) {
  Expr_Arguments arguments = {0};
  expr_list_for_each(expr, arg_expr) {
    Expr_Value arg = {.kind = EXPR_VALUE_KIND_EXPR, .expr = arg_expr};
    da_append(&arguments, arg);
  }
  return arguments;
}

Expr_Value expr_eval_special_form(Expr_Env *env, Expr_Evaluator evaluator, Expr *arguments_expr) {
  Expr_Arguments arguments = expr_arguments(arguments_expr);
  Expr_Value result = evaluator(env, arguments);
  da_free(arguments);
  return result;
}
Expr_Value expr_eval_builtin(Expr_Env *env, Expr_Evaluator evaluator, Expr *arguments_expr) {
  Expr_Arguments arguments = expr_arguments(arguments_expr);
  da_foreach(Expr_Value, arg, &arguments) {
    *arg = expr_eval(env, (Expr_Value){.kind = EXPR_VALUE_KIND_EXPR, .expr = arg->expr});
  }
  Expr_Value result = evaluator(env, arguments);
  da_free(arguments);
  return result;
}
Expr_Value expr_eval_closure(Expr_Env *env, Expr_Closure *closure, Expr *arguments) {
  UNUSED(env);
  Expr_Env call_env = make_expr_env(closure->env);
  Expr *it = arguments;
  expr_list_for_each(closure->arguments, name_expr) {
    const char *name = name_expr->symbol.name;
    Expr_Value value = {.kind = EXPR_VALUE_KIND_EXPR};
    if (name_expr_it->kind == EXPR_KIND_NIL) {
      value.expr = it;
      expr_env_put_symbol(&call_env, name, value);
      it = &EXPR_NIL;
      break;
    }
    if (it->kind == EXPR_KIND_NIL) UNREACHABLE("Too few arguments");
    value.expr = it->pair.left;
    expr_env_put_symbol(&call_env, name, value);
    it = it->pair.right;
  }
  if (it->kind != EXPR_KIND_NIL) UNREACHABLE("Too many arguments");
  Expr_Value result = expr_eval(&call_env, (Expr_Value){.kind = EXPR_VALUE_KIND_EXPR, .expr = closure->body});
  free_expr_env(&call_env);
  return result;
}
Expr_Value expr_eval(Expr_Env *env, Expr_Value input) {
  if (input.kind != EXPR_VALUE_KIND_EXPR) return input;
  if (input.expr->kind == EXPR_KIND_QUOTE) return (Expr_Value){.kind = EXPR_VALUE_KIND_EXPR, .expr = input.expr->quote};
  if (input.expr->kind == EXPR_KIND_SYMBOL) {
    Expr_Value *item = expr_env_lookup(env, input.expr->symbol.name);
    if (item == NULL) UNREACHABLE("unbound symbol");
    return *item;
  }
  if (input.expr->kind != EXPR_KIND_PAIR) return input;
  if (!input.expr->pair.left) return input;
  Expr_Value head = expr_eval(env, (Expr_Value){.kind = EXPR_VALUE_KIND_EXPR, .expr = input.expr->pair.left});
  Expr *arguments = input.expr->pair.right;
  switch (head.kind) {
    case EXPR_VALUE_KIND_EXPR: UNREACHABLE("is not a procedure");
    case EXPR_VALUE_KIND_SPECIAL_FORM: return expr_eval_special_form(env, head.special, arguments);
    case EXPR_VALUE_KIND_BUILTIN: return expr_eval_builtin(env, head.builtin, arguments);
    case EXPR_VALUE_KIND_CLOSURE: return expr_eval_closure(env, &head.closure, arguments);
  }
}

void expr_global_env_init(Expr_Env *env) {
  env->symbols.hasheq = ht_cstr_hasheq;
  expr_env_init_special_forms(env);
  expr_env_init_arithmetic(env);
  expr_env_init_comparison(env);
  expr_env_init_equality(env);
  expr_env_init_list(env);
  expr_env_init_string(env);
  expr_env_init_type_predicates(env);
  expr_env_init_type_conversion(env);
  expr_env_init_io(env);
}

#endif // EXPR_EVAL_IMPLEMENTATION
