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
  SyxV *cond_value = syx_convert_to_bool(env, syx_eval(env, arguments->items[0]));
  if (!cond_value) RUNTIME_ERROR("illegal if condition value", env);
  if (cond_value->kind != SYXV_KIND_BOOL) RUNTIME_ERROR("illegal if condition value", env);
  SyxV *then_value = arguments->items[1];
  SyxV *else_value = arguments->items[2];
  return cond_value->boolean ? syx_eval(env, then_value) : syx_eval(env, else_value);
}
/** lambda - Creates a closure and captures current environment */
SyxV *syx_special_form_lambda(Syx_Env *env, Syx_Arguments *arguments) {
  SYX_EVAL_ARGUMENTS_CLAMP(env, 2, 3);
  SyxV *argument_names_list = arguments->items[0];
  SyxV *name_value = arguments->count == 3 ? arguments->items[1] : NULL;
  SyxV *body_value = arguments->count == 2 ? arguments->items[1] : arguments->items[2];
  const char *name;
  if (name_value != NULL && name_value->kind == SYXV_KIND_SYMBOL) name = name_value->symbol.name;
  else name = nanoid("closure-", 10);
  return make_syxv_closure(name, argument_names_list, body_value, env);
}
/** define - Binds a name in the current environment */
SyxV *syx_special_form_define(Syx_Env *env, Syx_Arguments *arguments) {
  TODO("syx_special_form_define"); UNUSED(env); UNUSED(arguments);
  // if (arguments.count != 2) RUNTIME_ERROR("Incorrect amount of arguments");
  // Expr_Value name_val = arguments.items[0];
  // if (name_val.kind != EXPR_VALUE_KIND_EXPR) RUNTIME_ERROR("Name expected to be a symbol");
  // Expr_Value value_val = arguments.items[1];
  // if (name_val.expr->kind == EXPR_KIND_PAIR) {
  //   Expr *arguments_expr = name_val.expr->pair.right;
  //   if (arguments_expr->kind != EXPR_KIND_PAIR) RUNTIME_ERROR("Arguments expected to be a list");
  //   name_val.expr = name_val.expr->pair.left;
  //   if (value_val.kind != EXPR_VALUE_KIND_EXPR) RUNTIME_ERROR("Body expected to be a list");
  //   value_val = expr_special_form_lambda(env, expr_arguments(make_expr_list(
  //     arguments_expr,
  //     value_val.expr,
  //     NULL
  //   )));
  // } else {
  //   value_val = expr_eval(env, value_val);
  // }
  // if (name_val.expr->kind != EXPR_KIND_SYMBOL) RUNTIME_ERROR("Symbol expression expected as name");
  // expr_env_put_symbol(env, name_val.expr->symbol.name, value_val);
  // return (Expr_Value){.kind = EXPR_VALUE_KIND_EXPR, .expr = &EXPR_NIL};
}
/** Evaluates expressions in order and returns last */
SyxV *syx_special_form_begin(Syx_Env *env, Syx_Arguments *arguments) {
  TODO("syx_special_form_begin"); UNUSED(env); UNUSED(arguments);
  // da_foreach(Expr_Value, argument, &arguments) {
  //   if (argument->kind != EXPR_VALUE_KIND_EXPR) RUNTIME_ERROR("Every begin item supposed to be expressions");
  //   *argument = expr_eval(env, *argument);
  // }
  // return arguments.items[arguments.count - 1];
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
