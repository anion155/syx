#ifndef EXPR_EVAL_COMPARISON_H
#define EXPR_EVAL_COMPARISON_H

#include "expr_ast.h"
#include "expr_eval.h"

void expr_env_init_comparison(Expr_Env *env);

#endif // EXPR_EVAL_COMPARISON_H

#if defined(EXPR_EVAL_COMPARISON_IMPLEMENTATION) && !defined(EXPR_EVAL_COMPARISON_IMPLEMENTATION_C)
#define EXPR_EVAL_COMPARISON_IMPLEMENTATION_C

void expr_env_init_comparison(Expr_Env *env) {
  UNUSED(env);
  /** Builtins Comparison */
  // expr_env_put_symbol(env, "=", (Expr_Value){.kind = EXPR_VALUE_KIND_BUILTIN, .special = expr_builtin_comparison_equal});
  // expr_env_put_symbol(env, "<", (Expr_Value){.kind = EXPR_VALUE_KIND_BUILTIN, .special = expr_builtin_comparison_lower_than});
  // expr_env_put_symbol(env, ">", (Expr_Value){.kind = EXPR_VALUE_KIND_BUILTIN, .special = expr_builtin_comparison_greater_than});
  // expr_env_put_symbol(env, "<=", (Expr_Value){.kind = EXPR_VALUE_KIND_BUILTIN, .special = expr_builtin_comparison_lower_or_equal});
  // expr_env_put_symbol(env, ">=", (Expr_Value){.kind = EXPR_VALUE_KIND_BUILTIN, .special = expr_builtin_comparison_greater_or_equal});
  // expr_env_put_symbol(env, "not", (Expr_Value){.kind = EXPR_VALUE_KIND_BUILTIN, .special = expr_builtin_comparison_not});
}

#endif // EXPR_EVAL_COMPARISON_IMPLEMENTATION
