#ifndef SYX_EVAL_BUILTINS_H
#define SYX_EVAL_BUILTINS_H

#include "sexpr_ast.h"
#include "syx_eval.h"

void syx_env_define_builtins(Syx_Env *env);

#endif // SYX_EVAL_BUILTINS_H

#if defined(SYX_EVAL_BUILTINS_IMPL) && !defined(SYX_EVAL_BUILTINS_IMPL_C)
#define SYX_EVAL_BUILTINS_IMPL_C

/** Builtins */

/** Returns first argument unevaluated */
SyxV *syx_builtin_cons(Syx_Env *env, SyxV *arguments) {
  UNUSED(env);
  SyxV *left = syxv_list_next_safe(&arguments);
  SyxV *right = syxv_list_next_safe(&arguments);
  return make_syxv_pair(left, right);
}

// Expr_Value expr__builtin_arithmetic_sum_upgrade_real(Expr_Env *env, Expr_Arguments arguments, expr_int_t integer_value) {
//   UNUSED(env);
//   expr_real_t value = integer_value;
//   da_foreach(Expr_Value, argument, &arguments) {
//     if (argument->kind != EXPR_VALUE_KIND_EXPR) UNREACHABLE("Every argument supposed to be value");
//     argument->expr = expr_convert_to_real(argument->expr);
//     value += argument->expr->real;
//   }
//   return (Expr_Value){.kind = EXPR_VALUE_KIND_EXPR, .expr = make_expr_real(value)};
// }

// /** Sum of all arguments */
// Expr_Value expr_builtin_arithmetic_sum(Expr_Env *env, Expr_Arguments arguments) {
//   expr_int_t value = 0;
//   size_t index = 0;
//   da_foreach(Expr_Value, argument, &arguments) {
//     if (argument->kind != EXPR_VALUE_KIND_EXPR) UNREACHABLE("Every argument supposed to be value");
//     if (argument->expr->kind == EXPR_KIND_REAL) {
//       return expr__builtin_arithmetic_sum_upgrade_real(env, (Expr_Arguments){
//                                                                 .items = arguments.items + index,
//                                                                 .count = arguments.count - index,
//                                                                 .capacity = 0,
//                                                             },
//                                                        value);
//     }
//     argument->expr = expr_convert_to_integer(argument->expr);
//     value += argument->expr->integer;
//     index += 1;
//   }
//   return (Expr_Value){.kind = EXPR_VALUE_KIND_EXPR, .expr = make_expr_integer(value)};
// }

void syx_env_define_builtins(Syx_Env *env) {
  /** Builtins */
  syx_env_define_cstr(env, "cons", make_syxv_builtin(NULL, syx_builtin_cons));
  // expr_env_put_symbol(env, "+", (Expr_Value){.kind = EXPR_VALUE_KIND_BUILTIN, .special = expr_builtin_arithmetic_sum});
  // expr_env_put_symbol(env, "-", (Expr_Value){.kind = EXPR_VALUE_KIND_BUILTIN, .special = expr_builtin_arithmetic_sub});
  // expr_env_put_symbol(env, "*", (Expr_Value){.kind = EXPR_VALUE_KIND_BUILTIN, .special = expr_builtin_arithmetic_mul});
  // expr_env_put_symbol(env, "/", (Expr_Value){.kind = EXPR_VALUE_KIND_BUILTIN, .special = expr_builtin_arithmetic_div});
  // expr_env_put_symbol(env, "mod", (Expr_Value){.kind = EXPR_VALUE_KIND_BUILTIN, .special = expr_builtin_arithmetic_mod});
  // expr_env_put_symbol(env, "expt", (Expr_Value){.kind = EXPR_VALUE_KIND_BUILTIN, .special = expr_builtin_arithmetic_expt});
  // expr_env_put_symbol(env, "abs", (Expr_Value){.kind = EXPR_VALUE_KIND_BUILTIN, .special = expr_builtin_arithmetic_abs});
  // expr_env_put_symbol(env, "min", (Expr_Value){.kind = EXPR_VALUE_KIND_BUILTIN, .special = expr_builtin_arithmetic_min});
  // expr_env_put_symbol(env, "max", (Expr_Value){.kind = EXPR_VALUE_KIND_BUILTIN, .special = expr_builtin_arithmetic_max});
}

#endif // SYX_EVAL_BUILTINS_IMPL
