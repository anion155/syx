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
  SyxV *cond_eval = rc_acquire(syx_eval(env, arguments->items[0]));
  SyxV *cond_bool = rc_acquire(syx_convert_to_bool(env, cond_eval));
  rc_release(cond_eval);
  if (!cond_bool) RUNTIME_ERROR("illegal if condition value", env);
  SyxV *then_body = arguments->items[1];
  SyxV *else_body = arguments->items[2];
  if (cond_bool->boolean) {
    rc_release(cond_bool);
    return syx_eval(env, then_body);
  } else {
    rc_release(cond_bool);
    return syx_eval(env, else_body);
  }
}

/** lambda - Creates a closure and captures current environment */
SyxV *syx_special_form_lambda(Syx_Env *env, SyxVs *arguments) {
  SYX_EVAL_ARGUMENTS_CLAMP(env, 2, 3);
  SyxV *argument_names_list = arguments->items[0];
  SyxV *name_symbol = arguments->count == 3 ? arguments->items[1] : NULL;
  SyxV *body = arguments->count == 2 ? arguments->items[1] : arguments->items[2];
  const char *name;
  if (name_symbol != NULL && name_symbol->kind == SYXV_KIND_SYMBOL) name = name_symbol->symbol.name;
  else name = nanoid("closure-", 10);
  return make_syxv_closure(name, argument_names_list, body, env);
}

/** define - Binds a name in the current environment */
SyxV *syx_special_form_define(Syx_Env *env, SyxVs *arguments) {
  SYX_EVAL_ARGUMENTS_CLAMP(env, 2);
  SyxV *name_symbol = arguments->items[0];
  SyxV *value = arguments->items[1];
  if (name_symbol->kind == SYXV_KIND_PAIR) {
    SyxV *arguments_syxv = name_symbol->pair.right;
    switch (arguments_syxv->kind) {
      case SYXV_KIND_PAIR:
      case SYXV_KIND_NIL: break;
      default: RUNTIME_ERROR("Argument names expected to be a list", env);
    }
    name_symbol = name_symbol->pair.left;
    value = make_syxv_closure(name_symbol->symbol.name, arguments_syxv, value, env);
  } else {
    value = syx_eval(env, value);
  }
  if (name_symbol->kind != SYXV_KIND_SYMBOL) RUNTIME_ERROR("Symbol expression expected as name", env);
  syx_env_define(env, name_symbol->symbol.name, value);
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
  SyxV *name_symbol = arguments->items[0];
  SyxV *value = syx_eval(env, arguments->items[1]);
  if (name_symbol->kind != SYXV_KIND_SYMBOL) RUNTIME_ERROR("Symbol expression expected as name", env);
  syx_env_set(env, name_symbol->symbol.name, value);
  return make_syxv_nil();
}

/** Multi-branch conditional */
SyxV *syx_special_form_cond(Syx_Env *env, SyxVs *arguments) {
  da_foreach(SyxV *, argument, arguments) {
    if ((*argument)->kind != SYXV_KIND_PAIR) RUNTIME_ERROR("Pair expected as every argument of cond", env);
    SyxV *cond_uneval = (*argument)->pair.left;
    SyxV *cond_evaluated;
    if (cond_uneval->kind == SYXV_KIND_SYMBOL && strcmp(cond_uneval->symbol.name, "else") == 0) {
      cond_evaluated = rc_acquire(make_syxv_bool(true));
      goto evaluate_cdr;
    }
    cond_evaluated = rc_acquire(syx_eval(env, cond_uneval));
    SyxV *cond_bool = rc_acquire(syx_convert_to_bool(env, cond_evaluated));
    if (!cond_bool->boolean) {
      rc_release(cond_evaluated);
      rc_release(cond_bool);
      continue;
    }
    rc_release(cond_bool);
  evaluate_cdr:
    SyxV *value = (*argument)->pair.right;
    if (value->kind == SYXV_KIND_NIL) return rc_move(cond_evaluated);
    if (value->kind != SYXV_KIND_PAIR) {
      rc_release(cond_evaluated);
      return syx_eval(env, value);
    }
    if (value->pair.left->kind == SYXV_KIND_SYMBOL && strcmp(value->pair.left->symbol.name, "=>") == 0) {
      SyxV *right = value->pair.right;
      if (right->kind == SYXV_KIND_PAIR && right->pair.right->kind == SYXV_KIND_NIL) {
        SyxV *call = rc_acquire(make_syxv_list(right->pair.left, cond_evaluated, NULL));
        SyxV *result = rc_acquire(syx_eval(env, call));
        rc_release(call);
        rc_release(cond_evaluated);
        return rc_move(result);
      }
    }
    rc_release(cond_evaluated);
    SyxVs *list = rc_acquire(make_syxvs(env, value));
    syxvs_eval(env, list);
    SyxV *result = rc_acquire(list->items[list->count - 1]);
    rc_release(list);
    return rc_move(result);
  }
  return make_syxv_nil();
}

/** Evaluates left to right, returns first falsy value or last value if all truthy */
SyxV *syx_special_form_and(Syx_Env *env, SyxVs *arguments) {
  da_foreach(SyxV *, argument, arguments) {
    SyxV *evaluated = rc_acquire(syx_eval(env, *argument));
    SyxV *cond = rc_acquire(syx_convert_to_bool(env, evaluated));
    if (!cond->boolean) {
      rc_release(cond);
      return evaluated;
    }
    rc_release(cond);
    rc_release(*argument);
    *argument = evaluated;
  }
  return arguments->items[arguments->count - 1];
}

/** Evaluates left to right, returns first truthy value or last value if all falsy */
SyxV *syx_special_form_or(Syx_Env *env, SyxVs *arguments) {
  da_foreach(SyxV *, argument, arguments) {
    SyxV *evaluated = rc_acquire(syx_eval(env, *argument));
    SyxV *cond = rc_acquire(syx_convert_to_bool(env, evaluated));
    if (cond->boolean) {
      rc_release(cond);
      return evaluated;
    }
    rc_release(cond);
    rc_release(*argument);
    *argument = evaluated;
  }
  return arguments->items[arguments->count - 1];
}

/** Create new variable bindings in parallel and execute a series of forms that use these bindings */
SyxV *syx_special_form_let(Syx_Env *env, SyxVs *arguments) {
  Syx_Env *body_env = rc_acquire(make_syx_env(env, "let"));
  SyxV *bindings = arguments->items[0];
  TODO("syx_special_form_let");
  // da_foreach(SyxV *, argument, arguments) {
  //   SyxV *evaluated = rc_acquire(syx_eval(env, *argument));
  //   SyxV *cond = rc_acquire(syx_convert_to_bool(env, evaluated));
  //   if (cond->boolean) {
  //     rc_release(cond);
  //     return evaluated;
  //   }
  //   rc_release(cond);
  //   rc_release(*argument);
  //   *argument = evaluated;
  // }
  // return arguments->items[arguments->count - 1];
}

void syx_env_define_special_forms(Syx_Env *env) {
  /** Special forms */
  syx_env_define(env, "quote", make_syxv_specialf(NULL, syx_special_form_quote));
  syx_env_define(env, "if", make_syxv_specialf(NULL, syx_special_form_if));
  syx_env_define(env, "lambda", make_syxv_specialf(NULL, syx_special_form_lambda));
  syx_env_define(env, "define", make_syxv_specialf(NULL, syx_special_form_define));
  syx_env_define(env, "begin", make_syxv_specialf(NULL, syx_special_form_begin));
  syx_env_define(env, "set!", make_syxv_specialf(NULL, syx_special_form_set_excl));
  syx_env_define(env, "cond", make_syxv_specialf(NULL, syx_special_form_cond));
  syx_env_define(env, "and", make_syxv_specialf(NULL, syx_special_form_and));
  syx_env_define(env, "or", make_syxv_specialf(NULL, syx_special_form_or));
  syx_env_define(env, "let", make_syxv_specialf(NULL, syx_special_form_let));
}

#endif // SYX_EVAL_SPECIALF_IMPL
