#ifndef SYX_VALUE_H
#define SYX_VALUE_H

#include <ht.h>
#include <nob.h>
#include <rc.h>
#include <stdint.h>
#include <syx_new/syx_utils.h>

typedef Nob_String_View syx_string_view;
typedef Nob_String_Builder syx_string;

typedef struct Syx_Frame Syx_Frame;
typedef struct Syx_Eval_Ctx Syx_Eval_Ctx;
typedef struct Syx_Env Syx_Env;

typedef struct Syx_Value Syx_Value;
typedef struct Syx_Exit Syx_Exit;
typedef struct Syx_Pair Syx_Pair;
typedef struct Syx_Special Syx_Special;
typedef struct Syx_Symbol Syx_Symbol;
typedef struct Syx_Number Syx_Number;
typedef struct Syx_String Syx_String;
typedef struct Syx_Closure Syx_Closure;

typedef enum Syx_Value_Kind : unsigned int {
  SYX_VALUE_KIND_NIL,
  SYX_VALUE_KIND_BOOL_TRUE,
  SYX_VALUE_KIND_BOOL_FALSE,
  SYX_VALUE_KIND_EXIT,
  SYX_VALUE_KIND_PAIR,
  SYX_VALUE_KIND_SPECIAL,
  SYX_VALUE_KIND_SYMBOL,
  SYX_VALUE_KIND_NUMBER,
  SYX_VALUE_KIND_STRING,
  SYX_VALUE_KIND_CLOSURE,
  // SYX_VALUE_KIND_OBJECT,
  // SYX_VALUE_KIND_NATIVE,
} Syx_Value_Kind;

typedef struct Syx_Value {
  Syx_Value_Kind kind;

  union {
    Syx_Exit *exit;
    Syx_Pair *pair;
    Syx_Special *special;
    Syx_Symbol *symbol;
    Syx_Number *number;
    Syx_String *string;
    Syx_Closure *closure;
    // Syx_Object *object;
    // native;
  };
} Syx_Value;

typedef enum Syx_Exit_Kind : unsigned int {
  SYX_EXIT_KIND_RETURNED,
  SYX_EXIT_KIND_THROWN,
} Syx_Exit_Kind;

typedef struct Syx_Exit_Thrown {
  Syx_Value *reason;
  Syx_Frame *stack_frame;
} Syx_Exit_Thrown;

typedef struct Syx_Exit {
  Syx_Exit_Kind kind;

  union {
    Syx_Value *returned;
    Syx_Exit_Thrown *thrown;
  };
} Syx_Exit;

typedef struct Syx_Pair {
  Syx_Value *left;
  Syx_Value *right;
} Syx_Pair;

typedef enum Syx_Special_Kind : unsigned int {
  SYX_SPECIAL_KIND_QUOTE,
  SYX_SPECIAL_KIND_COLON,
} Syx_Special_Kind;

typedef struct Syx_Special {
  Syx_Special_Kind kind;
  Syx_Value *value;
} Syx_Special;

typedef struct Syx_Symbol {
  const char *data;
  size_t count;
  bool guarded;
} Syx_Symbol;

typedef enum Syx_Number_Kind : unsigned int {
  SYX_NUMBER_KIND_INTEGER,
  SYX_NUMBER_KIND_FRACTIONAL,
} Syx_Number_Kind;

typedef unsigned int syx_integer_t;
typedef double syx_fractional_t;

typedef struct Syx_Number {
  Syx_Number_Kind kind;

  union {
    syx_integer_t integer;
    syx_fractional_t fractional;
  };
} Syx_Number;

typedef struct Syx_String {
  const char *data;
  size_t count;
} Syx_String;

typedef Syx_Value *(*Syx_Closure_Special_Form)(Syx_Eval_Ctx *ctx, Syx_Pair *arguments);
typedef Syx_Value *(*Syx_Closure_Builtin)(Syx_Eval_Ctx *ctx, Syx_Pair *arguments);
typedef struct Syx_Closure_Lambda Syx_Closure_Lambda;

typedef enum Syx_Closure_Kind : unsigned int {
  SYX_CLOSURE_KIND_SPECIALF,
  SYX_CLOSURE_KIND_BUILTIN,
  SYX_CLOSURE_KIND_LAMBDA,
  // SYX_CLOSURE_KIND_NATIVE_CONSTRUCTOR,
} Syx_Closure_Kind;

typedef struct Syx_Closure {
  Syx_Closure_Kind kind;
  syx_string_view name;

  union {
    Syx_Closure_Special_Form specialf;
    Syx_Closure_Builtin builtin;
    Syx_Closure_Lambda *lambda;
    // *native_constructor;
  };
} Syx_Closure;

typedef struct Syx_Closure_Lambda {
  Syx_Env *env;
  Syx_Pair *defines;
  Syx_Pair *forms;
} Syx_Closure_Lambda;

Syx_Value *make_syx_value(Syx_Value_Kind kind, size_t size);
Syx_Value *syx_value_nil();
Syx_Value *make_syx_value_exit_returned(Syx_Value *returned);
Syx_Value *make_syx_value_exit_thrown(Syx_Value *reason, Syx_Frame *stack_frame);
Syx_Value *make_syx_value_pair(Syx_Value *left, Syx_Value *right);
Syx_Value *make_syx_value__list(size_t count, Syx_Value **items);
#define make_syx_value_list(...) make_syx_value__list(sizeof((Syx_Value *[]){__VA_ARGS__}) / sizeof(Syx_Value *), (Syx_Value *[]){__VA_ARGS__})
Syx_Value *make_syx_value_special(Syx_Value *inner_value);
Syx_Value *make_syx_value_symbol(syx_string_view symbol);
#define make_syx_value_symbol_strlit(symbol) make_syx_value_symbol((String_View){.data = (symbol), .count = sizeof(symbol) - 1})
Syx_Value *make_syx_value_symbol_n(const char *symbol, size_t count);
Syx_Value *make_syx_value_symbol_cstr(const char *symbol);
Syx_Value *syx_value_bool_false();
Syx_Value *syx_value_bool_true();
Syx_Value *make_syx_value_number(Syx_Number number);
Syx_Value *make_syx_value_number_integer(syx_integer_t value);
Syx_Value *make_syx_value_number_fractional(syx_fractional_t value);
Syx_Value *make_syx_value_string(Syx_String string);
#define make_syx_value_string_lit(string) make_syx_value_string((Syx_String){.data = (string), .count = sizeof(string) - 1, .managed = false});
Syx_Value *make_syx_value_string_n(const char *data, size_t count);
Syx_Value *make_syx_value_string_dup(const char *data, size_t count);
Syx_Value *make_syx_value_string_cstr_dup(const char *data);
Syx_Value *make_syx_value_closure(syx_string_view name, Syx_Closure_Kind kind, size_t size);
Syx_Value *make_syx_value_closure_specialf(syx_string_view name, Syx_Closure_Special_Form specialf);
Syx_Value *make_syx_value_closure_builtin(syx_string_view name, Syx_Closure_Builtin builtin);
Syx_Value *make_syx_value_closure_lambda(syx_string_view name, Syx_Closure_Lambda lambda);

static inline Syx_Value *syx_value_from_exit(Syx_Exit *exit) { return (Syx_Value *)exit - 1; }

static inline Syx_Value *syx_value_from_pair(Syx_Pair *pair) { return (Syx_Value *)pair - 1; }

static inline Syx_Value *syx_value_from_special(Syx_Special *special) { return (Syx_Value *)special - 1; }

static inline Syx_Value *syx_value_from_symbol(Syx_Symbol *symbol) { return (Syx_Value *)symbol - 1; }

static inline Syx_Value *syx_value_from_number(Syx_Number *number) { return (Syx_Value *)number - 1; }

static inline Syx_Value *syx_value_from_string(Syx_String *string) { return (Syx_Value *)string - 1; }

static inline Syx_Value *syx_value_from_closure(Syx_Closure *closure) { return (Syx_Value *)closure - 1; }

static inline Syx_Closure *syx_closure_from_specialf(Syx_Closure_Special_Form *specialf) { return (Syx_Closure *)(specialf - offsetof(Syx_Closure, specialf)); }

static inline Syx_Closure *syx_closure_from_builtin(Syx_Closure_Builtin *builtin) { return (Syx_Closure *)(builtin - offsetof(Syx_Closure, builtin)); }

static inline Syx_Closure *syx_closure_from_lambda(Syx_Closure_Lambda *lambda) { return (Syx_Closure *)lambda - 1; }

bool syx_list_for_each_next(Syx_Value **current, Syx_Value **next, Syx_Value **value, Syx_Value **cdr);
#define syx_list_for_each(list, value, ...)       \
  for (Syx_Value * value##_current,               \
       *value##_next = syx_value_from_pair(list), \
       *value;                                    \
       syx_list_for_each_next(&value##_current, &value##_next, &value, WITH_DEFAULT(NULL, __VA_ARGS__));)

bool syx_list_map_next(Syx_Value **source_it, Syx_Value ***target_it, Syx_Value ***value, Syx_Value ***cdr);
#define syx_list_map(list, value, results, ...)                  \
  for (Syx_Value *value##_source_it = syx_value_from_pair(list), \
                 **value##_target_it = (results),                \
                 **value = NULL;                                 \
       syx_list_map_next(&value##_source_it, &value##_target_it, &value, WITH_DEFAULT(NULL, __VA_ARGS__));)

#define syx_value_early_exit(value, ...)                 \
  do {                                                   \
    Syx_Value *_value = (value);                         \
    if (_value && _value->kind == SYX_VALUE_KIND_EXIT) { \
      rc_release_all(__VA_ARGS__);                       \
      return rc_move(_value);                            \
    }                                                    \
  } while (0)

#define SYX_THROW(message, ...)                                                            \
  do {                                                                                     \
    rc_release_all(REST_ARGS(__VA_ARGS__));                                                \
    Syx_Value *reason = make_syx_value_string_cstr_dup(message);                           \
    return make_syx_value_exit_thrown(reason, WITH_DEFAULT(NULL, FIRST_ARG(__VA_ARGS__))); \
  } while (0)
#define SYX_ASSERT(condition, message, ...)            \
  do {                                                 \
    if (!(condition)) SYX_THROW(message, __VA_ARGS__); \
  } while (0)
#define SYX_TODO(message, ...) SYX_THROW("TODO: " message __VA_OPT__(, ) __VA_ARGS__)

#endif // SYX_VALUE_H

#define SYX_VALUE_IMPL
#if defined(SYX_VALUE_IMPL) && !defined(SYX_VALUE_IMPL_C)
#define SYX_VALUE_IMPL_C

#define HT_IMPL
#include <ht.h>
#define NOB_IMPL
#include <nob.h>
#define RC_IMPL
#include <rc.h>
#define SYX_UTILS_IMPL
#include <syx_new/syx_utils.h>

define_constant(struct { Syx_Value *nil; Syx_Value *bool_true; Syx_Value *bool_false; }, SYX_VALUE_CONSTANTS) {
  SYX_VALUE_CONSTANTS->nil = rc_acquire(make_syx_value(SYX_VALUE_KIND_NIL, 0));
  SYX_VALUE_CONSTANTS->bool_true = rc_acquire(make_syx_value(SYX_VALUE_KIND_BOOL_TRUE, 0));
  SYX_VALUE_CONSTANTS->bool_false = rc_acquire(make_syx_value(SYX_VALUE_KIND_BOOL_FALSE, 0));
}

Syx_Value *make_syx_value(Syx_Value_Kind kind, size_t additional_size) {
  Syx_Value *value = rc_malloc(sizeof(Syx_Value) + additional_size);
  assert(value);
  value->kind = kind;
  return value;
}

inline Syx_Value *syx_value_nil() {
  return SYX_VALUE_CONSTANTS()->nil;
}

void syx_value_exit_returned_destructor(void *data) {
  Syx_Value *value = data;
  rc_release(value->exit->returned);
}

Syx_Value *make_syx_value_exit_returned(Syx_Value *returned) {
  Syx_Value *value = make_syx_value(SYX_VALUE_KIND_EXIT, sizeof(Syx_Exit));
  rc_get(value)->methods.destructor = syx_value_exit_returned_destructor;
  value->exit = (Syx_Exit *)(value + 1);
  value->exit->kind = SYX_EXIT_KIND_RETURNED;
  value->exit->returned = rc_acquire(returned);
  return value;
}

void syx_value_exit_thrown_destructor(void *data) {
  Syx_Value *value = data;
  rc_release(value->exit->thrown->reason);
  rc_release(value->exit->thrown->stack_frame);
}

Syx_Value *make_syx_value_exit_thrown(Syx_Value *reason, Syx_Frame *stack_frame) {
  Syx_Value *value = make_syx_value(SYX_VALUE_KIND_EXIT, sizeof(Syx_Exit) + sizeof(Syx_Exit_Thrown));
  rc_get(value)->methods.destructor = syx_value_exit_thrown_destructor;
  value->exit = (Syx_Exit *)(value + 1);
  value->exit->kind = SYX_EXIT_KIND_THROWN;
  value->exit->thrown = (Syx_Exit_Thrown *)(value->exit + 1);
  value->exit->thrown->reason = rc_acquire(reason);
  value->exit->thrown->stack_frame = rc_acquire(stack_frame);
  return value;
}

void syx_value_pair_destructor(void *data) {
  Syx_Value *value = data;
  rc_release(value->pair->left);
  rc_release(value->pair->right);
}

Syx_Value *make_syx_value_pair(Syx_Value *left, Syx_Value *right) {
  Syx_Value *value = make_syx_value(SYX_VALUE_KIND_PAIR, sizeof(Syx_Pair));
  rc_get(value)->methods.destructor = syx_value_pair_destructor;
  value->pair = (Syx_Pair *)(value + 1);
  value->pair->left = rc_acquire(left);
  value->pair->right = rc_acquire(right);
  return value;
}

Syx_Value *make_syx_value__list(size_t count, Syx_Value **items) {
  if (!count) UNREACHABLE("empty list array must contain [NULL]");
  Syx_Value *expr = items[count - 1] == NULL ? syx_value_nil() : items[count - 1];
  for (ssize_t index = (ssize_t)count - 2; index >= 0; index -= 1) {
    expr = make_syx_value_pair(items[index], expr);
  }
  return expr;
}

void syx_value_special_destructor(void *data) {
  Syx_Value *value = data;
  rc_release(value->special->value);
}

Syx_Value *make_syx_value_special(Syx_Value *inner_value) {
  Syx_Value *value = make_syx_value(SYX_VALUE_KIND_SPECIAL, sizeof(Syx_Special));
  rc_get(value)->methods.destructor = syx_value_special_destructor;
  value->special = (Syx_Special *)(value + 1);
  value->special->value = rc_acquire(inner_value);
  return value;
}

define_constant(Ht(const char *, Syx_Value *), SYX_SYMBOLS) {
  SYX_SYMBOLS->hasheq = ht_cstr_hasheq;
}

void syx_value_symbol_destructor(void *data) {
  Syx_Value *value = data;
  Syx_Value **stored = ht_find(SYX_SYMBOLS(), value->symbol->data);
  if (stored) ht_delete(SYX_SYMBOLS(), stored);
}

int issymbol(int c) {
  return (
      c == '-' ||
      c == '_' ||
      c == '#' ||
      c == '?' ||
      c == '@' ||
      c == '!' ||
      c == '$' ||
      c == '+' ||
      c == '-' ||
      c == '*' ||
      c == '/' ||
      c == '=' ||
      c == '<' ||
      c == '>' ||
      isalnum(c));
}

Syx_Value *make_syx_value_symbol(syx_string_view symbol) {
  Syx_Value **stored = ht_find(SYX_SYMBOLS(), symbol.data);
  if (stored) return *stored;
  Syx_Value *value = *stored = make_syx_value(SYX_VALUE_KIND_SYMBOL, sizeof(Syx_Symbol) + sizeof(char) * symbol.count);
  rc_get(value)->methods.destructor = syx_value_symbol_destructor;
  value->symbol = (Syx_Symbol *)(value + 1);
  value->symbol->data = (const char *)(value->symbol + 1);
  memcpy((char *)value->symbol->data, symbol.data, symbol.count);
  value->symbol->guarded = false;
  for (syx_string_view it = symbol; it.count; sv_chop_left(&it, 1)) {
    if (issymbol(*it.data)) continue;
    value->symbol->guarded = true;
    break;
  }
  return value;
}

inline Syx_Value *make_syx_value_symbol_n(const char *symbol, size_t count) {
  return make_syx_value_symbol((syx_string_view){.data = symbol, .count = count});
}

inline Syx_Value *make_syx_value_symbol_cstr(const char *symbol) {
  return make_syx_value_symbol((syx_string_view){.data = symbol, .count = strlen(symbol)});
}

inline Syx_Value *syx_value_bool_false() {
  return SYX_VALUE_CONSTANTS()->bool_false;
}

inline Syx_Value *syx_value_bool_true() {
  return SYX_VALUE_CONSTANTS()->bool_true;
}

Syx_Value *make_syx_value_number(Syx_Number number) {
  Syx_Value *value = make_syx_value(SYX_VALUE_KIND_NUMBER, sizeof(Syx_Number));
  value->number = (Syx_Number *)(value + 1);
  (*value->number) = number;
  return value;
}

inline Syx_Value *make_syx_value_number_integer(syx_integer_t value) {
  return make_syx_value_number((Syx_Number){.kind = SYX_NUMBER_KIND_INTEGER, .integer = value});
}

inline Syx_Value *make_syx_value_number_fractional(syx_fractional_t value) {
  return make_syx_value_number((Syx_Number){.kind = SYX_NUMBER_KIND_FRACTIONAL, .fractional = value});
}

Syx_Value *make_syx_value_string(Syx_String string) {
  Syx_Value *value = make_syx_value(SYX_VALUE_KIND_NUMBER, sizeof(Syx_String));
  value->string = (Syx_String *)(value + 1);
  (*value->string) = string;
  return value;
}

inline Syx_Value *make_syx_value_string_n(const char *data, size_t count) {
  return make_syx_value_string((Syx_String){.data = data, .count = count});
}

Syx_Value *make_syx_value_string_dup(const char *data, size_t count) {
  Syx_Value *value = make_syx_value(SYX_VALUE_KIND_NUMBER, sizeof(Syx_String) + sizeof(char) * count);
  value->string = (Syx_String *)(value + 1);
  value->string->data = (const char *)(value->string + 1);
  memcpy((char *)value->string->data, data, count);
  return value;
}

inline Syx_Value *make_syx_value_string_cstr_dup(const char *data) {
  return make_syx_value_string_dup(data, strlen(data));
}

Syx_Value *make_syx_value_closure(syx_string_view name, Syx_Closure_Kind kind, size_t additional_size) {
  Syx_Value *value = make_syx_value(SYX_VALUE_KIND_CLOSURE, sizeof(Syx_Closure) + additional_size + sizeof(char) * name.count);
  value->closure = (Syx_Closure *)(value + 1);
  value->closure->name.data = (const char *)((char *)(value->closure + 1) + additional_size);
  value->closure->name.count = name.count;
  memcpy((char *)value->closure->name.data, name.data, name.count);
  value->closure->kind = kind;
  return value;
}

// Syx_Value *syx_value_closure_rename(Syx_Value *value, syx_string_view name) {
//   assert(value->closure->name.data == NULL);
//   Rc *rc = rc_get(value);
//   Rc_Method_Destructor old_destructor = rc->methods.destructor;
//   rc->methods.destructor = [](void *data) {
//     if (old_destructor != NULL) old_destructor(data);
//     free(value->closure->name.data);
//   };
//   value->closure->name.data = strndup(name.data, name.count);
//   value->closure->name.count = name.count;
// }

Syx_Value *make_syx_value_closure_specialf(syx_string_view name, Syx_Closure_Special_Form specialf) {
  Syx_Value *value = make_syx_value_closure(name, SYX_CLOSURE_KIND_SPECIALF, 0);
  value->closure->specialf = specialf;
  return value;
}

Syx_Value *make_syx_value_closure_builtin(syx_string_view name, Syx_Closure_Builtin builtin) {
  Syx_Value *value = make_syx_value_closure(name, SYX_CLOSURE_KIND_BUILTIN, 0);
  value->closure->builtin = builtin;
  return value;
}

void syx_value_closure_lambda_destructor(void *data) {
  Syx_Value *value = data;
  rc_release(value->closure->lambda->env);
  rc_release(value->closure->lambda->defines);
  rc_release(value->closure->lambda->forms);
}

void syx_value_closure_lambda_graph_visitor(Rc_Circulars *circulars, const void *data, const void *source) {
  const Syx_Value *value = data;
  if (value->kind != SYX_VALUE_KIND_CLOSURE && value->closure->kind != SYX_CLOSURE_KIND_LAMBDA) return;
  const Syx_Closure_Lambda *lambda = value->closure->lambda;
  rc_graph_visitor(circulars, (void **)&lambda->env, source);
}

Syx_Value *make_syx_value_closure_lambda(syx_string_view name, Syx_Closure_Lambda lambda) {
  Syx_Value *value = make_syx_value_closure(name, SYX_CLOSURE_KIND_LAMBDA, sizeof(Syx_Closure_Lambda));
  rc_get(value)->methods = (Rc_Methods){.destructor = syx_value_closure_lambda_destructor, .graph_visitor = syx_value_closure_lambda_graph_visitor};
  value->closure->lambda = (Syx_Closure_Lambda *)(value->closure + 1);
  value->closure->lambda->env = rc_acquire(lambda.env);
  value->closure->lambda->defines = rc_acquire(lambda.defines);
  value->closure->lambda->forms = rc_acquire(lambda.forms);
  return value;
}

bool syx_list_for_each_next(Syx_Value **current, Syx_Value **next, Syx_Value **value, Syx_Value **cdr) {
  if (!(*next) || (*next)->kind != SYX_VALUE_KIND_PAIR) {
    if (cdr != NULL) (*cdr) = *next;
    else if ((*next)->kind != SYX_VALUE_KIND_NIL) UNREACHABLE("list expected");
    return false;
  }
  (*value) = (*next)->pair->left;
  (*current) = (*next);
  (*next) = (*next)->pair->right;
  return true;
}

bool syx_list_map_next(Syx_Value **source_it, Syx_Value ***target_it, Syx_Value ***value, Syx_Value ***cdr) {
  if ((*source_it)->kind != SYX_VALUE_KIND_PAIR) {
    if (cdr != NULL) (*cdr) = (*target_it);
    else if ((*source_it)->kind != SYX_VALUE_KIND_NIL) UNREACHABLE("list expected");
    else (**target_it) = rc_acquire(syx_value_nil());
    return false;
  }
  (**target_it) = rc_acquire(make_syx_value_pair(NULL, NULL));
  (*value) = &((**target_it)->pair->left);
  (**value) = (*source_it)->pair->left;
  (*source_it) = (*source_it)->pair->right;
  (*target_it) = &((**target_it)->pair->right);
  return true;
}

#endif // SYX_VALUE_IMPL_C
