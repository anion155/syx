#ifndef SYX_EVAL_BUILTINS_H
#define SYX_EVAL_BUILTINS_H

#include "syx_eval.h"
#include "syx_value.h"

void syx_env_define_builtins(Syx_Env *env);

#endif // SYX_EVAL_BUILTINS_H

#if defined(SYX_EVAL_BUILTINS_IMPL) && !defined(SYX_EVAL_BUILTINS_IMPL_C)
#define SYX_EVAL_BUILTINS_IMPL_C

#include <math.h>
#include <stdio.h>

/** Builtins */

/** Takes exactly 2 arguments and returns a pair (left . right). */
SyxV *syx_builtin_cons(Syx_Eval_Ctx *ctx, SyxV *arguments) {
  UNUSED(ctx);
  SyxV *left = syxv_list_next(&arguments);
  SyxV *right = syxv_list_next(&arguments);
  return make_syxv_pair(left, right);
}

/** Evaluates each argument and constructs a new list containing the results. */
SyxV *syx_builtin_list(Syx_Eval_Ctx *ctx, SyxV *arguments) {
  UNUSED(ctx);
  return arguments;
}

/** Returns the left element of a pair. */
SyxV *syx_builtin_car(Syx_Eval_Ctx *ctx, SyxV *arguments) {
  SyxV *list = syxv_list_next(&arguments);
  if (list->kind != SYXV_KIND_PAIR) RUNTIME_ERROR(ctx, "Pair expected as car argument");
  return list->pair.left;
}

/** Returns the right element of a pair. */
SyxV *syx_builtin_cdr(Syx_Eval_Ctx *ctx, SyxV *arguments) {
  SyxV *list = syxv_list_next(&arguments);
  if (list->kind != SYXV_KIND_PAIR) RUNTIME_ERROR(ctx, "Pair expected as cdr argument");
  return list->pair.right;
}

/** Calls a function with a list as its argument list. */
SyxV *syx_builtin_apply(Syx_Eval_Ctx *ctx, SyxV *arguments) {
  SyxV *fn = syxv_list_next(&arguments);
  SyxV *call = rc_acquire(make_syxv_pair(fn, NULL));
  SyxV **it = &call->pair.right;
  syxv_list_for_each(argument, arguments) {
    if ((*it)) RUNTIME_ERROR(ctx, "only last argument allowed to be pair with both values");
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
  (*it) = rc_acquire(make_syxv_nil());
  SyxV *result = syx_eval(ctx, call);
  syx_eval_early_exit(result);
  return result;
}

/** Applies a function to each element of a list and returns a new list of results. */
SyxV *syx_builtin_map(Syx_Eval_Ctx *ctx, SyxV *arguments) {
  SyxV *fn = syxv_list_next(&arguments);
  SyxV *list = syxv_list_next(&arguments);
  if (list->kind != SYXV_KIND_PAIR) RUNTIME_ERROR(ctx, "list expected");
  SyxV *results = NULL;
  syxv_list_map(item, list, &results) {
    SyxV *call = rc_acquire(make_syxv_pair(fn, make_syxv_pair(*item, make_syxv_nil())));
    *item = syx_eval(ctx, call);
    syx_eval_early_exit(*item, results, call);
    rc_acquire(*item);
    rc_release(call);
    rc_move(*item);
  }
  return results;
}

#define syx__builtin_operator(ctx, arguments, operator, nil)   \
  do {                                                         \
    Syx_Number value = {0};                                    \
    SyxV *first = syxv_list_next(&(arguments));                \
    switch (first->kind) {                                     \
      case SYXV_KIND_NUMBER: value = first->number; break;     \
      case SYXV_KIND_NIL: nil; return make_syxv_number(value); \
      default: syx_convert_to((ctx), first, &value);           \
    }                                                          \
    syxv_list_for_each(argument, (arguments)) {                \
      Syx_Number next = {0};                                   \
      switch (argument->kind) {                                \
        case SYXV_KIND_NUMBER: next = argument->number; break; \
        default: syx_convert_to((ctx), argument, &next);       \
      }                                                        \
      syx_number_operate(operator, value, next, &value);       \
    }                                                          \
    return make_syxv_number(value);                            \
  } while (0)

/** Concat arguments to string. */
SyxV *syx_builtin_concat(Syx_Eval_Ctx *ctx, SyxV *arguments) {
  String_Builder sb = {0};
  syxv_list_for_each(argument, arguments) {
    sb_append_converted_syxv(&sb, ctx, argument);
  }
  sb_append(&sb, 0);
  SyxV *string = make_syxv_string_n(sb.items, sb.count - 1);
  sb_free(sb);
  return string;
}

/** Sum of all arguments. */
SyxV *syx_builtin_summ(Syx_Eval_Ctx *ctx, SyxV *arguments) {
  SyxV *first = arguments->kind == SYXV_KIND_PAIR ? arguments->pair.left : NULL;
  if (first->kind == SYXV_KIND_STRING) return syx_builtin_concat(ctx, arguments);
  syx__builtin_operator(ctx, arguments, +, (value.kind = SYX_NUMBER_KIND_INTEGER, value.integer = 0));
}

/** Subtracts all arguments from first. */
SyxV *syx_builtin_sub(Syx_Eval_Ctx *ctx, SyxV *arguments) {
  syx__builtin_operator(ctx, arguments, -, RUNTIME_ERROR(ctx, "list of number expected"));
}

/** Multiplies all arguments. */
SyxV *syx_builtin_mul(Syx_Eval_Ctx *ctx, SyxV *arguments) {
  syx__builtin_operator(ctx, arguments, *, (value.kind = SYX_NUMBER_KIND_INTEGER, value.integer = 1));
}

/** Divide first argument by every next sequentialy. */
SyxV *syx_builtin_div(Syx_Eval_Ctx *ctx, SyxV *arguments) {
  syx__builtin_operator(ctx, arguments, /, RUNTIME_ERROR(ctx, "list of number expected"));
}

/** Returns `false` if argument is truthy, `true` if falsy. */
SyxV *syx_builtin_not(Syx_Eval_Ctx *ctx, SyxV *arguments) {
  bool value = {0};
  syx_convert_to(ctx, syxv_list_next(&arguments), &value);
  return make_syxv_bool(!value);
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
    case SYXV_KIND_NUMBER: return right->kind == SYXV_KIND_NUMBER && syx_number_equal(left->number, right->number);
    case SYXV_KIND_STRING: return right->kind == SYXV_KIND_STRING && sv_eq(left->string, right->string);
    case SYXV_KIND_QUOTE: return right->kind == SYXV_KIND_QUOTE && syx__builtin_equivalent_comparator(ctx, left->quote, right->quote);
    case SYXV_KIND_SPECIALF: return false; // should work on left == right level
    case SYXV_KIND_BUILTIN: return false;  // should work on left == right level
    case SYXV_KIND_CLOSURE: return false;  // should work on left == right level
    case SYXV_KIND_THROWN: UNREACHABLE("should never get throw object here");
    case SYXV_KIND_RETURN_VALUE: UNREACHABLE("should never get return value object here");
  }
}

/** Applies structural check between each consequence pairs. */
SyxV *syx_builtin_equivalent(Syx_Eval_Ctx *ctx, SyxV *arguments) {
  return syx__builtin_compare(ctx, arguments, syx__builtin_equivalent_comparator);
}

#define syx__builtin_comparison_comparator(self_name, operator)                              \
  switch (left->kind) {                                                                      \
    case SYXV_KIND_NUMBER: {                                                                 \
      right = rc_acquire(syx_convert_to_number(ctx, right));                                 \
      bool result = syx_number_value(left->number) operator syx_number_value(right->number); \
      rc_release(right);                                                                     \
      return result;                                                                         \
    }                                                                                        \
    case SYXV_KIND_STRING: {                                                                 \
      right = rc_acquire(syx_convert_to_string(ctx, right));                                 \
      bool result = strcmp(left->string.data, right->string.data) operator(0);               \
      rc_release(right);                                                                     \
      return result;                                                                         \
    }                                                                                        \
    case SYXV_KIND_PAIR: {                                                                   \
      if (right->kind != SYXV_KIND_PAIR) {                                                   \
        RUNTIME_ERROR(ctx, "can not compare with " STRINGIFY(operator));                     \
      }                                                                                      \
      return (                                                                               \
          self_name(ctx, left->pair.left, right->pair.left) &&                               \
          self_name(ctx, left->pair.right, right->pair.right));                              \
    }                                                                                        \
    case SYXV_KIND_QUOTE: {                                                                  \
      if (right->kind != SYXV_KIND_QUOTE) {                                                  \
        RUNTIME_ERROR(ctx, "can not compare with " STRINGIFY(operator));                     \
      }                                                                                      \
      return self_name(ctx, left->quote, right->quote);                                      \
    }                                                                                        \
    default: RUNTIME_ERROR(ctx, "can not compare with " STRINGIFY(operator));                \
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
    case SYXV_KIND_NUMBER: return right->kind == SYXV_KIND_NUMBER && syx_number_identity_equal(left->number, right->number);
    case SYXV_KIND_STRING: return false;   // should work on left == right level
    case SYXV_KIND_QUOTE: return false;    // should work on left == right level
    case SYXV_KIND_SPECIALF: return false; // should work on left == right level
    case SYXV_KIND_BUILTIN: return false;  // should work on left == right level
    case SYXV_KIND_CLOSURE: return false;  // should work on left == right level
    case SYXV_KIND_THROWN: UNREACHABLE("should never get thrown value here");
    case SYXV_KIND_RETURN_VALUE: UNREACHABLE("should never get return value object here");
  }
}

/** Applies identity check between each consequence pairs. */
SyxV *syx_builtin_identity(Syx_Eval_Ctx *ctx, SyxV *arguments) {
  return syx__builtin_compare(ctx, arguments, syx__builtin_identity_comparator);
}

#define syx__builtin_type_guard(kind_checks)                  \
  SyxV *value = syxv_list_next_nullable(&arguments);          \
  if (value == NULL) RUNTIME_ERROR(ctx, "argument expected"); \
  return make_syxv_bool((kind_checks))

/** Type checks id first argument is nil. */
SyxV *syx_builtin_is_nil(Syx_Eval_Ctx *ctx, SyxV *arguments) { syx__builtin_type_guard(value->kind == SYXV_KIND_NIL); }

/** Type checks id first argument is symbol. */
SyxV *syx_builtin_is_symbol(Syx_Eval_Ctx *ctx, SyxV *arguments) { syx__builtin_type_guard(value->kind == SYXV_KIND_SYMBOL); }

/** Type checks id first argument is pair. */
SyxV *syx_builtin_is_pair(Syx_Eval_Ctx *ctx, SyxV *arguments) { syx__builtin_type_guard(value->kind == SYXV_KIND_PAIR); }

/** Type checks id first argument is list. */
SyxV *syx_builtin_is_list(Syx_Eval_Ctx *ctx, SyxV *arguments) {
  SyxV *value = syxv_list_next_nullable(&arguments);
  if (value == NULL) RUNTIME_ERROR(ctx, "argument expected");
  SyxV *it = value;
  while (it->kind == SYXV_KIND_PAIR) it = it->pair.right;
  return make_syxv_bool(it->kind == SYXV_KIND_NIL);
}

/** Type checks id first argument is bool. */
SyxV *syx_builtin_is_bool(Syx_Eval_Ctx *ctx, SyxV *arguments) { syx__builtin_type_guard(value->kind == SYXV_KIND_BOOL); }

/** Type checks id first argument is number. */
SyxV *syx_builtin_is_number(Syx_Eval_Ctx *ctx, SyxV *arguments) { syx__builtin_type_guard(value->kind == SYXV_KIND_NUMBER); }

/** Type checks id first argument is integer. */
SyxV *syx_builtin_is_integer(Syx_Eval_Ctx *ctx, SyxV *arguments) { syx__builtin_type_guard(value->kind == SYXV_KIND_NUMBER && value->number.kind == SYX_NUMBER_KIND_INTEGER); }

/** Type checks id first argument is fractional. */
SyxV *syx_builtin_is_fractional(Syx_Eval_Ctx *ctx, SyxV *arguments) { syx__builtin_type_guard(value->kind == SYXV_KIND_NUMBER && value->number.kind == SYX_NUMBER_KIND_FRACTIONAL); }

/** Type checks id first argument is string. */
SyxV *syx_builtin_is_string(Syx_Eval_Ctx *ctx, SyxV *arguments) { syx__builtin_type_guard(value->kind == SYXV_KIND_STRING); }

/** Type checks id first argument is quote. */
SyxV *syx_builtin_is_quote(Syx_Eval_Ctx *ctx, SyxV *arguments) { syx__builtin_type_guard(value->kind == SYXV_KIND_QUOTE); }

/** Type checks id first argument is procedure. */
SyxV *syx_builtin_is_procedure(Syx_Eval_Ctx *ctx, SyxV *arguments) { syx__builtin_type_guard(value->kind == SYXV_KIND_SPECIALF || value->kind == SYXV_KIND_BUILTIN || value->kind == SYXV_KIND_CLOSURE); }

/** Type checks id first argument is special form. */
SyxV *syx_builtin_is_special_form(Syx_Eval_Ctx *ctx, SyxV *arguments) { syx__builtin_type_guard(value->kind == SYXV_KIND_SPECIALF); }

/** Type checks id first argument is builtin. */
SyxV *syx_builtin_is_builtin(Syx_Eval_Ctx *ctx, SyxV *arguments) { syx__builtin_type_guard(value->kind == SYXV_KIND_BUILTIN); }

/** Type checks id first argument is closure. */
SyxV *syx_builtin_is_closure(Syx_Eval_Ctx *ctx, SyxV *arguments) { syx__builtin_type_guard(value->kind == SYXV_KIND_CLOSURE); }

typedef struct File_Constant {
  int fd;
  FILE *stream;
} File_Constant;

define_constant(Ht(SyxV_Symbol *, File_Constant), FD_CONSTANTS) {
  FD_CONSTANTS->hasheq = ht_syxv_symbol_hasheq;
  *ht_put(FD_CONSTANTS, &(rc_acquire(make_syxv_symbol_cstr("stdout")))->symbol) = (File_Constant){.fd = STDOUT_FILENO, .stream = stdout};
  *ht_put(FD_CONSTANTS, &(rc_acquire(make_syxv_symbol_cstr("stderr")))->symbol) = (File_Constant){.fd = STDERR_FILENO, .stream = stderr};
  *ht_put(FD_CONSTANTS, &(rc_acquire(make_syxv_symbol_cstr("stdin")))->symbol) = (File_Constant){.fd = STDIN_FILENO, .stream = stdin};
}

FILE *parse_optional_file_descriptor(SyxV **arguments) {
  FILE *f = stdout;
  if ((*arguments)->kind != SYXV_KIND_PAIR) return f;
  if ((*arguments)->pair.left->kind != SYXV_KIND_SYMBOL) return f;
  File_Constant *constant = ht_find(FD_CONSTANTS(), &(*arguments)->pair.left->symbol);
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
    syx_string_t sb = {0};
    syx_convert_to(ctx, argument, &sb);
    if (!first && !io_putc(f, ' ')) return (sb_free(sb), make_syxv_nil());
    if (!io_puts(f, sb_to_sv(sb))) return (sb_free(sb), make_syxv_nil());
    sb_free(sb);
    first = false;
  }
  return make_syxv_nil();
}

/** Flash file descriptor. */
SyxV *syx_builtin_print_flash(Syx_Eval_Ctx *ctx, SyxV *arguments) {
  UNUSED(ctx);
  FILE *f = parse_optional_file_descriptor(&arguments);
  io_flash(f);
  return make_syxv_nil();
}

/** Prints arguments to file, adds new line to the end. */
SyxV *syx_builtin_println(Syx_Eval_Ctx *ctx, SyxV *arguments) {
  FILE *f = parse_optional_file_descriptor(&arguments);
  bool first = true;
  syxv_list_for_each(argument, arguments) {
    syx_string_t sb = {0};
    syx_convert_to(ctx, argument, &sb);
    if (!first && !io_putc(f, ' ')) return (sb_free(sb), make_syxv_nil());
    if (!io_puts(f, sb_to_sv(sb))) return (sb_free(sb), make_syxv_nil());
    sb_free(sb);
    first = false;
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
  syx_string_t fmt = {0};
  syx_convert_to(ctx, syxv_list_next(&arguments), &fmt);
  String_View it = sb_to_sv(fmt);
  String_View str = sb_to_sv(fmt);
  for (; it.count; sv_chop_left(&it, 1)) {
    size_t format_size = 1;
    if (it.data[0] != '%') continue;
    // TODO: implement custom formats
    if (io_put_sv_diff(f, &str, &it) < 0) return make_syxv_nil();
    sv_chop_left(&it, format_size);
    str = it;
    SyxV *argument = syxv_list_next(&arguments);
    syx_string_t sb = {0};
    syx_convert_to(ctx, argument, &sb);
    if (!io_puts(f, sb_to_sv(sb))) return make_syxv_nil();
    sb_free(sb);
  }
  if (io_put_sv_diff(f, &str, &it) < 0) return make_syxv_nil();
  sb_free(fmt);
  return make_syxv_nil();
}

void syx_env_define_builtins(Syx_Env *env) {
  /** Builtins */
  syx_env_define_cstr(env, "cons", make_syxv_builtin(NULL, syx_builtin_cons));
  syx_env_define_cstr(env, "list", make_syxv_builtin(NULL, syx_builtin_list));
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
  syx_env_define_cstr(env, "print-flash", make_syxv_builtin(NULL, syx_builtin_print_flash));
  syx_env_define_cstr(env, "println", make_syxv_builtin(NULL, syx_builtin_println));
  syx_env_define_cstr(env, "printf", make_syxv_builtin(NULL, syx_builtin_printf));
}

#endif // SYX_EVAL_BUILTINS_IMPL
