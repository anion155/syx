#ifndef SYX_EVAL_BUILTINS_H
#define SYX_EVAL_BUILTINS_H

#include "sexpr_ast.h"
#include "syx_eval.h"

void syx_env_define_builtins(Syx_Env *env);

#endif // SYX_EVAL_BUILTINS_H

#if defined(SYX_EVAL_BUILTINS_IMPL) && !defined(SYX_EVAL_BUILTINS_IMPL_C)
#define SYX_EVAL_BUILTINS_IMPL_C

#include <stdio.h>

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
  SyxV *first = syxv_list_next_safe(&arguments);
  if (first->kind == SYXV_KIND_NIL) RUNTIME_ERROR("list of number expected", env);
  if (first->kind == SYXV_KIND_REAL) return syx__builtin_sub_upgrade_real(env, arguments, first->real);
  sexpr_int_t value = syx_convert_to_integer_v(env, first);
  SyxV **last_argument = NULL;
  syxv_list_for_each(argument, arguments, &last_argument) {
    if ((*argument)->kind == SYXV_KIND_REAL) return syx__builtin_sub_upgrade_real(env, argument_list, value);
    value -= syx_convert_to_integer_v(env, *argument);
  }
  if ((*last_argument)->kind != SYXV_KIND_NIL) RUNTIME_ERROR("list of number expected", env);
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
  SyxV *first = syxv_list_next_safe(&arguments);
  if (first->kind == SYXV_KIND_NIL) RUNTIME_ERROR("list of number expected", env);
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

/** Returns `#f` if argument is truthy, `#t` if falsy. */
SyxV *syx_builtin_not(Syx_Env *env, SyxV *arguments) {
  SyxV *argument = syxv_list_next_safe(&arguments);
  return make_syxv_bool(!syx_convert_to_bool_v(env, argument));
}

typedef bool (*Syx_Comparison_Predicate)(SyxV *left, SyxV *right);
typedef Syx_Comparison_Predicate (*Syx_Comparison_Predicate_Selector)(Syx_Env *env, SyxV *value);

SyxV *syx__builtin_compare(Syx_Env *env, SyxV *arguments, Syx_Comparison_Predicate_Selector predicate_selector) {
  SyxV *previous = syxv_list_next_safe(&arguments);
  Syx_Comparison_Predicate predicate;
  SyxV **last_argument = NULL;
  syxv_list_for_each(argument, arguments, &last_argument) {
    predicate = predicate_selector(env, previous);
    if (!predicate(previous, *argument)) return make_syxv_bool(false);
    previous = *argument;
  }
  if ((*last_argument)->kind != SYXV_KIND_NIL) RUNTIME_ERROR("list arguments expected", env);
  return make_syxv_bool(true);
}

bool syx__builtin_equivalent_boolean(SyxV *left, SyxV *right) {
  return right->kind == SYXV_KIND_BOOL && left->boolean == right->boolean;
}

bool syx__builtin_equivalent_integer(SyxV *left, SyxV *right) {
  switch (right->kind) {
    case SYXV_KIND_INTEGER: return left->integer == right->integer;
    case SYXV_KIND_REAL: return left->integer == right->real;
    default: return false;
  }
}

bool syx__builtin_equivalent_real(SyxV *left, SyxV *right) {
  switch (right->kind) {
    case SYXV_KIND_INTEGER: return left->real == right->integer;
    case SYXV_KIND_REAL: return left->real == right->real;
    default: return false;
  }
}

bool syx__builtin_equivalent_string(SyxV *left, SyxV *right) {
  return right->kind == SYXV_KIND_STRING && strcmp(left->string, right->string) == 0;
}

Syx_Comparison_Predicate syx__builtin_equivalent_selector(Syx_Env *env, SyxV *value) {
  switch (value->kind) {
    case SYXV_KIND_BOOL: return syx__builtin_equivalent_boolean;
    case SYXV_KIND_INTEGER: return syx__builtin_equivalent_integer;
    case SYXV_KIND_REAL: return syx__builtin_equivalent_real;
    case SYXV_KIND_STRING: return syx__builtin_equivalent_string;
    default: RUNTIME_ERROR("can not compare equivalence", env);
  }
}

/** Compare two or more numeric arguments in order.
 * Return `#t` if the condition holds for every adjacent pair, `#f` otherwise. */
SyxV *syx_builtin_equivalent(Syx_Env *env, SyxV *arguments) {
  return syx__builtin_compare(env, arguments, syx__builtin_equivalent_selector);
}

bool syx__builtin_lower_than_integer(SyxV *left, SyxV *right) {
  switch (right->kind) {
    case SYXV_KIND_INTEGER: return left->integer < right->integer;
    case SYXV_KIND_REAL: return left->integer < right->real;
    default: return false;
  }
}

bool syx__builtin_lower_than_real(SyxV *left, SyxV *right) {
  switch (right->kind) {
    case SYXV_KIND_INTEGER: return left->real < right->integer;
    case SYXV_KIND_REAL: return left->real < right->real;
    default: return false;
  }
}

bool syx__builtin_lower_than_string(SyxV *left, SyxV *right) {
  return right->kind == SYXV_KIND_STRING && strcmp(left->string, right->string) < 0;
}

Syx_Comparison_Predicate syx__builtin_lower_than_selector(Syx_Env *env, SyxV *value) {
  switch (value->kind) {
    case SYXV_KIND_INTEGER: return syx__builtin_lower_than_integer;
    case SYXV_KIND_REAL: return syx__builtin_lower_than_real;
    case SYXV_KIND_STRING: return syx__builtin_lower_than_string;
    default: RUNTIME_ERROR("can not compare lower than", env);
  }
}

/** Compare two or more numeric arguments in order.
 * Return `#t` if the condition holds for every adjacent pair, `#f` otherwise. */
SyxV *syx_builtin_lower_than(Syx_Env *env, SyxV *arguments) {
  return syx__builtin_compare(env, arguments, syx__builtin_lower_than_selector);
}

bool syx__builtin_lower_or_equal_integer(SyxV *left, SyxV *right) {
  switch (right->kind) {
    case SYXV_KIND_INTEGER: return left->integer <= right->integer;
    case SYXV_KIND_REAL: return left->integer <= right->real;
    default: return false;
  }
}

bool syx__builtin_lower_or_equal_real(SyxV *left, SyxV *right) {
  switch (right->kind) {
    case SYXV_KIND_INTEGER: return left->real <= right->integer;
    case SYXV_KIND_REAL: return left->real <= right->real;
    default: return false;
  }
}

bool syx__builtin_lower_or_equal_string(SyxV *left, SyxV *right) {
  return right->kind == SYXV_KIND_STRING && strcmp(left->string, right->string) <= 0;
}

Syx_Comparison_Predicate syx__builtin_lower_or_equal_selector(Syx_Env *env, SyxV *value) {
  switch (value->kind) {
    case SYXV_KIND_INTEGER: return syx__builtin_lower_or_equal_integer;
    case SYXV_KIND_REAL: return syx__builtin_lower_or_equal_real;
    case SYXV_KIND_STRING: return syx__builtin_lower_or_equal_string;
    default: RUNTIME_ERROR("can not compare equivalence", env);
  }
}

/** Compare two or more numeric arguments in order.
 * Return `#t` if the condition holds for every adjacent pair, `#f` otherwise. */
SyxV *syx_builtin_lower_or_equal(Syx_Env *env, SyxV *arguments) {
  return syx__builtin_compare(env, arguments, syx__builtin_lower_or_equal_selector);
}

bool syx__builtin_greater_than_integer(SyxV *left, SyxV *right) {
  switch (right->kind) {
    case SYXV_KIND_INTEGER: return left->integer > right->integer;
    case SYXV_KIND_REAL: return left->integer > right->real;
    default: return false;
  }
}

bool syx__builtin_greater_than_real(SyxV *left, SyxV *right) {
  switch (right->kind) {
    case SYXV_KIND_INTEGER: return left->real > right->integer;
    case SYXV_KIND_REAL: return left->real > right->real;
    default: return false;
  }
}

bool syx__builtin_greater_than_string(SyxV *left, SyxV *right) {
  return right->kind == SYXV_KIND_STRING && strcmp(left->string, right->string) > 0;
}

Syx_Comparison_Predicate syx__builtin_greater_than_selector(Syx_Env *env, SyxV *value) {
  switch (value->kind) {
    case SYXV_KIND_INTEGER: return syx__builtin_greater_than_integer;
    case SYXV_KIND_REAL: return syx__builtin_greater_than_real;
    case SYXV_KIND_STRING: return syx__builtin_greater_than_string;
    default: RUNTIME_ERROR("can not compare equivalence", env);
  }
}

/** Compare two or more numeric arguments in order.
 * Return `#t` if the condition holds for every adjacent pair, `#f` otherwise. */
SyxV *syx_builtin_greater_than(Syx_Env *env, SyxV *arguments) {
  return syx__builtin_compare(env, arguments, syx__builtin_greater_than_selector);
}

bool syx__builtin_greater_or_equal_integer(SyxV *left, SyxV *right) {
  switch (right->kind) {
    case SYXV_KIND_INTEGER: return left->integer > right->integer;
    case SYXV_KIND_REAL: return left->integer > right->real;
    default: return false;
  }
}

bool syx__builtin_greater_or_equal_real(SyxV *left, SyxV *right) {
  switch (right->kind) {
    case SYXV_KIND_INTEGER: return left->real > right->integer;
    case SYXV_KIND_REAL: return left->real > right->real;
    default: return false;
  }
}

bool syx__builtin_greater_or_equal_string(SyxV *left, SyxV *right) {
  return right->kind == SYXV_KIND_STRING && strcmp(left->string, right->string) > 0;
}

Syx_Comparison_Predicate syx__builtin_greater_or_equal_selector(Syx_Env *env, SyxV *value) {
  switch (value->kind) {
    case SYXV_KIND_INTEGER: return syx__builtin_greater_or_equal_integer;
    case SYXV_KIND_REAL: return syx__builtin_greater_or_equal_real;
    case SYXV_KIND_STRING: return syx__builtin_greater_or_equal_string;
    default: RUNTIME_ERROR("can not compare equivalence", env);
  }
}

/** Compare two or more numeric arguments in order.
 * Return `#t` if the condition holds for every adjacent pair, `#f` otherwise. */
SyxV *syx_builtin_greater_or_equal(Syx_Env *env, SyxV *arguments) {
  return syx__builtin_compare(env, arguments, syx__builtin_greater_or_equal_selector);
}

typedef struct File_Constant {
  int fd;
  FILE *stream;
} File_Constant;

define_constants_ht(FD_CONSTANTS, File_Constant) {
  *ht_put(FD_CONSTANTS, "stdout") = (File_Constant){.fd = STDOUT_FILENO, .stream = stdout};
  *ht_put(FD_CONSTANTS, "stderr") = (File_Constant){.fd = STDERR_FILENO, .stream = stderr};
  *ht_put(FD_CONSTANTS, "stdin") = (File_Constant){.fd = STDIN_FILENO, .stream = stdin};
}

FILE *parse_optional_file_descriptor(SyxV **arguments) {
  FILE *f = stdout;
  if ((*arguments)->kind != SYXV_KIND_PAIR) return f;
  if ((*arguments)->pair.left->kind != SYXV_KIND_SYMBOL) return f;
  File_Constant *constant = ht_find(FD_CONSTANTS(), (*arguments)->pair.left->symbol.name);
  if (!constant) return f;
  syxv_list_next(arguments);
  f = constant->stream;
  // TODO: pass fd as value not as constant
  return f;
}

/** Prints arguments to file. */
SyxV *syx_builtin_print(Syx_Env *env, SyxV *arguments) {
  FILE *f = parse_optional_file_descriptor(&arguments);
  bool first = true;
  syxv_list_for_each(argument, arguments) {
    String_View sv = syx_convert_to_string_v(env, *argument);
    if (!first && !syx_putc(f, ' ')) return make_syxv_bool(false);
    if (!syx_puts(f, sv)) return make_syxv_bool(false);
    first = false;
  }
  return make_syxv_bool(true);
}

/** Prints arguments to file, adds new line to the end. */
SyxV *syx_builtin_println(Syx_Env *env, SyxV *arguments) {
  FILE *f = parse_optional_file_descriptor(&arguments);
  syxv_list_for_each(argument, arguments) {
    String_View sv = syx_convert_to_string_v(env, *argument);
    if (!syx_puts(f, sv)) return make_syxv_bool(false);
    if (!syx_putc(f, ' ')) return make_syxv_bool(false);
  }
  if (!syx_putc(f, '\n')) return make_syxv_bool(false);
  return make_syxv_bool(true);
}

/** Prints formatted string to file. */
SyxV *syx_builtin_printf(Syx_Env *env, SyxV *arguments) {
  FILE *f = parse_optional_file_descriptor(&arguments);
  String_View fmt = syx_convert_to_string_v(env, syxv_list_next_safe(&arguments));
  String_View it = fmt;
  String_View str = fmt;
  for (; it.count; sv_chop_left(&it, 1)) {
    size_t format_size = 1;
    if (it.data[0] != '%') continue;
    // TODO: implement custom formats
    if (syx_put_sv_diff(f, &str, &it) < 0) return make_syxv_bool(false);
    sv_chop_left(&it, format_size);
    str = it;
    SyxV *argument = syxv_list_next_safe(&arguments);
    String_View sv = syx_convert_to_string_v(env, argument);
    if (!syx_puts(f, sv)) return make_syxv_bool(false);
  }
  if (syx_put_sv_diff(f, &str, &it) < 0) return make_syxv_bool(false);
  return make_syxv_bool(true);
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

  syx_env_define_cstr(env, "=", make_syxv_builtin(NULL, syx_builtin_equivalent));
  syx_env_define_cstr(env, "<", make_syxv_builtin(NULL, syx_builtin_lower_than));
  syx_env_define_cstr(env, "<=", make_syxv_builtin(NULL, syx_builtin_lower_or_equal));
  syx_env_define_cstr(env, ">", make_syxv_builtin(NULL, syx_builtin_greater_than));
  syx_env_define_cstr(env, ">=", make_syxv_builtin(NULL, syx_builtin_greater_or_equal));

  syx_env_define_cstr(env, "not", make_syxv_builtin(NULL, syx_builtin_not));

  syx_env_define_cstr(env, "print", make_syxv_builtin(NULL, syx_builtin_print));
  syx_env_define_cstr(env, "println", make_syxv_builtin(NULL, syx_builtin_println));
  syx_env_define_cstr(env, "printf", make_syxv_builtin(NULL, syx_builtin_printf));

  // expr_env_put_symbol(env, "mod", (Expr_Value){.kind = EXPR_VALUE_KIND_BUILTIN, .special = expr_builtin_arithmetic_mod});
  // expr_env_put_symbol(env, "expt", (Expr_Value){.kind = EXPR_VALUE_KIND_BUILTIN, .special = expr_builtin_arithmetic_expt});
  // expr_env_put_symbol(env, "abs", (Expr_Value){.kind = EXPR_VALUE_KIND_BUILTIN, .special = expr_builtin_arithmetic_abs});
  // expr_env_put_symbol(env, "min", (Expr_Value){.kind = EXPR_VALUE_KIND_BUILTIN, .special = expr_builtin_arithmetic_min});
  // expr_env_put_symbol(env, "max", (Expr_Value){.kind = EXPR_VALUE_KIND_BUILTIN, .special = expr_builtin_arithmetic_max});
}

#endif // SYX_EVAL_BUILTINS_IMPL
