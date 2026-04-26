#ifndef EXPR_EVAL_LIST_H
#define EXPR_EVAL_LIST_H

#include "expr_ast.h"
#include "expr_eval.h"

void expr_env_init_list(Expr_Env *env);

#endif // EXPR_EVAL_LIST_H

#if defined(EXPR_EVAL_LIST_IMPLEMENTATION) && !defined(EXPR_EVAL_LIST_IMPLEMENTATION_C)
#define EXPR_EVAL_LIST_IMPLEMENTATION_C

void expr_env_init_list(Expr_Env *env) {
  UNUSED(env);
  /** Builtins List */
  // expr_env_put_symbol(env, "cons", (Expr_Value){.kind = EXPR_VALUE_KIND_BUILTIN, .special = expr_builtin_list_cons});
  // expr_env_put_symbol(env, "car", (Expr_Value){.kind = EXPR_VALUE_KIND_BUILTIN, .special = expr_builtin_list_car});
  // expr_env_put_symbol(env, "list", (Expr_Value){.kind = EXPR_VALUE_KIND_BUILTIN, .special = expr_builtin_list_list});
  // expr_env_put_symbol(env, "length", (Expr_Value){.kind = EXPR_VALUE_KIND_BUILTIN, .special = expr_builtin_list_length});
  // expr_env_put_symbol(env, "append", (Expr_Value){.kind = EXPR_VALUE_KIND_BUILTIN, .special = expr_builtin_list_append});
  // expr_env_put_symbol(env, "reverse", (Expr_Value){.kind = EXPR_VALUE_KIND_BUILTIN, .special = expr_builtin_list_reverse});
  // expr_env_put_symbol(env, "map", (Expr_Value){.kind = EXPR_VALUE_KIND_BUILTIN, .special = expr_builtin_list_map});
  // expr_env_put_symbol(env, "filter", (Expr_Value){.kind = EXPR_VALUE_KIND_BUILTIN, .special = expr_builtin_list_filter});
  // expr_env_put_symbol(env, "fold", (Expr_Value){.kind = EXPR_VALUE_KIND_BUILTIN, .special = expr_builtin_list_fold});
  // expr_env_put_symbol(env, "for-each", (Expr_Value){.kind = EXPR_VALUE_KIND_BUILTIN, .special = expr_builtin_list_for_each});
}

#endif // EXPR_EVAL_LIST_IMPLEMENTATION
