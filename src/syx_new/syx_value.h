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

typedef struct SyxV SyxV;
typedef struct Syx_Exit Syx_Exit;
typedef struct Syx_Pair Syx_Pair;
typedef struct Syx_Symbol Syx_Symbol;
typedef enum Syx_Const Syx_Const;
typedef struct Syx_Number Syx_Number;
typedef struct Syx_String Syx_String;
typedef struct Syx_Closure Syx_Closure;

typedef enum SyxV_Kind : unsigned int {
  SYXV_KIND_NIL,
  SYXV_KIND_EXIT,
  SYXV_KIND_PAIR,
  SYXV_KIND_SYMBOL,
  SYXV_KIND_CONST,
  SYXV_KIND_NUMBER,
  SYXV_KIND_STRING,
  SYXV_KIND_CLOSURE,
  // SYXV_KIND_NATIVE,
} SyxV_Kind;

typedef struct SyxV {
  SyxV_Kind kind;

  union {
    Syx_Exit *exit;
    Syx_Pair *pair;
    Syx_Symbol *symbol;
    Syx_Const constant;
    Syx_Number *number;
    Syx_String *string;
    Syx_Closure *closure;
    // native;
  };
} SyxV;

typedef enum Syx_Exit_Kind : unsigned int {
  SYX_EXIT_KIND_RETURNED,
  SYX_EXIT_KIND_THROWN,
} Syx_Exit_Kind;

typedef struct Syx_Exit_Thrown {
  SyxV *reason;
  Syx_Frame *stack_frame;
} Syx_Exit_Thrown;

typedef struct Syx_Exit {
  Syx_Exit_Kind kind;

  union {
    SyxV *returned;
    Syx_Exit_Thrown *thrown;
  };
} Syx_Exit;

typedef struct Syx_Pair {
  SyxV *left;
  SyxV *right;
} Syx_Pair;

typedef struct Syx_Symbol {
  const char *data;
  size_t count;
  bool guarded;
} Syx_Symbol;

typedef enum Syx_Const : unsigned int {
  SYX_CONST_TRUE,
  SYX_CONST_FALSE,
} Syx_Const;

typedef enum Syx_Number_Kind : unsigned int {
  SYX_NUMBER_KIND_INTEGER,
  SYX_NUMBER_KIND_FRACTIONAL,
} Syx_Number_Kind;

typedef struct Syx_Number {
  Syx_Number_Kind kind;

  union {
    unsigned int integer;
    double fractional;
  };
} Syx_Number;

typedef struct Syx_String {
  const char *data;
  size_t count;
} Syx_String;

typedef SyxV *(*Syx_Closure_Special_Form)(Syx_Eval_Ctx *ctx, SyxV *arguments);
typedef SyxV *(*Syx_Closure_Builtin)(Syx_Eval_Ctx *ctx, SyxV *arguments);
typedef struct Syx_Closure_Lambda Syx_Closure_Lambda;

typedef enum Syx_Closure_Kind : unsigned int {
  SYX_CLOSURE_KIND_SPECIALF,
  SYX_CLOSURE_KIND_BUILTIN,
  SYX_CLOSURE_KIND_LAMBDA,
  // SYX_CLOSURE_KIND_NATIVE_CONSTRUCTOR,
} Syx_Closure_Kind;

typedef struct Syx_Closure {
  Syx_Closure_Kind kind;

  union {
    Syx_Closure_Special_Form specialf;
    Syx_Closure_Builtin builtin;
    Syx_Closure_Lambda *lambda;
    // *native_constructor;
  };
} Syx_Closure;

typedef struct Syx_Closure_Lambda {
  syx_string_view name;
  Syx_Env *env;
  SyxV *defines;
  SyxV *forms;
} Syx_Closure_Lambda;

SyxV *syxv_nil();
SyxV *make_syxv_exit_returned(SyxV *returned);
SyxV *make_syxv_exit_thrown(SyxV *reason, Syx_Frame *stack_frame);
SyxV *make_syxv_pair(SyxV *left, SyxV *right);
SyxV *make_syxv_symbol(syx_string_view name);
SyxV *make_syxv_symbol_n(const char *symbol, size_t size);
SyxV *make_syxv_symbol_cstr(const char *symbol);
#define make_syxv_symbol_strlit(symbol) make_syxv_symbol((String_View){.data = (symbol), .count = sizeof(symbol) - 1})
SyxV *make_syxv_number(Syx_Number number);
SyxV *make_syxv_number_integer(unsigned int value);
SyxV *make_syxv_number_fractional(double value);
SyxV *make_syxv_string(Syx_String string);
SyxV *make_syxv_string_n(const char *data, size_t count);
#define make_syxv_string_lit(string) make_syxv_string((Syx_String){.data = (string), .count = sizeof(string) - 1, .managed = false});
SyxV *make_syxv_string_dup(const char *data, size_t count);
SyxV *make_syxv_closure_specialf(Syx_Closure_Special_Form specialf);
SyxV *make_syxv_closure_builtin(Syx_Closure_Builtin builtin);
SyxV *make_syxv_closure_lambda(Syx_Closure_Lambda lambda);

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

int issymbol_special(int c) {
  return (
      c == '#' || c == '?' || c == '@' || c == '!' || c == '$' || c == '+' || c == '-' || c == '*' || c == '/' || c == '=' || c == '<' || c == '>');
}

int issymbol(int c) {
  return c == '-' || c == '_' || issymbol_special(c) || isalnum(c);
}

define_constant(struct { SyxV *nil; SyxV *bool_true; SyxV *bool_false; }, SYXV_CONSTANTS) {
  SYXV_CONSTANTS->nil = rc_acquire(make_syxv(SYXV_KIND_NIL, 0));

  SYXV_CONSTANTS->bool_true = rc_acquire(make_syxv(SYXV_KIND_CONST, 0));
  SYXV_CONSTANTS->bool_true->constant = SYX_CONST_TRUE;

  SYXV_CONSTANTS->bool_false = rc_acquire(make_syxv(SYXV_KIND_CONST, 0));
  SYXV_CONSTANTS->bool_false->constant = SYX_CONST_FALSE;
}

SyxV *make_syxv(SyxV_Kind kind, size_t size) {
  SyxV *value = rc_acquire(rc_malloc(sizeof(SyxV) + size));
  assert(value);
  value->kind = kind;
  return value;
}

static inline SyxV *syxv_nil() {
  return SYXV_CONSTANTS()->nil;
}

void syxv_exit_returned_destructor(void *data) {
  SyxV *value = data;
  rc_release(value->exit->returned);
}

SyxV *make_syxv_exit_returned(SyxV *returned) {
  SyxV *value = make_syxv(SYXV_KIND_EXIT, sizeof(Syx_Exit));
  rc_get(value)->methods.destructor = syxv_exit_returned_destructor;
  value->exit = (Syx_Exit *)(value + 1);
  value->exit->kind = SYX_EXIT_KIND_RETURNED;
  value->exit->returned = returned;
  return value;
}

void syxv_exit_thrown_destructor(void *data) {
  SyxV *value = data;
  rc_release(value->exit->thrown->reason);
  rc_release(value->exit->thrown->stack_frame);
}

SyxV *make_syxv_exit_thrown(SyxV *reason, Syx_Frame *stack_frame) {
  SyxV *value = make_syxv(SYXV_KIND_EXIT, sizeof(Syx_Exit) + sizeof(Syx_Exit_Thrown));
  rc_get(value)->methods.destructor = syxv_exit_thrown_destructor;
  value->exit = (Syx_Exit *)(value + 1);
  value->exit->kind = SYX_EXIT_KIND_THROWN;
  value->exit->thrown = (Syx_Exit_Thrown *)(value->exit + 1);
  value->exit->thrown->reason = reason;
  value->exit->thrown->stack_frame = stack_frame;
  return value;
}

void syxv_pair_destructor(void *data) {
  SyxV *value = data;
  rc_release(value->pair->left);
  rc_release(value->pair->right);
}

SyxV *make_syxv_pair(SyxV *left, SyxV *right) {
  SyxV *value = make_syxv(SYXV_KIND_PAIR, sizeof(Syx_Pair));
  rc_get(value)->methods.destructor = syxv_pair_destructor;
  value->pair = (Syx_Pair *)(value + 1);
  value->pair->left = rc_acquire(left);
  value->pair->right = rc_acquire(right);
  return value;
}

SyxV *make_syxv_list_opt(size_t count, SyxV **items) {
  if (!count) UNREACHABLE("empty list array must contain [NULL]");
  SyxV *expr = items[count - 1] == NULL ? syxv_nil() : items[count - 1];
  for (ssize_t index = (ssize_t)count - 2; index >= 0; index -= 1) {
    expr = make_syxv_pair(items[index], expr);
  }
  return expr;
}

define_constant(Ht(const char *, SyxV *), SYXV_SYMBOLS) {
  SYXV_SYMBOLS->hasheq = ht_cstr_hasheq;
}

void syxv_symbol_destructor(void *data) {
  SyxV *value = data;
  SyxV **stored = ht_find(SYXV_SYMBOLS(), value->symbol->data);
  if (stored) ht_delete(SYXV_SYMBOLS(), stored);
  free(value->symbol->data);
}

SyxV *make_syxv_symbol(syx_string_view symbol) {
  SyxV **syxv = ht_find(SYXV_SYMBOLS(), symbol.data);
  if (syxv) return *syxv;
  SyxV *value = *syxv = make_syxv(SYXV_KIND_SYMBOL, sizeof(Syx_Symbol));
  rc_get(value)->methods.destructor = syxv_symbol_destructor;
  value->symbol = (Syx_Symbol *)(value + 1);
  value->symbol->data = symbol.data;
  value->symbol->count = symbol.count;
  value->symbol->guarded = false;
  for (syx_string_view it = symbol; it.count; sv_chop_left(&it, 1)) {
    if (issymbol(*it.data)) continue;
    value->symbol->guarded = true;
    break;
  }
  return value;
}

static inline SyxV *make_syxv_symbol_n(const char *symbol, size_t count) {
  return make_syxv_symbol((syx_string_view){.data = symbol, .count = count});
}

static inline SyxV *make_syxv_symbol_cstr(const char *symbol) {
  return make_syxv_symbol((syx_string_view){.data = symbol, .count = strlen(symbol)});
}

static inline SyxV *syxv_bool_false() {
  return SYXV_CONSTANTS()->bool_false;
}

static inline SyxV *syxv_bool_true() {
  return SYXV_CONSTANTS()->bool_true;
}

SyxV *make_syxv_number(Syx_Number number) {
  SyxV *value = make_syxv(SYXV_KIND_NUMBER, sizeof(Syx_Number));
  value->number = (Syx_Number *)(value + 1);
  (*value->number) = number;
  return value;
}

static inline SyxV *make_syxv_number_integer(unsigned int value) {
  return make_syxv_number((Syx_Number){.kind = SYX_NUMBER_KIND_INTEGER, .integer = value});
}

static inline SyxV *make_syxv_number_fractional(double value) {
  return make_syxv_number((Syx_Number){.kind = SYX_NUMBER_KIND_FRACTIONAL, .fractional = value});
}

SyxV *make_syxv_string(Syx_String string) {
  SyxV *value = make_syxv(SYXV_KIND_NUMBER, sizeof(Syx_String));
  value->string = (Syx_String *)(value + 1);
  (*value->string) = string;
  return value;
}

static inline SyxV *make_syxv_string_n(const char *data, size_t count) {
  return make_syxv_string((Syx_String){.data = data, .count = count});
}

void syxv_string_managed_destructor(void *data) {
  SyxV *value = data;
  free((char *)value->string->data);
}

SyxV *make_syxv_string_dup(const char *data, size_t count) {
  SyxV *value = make_syxv(SYXV_KIND_NUMBER, sizeof(Syx_String) + sizeof(char) * count);
  rc_get(value)->methods.destructor = syxv_string_managed_destructor;
  value->string = (Syx_String *)(value + 1);
  value->string->data = (const char *)(value->string + 1);
  memcpy((char *)value->string->data, data, count);
  return value;
}

SyxV *make_syxv_closure_specialf(Syx_Closure_Special_Form specialf) {
  SyxV *value = make_syxv(SYXV_KIND_CLOSURE, sizeof(Syx_Closure));
  value->closure = (Syx_Closure *)(value + 1);
  value->closure->specialf = specialf;
  return value;
}

SyxV *make_syxv_closure_builtin(Syx_Closure_Builtin builtin) {
  SyxV *value = make_syxv(SYXV_KIND_CLOSURE, sizeof(Syx_Closure));
  value->closure = (Syx_Closure *)(value + 1);
  value->closure->builtin = builtin;
  return value;
}

void syxv_closure_lambda_destructor(void *data) {
  SyxV *value = data;
  rc_release(value->closure->lambda->defines);
  rc_release(value->closure->lambda->forms);
}

void syxv_closure_lambda_graph_visitor(Rc_Circulars *circulars, const void *data, const void *source) {
  const SyxV *value = data;
  if (value->kind != SYXV_KIND_CLOSURE && value->closure->kind != SYX_CLOSURE_KIND_LAMBDA) return;
  const Syx_Closure_Lambda *lambda = value->closure->lambda;
  rc_graph_visitor(circulars, (void **)&lambda->env, source);
}

SyxV *make_syxv_closure_lambda(Syx_Closure_Lambda lambda) {
  SyxV *value = make_syxv(SYXV_KIND_CLOSURE, sizeof(Syx_Closure) + sizeof(Syx_Closure_Lambda));
  rc_get(value)->methods = (Rc_Methods){.destructor = syxv_closure_lambda_destructor, .graph_visitor = syxv_closure_lambda_graph_visitor};
  value->closure = (Syx_Closure *)(value + 1);
  value->closure->lambda = (Syx_Closure_Lambda *)(value->closure + 1);
  value->closure->lambda->env = rc_acquire(lambda.env);
  value->closure->lambda->defines = rc_acquire(lambda.defines);
  value->closure->lambda->forms = rc_acquire(lambda.forms);
  return value;
}

#endif // SYX_VALUE_IMPL_C
