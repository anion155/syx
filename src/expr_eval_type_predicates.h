#ifndef EXPR_EVAL_TYPE_PREDICATES_H
#define EXPR_EVAL_TYPE_PREDICATES_H

#include "expr_ast.h"
#include "expr_eval.h"

void expr_env_init_type_predicates(Expr_Env *env);

#endif // EXPR_EVAL_TYPE_PREDICATES_H

#if defined(EXPR_EVAL_TYPE_PREDICATES_IMPLEMENTATION) && !defined(EXPR_EVAL_TYPE_PREDICATES_IMPLEMENTATION_C)
#define EXPR_EVAL_TYPE_PREDICATES_IMPLEMENTATION_C

void expr_env_init_type_predicates(Expr_Env *env) {
  UNUSED(env);
  /** Builtins Type predicates */
  // expr_env_put_symbol(env, "list?", (Expr_Value){.kind = EXPR_VALUE_KIND_BUILTIN, .special = expr_builtin_type_predicates_is_list});
  // expr_env_put_symbol(env, "null?", (Expr_Value){.kind = EXPR_VALUE_KIND_BUILTIN, .special = expr_builtin_type_predicates_is_null});
  // expr_env_put_symbol(env, "pair?", (Expr_Value){.kind = EXPR_VALUE_KIND_BUILTIN, .special = expr_builtin_type_predicates_is_pair});
  // expr_env_put_symbol(env, "number?", (Expr_Value){.kind = EXPR_VALUE_KIND_BUILTIN, .special = expr_builtin_type_predicates_is_number});
  // expr_env_put_symbol(env, "string?", (Expr_Value){.kind = EXPR_VALUE_KIND_BUILTIN, .special = expr_builtin_type_predicates_is_string});
  // expr_env_put_symbol(env, "symbol?", (Expr_Value){.kind = EXPR_VALUE_KIND_BUILTIN, .special = expr_builtin_type_predicates_is_symbol});
  // expr_env_put_symbol(env, "boolean?", (Expr_Value){.kind = EXPR_VALUE_KIND_BUILTIN, .special = expr_builtin_type_predicates_is_boolean});
  // expr_env_put_symbol(env, "procedure?", (Expr_Value){.kind = EXPR_VALUE_KIND_BUILTIN, .special = expr_builtin_type_predicates_is_procedure});
}

#endif // EXPR_EVAL_TYPE_PREDICATES_IMPLEMENTATION
