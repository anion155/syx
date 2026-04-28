#ifndef SYX_EVAL_BUILTINS_H
#define SYX_EVAL_BUILTINS_H

#include "sexpr_ast.h"
#include "syx_eval.h"

void syx_env_define_builtins(Syx_Env *env);

#endif // SYX_EVAL_BUILTINS_H

#if defined(SYX_EVAL_BUILTINS_IMPL) && !defined(SYX_EVAL_BUILTINS_IMPL_C)
#define SYX_EVAL_BUILTINS_IMPL_C

/** Builtins */

/** Takes exactly 2 arguments and returns a pair (left . right). */
SyxV *syx_builtin_cons(Syx_Env *env, SyxV *arguments) {
  UNUSED(env);
  SyxV *left = syxv_list_next_safe(&arguments);
  SyxV *right = syxv_list_next_safe(&arguments);
  return make_syxv_pair(left, right);
}

/** Returns the left element of a pair. */
SyxV *syx_builtin_car(Syx_Env *env, SyxV *arguments) {
  SyxV *list = syxv_list_next_safe(&arguments);
  if (list->kind != SYXV_KIND_PAIR) RUNTIME_ERROR("Pair expected as car argument", env);
  return list->pair.left;
}

/** Returns the right element of a pair. */
SyxV *syx_builtin_cdr(Syx_Env *env, SyxV *arguments) {
  SyxV *list = syxv_list_next_safe(&arguments);
  if (list->kind != SYXV_KIND_PAIR) RUNTIME_ERROR("Pair expected as car argument", env);
  return list->pair.right;
}

// SyxV *syx__builtin_summ_upgrade_real(Expr_Env *env, Expr_Arguments arguments, expr_int_t integer_value) {
//   UNUSED(env);
//   expr_real_t value = integer_value;
//   da_foreach(Expr_Value, argument, &arguments) {
//     if (argument->kind != EXPR_VALUE_KIND_EXPR) UNREACHABLE("Every argument supposed to be value");
//     argument->expr = expr_convert_to_real(argument->expr);
//     value += argument->expr->real;
//   }
//   return (Expr_Value){.kind = EXPR_VALUE_KIND_EXPR, .expr = make_expr_real(value)};
// }

SyxV *syx__builtin_summ_upgrade_real(Syx_Env *env, SyxV *arguments, sexpr_int_t integer_value) {
  sexpr_real_t value = integer_value;
  SyxV **last_argument = NULL;
  syxv_list_for_each(argument, arguments, &last_argument) {
    value += syx_convert_to_real_v(env, *argument);
  }
  if ((*last_argument)->kind != SYXV_KIND_NIL) RUNTIME_ERROR("list arguments expected", env);
  return make_syxv_real(value);
}

/** Sum of all arguments. */
SyxV *syx_builtin_summ(Syx_Env *env, SyxV *arguments) {
  sexpr_int_t value = 0;
  SyxV **last_argument = NULL;
  syxv_list_for_each(argument, arguments, &last_argument) {
    if ((*argument)->kind == SYXV_KIND_REAL) return syx__builtin_summ_upgrade_real(env, argument_list, value);
    value += syx_convert_to_integer_v(env, *argument);
  }
  if ((*last_argument)->kind != SYXV_KIND_NIL) RUNTIME_ERROR("list arguments expected", env);
  return make_syxv_integer(value);
}

SyxV *syx__builtin_sub_upgrade_real(Syx_Env *env, SyxV *arguments, sexpr_real_t initial_value) {
  sexpr_real_t value = initial_value;
  SyxV **last_argument = NULL;
  syxv_list_for_each(argument, arguments, &last_argument) {
    value -= syx_convert_to_real_v(env, *argument);
  }
  if ((*last_argument)->kind != SYXV_KIND_NIL) RUNTIME_ERROR("list arguments expected", env);
  return make_syxv_real(value);
}

/** Subtracts all arguments from first. */
SyxV *syx_builtin_sub(Syx_Env *env, SyxV *arguments) {
  SyxV *first = syxv_list_next(&arguments);
  if (first->kind == SYXV_KIND_REAL) return syx__builtin_sub_upgrade_real(env, arguments, first->real);
  sexpr_int_t value = syx_convert_to_integer_v(env, first);
  SyxV **last_argument = NULL;
  syxv_list_for_each(argument, arguments, &last_argument) {
    if ((*argument)->kind == SYXV_KIND_REAL) return syx__builtin_sub_upgrade_real(env, argument_list, value);
    value -= syx_convert_to_integer_v(env, *argument);
  }
  if ((*last_argument)->kind != SYXV_KIND_NIL) RUNTIME_ERROR("list arguments expected", env);
  return make_syxv_integer(value);
}

SyxV *syx__builtin_mul_upgrade_real(Syx_Env *env, SyxV *arguments, sexpr_int_t integer_value) {
  sexpr_real_t value = integer_value;
  SyxV **last_argument = NULL;
  syxv_list_for_each(argument, arguments, &last_argument) {
    value *= syx_convert_to_real_v(env, *argument);
  }
  if ((*last_argument)->kind != SYXV_KIND_NIL) RUNTIME_ERROR("list arguments expected", env);
  return make_syxv_real(value);
}

/** Multiplies all arguments. */
SyxV *syx_builtin_mul(Syx_Env *env, SyxV *arguments) {
  sexpr_int_t value = 1;
  SyxV **last_argument = NULL;
  syxv_list_for_each(argument, arguments, &last_argument) {
    if ((*argument)->kind == SYXV_KIND_REAL) return syx__builtin_mul_upgrade_real(env, argument_list, value);
    value *= syx_convert_to_integer_v(env, *argument);
  }
  if ((*last_argument)->kind != SYXV_KIND_NIL) RUNTIME_ERROR("list arguments expected", env);
  return make_syxv_integer(value);
}

SyxV *syx__builtin_div_upgrade_real(Syx_Env *env, SyxV *arguments, sexpr_real_t initial_value) {
  sexpr_real_t value = initial_value;
  SyxV **last_argument = NULL;
  syxv_list_for_each(argument, arguments, &last_argument) {
    value /= syx_convert_to_real_v(env, *argument);
  }
  if ((*last_argument)->kind != SYXV_KIND_NIL) RUNTIME_ERROR("list arguments expected", env);
  return make_syxv_real(value);
}

/** Divide first argument by every next sequentialy. */
SyxV *syx_builtin_div(Syx_Env *env, SyxV *arguments) {
  SyxV *first = syxv_list_next(&arguments);
  if (first->kind == SYXV_KIND_REAL) return syx__builtin_div_upgrade_real(env, arguments, first->real);
  sexpr_int_t value = syx_convert_to_integer_v(env, first);
  SyxV **last_argument = NULL;
  syxv_list_for_each(argument, arguments, &last_argument) {
    if ((*argument)->kind == SYXV_KIND_REAL) return syx__builtin_div_upgrade_real(env, argument_list, value);
    value /= syx_convert_to_integer_v(env, *argument);
  }
  if ((*last_argument)->kind != SYXV_KIND_NIL) RUNTIME_ERROR("list arguments expected", env);
  return make_syxv_integer(value);
}

void syx_env_define_builtins(Syx_Env *env) {
  /** Builtins */
  syx_env_define_cstr(env, "cons", make_syxv_builtin(NULL, syx_builtin_cons));
  syx_env_define_cstr(env, "car", make_syxv_builtin(NULL, syx_builtin_car));
  syx_env_define_cstr(env, "cdr", make_syxv_builtin(NULL, syx_builtin_cdr));
  syx_env_define_cstr(env, "+", make_syxv_builtin(NULL, syx_builtin_summ));
  syx_env_define_cstr(env, "-", make_syxv_builtin(NULL, syx_builtin_sub));
  syx_env_define_cstr(env, "*", make_syxv_builtin(NULL, syx_builtin_mul));
  syx_env_define_cstr(env, "/", make_syxv_builtin(NULL, syx_builtin_div));
  // expr_env_put_symbol(env, "mod", (Expr_Value){.kind = EXPR_VALUE_KIND_BUILTIN, .special = expr_builtin_arithmetic_mod});
  // expr_env_put_symbol(env, "expt", (Expr_Value){.kind = EXPR_VALUE_KIND_BUILTIN, .special = expr_builtin_arithmetic_expt});
  // expr_env_put_symbol(env, "abs", (Expr_Value){.kind = EXPR_VALUE_KIND_BUILTIN, .special = expr_builtin_arithmetic_abs});
  // expr_env_put_symbol(env, "min", (Expr_Value){.kind = EXPR_VALUE_KIND_BUILTIN, .special = expr_builtin_arithmetic_min});
  // expr_env_put_symbol(env, "max", (Expr_Value){.kind = EXPR_VALUE_KIND_BUILTIN, .special = expr_builtin_arithmetic_max});
}

#endif // SYX_EVAL_BUILTINS_IMPL
