#ifndef EXPR_EVAL_EQUALITY_H
#define EXPR_EVAL_EQUALITY_H

#include "expr_ast.h"
#include "expr_eval.h"

void expr_env_init_equality(Expr_Env *env);

#endif // EXPR_EVAL_EQUALITY_H

#if defined(EXPR_EVAL_EQUALITY_IMPLEMENTATION) && !defined(EXPR_EVAL_EQUALITY_IMPLEMENTATION_C)
#define EXPR_EVAL_EQUALITY_IMPLEMENTATION_C

void expr_env_init_equality(Expr_Env *env) {
  UNUSED(env);
  /** Builtins Equality */
  // expr_env_put_symbol(env, "eq?", (Expr_Value){.kind = EXPR_VALUE_KIND_BUILTIN, .special = expr_builtin_equality_is_eq});
  // expr_env_put_symbol(env, "eqv?", (Expr_Value){.kind = EXPR_VALUE_KIND_BUILTIN, .special = expr_builtin_equality_is_eqv});
  // expr_env_put_symbol(env, "equal?", (Expr_Value){.kind = EXPR_VALUE_KIND_BUILTIN, .special = expr_builtin_equality_is_equal});
}

#endif // EXPR_EVAL_EQUALITY_IMPLEMENTATION
