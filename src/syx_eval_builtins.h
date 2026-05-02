#ifndef SYX_EVAL_BUILTINS_H
#define SYX_EVAL_BUILTINS_H

#include "syx_eval.h"
#include "syx_value.h"

void syx_env_define_builtins(Syx_Env *env);

#endif // SYX_EVAL_BUILTINS_H

#if defined(SYX_EVAL_BUILTINS_IMPL) && !defined(SYX_EVAL_BUILTINS_IMPL_C)
#define SYX_EVAL_BUILTINS_IMPL_C

#include <stdio.h>

/** Builtins */

/** Takes exactly 2 arguments and returns a pair (left . right). */
SyxV *syx_builtin_cons(Syx_Eval_Ctx *ctx, SyxV *arguments) {
  UNUSED(ctx);
  SyxV *left = syxv_list_next(&arguments);
  SyxV *right = syxv_list_next(&arguments);
  return make_syxv_pair(left, right);
}

/** Returns the left element of a pair. */
SyxV *syx_builtin_car(Syx_Eval_Ctx *ctx, SyxV *arguments) {
  SyxV *list = syxv_list_next(&arguments);
  if (list->kind != SYXV_KIND_PAIR) RUNTIME_ERROR("Pair expected as car argument", ctx);
  return list->pair.left;
}

/** Returns the right element of a pair. */
SyxV *syx_builtin_cdr(Syx_Eval_Ctx *ctx, SyxV *arguments) {
  SyxV *list = syxv_list_next(&arguments);
  if (list->kind != SYXV_KIND_PAIR) RUNTIME_ERROR("Pair expected as car argument", ctx);
  return list->pair.right;
}

/** Calls a function with a list as its argument list. */
SyxV *syx_builtin_apply(Syx_Eval_Ctx *ctx, SyxV *arguments) {
  SyxV *fn = syxv_list_next(&arguments);
  SyxV *call = rc_acquire(make_syxv_pair(fn, NULL));
  SyxV **it = &call->pair.right;
  SyxV **last_item = NULL;
  syxv_list_for_each(argument, arguments, &last_item) {
    if ((*it)) RUNTIME_ERROR("only last argument allowed to be pair with both values", ctx);
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
  if ((*last_item)->kind != SYXV_KIND_NIL) RUNTIME_ERROR("Invalid arguments list", ctx);
  (*it) = rc_acquire(make_syxv_nil());
  return syx_eval(ctx, call);
}

/** Applies a function to each element of a list and returns a new list of results. */
SyxV *syx_builtin_map(Syx_Eval_Ctx *ctx, SyxV *arguments) {
  SyxV *fn = syxv_list_next(&arguments);
  SyxV *list = syxv_list_next(&arguments);
  if (list->kind != SYXV_KIND_PAIR) RUNTIME_ERROR("list expected", ctx);
  SyxV *results = NULL;
  SyxV *call = NULL;
  syxv_list_map(item, list, &results) {
    if (call) rc_release(call);
    call = rc_acquire(make_syxv_pair(fn, make_syxv_pair(*item, make_syxv_nil())));
    *item = syx_eval(ctx, call);
  }
  if (call) rc_release(call);
  return results;
}

struct syx_upgradable_operator {
  syx_integer_t (*nil)(Syx_Eval_Ctx *ctx);
  syx_integer_t (*integer)(syx_integer_t left, syx_integer_t right);
  syx_fractional_t (*fractional)(syx_fractional_t left, syx_fractional_t right);
};

SyxV *syx__builtin_operator_upgrade(Syx_Eval_Ctx *ctx, SyxV *arguments, struct syx_upgradable_operator *operator, syx_fractional_t initial_value) {
  syx_fractional_t value = initial_value;
  syxv_list_for_each(argument, arguments) {
    value = operator->fractional(value, syx_convert_to_fractional_v(ctx, argument));
  }
  return make_syxv_fractional(value);
}

SyxV *syx__builtin_operator(Syx_Eval_Ctx *ctx, SyxV *arguments, struct syx_upgradable_operator *operator) {
  SyxV *first = syxv_list_next(&arguments);
  syx_integer_t value;
  if (first->kind == SYXV_KIND_FRACTIONAL) return syx__builtin_operator_upgrade(ctx, arguments, operator, first->fractional);
  if (first->kind == SYXV_KIND_NIL) value = operator->nil(ctx);
  else value = syx_convert_to_integer_v(ctx, first);
  syxv_list_for_each(argument, arguments) {
    if (argument->kind == SYXV_KIND_FRACTIONAL) return syx__builtin_operator_upgrade(ctx, argument_list, operator, value);
    value = operator->integer(value, syx_convert_to_integer_v(ctx, argument));
  }
  return make_syxv_integer(value);
}

syx_integer_t syx__operator_summ_nil(Syx_Eval_Ctx *ctx) { return (UNUSED(ctx), 0); }

syx_integer_t syx__operator_summ_integer(syx_integer_t left, syx_integer_t right) { return left + right; }

syx_fractional_t syx__operator_summ_fractional(syx_fractional_t left, syx_fractional_t right) { return left + right; }

/** Sum of all arguments. */
SyxV *syx_builtin_summ(Syx_Eval_Ctx *ctx, SyxV *arguments) {
  struct syx_upgradable_operator operator= {
    .nil = syx__operator_summ_nil,
    .integer = syx__operator_summ_integer,
    .fractional = syx__operator_summ_fractional
  };
  return syx__builtin_operator(ctx, arguments, &operator);
}

syx_integer_t syx__operator_sub_nil(Syx_Eval_Ctx *ctx) { RUNTIME_ERROR("list of number expected", ctx); }

syx_integer_t syx__operator_sub_integer(syx_integer_t left, syx_integer_t right) { return left - right; }

syx_fractional_t syx__operator_sub_fractional(syx_fractional_t left, syx_fractional_t right) { return left - right; }

/** Subtracts all arguments from first. */
SyxV *syx_builtin_sub(Syx_Eval_Ctx *ctx, SyxV *arguments) {
  struct syx_upgradable_operator operator= {
    .nil = syx__operator_sub_nil,
    .integer = syx__operator_sub_integer,
    .fractional = syx__operator_sub_fractional
  };
  return syx__builtin_operator(ctx, arguments, &operator);
}

syx_integer_t syx__operator_mul_nil(Syx_Eval_Ctx *ctx) { return (UNUSED(ctx), 1); }

syx_integer_t syx__operator_mul_integer(syx_integer_t left, syx_integer_t right) { return left * right; }

syx_fractional_t syx__operator_mul_fractional(syx_fractional_t left, syx_fractional_t right) { return left * right; }

/** Multiplies all arguments. */
SyxV *syx_builtin_mul(Syx_Eval_Ctx *ctx, SyxV *arguments) {
  struct syx_upgradable_operator operator= {
    .nil = syx__operator_mul_nil,
    .integer = syx__operator_mul_integer,
    .fractional = syx__operator_mul_fractional
  };
  return syx__builtin_operator(ctx, arguments, &operator);
}

syx_integer_t syx__operator_div_nil(Syx_Eval_Ctx *ctx) { RUNTIME_ERROR("list of number expected", ctx); }

syx_integer_t syx__operator_div_integer(syx_integer_t left, syx_integer_t right) { return left / right; }

syx_fractional_t syx__operator_div_fractional(syx_fractional_t left, syx_fractional_t right) { return left / right; }

/** Divide first argument by every next sequentialy. */
SyxV *syx_builtin_div(Syx_Eval_Ctx *ctx, SyxV *arguments) {
  struct syx_upgradable_operator operator= {
    .nil = syx__operator_div_nil,
    .integer = syx__operator_div_integer,
    .fractional = syx__operator_div_fractional
  };
  return syx__builtin_operator(ctx, arguments, &operator);
}

/** Returns `#f` if argument is truthy, `#t` if falsy. */
SyxV *syx_builtin_not(Syx_Eval_Ctx *ctx, SyxV *arguments) {
  SyxV *argument = syxv_list_next(&arguments);
  return make_syxv_bool(!syx_convert_to_bool_v(ctx, argument));
}

typedef bool (*Syx_Compare)(Syx_Eval_Ctx *ctx, SyxV *left, SyxV *right);

SyxV *syx__builtin_compare(Syx_Eval_Ctx *ctx, SyxV *arguments, Syx_Compare compare) {
  SyxV *previous = syxv_list_next(&arguments);
  syxv_list_for_each(argument, arguments) {
    if (!compare(ctx, previous, argument)) return make_syxv_bool(false);
    previous = argument;
  }
  return make_syxv_bool(true);
}

bool syx__builtin_equivalent_comparator(Syx_Eval_Ctx *ctx, SyxV *left, SyxV *right) {
  if (left == right) return true;
  switch (left->kind) {
    case SYXV_KIND_NIL: return right->kind == SYXV_KIND_NIL;
    case SYXV_KIND_SYMBOL: return right->kind == SYXV_KIND_SYMBOL && strcmp(left->symbol.name, right->symbol.name) == 0;
    case SYXV_KIND_PAIR: return (
        right->kind == SYXV_KIND_PAIR &&
        syx__builtin_equivalent_comparator(ctx, left->pair.left, right->pair.left) &&
        syx__builtin_equivalent_comparator(ctx, left->pair.right, right->pair.right));
    case SYXV_KIND_BOOL: return false; // should work on left == right level
    case SYXV_KIND_INTEGER: {
      switch (right->kind) {
        case SYXV_KIND_INTEGER: return left->integer == right->integer;
        case SYXV_KIND_FRACTIONAL: return left->integer == right->fractional;
        default: return false;
      }
    }
    case SYXV_KIND_FRACTIONAL: {
      switch (right->kind) {
        case SYXV_KIND_INTEGER: return left->fractional == right->integer;
        case SYXV_KIND_FRACTIONAL: return left->fractional == right->fractional;
        default: return false;
      }
    }
    case SYXV_KIND_STRING: return right->kind == SYXV_KIND_SYMBOL && sv_eq(left->string, right->string) == 0;
    case SYXV_KIND_QUOTE: return right->kind == SYXV_KIND_QUOTE && syx__builtin_equivalent_comparator(ctx, left->quote, right->quote);
    case SYXV_KIND_SPECIALF: return false; // should work on left == right level
    case SYXV_KIND_BUILTIN: return false;  // should work on left == right level
    case SYXV_KIND_CLOSURE: return false;  // should work on left == right level
  }
}

/** Applies structural check between each consequence pairs. */
SyxV *syx_builtin_equivalent(Syx_Eval_Ctx *ctx, SyxV *arguments) {
  return syx__builtin_compare(ctx, arguments, syx__builtin_equivalent_comparator);
}

#define syx__builtin_comparison_comparator(self_name, operator)                                               \
  switch (left->kind) {                                                                                       \
    case SYXV_KIND_INTEGER: {                                                                                 \
      switch (right->kind) {                                                                                  \
        case SYXV_KIND_INTEGER: return left->integer operator right->integer;                                 \
        case SYXV_KIND_FRACTIONAL: return left->integer operator right->fractional;                           \
        default: RUNTIME_ERROR("can not compare with lower than", ctx);                                       \
      }                                                                                                       \
    }                                                                                                         \
    case SYXV_KIND_FRACTIONAL: {                                                                              \
      switch (right->kind) {                                                                                  \
        case SYXV_KIND_INTEGER: return left->fractional operator right->integer;                              \
        case SYXV_KIND_FRACTIONAL: return left->fractional operator right->fractional;                        \
        default: RUNTIME_ERROR("can not compare with lower than", ctx);                                       \
      }                                                                                                       \
    }                                                                                                         \
    case SYXV_KIND_STRING: {                                                                                  \
      right = rc_acquire(syx_convert_to_string(ctx, right));                                                  \
      bool result = strcmp(left->string.data, right->string.data) operator(0);                                \
      rc_release(right);                                                                                      \
      return result;                                                                                          \
    }                                                                                                         \
    case SYXV_KIND_PAIR: return (                                                                             \
        right->kind == SYXV_KIND_PAIR &&                                                                      \
        self_name(ctx, left->pair.left, right->pair.left) &&                                                  \
        self_name(ctx, left->pair.right, right->pair.right));                                                 \
    case SYXV_KIND_QUOTE: return right->kind == SYXV_KIND_QUOTE && self_name(ctx, left->quote, right->quote); \
    default: RUNTIME_ERROR("can not compare with lower than", ctx);                                           \
  }

bool syx__builtin_lower_than_comparator(Syx_Eval_Ctx *ctx, SyxV *left, SyxV *right);

bool syx__builtin_lower_than_comparator(Syx_Eval_Ctx *ctx, SyxV *left, SyxV *right) {
  syx__builtin_comparison_comparator(syx__builtin_lower_than_comparator, <);
}

/** Applies lower than between each consequence pairs. */
SyxV *syx_builtin_lower_than(Syx_Eval_Ctx *ctx, SyxV *arguments) {
  return syx__builtin_compare(ctx, arguments, syx__builtin_lower_than_comparator);
}

bool syx__builtin_lower_or_equal_comparator(Syx_Eval_Ctx *ctx, SyxV *left, SyxV *right);

bool syx__builtin_lower_or_equal_comparator(Syx_Eval_Ctx *ctx, SyxV *left, SyxV *right) {
  syx__builtin_comparison_comparator(syx__builtin_lower_or_equal_comparator, <=);
}

/** Applies lower or equal between each consequence pairs. */
SyxV *syx_builtin_lower_or_equal(Syx_Eval_Ctx *ctx, SyxV *arguments) {
  return syx__builtin_compare(ctx, arguments, syx__builtin_lower_or_equal_comparator);
}

bool syx__builtin_greater_than_comparator(Syx_Eval_Ctx *ctx, SyxV *left, SyxV *right);

bool syx__builtin_greater_than_comparator(Syx_Eval_Ctx *ctx, SyxV *left, SyxV *right) {
  syx__builtin_comparison_comparator(syx__builtin_greater_than_comparator, >);
}

/** Applies greater than between each consequence pairs. */
SyxV *syx_builtin_greater_than(Syx_Eval_Ctx *ctx, SyxV *arguments) {
  return syx__builtin_compare(ctx, arguments, syx__builtin_greater_than_comparator);
}

bool syx__builtin_greater_or_equal_comparator(Syx_Eval_Ctx *ctx, SyxV *left, SyxV *right);

bool syx__builtin_greater_or_equal_comparator(Syx_Eval_Ctx *ctx, SyxV *left, SyxV *right) {
  syx__builtin_comparison_comparator(syx__builtin_greater_or_equal_comparator, >=);
}

/** Applies greater or equal between each consequence pairs. */
SyxV *syx_builtin_greater_or_equal(Syx_Eval_Ctx *ctx, SyxV *arguments) {
  return syx__builtin_compare(ctx, arguments, syx__builtin_greater_or_equal_comparator);
}

bool syx__builtin_identity_comparator(Syx_Eval_Ctx *ctx, SyxV *left, SyxV *right);

bool syx__builtin_identity_comparator(Syx_Eval_Ctx *ctx, SyxV *left, SyxV *right) {
  UNUSED(ctx);
  if (left == right) return true;
  switch (left->kind) {
    case SYXV_KIND_NIL: return right->kind == SYXV_KIND_NIL;
    case SYXV_KIND_SYMBOL: return right->kind == SYXV_KIND_SYMBOL && strcmp(left->symbol.name, right->symbol.name) == 0;
    case SYXV_KIND_PAIR: return false; // should work on left == right level
    case SYXV_KIND_BOOL: return false; // should work on left == right level
    case SYXV_KIND_INTEGER: return right->kind == SYXV_KIND_INTEGER && left->integer == right->integer;
    case SYXV_KIND_FRACTIONAL: return right->kind == SYXV_KIND_FRACTIONAL && left->fractional == right->fractional;
    case SYXV_KIND_STRING: return false;   // should work on left == right level
    case SYXV_KIND_QUOTE: return false;    // should work on left == right level
    case SYXV_KIND_SPECIALF: return false; // should work on left == right level
    case SYXV_KIND_BUILTIN: return false;  // should work on left == right level
    case SYXV_KIND_CLOSURE: return false;  // should work on left == right level
  }
}

/** Applies identity check between each consequence pairs. */
SyxV *syx_builtin_identity(Syx_Eval_Ctx *ctx, SyxV *arguments) {
  return syx__builtin_compare(ctx, arguments, syx__builtin_identity_comparator);
}

#define syx__builtin_type_guard(name, kind_checks)            \
  SyxV *value = syxv_list_next_nullable(&arguments);          \
  if (value == NULL) RUNTIME_ERROR("argument expected", ctx); \
  return make_syxv_bool((kind_checks))

/** Type checks id first argument is nil. */
SyxV *syx_builtin_is_nil(Syx_Eval_Ctx *ctx, SyxV *arguments) { syx__builtin_type_guard(nil, value->kind == SYXV_KIND_NIL); }

/** Type checks id first argument is symbol. */
SyxV *syx_builtin_is_symbol(Syx_Eval_Ctx *ctx, SyxV *arguments) { syx__builtin_type_guard(symbol, value->kind == SYXV_KIND_SYMBOL); }

/** Type checks id first argument is pair. */
SyxV *syx_builtin_is_pair(Syx_Eval_Ctx *ctx, SyxV *arguments) { syx__builtin_type_guard(pair, value->kind == SYXV_KIND_PAIR); }

/** Type checks id first argument is list. */
SyxV *syx_builtin_is_list(Syx_Eval_Ctx *ctx, SyxV *arguments) {
  SyxV *value = syxv_list_next_nullable(&arguments);
  if (value == NULL) RUNTIME_ERROR("argument expected", ctx);
  SyxV *it = value;
  while (it->kind == SYXV_KIND_PAIR) it = it->pair.right;
  return make_syxv_bool(it->kind == SYXV_KIND_NIL);
}

/** Type checks id first argument is bool. */
SyxV *syx_builtin_is_bool(Syx_Eval_Ctx *ctx, SyxV *arguments) { syx__builtin_type_guard(bool, value->kind == SYXV_KIND_BOOL); }

/** Type checks id first argument is number. */
SyxV *syx_builtin_is_number(Syx_Eval_Ctx *ctx, SyxV *arguments) { syx__builtin_type_guard(number, value->kind == SYXV_KIND_INTEGER || value->kind == SYXV_KIND_FRACTIONAL); }

/** Type checks id first argument is integer. */
SyxV *syx_builtin_is_integer(Syx_Eval_Ctx *ctx, SyxV *arguments) { syx__builtin_type_guard(integer, value->kind == SYXV_KIND_INTEGER); }

/** Type checks id first argument is fractional. */
SyxV *syx_builtin_is_fractional(Syx_Eval_Ctx *ctx, SyxV *arguments) { syx__builtin_type_guard(fractional, value->kind == SYXV_KIND_FRACTIONAL); }

/** Type checks id first argument is string. */
SyxV *syx_builtin_is_string(Syx_Eval_Ctx *ctx, SyxV *arguments) { syx__builtin_type_guard(string, value->kind == SYXV_KIND_STRING); }

/** Type checks id first argument is quote. */
SyxV *syx_builtin_is_quote(Syx_Eval_Ctx *ctx, SyxV *arguments) { syx__builtin_type_guard(quote, value->kind == SYXV_KIND_QUOTE); }

/** Type checks id first argument is procedure. */
SyxV *syx_builtin_is_procedure(Syx_Eval_Ctx *ctx, SyxV *arguments) { syx__builtin_type_guard(procedure, value->kind == SYXV_KIND_SPECIALF || value->kind == SYXV_KIND_BUILTIN || value->kind == SYXV_KIND_CLOSURE); }

/** Type checks id first argument is special form. */
SyxV *syx_builtin_is_special_form(Syx_Eval_Ctx *ctx, SyxV *arguments) { syx__builtin_type_guard(special_form, value->kind == SYXV_KIND_SPECIALF); }

/** Type checks id first argument is builtin. */
SyxV *syx_builtin_is_builtin(Syx_Eval_Ctx *ctx, SyxV *arguments) { syx__builtin_type_guard(builtin, value->kind == SYXV_KIND_BUILTIN); }

/** Type checks id first argument is closure. */
SyxV *syx_builtin_is_closure(Syx_Eval_Ctx *ctx, SyxV *arguments) { syx__builtin_type_guard(closure, value->kind == SYXV_KIND_CLOSURE); }

typedef struct File_Constant {
  int fd;
  FILE *stream;
} File_Constant;

define_constant(Ht(const char *, File_Constant), FD_CONSTANTS) {
  FD_CONSTANTS->hasheq = ht_cstr_hasheq;
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
SyxV *syx_builtin_print(Syx_Eval_Ctx *ctx, SyxV *arguments) {
  FILE *f = parse_optional_file_descriptor(&arguments);
  bool first = true;
  syxv_list_for_each(argument, arguments) {
    String_View sv = syx_convert_to_string_v(ctx, argument);
    if (!first && !io_putc(f, ' ')) return make_syxv_nil();
    if (!io_puts(f, sv)) return make_syxv_nil();
    first = false;
  }
  return make_syxv_nil();
}

/** Prints arguments to file, adds new line to the end. */
SyxV *syx_builtin_println(Syx_Eval_Ctx *ctx, SyxV *arguments) {
  FILE *f = parse_optional_file_descriptor(&arguments);
  syxv_list_for_each(argument, arguments) {
    String_View sv = syx_convert_to_string_v(ctx, argument);
    if (!io_puts(f, sv)) return make_syxv_nil();
    if (!io_putc(f, ' ')) return make_syxv_nil();
  }
  if (!io_putc(f, '\n')) return make_syxv_nil();
  return make_syxv_nil();
}

ssize_t io_put_sv_diff(FILE *fd, String_View *base, String_View *offset) {
  ptrdiff_t diff = offset->data - base->data;
  if (diff <= 0) return 0;
  if (!io_puts_n(fd, base->data, offset->data - base->data)) return -1;
  *base = *offset;
  return diff;
}

/** Prints formatted string to file. */
SyxV *syx_builtin_printf(Syx_Eval_Ctx *ctx, SyxV *arguments) {
  FILE *f = parse_optional_file_descriptor(&arguments);
  String_View fmt = syx_convert_to_string_v(ctx, syxv_list_next(&arguments));
  String_View it = fmt;
  String_View str = fmt;
  for (; it.count; sv_chop_left(&it, 1)) {
    size_t format_size = 1;
    if (it.data[0] != '%') continue;
    // TODO: implement custom formats
    if (io_put_sv_diff(f, &str, &it) < 0) return make_syxv_nil();
    sv_chop_left(&it, format_size);
    str = it;
    SyxV *argument = syxv_list_next(&arguments);
    String_View sv = syx_convert_to_string_v(ctx, argument);
    if (!io_puts(f, sv)) return make_syxv_nil();
  }
  if (io_put_sv_diff(f, &str, &it) < 0) return make_syxv_nil();
  return make_syxv_nil();
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

  syx_env_define_cstr(env, "eq?", make_syxv_builtin(NULL, syx_builtin_identity));
  syx_env_define_cstr(env, "nil?", make_syxv_builtin(NULL, syx_builtin_is_nil));
  syx_env_define_cstr(env, "symbol?", make_syxv_builtin(NULL, syx_builtin_is_symbol));
  syx_env_define_cstr(env, "pair?", make_syxv_builtin(NULL, syx_builtin_is_pair));
  syx_env_define_cstr(env, "list?", make_syxv_builtin(NULL, syx_builtin_is_list));
  syx_env_define_cstr(env, "bool?", make_syxv_builtin(NULL, syx_builtin_is_bool));
  syx_env_define_cstr(env, "number?", make_syxv_builtin(NULL, syx_builtin_is_number));
  syx_env_define_cstr(env, "integer?", make_syxv_builtin(NULL, syx_builtin_is_integer));
  syx_env_define_cstr(env, "fractional?", make_syxv_builtin(NULL, syx_builtin_is_fractional));
  syx_env_define_cstr(env, "string?", make_syxv_builtin(NULL, syx_builtin_is_string));
  syx_env_define_cstr(env, "quote?", make_syxv_builtin(NULL, syx_builtin_is_quote));
  syx_env_define_cstr(env, "procedure?", make_syxv_builtin(NULL, syx_builtin_is_procedure));
  syx_env_define_cstr(env, "special-form?", make_syxv_builtin(NULL, syx_builtin_is_special_form));
  syx_env_define_cstr(env, "builtin?", make_syxv_builtin(NULL, syx_builtin_is_builtin));
  syx_env_define_cstr(env, "closure?", make_syxv_builtin(NULL, syx_builtin_is_closure));

  syx_env_define_cstr(env, "not", make_syxv_builtin(NULL, syx_builtin_not));

  syx_env_define_cstr(env, "print", make_syxv_builtin(NULL, syx_builtin_print));
  syx_env_define_cstr(env, "println", make_syxv_builtin(NULL, syx_builtin_println));
  syx_env_define_cstr(env, "printf", make_syxv_builtin(NULL, syx_builtin_printf));
}

#endif // SYX_EVAL_BUILTINS_IMPL
