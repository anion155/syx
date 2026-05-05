#ifndef SYX_VALUE_H
#define SYX_VALUE_H

#include <ht.h>
#include <magic.h>
#include <nob.h>
#include <rc.h>

#include "syx_utils.h"

typedef struct SyxV SyxV;
typedef struct Syx_Eval_Ctx Syx_Eval_Ctx;
typedef struct Syx_Env Syx_Env;
typedef struct Syx_Frame Syx_Frame;

typedef SyxV *(*Syx_Evaluator)(Syx_Eval_Ctx *ctx, SyxV *arguments);

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
  SYXV_KIND_THROWN,
  SYXV_KIND_RETURN_VALUE,
} SyxV_Kind;

typedef struct SyxV_Symbol {
  char *name;
  size_t length;
  bool guarded;
} SyxV_Symbol;

typedef struct SyxV_Pair {
  SyxV *left;
  SyxV *right;
} SyxV_Pair;

typedef struct SyxV_Thrown {
  SyxV *reason;
  Syx_Frame *stack_frame;
} SyxV_Thrown;

struct SyxV {
  SyxV_Kind kind;

  union {
    SyxV_Symbol symbol;
    SyxV_Pair pair;
    syx_bool_t boolean;
    syx_integer_t integer;
    syx_fractional_t fractional;
    syx_string_view_t string;
    SyxV *quote;
    Syx_SpecialF specialf;
    Syx_Builtin builtin;
    Syx_Closure closure;
    SyxV_Thrown thrown;
    SyxV *return_value;
  };
};

typedef struct SyxVs {
  SyxV **items;
  size_t count;
  size_t capacity;
} SyxVs;

void syxv_destructor(void *data);
SyxV *get_syxv_from_symbol(SyxV_Symbol *symbol);
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
SyxV *make_syxv_string(syx_string_t value);
SyxV *make_syxv_string_sv(syx_string_view_t value);
SyxV *make_syxv_string_n(const char *value, size_t size);
SyxV *make_syxv_string_cstr(const char *value);
#define make_syxv_value(value)                       \
  _Generic(value,                                    \
      syx_bool_t: make_syxv_bool(value),             \
      syx_integer_t: make_syxv_integer(value),       \
      syx_fractional_t: make_syxv_fractional(value), \
      syx_string_t: make_syxv_string(value),         \
      syx_string_view_t: make_syxv_string_sv(value), \
      const char *: make_syxv_string_cstr(value))
SyxV *make_syxv_quote(SyxV *quote);
SyxV *make_syxv_specialf(const char *name, Syx_Evaluator eval);
SyxV *make_syxv_builtin(const char *name, Syx_Evaluator eval);
SyxV *make_syxv_closure(const char *name, SyxV *defines, SyxV *body, Syx_Env *env);
SyxV *make_syxv_throw(Syx_Frame *stack_frame, SyxV *reason);
SyxV *make_syxv_return_value(SyxV *return_value);

SyxV *syxv_list_next_nullable(SyxV **list);
SyxV *syxv_list_next(SyxV **list);

bool syxv__list_for_each_next(SyxV **list, SyxV **value, SyxV ***cdr);
#define syxv_list_for_each(value, list, ...) \
  for (SyxV *value##_list = (list), *value; syxv__list_for_each_next(&value##_list, &value, WITH_DEFAULT(NULL, __VA_ARGS__));)

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

typedef Ht(SyxV *, String_Builder, SyxV_Stringify_Cache) SyxV_Stringify_Cache;
size_t get__syxv_string_width(SyxV *value, SyxV_Stringify_Cache *cache);
#define get_syxv_string_width(value, ...) get__syxv_string_width((value), WITH_DEFAULT(NULL, __VA_ARGS__))
void stringify__syxv_n(SyxV *value, size_t length, char *string, SyxV_Stringify_Cache *cache);
#define stringify_syxv_n(value, length, string, ...) stringify__syxv_n((value), (length), (string), WITH_DEFAULT(NULL, __VA_ARGS__))
String_Builder stringify__syxv(SyxV *value, SyxV_Stringify_Cache *cache);
#define stringify_syxv(value, ...) stringify__syxv((value), WITH_DEFAULT(NULL, __VA_ARGS__))

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

SyxV *make_syxv(SyxV_Kind kind) {
  SyxV *value = rc_alloc(sizeof(SyxV), syxv_destructor);
  value->kind = kind;
  return value;
}

define_constant(struct SYXV_CONSTANTS_t {
  SyxV *nil;
  SyxV *t;
  SyxV *f; }, SYXV_CONSTANTS) {
  SYXV_CONSTANTS->nil = rc_acquire(make_syxv(SYXV_KIND_NIL));
  SYXV_CONSTANTS->t = rc_acquire(make_syxv(SYXV_KIND_BOOL));
  SYXV_CONSTANTS->t->boolean = true;
  SYXV_CONSTANTS->f = rc_acquire(make_syxv(SYXV_KIND_BOOL));
  SYXV_CONSTANTS->f->boolean = false;
}

define_constant(Ht(const char *, SyxV *), SYXV_SYMBOLS) {
  SYXV_SYMBOLS->hasheq = ht_cstr_hasheq;
}

void syxv_destructor(void *data) {
  SyxV *syxv = data;
  switch (syxv->kind) {
    case SYXV_KIND_NIL: break;
    case SYXV_KIND_SYMBOL: {
      ht_delete(SYXV_SYMBOLS(), syxv);
      free(syxv->symbol.name);
    } break;
    case SYXV_KIND_PAIR:
      if (syxv->pair.left) rc_release(syxv->pair.left);
      if (syxv->pair.right) rc_release(syxv->pair.right);
      break;
    case SYXV_KIND_BOOL: break;
    case SYXV_KIND_INTEGER: break;
    case SYXV_KIND_FRACTIONAL: break;
    case SYXV_KIND_STRING: free((char *)syxv->string.data); break;
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
  SyxV **symbol = ht_find(SYXV_SYMBOLS(), name.data);
  if (!symbol) {
    char *key = strndup(name.data, name.count);
    symbol = ht_put(SYXV_SYMBOLS(), key);
    *symbol = rc_acquire(make_syxv(SYXV_KIND_SYMBOL));
    (*symbol)->symbol.name = key;
    (*symbol)->symbol.length = name.count;
    for (String_View it = name; it.count; sv_chop_left(&it, 1)) {
      if (issymbol(*it.data)) continue;
      (*symbol)->symbol.guarded = true;
      break;
    }
  }
  return (*symbol);
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
  return value ? SYXV_CONSTANTS()->t : SYXV_CONSTANTS()->f;
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

SyxV *make_syxv_string(syx_string_t value) {
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

SyxV *make_syxv_throw(Syx_Frame *stack_frame, SyxV *reason) {
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

bool syxv__list_for_each_next(SyxV **list, SyxV **value, SyxV ***cdr) {
  if ((*list)->kind != SYXV_KIND_PAIR) {
    if (cdr != NULL) (*cdr) = list;
    else if ((*list)->kind != SYXV_KIND_NIL) UNREACHABLE("list expected");
    return false;
  }
  (*value) = (*list)->pair.left;
  (*list) = (*list)->pair.right;
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

size_t stringify__cached_syxv(SyxV *value, SyxV_Stringify_Cache *cache, char *string) {
  if (!cache) {
    size_t width = get__syxv_string_width(value, NULL);
    if (string) stringify__syxv_n(value, width, string, NULL);
    return width;
  }
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

size_t get__syxv_string_width(SyxV *value, SyxV_Stringify_Cache *cache) {
  syx_string_t *cached = cache ? ht_find(cache, value) : NULL;
  if (cached) return (*cached).count;
  switch (value->kind) {
    case SYXV_KIND_NIL: return 2;
    case SYXV_KIND_SYMBOL: return value->symbol.length + (value->symbol.guarded ? 2 : 0);
    case SYXV_KIND_PAIR: {
      size_t width = 1;
      SyxV *it = value;
      SyxV *last_left = it->pair.left;
      if (last_left->kind == SYXV_KIND_NIL) width += 2;
      else width += stringify__cached_syxv(last_left, cache, NULL);
      it = it->pair.right;
      while (it->kind == SYXV_KIND_PAIR) {
        width += 1 + stringify__cached_syxv(it->pair.left, cache, NULL);
        last_left = it->pair.left;
        it = it->pair.right;
      }
      if (it->kind != SYXV_KIND_NIL) {
        width += 3 + stringify__cached_syxv(it, cache, NULL);
      }
      width += 1;
      return width;
    }
    case SYXV_KIND_BOOL: return value->boolean ? 4 : 5;
    case SYXV_KIND_INTEGER: return get_integer_string_width(value->integer);
    case SYXV_KIND_FRACTIONAL: return get_fractional_string_width(value->fractional, get_fractions_string_width(value->fractional));
    case SYXV_KIND_STRING: return 1 + value->string.count + 1;
    case SYXV_KIND_QUOTE: return 1 + stringify__cached_syxv(value->quote, cache, NULL);
    case SYXV_KIND_SPECIALF: {
      size_t name_width = (value->specialf.name ? strlen(value->specialf.name) : 0);
      return 2 + name_width + 1;
    }
    case SYXV_KIND_BUILTIN: {
      size_t name_width = (value->builtin.name ? strlen(value->builtin.name) : 0);
      return 2 + name_width + 1;
    }
    case SYXV_KIND_CLOSURE: {
      size_t name_width = (value->closure.name ? strlen(value->closure.name) : 0);
      size_t width = 1 + 2 + name_width + 1 + 1 + stringify__cached_syxv(value->closure.defines, cache, NULL);
      SyxV *it = value->closure.forms;
      while (it->kind == SYXV_KIND_PAIR) {
        width += 1 + stringify__cached_syxv(it->pair.left, cache, NULL);
        it = it->pair.right;
      }
      width += 1;
      return width;
    }
    case SYXV_KIND_THROWN: UNREACHABLE("thrown object can't be convderted");
    case SYXV_KIND_RETURN_VALUE: UNREACHABLE("return value object can't be convderted");
  }
}

void stringify__syxv_n(SyxV *value, size_t length, char *string, SyxV_Stringify_Cache *cache) {
  syx_string_t *cached = cache ? ht_find(cache, value) : NULL;
  if (cached) {
    memcpy(string, cached->items, cached->count);
    return;
  }
  switch (value->kind) {
    case SYXV_KIND_NIL: {
      *(string++) = '(';
      *(string++) = ')';
    } break;
    case SYXV_KIND_SYMBOL: {
      if (value->symbol.guarded) {
        *(string++) = '|';
        memcpy(string, value->symbol.name, value->symbol.length);
        string[value->symbol.length] = '|';
      } else {
        memcpy(string, value->symbol.name, length);
      }
    } break;
    case SYXV_KIND_PAIR: {
      *(string++) = '(';
      SyxV *it = value;
      SyxV *last_left = it->pair.left;
      string += stringify__cached_syxv(last_left, cache, string);
      it = it->pair.right;
      while (it->kind == SYXV_KIND_PAIR) {
        *(string++) = ' ';
        last_left = it->pair.left;
        string += stringify__cached_syxv(last_left, cache, string);
        it = it->pair.right;
      }
      if (it->kind != SYXV_KIND_NIL) {
        *(string++) = ' ';
        *(string++) = '.';
        *(string++) = ' ';
        string += stringify__cached_syxv(it, cache, string);
      }
      *(string++) = ')';
    } break;
    case SYXV_KIND_BOOL: {
      if (value->boolean) {
        memcpy(string, "true", 4);
      } else {
        memcpy(string, "false", 5);
      }
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
      memcpy(string, value->string.data, value->string.count);
      string += value->string.count;
      *(string++) = '"';
    } break;
    case SYXV_KIND_QUOTE: {
      *(string++) = '\'';
      string += stringify__cached_syxv(value->quote, cache, string);
    } break;
    case SYXV_KIND_SPECIALF: {
      *(string++) = '?';
      *(string++) = '<';
      if (value->specialf.name) {
        size_t name_size = strlen(value->specialf.name);
        memcpy(string, value->specialf.name, name_size);
        string += name_size;
      }
      *(string++) = '>';
    } break;
    case SYXV_KIND_BUILTIN: {
      *(string++) = '!';
      *(string++) = '<';
      if (value->builtin.name) {
        size_t name_size = strlen(value->builtin.name);
        memcpy(string, value->builtin.name, name_size);
        string += name_size;
      }
      *(string++) = '>';
    } break;
    case SYXV_KIND_CLOSURE: {
      *(string++) = '(';
      *(string++) = '#';
      *(string++) = '<';
      if (value->closure.name) {
        size_t name_size = strlen(value->closure.name);
        memcpy(string, value->closure.name, name_size);
        string += name_size;
      }
      *(string++) = '>';
      *(string++) = ' ';
      string += stringify__cached_syxv(value->closure.defines, cache, string);

      SyxV *it = value->closure.forms;
      while (it->kind == SYXV_KIND_PAIR) {
        *(string++) = ' ';
        string += stringify__cached_syxv(it->pair.left, cache, string);
        it = it->pair.right;
      }
      *string = ')';
    } break;
    case SYXV_KIND_THROWN: UNREACHABLE("thrown object can't be converted");
    case SYXV_KIND_RETURN_VALUE: UNREACHABLE("return value object can't be convderted");
  }
}

void syxv_stringify_cache_destructor(void *data) {
  SyxV_Stringify_Cache *cache = data;
  ht_foreach(item, cache) {
    rc_release(ht_key(cache, item));
    sb_free(*item);
  }
  ht_free(cache);
}

syx_string_t stringify__syxv(SyxV *value, SyxV_Stringify_Cache *cache) {
  SyxV_Stringify_Cache *_cache;
  if (cache) _cache = rc_acquire(cache);
  else _cache = rc_acquire(rc_alloc(sizeof(SyxV_Stringify_Cache), syxv_stringify_cache_destructor));
  size_t width = get__syxv_string_width(value, _cache);
  char *string = malloc(width + 1);
  syx_string_t result = {.items = string, .count = width, .capacity = width + 1};
  stringify__syxv_n(value, width, string, _cache);
  rc_release(_cache);
  return result;
}

void fprint_syxv(FILE *f, SyxV *value) {
  syx_string_t sb = stringify_syxv(value);
  fprintf(f, "%s", sb.items);
  sb_free(sb);
}

#endif // SYX_VALUE_IMPL
