#ifndef SYX_EVAL_BUILTINS_H
#define SYX_EVAL_BUILTINS_H

#include <syx_new/syx_eval.h>
#include <syx_new/syx_io.h>
#include <syx_new/syx_value.h>

void syx_env_define_builtins(Syx_Env *env);

#endif // SYX_EVAL_BUILTINS_H

#if defined(SYX_EVAL_BUILTINS_IMPL) && !defined(SYX_EVAL_BUILTINS_IMPL_C)
#define SYX_EVAL_BUILTINS_IMPL_C

#include <math.h>
#include <stdio.h>
#define SYX_IO_IMPL
#include <syx_new/syx_io.h>

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
  Syx_Value *list = syx_value_from_pair(arguments);
  if (!list) list = syx_value_nil();
  return list;
}

/** Returns the left element of a pair. */
Syx_Value *syx_builtin_car(Syx_Eval_Ctx *ctx, Syx_Pair *arguments) {
  Syx_Value *list = syx_list_next(&arguments);
  SYX_EVAL_ASSERT(ctx, list->kind == SYX_VALUE_KIND_PAIR && list->pair, "list expected as car argument");
  return list->pair->left;
}

/** Returns the right element of a pair. */
Syx_Value *syx_builtin_cdr(Syx_Eval_Ctx *ctx, Syx_Pair *arguments) {
  Syx_Value *list = syx_list_next(&arguments);
  SYX_EVAL_ASSERT(ctx, list->kind == SYX_VALUE_KIND_PAIR && list->pair, "list expected as cdr argument");
  return list->pair->right;
}

/** Calls a function with a list as its argument list. */
Syx_Value *syx_builtin_apply(Syx_Eval_Ctx *ctx, Syx_Pair *arguments) {
  Syx_Value *fn = syx_list_next(&arguments);
  Syx_Value *call = rc_acquire(make_syx_value_pair(fn, NULL));
  Syx_Value **it = &call->pair->right;
  syx_list_for_each(arguments, argument) {
    if ((*it)) SYX_EVAL_THROW(ctx, "only last argument allowed to be pair with both values");
    if (argument->kind == SYX_VALUE_KIND_PAIR && argument->pair) {
      (*it) = rc_acquire(argument);
      while ((*it)->kind == SYX_VALUE_KIND_PAIR && (*it)->pair) it = &(*it)->pair->right;
      if ((*it)->kind == SYX_VALUE_KIND_PAIR && !(*it)->pair) {
        rc_release((*it));
        (*it) = NULL;
      }
      continue;
    }
    (*it) = rc_acquire(make_syx_value_pair(NULL, NULL));
    (*it)->pair->left = rc_acquire(argument);
    it = &(*it)->pair->right;
  }
  (*it) = rc_acquire(syx_value_nil());
  return syx_eval(ctx, call);
}

/** Applies a function to each element of a list and returns a new list of results. */
Syx_Value *syx_builtin_map(Syx_Eval_Ctx *ctx, Syx_Pair *arguments) {
  Syx_Value *fn = syx_list_next(&arguments);
  Syx_Value *list = syx_list_next(&arguments);
  if (list->kind != SYX_VALUE_KIND_PAIR) SYX_EVAL_THROW(ctx, "list expected");
  Syx_Value *results = NULL;
  syx_list_map(list->pair, item, &results) {
    Syx_Value *call = rc_acquire(make_syx_value_list(fn, *item, NULL));
    *item = rc_acquire(syx_eval(ctx, call));
    syx_value_early_exit(*item, (results, call));
    rc_release(call);
    rc_move(*item);
  }
  return rc_move(results);
}

/** Concat arguments to string. */
Syx_Value *syx_builtin_concat(Syx_Eval_Ctx *ctx, Syx_Pair *arguments) {
  UNUSED(ctx);
  UNUSED(arguments);
  SYX_EVAL_TODO(ctx, "syx_builtin_concat");
  // String_Builder sb = {0};
  // syx_list_for_each(arguments, argument) {
  //   sb_append_converted_syxv(&sb, ctx, argument);
  // }
  // sb_append(&sb, 0);
  // Syx_Value *string = make_syxv_string_n(sb.items, sb.count - 1);
  // sb_free(sb);
  // return string;
}

#define syx__builtin_operator(ctx, arguments, operator, nil) ({    \
  Syx_Number value = {0};                                          \
  Syx_Value *first = syx_list_next(&(arguments));                  \
  switch (first->kind) {                                           \
    case SYX_VALUE_KIND_NUMBER: value = *first->number; break;     \
    case SYX_VALUE_KIND_PAIR: {                                    \
      if (!first->pair) {                                          \
        nil;                                                       \
        return make_syx_value_number(value);                       \
      }                                                            \
    }                                                              \
    default: syx_convert_to((ctx), first, &value);                 \
  }                                                                \
  syx_list_for_each((arguments), argument) {                       \
    Syx_Number next = {0};                                         \
    switch (argument->kind) {                                      \
      case SYX_VALUE_KIND_NUMBER: next = *argument->number; break; \
      default: syx_convert_to((ctx), argument, &next);             \
    }                                                              \
    value = syx_number_operate(&value, operator, & next);          \
  }                                                                \
  return make_syx_value_number(value);                             \
})

/** Sum of all arguments. */
Syx_Value *syx_builtin_summ(Syx_Eval_Ctx *ctx, Syx_Pair *arguments) {
  Syx_Value *first = arguments ? arguments->left : NULL;
  if (first && first->kind == SYX_VALUE_KIND_STRING) return syx_builtin_concat(ctx, arguments);
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
    case SYX_VALUE_KIND_PAIR: return (right->kind == SYX_VALUE_KIND_PAIR &&
                                      !left->pair == !right->pair &&
                                      (!left->pair || (syx__builtin_equivalent_comparator(ctx, left->pair->left, right->pair->left) &&
                                                       syx__builtin_equivalent_comparator(ctx, left->pair->right, right->pair->right))));
    case SYX_VALUE_KIND_CONST: return false;  // should work on left == right level
    case SYX_VALUE_KIND_SYMBOL: return false; // should work on left == right level
    case SYX_VALUE_KIND_NUMBER: return right->kind == SYX_VALUE_KIND_NUMBER && (syx_number_get(left->number) == syx_number_get(right->number));
    case SYX_VALUE_KIND_STRING: return right->kind == SYX_VALUE_KIND_STRING && sv_eq(*left->string, *right->string);
    case SYX_VALUE_KIND_OBJECT: return false;  // should work on left == right level
    case SYX_VALUE_KIND_CLOSURE: return false; // should work on left == right level
    case SYX_VALUE_KIND_NATIVE: return (right->kind == SYX_VALUE_KIND_NATIVE && right->native->type == left->native->type && memcmp(left->native->data, right->native->data, left->native->type->size) == 0);
    case SYX_VALUE_KIND_EXIT: UNREACHABLE("should never get exit value here");
    case SYX_VALUE_KIND_PREFIXED: return (
        right->kind == SYX_VALUE_KIND_PREFIXED &&
        left->prefixed->kind == right->prefixed->kind &&
        syx__builtin_equivalent_comparator(ctx, left->prefixed->value, right->prefixed->value));
  }
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

bool syx__builtin_lower_than_comparator(Syx_Eval_Ctx *ctx, Syx_Value *left, Syx_Value *right) {
  syx__builtin_comparison_comparator(syx__builtin_lower_than_comparator, <);
}

/** Applies lower than between each consequence pairs. */
Syx_Value *syx_builtin_lower_than(Syx_Eval_Ctx *ctx, Syx_Pair *arguments) {
  return syx__builtin_compare(ctx, arguments, syx__builtin_lower_than_comparator);
}

bool syx__builtin_lower_or_equal_comparator(Syx_Eval_Ctx *ctx, Syx_Value *left, Syx_Value *right) {
  syx__builtin_comparison_comparator(syx__builtin_lower_or_equal_comparator, <=);
}

/** Applies lower or equal between each consequence pairs. */
Syx_Value *syx_builtin_lower_or_equal(Syx_Eval_Ctx *ctx, Syx_Pair *arguments) {
  return syx__builtin_compare(ctx, arguments, syx__builtin_lower_or_equal_comparator);
}

bool syx__builtin_greater_than_comparator(Syx_Eval_Ctx *ctx, Syx_Value *left, Syx_Value *right) {
  syx__builtin_comparison_comparator(syx__builtin_greater_than_comparator, >);
}

/** Applies greater than between each consequence pairs. */
Syx_Value *syx_builtin_greater_than(Syx_Eval_Ctx *ctx, Syx_Pair *arguments) {
  return syx__builtin_compare(ctx, arguments, syx__builtin_greater_than_comparator);
}

bool syx__builtin_greater_or_equal_comparator(Syx_Eval_Ctx *ctx, Syx_Value *left, Syx_Value *right) {
  syx__builtin_comparison_comparator(syx__builtin_greater_or_equal_comparator, >=);
}

/** Applies greater or equal between each consequence pairs. */
Syx_Value *syx_builtin_greater_or_equal(Syx_Eval_Ctx *ctx, Syx_Pair *arguments) {
  return syx__builtin_compare(ctx, arguments, syx__builtin_greater_or_equal_comparator);
}

#undef syx__builtin_comparison_comparator

bool syx__builtin_identity_comparator(Syx_Eval_Ctx *ctx, Syx_Value *left, Syx_Value *right) {
  UNUSED(ctx);
  if (left == right) return true;
  switch (left->kind) {
    case SYX_VALUE_KIND_PAIR: return false;   // should work on left == right level
    case SYX_VALUE_KIND_CONST: return false;  // should work on left == right level
    case SYX_VALUE_KIND_SYMBOL: return false; // should work on left == right level
    case SYX_VALUE_KIND_NUMBER: return (
        right->kind == SYX_VALUE_KIND_NUMBER &&
        left->number->kind == right->number->kind &&
        syx_number_get(left->number) == syx_number_get(right->number));
    case SYX_VALUE_KIND_STRING: return false;  // should work on left == right level
    case SYX_VALUE_KIND_OBJECT: return false;  // should work on left == right level
    case SYX_VALUE_KIND_CLOSURE: return false; // should work on left == right level
    case SYX_VALUE_KIND_NATIVE: return (right->kind == SYX_VALUE_KIND_NATIVE && right->native->type == left->native->type && left->native->data == right->native->data);
    case SYX_VALUE_KIND_EXIT: UNREACHABLE("should never get exit value here");
    case SYX_VALUE_KIND_PREFIXED: return false; // should work on left == right level
  }
}

/** Applies identity check between each consequence pairs. */
Syx_Value *syx_builtin_identity(Syx_Eval_Ctx *ctx, Syx_Pair *arguments) {
  return syx__builtin_compare(ctx, arguments, syx__builtin_identity_comparator);
}

#define syx__builtin_type_guard(kind_checks)                   \
  Syx_Value *value = syx_list_next_nullable(&arguments);       \
  if (value == NULL) SYX_EVAL_THROW(ctx, "argument expected"); \
  return syx_value_bool((kind_checks))

/** Type checks if first argument is pair. */
Syx_Value *syx_builtin_is_pair(Syx_Eval_Ctx *ctx, Syx_Pair *arguments) { syx__builtin_type_guard(value->kind == SYX_VALUE_KIND_PAIR); }

/** Type checks if first argument is nil. */
Syx_Value *syx_builtin_is_nil(Syx_Eval_Ctx *ctx, Syx_Pair *arguments) { syx__builtin_type_guard(value->kind == SYX_VALUE_KIND_PAIR && !value->pair); }

/** Type checks if first argument is full pair. */
Syx_Value *syx_builtin_is_full_pair(Syx_Eval_Ctx *ctx, Syx_Pair *arguments) { syx__builtin_type_guard(value->kind == SYX_VALUE_KIND_PAIR && value->pair); }

/** Type checks if first argument is list. */
Syx_Value *syx_builtin_is_list(Syx_Eval_Ctx *ctx, Syx_Pair *arguments) {
  Syx_Value *value = syx_list_next_nullable(&arguments);
  if (value == NULL) SYX_EVAL_THROW(ctx, "argument expected");
  Syx_Value *it = value;
  while (it->kind == SYX_VALUE_KIND_PAIR && it->pair) it = it->pair->right;
  return syx_value_bool(it == syx_value_nil());
}

/** Type checks if first argument is const. */
Syx_Value *syx_builtin_is_const(Syx_Eval_Ctx *ctx, Syx_Pair *arguments) { syx__builtin_type_guard(value->kind == SYX_VALUE_KIND_CONST); }

/** Type checks if first argument is bool true. */
Syx_Value *syx_builtin_is_bool_true(Syx_Eval_Ctx *ctx, Syx_Pair *arguments) { syx__builtin_type_guard(value == syx_value_bool_true()); }

/** Type checks if first argument is bool false. */
Syx_Value *syx_builtin_is_bool_false(Syx_Eval_Ctx *ctx, Syx_Pair *arguments) { syx__builtin_type_guard(value == syx_value_bool_false()); }

/** Type checks if first argument is bool. */
Syx_Value *syx_builtin_is_bool(Syx_Eval_Ctx *ctx, Syx_Pair *arguments) { syx__builtin_type_guard(value == syx_value_bool_true() || value == syx_value_bool_false()); }

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

/** Type checks if first argument is object. */
Syx_Value *syx_builtin_is_object(Syx_Eval_Ctx *ctx, Syx_Pair *arguments) { syx__builtin_type_guard(value->kind == SYX_VALUE_KIND_OBJECT); }

/** Type checks if first argument is closure. */
Syx_Value *syx_builtin_is_closure(Syx_Eval_Ctx *ctx, Syx_Pair *arguments) { syx__builtin_type_guard(value->kind == SYX_VALUE_KIND_CLOSURE); }

/** Type checks if first argument is special form. */
Syx_Value *syx_builtin_is_special_form(Syx_Eval_Ctx *ctx, Syx_Pair *arguments) { syx__builtin_type_guard(value->kind == SYX_VALUE_KIND_CLOSURE && value->closure->kind == SYX_CLOSURE_KIND_SPECIALF); }

/** Type checks if first argument is builtin. */
Syx_Value *syx_builtin_is_builtin(Syx_Eval_Ctx *ctx, Syx_Pair *arguments) { syx__builtin_type_guard(value->kind == SYX_VALUE_KIND_CLOSURE && value->closure->kind == SYX_CLOSURE_KIND_BUILTIN); }

/** Type checks if first argument is lambda. */
Syx_Value *syx_builtin_is_lambda(Syx_Eval_Ctx *ctx, Syx_Pair *arguments) { syx__builtin_type_guard(value->kind == SYX_VALUE_KIND_CLOSURE && value->closure->kind == SYX_CLOSURE_KIND_LAMBDA); }

/** Type checks if first argument is native constructor. */
Syx_Value *syx_builtin_is_native_constructor(Syx_Eval_Ctx *ctx, Syx_Pair *arguments) { syx__builtin_type_guard(value->kind == SYX_VALUE_KIND_CLOSURE && value->closure->kind == SYX_CLOSURE_KIND_NATIVE_CONSTRUCTOR); }

/** Type checks if first argument is native wrapper. */
Syx_Value *syx_builtin_is_native(Syx_Eval_Ctx *ctx, Syx_Pair *arguments) { syx__builtin_type_guard(value->kind == SYX_VALUE_KIND_NATIVE); }

/** Type checks if first argument is prefixed. */
Syx_Value *syx_builtin_is_prefixed(Syx_Eval_Ctx *ctx, Syx_Pair *arguments) { syx__builtin_type_guard(value->kind == SYX_VALUE_KIND_PREFIXED); }

/** Type checks if first argument is quoted. */
Syx_Value *syx_builtin_is_quoted(Syx_Eval_Ctx *ctx, Syx_Pair *arguments) { syx__builtin_type_guard(value->kind == SYX_VALUE_KIND_PREFIXED && value->prefixed->kind == SYX_PREFIXED_KIND_QUOTE); }

/** Type checks if first argument is unquoted. */
Syx_Value *syx_builtin_is_unquoted(Syx_Eval_Ctx *ctx, Syx_Pair *arguments) { syx__builtin_type_guard(value->kind == SYX_VALUE_KIND_PREFIXED && value->prefixed->kind == SYX_PREFIXED_KIND_UNQUOTE); }

/** Type checks if first argument is coloned. */
Syx_Value *syx_builtin_is_coloned(Syx_Eval_Ctx *ctx, Syx_Pair *arguments) { syx__builtin_type_guard(value->kind == SYX_VALUE_KIND_PREFIXED && value->prefixed->kind == SYX_PREFIXED_KIND_COLON); }

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

Syx_Closure_Builtin syx__builtints_static_check_is() {
  Syx_Value_Kind value_kind = (Syx_Value_Kind)0;
  switch (value_kind) {
    case SYX_VALUE_KIND_PAIR: {
      UNUSED(syx_builtin_is_nil);
      UNUSED(syx_builtin_is_full_pair);
      UNUSED(syx_builtin_is_list);
      return syx_builtin_is_pair;
    }
    case SYX_VALUE_KIND_CONST: {
      UNUSED(syx_builtin_is_bool_true);
      UNUSED(syx_builtin_is_bool_false);
      UNUSED(syx_builtin_is_bool);
      return syx_builtin_is_const;
    }
    case SYX_VALUE_KIND_SYMBOL: return syx_builtin_is_symbol;
    case SYX_VALUE_KIND_NUMBER: {
      Syx_Number_Kind number_kind = (Syx_Number_Kind)0;
      switch (number_kind) {
        case SYX_NUMBER_KIND_INTEGER: return syx_builtin_is_integer;
        case SYX_NUMBER_KIND_FRACTIONAL: return syx_builtin_is_fractional;
      }
      return syx_builtin_is_number;
    }
    case SYX_VALUE_KIND_STRING: return syx_builtin_is_string;
    case SYX_VALUE_KIND_OBJECT: return syx_builtin_is_object;
    case SYX_VALUE_KIND_CLOSURE: {
      Syx_Closure_Kind closure_kind = (Syx_Closure_Kind)0;
      switch (closure_kind) {
        case SYX_CLOSURE_KIND_SPECIALF: return syx_builtin_is_special_form;
        case SYX_CLOSURE_KIND_BUILTIN: return syx_builtin_is_builtin;
        case SYX_CLOSURE_KIND_LAMBDA: return syx_builtin_is_lambda;
        case SYX_CLOSURE_KIND_NATIVE_CONSTRUCTOR: return syx_builtin_is_native_constructor;
      }
      return syx_builtin_is_closure;
    }
    case SYX_VALUE_KIND_NATIVE: return syx_builtin_is_native;
    case SYX_VALUE_KIND_EXIT: return (Syx_Closure_Builtin)0;
    case SYX_VALUE_KIND_PREFIXED: {
      Syx_Prefixed_Kind prefixed_kind = (Syx_Prefixed_Kind)0;
      switch (prefixed_kind) {
        case SYX_PREFIXED_KIND_QUOTE: return syx_builtin_is_quoted;
        case SYX_PREFIXED_KIND_UNQUOTE: return syx_builtin_is_unquoted;
        case SYX_PREFIXED_KIND_COLON: return syx_builtin_is_coloned;
        case SYX_PREFIXED_KIND_DOLLAR: return (Syx_Closure_Builtin)0;
      }
      return syx_builtin_is_prefixed;
    }
  }
}

#undef syx__builtin_type_guard

typedef struct Syx_File_Constant {
  int fd;
  FILE *stream;
} Syx_File_Constant;

syx_define_constant(Ht(Syx_Symbol *, Syx_File_Constant), FD_CONSTANTS) {
  FD_CONSTANTS->hasheq = ht_syx_symbol_hasheq;
  *ht_put(FD_CONSTANTS, make_syx_value_symbol_strlit("stdout")->symbol) = (Syx_File_Constant){.fd = STDOUT_FILENO, .stream = stdout};
  *ht_put(FD_CONSTANTS, make_syx_value_symbol_strlit("stderr")->symbol) = (Syx_File_Constant){.fd = STDERR_FILENO, .stream = stderr};
  *ht_put(FD_CONSTANTS, make_syx_value_symbol_strlit("stdin")->symbol) = (Syx_File_Constant){.fd = STDIN_FILENO, .stream = stdin};
  ht_foreach(symbol, FD_CONSTANTS) {
    rc_acquire(syx_value_from_symbol(ht_key(FD_CONSTANTS, symbol)));
  }
}

FILE *parse_optional_file_descriptor(Syx_Pair **arguments) {
  FILE *f = stdout;
  if (!(*arguments)) return f;
  Syx_Value *argument = (*arguments)->left;
  switch (argument->kind) {
    case SYX_VALUE_KIND_PREFIXED: {
      if (argument->prefixed->kind != SYX_PREFIXED_KIND_COLON) return f;
      argument = argument->prefixed->value;
      if (argument->kind != SYX_VALUE_KIND_SYMBOL) return f;
      if ((*arguments)->right->kind != SYX_VALUE_KIND_PAIR) return f;
      Syx_File_Constant *constant = ht_find(FD_CONSTANTS(), (*arguments)->left->symbol);
      if (!constant) return f;
      (*arguments) = (*arguments)->right->pair;
      f = constant->stream;
    } break;
    case SYX_VALUE_KIND_NATIVE: {
      if (argument->native->type != SYX_KNOWN_TYPES()->c_file) return f;
      f = *(FILE **)argument->native->data;
    } break;
    default:
  }
  return f;
}

Syx_Value *syx__builtin_print_values(Syx_Eval_Ctx *ctx, FILE *f, Syx_Pair *arguments) {
  bool first = true;
  size_t count = 0;
  String_Builder sb = {0};
  syx_list_for_each(arguments, argument) {
    sb.count = 0;
    syx_convert_to(ctx, argument, &sb);
    if (!first) {
      if (!syx_io_putc(f, ' ')) goto result;
      count += 1;
    }
    first = false;
    size_t value_count = syx_io_puts_n(f, sb.data, sb.count);
    if (!value_count) goto result;
    count += value_count;
  }
result:
  sb_free(&sb);
  return make_syx_value_number_integer(count);
}

/** Prints arguments to file. */
Syx_Value *syx_builtin_print(Syx_Eval_Ctx *ctx, Syx_Pair *arguments) {
  FILE *f = parse_optional_file_descriptor(&arguments);
  return syx__builtin_print_values(ctx, f, arguments);
}

/** Flash file descriptor. */
Syx_Value *syx_builtin_print_flash(Syx_Eval_Ctx *ctx, Syx_Pair *arguments) {
  UNUSED(ctx);
  FILE *f = parse_optional_file_descriptor(&arguments);
  syx_io_flash(f);
  return syx_value_nil();
}

/** Prints arguments to file, adds new line to the end. */
Syx_Value *syx_builtin_println(Syx_Eval_Ctx *ctx, Syx_Pair *arguments) {
  FILE *f = parse_optional_file_descriptor(&arguments);
  Syx_Value *count = syx__builtin_print_values(ctx, f, arguments);
  if (count->kind != SYX_VALUE_KIND_NUMBER || count->number->kind != SYX_NUMBER_KIND_INTEGER) return count;
  if (!syx_io_putc(f, '\n')) return count;
  return make_syx_value_number_integer(count->number->integer + 1);
}

/** Prints formatted string to file. */
Syx_Value *syx_builtin_printf(Syx_Eval_Ctx *ctx, Syx_Pair *arguments) {
  FILE *f = parse_optional_file_descriptor(&arguments);
  String fmt = {0};
  syx_convert_to(ctx, syx_list_next(&arguments), &fmt);
  size_t count = 0;
  String_Builder sb = {0};
  for (size_t index = 0; index < fmt.count; index += utf8_character_lengths[(uint8_t)fmt.data[index]]) {
    if (fmt.data[index] != '%') goto put_char;
    if (index + 1 < fmt.count && fmt.data[index + 1] == '%') goto put_char;
    Syx_Value *argument = syx_list_next(&arguments);
    sb.count = 0;
    syx_convert_to(ctx, argument, &sb);
    size_t value_count = syx_io_puts_n(f, sb.data, sb.count);
    if (!value_count) goto result;
    count += value_count;
    continue;
  put_char:
    if (!syx_io_putc(f, fmt.data[index])) goto result;
    count += 1;
  }
result:
  sb_free(&sb);
  return make_syx_value_number_integer(count);
}

void syx_env_define_builtins(Syx_Env *env) {
  /** Builtins */
  syx_env_define_strlit(env, "cons", make_syx_value_closure_builtin(NULL, syx_builtin_cons));
  syx_env_define_strlit(env, "list", make_syx_value_closure_builtin(NULL, syx_builtin_list));
  syx_env_define_strlit(env, "car", make_syx_value_closure_builtin(NULL, syx_builtin_car));
  syx_env_define_strlit(env, "cdr", make_syx_value_closure_builtin(NULL, syx_builtin_cdr));
  syx_env_define_strlit(env, "apply", make_syx_value_closure_builtin(NULL, syx_builtin_apply));
  syx_env_define_strlit(env, "map", make_syx_value_closure_builtin(NULL, syx_builtin_map));

  syx_env_define_strlit(env, "+", make_syx_value_closure_builtin(NULL, syx_builtin_summ));
  syx_env_define_strlit(env, "-", make_syx_value_closure_builtin(NULL, syx_builtin_sub));
  syx_env_define_strlit(env, "*", make_syx_value_closure_builtin(NULL, syx_builtin_mul));
  syx_env_define_strlit(env, "/", make_syx_value_closure_builtin(NULL, syx_builtin_div));

  syx_env_define_strlit(env, "=", make_syx_value_closure_builtin(NULL, syx_builtin_equivalent));
  syx_env_define_strlit(env, "<", make_syx_value_closure_builtin(NULL, syx_builtin_lower_than));
  syx_env_define_strlit(env, "<=", make_syx_value_closure_builtin(NULL, syx_builtin_lower_or_equal));
  syx_env_define_strlit(env, ">", make_syx_value_closure_builtin(NULL, syx_builtin_greater_than));
  syx_env_define_strlit(env, ">=", make_syx_value_closure_builtin(NULL, syx_builtin_greater_or_equal));

  syx_env_define_strlit(env, "eq?", make_syx_value_closure_builtin(NULL, syx_builtin_identity));

  syx_env_define_strlit(env, "pair?", make_syx_value_closure_builtin(NULL, syx_builtin_is_pair));
  syx_env_define_strlit(env, "nil?", make_syx_value_closure_builtin(NULL, syx_builtin_is_nil));
  syx_env_define_strlit(env, "pair-full?", make_syx_value_closure_builtin(NULL, syx_builtin_is_full_pair));
  syx_env_define_strlit(env, "list?", make_syx_value_closure_builtin(NULL, syx_builtin_is_list));
  syx_env_define_strlit(env, "const?", make_syx_value_closure_builtin(NULL, syx_builtin_is_const));
  syx_env_define_strlit(env, "true?", make_syx_value_closure_builtin(NULL, syx_builtin_is_bool_true));
  syx_env_define_strlit(env, "false?", make_syx_value_closure_builtin(NULL, syx_builtin_is_bool_false));
  syx_env_define_strlit(env, "bool?", make_syx_value_closure_builtin(NULL, syx_builtin_is_bool));
  syx_env_define_strlit(env, "symbol?", make_syx_value_closure_builtin(NULL, syx_builtin_is_symbol));
  syx_env_define_strlit(env, "number?", make_syx_value_closure_builtin(NULL, syx_builtin_is_number));
  syx_env_define_strlit(env, "integer?", make_syx_value_closure_builtin(NULL, syx_builtin_is_integer));
  syx_env_define_strlit(env, "fractional?", make_syx_value_closure_builtin(NULL, syx_builtin_is_fractional));
  syx_env_define_strlit(env, "string?", make_syx_value_closure_builtin(NULL, syx_builtin_is_string));
  syx_env_define_strlit(env, "object?", make_syx_value_closure_builtin(NULL, syx_builtin_is_object));
  syx_env_define_strlit(env, "closure?", make_syx_value_closure_builtin(NULL, syx_builtin_is_closure));
  syx_env_define_strlit(env, "special-form?", make_syx_value_closure_builtin(NULL, syx_builtin_is_special_form));
  syx_env_define_strlit(env, "builtin?", make_syx_value_closure_builtin(NULL, syx_builtin_is_builtin));
  syx_env_define_strlit(env, "lambda?", make_syx_value_closure_builtin(NULL, syx_builtin_is_lambda));
  syx_env_define_strlit(env, "constructor?", make_syx_value_closure_builtin(NULL, syx_builtin_is_native_constructor));
  syx_env_define_strlit(env, "native?", make_syx_value_closure_builtin(NULL, syx_builtin_is_native));
  syx_env_define_strlit(env, "prefixed?", make_syx_value_closure_builtin(NULL, syx_builtin_is_prefixed));
  syx_env_define_strlit(env, "quoted?", make_syx_value_closure_builtin(NULL, syx_builtin_is_quoted));
  syx_env_define_strlit(env, "unquoted?", make_syx_value_closure_builtin(NULL, syx_builtin_is_unquoted));
  syx_env_define_strlit(env, "coloned?", make_syx_value_closure_builtin(NULL, syx_builtin_is_coloned));

  syx_env_define_strlit(env, "not", make_syx_value_closure_builtin(NULL, syx_builtin_not));

  syx_env_define_strlit(env, "print", make_syx_value_closure_builtin(NULL, syx_builtin_print));
  syx_env_define_strlit(env, "print-flash", make_syx_value_closure_builtin(NULL, syx_builtin_print_flash));
  syx_env_define_strlit(env, "println", make_syx_value_closure_builtin(NULL, syx_builtin_println));
  syx_env_define_strlit(env, "printf", make_syx_value_closure_builtin(NULL, syx_builtin_printf));
}

#endif // SYX_EVAL_BUILTINS_IMPL
