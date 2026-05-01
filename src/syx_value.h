#ifndef SYX_VALUE_H
#define SYX_VALUE_H

#include <ht.h>
#include <magic.h>
#include <nob.h>
#include <rc.h>

#include "syx_utils.h"

typedef struct SyxV SyxV;
typedef struct Syx_Env Syx_Env;

typedef SyxV *(*Syx_Evaluator)(Syx_Env *env, SyxV *arguments);

typedef struct Syx_SpecialF {
  char *name;
  Syx_Evaluator eval;
} Syx_SpecialF;

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

typedef enum : unsigned int {
  SYXV_KIND_NIL,
  SYXV_KIND_SYMBOL,
  SYXV_KIND_PAIR,
  SYXV_KIND_BOOL,
  SYXV_KIND_INTEGER,
  SYXV_KIND_FRACTIONAL,
  SYXV_KIND_STRING,
  SYXV_KIND_QUOTE,
  SYXV_KIND_SPECIALF,
  SYXV_KIND_BUILTIN,
  SYXV_KIND_CLOSURE,
} SyxV_Kind;

typedef struct SyxV_Symbol {
  char *name;
  bool guarded;
} SyxV_Symbol;

typedef struct SyxV_Pair {
  SyxV *left;
  SyxV *right;
} SyxV_Pair;

struct SyxV {
  SyxV_Kind kind;

  union {
    SyxV_Symbol symbol;
    SyxV_Pair pair;
    syx_bool_t boolean;
    syx_integer_t integer;
    syx_fractional_t fractional;
    syx_string_t string;
    SyxV *quote;
    Syx_SpecialF specialf;
    Syx_Builtin builtin;
    Syx_Closure closure;
  };
};

typedef struct SyxVs {
  SyxV **items;
  size_t count;
  size_t capacity;
} SyxVs;

void syxv_destructor(void *data);
SyxV *make_syxv_nil();
SyxV *make_syxv_symbol(String_View symbol);
SyxV *make_syxv_symbol_n(char *symbol, size_t size);
SyxV *make_syxv_symbol_cstr(char *symbol);
SyxV *make_syxv_pair(SyxV *left, SyxV *right);
SyxV *make_syxv_list_opt(size_t count, SyxV **items);
#define make_syxv_list(...)                             \
  make_syxv_list_opt(                                   \
      sizeof((SyxV *[]){__VA_ARGS__}) / sizeof(SyxV *), \
      (SyxV *[]){__VA_ARGS__})
SyxV *make_syxv_bool(syx_bool_t value);
SyxV *make_syxv_integer(syx_integer_t value);
SyxV *make_syxv_fractional(syx_fractional_t value);
SyxV *make_syxv_string(String_View value);
SyxV *make_syxv_string_n(syx_string_t value, size_t size);
SyxV *make_syxv_string_cstr(syx_string_t value);
#define make_syxv_value(value)                       \
  _Generic(value,                                    \
      syx_bool_t: make_syxv_bool(value),             \
      syx_integer_t: make_syxv_integer(value),       \
      syx_fractional_t: make_syxv_fractional(value), \
      syx_string_t: make_syxv_string(value))
SyxV *make_syxv_quote(SyxV *quote);

SyxV *make_syxv_specialf(const char *name, Syx_Evaluator eval);
SyxV *make_syxv_builtin(const char *name, Syx_Evaluator eval);
SyxV *make_syxv_closure(const char *name, SyxV *defines, SyxV *body, Syx_Env *env);

SyxV *syxv_list_next_nullable(SyxV **list);
SyxV *syxv_list_next(SyxV **list);

bool syxv__list_for_each_next(Syx_Env *env, SyxV **list, SyxV **value, SyxV ***cdr);
#define syxv_list_for_each(env, value, list, ...) \
  for (SyxV *value##_list = (list), *value; syxv__list_for_each_next((env), &value##_list, &value, WITH_DEFAULT(NULL, __VA_ARGS__));)

bool syxv__list_map_next(Syx_Env *env, SyxV **source_it, SyxV ***target_it, SyxV ***value, SyxV ***cdr);
#define syxv_list_map(env, value, list, results, ...) \
  for (SyxV *value##_source_it = (list),              \
            **value##_target_it = (results),          \
            **value = NULL;                           \
       syxv__list_map_next(                           \
           (env),                                     \
           &value##_source_it,                        \
           &value##_target_it,                        \
           &value,                                    \
           WITH_DEFAULT(NULL, __VA_ARGS__));)

#define define_syxv_constants_ht(name) define_constants_ht(name, SyxV *)

typedef Ht(SyxV *, size_t, SyxV_Stringify_Cache) SyxV_Stringify_Cache;
size_t get__syxv_string_width(SyxV *value, SyxV_Stringify_Cache *cache);
#define get_syxv_string_width(value, ...) get__syxv_string_width((value), WITH_DEFAULT(NULL, __VA_ARGS__))
void stringify__syxv_n(SyxV *value, size_t length, char *string, SyxV_Stringify_Cache *cache);
#define stringify_syxv_n(value, length, string, ...) stringify__syxv_n((value), (length), (string), WITH_DEFAULT(NULL, __VA_ARGS__))
String_View stringify_syxv(SyxV *value);

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
#define SYX_EVAL_IMPL
#include "syx_eval.h"
#define SYX_UTILS_IMPL
#include "syx_utils.h"

void syxv_destructor(void *data) {
  SyxV *syxv = data;
  switch (syxv->kind) {
    case SYXV_KIND_NIL: break;
    case SYXV_KIND_SYMBOL: rc_release(syxv->symbol.name); break;
    case SYXV_KIND_PAIR:
      rc_release(syxv->pair.left);
      rc_release(syxv->pair.right);
      break;
    case SYXV_KIND_BOOL: break;
    case SYXV_KIND_INTEGER: break;
    case SYXV_KIND_FRACTIONAL: break;
    case SYXV_KIND_STRING: free(syxv->string); break;
    case SYXV_KIND_QUOTE: rc_release(syxv->quote); break;
    case SYXV_KIND_SPECIALF: {
      if (syxv->specialf.name) free(syxv->specialf.name);
    } break;
    case SYXV_KIND_BUILTIN: {
      if (syxv->builtin.name) free(syxv->builtin.name);
    } break;
    case SYXV_KIND_CLOSURE: {
      if (syxv->closure.name) free(syxv->closure.name);
      rc_release(syxv->closure.env);
      rc_release(syxv->closure.defines);
      rc_release(syxv->closure.forms);
    } break;
  }
}

SyxV *make_syxv(SyxV_Kind kind) {
  SyxV *value = rc_alloc(sizeof(SyxV), syxv_destructor);
  value->kind = kind;
  return value;
}

define_syxv_constants_ht(SYXV_CONSTANTS) {
  SyxV *nil = rc_acquire(make_syxv(SYXV_KIND_NIL));
  *ht_put(SYXV_CONSTANTS, "nil") = nil;
  *ht_put(SYXV_CONSTANTS, "null") = nil;

  SyxV *t = rc_acquire(make_syxv(SYXV_KIND_BOOL));
  t->boolean = true;
  *ht_put(SYXV_CONSTANTS, "#t") = t;
  *ht_put(SYXV_CONSTANTS, "true") = t;

  SyxV *f = rc_acquire(make_syxv(SYXV_KIND_BOOL));
  f->boolean = false;
  *ht_put(SYXV_CONSTANTS, "#f") = f;
  *ht_put(SYXV_CONSTANTS, "false") = f;
}

SyxV *make_syxv_nil() {
  return *ht_find(SYXV_CONSTANTS(), "nil");
}

SyxV *make_syxv_symbol(String_View symbol) {
  SyxV **constant = ht_find(SYXV_CONSTANTS(), symbol.data);
  if (constant != NULL) return *constant;
  bool guarded = false;
  for (String_View it = symbol; it.count; sv_chop_left(&it, 1)) {
    if (issymbol(*it.data)) continue;
    guarded = true;
    break;
  }
  SyxV *syxv = make_syxv(SYXV_KIND_SYMBOL);
  syxv->symbol.name = rc_acquire(rc_manage_strndup(symbol.data, symbol.count));
  syxv->symbol.guarded = guarded;
  return syxv;
}

SyxV *make_syxv_symbol_n(char *symbol, size_t size) {
  return make_syxv_symbol((String_View){.data = symbol, .count = size});
}

SyxV *make_syxv_symbol_cstr(char *symbol) {
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
  return value ? *ht_find(SYXV_CONSTANTS(), "#t") : *ht_find(SYXV_CONSTANTS(), "#f");
}

SyxV *make_syxv_integer(syx_integer_t value) {
  SyxV *syxv = make_syxv(SYXV_KIND_INTEGER);
  syxv->integer = value;
  return syxv;
}

SyxV *make_syxv_fractional(syx_fractional_t value) {
  SyxV *syxv = make_syxv(SYXV_KIND_FRACTIONAL);
  syxv->fractional = value;
  return syxv;
}

SyxV *make_syxv_string(String_View value) {
  SyxV *syxv = make_syxv(SYXV_KIND_STRING);
  syxv->string = strndup(value.data, value.count);
  return syxv;
}

SyxV *make_syxv_string_n(syx_string_t value, size_t size) {
  return make_syxv_string((String_View){.data = value, .count = size});
}

SyxV *make_syxv_string_cstr(syx_string_t value) {
  return make_syxv_string(sv_from_cstr(value));
}

SyxV *make_syxv_quote(SyxV *quote) {
  SyxV *syxv = make_syxv(SYXV_KIND_QUOTE);
  syxv->quote = quote ? rc_acquire(quote) : NULL;
  return syxv;
}

SyxV *make_syxv_specialf(const char *name, Syx_Evaluator eval) {
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
  value->closure.env = env ? rc_acquire(env) : NULL;
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

bool syxv__list_for_each_next(Syx_Env *env, SyxV **list, SyxV **value, SyxV ***cdr) {
  if ((*list)->kind != SYXV_KIND_PAIR) {
    if (cdr != NULL) (*cdr) = list;
    else if ((*list)->kind != SYXV_KIND_NIL) RUNTIME_ERROR("list expected", env);
    return false;
  }
  (*value) = (*list)->pair.left;
  (*list) = (*list)->pair.right;
  return true;
}

bool syxv__list_map_next(Syx_Env *env, SyxV **source_it, SyxV ***target_it, SyxV ***value, SyxV ***cdr) {
  if (*value) rc_acquire(**value);
  if ((*source_it)->kind != SYXV_KIND_PAIR) {
    if (cdr != NULL) (*cdr) = (*target_it);
    else if ((*source_it)->kind != SYXV_KIND_NIL) RUNTIME_ERROR("list expected", env);
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

size_t get__syxv_string_width(SyxV *value, SyxV_Stringify_Cache *cache) {
  size_t __result = 0;
  size_t *_result = cache ? ht_find_or_put(cache, value) : &__result;
#define result (*_result)
  switch (value->kind) {
    case SYXV_KIND_NIL: nob_return_defer(2);
    case SYXV_KIND_SYMBOL: nob_return_defer(strlen(value->symbol.name) + (value->symbol.guarded ? 2 : 0));
    case SYXV_KIND_PAIR: {
      size_t width = 1;
      SyxV *it = value;
      SyxV *last_left = it->pair.left;
      if (last_left->kind == SYXV_KIND_NIL) width += 2;
      else width += get__syxv_string_width(last_left, cache);
      it = it->pair.right;
      while (it->kind == SYXV_KIND_PAIR) {
        width += 1 + get__syxv_string_width(it->pair.left, cache);
        last_left = it->pair.left;
        it = it->pair.right;
      }
      if (it->kind != SYXV_KIND_NIL) {
        width += 3 + get__syxv_string_width(it, cache);
      }
      width += 1;
      nob_return_defer(width);
    }
    case SYXV_KIND_BOOL: nob_return_defer(2);
    case SYXV_KIND_INTEGER: nob_return_defer(get_integer_string_width(value->integer));
    case SYXV_KIND_FRACTIONAL: nob_return_defer(get_fractional_string_width(value->fractional, get_fractions_string_width(value->fractional)));
    case SYXV_KIND_STRING: nob_return_defer(1 + strlen(value->string) + 1);
    case SYXV_KIND_QUOTE: nob_return_defer(1 + get__syxv_string_width(value->quote, cache));
    case SYXV_KIND_SPECIALF: nob_return_defer(2 + strlen(value->specialf.name) + 1);
    case SYXV_KIND_BUILTIN: nob_return_defer(2 + strlen(value->builtin.name) + 1);
    case SYXV_KIND_CLOSURE: {
      size_t width = 1 + 2 + strlen(value->closure.name) + 1 + 1 + get__syxv_string_width(value->closure.defines, cache);
      SyxV *it = value->closure.forms;
      while (it->kind == SYXV_KIND_PAIR) {
        width += 1 + get__syxv_string_width(it->pair.left, cache);
        it = it->pair.right;
      }
      width += 1;
      nob_return_defer(width);
    }
  }
defer:
  return result;
#undef result
}

void stringify__syxv_n(SyxV *value, size_t length, char *string, SyxV_Stringify_Cache *cache) {
  size_t *_cached;
#define get_cached_width(subvalue) (cache && (_cached = ht_find(cache, (subvalue)), _cached != NULL) ? *_cached : get__syxv_string_width((subvalue), cache))
  switch (value->kind) {
    case SYXV_KIND_NIL: {
      *(string++) = '(';
      *(string++) = ')';
    } break;
    case SYXV_KIND_SYMBOL: {
      if (value->symbol.guarded) {
        *(string++) = '|';
        memcpy(string, value->symbol.name, length - 2);
        string[length - 2] = '|';
      } else {
        memcpy(string, value->symbol.name, length);
      }
    } break;
    case SYXV_KIND_PAIR: {
      *(string++) = '(';
      SyxV *it = value;
      SyxV *last_left = it->pair.left;
      size_t first_width = get_cached_width(last_left);
      stringify__syxv_n(last_left, first_width, string, cache);
      string += first_width;
      it = it->pair.right;
      while (it->kind == SYXV_KIND_PAIR) {
        *(string++) = ' ';
        last_left = it->pair.left;
        size_t width = get_cached_width(last_left);
        stringify__syxv_n(last_left, width, string, cache);
        string += width;
        it = it->pair.right;
      }
      if (it->kind != SYXV_KIND_NIL) {
        *(string++) = ' ';
        *(string++) = '.';
        *(string++) = ' ';
        size_t width = get_cached_width(it);
        stringify__syxv_n(it, width, string, cache);
        string += width;
      }
      *(string++) = ')';
    } break;
    case SYXV_KIND_BOOL: {
      *(string++) = '#';
      *(string++) = value->boolean ? 't' : 'f';
    } break;
    case SYXV_KIND_INTEGER: {
      stringify_integer_n(value->integer, length, string);
    } break;
    case SYXV_KIND_FRACTIONAL: {
      size_t precision = get_fractions_string_width(value->fractional);
      stringify_fractional_n(value->fractional, length - precision - 1, precision, string);
    } break;
    case SYXV_KIND_STRING: {
      *(string++) = '"';
      memcpy(string, value->string, length - 2);
      string += length - 2;
      *(string++) = '"';
    } break;
    case SYXV_KIND_QUOTE: {
      *(string++) = '\'';
      stringify__syxv_n(value->quote, length - 1, string, cache);
    } break;
    case SYXV_KIND_SPECIALF: {
      *(string++) = '?';
      *(string++) = '<';
      memcpy(string, value->specialf.name, length - 3);
      string[length - 3] = '>';
    } break;
    case SYXV_KIND_BUILTIN: {
      *(string++) = '!';
      *(string++) = '<';
      memcpy(string, value->builtin.name, length - 3);
      string[length - 3] = '>';
    } break;
    case SYXV_KIND_CLOSURE: {
      *(string++) = '(';
      *(string++) = '#';
      *(string++) = '<';
      size_t name_size = strlen(value->closure.name);
      memcpy(string, value->closure.name, name_size);
      string += name_size;
      *(string++) = '>';
      *(string++) = ' ';
      size_t defines_size = get_cached_width(value->closure.defines);
      stringify__syxv_n(value->closure.defines, defines_size, string, cache);
      string += defines_size;

      SyxV *it = value->closure.forms;
      while (it->kind == SYXV_KIND_PAIR) {
        *(string++) = ' ';
        size_t left_width = get_cached_width(it->pair.left);
        stringify__syxv_n(it->pair.left, left_width, string, cache);
        string += left_width;
        it = it->pair.right;
      }
      *string = ')';
    } break;
  }
#undef get_cached_width
}

String_View stringify_syxv(SyxV *value) {
  SyxV_Stringify_Cache cache = {0};
  size_t width = get__syxv_string_width(value, &cache);
  char *string = malloc(width + 1);
  String_View result = {.data = string, .count = width};
  stringify__syxv_n(value, width, string, &cache);
  ht_free(&cache);
  return result;
}

void fprint_syxv(FILE *f, SyxV *value) {
  String_View sv = stringify_syxv(value);
  fprintf(f, SV_Fmt, SV_Arg(sv));
}

#endif // SYX_VALUE_IMPL
