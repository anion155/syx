#ifndef SYX_VALUE_H
#define SYX_VALUE_H

#include <defines.h>
#include <ht.h>
#include <rc.h>
#include <stdint.h>
#include <sv.h>
#include <syx_new/syx_utils.h>

typedef String_View syx_string_view;
typedef String_Builder syx_string;

typedef struct Syx_Frame Syx_Frame;
typedef struct Syx_Eval_Ctx Syx_Eval_Ctx;
typedef struct Syx_Env Syx_Env;

typedef struct Syx_Value Syx_Value;
typedef struct Syx_Pair Syx_Pair;
typedef struct Syx_Symbol Syx_Symbol;
typedef struct Syx_Number Syx_Number;
typedef struct Syx_String Syx_String;
typedef struct Syx_Object Syx_Object;
typedef struct Syx_Closure Syx_Closure;
typedef struct Syx_Exit Syx_Exit;
typedef struct Syx_Prefixed Syx_Prefixed;

typedef enum Syx_Value_Kind : unsigned int {
  SYX_VALUE_KIND_PAIR,
  SYX_VALUE_KIND_CONST,
  SYX_VALUE_KIND_SYMBOL,
  SYX_VALUE_KIND_NUMBER,
  SYX_VALUE_KIND_STRING,
  SYX_VALUE_KIND_OBJECT,
  SYX_VALUE_KIND_CLOSURE,
  SYX_VALUE_KIND_EXIT,
  SYX_VALUE_KIND_PREFIXED,
} Syx_Value_Kind;

typedef struct Syx_Value {
  Syx_Value_Kind kind;

  union {
    Syx_Pair *pair;
    Syx_Symbol *symbol;
    Syx_Number *number;
    Syx_String *string;
    Syx_Object *object;
    Syx_Closure *closure;
    Syx_Exit *exit;
    Syx_Prefixed *prefixed;
  };
} Syx_Value;

#define syx_boolean_select(value, is_true, is_false, non_boolean) (                           \
    (value) == syx_value_bool_true() ? is_true : (value) == syx_value_bool_false() ? is_false \
                                                                                   : non_boolean)
#define syx_boolean_get(value) syx_boolean_select((value), true, false, (UNREACHABLE("boolean expected"), false))

typedef struct Syx_Pair {
  Syx_Value *left;
  Syx_Value *right;
} Syx_Pair;

typedef struct Syx_Symbol {
  const char *data;
  size_t count;
  bool guarded;
} Syx_Symbol;

uintptr_t ht_syx_symbol_hasheq(Ht_Op op, void const *a_, void const *b_, size_t n);

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

#define syx_number_get(number) (                                          \
    (number)->kind == SYX_NUMBER_KIND_INTEGER      ? (number)->integer    \
    : (number)->kind == SYX_NUMBER_KIND_FRACTIONAL ? (number)->fractional \
                                                   : (UNREACHABLE("unknown number type"), false))

#define syx_number_optimize_fractional(value) ({     \
  if ((value)->kind == SYX_NUMBER_KIND_FRACTIONAL) { \
    if (fmodl((value)->fractional, 1.0) == 0.0) {    \
      (value)->kind = SYX_NUMBER_KIND_INTEGER;       \
      (value)->integer = (value)->fractional;        \
    }                                                \
  }                                                  \
})

#define syx_number_operate(left, operator, right) ({                                                                             \
  Syx_Number result = {0};                                                                                                       \
  if (STRINGIFY(operator)[0] == '/') {                                                                                           \
    result.kind = SYX_NUMBER_KIND_FRACTIONAL;                                                                                    \
    result.fractional = ((syx_fractional_t)syx_number_get(left) operator((syx_fractional_t)syx_number_get(right)));              \
  } else if ((left)->kind == (right)->kind) {                                                                                    \
    result.kind = (left)->kind;                                                                                                  \
    switch ((left)->kind) {                                                                                                      \
      case SYX_NUMBER_KIND_INTEGER: result.integer = (left)->integer operator((right)->integer); break;                          \
      case SYX_NUMBER_KIND_FRACTIONAL: result.fractional = (left)->fractional operator((right)->fractional); break;              \
    }                                                                                                                            \
  } else {                                                                                                                       \
    result.kind = SYX_NUMBER_KIND_FRACTIONAL;                                                                                    \
    switch ((left)->kind) {                                                                                                      \
      case SYX_NUMBER_KIND_INTEGER: result.fractional = (syx_fractional_t)(left)->integer operator(right)->fractional; break;    \
      case SYX_NUMBER_KIND_FRACTIONAL: result.fractional = (left)->fractional operator(syx_fractional_t)(right)->integer; break; \
    }                                                                                                                            \
  }                                                                                                                              \
  syx_number_optimize_fractional(&result);                                                                                       \
  result;                                                                                                                        \
})

typedef struct Syx_String {
  const char *data;
  size_t count;
} Syx_String;

typedef Ht(Syx_Symbol *, Syx_Value *) Syx_Symbols_Ht;

typedef struct Syx_Object {
  Syx_Symbols_Ht fields;
  Syx_Object *proto;
} Syx_Object;

typedef Syx_Value *(*Syx_Closure_Special_Form)(Syx_Eval_Ctx *ctx, Syx_Pair *arguments);
typedef Syx_Value *(*Syx_Closure_Builtin)(Syx_Eval_Ctx *ctx, Syx_Pair *arguments);
typedef struct Syx_Closure_Lambda Syx_Closure_Lambda;

typedef enum Syx_Closure_Kind : unsigned int {
  SYX_CLOSURE_KIND_SPECIALF,
  SYX_CLOSURE_KIND_BUILTIN,
  SYX_CLOSURE_KIND_LAMBDA,
} Syx_Closure_Kind;

typedef struct Syx_Closure {
  Syx_Closure_Kind kind;
  Syx_Symbol *name;

  union {
    Syx_Closure_Special_Form specialf;
    Syx_Closure_Builtin builtin;
    Syx_Closure_Lambda *lambda;
  };
} Syx_Closure;

typedef struct Syx_Closure_Lambda {
  Syx_Env *env;
  Syx_Pair *defines;
  Syx_Pair *forms;
} Syx_Closure_Lambda;

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

typedef enum Syx_Prefixed_Kind : unsigned int {
  SYX_PREFIXED_KIND_QUOTE = '\'',
  SYX_PREFIXED_KIND_UNQUOTE = ',',
  SYX_PREFIXED_KIND_COLON = ':',
  SYX_PREFIXED_KIND_DOLLAR = '$',
} Syx_Prefixed_Kind;

typedef struct Syx_Prefixed {
  Syx_Prefixed_Kind kind;
  Syx_Value *value;
} Syx_Prefixed;

Syx_Value *make_syx_value(Syx_Value_Kind kind, size_t size);
Syx_Value *syx_value_nil();
Syx_Value *make_syx_value_pair(Syx_Value *left, Syx_Value *right);
Syx_Value *make_syx_value__list(size_t count, Syx_Value **items);
#define make_syx_value_list(...) make_syx_value__list(sizeof((Syx_Value *[]){__VA_ARGS__}) / sizeof(Syx_Value *), (Syx_Value *[]){__VA_ARGS__})
Syx_Value *make_syx_value_symbol(syx_string_view symbol);
#define make_syx_value_symbol_strlit(symbol) make_syx_value_symbol((String_View){.data = (symbol), .count = sizeof(symbol) - 1})
Syx_Value *make_syx_value_symbol_n(const char *symbol, size_t count);
Syx_Value *make_syx_value_symbol_f(PRINTF_FMT_PARAM const char *format, ...) PRINTF_ATTRIBUTE(1, 2);
Syx_Value *make_syx_value_symbol_cstr(const char *symbol);
Syx_Value *syx_value_bool_false();
Syx_Value *syx_value_bool_true();
Syx_Value *syx_value_bool(bool value);
Syx_Value *make_syx_value_number(Syx_Number number);
Syx_Value *make_syx_value_number_integer(syx_integer_t value);
Syx_Value *make_syx_value_number_fractional(syx_fractional_t value);
Syx_Value *make_syx_value_string(Syx_String string);
#define make_syx_value_string_lit(string) make_syx_value_string((Syx_String){.data = (string), .count = sizeof(string) - 1, .managed = false});
Syx_Value *make_syx_value_string_n(char *data, size_t count);
Syx_Value *make_syx_value_string_dup(const char *data, size_t count);
Syx_Value *make_syx_value_string_cstr_dup(const char *data);
Syx_Value *make_syx_value_stringf(PRINTF_FMT_PARAM const char *format, ...) PRINTF_ATTRIBUTE(1, 2);
Syx_Value *make_syx_value_object(Syx_Object *proto);
Syx_Value *make_syx_value_closure(Syx_Symbol *name, Syx_Closure_Kind kind, size_t size);
void syx_value_closure_rename(Syx_Closure *closure, Syx_Symbol *name);
Syx_Value *make_syx_value_closure_specialf(Syx_Symbol *name, Syx_Closure_Special_Form specialf);
Syx_Value *make_syx_value_closure_builtin(Syx_Symbol *name, Syx_Closure_Builtin builtin);
Syx_Value *make_syx_value_closure_lambda(Syx_Symbol *name, Syx_Closure_Lambda lambda);
Syx_Value *make_syx_value_exit_returned(Syx_Value *returned);
Syx_Value *make_syx_value_exit_thrown(Syx_Value *reason, Syx_Frame *stack_frame);
Syx_Value *make_syx_value_prefixed(Syx_Prefixed_Kind kind, Syx_Value *inner_value);

static inline Syx_Value *syx_value_from_pair(Syx_Pair *pair) { return pair ? (Syx_Value *)pair - 1 : syx_value_nil(); }
static inline Syx_Value *syx_value_from_symbol(Syx_Symbol *symbol) { return (Syx_Value *)symbol - 1; }
static inline Syx_Value *syx_value_from_number(Syx_Number *number) { return (Syx_Value *)number - 1; }
static inline Syx_Value *syx_value_from_string(Syx_String *string) { return (Syx_Value *)string - 1; }
static inline Syx_Value *syx_value_from_object(Syx_Object *object) { return (Syx_Value *)object - 1; }
static inline Syx_Value *syx_value_from_closure(Syx_Closure *closure) { return (Syx_Value *)closure - 1; }
static inline Syx_Closure *syx_closure_from_specialf(Syx_Closure_Special_Form *specialf) { return (Syx_Closure *)((char *)specialf - offsetof(Syx_Closure, specialf)); }
static inline Syx_Closure *syx_closure_from_builtin(Syx_Closure_Builtin *builtin) { return (Syx_Closure *)((char *)builtin - offsetof(Syx_Closure, builtin)); }
static inline Syx_Closure *syx_closure_from_lambda(Syx_Closure_Lambda *lambda) { return (Syx_Closure *)lambda - 1; }
static inline Syx_Value *syx_value_from_exit(Syx_Exit *exit) { return (Syx_Value *)exit - 1; }
static inline Syx_Value *syx_value_from_prefixed(Syx_Prefixed *prefixed) { return (Syx_Value *)prefixed - 1; }

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

Syx_Value *syx_list_next_nullable(Syx_Pair **list);
Syx_Value *syx_list_next(Syx_Pair **list);

#define syx_value_early_exit(value, ...) ({               \
  Syx_Value *_value_ = (value);                           \
  if (_value_ && _value_->kind == SYX_VALUE_KIND_EXIT) {  \
    rc_release_all(EXPAND WITH_DEFAULT((), __VA_ARGS__)); \
    return rc_move(_value_);                              \
  }                                                       \
})

#define SYX_THROW(message, ...) ({                                                                                        \
  rc_release_all(EXPAND WITH_DEFAULT((), SECOND_ARG(__VA_ARGS__, )));                                                     \
  Syx_Value *reason = make_syx_value_stringf(message EXPAND(EXPAND_WITH_COMMA WITH_DEFAULT((), FIRST_ARG(__VA_ARGS__)))); \
  return make_syx_value_exit_thrown(reason, WITH_DEFAULT(NULL, THIRD_ARG(__VA_ARGS__, , )));                              \
})
#define SYX_TODO(message, ...) SYX_THROW("TODO: " message __VA_OPT__(, ) __VA_ARGS__)
#define SYX_ASSERT(condition, message, ...) ({       \
  if (!(condition)) SYX_THROW(message, __VA_ARGS__); \
})

#endif // SYX_VALUE_H

#if defined(SYX_VALUE_IMPL) && !defined(SYX_VALUE_IMPL_C)
#define SYX_VALUE_IMPL_C

#define HT_IMPL
#include <ht.h>
#define RC_IMPL
#include <rc.h>
#define GENERAL_UTILS_IMPL
#include <general_utils.h>
#define SYX_UTILS_IMPL
#include <syx_new/syx_utils.h>

syx_define_constant(struct { Syx_Value *nil; Syx_Value *bool_true; Syx_Value *bool_false; }, SYX_VALUE_CONSTANTS) {
  SYX_VALUE_CONSTANTS->nil = rc_acquire(make_syx_value(SYX_VALUE_KIND_PAIR, 0));
  SYX_VALUE_CONSTANTS->bool_true = rc_acquire(make_syx_value(SYX_VALUE_KIND_CONST, 0));
  SYX_VALUE_CONSTANTS->bool_false = rc_acquire(make_syx_value(SYX_VALUE_KIND_CONST, 0));
}

Syx_Value *make_syx_value(Syx_Value_Kind kind, size_t additional_size) {
  Syx_Value *value = rc_malloc(sizeof(Syx_Value) + additional_size);
  assert(value);
  memset(value, 0, sizeof(Syx_Value) + additional_size);
  value->kind = kind;
  return value;
}

inline Syx_Value *syx_value_nil() {
  return SYX_VALUE_CONSTANTS()->nil;
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

syx_define_constant(Ht(const char *, Syx_Value *), SYX_SYMBOLS) {
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
  Syx_Value *value = make_syx_value(SYX_VALUE_KIND_SYMBOL, sizeof(Syx_Symbol) + sizeof(char) * symbol.count);
  rc_get(value)->methods.destructor = syx_value_symbol_destructor;
  value->symbol = (Syx_Symbol *)(value + 1);
  value->symbol->data = (const char *)(value->symbol + 1);
  value->symbol->count = symbol.count;
  memcpy((char *)value->symbol->data, symbol.data, symbol.count);
  *ht_put(SYX_SYMBOLS(), value->symbol->data) = value;
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

Syx_Value *make_syx_value_symbol_f(const char *format, ...) {
  size_t checkpoint = nob_temp_save();
  va_list args;
  va_start(args, format);
  syx_string_view sv = temp_view_vsprintf(format, args);
  va_end(args);
  Syx_Value *value = make_syx_value_symbol(sv);
  nob_temp_rewind(checkpoint);
  return value;
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

inline Syx_Value *syx_value_bool(bool value) {
  return value ? SYX_VALUE_CONSTANTS()->bool_true : SYX_VALUE_CONSTANTS()->bool_false;
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
  Syx_Value *value = make_syx_value(SYX_VALUE_KIND_STRING, sizeof(Syx_String));
  value->string = (Syx_String *)(value + 1);
  (*value->string) = string;
  return value;
}

inline Syx_Value *make_syx_value_string_n(char *data, size_t count) {
  return make_syx_value_string((Syx_String){.data = data, .count = count});
}

Syx_Value *make_syx_value_string_dup(const char *data, size_t count) {
  Syx_Value *value = make_syx_value(SYX_VALUE_KIND_STRING, sizeof(Syx_String) + sizeof(char) * count);
  value->string = (Syx_String *)(value + 1);
  value->string->data = (const char *)(value->string + 1);
  value->string->count = count;
  memcpy((char *)value->string->data, data, count);
  return value;
}

inline Syx_Value *make_syx_value_string_cstr_dup(const char *data) {
  return make_syx_value_string_dup(data, strlen(data));
}

Syx_Value *make_syx_value_stringf(const char *format, ...) {
  va_list args;
  va_start(args, format);
  size_t count = vsnprintf(NULL, 0, format, args);
  va_end(args);
  assert(count >= 0);

  Syx_Value *value = make_syx_value(SYX_VALUE_KIND_STRING, sizeof(Syx_String) + sizeof(char) * count);
  value->string = (Syx_String *)(value + 1);
  value->string->data = (const char *)(value->string + 1);
  value->string->count = count;

  va_start(args, format);
  vsnprintf((char *)value->string->data, count, format, args);
  va_end(args);

  return value;
}

void syx_value_object_destructor(void *data) {
  Syx_Value *value = data;
  if (value->object->proto) rc_release(syx_value_from_object(value->object->proto));
  Syx_Symbols_Ht *fields = &value->object->fields;
  ht_foreach(value, fields) {
    Syx_Symbol *symbol = ht_key(fields, value);
    rc_release(syx_value_from_symbol(symbol));
    rc_release(*value);
  }
}

Syx_Value *make_syx_value_object(Syx_Object *proto) {
  Syx_Value *value = make_syx_value(SYX_VALUE_KIND_OBJECT, sizeof(Syx_Object));
  rc_get(value)->methods.destructor = syx_value_object_destructor;
  value->object = (Syx_Object *)(value + 1);
  value->object->fields.hasheq = ht_syx_symbol_hasheq;
  if (proto) rc_acquire(syx_value_from_object(proto));
  value->object->proto = proto;
  return value;
}

void syx_value_closure_destructor(void *data) {
  Syx_Value *value = data;
  if (value->closure->name) rc_release(syx_value_from_symbol(value->closure->name));
}

Syx_Value *make_syx_value_closure(Syx_Symbol *name, Syx_Closure_Kind kind, size_t additional_size) {
  Syx_Value *value = make_syx_value(SYX_VALUE_KIND_CLOSURE, sizeof(Syx_Closure) + additional_size);
  rc_get(value)->methods.destructor = syx_value_closure_destructor;
  value->closure = (Syx_Closure *)(value + 1);
  value->closure->kind = kind;
  if (name) rc_acquire(syx_value_from_symbol(name));
  value->closure->name = name;
  return value;
}

void syx_value_closure_rename(Syx_Closure *closure, Syx_Symbol *name) {
  rc_acquire(syx_value_from_symbol(name));
  if (closure->name) rc_release(syx_value_from_symbol(closure->name));
  closure->name = name;
}

Syx_Value *make_syx_value_closure_specialf(Syx_Symbol *name, Syx_Closure_Special_Form specialf) {
  Syx_Value *value = make_syx_value_closure(name, SYX_CLOSURE_KIND_SPECIALF, 0);
  value->closure->specialf = specialf;
  return value;
}

Syx_Value *make_syx_value_closure_builtin(Syx_Symbol *name, Syx_Closure_Builtin builtin) {
  Syx_Value *value = make_syx_value_closure(name, SYX_CLOSURE_KIND_BUILTIN, 0);
  value->closure->builtin = builtin;
  return value;
}

void syx_value_closure_lambda_destructor(void *data) {
  Syx_Value *value = data;
  rc_release(value->closure->lambda->env);
  rc_release(value->closure->lambda->defines);
  rc_release(value->closure->lambda->forms);
  syx_value_closure_destructor(data);
}

void syx_value_closure_lambda_graph_visitor(Rc_Circulars *circulars, const void *data, const void *source) {
  const Syx_Value *value = data;
  if (value->kind != SYX_VALUE_KIND_CLOSURE && value->closure->kind != SYX_CLOSURE_KIND_LAMBDA) return;
  const Syx_Closure_Lambda *lambda = value->closure->lambda;
  rc_graph_visitor(circulars, (void **)&lambda->env, source);
}

Syx_Value *make_syx_value_closure_lambda(Syx_Symbol *name, Syx_Closure_Lambda lambda) {
  Syx_Value *value = make_syx_value_closure(name, SYX_CLOSURE_KIND_LAMBDA, sizeof(Syx_Closure_Lambda));
  rc_get(value)->methods.destructor = syx_value_closure_lambda_destructor;
  rc_get(value)->methods.graph_visitor = syx_value_closure_lambda_graph_visitor;
  value->closure->lambda = (Syx_Closure_Lambda *)(value->closure + 1);
  value->closure->lambda->env = rc_acquire(lambda.env);
  value->closure->lambda->defines = rc_acquire(lambda.defines);
  value->closure->lambda->forms = rc_acquire(lambda.forms);
  return value;
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

void syx_value_prefixed_destructor(void *data) {
  Syx_Value *value = data;
  rc_release(value->prefixed->value);
}

Syx_Value *make_syx_value_prefixed(Syx_Prefixed_Kind kind, Syx_Value *inner_value) {
  Syx_Value *value = make_syx_value(SYX_VALUE_KIND_PREFIXED, sizeof(Syx_Prefixed));
  rc_get(value)->methods.destructor = syx_value_prefixed_destructor;
  value->prefixed = (Syx_Prefixed *)(value + 1);
  value->prefixed->kind = kind;
  value->prefixed->value = rc_acquire(inner_value);
  return value;
}

bool syx_list_for_each_next(Syx_Value **current, Syx_Value **next, Syx_Value **value, Syx_Value **cdr) {
  if ((*next)->kind != SYX_VALUE_KIND_PAIR) {
    if (cdr != NULL) (*cdr) = *next;
    else if ((*next)->kind != SYX_VALUE_KIND_PAIR) UNREACHABLE("list expected");
    return false;
  }
  if (!(*next)->pair) {
    if (cdr != NULL) (*cdr) = *next;
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
    else if ((*source_it)->kind != SYX_VALUE_KIND_PAIR) UNREACHABLE("list expected");
    else (**target_it) = rc_acquire(syx_value_nil());
    return false;
  }
  if (!(*source_it)->pair) {
    if (cdr != NULL) (*cdr) = (*target_it);
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

Syx_Value *syx_list_next_nullable(Syx_Pair **list) {
  if (!(*list)) return NULL;
  Syx_Value *value = (*list)->left;
  if ((*list)->right->kind == SYX_VALUE_KIND_PAIR) {
    (*list) = (*list)->right->pair;
  } else {
    SYX_THROW("list expected");
  }
  return value;
}

Syx_Value *syx_list_next(Syx_Pair **list) {
  Syx_Value *item = syx_list_next_nullable(list);
  if (!*list) (*list) = syx_value_nil()->pair;
  if (!item) return syx_value_nil();
  return item;
}

uintptr_t ht_syx_symbol_hasheq(Ht_Op op, void const *a_, void const *b_, size_t n) {
  UNUSED(n);
  Syx_Symbol const **a = (Syx_Symbol const **)a_;
  Syx_Symbol const **b = (Syx_Symbol const **)b_;
  switch (op) {
    case HT_HASH: return ht_default_hash((*a)->data, (*a)->count);
    case HT_EQ: return (*a)->count != (*b)->count ? false : memcmp((*a)->data, (*b)->data, (*a)->count) == 0;
  }
  return 0;
}

#endif // SYX_VALUE_IMPL_C
