#ifndef SYX_EVAL_SPECIALF_H
#define SYX_EVAL_SPECIALF_H

#include "syx_eval.h"

void syx_env_define_special_forms(Syx_Env *env);

#endif // SYX_EVAL_SPECIALF_H

#if defined(SYX_EVAL_SPECIALF_IMPL) && !defined(SYX_EVAL_SPECIALF_IMPL_C)
#define SYX_EVAL_SPECIALF_IMPL_C

#define NANOID_IMPL
#include <nanoid.h>

/** Special forms */

/** quote - Returns argument unevaluated */
SyxV *syx_special_form_quote(Syx_Env *env, SyxVs *arguments) {
  SYX_EVAL_ARGUMENTS_CLAMP(env, 1);
  return arguments->items[0];
}

/** if - Evaluates condition then evaluates only one branch */
SyxV *syx_special_form_if(Syx_Env *env, SyxVs *arguments) {
  SYX_EVAL_ARGUMENTS_CLAMP(env, 2, 3);
  SyxV *cond_eval_syxv = rc_acquire(syx_eval(env, arguments->items[0]));
  SyxV *cond_bool = rc_acquire(syx_convert_to_bool(env, cond_eval_syxv));
  rc_release(cond_eval_syxv);
  if (!cond_bool) RUNTIME_ERROR("illegal if condition value", env);
  SyxV *then_syxv = arguments->items[1];
  SyxV *else_syxv = arguments->items[2];
  if (cond_bool->boolean) {
    rc_release(cond_bool);
    return syx_eval(env, then_syxv);
  } else {
    rc_release(cond_bool);
    return syx_eval(env, else_syxv);
  }
}

/** lambda - Creates a closure and captures current environment */
SyxV *syx_special_form_lambda(Syx_Env *env, SyxVs *arguments) {
  SYX_EVAL_ARGUMENTS_CLAMP(env, 2, 3);
  SyxV *argument_names_list = arguments->items[0];
  SyxV *name_syxv = arguments->count == 3 ? arguments->items[1] : NULL;
  SyxV *body_syxv = arguments->count == 2 ? arguments->items[1] : arguments->items[2];
  const char *name;
  if (name_syxv != NULL && name_syxv->kind == SYXV_KIND_SYMBOL) name = name_syxv->symbol.name;
  else name = nanoid("closure-", 10);
  return make_syxv_closure(name, argument_names_list, body_syxv, env);
}

/** define - Binds a name in the current environment */
SyxV *syx_special_form_define(Syx_Env *env, SyxVs *arguments) {
  SYX_EVAL_ARGUMENTS_CLAMP(env, 2);
  SyxV *name_syxv = arguments->items[0];
  SyxV *value_syxv = arguments->items[1];
  if (name_syxv->kind == SYXV_KIND_PAIR) {
    SyxV *arguments_syxv = name_syxv->pair.right;
    switch (arguments_syxv->kind) {
      case SYXV_KIND_PAIR:
      case SYXV_KIND_NIL: break;
      default: RUNTIME_ERROR("Argument names expected to be a list", env);
    }
    name_syxv = name_syxv->pair.left;
    value_syxv = make_syxv_closure(name_syxv->symbol.name, arguments_syxv, value_syxv, env);
  } else {
    value_syxv = syx_eval(env, value_syxv);
  }
  if (name_syxv->kind != SYXV_KIND_SYMBOL) RUNTIME_ERROR("Symbol expression expected as name", env);
  syx_env_define(env, name_syxv->symbol.name, value_syxv);
  return make_syxv_nil();
}

/** Evaluates expressions in order and returns last */
SyxV *syx_special_form_begin(Syx_Env *env, SyxVs *arguments) {
  arguments = syxvs_eval(env, arguments);
  return arguments->items[arguments->count - 1];
}

/** Mutate an existing binding */
SyxV *syx_special_form_set_excl(Syx_Env *env, SyxVs *arguments) {
  SYX_EVAL_ARGUMENTS_CLAMP(env, 2);
  SyxV *name_syxv = arguments->items[0];
  SyxV *value_syxv = syx_eval(env, arguments->items[1]);
  if (name_syxv->kind != SYXV_KIND_SYMBOL) RUNTIME_ERROR("Symbol expression expected as name", env);
  syx_env_set(env, name_syxv->symbol.name, value_syxv);
  return make_syxv_nil();
}

/** Multi-branch conditional */
SyxV *syx_special_form_cond(Syx_Env *env, SyxVs *arguments) {
  da_foreach(SyxV *, argument, arguments) {
    if ((*argument)->kind != SYXV_KIND_PAIR) RUNTIME_ERROR("Pair expected as every argument of cond", env);
    SyxV *cond_syxv = (*argument)->pair.left;
    SyxV *cond_eval_syxv;
    if (cond_syxv->kind == SYXV_KIND_SYMBOL && strcmp(cond_syxv->symbol.name, "else") == 0) {
      cond_eval_syxv = rc_acquire(make_syxv_nil());
      goto evaluate_cdr;
    }
    cond_eval_syxv = rc_acquire(syx_eval(env, cond_syxv));
    SyxV *cond_bool = rc_acquire(syx_convert_to_bool(env, cond_eval_syxv));
    if (!cond_bool->boolean) {
      rc_release(cond_eval_syxv);
      rc_release(cond_bool);
      continue;
    }
    rc_release(cond_bool);
  evaluate_cdr:
    SyxV *value_syxv = (*argument)->pair.right;
    if (value_syxv->kind == SYXV_KIND_NIL) return rc_move(cond_eval_syxv);
    rc_release(cond_eval_syxv);
    if (value_syxv->kind != SYXV_KIND_PAIR) return syx_eval(env, value_syxv);
    SyxVs *list = rc_acquire(make_syxvs(env, value_syxv));
    syxvs_eval(env, list);
    SyxV *result = rc_acquire(list->items[list->count - 1]);
    rc_release(list);
    return rc_move(result);
  }
  return make_syxv_nil();
}

void syx_env_define_special_forms(Syx_Env *env) {
  /** Special forms */
  syx_env_define(env, "quote", make_syxv_specialf("quote", syx_special_form_quote));
  syx_env_define(env, "if", make_syxv_specialf("if", syx_special_form_if));
  syx_env_define(env, "lambda", make_syxv_specialf("lambda", syx_special_form_lambda));
  syx_env_define(env, "define", make_syxv_specialf("define", syx_special_form_define));
  syx_env_define(env, "begin", make_syxv_specialf("begin", syx_special_form_begin));
  syx_env_define(env, "set!", make_syxv_specialf("set!", syx_special_form_set_excl));
  syx_env_define(env, "cond", make_syxv_specialf("cond", syx_special_form_cond));
}

#endif // SYX_EVAL_SPECIALF_IMPL
