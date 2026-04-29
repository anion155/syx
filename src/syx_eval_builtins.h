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
  SyxV *left = syxv_list_next(&arguments);
  SyxV *right = syxv_list_next(&arguments);
  return make_syxv_pair(left, right);
}

/** Returns the left element of a pair. */
SyxV *syx_builtin_car(Syx_Env *env, SyxV *arguments) {
  SyxV *list = syxv_list_next(&arguments);
  if (list->kind != SYXV_KIND_PAIR) RUNTIME_ERROR("Pair expected as car argument", env);
  return list->pair.left;
}

/** Returns the right element of a pair. */
SyxV *syx_builtin_cdr(Syx_Env *env, SyxV *arguments) {
  SyxV *list = syxv_list_next(&arguments);
  if (list->kind != SYXV_KIND_PAIR) RUNTIME_ERROR("Pair expected as car argument", env);
  return list->pair.right;
}

/** Calls a function with a list as its argument list. */
SyxV *syx_builtin_apply(Syx_Env *env, SyxV *arguments) {
  SyxV *fn = syxv_list_next(&arguments);
  SyxV *call = rc_acquire(make_syxv_pair(fn, NULL));
  SyxV **it = &call->pair.right;
  SyxV **last_item = NULL;
  syxv_list_for_each(env, argument, arguments, &last_item) {
    if ((*it)) RUNTIME_ERROR("only last argument allowed to be pair with both values", env);
    if (argument->kind == SYXV_KIND_PAIR) {
      (*it) = argument;
      while ((*it)->kind == SYXV_KIND_PAIR) it = &(*it)->pair.right;
      if ((*it)->kind == SYXV_KIND_NIL) {
        rc_release((*it));
        (*it) = NULL;
      }
      continue;
    }
    (*it) = rc_acquire(make_syxv_pair(NULL, NULL));
    (*it)->pair.left = argument;
    it = &(*it)->pair.right;
  }
  if ((*last_item)->kind != SYXV_KIND_NIL) RUNTIME_ERROR("Invalid arguments list", env);
  (*it) = rc_acquire(make_syxv_nil());
  return syx_eval(env, call);
}

/** Applies a function to each element of a list and returns a new list of results. */
SyxV *syx_builtin_map(Syx_Env *env, SyxV *arguments) {
  SyxV *fn = syxv_list_next(&arguments);
  SyxV *list = syxv_list_next(&arguments);
  if (list->kind != SYXV_KIND_PAIR) RUNTIME_ERROR("list expected", env);
  SyxV *results = NULL;
  SyxV *call = NULL;
  syxv_list_map(env, item, list, &results) {
    if (call) rc_release(call);
    call = rc_acquire(make_syxv_pair(fn, make_syxv_pair(*item, make_syxv_nil())));
    *item = syx_eval(env, call);
  }
  if (call) rc_release(call);
  return results;
}

struct syx_upgradable_operator {
  sexpr_int_t (*nil)(Syx_Env *env);
  sexpr_int_t (*integer)(sexpr_int_t left, sexpr_int_t right);
  sexpr_real_t (*real)(sexpr_real_t left, sexpr_real_t right);
};

SyxV *syx__builtin_operator_upgrade_real(Syx_Env *env, SyxV *arguments, struct syx_upgradable_operator *operator, sexpr_real_t initial_value) {
  sexpr_real_t value = initial_value;
  syxv_list_for_each(env, argument, arguments) {
    value = operator->real(value, syx_convert_to_real_v(env, argument));
  }
  return make_syxv_real(value);
}

SyxV *syx__builtin_operator(Syx_Env *env, SyxV *arguments, struct syx_upgradable_operator *operator) {
  SyxV *first = syxv_list_next(&arguments);
  sexpr_int_t value;
  if (first->kind == SYXV_KIND_REAL) return syx__builtin_operator_upgrade_real(env, arguments, operator, first->real);
  if (first->kind == SYXV_KIND_NIL) value = operator->nil(env);
  else value = syx_convert_to_integer_v(env, first);
  syxv_list_for_each(env, argument, arguments) {
    if (argument->kind == SYXV_KIND_REAL) return syx__builtin_operator_upgrade_real(env, argument_list, operator, value);
    value = operator->integer(value, syx_convert_to_integer_v(env, argument));
  }
  return make_syxv_integer(value);
}

sexpr_int_t syx__operator_summ_nil(Syx_Env *env) { return (UNUSED(env), 0); }

sexpr_int_t syx__operator_summ_integer(sexpr_int_t left, sexpr_int_t right) { return left + right; }

sexpr_real_t syx__operator_summ_real(sexpr_real_t left, sexpr_real_t right) { return left + right; }

/** Sum of all arguments. */
SyxV *syx_builtin_summ(Syx_Env *env, SyxV *arguments) {
  struct syx_upgradable_operator operator= {
    .nil = syx__operator_summ_nil,
    .integer = syx__operator_summ_integer,
    .real = syx__operator_summ_real
  };
  return syx__builtin_operator(env, arguments, &operator);
}

sexpr_int_t syx__operator_sub_nil(Syx_Env *env) { RUNTIME_ERROR("list of number expected", env); }

sexpr_int_t syx__operator_sub_integer(sexpr_int_t left, sexpr_int_t right) { return left - right; }

sexpr_real_t syx__operator_sub_real(sexpr_real_t left, sexpr_real_t right) { return left - right; }

/** Subtracts all arguments from first. */
SyxV *syx_builtin_sub(Syx_Env *env, SyxV *arguments) {
  struct syx_upgradable_operator operator= {
    .nil = syx__operator_sub_nil,
    .integer = syx__operator_sub_integer,
    .real = syx__operator_sub_real
  };
  return syx__builtin_operator(env, arguments, &operator);
}

sexpr_int_t syx__operator_mul_nil(Syx_Env *env) { return (UNUSED(env), 1); }

sexpr_int_t syx__operator_mul_integer(sexpr_int_t left, sexpr_int_t right) { return left * right; }

sexpr_real_t syx__operator_mul_real(sexpr_real_t left, sexpr_real_t right) { return left * right; }

/** Multiplies all arguments. */
SyxV *syx_builtin_mul(Syx_Env *env, SyxV *arguments) {
  struct syx_upgradable_operator operator= {
    .nil = syx__operator_mul_nil,
    .integer = syx__operator_mul_integer,
    .real = syx__operator_mul_real
  };
  return syx__builtin_operator(env, arguments, &operator);
}

sexpr_int_t syx__operator_div_nil(Syx_Env *env) { RUNTIME_ERROR("list of number expected", env); }

sexpr_int_t syx__operator_div_integer(sexpr_int_t left, sexpr_int_t right) { return left / right; }

sexpr_real_t syx__operator_div_real(sexpr_real_t left, sexpr_real_t right) { return left / right; }

/** Divide first argument by every next sequentialy. */
SyxV *syx_builtin_div(Syx_Env *env, SyxV *arguments) {
  struct syx_upgradable_operator operator= {
    .nil = syx__operator_div_nil,
    .integer = syx__operator_div_integer,
    .real = syx__operator_div_real
  };
  return syx__builtin_operator(env, arguments, &operator);
}

/** Returns `#f` if argument is truthy, `#t` if falsy. */
SyxV *syx_builtin_not(Syx_Env *env, SyxV *arguments) {
  SyxV *argument = syxv_list_next(&arguments);
  return make_syxv_bool(!syx_convert_to_bool_v(env, argument));
}

typedef bool (*Syx_Comparison_Predicate)(SyxV *left, SyxV *right);
typedef Syx_Comparison_Predicate (*Syx_Comparison_Predicate_Selector)(Syx_Env *env, SyxV *value);

SyxV *syx__builtin_compare(Syx_Env *env, SyxV *arguments, Syx_Comparison_Predicate_Selector predicate_selector) {
  SyxV *previous = syxv_list_next(&arguments);
  Syx_Comparison_Predicate predicate;
  syxv_list_for_each(env, argument, arguments) {
    predicate = predicate_selector(env, previous);
    if (!predicate(previous, argument)) return make_syxv_bool(false);
    previous = argument;
  }
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
  syxv_list_next_nullable(arguments);
  f = constant->stream;
  // TODO: pass fd as value not as constant
  return f;
}

/** Prints arguments to file. */
SyxV *syx_builtin_print(Syx_Env *env, SyxV *arguments) {
  FILE *f = parse_optional_file_descriptor(&arguments);
  bool first = true;
  syxv_list_for_each(env, argument, arguments) {
    String_View sv = syx_convert_to_string_v(env, argument);
    if (!first && !syx_putc(f, ' ')) return make_syxv_bool(false);
    if (!syx_puts(f, sv)) return make_syxv_bool(false);
    first = false;
  }
  return make_syxv_bool(true);
}

/** Prints arguments to file, adds new line to the end. */
SyxV *syx_builtin_println(Syx_Env *env, SyxV *arguments) {
  FILE *f = parse_optional_file_descriptor(&arguments);
  syxv_list_for_each(env, argument, arguments) {
    String_View sv = syx_convert_to_string_v(env, argument);
    if (!syx_puts(f, sv)) return make_syxv_bool(false);
    if (!syx_putc(f, ' ')) return make_syxv_bool(false);
  }
  if (!syx_putc(f, '\n')) return make_syxv_bool(false);
  return make_syxv_bool(true);
}

/** Prints formatted string to file. */
SyxV *syx_builtin_printf(Syx_Env *env, SyxV *arguments) {
  FILE *f = parse_optional_file_descriptor(&arguments);
  String_View fmt = syx_convert_to_string_v(env, syxv_list_next(&arguments));
  String_View it = fmt;
  String_View str = fmt;
  for (; it.count; sv_chop_left(&it, 1)) {
    size_t format_size = 1;
    if (it.data[0] != '%') continue;
    // TODO: implement custom formats
    if (syx_put_sv_diff(f, &str, &it) < 0) return make_syxv_bool(false);
    sv_chop_left(&it, format_size);
    str = it;
    SyxV *argument = syxv_list_next(&arguments);
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
  syx_env_define_cstr(env, "apply", make_syxv_builtin(NULL, syx_builtin_apply));
  syx_env_define_cstr(env, "map", make_syxv_builtin(NULL, syx_builtin_map));

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
}

#endif // SYX_EVAL_BUILTINS_IMPL
