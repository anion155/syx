#ifndef EXPR_EVAL_TYPE_CONVERSION_H
#define EXPR_EVAL_TYPE_CONVERSION_H

#include "expr_ast.h"
#include "expr_eval.h"

void expr_env_init_type_conversion(Expr_Env *env);

#endif // EXPR_EVAL_TYPE_CONVERSION_H

#if defined(EXPR_EVAL_TYPE_CONVERSION_IMPLEMENTATION) && !defined(EXPR_EVAL_TYPE_CONVERSION_IMPLEMENTATION_C)
#define EXPR_EVAL_TYPE_CONVERSION_IMPLEMENTATION_C

void expr_env_init_type_conversion(Expr_Env *env) {
  UNUSED(env);
  /** Builtins Type conversion */
  // expr_env_put_symbol(env, "string->number", (Expr_Value){.kind = EXPR_VALUE_KIND_BUILTIN, .special = expr_builtin_type_conversion_string_to_number});
  // expr_env_put_symbol(env, "number->string", (Expr_Value){.kind = EXPR_VALUE_KIND_BUILTIN, .special = expr_builtin_type_conversion_number_to_string});
  // expr_env_put_symbol(env, "string->symbol", (Expr_Value){.kind = EXPR_VALUE_KIND_BUILTIN, .special = expr_builtin_type_conversion_string_to_symbol});
  // expr_env_put_symbol(env, "symbol->string", (Expr_Value){.kind = EXPR_VALUE_KIND_BUILTIN, .special = expr_builtin_type_conversion_symbol_to_string});
  // expr_env_put_symbol(env, "number->string", (Expr_Value){.kind = EXPR_VALUE_KIND_BUILTIN, .special = expr_builtin_type_conversion_number_to_string});
  // expr_env_put_symbol(env, "string->number", (Expr_Value){.kind = EXPR_VALUE_KIND_BUILTIN, .special = expr_builtin_type_conversion_string_to_number});
  // expr_env_put_symbol(env, "char->integer", (Expr_Value){.kind = EXPR_VALUE_KIND_BUILTIN, .special = expr_builtin_type_conversion_char_to_integer});
  // expr_env_put_symbol(env, "integer->char", (Expr_Value){.kind = EXPR_VALUE_KIND_BUILTIN, .special = expr_builtin_type_conversion_integer_to_char});
}

#endif // EXPR_EVAL_TYPE_CONVERSION_IMPLEMENTATION
