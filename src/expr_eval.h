#ifndef EXPR_EVAL_H
#define EXPR_EVAL_H

#include <nob.h>
#include <ht.h>
#include "expr_ast.h"

typedef struct Expr_Env Expr_Env;
typedef Expr *(*Expr_Evaluator)(Expr_Env *env, Exprs arguments, Expr *arguments_expr);
typedef enum {
  EXPR_ENV_SYMBOL_KIND_VARIABLE,
  EXPR_ENV_SYMBOL_KIND_SPECIAL_FORM,
  EXPR_ENV_SYMBOL_KIND_BUILTIN,
  EXPR_ENV_SYMBOL_KIND_CLOSURE,
} Expr_Env_Symbol_Kind;
typedef struct Expr_Env_Symbol {
  Expr_Env_Symbol_Kind kind;
  union {
    void *variable;
    Expr_Evaluator special;
    Expr_Evaluator builtin;
    // Expr_Closure closure;
  };
} Expr_Env_Symbol;
typedef Ht(const char *, Expr_Env_Symbol, Expr_Env_Symbols) Expr_Env_Symbols;
struct Expr_Env {
  Expr_Env_Symbols symbols;
};

void init_expr_env(Expr_Env *env);


/** Special forms */
/**
 * quote
 * Returns argument unevaluated
 */
Expr *expr_evaluate_quote(Expr_Env *env, Exprs arguments, Expr *arguments_expr);
/**
 * if
 * Evaluates condition then evaluates only one branch
 */
Expr *expr_evaluate_if(Expr_Env *env, Exprs arguments, Expr *arguments_expr);
/**
 * lambda
 * Creates a closure and captures current environment
 */
Expr *expr_evaluate_lambda(Expr_Env *env, Exprs arguments, Expr *arguments_expr);
/**
 * define
 * Binds a name in the current environment
 */
Expr *expr_evaluate_define(Expr_Env *env, Exprs arguments, Expr *arguments_expr);
/**
 * set!
 * Mutates an existing binding
 */
Expr *expr_evaluate_set_excl(Expr_Env *env, Exprs arguments, Expr *arguments_expr);
/**
 * begin
 * Evaluates expressions in order and returns last
 */
Expr *expr_evaluate_begin(Expr_Env *env, Exprs arguments, Expr *arguments_expr);
/**
 * let
 * Binds names in a new environment with all RHS evaluated in current env
 */
Expr *expr_evaluate_let(Expr_Env *env, Exprs arguments, Expr *arguments_expr);
/**
 * let*
 * Like let but each binding sees previous ones
 */
Expr *expr_evaluate_let_star(Expr_Env *env, Exprs arguments, Expr *arguments_expr);
/**
 * letrec
 * Like let but bindings can refer to each other for mutual recursion
 */
Expr *expr_evaluate_letrec(Expr_Env *env, Exprs arguments, Expr *arguments_expr);
/**
 * and
 * Evaluates left to right and stops at first falsy and returns last
 */
Expr *expr_evaluate_and(Expr_Env *env, Exprs arguments, Expr *arguments_expr);
/**
 * or
 * Evaluates left to right and stops at first truthy and returns it
 */
Expr *expr_evaluate_or(Expr_Env *env, Exprs arguments, Expr *arguments_expr);
/**
 * cond
 * Chain of (test expr) clauses that evaluates first matching branch
 */
Expr *expr_evaluate_cond(Expr_Env *env, Exprs arguments, Expr *arguments_expr);
/**
 * when
 * If condition is true evaluates body else returns nil
 */
Expr *expr_evaluate_when(Expr_Env *env, Exprs arguments, Expr *arguments_expr);
/**
 * unless
 * If condition is false evaluates body else returns nil
 */
Expr *expr_evaluate_unless(Expr_Env *env, Exprs arguments, Expr *arguments_expr);
/**
 * do
 * Iterative loop with step expressions and exit condition
 */
Expr *expr_evaluate_do(Expr_Env *env, Exprs arguments, Expr *arguments_expr);
/**
 * delay
 * Wraps expression in a promise without evaluating it
 */
Expr *expr_evaluate_delay(Expr_Env *env, Exprs arguments, Expr *arguments_expr);
/**
 * force
 * Evaluates a delayed promise and caches result
 */
Expr *expr_evaluate_force(Expr_Env *env, Exprs arguments, Expr *arguments_expr);
/**
 * define-syntax
 * Defines a macro transformation rule
 */
Expr *expr_evaluate_define_syntax(Expr_Env *env, Exprs arguments, Expr *arguments_expr);
/**
 * let-syntax
 * Locally scoped macro definitions
 */
Expr *expr_evaluate_let_syntax(Expr_Env *env, Exprs arguments, Expr *arguments_expr);
/**
 * syntax-rules
 * Pattern-based macro expansion engine
 */
Expr *expr_evaluate_syntax_rules(Expr_Env *env, Exprs arguments, Expr *arguments_expr);


void expr_eval_arguments(Expr_Env *env, Exprs *arguments);
Expr *expr_eval(Expr_Env *env, Expr *input);

#endif // EXPR_EVAL_H

#ifdef EXPR_EVAL_IMPLEMENTATION
#undef EXPR_EVAL_IMPLEMENTATION

#include "expr_utils.h"

Expr_Env_Symbol *_expr_env_put_symbol(Expr_Env *env, const char *name, Expr_Env_Symbol_Kind kind) {
  Expr_Env_Symbol *item = ht_find(&env->symbols, name);
  if (item != NULL) {
    switch (item->kind) {
    case EXPR_ENV_SYMBOL_KIND_VARIABLE: break;
    case EXPR_ENV_SYMBOL_KIND_SPECIAL_FORM: UNREACHABLE("trying to redefine special form");
    case EXPR_ENV_SYMBOL_KIND_BUILTIN: UNREACHABLE("trying to redefine builtin");
    case EXPR_ENV_SYMBOL_KIND_CLOSURE: break;
    }
  } else {
    item = ht_put(&env->symbols, name);
  }
  item->kind = kind;
  return item;
}
Expr_Env_Symbol *_expr_env_put_symbol_variable(Expr_Env *env, const char *name, Expr *value) {
  Expr_Env_Symbol *item = _expr_env_put_symbol(env, name, EXPR_ENV_SYMBOL_KIND_VARIABLE);
  item->variable = value;
  return item;
}
Expr_Env_Symbol *_expr_env_put_symbol_special_form(Expr_Env *env, const char *name, Expr_Evaluator special) {
  Expr_Env_Symbol *item = _expr_env_put_symbol(env, name, EXPR_ENV_SYMBOL_KIND_SPECIAL_FORM);
  item->special = special;
  return item;
}
Expr_Env_Symbol *_expr_env_put_symbol_builtin(Expr_Env *env, const char *name, Expr_Evaluator builtin) {
  Expr_Env_Symbol *item = _expr_env_put_symbol(env, name, EXPR_ENV_SYMBOL_KIND_SPECIAL_FORM);
  item->builtin = builtin;
  return item;
}
void init_expr_env(Expr_Env *env) {
  env->symbols.hasheq = ht_cstr_hasheq;
  _expr_env_put_symbol_special_form(env, "quote", expr_evaluate_quote);
  _expr_env_put_symbol_special_form(env, "if", expr_evaluate_if);
  _expr_env_put_symbol_special_form(env, "lambda", expr_evaluate_lambda);
  _expr_env_put_symbol_special_form(env, "define", expr_evaluate_define);
  _expr_env_put_symbol_special_form(env, "set!", expr_evaluate_set_excl);
  _expr_env_put_symbol_special_form(env, "begin", expr_evaluate_let);
  _expr_env_put_symbol_special_form(env, "let", expr_evaluate_begin);
  _expr_env_put_symbol_special_form(env, "let*", expr_evaluate_let_star);
  _expr_env_put_symbol_special_form(env, "letrec", expr_evaluate_letrec);
  _expr_env_put_symbol_special_form(env, "and", expr_evaluate_and);
  _expr_env_put_symbol_special_form(env, "or", expr_evaluate_or);
  _expr_env_put_symbol_special_form(env, "cond", expr_evaluate_cond);
  _expr_env_put_symbol_special_form(env, "when", expr_evaluate_when);
  _expr_env_put_symbol_special_form(env, "unless", expr_evaluate_unless);
  _expr_env_put_symbol_special_form(env, "do", expr_evaluate_do);
  _expr_env_put_symbol_special_form(env, "delay", expr_evaluate_delay);
  _expr_env_put_symbol_special_form(env, "force", expr_evaluate_force);
  _expr_env_put_symbol_special_form(env, "define-syntax", expr_evaluate_define_syntax);
  _expr_env_put_symbol_special_form(env, "let-syntax", expr_evaluate_let_syntax);
  _expr_env_put_symbol_special_form(env, "syntax-rules", expr_evaluate_syntax_rules);
}

void expr_eval_arguments(Expr_Env *env, Exprs *arguments) {
  da_foreach(Expr *, arg, arguments) {
    *arg = expr_eval(env, *arg);
  }
}
Expr *expr_eval(Expr_Env *env, Expr *input) {
  if (input->kind == EXPR_KIND_QUOTE) return input->quote;
  if (input->kind != EXPR_KIND_PAIR) return input;
  Expr *head = input->pair.left;
  if (!head) return input;
  head = expr_eval(env, head);
  if (head->kind != EXPR_KIND_SYMBOL) UNREACHABLE("is not a procedure");
  if (head->kind != EXPR_KIND_SYMBOL) UNREACHABLE("is not a procedure");
  Expr_Env_Symbol *item = ht_find(&env->symbols, head->symbol.name);
  if (!item) UNREACHABLE("unbound symbol");
  Exprs arguments = exprs_from_list(input->pair.right);
  if (arguments.items[arguments.count - 1] != NULL) UNREACHABLE("malformed arguments list");
  arguments.count -= 1;
  Expr_Env *call_env = env;
  Expr_Evaluator evaluator;
  switch (item->kind) {
  case EXPR_ENV_SYMBOL_KIND_SPECIAL_FORM: evaluator = item->special; break;
  case EXPR_ENV_SYMBOL_KIND_BUILTIN: {
    expr_eval_arguments(env, &arguments);
    evaluator = item->special;
  } break;
  case EXPR_ENV_SYMBOL_KIND_CLOSURE: TODO("closure call");
  default: UNREACHABLE("is not a procedure");
  }
  Expr *result = evaluator(call_env, arguments, input);
  da_free(arguments);
  return result;
}

Expr *expr_evaluate_quote(Expr_Env *env, Exprs arguments, Expr *arguments_expr) {
  UNUSED(env);
  UNUSED(arguments);
  return arguments_expr;
}
Expr *expr_evaluate_if(Expr_Env *env, Exprs arguments, Expr *arguments_expr) {
  UNUSED(arguments_expr);
  if (arguments.count < 2) UNREACHABLE("Too few arguments");
  if (arguments.count > 3) UNREACHABLE("Too many arguments");
  Expr *cond_expr = arguments.items[0];
  Expr *then_expr = arguments.items[1];
  Expr *else_expr = arguments.items[2];
  cond_expr = expr_convert_to_bool(expr_eval(env, cond_expr));
  if (cond_expr->kind != EXPR_KIND_BOOL) UNREACHABLE("illegal if condition expression kind");
  if (cond_expr->boolean) return expr_eval(env, then_expr);
  return expr_eval(env, else_expr);
}
Expr *expr_evaluate_lambda(Expr_Env *env, Exprs arguments, Expr *arguments_expr) {
  UNUSED(env); UNUSED(arguments); UNUSED(arguments_expr); TODO("expr_evaluate_lambda");
}
Expr *expr_evaluate_define(Expr_Env *env, Exprs arguments, Expr *arguments_expr) {
  UNUSED(env); UNUSED(arguments); UNUSED(arguments_expr); TODO("expr_evaluate_define");
}
Expr *expr_evaluate_set_excl(Expr_Env *env, Exprs arguments, Expr *arguments_expr) {
  UNUSED(env); UNUSED(arguments); UNUSED(arguments_expr); TODO("expr_evaluate_set_excl");
}
Expr *expr_evaluate_begin(Expr_Env *env, Exprs arguments, Expr *arguments_expr) {
  UNUSED(env); UNUSED(arguments); UNUSED(arguments_expr); TODO("expr_evaluate_begin");
}
Expr *expr_evaluate_let(Expr_Env *env, Exprs arguments, Expr *arguments_expr) {
  UNUSED(env); UNUSED(arguments); UNUSED(arguments_expr); TODO("expr_evaluate_let");
}
Expr *expr_evaluate_let_star(Expr_Env *env, Exprs arguments, Expr *arguments_expr) {
  UNUSED(env); UNUSED(arguments); UNUSED(arguments_expr); TODO("expr_evaluate_let_star");
}
Expr *expr_evaluate_letrec(Expr_Env *env, Exprs arguments, Expr *arguments_expr) {
  UNUSED(env); UNUSED(arguments); UNUSED(arguments_expr); TODO("expr_evaluate_letrec");
}
Expr *expr_evaluate_and(Expr_Env *env, Exprs arguments, Expr *arguments_expr) {
  UNUSED(env); UNUSED(arguments); UNUSED(arguments_expr); TODO("expr_evaluate_and");
}
Expr *expr_evaluate_or(Expr_Env *env, Exprs arguments, Expr *arguments_expr) {
  UNUSED(env); UNUSED(arguments); UNUSED(arguments_expr); TODO("expr_evaluate_or");
}
Expr *expr_evaluate_cond(Expr_Env *env, Exprs arguments, Expr *arguments_expr) {
  UNUSED(env); UNUSED(arguments); UNUSED(arguments_expr); TODO("expr_evaluate_cond");
}
Expr *expr_evaluate_when(Expr_Env *env, Exprs arguments, Expr *arguments_expr) {
  UNUSED(env); UNUSED(arguments); UNUSED(arguments_expr); TODO("expr_evaluate_when");
}
Expr *expr_evaluate_unless(Expr_Env *env, Exprs arguments, Expr *arguments_expr) {
  UNUSED(env); UNUSED(arguments); UNUSED(arguments_expr); TODO("expr_evaluate_unless");
}
Expr *expr_evaluate_do(Expr_Env *env, Exprs arguments, Expr *arguments_expr) {
  UNUSED(env); UNUSED(arguments); UNUSED(arguments_expr); TODO("expr_evaluate_do");
}
Expr *expr_evaluate_delay(Expr_Env *env, Exprs arguments, Expr *arguments_expr) {
  UNUSED(env); UNUSED(arguments); UNUSED(arguments_expr); TODO("expr_evaluate_delay");
}
Expr *expr_evaluate_force(Expr_Env *env, Exprs arguments, Expr *arguments_expr) {
  UNUSED(env); UNUSED(arguments); UNUSED(arguments_expr); TODO("expr_evaluate_force");
}
Expr *expr_evaluate_define_syntax(Expr_Env *env, Exprs arguments, Expr *arguments_expr) {
  UNUSED(env); UNUSED(arguments); UNUSED(arguments_expr); TODO("expr_evaluate_define_syntax");
}
Expr *expr_evaluate_let_syntax(Expr_Env *env, Exprs arguments, Expr *arguments_expr) {
  UNUSED(env); UNUSED(arguments); UNUSED(arguments_expr); TODO("expr_evaluate_let_syntax");
}
Expr *expr_evaluate_syntax_rules(Expr_Env *env, Exprs arguments, Expr *arguments_expr) {
  UNUSED(env); UNUSED(arguments); UNUSED(arguments_expr); TODO("expr_evaluate_syntax_rules");
}

#endif // EXPR_EVAL_IMPLEMENTATION
