#ifndef SYX_VALUE_H
#define SYX_VALUE_H

#include <ht.h>
#include <magic.h>
#include <nob.h>
#include <rc.h>

#include "sexpr_ast.h"

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

void fprint_syx_closure(FILE *f, Syx_Closure *closure);
#define print_syx_closure(closure) fprint_syx_closure(stdout, (closure))

typedef enum : unsigned int {
  SYXV_KIND_NIL = SEXPR_KIND_NIL,
  SYXV_KIND_SYMBOL = SEXPR_KIND_SYMBOL,
  SYXV_KIND_PAIR = SEXPR_KIND_PAIR,
  SYXV_KIND_BOOL = SEXPR_KIND_BOOL,
  SYXV_KIND_INTEGER = SEXPR_KIND_INTEGER,
  SYXV_KIND_FRACTIONAL = SEXPR_KIND_FRACTIONAL,
  SYXV_KIND_STRING = SEXPR_KIND_STRING,
  SYXV_KIND_QUOTE = SEXPR_KIND_QUOTE,
  SYXV_KIND_SPECIALF,
  SYXV_KIND_BUILTIN,
  SYXV_KIND_CLOSURE,
} SyxV_Kind;

struct SyxV {
  SyxV_Kind kind;

  union {
    SExpr_Symbol symbol;

    struct SyxV_Pair {
      SyxV *left;
      SyxV *right;
    } pair;

    bool_t boolean;
    integer_t integer;
    fractional_t fractional;
    string_t string;
    SyxV *quote;
    Syx_SpecialF specialf;
    Syx_Builtin builtin;
    Syx_Closure closure;
  };
};

void syxv_destructor(void *data);
SyxV *make_syxv_nil();
SyxV *make_syxv_symbol(String_View symbol);
SyxV *make_syxv_symbol_n(char *symbol, size_t size);
SyxV *make_syxv_symbol_ctrs(char *symbol);
SyxV *make_syxv_pair(SyxV *left, SyxV *right);
SyxV *make_syxv_bool(bool_t value);
SyxV *make_syxv_integer(integer_t value);
SyxV *make_syxv_fractional(fractional_t value);
SyxV *make_syxv_string(String_View value);
SyxV *make_syxv_string_n(string_t value, size_t size);
SyxV *make_syxv_string_cstr(string_t value);
SyxV *make_syxv_quote(SyxV *quote);
#define make_syxv_value(value)                   \
  _Generic(value,                                \
      bool_t: make_syxv_bool(value),             \
      integer_t: make_syxv_integer(value),       \
      fractional_t: make_syxv_fractional(value), \
      string_t: make_syxv_string(value))
SyxV *make_syxv_list_opt(size_t count, SyxV **items);
#define make_syxv_list(...)                             \
  make_syxv_list_opt(                                   \
      sizeof((SyxV *[]){__VA_ARGS__}) / sizeof(SyxV *), \
      (SyxV *[]){__VA_ARGS__})

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

#define define_syxv_constants_ht(name) define_constants_ht((name), SyxV *)

void fprint_syxv(FILE *f, SyxV *value);
#define print_syxv(value) fprint_syxv(stdout, (value))

#endif // SYX_VALUE_H

#if defined(SYX_VALUE_IMPL) && !defined(SYX_VALUE_IMPL_C)
#define SYX_VALUE_IMPL_C

#define SEXPR_AST_IMPL
#include "sexpr_ast.h"
#include "syx_eval.h"

void syxv_destructor(void *data) {
  SyxV *syxv = data;
  switch (syxv->kind) {
    case SYXV_KIND_NIL:
    case SYXV_KIND_SYMBOL:
    case SYXV_KIND_PAIR:
    case SYXV_KIND_BOOL:
    case SYXV_KIND_INTEGER:
    case SYXV_KIND_FRACTIONAL:
    case SYXV_KIND_STRING:
    case SYXV_KIND_QUOTE: UNREACHABLE("should not be called for s-expressions");
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

SyxV *make_syxv_nil() { return (SyxV *)make_sexpr_nil(); }

SyxV *make_syxv_symbol(String_View symbol) { return (SyxV *)make_sexpr_symbol(symbol); }

SyxV *make_syxv_symbol_n(char *symbol, size_t size) { return (SyxV *)make_sexpr_symbol_n(symbol, size); }

SyxV *make_syxv_symbol_ctrs(char *symbol) { return (SyxV *)make_sexpr_symbol_ctrs(symbol); }

SyxV *make_syxv_pair(SyxV *left, SyxV *right) { return (SyxV *)make_sexpr_pair((SExpr *)left, (SExpr *)right); }

SyxV *make_syxv_bool(bool_t value) { return (SyxV *)make_sexpr_bool(value); }

SyxV *make_syxv_integer(integer_t value) { return (SyxV *)make_sexpr_integer(value); }

SyxV *make_syxv_fractional(fractional_t value) { return (SyxV *)make_sexpr_fractional(value); }

SyxV *make_syxv_string(String_View value) { return (SyxV *)make_sexpr_string(value); }

SyxV *make_syxv_string_n(string_t value, size_t size) { return (SyxV *)make_sexpr_string_n(value, size); }

SyxV *make_syxv_string_cstr(string_t value) { return (SyxV *)make_sexpr_string_cstr(value); }

SyxV *make_syxv_quote(SyxV *quote) { return (SyxV *)make_sexpr_quote((SExpr *)quote); }

SyxV *make_syxv_list_opt(size_t count, SyxV **items) { return (SyxV *)make_sexpr_list_opt(count, (SExpr **)items); }

SyxV *make_syxv(SyxV_Kind kind) {
  SyxV *value = rc_alloc(sizeof(SyxV), syxv_destructor);
  value->kind = kind;
  return value;
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
  value->closure.defines = rc_acquire(defines);
  value->closure.forms = rc_acquire(forms);
  value->closure.env = rc_acquire(env);
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

void fprint_syxv(FILE *f, SyxV *value) {
  switch (value->kind) {
    case SYXV_KIND_NIL: return fprint_sexpr(f, (SExpr *)value);
    case SYXV_KIND_SYMBOL: return fprint_sexpr(f, (SExpr *)value);
    case SYXV_KIND_PAIR: {
      fprintf(f, "(");
      SyxV *it = value;
      SyxV *last_left = it->pair.left;
      if (last_left->kind == SYXV_KIND_NIL) fprintf(f, "nil");
      else fprint_syxv(f, last_left);
      it = it->pair.right;
      while (it->kind == SYXV_KIND_PAIR) {
        fprintf(f, " ");
        last_left = it->pair.left;
        fprint_syxv(f, last_left);
        it = it->pair.right;
      }
      if (it->kind != SYXV_KIND_NIL) {
        fprintf(f, " . ");
        fprint_syxv(f, it);
      } else if (last_left->kind == SYXV_KIND_NIL) {
        fprintf(f, " . nil");
      }
      fprintf(f, ")");
    } break;
    case SYXV_KIND_BOOL: return fprint_sexpr(f, (SExpr *)value);
    case SYXV_KIND_INTEGER: return fprint_sexpr(f, (SExpr *)value);
    case SYXV_KIND_FRACTIONAL: return fprint_sexpr(f, (SExpr *)value);
    case SYXV_KIND_STRING: return fprint_sexpr(f, (SExpr *)value);
    case SYXV_KIND_QUOTE: {
      fprintf(f, "'");
      fprint_syxv(f, value->quote);
    } break;
    case SYXV_KIND_SPECIALF: fprintf(f, "?<%s>", value->specialf.name); break;
    case SYXV_KIND_BUILTIN: fprintf(f, "!<%s>", value->builtin.name); break;
    case SYXV_KIND_CLOSURE: fprint_syx_closure(f, &value->closure); break;
  }
}

void fprint_syx_closure(FILE *f, Syx_Closure *closure) {
  fprintf(f, "(#<%s> ", closure->name);
  fprint_syxv(f, closure->defines);
  SyxV *it = closure->forms;
  while (it->kind == SYXV_KIND_PAIR) {
    fprintf(f, " ");
    fprint_syxv(f, it->pair.left);
    it = it->pair.right;
  }
  fprintf(f, ")");
}

#endif // SYX_VALUE_IMPL
