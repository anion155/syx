#ifndef SYX_VALUE_H
#define SYX_VALUE_H

#include <ht.h>
#include <magic.h>
#include <nob.h>
#include <rc.h>

#include "syx_utils.h"

typedef enum : unsigned int {
  SYXV_KIND_NIL,
  SYXV_KIND_SYMBOL,
  SYXV_KIND_PAIR,
  SYXV_KIND_BOOL,
  SYXV_KIND_NUMBER,
  SYXV_KIND_STRING,
  SYXV_KIND_BOXED,
  SYXV_KIND_BOXED_METHOD,
  SYXV_KIND_SPECIALF,
  SYXV_KIND_BUILTIN,
  SYXV_KIND_CLOSURE,
  SYXV_KIND_CONSTRUCTOR,
  SYXV_KIND_THROWN,
  SYXV_KIND_RETURN_VALUE,
} SyxV_Kind;

typedef struct SyxV SyxV;
typedef struct Syx_Eval_Ctx Syx_Eval_Ctx;
typedef struct Syx_Env Syx_Env;
typedef struct Syx_Frame Syx_Frame;

typedef struct SyxV_Symbol {
  char *name;
  size_t length;
  bool guarded;
} SyxV_Symbol;

typedef struct SyxV_Pair {
  SyxV *left;
  SyxV *right;
} SyxV_Pair;

typedef struct Syx_SpecialF Syx_SpecialF;
typedef SyxV *(*Syx_SpecialF_Evaluator)(Syx_Eval_Ctx *ctx, Syx_SpecialF *callable, SyxV *arguments);

struct Syx_SpecialF {
  char *name;
  Syx_SpecialF_Evaluator eval;
};

typedef SyxV *(*Syx_Evaluator)(Syx_Eval_Ctx *ctx, SyxV *arguments);

typedef struct Syx_Builtin {
  char *name;
  Syx_Evaluator eval;
} Syx_Builtin;

typedef struct Syx_Closure {
  char *name;
  Syx_Env *env;
  SyxV *defines;
  SyxV *forms;
} Syx_Closure;

typedef struct Syx_Type_Info Syx_Type_Info;

typedef struct Syx_Constructor {
  Syx_Type_Info *typeinfo;
} Syx_Constructor;

typedef struct Syx_Boxed Syx_Boxed;
typedef struct Syx_Boxed_Method Syx_Boxed_Method;

typedef struct SyxV_Thrown {
  SyxV *reason;
  Syx_Frame *stack_frame;
} SyxV_Thrown;

typedef SyxV *(*Syx_LValue)(Syx_Eval_Ctx *ctx, SyxV *target, void *data, SyxV *value);

typedef struct Syx_LValue_Closure {
  Syx_LValue callback;
  void *data;
} Syx_LValue_Closure;

Syx_LValue_Closure *make_syx_lvalue_closure(Syx_LValue callback, void *data);

struct SyxV {
  SyxV_Kind kind;
  Syx_LValue_Closure *lvalue;

  union {
    SyxV_Symbol symbol;
    SyxV_Pair pair;
    syx_bool_t boolean;
    Syx_Number number;
    syx_string_view_t string;
    Syx_Boxed *boxed;
    Syx_Boxed_Method *boxed_method;
    SyxV *quote;
    Syx_SpecialF specialf;
    Syx_Builtin builtin;
    Syx_Closure closure;
    Syx_Constructor constructor;
    SyxV_Thrown thrown;
    SyxV *return_value;
  };
};

void syxv_destructor(void *data);
SyxV *get_syxv_from_symbol(SyxV_Symbol *symbol);
SyxV *make_syxv_nil();
SyxV *make_syxv_symbol(String_View symbol);
SyxV *make_syxv_symbol_n(const char *symbol, size_t size);
SyxV *make_syxv_symbol_cstr(const char *symbol);
SyxV *make_syxv_pair(SyxV *left, SyxV *right);
SyxV *make_syxv_list_opt(size_t count, SyxV **items);
#define make_syxv_list(...)                             \
  make_syxv_list_opt(                                   \
      sizeof((SyxV *[]){__VA_ARGS__}) / sizeof(SyxV *), \
      (SyxV *[]){__VA_ARGS__})
SyxV *make_syxv_bool(syx_bool_t value);
SyxV *make_syxv_number(Syx_Number value);
#define make_syxv_number_integer(value) make_syxv_number(make_syx_number_integer(value))
#define make_syxv_number_fractional(value) make_syxv_number(make_syx_number_fractional(value))
SyxV *make_syxv_string(syx_string_t value);
SyxV *make_syxv_string_copy(syx_string_t value);
SyxV *make_syxv_string_sv(syx_string_view_t value);
SyxV *make_syxv_string_n(const char *value, size_t size);
SyxV *make_syxv_string_cstr(const char *value);
#define make_syxv_value(value)                              \
  _Generic(value,                                           \
      syx_bool_t: make_syxv_bool(value),                    \
      Syx_Number: make_syxv_number(value),                  \
      syx_integer_t: make_syxv_number_integer(value),       \
      syx_fractional_t: make_syxv_number_fractional(value), \
      syx_string_t: make_syxv_string_copy(value),           \
      syx_string_view_t: make_syxv_string_sv(value),        \
      const char *: make_syxv_string_cstr(value))
SyxV *make_syxv_boxed(Syx_Boxed *boxed);
SyxV *make_syxv_boxed_method(Syx_Boxed_Method *method);
SyxV *make_syxv_specialf(const char *name, Syx_SpecialF_Evaluator eval);
SyxV *make_syxv_builtin(const char *name, Syx_Evaluator eval);
SyxV *make_syxv_closure(const char *name, SyxV *defines, SyxV *body, Syx_Env *env);
SyxV *make_syxv_constructor(Syx_Type_Info *typeinfo);
SyxV *make_syxv_thrown(Syx_Frame *stack_frame, SyxV *reason);
SyxV *make_syxv_return_value(SyxV *return_value);

SyxV *syxv_list_next_nullable(SyxV **list);
SyxV *syxv_list_next(SyxV **list);

bool syxv__list_for_each_next(SyxV **current, SyxV **next, SyxV **value, SyxV **cdr);
#define syxv_list_for_each(value, list, ...)                   \
  for (SyxV * value##_current, *value##_next = (list), *value; \
       syxv__list_for_each_next(                               \
           &value##_current,                                   \
           &value##_next,                                      \
           &value,                                             \
           WITH_DEFAULT(NULL, __VA_ARGS__));)

bool syxv__list_map_next(SyxV **source_it, SyxV ***target_it, SyxV ***value, SyxV ***cdr);
#define syxv_list_map(value, list, results, ...) \
  for (SyxV *value##_source_it = (list),         \
            **value##_target_it = (results),     \
            **value = NULL;                      \
       syxv__list_map_next(                      \
           &value##_source_it,                   \
           &value##_target_it,                   \
           &value,                               \
           WITH_DEFAULT(NULL, __VA_ARGS__));)

size_t stringify_syxv_symbol_n(char *string, SyxV_Symbol *symbol);
// syx_string_t stringify_syxv_symbol(SyxV_Symbol *symbol);
// void sb_append_syxv_symbol(String_Builder *sb, SyxV_Symbol *symbol);

typedef Ht(SyxV *, String_Builder, SyxV_Stringify_Cache) SyxV_Stringify_Cache;
size_t stringify__syxv_n(char *string, SyxV *value, SyxV_Stringify_Cache *cache);
#define stringify_syxv_n(string, value, ...) stringify__syxv_n((string), (value), WITH_DEFAULT(NULL, __VA_ARGS__))
syx_string_t stringify__syxv(SyxV *value, SyxV_Stringify_Cache *cache);
#define stringify_syxv(value, ...) stringify__syxv((value), WITH_DEFAULT(NULL, __VA_ARGS__))
void sb_append__syxv(String_Builder *sb, SyxV *value, SyxV_Stringify_Cache *cache);
#define sb_append_syxv(sb, value, ...) sb_append__syxv((sb), (value), WITH_DEFAULT(NULL, __VA_ARGS__))

void fprint_syxv(FILE *f, SyxV *value);
#define print_syxv(value) fprint_syxv(stdout, (value))

#endif // SYX_VALUE_H

#if defined(SYX_VALUE_IMPL) && !defined(SYX_VALUE_IMPL_C)
#define SYX_VALUE_IMPL_C

#define HT_IMPL
#include <ht.h>
#define NOB_IMPL
#include <nob.h>
#define RC_IMPL
#include <rc.h>

#define SYX_UTILS_IMPL
#include "syx_utils.h"
#define SYX_TYPE_INFO_IMPL
#include "syx_type_info.h"

void syx_lvalue_closure_destructor(void *data) {
  Syx_LValue_Closure *lvalue = data;
  free(lvalue->data);
}

Syx_LValue_Closure *make_syx_lvalue_closure(Syx_LValue callback, void *data) {
  Syx_LValue_Closure *lvalue = rc_malloc(sizeof(Syx_LValue_Closure), .destructor = syx_lvalue_closure_destructor);
  lvalue->callback = callback;
  lvalue->data = data;
  return lvalue;
}

SyxV *make_syxv(SyxV_Kind kind) {
  SyxV *value = rc_malloc(sizeof(SyxV), .destructor = syxv_destructor);
  assert(value);
  memset(value, 0, sizeof(SyxV));
  value->kind = kind;
  return value;
}

define_constant(struct { SyxV *nil; SyxV *bool_true; SyxV *bool_false; }, SYXV_CONSTANTS) {
  SYXV_CONSTANTS->nil = rc_acquire(make_syxv(SYXV_KIND_NIL));

  SYXV_CONSTANTS->bool_true = rc_acquire(make_syxv(SYXV_KIND_BOOL));
  SYXV_CONSTANTS->bool_true->boolean = true;

  SYXV_CONSTANTS->bool_false = rc_acquire(make_syxv(SYXV_KIND_BOOL));
  SYXV_CONSTANTS->bool_false->boolean = false;
}

define_constant(Ht(const char *, SyxV *), SYXV_SYMBOLS) {
  SYXV_SYMBOLS->hasheq = ht_cstr_hasheq;
}

void syxv_destructor(void *data) {
  SyxV *syxv = data;
  if (syxv->lvalue) rc_release(syxv->lvalue);
  switch (syxv->kind) {
    case SYXV_KIND_NIL: break;
    case SYXV_KIND_SYMBOL: {
      SyxV **stored = ht_find(SYXV_SYMBOLS(), syxv->symbol.name);
      if (stored) ht_delete(SYXV_SYMBOLS(), stored);
      free(syxv->symbol.name);
    } break;
    case SYXV_KIND_PAIR:
      if (syxv->pair.left) rc_release(syxv->pair.left);
      if (syxv->pair.right) rc_release(syxv->pair.right);
      break;
    case SYXV_KIND_BOOL: break;
    case SYXV_KIND_NUMBER: break;
    case SYXV_KIND_STRING: free((char *)syxv->string.data); break;
    case SYXV_KIND_BOXED: rc_release(syxv->boxed); break;
    case SYXV_KIND_BOXED_METHOD: rc_release(syxv->boxed_method); break;
    case SYXV_KIND_SPECIALF: {
      if (syxv->specialf.name) free(syxv->specialf.name);
    } break;
    case SYXV_KIND_BUILTIN: {
      if (syxv->builtin.name) free(syxv->builtin.name);
    } break;
    case SYXV_KIND_CLOSURE: {
      if (syxv->closure.name) free(syxv->closure.name);
      if (syxv->closure.defines) rc_release(syxv->closure.defines);
      if (syxv->closure.forms) rc_release(syxv->closure.forms);
    } break;
    case SYXV_KIND_CONSTRUCTOR: {
      rc_release(syxv->constructor.typeinfo);
    } break;
    case SYXV_KIND_THROWN: {
      if (syxv->thrown.stack_frame) rc_release(syxv->thrown.stack_frame);
      if (syxv->thrown.reason) rc_release(syxv->thrown.reason);
    } break;
    case SYXV_KIND_RETURN_VALUE: rc_release(syxv->return_value); break;
  }
}

SyxV *get_syxv_from_symbol(SyxV_Symbol *symbol) {
  return (SyxV *)((char *)symbol - offsetof(SyxV, symbol));
};

SyxV *make_syxv_nil() {
  return SYXV_CONSTANTS()->nil;
}

SyxV *make_syxv_symbol(String_View name) {
  SyxV **syxv = ht_find(SYXV_SYMBOLS(), name.data);
  if (!syxv) {
    char *key = strndup(name.data, name.count);
    syxv = ht_put(SYXV_SYMBOLS(), key);
    *syxv = make_syxv(SYXV_KIND_SYMBOL);
    (*syxv)->symbol.name = key;
    (*syxv)->symbol.length = name.count;
    for (String_View it = name; it.count; sv_chop_left(&it, 1)) {
      if (issymbol(*it.data)) continue;
      (*syxv)->symbol.guarded = true;
      break;
    }
  }
  return (*syxv);
}

SyxV *make_syxv_symbol_n(const char *symbol, size_t size) {
  return make_syxv_symbol((String_View){.data = symbol, .count = size});
}

SyxV *make_syxv_symbol_cstr(const char *symbol) {
  return make_syxv_symbol(sv_from_cstr(symbol));
}

SyxV *make_syxv_pair(SyxV *left, SyxV *right) {
  SyxV *syxv = make_syxv(SYXV_KIND_PAIR);
  syxv->pair.left = left ? rc_acquire(left) : NULL;
  syxv->pair.right = right ? rc_acquire(right) : NULL;
  return syxv;
}

SyxV *make_syxv_list_opt(size_t count, SyxV **items) {
  if (!count) UNREACHABLE("empty list array must contain [NULL]");
  SyxV *expr = items[count - 1] == NULL ? make_syxv_nil() : items[count - 1];
  for (ssize_t index = (ssize_t)count - 2; index >= 0; index -= 1) {
    expr = make_syxv_pair(items[index], expr);
  }
  return expr;
}

SyxV *make_syxv_bool(syx_bool_t value) {
  return value ? SYXV_CONSTANTS()->bool_true : SYXV_CONSTANTS()->bool_false;
}

SyxV *make_syxv_number(Syx_Number value) {
  SyxV *syxv = make_syxv(SYXV_KIND_NUMBER);
  syxv->number = value;
  return syxv;
}

SyxV *make_syxv_string(syx_string_t value) {
  SyxV *syxv = make_syxv(SYXV_KIND_STRING);
  syxv->string.data = value.items;
  syxv->string.count = value.count;
  return syxv;
}

SyxV *make_syxv_string_copy(syx_string_t value) {
  SyxV *syxv = make_syxv(SYXV_KIND_STRING);
  syxv->string.data = strndup(value.items, value.count);
  syxv->string.count = value.count;
  return syxv;
}

SyxV *make_syxv_string_sv(syx_string_view_t value) {
  SyxV *syxv = make_syxv(SYXV_KIND_STRING);
  syxv->string.data = strndup(value.data, value.count);
  syxv->string.count = value.count;
  return syxv;
}

SyxV *make_syxv_string_n(const char *value, size_t size) {
  SyxV *syxv = make_syxv(SYXV_KIND_STRING);
  syxv->string.data = strndup(value, size);
  syxv->string.count = size;
  return syxv;
}

SyxV *make_syxv_string_cstr(const char *value) {
  return make_syxv_string_sv(sv_from_cstr(value));
}

SyxV *make_syxv_boxed(Syx_Boxed *boxed) {
  SyxV *value = make_syxv(SYXV_KIND_BOXED);
  if (boxed) value->boxed = rc_acquire(boxed);
  return value;
}

SyxV *make_syxv_boxed_method(Syx_Boxed_Method *method) {
  SyxV *value = make_syxv(SYXV_KIND_BOXED_METHOD);
  if (method) value->boxed_method = rc_acquire(method);
  return value;
}

SyxV *make_syxv_specialf(const char *name, Syx_SpecialF_Evaluator eval) {
  SyxV *value = make_syxv(SYXV_KIND_SPECIALF);
  value->specialf.name = name ? strdup(name) : NULL;
  value->specialf.eval = eval;
  return value;
}

SyxV *make_syxv_builtin(const char *name, Syx_Evaluator eval) {
  SyxV *value = make_syxv(SYXV_KIND_BUILTIN);
  value->builtin.name = name ? strdup(name) : NULL;
  value->builtin.eval = eval;
  return value;
}

SyxV *make_syxv_closure(const char *name, SyxV *defines, SyxV *forms, Syx_Env *env) {
  SyxV *value = make_syxv(SYXV_KIND_CLOSURE);
  value->closure.name = name ? strdup(name) : NULL;
  value->closure.defines = defines ? rc_acquire(defines) : NULL;
  value->closure.forms = forms ? rc_acquire(forms) : NULL;
  value->closure.env = env;
  return value;
}

SyxV *make_syxv_constructor(Syx_Type_Info *typeinfo) {
  SyxV *value = make_syxv(SYXV_KIND_CONSTRUCTOR);
  value->constructor.typeinfo = rc_acquire(typeinfo);
  return value;
}

SyxV *make_syxv_thrown(Syx_Frame *stack_frame, SyxV *reason) {
  SyxV *value = make_syxv(SYXV_KIND_THROWN);
  value->thrown.stack_frame = stack_frame ? rc_acquire(stack_frame) : NULL;
  value->thrown.reason = reason ? rc_acquire(reason) : NULL;
  return value;
}

SyxV *make_syxv_return_value(SyxV *return_value) {
  SyxV *value = make_syxv(SYXV_KIND_RETURN_VALUE);
  value->return_value = return_value ? rc_acquire(return_value) : NULL;
  return value;
}

SyxV *syxv_list_next_nullable(SyxV **list) {
  if (!(*list)) return NULL;
  if ((*list)->kind != SYXV_KIND_PAIR) {
    SyxV *value = (*list);
    (*list) = NULL;
    return value;
  }
  SyxV *value = (*list)->pair.left;
  (*list) = (*list)->pair.right;
  return value;
}

SyxV *syxv_list_next(SyxV **list) {
  SyxV *item = syxv_list_next_nullable(list);
  if (!item) return make_syxv_nil();
  return item;
}

bool syxv__list_for_each_next(SyxV **current, SyxV **next, SyxV **value, SyxV **cdr) {
  if (!(*next) || (*next)->kind != SYXV_KIND_PAIR) {
    if (cdr != NULL) (*cdr) = *next;
    else if ((*next)->kind != SYXV_KIND_NIL) UNREACHABLE("list expected");
    return false;
  }
  (*value) = (*next)->pair.left;
  (*current) = (*next);
  (*next) = (*next)->pair.right;
  return true;
}

bool syxv__list_map_next(SyxV **source_it, SyxV ***target_it, SyxV ***value, SyxV ***cdr) {
  if (*value) rc_acquire(**value);
  if ((*source_it)->kind != SYXV_KIND_PAIR) {
    if (cdr != NULL) (*cdr) = (*target_it);
    else if ((*source_it)->kind != SYXV_KIND_NIL) UNREACHABLE("list expected");
    else (**target_it) = rc_acquire(make_syxv_nil());
    return false;
  }
  (**target_it) = rc_acquire(make_syxv_pair(NULL, NULL));
  (*value) = &((**target_it)->pair.left);
  (**value) = (*source_it)->pair.left;
  (*source_it) = (*source_it)->pair.right;
  (*target_it) = &((**target_it)->pair.right);
  return true;
}

size_t stringify_syxv_symbol_n(char *string, SyxV_Symbol *symbol) {
  if (!symbol->name) return 0;
  __str_it();
  if (symbol->guarded) {
    __str_push('|');
    __str_convert(stringify_string_n, symbol->name, symbol->length);
    __str_push('|');
  } else {
    __str_convert(stringify_string_n, symbol->name, symbol->length);
  }
  return __str_width();
}

void syxv_stringify_cache_destructor(void *data) {
  SyxV_Stringify_Cache *cache = data;
  ht_foreach(item, cache) {
    rc_release(ht_key(cache, item));
    sb_free(*item);
  }
  ht_free(cache);
}

SyxV_Stringify_Cache *make_syxv_stringify_cache() {
  SyxV_Stringify_Cache *cache = rc_malloc(sizeof(SyxV_Stringify_Cache), .destructor = syxv_stringify_cache_destructor);
  assert(cache);
  memset(cache, 0, sizeof(SyxV_Stringify_Cache));
  return cache;
}

size_t stringify__cached_syxv(char *string, SyxV *value, SyxV_Stringify_Cache *cache) {
  if (!cache) return stringify__syxv_n(string, value, NULL);
  syx_string_t *cached = ht_find(cache, value);
  if (cached != NULL) {
    if (string) memcpy(string, cached->items, cached->count);
    return cached->count;
  }
  syx_string_t sb = stringify__syxv(value, cache);
  cached = ht_put(cache, rc_acquire(value));
  *cached = sb;
  if (string) memcpy(string, cached->items, cached->count);
  return cached->count;
}

size_t stringify__syxv_n(char *string, SyxV *value, SyxV_Stringify_Cache *cache) {
  syx_string_t *cached = cache ? ht_find(cache, value) : NULL;
  if (cached) {
    memcpy(string, cached->items, cached->count);
    return cached->count;
  }
  __str_it();
  switch (value->kind) {
    case SYXV_KIND_NIL: {
      __str_push_cstr("#nil");
    } break;
    case SYXV_KIND_SYMBOL: {
      __str_convert(stringify_syxv_symbol_n, &value->symbol);
    } break;
    case SYXV_KIND_PAIR: {
      __str_push('(');
      SyxV *it = value;
      SyxV *last_left = it->pair.left;
      __str_convert(stringify__cached_syxv, last_left, cache);
      it = it->pair.right;
      while (it->kind == SYXV_KIND_PAIR) {
        __str_push(' ');
        last_left = it->pair.left;
        __str_convert(stringify__cached_syxv, last_left, cache);
        it = it->pair.right;
      }
      if (it->kind != SYXV_KIND_NIL) {
        __str_push(' ');
        __str_push('.');
        __str_push(' ');
        __str_convert(stringify__cached_syxv, it, cache);
      }
      __str_push(')');
    } break;
    case SYXV_KIND_BOOL: {
      __str_push('#');
      if (value->boolean) __str_push_cstr("true");
      else __str_push_cstr("false");
    } break;
    case SYXV_KIND_NUMBER: {
      __str_convert(stringify_number_n, value->number);
    } break;
    case SYXV_KIND_STRING: {
      __str_push('"');
      __str_convert(stringify_string_n, value->string.data, value->string.count);
      __str_push('"');
    } break;
    case SYXV_KIND_BOXED: {
      __str_convert(stringify_syx_boxed_n, value->boxed);
    } break;
    case SYXV_KIND_BOXED_METHOD: {
      __str_convert(stringify_syx_boxed_method_n, value->boxed_method);
    } break;
    case SYXV_KIND_SPECIALF: {
      __str_push('?');
      __str_push('<');
      if (value->specialf.name) __str_push_cstr(value->specialf.name);
      __str_push('>');
    } break;
    case SYXV_KIND_BUILTIN: {
      __str_push('!');
      __str_push('<');
      if (value->builtin.name) __str_push_cstr(value->builtin.name);
      __str_push('>');
    } break;
    case SYXV_KIND_CLOSURE: {
      __str_push('(');
      __str_push('$');
      __str_push('<');
      if (value->closure.name) __str_push_cstr(value->closure.name);
      __str_push('>');
      __str_push(' ');
      __str_convert(stringify__cached_syxv, value->closure.defines, cache);
      SyxV *it = value->closure.forms;
      while (it->kind == SYXV_KIND_PAIR) {
        __str_push(' ');
        __str_convert(stringify__cached_syxv, it->pair.left, cache);
        it = it->pair.right;
      }
      __str_push(')');
    } break;
    case SYXV_KIND_CONSTRUCTOR: {
      __str_push_cstr("new ");
      __str_convert(stringify_syx_type_info_n, value->constructor.typeinfo);
    } break;
    case SYXV_KIND_THROWN: UNREACHABLE("thrown object can't be converted");
    case SYXV_KIND_RETURN_VALUE: UNREACHABLE("return value object can't be convderted");
  }
  return __str_width();
}

syx_string_t stringify__syxv(SyxV *value, SyxV_Stringify_Cache *cache) {
  SyxV_Stringify_Cache *_cache;
  if (cache) _cache = rc_acquire(cache);
  else _cache = rc_acquire(make_syxv_stringify_cache());

  size_t width = stringify__syxv_n(NULL, value, _cache);
  syx_string_t result = {.items = malloc(width + 1), .count = width, .capacity = width + 1};
  result.items[width] = 0;
  stringify__syxv_n(result.items, value, _cache);

  rc_release(_cache);
  return result;
}

void sb_append__syxv(String_Builder *sb, SyxV *value, SyxV_Stringify_Cache *cache) {
  SyxV_Stringify_Cache *_cache;
  if (cache) _cache = rc_acquire(cache);
  else _cache = rc_acquire(make_syxv_stringify_cache());

  size_t width = stringify__syxv_n(NULL, value, _cache);
  da_realloc_capacity(sb, sb->count + width);
  sb->count += stringify__syxv_n(sb->items + sb->count, value, _cache);

  rc_release(_cache);
}

void fprint_syxv(FILE *f, SyxV *value) {
  syx_string_t sb = stringify_syxv(value);
  fprintf(f, "%.*s", (int)sb.count, sb.items);
  sb_free(sb);
}

#endif // SYX_VALUE_IMPL
