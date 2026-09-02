#ifndef SYX_EVAL_BUILTINS_H
#define SYX_EVAL_BUILTINS_H

#include <syx_new/syx_eval.h>
#include <syx_new/syx_value.h>

void syx_env_define_builtins(Syx_Env *env);

#endif // SYX_EVAL_BUILTINS_H

#if defined(SYX_EVAL_BUILTINS_IMPL) && !defined(SYX_EVAL_BUILTINS_IMPL_C)
#define SYX_EVAL_BUILTINS_IMPL_C

#include <math.h>
#include <stdio.h>

#define GENERAL_UTILS_IMPL
#include <general_utils.h>

/** Builtins */

/** Takes exactly 2 arguments and returns a pair (left . right). */
Syx_Value *syx_builtin_cons(Syx_Eval_Ctx *ctx, Syx_Pair *arguments) {
  UNUSED(ctx);
  Syx_Value *left = syx_list_next(&arguments);
  Syx_Value *right = syx_list_next(&arguments);
  return make_syx_value_pair(left, right);
}

/** Evaluates each argument and constructs a new list containing the results. */
Syx_Value *syx_builtin_list(Syx_Eval_Ctx *ctx, Syx_Pair *arguments) {
  UNUSED(ctx);
  return syx_value_from_pair(arguments);
}

/** Returns the left element of a pair. */
Syx_Value *syx_builtin_car(Syx_Eval_Ctx *ctx, Syx_Pair *arguments) {
  Syx_Value *list = syx_list_next(&arguments);
  if (list->kind != SYX_VALUE_KIND_PAIR) SYX_EVAL_THROW(ctx, "pair expected as car argument");
  return list->pair->left;
}

/** Returns the right element of a pair. */
Syx_Value *syx_builtin_cdr(Syx_Eval_Ctx *ctx, Syx_Pair *arguments) {
  Syx_Value *list = syx_list_next(&arguments);
  if (list->kind != SYX_VALUE_KIND_PAIR) SYX_EVAL_THROW(ctx, "Pair expected as cdr argument");
  return list->pair->right;
}

/** Calls a function with a list as its argument list. */
Syx_Value *syx_builtin_apply(Syx_Eval_Ctx *ctx, Syx_Pair *arguments) {
  Syx_Value *fn = syx_list_next(&arguments);
  Syx_Value *call = rc_acquire(make_syx_value_pair(fn, NULL));
  Syx_Value **it = &call->pair->right;
  syx_list_for_each(arguments, argument) {
    if ((*it)) SYX_EVAL_THROW(ctx, "only last argument allowed to be pair with both values");
    if (argument->kind == SYX_VALUE_KIND_PAIR) {
      (*it) = argument;
      while ((*it)->kind == SYX_VALUE_KIND_PAIR) it = &(*it)->pair->right;
      if ((*it)->kind == SYX_VALUE_KIND_NIL) {
        rc_release((*it));
        (*it) = NULL;
      }
      continue;
    }
    (*it) = rc_acquire(make_syx_value_pair(NULL, NULL));
    (*it)->pair->left = argument;
    it = &(*it)->pair->right;
  }
  (*it) = rc_acquire(syx_value_nil());
  Syx_Value *result = rc_acquire(syx_eval(ctx, call));
  syx_value_early_exit(result);
  return rc_move(result);
}

/** Applies a function to each element of a list and returns a new list of results. */
Syx_Value *syx_builtin_map(Syx_Eval_Ctx *ctx, Syx_Pair *arguments) {
  Syx_Value *fn = syx_list_next(&arguments);
  Syx_Value *list = syx_list_next(&arguments);
  if (list->kind != SYX_VALUE_KIND_PAIR) SYX_EVAL_THROW(ctx, "list expected");
  Syx_Value *results = NULL;
  syx_list_map(list->pair, item, &results) {
    Syx_Value *call = rc_acquire(make_syx_value_pair(fn, make_syx_value_pair(*item, syx_value_nil())));
    *item = rc_acquire(syx_eval(ctx, call));
    syx_value_early_exit(*item, results, call);
    rc_release(call);
    rc_move(*item);
  }
  return results;
}

#define syx__builtin_operator(ctx, arguments, operator, nil) ({        \
  Syx_Number value = {0};                                              \
  Syx_Value *first = syx_list_next(&(arguments));                      \
  switch (first->kind) {                                               \
    case SYX_VALUE_KIND_NUMBER: value = *first->number; break;         \
    case SYX_VALUE_KIND_NIL: nil; return make_syx_value_number(value); \
    default: syx_convert_to((ctx), first, &value);                     \
  }                                                                    \
  syx_list_for_each((arguments), argument) {                           \
    Syx_Number next = {0};                                             \
    switch (argument->kind) {                                          \
      case SYX_VALUE_KIND_NUMBER: next = *argument->number; break;     \
      default: syx_convert_to((ctx), argument, &next);                 \
    }                                                                  \
    value = syx_number_operate(&value, operator, & next);              \
  }                                                                    \
  return make_syx_value_number(value);                                 \
})

// /** Concat arguments to string. */
// Syx_Value *syx_builtin_concat(Syx_Eval_Ctx *ctx, Syx_Pair *arguments) {
//   String_Builder sb = {0};
//   syx_list_for_each(arguments, argument) {
//     sb_append_converted_syxv(&sb, ctx, argument);
//   }
//   sb_append(&sb, 0);
//   Syx_Value *string = make_syxv_string_n(sb.items, sb.count - 1);
//   sb_free(sb);
//   return string;
// }

/** Sum of all arguments. */
Syx_Value *syx_builtin_summ(Syx_Eval_Ctx *ctx, Syx_Pair *arguments) {
  // Syx_Value *first = arguments ? arguments->left : NULL;
  // if (first && first->kind == SYX_VALUE_KIND_STRING) return syx_builtin_concat(ctx, arguments);
  syx__builtin_operator(ctx, arguments, +, (value.kind = SYX_NUMBER_KIND_INTEGER, value.integer = 0));
}

/** Subtracts all arguments from first. */
Syx_Value *syx_builtin_sub(Syx_Eval_Ctx *ctx, Syx_Pair *arguments) {
  syx__builtin_operator(ctx, arguments, -, SYX_EVAL_THROW(ctx, "list of number expected"));
}

/** Multiplies all arguments. */
Syx_Value *syx_builtin_mul(Syx_Eval_Ctx *ctx, Syx_Pair *arguments) {
  syx__builtin_operator(ctx, arguments, *, (value.kind = SYX_NUMBER_KIND_INTEGER, value.integer = 1));
}

/** Divide first argument by every next sequentialy. */
Syx_Value *syx_builtin_div(Syx_Eval_Ctx *ctx, Syx_Pair *arguments) {
  syx__builtin_operator(ctx, arguments, /, SYX_EVAL_THROW(ctx, "list of number expected"));
}

#undef syx__builtin_operator

/** Returns `false` if argument is truthy, `true` if falsy. */
Syx_Value *syx_builtin_not(Syx_Eval_Ctx *ctx, Syx_Pair *arguments) {
  bool value = {0};
  syx_convert_to(ctx, syx_list_next(&arguments), &value);
  return syx_value_bool(!value);
}

typedef bool (*Syx_Compare)(Syx_Eval_Ctx *ctx, Syx_Value *left, Syx_Value *right);

Syx_Value *syx__builtin_compare(Syx_Eval_Ctx *ctx, Syx_Pair *arguments, Syx_Compare compare) {
  Syx_Value *previous = syx_list_next(&arguments);
  syx_list_for_each(arguments, argument) {
    if (!compare(ctx, previous, argument)) return syx_value_bool(false);
    previous = argument;
  }
  return syx_value_bool(true);
}

bool syx__builtin_equivalent_comparator(Syx_Eval_Ctx *ctx, Syx_Value *left, Syx_Value *right) {
  if (left == right) return true;
  switch (left->kind) {
    case SYX_VALUE_KIND_NIL: return right->kind == SYX_VALUE_KIND_NIL;
    case SYX_VALUE_KIND_BOOL_TRUE: return right->kind == SYX_VALUE_KIND_BOOL_TRUE;
    case SYX_VALUE_KIND_BOOL_FALSE: return right->kind == SYX_VALUE_KIND_BOOL_FALSE;
    case SYX_VALUE_KIND_EXIT: UNREACHABLE("should never get exit value here");
    case SYX_VALUE_KIND_PAIR: return (
        right->kind == SYX_VALUE_KIND_PAIR &&
        syx__builtin_equivalent_comparator(ctx, left->pair->left, right->pair->left) &&
        syx__builtin_equivalent_comparator(ctx, left->pair->right, right->pair->right));
    case SYX_VALUE_KIND_SPECIAL: return (
        right->kind == SYX_VALUE_KIND_SPECIAL &&
        left->special->kind == right->special->kind &&
        syx__builtin_equivalent_comparator(ctx, left->special->value, right->special->value));
    case SYX_VALUE_KIND_SYMBOL: return false; // should work on left == right level
    case SYX_VALUE_KIND_NUMBER: return right->kind == SYX_VALUE_KIND_NUMBER && (syx_number_get(left->number) == syx_number_get(right->number));
    case SYX_VALUE_KIND_STRING: return right->kind == SYX_VALUE_KIND_STRING && sv_like_eq(*left->string, *right->string);
    case SYX_VALUE_KIND_CLOSURE: return false; // should work on left == right level
  }
  // SYX_VALUE_KIND_OBJECT,
  // SYX_VALUE_KIND_NATIVE,
}

/** Applies structural check between each consequence pairs. */
Syx_Value *syx_builtin_equivalent(Syx_Eval_Ctx *ctx, Syx_Pair *arguments) {
  return syx__builtin_compare(ctx, arguments, syx__builtin_equivalent_comparator);
}

#define syx__builtin_comparison_comparator(self_name, operator)                          \
  switch (left->kind) {                                                                  \
    case SYX_VALUE_KIND_NUMBER: {                                                        \
      right = rc_acquire(syx_convert_to_number(ctx, right));                             \
      bool result = syx_number_get(left->number) operator syx_number_get(right->number); \
      rc_release(right);                                                                 \
      return result;                                                                     \
    }                                                                                    \
    case SYX_VALUE_KIND_STRING: {                                                        \
      right = rc_acquire(syx_convert_to_string(ctx, right));                             \
      bool result = strcmp(left->string->data, right->string->data) operator(0);         \
      rc_release(right);                                                                 \
      return result;                                                                     \
    }                                                                                    \
    case SYX_VALUE_KIND_PAIR: {                                                          \
      if (right->kind != SYX_VALUE_KIND_PAIR) {                                          \
        SYX_EVAL_THROW(ctx, "can not compare with " STRINGIFY(operator));                \
      }                                                                                  \
      return (                                                                           \
          self_name(ctx, left->pair->left, right->pair->left) &&                         \
          self_name(ctx, left->pair->right, right->pair->right));                        \
    }                                                                                    \
    default: SYX_EVAL_THROW(ctx, "can not compare with " STRINGIFY(operator));           \
  }

bool syx__builtin_lower_than_comparator(Syx_Eval_Ctx *ctx, Syx_Value *left, Syx_Value *right);

bool syx__builtin_lower_than_comparator(Syx_Eval_Ctx *ctx, Syx_Value *left, Syx_Value *right) {
  syx__builtin_comparison_comparator(syx__builtin_lower_than_comparator, <);
}

/** Applies lower than between each consequence pairs. */
Syx_Value *syx_builtin_lower_than(Syx_Eval_Ctx *ctx, Syx_Pair *arguments) {
  return syx__builtin_compare(ctx, arguments, syx__builtin_lower_than_comparator);
}

bool syx__builtin_lower_or_equal_comparator(Syx_Eval_Ctx *ctx, Syx_Value *left, Syx_Value *right);

bool syx__builtin_lower_or_equal_comparator(Syx_Eval_Ctx *ctx, Syx_Value *left, Syx_Value *right) {
  syx__builtin_comparison_comparator(syx__builtin_lower_or_equal_comparator, <=);
}

/** Applies lower or equal between each consequence pairs. */
Syx_Value *syx_builtin_lower_or_equal(Syx_Eval_Ctx *ctx, Syx_Pair *arguments) {
  return syx__builtin_compare(ctx, arguments, syx__builtin_lower_or_equal_comparator);
}

bool syx__builtin_greater_than_comparator(Syx_Eval_Ctx *ctx, Syx_Value *left, Syx_Value *right);

bool syx__builtin_greater_than_comparator(Syx_Eval_Ctx *ctx, Syx_Value *left, Syx_Value *right) {
  syx__builtin_comparison_comparator(syx__builtin_greater_than_comparator, >);
}

/** Applies greater than between each consequence pairs. */
Syx_Value *syx_builtin_greater_than(Syx_Eval_Ctx *ctx, Syx_Pair *arguments) {
  return syx__builtin_compare(ctx, arguments, syx__builtin_greater_than_comparator);
}

bool syx__builtin_greater_or_equal_comparator(Syx_Eval_Ctx *ctx, Syx_Value *left, Syx_Value *right);

bool syx__builtin_greater_or_equal_comparator(Syx_Eval_Ctx *ctx, Syx_Value *left, Syx_Value *right) {
  syx__builtin_comparison_comparator(syx__builtin_greater_or_equal_comparator, >=);
}

/** Applies greater or equal between each consequence pairs. */
Syx_Value *syx_builtin_greater_or_equal(Syx_Eval_Ctx *ctx, Syx_Pair *arguments) {
  return syx__builtin_compare(ctx, arguments, syx__builtin_greater_or_equal_comparator);
}

#undef syx__builtin_comparison_comparator

bool syx__builtin_identity_comparator(Syx_Eval_Ctx *ctx, Syx_Value *left, Syx_Value *right);

bool syx__builtin_identity_comparator(Syx_Eval_Ctx *ctx, Syx_Value *left, Syx_Value *right) {
  UNUSED(ctx);
  if (left == right) return true;
  switch (left->kind) {
    case SYX_VALUE_KIND_NIL: return right->kind == SYX_VALUE_KIND_NIL;
    case SYX_VALUE_KIND_BOOL_TRUE: return right->kind == SYX_VALUE_KIND_BOOL_TRUE;
    case SYX_VALUE_KIND_BOOL_FALSE: return right->kind == SYX_VALUE_KIND_BOOL_FALSE;
    case SYX_VALUE_KIND_EXIT: UNREACHABLE("should never get exit value here");
    case SYX_VALUE_KIND_PAIR: return false;    // should work on left == right level
    case SYX_VALUE_KIND_SPECIAL: return false; // should work on left == right level
    case SYX_VALUE_KIND_SYMBOL: return right->kind == SYX_VALUE_KIND_SYMBOL && strcmp(left->symbol->data, right->symbol->data) == 0;
    case SYX_VALUE_KIND_NUMBER: return (
        right->kind == SYX_VALUE_KIND_NUMBER &&
        left->number->kind == right->number->kind &&
        syx_number_get(left->number) == syx_number_get(right->number));
    case SYX_VALUE_KIND_STRING: return false;  // should work on left == right level
    case SYX_VALUE_KIND_CLOSURE: return false; // should work on left == right level
  }
  // SYX_VALUE_KIND_OBJECT,
  // SYX_VALUE_KIND_NATIVE,
}

/** Applies identity check between each consequence pairs. */
Syx_Value *syx_builtin_identity(Syx_Eval_Ctx *ctx, Syx_Pair *arguments) {
  return syx__builtin_compare(ctx, arguments, syx__builtin_identity_comparator);
}

#define syx__builtin_type_guard(kind_checks)                   \
  Syx_Value *value = syx_list_next_nullable(&arguments);       \
  if (value == NULL) SYX_EVAL_THROW(ctx, "argument expected"); \
  return syx_value_bool((kind_checks))

/** Type checks if first argument is nil. */
Syx_Value *syx_builtin_is_nil(Syx_Eval_Ctx *ctx, Syx_Pair *arguments) { syx__builtin_type_guard(value->kind == SYX_VALUE_KIND_NIL); }

/** Type checks if first argument is bool. */
Syx_Value *syx_builtin_is_bool(Syx_Eval_Ctx *ctx, Syx_Pair *arguments) { syx__builtin_type_guard(value->kind == SYX_VALUE_KIND_BOOL_TRUE || value->kind == SYX_VALUE_KIND_BOOL_FALSE); }

/** Type checks if first argument is pair. */
Syx_Value *syx_builtin_is_pair(Syx_Eval_Ctx *ctx, Syx_Pair *arguments) { syx__builtin_type_guard(value->kind == SYX_VALUE_KIND_PAIR); }

/** Type checks if first argument is list. */
Syx_Value *syx_builtin_is_list(Syx_Eval_Ctx *ctx, Syx_Pair *arguments) {
  Syx_Value *value = syx_list_next_nullable(&arguments);
  if (value == NULL) SYX_EVAL_THROW(ctx, "argument expected");
  Syx_Value *it = value;
  while (it->kind == SYX_VALUE_KIND_PAIR) it = it->pair->right;
  return syx_value_bool(it->kind == SYX_VALUE_KIND_NIL);
}

/** Type checks if first argument is special. */
Syx_Value *syx_builtin_is_special(Syx_Eval_Ctx *ctx, Syx_Pair *arguments) { syx__builtin_type_guard(value->kind == SYX_VALUE_KIND_SPECIAL); }

/** Type checks if first argument is special quote. */
Syx_Value *syx_builtin_is_special_quote(Syx_Eval_Ctx *ctx, Syx_Pair *arguments) { syx__builtin_type_guard(value->kind == SYX_VALUE_KIND_SPECIAL && value->special->kind == SYX_SPECIAL_KIND_QUOTE); }

/** Type checks if first argument is special unquote. */
Syx_Value *syx_builtin_is_special_unquote(Syx_Eval_Ctx *ctx, Syx_Pair *arguments) { syx__builtin_type_guard(value->kind == SYX_VALUE_KIND_SPECIAL && value->special->kind == SYX_SPECIAL_KIND_COLON); }

/** Type checks if first argument is symbol. */
Syx_Value *syx_builtin_is_symbol(Syx_Eval_Ctx *ctx, Syx_Pair *arguments) { syx__builtin_type_guard(value->kind == SYX_VALUE_KIND_SYMBOL); }

/** Type checks if first argument is number. */
Syx_Value *syx_builtin_is_number(Syx_Eval_Ctx *ctx, Syx_Pair *arguments) { syx__builtin_type_guard(value->kind == SYX_VALUE_KIND_NUMBER); }

/** Type checks if first argument is integer. */
Syx_Value *syx_builtin_is_integer(Syx_Eval_Ctx *ctx, Syx_Pair *arguments) { syx__builtin_type_guard(value->kind == SYX_VALUE_KIND_NUMBER && value->number->kind == SYX_NUMBER_KIND_INTEGER); }

/** Type checks if first argument is fractional. */
Syx_Value *syx_builtin_is_fractional(Syx_Eval_Ctx *ctx, Syx_Pair *arguments) { syx__builtin_type_guard(value->kind == SYX_VALUE_KIND_NUMBER && value->number->kind == SYX_NUMBER_KIND_FRACTIONAL); }

/** Type checks if first argument is string. */
Syx_Value *syx_builtin_is_string(Syx_Eval_Ctx *ctx, Syx_Pair *arguments) { syx__builtin_type_guard(value->kind == SYX_VALUE_KIND_STRING); }

// /** Type checks if first argument is boxed value. */
// Syx_Value *syx_builtin_is_boxed(Syx_Eval_Ctx *ctx, Syx_Pair *arguments) {
//   Syx_Value *constructor = syx_list_next_nullable(&arguments);
//   Syx_Value *value = syx_list_next_nullable(&arguments);
//   if (value == NULL) (value = constructor, constructor = NULL);
//   if (value == NULL) SYX_EVAL_THROW(ctx, "argument expected");
//   if (value->kind != SYXV_KIND_BOXED) return syx_value_bool(false);
//   if (constructor == NULL) return syx_value_bool(true);
//   if (constructor->kind != SYXV_KIND_CONSTRUCTOR) SYX_EVAL_THROW(ctx, "constructor expected");
//   return syx_value_bool(value->boxed->typeinfo == constructor->constructor.typeinfo);
// }

/** Type checks if first argument is closure. */
Syx_Value *syx_builtin_is_closure(Syx_Eval_Ctx *ctx, Syx_Pair *arguments) { syx__builtin_type_guard(value->kind == SYX_VALUE_KIND_CLOSURE); }

/** Type checks if first argument is special form. */
Syx_Value *syx_builtin_is_special_form(Syx_Eval_Ctx *ctx, Syx_Pair *arguments) { syx__builtin_type_guard(value->kind == SYX_VALUE_KIND_CLOSURE && value->closure->kind == SYX_CLOSURE_KIND_SPECIALF); }

/** Type checks if first argument is builtin. */
Syx_Value *syx_builtin_is_builtin(Syx_Eval_Ctx *ctx, Syx_Pair *arguments) { syx__builtin_type_guard(value->kind == SYX_VALUE_KIND_CLOSURE && value->closure->kind == SYX_CLOSURE_KIND_BUILTIN); }

/** Type checks if first argument is lambda. */
Syx_Value *syx_builtin_is_lambda(Syx_Eval_Ctx *ctx, Syx_Pair *arguments) { syx__builtin_type_guard(value->kind == SYX_VALUE_KIND_CLOSURE && value->closure->kind == SYX_CLOSURE_KIND_LAMBDA); }

#undef syx__builtin_type_guard

typedef struct File_Constant {
  int fd;
  FILE *stream;
} File_Constant;

syx_define_constant(Ht(Syx_Symbol *, File_Constant), FD_CONSTANTS) {
  FD_CONSTANTS->hasheq = ht_syx_symbol_hasheq;
  *ht_put(FD_CONSTANTS, make_syx_value_symbol_strlit("stdout")->symbol) = (File_Constant){.fd = STDOUT_FILENO, .stream = stdout};
  *ht_put(FD_CONSTANTS, make_syx_value_symbol_strlit("stderr")->symbol) = (File_Constant){.fd = STDERR_FILENO, .stream = stderr};
  *ht_put(FD_CONSTANTS, make_syx_value_symbol_strlit("stdin")->symbol) = (File_Constant){.fd = STDIN_FILENO, .stream = stdin};
  ht_foreach(symbol, FD_CONSTANTS) {
    rc_acquire(syx_value_from_symbol(ht_key(FD_CONSTANTS, symbol)));
  }
}

FILE *parse_optional_file_descriptor(Syx_Pair **arguments) {
  FILE *f = stdout;
  if ((*arguments)->left->kind != SYX_VALUE_KIND_SYMBOL) return f;
  File_Constant *constant = ht_find(FD_CONSTANTS(), (*arguments)->left->symbol);
  if (!constant) return f;
  syx_list_next_nullable(arguments);
  f = constant->stream;
  // TODO: pass fd as value not as constant
  return f;
}

// /** Prints arguments to file. */
// Syx_Value *syx_builtin_print(Syx_Eval_Ctx *ctx, Syx_Pair *arguments) {
//   FILE *f = parse_optional_file_descriptor(&arguments);
//   bool first = true;
//   syx_list_for_each(arguments, argument) {
//     syx_string sb = {0};
//     sb_copy_from_sv(sv_from_like(*argument->string));
//     syx_convert_to(ctx, argument, &sb);
//     if (!first && !io_putc(f, ' ')) return (sb_free(sb), syx_value_nil());
//     if (!io_puts(f, sb_to_sv(sb))) return (sb_free(sb), syx_value_nil());
//     sb_free(sb);
//     first = false;
//   }
//   return syx_value_nil();
// }

// /** Flash file descriptor. */
// Syx_Value *syx_builtin_print_flash(Syx_Eval_Ctx *ctx, Syx_Pair *arguments) {
//   UNUSED(ctx);
//   FILE *f = parse_optional_file_descriptor(&arguments);
//   io_flash(f);
//   return syx_value_nil();
// }

// /** Prints arguments to file, adds new line to the end. */
// Syx_Value *syx_builtin_println(Syx_Eval_Ctx *ctx, Syx_Pair *arguments) {
//   FILE *f = parse_optional_file_descriptor(&arguments);
//   bool first = true;
//   syx_list_for_each(arguments, argument) {
//     syx_string sb = {0};
//     syx_convert_to(ctx, argument, &sb);
//     if (!first && !io_putc(f, ' ')) return (sb_free(sb), syx_value_nil());
//     if (!io_puts(f, sb_to_sv(sb))) return (sb_free(sb), syx_value_nil());
//     sb_free(sb);
//     first = false;
//   }
//   if (!io_putc(f, '\n')) return syx_value_nil();
//   return syx_value_nil();
// }

// ssize_t io_put_sv_diff(FILE *fd, String_View *base, String_View *offset) {
//   ptrdiff_t diff = offset->data - base->data;
//   if (diff <= 0) return 0;
//   if (!io_puts_n(fd, base->data, offset->data - base->data)) return -1;
//   *base = *offset;
//   return diff;
// }

// /** Prints formatted string to file. */
// Syx_Value *syx_builtin_printf(Syx_Eval_Ctx *ctx, Syx_Pair *arguments) {
//   FILE *f = parse_optional_file_descriptor(&arguments);
//   syx_string fmt = {0};
//   syx_convert_to(ctx, syx_list_next(&arguments), &fmt);
//   String_View it = sb_to_sv(fmt);
//   String_View str = sb_to_sv(fmt);
//   for (; it.count; sv_chop_left(&it, 1)) {
//     size_t format_size = 1;
//     if (it.data[0] != '%') continue;
//     // TODO: implement custom formats
//     if (io_put_sv_diff(f, &str, &it) < 0) return syx_value_nil();
//     sv_chop_left(&it, format_size);
//     str = it;
//     Syx_Value *argument = syx_list_next(&arguments);
//     syx_string sb = {0};
//     syx_convert_to(ctx, argument, &sb);
//     if (!io_puts(f, sb_to_sv(sb))) return syx_value_nil();
//     sb_free(sb);
//   }
//   if (io_put_sv_diff(f, &str, &it) < 0) return syx_value_nil();
//   sb_free(fmt);
//   return syx_value_nil();
// }

void syx_env_define_builtins(Syx_Env *env) {
  /** Builtins */
  syx_env_define_cstr(env, "cons", make_syx_value_closure_builtin(SVLIT("cons"), syx_builtin_cons));
  syx_env_define_cstr(env, "list", make_syx_value_closure_builtin(SVLIT("list"), syx_builtin_list));
  syx_env_define_cstr(env, "car", make_syx_value_closure_builtin(SVLIT("car"), syx_builtin_car));
  syx_env_define_cstr(env, "cdr", make_syx_value_closure_builtin(SVLIT("cdr"), syx_builtin_cdr));
  syx_env_define_cstr(env, "apply", make_syx_value_closure_builtin(SVLIT("apply"), syx_builtin_apply));
  syx_env_define_cstr(env, "map", make_syx_value_closure_builtin(SVLIT("map"), syx_builtin_map));

  syx_env_define_cstr(env, "+", make_syx_value_closure_builtin(SVLIT("+"), syx_builtin_summ));
  syx_env_define_cstr(env, "-", make_syx_value_closure_builtin(SVLIT("-"), syx_builtin_sub));
  syx_env_define_cstr(env, "*", make_syx_value_closure_builtin(SVLIT("*"), syx_builtin_mul));
  syx_env_define_cstr(env, "/", make_syx_value_closure_builtin(SVLIT("/"), syx_builtin_div));

  syx_env_define_cstr(env, "=", make_syx_value_closure_builtin(SVLIT("="), syx_builtin_equivalent));
  syx_env_define_cstr(env, "<", make_syx_value_closure_builtin(SVLIT("<"), syx_builtin_lower_than));
  syx_env_define_cstr(env, "<=", make_syx_value_closure_builtin(SVLIT("<="), syx_builtin_lower_or_equal));
  syx_env_define_cstr(env, ">", make_syx_value_closure_builtin(SVLIT(">"), syx_builtin_greater_than));
  syx_env_define_cstr(env, ">=", make_syx_value_closure_builtin(SVLIT(">="), syx_builtin_greater_or_equal));

  syx_env_define_cstr(env, "eq?", make_syx_value_closure_builtin(SVLIT("eq?"), syx_builtin_identity));
  syx_env_define_cstr(env, "nil?", make_syx_value_closure_builtin(SVLIT("nil?"), syx_builtin_is_nil));
  syx_env_define_cstr(env, "bool?", make_syx_value_closure_builtin(SVLIT("bool?"), syx_builtin_is_bool));
  syx_env_define_cstr(env, "pair?", make_syx_value_closure_builtin(SVLIT("pair?"), syx_builtin_is_pair));
  syx_env_define_cstr(env, "list?", make_syx_value_closure_builtin(SVLIT("list?"), syx_builtin_is_list));
  syx_env_define_cstr(env, "special?", make_syx_value_closure_builtin(SVLIT("special?"), syx_builtin_is_special));
  syx_env_define_cstr(env, "special-quote?", make_syx_value_closure_builtin(SVLIT("special-quote?"), syx_builtin_is_special_quote));
  syx_env_define_cstr(env, "special-unquote?", make_syx_value_closure_builtin(SVLIT("special-unquote?"), syx_builtin_is_special_unquote));
  syx_env_define_cstr(env, "symbol?", make_syx_value_closure_builtin(SVLIT("symbol?"), syx_builtin_is_symbol));
  syx_env_define_cstr(env, "number?", make_syx_value_closure_builtin(SVLIT("number?"), syx_builtin_is_number));
  syx_env_define_cstr(env, "integer?", make_syx_value_closure_builtin(SVLIT("integer?"), syx_builtin_is_integer));
  syx_env_define_cstr(env, "fractional?", make_syx_value_closure_builtin(SVLIT("fractional?"), syx_builtin_is_fractional));
  syx_env_define_cstr(env, "string?", make_syx_value_closure_builtin(SVLIT("string?"), syx_builtin_is_string));
  // syx_env_define_cstr(env, "boxed?", make_syx_value_closure_builtin(SVLIT("boxed?"), syx_builtin_is_boxed));
  syx_env_define_cstr(env, "closure?", make_syx_value_closure_builtin(SVLIT("closure?"), syx_builtin_is_closure));
  syx_env_define_cstr(env, "special-form?", make_syx_value_closure_builtin(SVLIT("special-form?"), syx_builtin_is_special_form));
  syx_env_define_cstr(env, "builtin?", make_syx_value_closure_builtin(SVLIT("builtin?"), syx_builtin_is_builtin));
  syx_env_define_cstr(env, "lambda?", make_syx_value_closure_builtin(SVLIT("lambda?"), syx_builtin_is_lambda));

  syx_env_define_cstr(env, "not", make_syx_value_closure_builtin(SVLIT("not"), syx_builtin_not));

  // syx_env_define_cstr(env, "print", make_syx_value_closure_builtin(SVLIT("print"), syx_builtin_print));
  // syx_env_define_cstr(env, "print-flash", make_syx_value_closure_builtin(SVLIT("print-flash"), syx_builtin_print_flash));
  // syx_env_define_cstr(env, "println", make_syx_value_closure_builtin(SVLIT("println"), syx_builtin_println));
  // syx_env_define_cstr(env, "printf", make_syx_value_closure_builtin(SVLIT("printf"), syx_builtin_printf));
}

#endif // SYX_EVAL_BUILTINS_IMPL
