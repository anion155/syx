#ifndef SYX_EVAL_SPECIALF_H
#define SYX_EVAL_SPECIALF_H

#include "syx_eval.h"

void syx_env_put_special_forms(Syx_Env *env);

#endif // SYX_EVAL_SPECIALF_H

#if defined(SYX_EVAL_SPECIALF_IMPL) && !defined(SYX_EVAL_SPECIALF_IMPL_C)
#define SYX_EVAL_SPECIALF_IMPL_C

#define NANOID_IMPL
#include <nanoid.h>

/** Special forms */

/** quote - Returns argument unevaluated */
SyxV *syx_special_form_quote(Syx_Env *env, Syx_Arguments *arguments) {
  SYX_EVAL_ARGUMENTS_CLAMP(env, 1);
  return arguments->items[0];
}
/** if - Evaluates condition then evaluates only one branch */
SyxV *syx_special_form_if(Syx_Env *env, Syx_Arguments *arguments) {
  SYX_EVAL_ARGUMENTS_CLAMP(env, 2, 3);
  SyxV *cond_syxv = syx_convert_to_bool(env, syx_eval(env, arguments->items[0]));
  if (!cond_syxv) RUNTIME_ERROR("illegal if condition value", env);
  if (cond_syxv->kind != SYXV_KIND_BOOL) RUNTIME_ERROR("illegal if condition value", env);
  SyxV *then_syxv = arguments->items[1];
  SyxV *else_syxv = arguments->items[2];
  return cond_syxv->boolean ? syx_eval(env, then_syxv) : syx_eval(env, else_syxv);
}
/** lambda - Creates a closure and captures current environment */
SyxV *syx_special_form_lambda(Syx_Env *env, Syx_Arguments *arguments) {
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
SyxV *syx_special_form_define(Syx_Env *env, Syx_Arguments *arguments) {
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
  syx_env_put(env, name_syxv->symbol.name, value_syxv);
  return make_syxv_nil();
}
/** Evaluates expressions in order and returns last */
SyxV *syx_special_form_begin(Syx_Env *env, Syx_Arguments *arguments) {
  arguments = eval_syx_arguments(env, arguments);
  return arguments->items[arguments->count - 1];
}

void syx_env_put_special_forms(Syx_Env *env) {
  /** Special forms */
  syx_env_put(env, "quote", make_syxv_specialf("quote", syx_special_form_quote));
  syx_env_put(env, "if", make_syxv_specialf("if", syx_special_form_if));
  syx_env_put(env, "lambda", make_syxv_specialf("lambda", syx_special_form_lambda));
  syx_env_put(env, "define", make_syxv_specialf("define", syx_special_form_define));
  syx_env_put(env, "begin", make_syxv_specialf("begin", syx_special_form_begin));
}

#endif // SYX_EVAL_SPECIALF_IMPL
