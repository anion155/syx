#ifndef EXPR_EVAL_ARITHMETIC_H
#define EXPR_EVAL_ARITHMETIC_H

#include "expr_ast.h"
#include "expr_eval.h"

void expr_env_init_arithmetic(Expr_Env *env);

#endif // EXPR_EVAL_ARITHMETIC_H

#if defined(EXPR_EVAL_ARITHMETIC_IMPLEMENTATION) && !defined(EXPR_EVAL_ARITHMETIC_IMPLEMENTATION_C)
#define EXPR_EVAL_ARITHMETIC_IMPLEMENTATION_C

/** Builtins */
Expr_Value expr__builtin_arithmetic_sum_upgrade_real(Expr_Env *env, Expr_Arguments arguments, expr_int_t integer_value) {
  UNUSED(env);
  expr_real_t value = integer_value;
  da_foreach(Expr_Value, argument, &arguments) {
    if (argument->kind != EXPR_VALUE_KIND_EXPR) UNREACHABLE("Every argument supposed to be value");
    argument->expr = expr_convert_to_real(argument->expr);
    value += argument->expr->real;
  }
  return (Expr_Value){.kind = EXPR_VALUE_KIND_EXPR, .expr = make_expr_real(value)};
}
/** Sum of all arguments */
Expr_Value expr_builtin_arithmetic_sum(Expr_Env *env, Expr_Arguments arguments) {
  expr_int_t value = 0;
  size_t index = 0;
  da_foreach(Expr_Value, argument, &arguments) {
    if (argument->kind != EXPR_VALUE_KIND_EXPR) UNREACHABLE("Every argument supposed to be value");
    if (argument->expr->kind == EXPR_KIND_REAL) {
      return expr__builtin_arithmetic_sum_upgrade_real(env, (Expr_Arguments){
        .items = arguments.items + index,
        .count = arguments.count - index,
        .capacity = 0,
      }, value);
    }
    argument->expr = expr_convert_to_integer(argument->expr);
    value += argument->expr->integer;
    index += 1;
  }
  return (Expr_Value){.kind = EXPR_VALUE_KIND_EXPR, .expr = make_expr_integer(value)};
}

/** Subtract rest from first, or negate if one arg */
Expr_Value expr_builtin_arithmetic_sub(Expr_Env *env, Expr_Arguments arguments) {
  UNUSED(env); UNUSED(arguments); TODO("expr_builtin_arithmetic_sub");
}

/** Product of all arguments */
Expr_Value expr_builtin_arithmetic_mul(Expr_Env *env, Expr_Arguments arguments) {
  UNUSED(env); UNUSED(arguments); TODO("expr_builtin_arithmetic_mul");
}

/** Divide first by rest, or reciprocal if one arg */
Expr_Value expr_builtin_arithmetic_div(Expr_Env *env, Expr_Arguments arguments) {
  UNUSED(env); UNUSED(arguments); TODO("expr_builtin_arithmetic_div");
}

/** Remainder after division */
Expr_Value expr_builtin_arithmetic_mod(Expr_Env *env, Expr_Arguments arguments) {
  UNUSED(env); UNUSED(arguments); TODO("expr_builtin_arithmetic_mod");
}

/** Raise first argument to the power of second */
Expr_Value expr_builtin_arithmetic_expt(Expr_Env *env, Expr_Arguments arguments) {
  UNUSED(env); UNUSED(arguments); TODO("expr_builtin_arithmetic_expt");
}

/** Absolute value */
Expr_Value expr_builtin_arithmetic_abs(Expr_Env *env, Expr_Arguments arguments) {
  UNUSED(env); UNUSED(arguments); TODO("expr_builtin_arithmetic_abs");
}

/** Smallest of all arguments */
Expr_Value expr_builtin_arithmetic_min(Expr_Env *env, Expr_Arguments arguments) {
  UNUSED(env); UNUSED(arguments); TODO("expr_builtin_arithmetic_min");
}

/** Largest of all arguments */
Expr_Value expr_builtin_arithmetic_max(Expr_Env *env, Expr_Arguments arguments) {
  UNUSED(env); UNUSED(arguments); TODO("expr_builtin_arithmetic_max");
}

void expr_env_init_arithmetic(Expr_Env *env) {
  /** Builtins Arithmetic */
  expr_env_put_symbol(env, "+", (Expr_Value){.kind = EXPR_VALUE_KIND_BUILTIN, .special = expr_builtin_arithmetic_sum});
  expr_env_put_symbol(env, "-", (Expr_Value){.kind = EXPR_VALUE_KIND_BUILTIN, .special = expr_builtin_arithmetic_sub});
  expr_env_put_symbol(env, "*", (Expr_Value){.kind = EXPR_VALUE_KIND_BUILTIN, .special = expr_builtin_arithmetic_mul});
  expr_env_put_symbol(env, "/", (Expr_Value){.kind = EXPR_VALUE_KIND_BUILTIN, .special = expr_builtin_arithmetic_div});
  expr_env_put_symbol(env, "mod", (Expr_Value){.kind = EXPR_VALUE_KIND_BUILTIN, .special = expr_builtin_arithmetic_mod});
  expr_env_put_symbol(env, "expt", (Expr_Value){.kind = EXPR_VALUE_KIND_BUILTIN, .special = expr_builtin_arithmetic_expt});
  expr_env_put_symbol(env, "abs", (Expr_Value){.kind = EXPR_VALUE_KIND_BUILTIN, .special = expr_builtin_arithmetic_abs});
  expr_env_put_symbol(env, "min", (Expr_Value){.kind = EXPR_VALUE_KIND_BUILTIN, .special = expr_builtin_arithmetic_min});
  expr_env_put_symbol(env, "max", (Expr_Value){.kind = EXPR_VALUE_KIND_BUILTIN, .special = expr_builtin_arithmetic_max});
}

#endif // EXPR_EVAL_ARITHMETIC_IMPLEMENTATION
