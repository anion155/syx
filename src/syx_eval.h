#ifndef SYX_EVAL_H
#define SYX_EVAL_H

#include <ht.h>
#include <magic.h>
#include <nob.h>
#include <rc.h>

#include "syx_value.h"

#define RUNTIME_ERROR(message, env) UNREACHABLE((UNUSED(env), (message)))

typedef Ht(const char *, SyxV *) Syx_Env_Symbols;

struct Syx_Env {
  Syx_Env *parent;
  Syx_Env_Symbols symbols;
  char *description;
};

void syx_env_destructor(void *data);
Syx_Env *make_syx_env(Syx_Env *parent, const char *description);
Syx_Env *syx_env_global(Syx_Env *env);
SyxV **syx_env_lookup(Syx_Env *env, const char *name);
SyxV *syx_env_lookup_get(Syx_Env *env, const char *name);
void syx_env_define(Syx_Env *env, const char *name, SyxV *value);
void syx_env_set(Syx_Env *env, const char *name, SyxV *value);
Syx_Env *make_global_syx_env();

SyxVs *make_syxvs(Syx_Env *env, SyxV *value);
SyxVs *syxvs_eval(Syx_Env *env, SyxVs *syxvs);

SyxV *syx_eval_specialf(Syx_Env *env, Syx_SpecialF *specialf, SyxV *arguments_syxv);
SyxV *syx_eval_builtin(Syx_Env *env, Syx_Builtin *builtin, SyxV *arguments_syxv);
SyxV *syx_eval_closure(Syx_Env *env, Syx_Closure *closure, SyxV *arguments_syxv);
SyxV *syx_eval(Syx_Env *env, SyxV *input);
SyxV *syx_eval_with_release(Syx_Env *env, SyxV *input);

SyxV *syxv_list_next(SyxV *list);
SyxV *syxv_list_get_value(SyxV *list);
#define syxv_list_for_each(value, first) for ( \
    SyxV *value##_it = (first),                \
    *value = syxv_list_get_value(value##_it);  \
    value##_it != NULL;                        \
    value##_it = syxv_list_next(value##_it),   \
    value = syxv_list_get_value(value##_it))

SyxV *syx_convert_to_bool(Syx_Env *env, SyxV *value);
SyxV *syx_convert_to_integer(Syx_Env *env, SyxV *value);
SyxV *syx_convert_to_real(Syx_Env *env, SyxV *value);
SyxV *syx_convert_to_string(Syx_Env *env, SyxV *value);

#define SYX_EVAL_ARGUMENTS_MIN(env, min, ...) \
  if (arguments->count < (min)) RUNTIME_ERROR("Too few arguments", env)
#define SYX_EVAL_ARGUMENTS_MAX(env, max, ...) \
  if (arguments->count > (max)) RUNTIME_ERROR("Too many arguments", env)
#define SYX_EVAL_ARGUMENTS_CLAMP(env, min, ...) \
  SYX_EVAL_ARGUMENTS_MIN((env), (min));         \
  SYX_EVAL_ARGUMENTS_MAX((env), (WITH_DEFAULT((min), __VA_ARGS__)))

#endif // SYX_EVAL_H

#if defined(SYX_EVAL_IMPL) && !defined(SYX_EVAL_IMPL_C)
#define SYX_EVAL_IMPL_C

#define SYX_VALUE_IMPL
#include "syx_value.h"
#define SYX_EVAL_SPECIALF_IMPL
#include "syx_eval_specialf.h"

// #define SYX_EVAL_ARITHMETIC_IMPL
// #include "expr_eval_arithmetic.h"
// #define SYX_EVAL_COMPARISON_IMPL
// #include "expr_eval_comparison.h"
// #define SYX_EVAL_LIST_IMPL
// #include "expr_eval_list.h"
// #define SYX_EVAL_TYPE_PREDICATES_IMPL
// #include "expr_eval_type_predicates.h"
// #define SYX_EVAL_STRING_IMPL
// #include "expr_eval_string.h"
// #define SYX_EVAL_TYPE_CONVERSION_IMPL
// #include "expr_eval_type_conversion.h"
// #define SYX_EVAL_EQUALITY_IMPL
// #include "expr_eval_equality.h"
// #define SYX_EVAL_IO_IMPL
// #include "expr_eval_io.h"

SyxV *syxv_list_next(SyxV *list) {
  if (list->kind == SYXV_KIND_PAIR) return list->pair.right;
  if (list->kind == SYXV_KIND_NIL) return NULL;
  return make_syxv_nil();
}

SyxV *syxv_list_get_value(SyxV *list) {
  if (list == NULL) return NULL;
  if (list->kind == SYXV_KIND_PAIR) return list->pair.left;
  if (list->kind == SYXV_KIND_NIL) return NULL;
  return list;
}

void syx_env_destructor(void *data) {
  Syx_Env *env = data;
  ht_foreach(symbol, &env->symbols) rc_release(*symbol);
  ht_free(&env->symbols);
  if (env->parent) rc_release(env->parent);
  if (env->description) free(env->description);
}

Syx_Env *make_syx_env(Syx_Env *parent, const char *description) {
  Syx_Env *env = rc_alloc(sizeof(Syx_Env), syx_env_destructor);
  env->parent = parent ? rc_acquire(parent) : NULL;
  env->symbols.hasheq = ht_cstr_hasheq;
  env->description = description ? strdup(description) : NULL;
  return env;
}

Syx_Env *syx_env_global(Syx_Env *env) {
  while (env->parent != NULL)
    env = env->parent;
  return env;
}

SyxV **syx_env_lookup(Syx_Env *env, const char *name) {
  SyxV **item = NULL;
  while (env != NULL && item == NULL) {
    item = ht_find(&env->symbols, name);
    env = env->parent;
  }
  return item;
}

SyxV *syx_env_lookup_get(Syx_Env *env, const char *name) {
  SyxV **item = syx_env_lookup(env, name);
  if (item == NULL) return NULL;
  return *item;
}

SyxV **ensure_syxv_redefinable(Syx_Env *env, const char *name) {
  SyxV **item = syx_env_lookup(env, name);
  if (item != NULL) {
    switch ((*item)->kind) {
      case SYXV_KIND_NIL:
      case SYXV_KIND_SYMBOL:
      case SYXV_KIND_PAIR:
      case SYXV_KIND_BOOL:
      case SYXV_KIND_INTEGER:
      case SYXV_KIND_REAL:
      case SYXV_KIND_STRING:
      case SYXV_KIND_QUOTE: break;
      case SYXV_KIND_SPECIALF: RUNTIME_ERROR("trying to redefine special form", env);
      case SYXV_KIND_BUILTIN: RUNTIME_ERROR("trying to redefine builtin", env);
      case SYXV_KIND_CLOSURE: break;
    }
  }
  return item;
}

void syx_env_define(Syx_Env *env, const char *name, SyxV *value) {
  SyxV **item = ensure_syxv_redefinable(env, name);
  if (item == NULL) item = ht_put(&env->symbols, name);
  *item = rc_acquire(value);
  switch (value->kind) {
    case SYXV_KIND_NIL: break;
    case SYXV_KIND_SYMBOL: break;
    case SYXV_KIND_PAIR: break;
    case SYXV_KIND_BOOL: break;
    case SYXV_KIND_INTEGER: break;
    case SYXV_KIND_REAL: break;
    case SYXV_KIND_STRING: break;
    case SYXV_KIND_QUOTE: break;
    case SYXV_KIND_SPECIALF: {
      if (!value->specialf.name) value->specialf.name = strdup(name);
    } break;
    case SYXV_KIND_BUILTIN: {
      if (!value->builtin.name) value->builtin.name = strdup(name);
    } break;
    case SYXV_KIND_CLOSURE: {
      if (!value->closure.name) value->closure.name = strdup(name);
    } break;
  }
}

void syx_env_set(Syx_Env *env, const char *name, SyxV *value) {
  SyxV **item = ensure_syxv_redefinable(env, name);
  if (item == NULL) RUNTIME_ERROR("unbound symbol", env);
  *item = rc_acquire(value);
}

Syx_Env *make_global_syx_env() {
  Syx_Env *env = make_syx_env(NULL, "<global>");
  syx_env_define_special_forms(env);
  //   syx_env_define_arithmetic(env);
  //   syx_env_define_comparison(env);
  //   syx_env_define_equality(env);
  //   syx_env_define_list(env);
  //   syx_env_define_string(env);
  //   syx_env_define_type_predicates(env);
  //   syx_env_define_type_conversion(env);
  //   syx_env_define_io(env);
  return env;
}

SyxVs *make_syxvs(Syx_Env *env, SyxV *value) {
  SyxVs *arguments = rc_alloc(sizeof(SyxVs), da_destructor);
  syxv_list_for_each(arg, value) {
    if (arg_it == arg) RUNTIME_ERROR("malformed arguments list", env);
    if (arg) da_append(arguments, rc_acquire(arg));
  }
  return arguments;
}

SyxVs *syxvs_eval(Syx_Env *env, SyxVs *syxvs) {
  da_foreach(SyxV *, argument, syxvs) {
    *argument = syx_eval_with_release(env, *argument);
  }
  return syxvs;
}

SyxV *syx_eval_specialf(Syx_Env *env, Syx_SpecialF *specialf, SyxV *arguments_syxv) {
  SyxVs *arguments = rc_acquire(make_syxvs(env, arguments_syxv));
  SyxV *result = specialf->eval(env, arguments);
  if (!result) result = make_syxv_nil();
  rc_release(arguments);
  return result;
}

SyxV *syx_eval_builtin(Syx_Env *env, Syx_Builtin *builtin, SyxV *arguments_syxv) {
  SyxVs *arguments = rc_acquire(make_syxvs(env, arguments_syxv));
  arguments = syxvs_eval(env, arguments);
  SyxV *result = builtin->eval(env, arguments);
  if (!result) result = make_syxv_nil();
  rc_release(arguments);
  return result;
}

SyxV *syx_eval_closure(Syx_Env *env, Syx_Closure *closure, SyxV *arguments_syxv) {
  Syx_Env *call_env = rc_acquire(make_syx_env(closure->env, closure->name));
  SyxV *it = arguments_syxv;
  syxv_list_for_each(name_syxv, closure->arguments) {
    if (!name_syxv) continue;
    const char *name = name_syxv->symbol.name;
    if (name_syxv->kind == SYXV_KIND_NIL) {
      syx_env_define(call_env, name, it);
      it = make_syxv_nil();
      break;
    }
    if (it->kind == SYXV_KIND_NIL) RUNTIME_ERROR("Too few arguments", env);
    syx_env_define(call_env, name, it->pair.left);
    it = it->pair.right;
  }
  if (it->kind != SYXV_KIND_NIL) RUNTIME_ERROR("Too many arguments", env);
  SyxV *result = syx_eval(call_env, closure->body);
  rc_release(call_env);
  return result;
}

SyxV *syx_eval(Syx_Env *env, SyxV *input) {
  if (input->kind == SYXV_KIND_QUOTE) return input->quote;
  if (input->kind == SYXV_KIND_SYMBOL) {
    SyxV *item = syx_env_lookup_get(env, input->symbol.name);
    if (item == NULL) RUNTIME_ERROR("unbound symbol", env);
    return item;
  }
  if (input->kind != SYXV_KIND_PAIR) return input;
  if (!input->pair.left) return input;
  SyxV *head = rc_acquire(syx_eval(env, input->pair.left));
  SyxV *arguments = input->pair.right;
  SyxV *result;
  switch (head->kind) {
    case SYXV_KIND_NIL:
    case SYXV_KIND_SYMBOL:
    case SYXV_KIND_PAIR:
    case SYXV_KIND_BOOL:
    case SYXV_KIND_INTEGER:
    case SYXV_KIND_REAL:
    case SYXV_KIND_STRING:
    case SYXV_KIND_QUOTE: RUNTIME_ERROR("is not a procedure", env);
    case SYXV_KIND_SPECIALF: result = syx_eval_specialf(env, &head->specialf, arguments); break;
    case SYXV_KIND_BUILTIN: result = syx_eval_builtin(env, &head->builtin, arguments); break;
    case SYXV_KIND_CLOSURE: result = syx_eval_closure(env, &head->closure, arguments); break;
  }
  rc_release(head);
  return result;
}

SyxV *syx_eval_with_release(Syx_Env *env, SyxV *value) {
  SyxV *unevaluated = value;
  value = rc_acquire(syx_eval(env, unevaluated));
  rc_release(unevaluated);
  return value;
}

SyxV *syx_convert_to_bool(Syx_Env *env, SyxV *value) {
  UNUSED(env);
  switch (value->kind) {
    case SYXV_KIND_NIL: return make_syxv_bool(false);
    case SYXV_KIND_SYMBOL: return make_syxv_bool(true);
    case SYXV_KIND_PAIR: return make_syxv_bool(true);
    case SYXV_KIND_BOOL: return value;
    case SYXV_KIND_INTEGER: return make_syxv_bool((sexpr_bool_t)value->integer);
    case SYXV_KIND_REAL: return make_syxv_bool((sexpr_bool_t)value->real);
    case SYXV_KIND_STRING: return make_syxv_bool(*value->string != 0);
    case SYXV_KIND_QUOTE: return syx_convert_to_bool(env, value->quote);
    case SYXV_KIND_SPECIALF: return make_syxv_bool(true);
    case SYXV_KIND_BUILTIN: return make_syxv_bool(true);
    case SYXV_KIND_CLOSURE: return make_syxv_bool(true);
  }
}

SyxV *syx_convert_to_integer(Syx_Env *env, SyxV *value) {
  switch (value->kind) {
    case SYXV_KIND_NIL: return make_syxv_integer(0);
    case SYXV_KIND_SYMBOL: return NULL;
    case SYXV_KIND_PAIR: return NULL;
    case SYXV_KIND_BOOL: return make_syxv_integer(value->boolean ? 1 : 0);
    case SYXV_KIND_INTEGER: return value;
    case SYXV_KIND_REAL: return make_syxv_integer((sexpr_int_t)value->real);
    case SYXV_KIND_STRING: {
      String_View sv = sv_from_cstr(value->string);
      sexpr_int_t result = 0;
      if (!parse_integer(&sv, &result)) return NULL;
      return make_syxv_integer(result);
    }
    case SYXV_KIND_QUOTE: return syx_convert_to_integer(env, value->quote);
    case SYXV_KIND_SPECIALF: return NULL;
    case SYXV_KIND_BUILTIN: return NULL;
    case SYXV_KIND_CLOSURE: return NULL;
  }
}

SyxV *syx_convert_to_real(Syx_Env *env, SyxV *value) {
  switch (value->kind) {
    case SYXV_KIND_NIL: return make_syxv_real(0.0);
    case SYXV_KIND_SYMBOL: return NULL;
    case SYXV_KIND_PAIR: return NULL;
    case SYXV_KIND_BOOL: return make_syxv_real(value->boolean ? 1.0 : 0.0);
    case SYXV_KIND_INTEGER: return make_syxv_real((sexpr_real_t)value->integer);
    case SYXV_KIND_REAL: return value;
    case SYXV_KIND_STRING: {
      String_View sv = sv_from_cstr(value->string);
      sexpr_real_t result = 0;
      if (!parse_real(&sv, &result)) return NULL;
      return make_syxv_real(result);
    }
    case SYXV_KIND_QUOTE: return syx_convert_to_real(env, value->quote);
    case SYXV_KIND_SPECIALF: return NULL;
    case SYXV_KIND_BUILTIN: return NULL;
    case SYXV_KIND_CLOSURE: return NULL;
  }
}

SyxV *syx_convert_to_string(Syx_Env *env, SyxV *value) {
  switch (value->kind) {
    case SYXV_KIND_NIL: return make_syxv_string_cstr("nil");
    case SYXV_KIND_SYMBOL: return NULL;
    case SYXV_KIND_PAIR: return NULL;
    case SYXV_KIND_BOOL: return make_syxv_string_cstr(value->boolean ? "true" : "false");
    case SYXV_KIND_INTEGER: return make_syxv_string(stringify_int(value->integer));
    case SYXV_KIND_REAL: return make_syxv_string(stringify_real(value->integer));
    case SYXV_KIND_STRING: return value;
    case SYXV_KIND_QUOTE: return syx_convert_to_string(env, value->quote);
    case SYXV_KIND_SPECIALF: return NULL;
    case SYXV_KIND_BUILTIN: return NULL;
    case SYXV_KIND_CLOSURE: return NULL;
  }
}

#endif // SYX_EVAL_IMPL
