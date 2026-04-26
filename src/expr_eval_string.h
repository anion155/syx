#ifndef EXPR_EVAL_STRING_H
#define EXPR_EVAL_STRING_H

#include "expr_ast.h"
#include "expr_eval.h"

void expr_env_init_string(Expr_Env *env);

#endif // EXPR_EVAL_STRING_H

#if defined(EXPR_EVAL_STRING_IMPLEMENTATION) && !defined(EXPR_EVAL_STRING_IMPLEMENTATION_C)
#define EXPR_EVAL_STRING_IMPLEMENTATION_C

void expr_env_init_string(Expr_Env *env) {
  UNUSED(env);
  /** Builtins String */
  // expr_env_put_symbol(env, "string-append", (Expr_Value){.kind = EXPR_VALUE_KIND_BUILTIN, .special = expr_builtin_string_append});
  // expr_env_put_symbol(env, "string-length", (Expr_Value){.kind = EXPR_VALUE_KIND_BUILTIN, .special = expr_builtin_string_length});
  // expr_env_put_symbol(env, "substring", (Expr_Value){.kind = EXPR_VALUE_KIND_BUILTIN, .special = expr_builtin_string_substring});
}

#endif // EXPR_EVAL_STRING_IMPLEMENTATION
