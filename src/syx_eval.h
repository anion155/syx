#ifndef SYX_EVAL_H
#define SYX_EVAL_H

#include <ht.h>
#include <magic.h>
#include <nob.h>
#include <rc.h>

#include "syx_value.h"

#define RUNTIME_ERROR(message, ctx) UNREACHABLE((UNUSED((ctx)), (message)))

typedef Ht(const char *, SyxV *) Syx_Env_Symbols;

struct Syx_Env {
  Syx_Env *parent;
  Syx_Env_Symbols symbols;
  char *description;
};

struct Syx_Eval_Ctx {
  Syx_Env *env;
};

Syx_Eval_Ctx make_syx_ctx(Syx_Env *env);

void syx_env_destructor(void *data);
Syx_Env *make_syx_env(Syx_Env *parent, const char *description);
Syx_Env *syx_env_global(Syx_Env *env);
SyxV **syx_env_lookup(Syx_Env *env, const char *name);
SyxV *syx_env_lookup_get(Syx_Env *env, const char *name);
void syx_env_define(Syx_Env *env, SyxV_Symbol *symbol, SyxV *value);
void syx_env_define_cstr(Syx_Env *env, const char *name, SyxV *value);
void syx_env_set(Syx_Env *env, SyxV_Symbol *symbol, SyxV *value);
void syx_env_set_cstr(Syx_Env *env, const char *name, SyxV *value);
Syx_Env *make_global_syx_env();

SyxV *syx_eval_specialf(Syx_Eval_Ctx *ctx, Syx_SpecialF *specialf, SyxV *arguments);
SyxV *syx_eval_builtin(Syx_Eval_Ctx *ctx, Syx_Builtin *builtin, SyxV *arguments);
SyxV *syx_eval_closure(Syx_Eval_Ctx *ctx, Syx_Closure *closure, SyxV *arguments);
SyxV *syx_eval(Syx_Eval_Ctx *ctx, SyxV *input);

typedef struct {
  bool (*should_stop)(Syx_Eval_Ctx *ctx, SyxV *evaluated);
  SyxV *default_result;
} Syx_Eval_Forms_List_Opt;

SyxV *syx__eval_forms_list_opt(Syx_Eval_Ctx *ctx, SyxV *forms_list, Syx_Eval_Forms_List_Opt opt);
#define syx_eval_forms_list(env, forms_list, ...) syx__eval_forms_list_opt((env), (forms_list), (Syx_Eval_Forms_List_Opt){__VA_ARGS__})

bool syx_convert_to_bool_v(Syx_Eval_Ctx *ctx, SyxV *value);
SyxV *syx_convert_to_bool(Syx_Eval_Ctx *ctx, SyxV *value);
syx_integer_t syx_convert_to_integer_v(Syx_Eval_Ctx *ctx, SyxV *value);
SyxV *syx_convert_to_integer(Syx_Eval_Ctx *ctx, SyxV *value);
syx_fractional_t syx_convert_to_fractional_v(Syx_Eval_Ctx *ctx, SyxV *value);
SyxV *syx_convert_to_fractional(Syx_Eval_Ctx *ctx, SyxV *value);
syx_string_view_t syx_convert_to_string_v(Syx_Eval_Ctx *ctx, SyxV *value);
SyxV *syx_convert_to_string(Syx_Eval_Ctx *ctx, SyxV *value);

#endif // SYX_EVAL_H

#if defined(SYX_EVAL_IMPL) && !defined(SYX_EVAL_IMPL_C)
#define SYX_EVAL_IMPL_C

#define SYX_VALUE_IMPL
#include "syx_value.h"
#define SYX_EVAL_CONSTANTS_IMPL
#include "syx_eval_constants.h"
#define SYX_EVAL_SPECIALF_IMPL
#include "syx_eval_specialf.h"
#define SYX_EVAL_BUILTINS_IMPL
#include "syx_eval_builtins.h"

Syx_Eval_Ctx make_syx_ctx(Syx_Env *env) {
  return (Syx_Eval_Ctx){.env = env};
}

void syx_env_destructor(void *data) {
  Syx_Env *env = data;
  ht_foreach(symbol, &env->symbols) {
    rc_release(ht_key(&env->symbols, symbol));
    rc_release(*symbol);
  }
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
      case SYXV_KIND_FRACTIONAL:
      case SYXV_KIND_STRING:
      case SYXV_KIND_QUOTE: break;
      case SYXV_KIND_SPECIALF: RUNTIME_ERROR("trying to redefine special form", env);
      case SYXV_KIND_BUILTIN: RUNTIME_ERROR("trying to redefine builtin", env);
      case SYXV_KIND_CLOSURE: break;
    }
  }
  return item;
}

void syxv_update_name(SyxV *value, const char *name) {
  switch (value->kind) {
    case SYXV_KIND_SPECIALF: {
      if (!value->specialf.name) value->specialf.name = strdup(name);
    } break;
    case SYXV_KIND_BUILTIN: {
      if (!value->builtin.name) value->builtin.name = strdup(name);
    } break;
    case SYXV_KIND_CLOSURE: {
      if (!value->closure.name) value->closure.name = strdup(name);
    } break;
    default: break;
  }
}

void syx_env_define(Syx_Env *env, SyxV_Symbol *symbol, SyxV *value) {
  rc_acquire(get_syxv_from_symbol(symbol));
  ensure_syxv_redefinable(env, symbol->name);
  *ht_find_or_put(&env->symbols, symbol->name) = rc_acquire(value);
  syxv_update_name(value, symbol->name);
}

void syx_env_define_cstr(Syx_Env *env, const char *name, SyxV *value) {
  SyxV **item = ht_find(&env->symbols, name);
  if (item == NULL) name = rc_manage_strdup(name);
  ensure_syxv_redefinable(env, name);
  *ht_find_or_put(&env->symbols, name) = rc_acquire(value);
  syxv_update_name(value, name);
}

void syx_env_set(Syx_Env *env, SyxV_Symbol *symbol, SyxV *value) {
  rc_acquire(get_syxv_from_symbol(symbol));
  SyxV **item = ensure_syxv_redefinable(env, symbol->name);
  if (item == NULL) item = ht_put(&env->symbols, symbol->name);
  *item = rc_acquire(value);
}

void syx_env_set_cstr(Syx_Env *env, const char *name, SyxV *value) {
  SyxV **item = ensure_syxv_redefinable(env, name);
  if (item == NULL) item = ht_put(&env->symbols, rc_manage_strdup(name));
  *item = rc_acquire(value);
}

Syx_Env *make_global_syx_env() {
  Syx_Env *env = make_syx_env(NULL, "<global>");
  syx_env_define_constants(env);
  syx_env_define_special_forms(env);
  syx_env_define_builtins(env);
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

SyxV *syx_eval_specialf(Syx_Eval_Ctx *ctx, Syx_SpecialF *specialf, SyxV *arguments) {
  SyxV *result = specialf->eval(ctx, arguments);
  if (!result) return make_syxv_nil();
  return result;
}

SyxV *syx_eval_builtin(Syx_Eval_Ctx *ctx, Syx_Builtin *builtin, SyxV *arguments) {
  SyxV *evaluated = NULL;
  syxv_list_map(argument, arguments, &evaluated) {
    *argument = syx_eval(ctx, *argument);
  }
  SyxV *result = builtin->eval(ctx, evaluated);
  if (!result) return make_syxv_nil();
  return result;
}

SyxV *syx_eval_closure(Syx_Eval_Ctx *ctx, Syx_Closure *closure, SyxV *arguments) {
  Syx_Env *call_env = rc_acquire(make_syx_env(closure->env, temp_sprintf("<%s>", closure->name)));
  SyxV *it = arguments;
  SyxV **last_name = NULL;
  syxv_list_for_each(name_v, closure->defines, &last_name) {
    SyxV_Symbol *symbol;
    if (name_v->kind == SYXV_KIND_PAIR) {
      symbol = &name_v->pair.left->symbol;
    } else {
      symbol = &name_v->symbol;
    }
  continue_same_param:
    if (ht_find(&call_env->symbols, symbol->name)) continue;
    if (it->kind == SYXV_KIND_NIL) TODO("Implement smaller arguments passed or currying");
    if (it->kind != SYXV_KIND_PAIR) RUNTIME_ERROR("Malformed arguments list", ctx);
    if (it->pair.left->kind == SYXV_KIND_SYMBOL && it->pair.left->symbol.name[0] == ':') {
      TODO("redone named param binding");
      const char *named_param = it->pair.left->symbol.name + 1;
      it = it->pair.right;
      if (it->kind != SYXV_KIND_PAIR) RUNTIME_ERROR("Malformed arguments list", ctx);
      syx_env_define_cstr(call_env, named_param, syx_eval(ctx, it->pair.left));
      it = it->pair.right;
      goto continue_same_param;
    }
    syx_env_define(call_env, symbol, syx_eval(ctx, it->pair.left));
    it = it->pair.right;
  }
  if ((*last_name)->kind != SYXV_KIND_NIL) {
    SyxV_Symbol *symbol = &(*last_name)->symbol;
    SyxV *rest = it;
    SyxV *evaluated = NULL;
    SyxV **last_argument = NULL;
    syxv_list_map(argument, rest, &evaluated, &last_argument) {
      *argument = syx_eval(ctx, *argument);
    }
    if ((*last_argument)->kind != SYXV_KIND_NIL) {
      *last_argument = rc_acquire(syx_eval(ctx, *last_argument));
    }
    syx_env_define(call_env, symbol, rest);
  }
  syxv_list_for_each(name_v, closure->defines) {
    if (name_v->kind != SYXV_KIND_PAIR) continue;
    const char *name = name_v->pair.left->symbol.name;
    SyxV **value = ht_find(&call_env->symbols, name);
    if (value != NULL) continue;
    (*value) = rc_acquire(syx_eval(ctx, name_v->pair.right));
  }
  Syx_Eval_Ctx call_ctx = {.env = call_env};
  SyxV *result = syx_eval_forms_list(&call_ctx, closure->forms);
  rc_release(call_env);
  return result;
}

SyxV *syx_eval(Syx_Eval_Ctx *ctx, SyxV *input) {
  if (input->kind == SYXV_KIND_QUOTE) return input->quote;
  if (input->kind == SYXV_KIND_SYMBOL) {
    SyxV *item = syx_env_lookup_get(ctx->env, input->symbol.name);
    if (item == NULL) RUNTIME_ERROR(temp_sprintf("unbound symbol '%s'", input->symbol.name), ctx);
    return item;
  }
  if (input->kind != SYXV_KIND_PAIR) return input;
  if (!input->pair.left) return input;
  SyxV *head = rc_acquire(syx_eval(ctx, input->pair.left));
  SyxV *arguments = input->pair.right;
  SyxV *result;
  switch (head->kind) {
    case SYXV_KIND_NIL: RUNTIME_ERROR("nil is not a procedure", ctx);
    case SYXV_KIND_SYMBOL: RUNTIME_ERROR("symbol is not a procedure", ctx);
    case SYXV_KIND_PAIR: RUNTIME_ERROR("pair is not a procedure", ctx);
    case SYXV_KIND_BOOL: RUNTIME_ERROR("bool is not a procedure", ctx);
    case SYXV_KIND_INTEGER: RUNTIME_ERROR("integer is not a procedure", ctx);
    case SYXV_KIND_FRACTIONAL: RUNTIME_ERROR("fractional is not a procedure", ctx);
    case SYXV_KIND_STRING: RUNTIME_ERROR("string is not a procedure", ctx);
    case SYXV_KIND_QUOTE: RUNTIME_ERROR("quote is not a procedure", ctx);
    case SYXV_KIND_SPECIALF: result = syx_eval_specialf(ctx, &head->specialf, arguments); break;
    case SYXV_KIND_BUILTIN: result = syx_eval_builtin(ctx, &head->builtin, arguments); break;
    case SYXV_KIND_CLOSURE: result = syx_eval_closure(ctx, &head->closure, arguments); break;
  }
  rc_release(head);
  return result;
}

SyxV *syx__eval_forms_list_opt(Syx_Eval_Ctx *ctx, SyxV *forms_list, Syx_Eval_Forms_List_Opt opt) {
  SyxV *result = NULL;
  if (opt.default_result) result = rc_acquire(opt.default_result);
  syxv_list_for_each(form, forms_list) {
    if (result) rc_release(result);
    result = rc_acquire(syx_eval(ctx, form));
    if (opt.should_stop != NULL && opt.should_stop(ctx, result)) return rc_move(result);
  }
  if (result == NULL) RUNTIME_ERROR("empty forms list", ctx);
  return rc_move(result);
}

bool syx_convert_to_bool_v(Syx_Eval_Ctx *ctx, SyxV *value) {
  switch (value->kind) {
    case SYXV_KIND_NIL: return (false);
    case SYXV_KIND_SYMBOL: return (true);
    case SYXV_KIND_PAIR: return (true);
    case SYXV_KIND_BOOL: return value->boolean;
    case SYXV_KIND_INTEGER: return ((syx_bool_t)value->integer);
    case SYXV_KIND_FRACTIONAL: return ((syx_bool_t)value->fractional);
    case SYXV_KIND_STRING: return (bool)value->string.count;
    case SYXV_KIND_QUOTE: return syx_convert_to_bool_v(ctx, value->quote);
    case SYXV_KIND_SPECIALF: return (true);
    case SYXV_KIND_BUILTIN: return (true);
    case SYXV_KIND_CLOSURE: return (true);
  }
}

SyxV *syx_convert_to_bool(Syx_Eval_Ctx *ctx, SyxV *value) {
  if (value->kind == SYXV_KIND_BOOL) return value;
  return make_syxv_bool(syx_convert_to_bool_v(ctx, value));
}

syx_integer_t syx_convert_to_integer_v(Syx_Eval_Ctx *ctx, SyxV *value) {
  switch (value->kind) {
    case SYXV_KIND_NIL: return 0;
    case SYXV_KIND_SYMBOL: RUNTIME_ERROR("illegal conversion of symbol to integer number", ctx);
    case SYXV_KIND_PAIR: RUNTIME_ERROR("illegal conversion of pair to integer number", ctx);
    case SYXV_KIND_BOOL: return value->boolean ? 1 : 0;
    case SYXV_KIND_INTEGER: return value->integer;
    case SYXV_KIND_FRACTIONAL: return (syx_integer_t)value->fractional;
    case SYXV_KIND_STRING: {
      String_View sv = value->string;
      syx_integer_t result = 0;
      if (!parse_integer(&sv, &result)) RUNTIME_ERROR("illegal conversion of string to integer number", ctx);
      return result;
    }
    case SYXV_KIND_QUOTE: return syx_convert_to_integer_v(ctx, value->quote);
    case SYXV_KIND_SPECIALF: RUNTIME_ERROR("illegal conversion of special form to integer number", ctx);
    case SYXV_KIND_BUILTIN: RUNTIME_ERROR("illegal conversion of builtin function to integer number", ctx);
    case SYXV_KIND_CLOSURE: RUNTIME_ERROR("illegal conversion of closure to integer number", ctx);
  }
}

SyxV *syx_convert_to_integer(Syx_Eval_Ctx *ctx, SyxV *value) {
  switch (value->kind) {
    case SYXV_KIND_SYMBOL: return NULL;
    case SYXV_KIND_PAIR: return NULL;
    case SYXV_KIND_INTEGER: return value;
    case SYXV_KIND_STRING: {
      String_View sv = value->string;
      syx_integer_t result = 0;
      if (!parse_integer(&sv, &result)) return NULL;
      return make_syxv_integer(result);
    }
    case SYXV_KIND_SPECIALF: return NULL;
    case SYXV_KIND_BUILTIN: return NULL;
    case SYXV_KIND_CLOSURE: return NULL;
    default: return make_syxv_integer(syx_convert_to_integer_v(ctx, value));
  }
}

syx_fractional_t syx_convert_to_fractional_v(Syx_Eval_Ctx *ctx, SyxV *value) {
  switch (value->kind) {
    case SYXV_KIND_NIL: return 0.0;
    case SYXV_KIND_SYMBOL: RUNTIME_ERROR("illegal conversion of symbol to fractional number", ctx);
    case SYXV_KIND_PAIR: RUNTIME_ERROR("illegal conversion of pair to fractional number", ctx);
    case SYXV_KIND_BOOL: return value->boolean ? 1.0 : 0.0;
    case SYXV_KIND_INTEGER: return (syx_fractional_t)value->integer;
    case SYXV_KIND_FRACTIONAL: return value->fractional;
    case SYXV_KIND_STRING: {
      String_View sv = value->string;
      syx_fractional_t result = 0;
      if (!parse_fractional(&sv, &result)) RUNTIME_ERROR("illegal conversion of string to fractional number", ctx);
      return result;
    }
    case SYXV_KIND_QUOTE: return syx_convert_to_fractional_v(ctx, value->quote);
    case SYXV_KIND_SPECIALF: RUNTIME_ERROR("illegal conversion of special form to fractional number", ctx);
    case SYXV_KIND_BUILTIN: RUNTIME_ERROR("illegal conversion of builtin function to fractional number", ctx);
    case SYXV_KIND_CLOSURE: RUNTIME_ERROR("illegal conversion of closure to fractional number", ctx);
  }
}

SyxV *syx_convert_to_fractional(Syx_Eval_Ctx *ctx, SyxV *value) {
  switch (value->kind) {
    case SYXV_KIND_SYMBOL: return NULL;
    case SYXV_KIND_PAIR: return NULL;
    case SYXV_KIND_FRACTIONAL: return value;
    case SYXV_KIND_STRING: {
      String_View sv = value->string;
      syx_fractional_t result = 0;
      if (!parse_fractional(&sv, &result)) return NULL;
      return make_syxv_fractional(result);
    }
    case SYXV_KIND_SPECIALF: return NULL;
    case SYXV_KIND_BUILTIN: return NULL;
    case SYXV_KIND_CLOSURE: return NULL;
    default: return make_syxv_fractional(syx_convert_to_fractional_v(ctx, value));
  }
}

syx_string_view_t syx_convert_to_string_v(Syx_Eval_Ctx *ctx, SyxV *value) {
  switch (value->kind) {
    case SYXV_KIND_NIL: return sv_from_cstr("nil");
    case SYXV_KIND_SYMBOL: RUNTIME_ERROR("illegal conversion of symbol to string", ctx);
    case SYXV_KIND_PAIR: RUNTIME_ERROR("illegal conversion of pair to string", ctx);
    case SYXV_KIND_BOOL: return sv_from_cstr(value->boolean ? "true" : "false");
    case SYXV_KIND_INTEGER: return stringify_integer(value->integer);
    case SYXV_KIND_FRACTIONAL: return stringify_fractional(value->fractional);
    case SYXV_KIND_STRING: return value->string;
    case SYXV_KIND_QUOTE: return syx_convert_to_string_v(ctx, value->quote);
    case SYXV_KIND_SPECIALF: RUNTIME_ERROR("illegal conversion of special form to string", ctx);
    case SYXV_KIND_BUILTIN: RUNTIME_ERROR("illegal conversion of builtin function to string", ctx);
    case SYXV_KIND_CLOSURE: RUNTIME_ERROR("illegal conversion of closure to string", ctx);
  }
}

SyxV *syx_convert_to_string(Syx_Eval_Ctx *ctx, SyxV *value) {
  switch (value->kind) {
    case SYXV_KIND_SYMBOL: return NULL;
    case SYXV_KIND_PAIR: return NULL;
    case SYXV_KIND_STRING: return value;
    case SYXV_KIND_SPECIALF: return NULL;
    case SYXV_KIND_BUILTIN: return NULL;
    case SYXV_KIND_CLOSURE: return NULL;
    default: {
      syx_string_view_t str = syx_convert_to_string_v(ctx, value);
      SyxV *syxv = make_syxv_string(str);
      // TODO: delete str
      return syxv;
    }
  }
}

#endif // SYX_EVAL_IMPL
