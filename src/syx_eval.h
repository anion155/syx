#ifndef SYX_EVAL_H
#define SYX_EVAL_H

#include <nob.h>
#include <ht.h>
#include <rc.h>
#include <magic.h>
#include "syx_value.h"

#define RUNTIME_ERROR(message, env) UNREACHABLE((UNUSED(env), (message)))

typedef Ht(const char *, SyxV *) Syx_Env_Symbols;
struct Syx_Env {
  Syx_Env *parent;
  Syx_Env_Symbols symbols;
  char *description;
};
void syx_env_destructor(void *data);
Syx_Env *make_syx_env(Syx_Env *parent, char *description);
Syx_Env *syx_env_global(Syx_Env *env);
void syx_env_put(Syx_Env *env, const char *name, SyxV *value);
SyxV **syx_env_lookup(Syx_Env *env, const char *name);
Syx_Env *make_global_syx_env();

SyxV *syx_eval_specialf(Syx_Env *env, Syx_SpecialF *specialf, SyxV *arguments_value);
SyxV *syx_eval_builtin(Syx_Env *env, Syx_Builtin *builtin, SyxV *arguments_value);
SyxV *syx_eval_closure(Syx_Env *env, Syx_Closure *closure, SyxV *arguments_value);
SyxV *syx_eval(Syx_Env *env, SyxV *input);

#endif // SYX_EVAL_H

#if defined(SYX_EVAL_IMPL) && !defined(SYX_EVAL_IMPL_C)
#define SYX_EVAL_IMPL_C

#define SYX_VALUE_IMPL
#include "syx_value.h"
// #define SYX_EVAL_SPECIAL_FORMS_IMPLEMENTATION
// #include "expr_eval_special_forms.h"
// #define SYX_EVAL_ARITHMETIC_IMPLEMENTATION
// #include "expr_eval_arithmetic.h"
// #define SYX_EVAL_COMPARISON_IMPLEMENTATION
// #include "expr_eval_comparison.h"
// #define SYX_EVAL_LIST_IMPLEMENTATION
// #include "expr_eval_list.h"
// #define SYX_EVAL_TYPE_PREDICATES_IMPLEMENTATION
// #include "expr_eval_type_predicates.h"
// #define SYX_EVAL_STRING_IMPLEMENTATION
// #include "expr_eval_string.h"
// #define SYX_EVAL_TYPE_CONVERSION_IMPLEMENTATION
// #include "expr_eval_type_conversion.h"
// #define SYX_EVAL_EQUALITY_IMPLEMENTATION
// #include "expr_eval_equality.h"
// #define SYX_EVAL_IO_IMPLEMENTATION
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
#define syxv_list_for_each(value, first) for ( \
  SyxV *value##_it = (first),                  \
    *value = syxv_list_get_value(value##_it);  \
  value##_it != NULL;                          \
  value##_it = syxv_list_next(value##_it),     \
    value = syxv_list_get_value(value##_it)    \
)

void syx_env_destructor(void *data) {
  Syx_Env *env = data;
  ht_free(&env->symbols);
  if (env->parent) rc_release(env->parent);
  free(env->description);
}
Syx_Env *make_syx_env(Syx_Env *parent, char *description) {
  Syx_Env *env = rc_alloc(sizeof(Syx_Env), syx_env_destructor);
  env->parent = parent ? rc_acquire(parent) : NULL;
  env->symbols.hasheq = ht_cstr_hasheq;
  env->description = strdup(description);
  return env;
}
Syx_Env *syx_env_global(Syx_Env *env) {
  while (env->parent != NULL) env = env->parent;
  return env;
}
void syx_env_put(Syx_Env *env, const char *name, SyxV *value) {
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
  } else {
    item = ht_put(&env->symbols, name);
  }
  *item = value;
}
SyxV **syx_env_lookup(Syx_Env *env, const char *name) {
  SyxV **item = NULL;
  while (env != NULL && item == NULL) {
    item = ht_find(&env->symbols, name);
    env = env->parent;
  }
  return item;
}
Syx_Env *make_global_syx_env() {
  Syx_Env *env = make_syx_env(NULL, "<global>");
//   expr_env_init_special_forms(env);
//   expr_env_init_arithmetic(env);
//   expr_env_init_comparison(env);
//   expr_env_init_equality(env);
//   expr_env_init_list(env);
//   expr_env_init_string(env);
//   expr_env_init_type_predicates(env);
//   expr_env_init_type_conversion(env);
//   expr_env_init_io(env);
  return env;
}

Syx_Arguments syx_arguments(Syx_Env *env, SyxV *value) {
  Syx_Arguments arguments = {0};
  syxv_list_for_each(arg, value) {
    if (arg_it == arg) RUNTIME_ERROR("malformed arguments list", env);
    if (arg) da_append(&arguments, arg);
  }
  return arguments;
}

SyxV *syx_eval_specialf(Syx_Env *env, Syx_SpecialF *specialf, SyxV *arguments_value) {
  Syx_Arguments arguments = syx_arguments(env, arguments_value);
  SyxV *result = specialf->eval(env, arguments);
  if (!result) result = make_syxv_nil();
  da_free(arguments);
  return result;
}
SyxV *syx_eval_builtin(Syx_Env *env, Syx_Builtin *builtin, SyxV *arguments_value) {
  Syx_Arguments arguments = syx_arguments(env, arguments_value);
  da_foreach(SyxV *, arg, &arguments) *arg = syx_eval(env, *arg);
  SyxV *result = builtin->eval(env, arguments);
  if (!result) result = make_syxv_nil();
  da_free(arguments);
  return result;
}
SyxV *syx_eval_closure(Syx_Env *env, Syx_Closure *closure, SyxV *arguments_value) {
  Syx_Env *call_env = rc_acquire(make_syx_env(closure->env, closure->name));
  SyxV *it = arguments_value;
  syxv_list_for_each(name_value, closure->arguments) {
    if (!name_value) continue;
    const char *name = name_value->symbol.name;
    if (name_value->kind == SYXV_KIND_NIL) {
      syx_env_put(call_env, name, it);
      it = make_syxv_nil();
      break;
    }
    if (it->kind == SYXV_KIND_NIL) RUNTIME_ERROR("Too few arguments", env);
    syx_env_put(call_env, name, it->pair.left);
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
    SyxV **item = syx_env_lookup(env, input->symbol.name);
    if (item == NULL) RUNTIME_ERROR("unbound symbol", env);
    return *item;
  }
  if (input->kind != SYXV_KIND_PAIR) return input;
  if (!input->pair.left) return input;
  SyxV *head = syx_eval(env, input->pair.left);
  SyxV *arguments = input->pair.right;
  switch (head->kind) {
    case SYXV_KIND_NIL:
    case SYXV_KIND_SYMBOL:
    case SYXV_KIND_PAIR:
    case SYXV_KIND_BOOL:
    case SYXV_KIND_INTEGER:
    case SYXV_KIND_REAL:
    case SYXV_KIND_STRING:
    case SYXV_KIND_QUOTE: RUNTIME_ERROR("is not a procedure", env);
    case SYXV_KIND_SPECIALF: return syx_eval_specialf(env, &head->specialf, arguments);
    case SYXV_KIND_BUILTIN: return syx_eval_builtin(env, &head->builtin, arguments);
    case SYXV_KIND_CLOSURE: return syx_eval_closure(env, &head->closure, arguments);
  }
}

#endif // SYX_EVAL_IMPL
