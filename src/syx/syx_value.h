#ifndef SYX_VALUE_H
#define SYX_VALUE_H

#include <defines.h>
#include <ht.h>
#include <rc.h>
#include <sb.h>
#include <sb_number.h>
#include <stdint.h>
#include <syx/syx_utils.h>

typedef struct Syx_Frame Syx_Frame;
typedef struct Syx_Eval_Ctx Syx_Eval_Ctx;
typedef struct Syx_Env Syx_Env;

typedef struct Syx_Value Syx_Value;
typedef struct Syx_Pair Syx_Pair;
typedef struct Syx_Symbol Syx_Symbol;
typedef struct Syx_Number Syx_Number;
typedef String Syx_String;
typedef struct Syx_Object Syx_Object;
typedef struct Syx_Closure Syx_Closure;
typedef struct Syx_Native Syx_Native;
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
  SYX_VALUE_KIND_NATIVE,
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
    Syx_Native *native;
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
  char *data;
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

typedef Ht(Syx_Symbol *, Syx_Value *) Syx_Symbols_Ht;

typedef struct Syx_Object {
  Syx_Symbols_Ht fields;
  Syx_Object *proto;
} Syx_Object;

typedef Syx_Value *(*Syx_Closure_Special_Form)(Syx_Eval_Ctx *ctx, Syx_Pair *arguments);
typedef Syx_Value *(*Syx_Closure_Builtin)(Syx_Eval_Ctx *ctx, Syx_Pair *arguments);
typedef struct Syx_Closure_Lambda Syx_Closure_Lambda;
typedef struct Syx_Type Syx_Type;

typedef enum Syx_Closure_Kind : unsigned int {
  SYX_CLOSURE_KIND_SPECIALF,
  SYX_CLOSURE_KIND_BUILTIN,
  SYX_CLOSURE_KIND_LAMBDA,
  SYX_CLOSURE_KIND_NATIVE_CONSTRUCTOR,
} Syx_Closure_Kind;

typedef struct Syx_Closure {
  Syx_Closure_Kind kind;
  Syx_Symbol *name;

  union {
    Syx_Closure_Special_Form specialf;
    Syx_Closure_Builtin builtin;
    Syx_Closure_Lambda *lambda;
    Syx_Type *native;
  };
} Syx_Closure;

typedef struct Syx_Closure_Lambda {
  Syx_Env *env;
  Syx_Pair *defines;
  Syx_Pair *forms;
} Syx_Closure_Lambda;

typedef struct Syx_Type Syx_Type;
typedef struct Syx_Native {
  Syx_Native *parent;
  Syx_Type *type;
  void *data;
} Syx_Native;

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

Syx_Value *make_syx_value(Syx_Value_Kind kind, size_t additional_size);
Syx_Value *syx_value_nil();
Syx_Value *make_syx_value_pair(Syx_Value *left, Syx_Value *right);
Syx_Value *make_syx_value__list(size_t count, Syx_Value **items);
#define make_syx_value_list(...) make_syx_value__list(sizeof((Syx_Value *[]){__VA_ARGS__}) / sizeof(Syx_Value *), (Syx_Value *[]){__VA_ARGS__})
Syx_Value *make_syx_value_symbol_n(const char *symbol, size_t count);
#define make_syx_value_symbol_strlit(symbol) make_syx_value_symbol_n((symbol), sizeof(symbol) - 1)
Syx_Value *make_syx_value_symbol_sv(String_View symbol);
Syx_Value *make_syx_value_symbolf(PRINTF_FMT_PARAM const char *format, ...) PRINTF_ATTRIBUTE(1, 2);
Syx_Value *make_syx_value_symbol_cstr(const char *symbol);
Syx_Value *syx_value_bool_false();
Syx_Value *syx_value_bool_true();
Syx_Value *syx_value_bool(bool value);
Syx_Value *make_syx_value_number(Syx_Number number);
Syx_Value *make_syx_value_number_integer(syx_integer_t value);
Syx_Value *make_syx_value_number_fractional(syx_fractional_t value);
Syx_Value *make_syx_value_string(Syx_String *string);
Syx_Value *make_syx_value_string_dup(Syx_String string);
Syx_Value *make_syx_value_string_n_dup(const char *data, size_t count);
Syx_Value *make_syx_value_string_cstr_dup(const char *data);
#define make_syx_value_string_strlit_dup(string) make_syx_value_string_n_dup((string), sizeof(string) - 1);
Syx_Value *make_syx_value_stringf_dup(PRINTF_FMT_PARAM const char *format, ...) PRINTF_ATTRIBUTE(1, 2);
Syx_Value *make_syx_value_object(Syx_Object *proto);
Syx_Value *make_syx_value_closure(Syx_Symbol *name, Syx_Closure_Kind kind, size_t size);
void syx_value_closure_rename(Syx_Closure *closure, Syx_Symbol *name);
Syx_Value *make_syx_value_closure_specialf(Syx_Symbol *name, Syx_Closure_Special_Form specialf);
Syx_Value *make_syx_value_closure_builtin(Syx_Symbol *name, Syx_Closure_Builtin builtin);
Syx_Value *make_syx_value_closure_lambda(Syx_Symbol *name, Syx_Closure_Lambda lambda);
Syx_Value *make_syx_value_closure_native_constructor(Syx_Symbol *name, Syx_Type *type);
Syx_Value *make_syx_value_native(Syx_Native *parent, Syx_Type *type, void *data, size_t additional_size);
Syx_Value *make_syx_value_native_instance(Syx_Type *type);
Syx_Value *make_syx_value_exit_returned(Syx_Value *returned);
Syx_Value *make_syx_value_exit_thrown(Syx_Value *reason, Syx_Frame *stack_frame);
Syx_Value *make_syx_value_prefixed(Syx_Prefixed_Kind kind, Syx_Value *inner_value);

static inline Syx_Value *syx_value_from_pair(Syx_Pair *pair) { return pair ? (Syx_Value *)pair - 1 : NULL; }
static inline Syx_Value *syx_value_from_symbol(Syx_Symbol *symbol) { return symbol ? (Syx_Value *)symbol - 1 : NULL; }
static inline Syx_Value *syx_value_from_number(Syx_Number *number) { return number ? (Syx_Value *)number - 1 : NULL; }
static inline Syx_Value *syx_value_from_string(Syx_String *string) { return string ? (Syx_Value *)string - 1 : NULL; }
static inline Syx_Value *syx_value_from_object(Syx_Object *object) { return object ? (Syx_Value *)object - 1 : NULL; }
static inline Syx_Value *syx_value_from_closure(Syx_Closure *closure) { return closure ? (Syx_Value *)closure - 1 : NULL; }
static inline Syx_Closure *syx_closure_from_specialf(Syx_Closure_Special_Form *specialf) { return specialf ? (Syx_Closure *)((char *)specialf - offsetof(Syx_Closure, specialf)) : NULL; }
static inline Syx_Closure *syx_closure_from_builtin(Syx_Closure_Builtin *builtin) { return builtin ? (Syx_Closure *)((char *)builtin - offsetof(Syx_Closure, builtin)) : NULL; }
static inline Syx_Closure *syx_closure_from_lambda(Syx_Closure_Lambda *lambda) { return lambda ? (Syx_Closure *)lambda - 1 : NULL; }
static inline Syx_Value *syx_value_from_native(Syx_Native *native) { return native ? (Syx_Value *)native - 1 : NULL; }
static inline Syx_Value *syx_value_from_exit(Syx_Exit *exit) { return exit ? (Syx_Value *)exit - 1 : NULL; }
static inline Syx_Value *syx_value_from_prefixed(Syx_Prefixed *prefixed) { return prefixed ? (Syx_Value *)prefixed - 1 : NULL; }

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

size_t sb_append_syx_symbol(String_Builder *sb, const Syx_Symbol *symbol);
size_t sb_append_syx_value(String_Builder *sb, const Syx_Value *value);

#define syx_value_early_exit(value, ...) ({               \
  Syx_Value *_value_ = (value);                           \
  if (_value_ && _value_->kind == SYX_VALUE_KIND_EXIT) {  \
    rc_release_all(EXPAND WITH_DEFAULT((), __VA_ARGS__)); \
    return rc_move(_value_);                              \
  }                                                       \
})

#define SYX_THROW(message, ...) ({                                                                                            \
  rc_release_all(EXPAND WITH_DEFAULT((), SECOND_ARG(__VA_ARGS__, )));                                                         \
  Syx_Value *reason = make_syx_value_stringf_dup(message EXPAND(EXPAND_WITH_COMMA WITH_DEFAULT((), FIRST_ARG(__VA_ARGS__)))); \
  return make_syx_value_exit_thrown(reason, WITH_DEFAULT(NULL, THIRD_ARG(__VA_ARGS__, , )));                                  \
})
#define SYX_TODO(...) SYX_THROW("TODO: " WITH_DEFAULT(TODO_DEFAULT_MESSAGE, __VA_ARGS__), WITH_DEFAULT((), SECOND_ARG(__VA_ARGS__, )), WITH_DEFAULT((), THIRD_ARG(__VA_ARGS__, , )), WITH_DEFAULT(NULL, FORTH_ARG(__VA_ARGS__, , , )))
#define SYX_ASSERT(condition, message, ...) ({                     \
  if (!(condition)) SYX_THROW(message __VA_OPT__(, ) __VA_ARGS__); \
})

#endif // SYX_VALUE_H

#if defined(SYX_VALUE_IMPL) && !defined(SYX_VALUE_IMPL_C)
#define SYX_VALUE_IMPL_C

#define HT_IMPL
#include <ht.h>
#define RC_IMPL
#include <rc.h>
#define SB_NUMBER_IMPL
#include <sb_number.h>
#define SYX_UTILS_IMPL
#include <syx/syx_utils.h>
#define SYX_TYPES_IMPL
#include <syx/syx_types.h>

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

Syx_Value *make_syx_value_symbol_n(const char *symbol, size_t count) {
  Syx_Value **stored = ht_find(SYX_SYMBOLS(), symbol);
  if (stored) return *stored;
  Syx_Value *value = make_syx_value(SYX_VALUE_KIND_SYMBOL, sizeof(Syx_Symbol) + sizeof(char) * count);
  rc_get(value)->methods.destructor = syx_value_symbol_destructor;
  value->symbol = (Syx_Symbol *)(value + 1);
  value->symbol->data = (char *)(value->symbol + 1);
  value->symbol->count = count;
  memcpy((char *)value->symbol->data, symbol, count);
  *ht_put(SYX_SYMBOLS(), value->symbol->data) = value;
  value->symbol->guarded = false;
  for (size_t index = 0; index < count; index += 1) {
    if (issymbol(symbol[index])) continue;
    value->symbol->guarded = true;
    break;
  }
  return value;
}

inline Syx_Value *make_syx_value_symbol_sv(String_View symbol) {
  return make_syx_value_symbol_n(symbol.data, symbol.count);
}

Syx_Value *make_syx_value_symbolf(const char *format, ...) {
  String_Builder sb = {0};
  va_list args;
  va_start(args, format);
  sb_vappendf(&sb, format, args);
  Syx_Value *value = make_syx_value_symbol_n(sb.data, sb.count);
  va_end(args);
  sb_free(&sb);
  return value;
}

inline Syx_Value *make_syx_value_symbol_cstr(const char *symbol) {
  return make_syx_value_symbol_n(symbol, strlen(symbol));
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

void syx_value_string_destructor(void *data) {
  Syx_Value *value = data;
  free((char *)value->string->data);
}

Syx_Value *make_syx_value_string(Syx_String *string) {
  Syx_Value *value = make_syx_value(SYX_VALUE_KIND_STRING, 0);
  rc_get(value)->methods.destructor = syx_value_string_destructor;
  value->string = string;
  return value;
}

Syx_Value *make_syx_value_string_n_dup(const char *data, size_t count) {
  Syx_Value *value = make_syx_value(SYX_VALUE_KIND_STRING, sizeof(Syx_String) + sizeof(char) * count + 1);
  value->string = (Syx_String *)(value + 1);
  string_assign(value->string, (Syx_String){.data = (const char *const)(value->string + 1), .count = count});
  memcpy((char *)value->string->data, data, count);
  ((char *)value->string->data)[count] = '\0';
  return value;
}

Syx_Value *make_syx_value_string_dup(Syx_String string) {
  return make_syx_value_string_n_dup(string.data, string.count);
}

inline Syx_Value *make_syx_value_string_cstr_dup(const char *data) {
  return make_syx_value_string_n_dup(data, strlen(data));
}

Syx_Value *make_syx_value_stringf_dup(const char *format, ...) {
  va_list args;
  va_start(args, format);
  size_t count = vsnprintf(NULL, 0, format, args);
  va_end(args);
  assert(count >= 0);

  Syx_Value *value = make_syx_value(SYX_VALUE_KIND_STRING, sizeof(Syx_String) + sizeof(char) * count);
  value->string = (Syx_String *)(value + 1);
  string_assign(value->string, (Syx_String){.data = (const char *const)(value->string + 1), .count = count});

  va_start(args, format);
  vsnprintf((char *)value->string->data, count + 1, format, args);
  va_end(args);

  return value;
}

void syx_value_object_destructor(void *data) {
  Syx_Value *value = data;
  rc_release(syx_value_from_object(value->object->proto));
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
  rc_acquire(syx_value_from_object(proto));
  value->object->proto = proto;
  return value;
}

void syx_value_closure_destructor(void *data) {
  Syx_Value *value = data;
  rc_release(syx_value_from_symbol(value->closure->name));
}

Syx_Value *make_syx_value_closure(Syx_Symbol *name, Syx_Closure_Kind kind, size_t additional_size) {
  Syx_Value *value = make_syx_value(SYX_VALUE_KIND_CLOSURE, sizeof(Syx_Closure) + additional_size);
  rc_get(value)->methods.destructor = syx_value_closure_destructor;
  value->closure = (Syx_Closure *)(value + 1);
  value->closure->kind = kind;
  rc_acquire(syx_value_from_symbol(name));
  value->closure->name = name;
  return value;
}

void syx_value_closure_rename(Syx_Closure *closure, Syx_Symbol *name) {
  rc_acquire(syx_value_from_symbol(name));
  rc_release(syx_value_from_symbol(closure->name));
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

void syx_value_closure_native_constructor_destructor(void *data) {
  Syx_Value *value = data;
  rc_release(value->closure->native);
}

Syx_Value *make_syx_value_closure_native_constructor(Syx_Symbol *name, Syx_Type *type) {
  Syx_Value *value = make_syx_value_closure(name, SYX_CLOSURE_KIND_NATIVE_CONSTRUCTOR, 0);
  rc_get(value)->methods.destructor = syx_value_closure_native_constructor_destructor;
  value->closure->native = rc_acquire(type);
  return value;
}

void syx_value_native_destructor(void *data) {
  Syx_Value *value = data;
  rc_release(value->native->parent);
  rc_release(value->native->type);
}

void syx_value_native_structure_destructor(void *data) {
  Syx_Native *native = ((Syx_Value *)data)->native;
  if (native->type->structure->destructor) {
    native->type->structure->destructor(native->data);
  }
  syx_value_native_destructor(data);
}

Syx_Value *make_syx_value_native(Syx_Native *parent, Syx_Type *type, void *data, size_t additional_size) {
  Syx_Value *value = make_syx_value(SYX_VALUE_KIND_NATIVE, sizeof(Syx_Native) + additional_size);
  if (type->kind == SYX_TYPE_KIND_STRUCTURE) {
    rc_get(value)->methods.destructor = syx_value_native_structure_destructor;
  } else {
    rc_get(value)->methods.destructor = syx_value_native_destructor;
  }
  rc_acquire(syx_value_from_native(parent));
  value->native = (Syx_Native *)(value + 1);
  value->native->parent = (parent);
  value->native = (Syx_Native *)(value + 1);
  value->native->type = rc_acquire(type);
  value->native->data = data;
  return value;
}

Syx_Value *make_syx_value_native_instance(Syx_Type *type) {
  Syx_Value *value = make_syx_value_native(NULL, type, NULL, type->size);
  value->native->data = (void *)(value->native + 1);
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
  if (!(*next)) {
    if (cdr != NULL) (*cdr) = NULL;
    return false;
  }
  if ((*next)->kind != SYX_VALUE_KIND_PAIR) {
    if (cdr != NULL) (*cdr) = *next;
    else if ((*next)->kind != SYX_VALUE_KIND_PAIR) UNREACHABLE("list expected");
    return false;
  }
  if (!(*next)->pair) {
    if (cdr != NULL) (*cdr) = NULL;
    return false;
  }
  (*value) = (*next)->pair->left;
  (*current) = (*next);
  (*next) = (*next)->pair->right;
  return true;
}

bool syx_list_map_next(Syx_Value **source_it, Syx_Value ***target_it, Syx_Value ***value, Syx_Value ***cdr) {
  if (!(*source_it)) {
    if (cdr != NULL) (*cdr) = (*target_it);
    else (**target_it) = rc_acquire(syx_value_nil());
    return false;
  }
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

size_t sb_append_syx_symbol(String_Builder *sb, const Syx_Symbol *symbol) {
  Stringify_State state = make_stringify_state(sb, 256);
  if (symbol->guarded) stringify_append(&state, sb_append, '|');
  stringify_append(&state, sb_append_sv, *symbol);
  if (symbol->guarded) stringify_append(&state, sb_append, '|');
  return state.count;
}

size_t sb_append_syx_value(String_Builder *sb, const Syx_Value *value) {
  Stringify_State state = make_stringify_state(sb, 256);
  switch (value->kind) {
    case SYX_VALUE_KIND_PAIR: {
      if (!value->pair) {
        stringify_append(&state, sb_append_strlit, "#n");
      } else {
        stringify_append(&state, sb_append, '(');
        Syx_Pair *pair = value->pair;
        stringify_append(&state, sb_append_syx_value, syx_list_next_nullable(&pair));
        Syx_Value *last = NULL;
        syx_list_for_each(pair, item, &last) {
          stringify_append(&state, sb_append, ' ');
          stringify_append(&state, sb_append_syx_value, syx_list_next_nullable(&pair));
        }
        if (last) {
          stringify_append(&state, sb_append_strlit, " . ");
          stringify_append(&state, sb_append_syx_value, last);
        }
        stringify_append(&state, sb_append, ')');
      }
    } break;
    case SYX_VALUE_KIND_CONST: {
      if (value == syx_value_bool_true()) {
        stringify_append(&state, sb_append_strlit, "#t");
      } else if (value == syx_value_bool_false()) {
        stringify_append(&state, sb_append_strlit, "#f");
      } else {
        UNREACHABLE("unknown constant");
      }
    } break;
    case SYX_VALUE_KIND_SYMBOL: {
      stringify_append(&state, sb_append_syx_symbol, value->symbol);
    } break;
    case SYX_VALUE_KIND_NUMBER: {
      switch (value->number->kind) {
        case SYX_NUMBER_KIND_INTEGER: stringify_append(&state, sb_append_integer, value->number->integer); break;
        case SYX_NUMBER_KIND_FRACTIONAL: stringify_append(&state, sb_append_floating, value->number->fractional); break;
      }
    } break;
    case SYX_VALUE_KIND_STRING: {
      stringify_append(&state, sb_append, '"');
      stringify_append(&state, sb_append_sv, *value->string);
      stringify_append(&state, sb_append, '"');
    } break;
    case SYX_VALUE_KIND_OBJECT: {
      stringify_append(&state, sb_append_strlit, "(object");
      if (value->object->proto) {
        stringify_append(&state, sb_append, ' ');
        stringify_append(&state, sb_append_syx_value, syx_value_from_object(value->object->proto));
      }
      ht_foreach(field, &value->object->fields) {
        stringify_append(&state, sb_append, ' ');
        stringify_append(&state, sb_append, ':');
        Syx_Symbol *name = ht_key(&value->object->fields, field);
        stringify_append(&state, sb_append_syx_value, syx_value_from_symbol(name));
        stringify_append(&state, sb_append, ' ');
        stringify_append(&state, sb_append_syx_value, *field);
      }
      stringify_append(&state, sb_append, ')');
    } break;
    case SYX_VALUE_KIND_CLOSURE: {
      TODO("TASK(20260913-072838): sb_append_syx_value: different closures");
      stringify_append(&state, sb_append_strlit, "<fn ");
      if (value->closure->name) stringify_append(&state, sb_append_syx_symbol, value->closure->name);
      stringify_append(&state, sb_append, '>');
      //   __str_append_with(str_append_syxv, value->closure.defines);
      //   SyxV *it = value->closure.forms;
      //   while (it->kind == SYXV_KIND_PAIR) {
      //     __str_append(' ');
      //     __str_append_with(str_append_syxv, it->pair.left);
      //     it = it->pair.right;
      //   }
      // case SYXV_KIND_CONSTRUCTOR: {
      //   __str_append_cstr("new ");
      //   __str_append_with(str_append_syx_type_info, value->constructor.typeinfo);
      // } break;
    } break;
    case SYX_VALUE_KIND_NATIVE: {
      Syx_Native *native = value->native;
      stringify_append(&state, sb_append, '(');
      if (native->type->name) {
        stringify_append(&state, sb_append_syx_symbol, native->type->name);
      } else {
        stringify_append(&state, sb_append_syx_type, native->type);
      }
      if (native->type->kind != SYX_TYPE_KIND_VOID) {
        stringify_append(&state, sb_append, ' ');
        switch (native->type->kind) {
          case SYX_TYPE_KIND_VOID: UNREACHABLE("should be filtered out already");
          case SYX_TYPE_KIND_PRIMITIVE: {
#define X(type) stringify_append(&state, sb_append_number, *(type *)native->data)
            syx_native_primitive_xy_macro(native->type, X, X);
#undef X
          } break;
          case SYX_TYPE_KIND_STRUCTURE: TODO("TASK(20260913-075944): sb_append_syx_value: structure to string");
          case SYX_TYPE_KIND_PTR:
            if (native->type == SYX_KNOWN_TYPES()->c_value) {
              stringify_append(&state, sb_append_syx_value, *(Syx_Value **)native->data);
              break;
            }
          case SYX_TYPE_KIND_FUNCTION_PTR: {
            stringify_append(&state, sb_append_unsigned_integer, (uintptr_t)(void **)native->data, .kind = SB_INTEGER_FORMAT_KIND_HEX_BIG, .prefix = true, .min_width = sizeof(void *) * 2);
          } break;
        }
      }
      stringify_append(&state, sb_append, ')');
    } break;
    case SYX_VALUE_KIND_EXIT: {
      UNREACHABLE("thrown value can't be converted to string");
    } break;
    case SYX_VALUE_KIND_PREFIXED: {
      stringify_append(&state, sb_append, value->prefixed->kind);
      stringify_append(&state, sb_append_syx_value, value->prefixed->value);
    } break;
  }
  return state.count;
}

#endif // SYX_VALUE_IMPL_C
