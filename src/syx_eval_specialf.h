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

/** Returns first argument unevaluated */
SyxV *syx_special_form_quote(Syx_Env *env, SyxV *arguments) {
  UNUSED(env);
  return syxv_list_next(&arguments);
}

/** Evaluates forms in order and returns last result. */
SyxV *syx_special_form_begin(Syx_Env *env, SyxV *arguments) {
  return syx_eval_forms_list(env, arguments);
}

/** Creates a closure that captures current environment. */
SyxV *syx_special_form_lambda(Syx_Env *env, SyxV *arguments) {
  SyxV *defines = syxv_list_next(&arguments);
  SyxV **rest_define = NULL;
  syxv_list_for_each(define, defines, &rest_define) {
    if ((*define)->kind == SYXV_KIND_SYMBOL) continue;
    if ((*define)->kind != SYXV_KIND_PAIR) RUNTIME_ERROR("malformed lambda arguments definitions list", env);
    if ((*define)->pair.left->kind != SYXV_KIND_SYMBOL) RUNTIME_ERROR("malformed lambda arguments definitions list", env);
    if ((*define)->pair.right->kind == SYXV_KIND_PAIR) {
      if ((*define)->pair.right->pair.right->kind != SYXV_KIND_NIL) RUNTIME_ERROR("malformed lambda arguments definitions list", env);
      SyxV *value = rc_acquire((*define)->pair.right->pair.left);
      rc_release((*define)->pair.right);
      (*define)->pair.right = value;
    }
  }
  if ((*rest_define)->kind != SYXV_KIND_NIL) {
    if ((*rest_define)->kind != SYXV_KIND_SYMBOL) RUNTIME_ERROR("malformed lambda rest argument", env);
  }
  SyxV *body = arguments;
  if (body->kind != SYXV_KIND_PAIR) RUNTIME_ERROR("malformed lambda body", env);
  return make_syxv_closure(NULL, defines, body, env);
}

/** Binds a name in the current environment. */
SyxV *syx_special_form_define(Syx_Env *env, SyxV *arguments) {
  SyxV *name_s = syxv_list_next(&arguments);
  SyxV *value;
  if (name_s->kind == SYXV_KIND_PAIR) {
    SyxV *pair = rc_acquire(make_syxv_pair(name_s->pair.right, arguments));
    value = syx_special_form_lambda(env, pair);
    rc_release(pair);
  } else {
    if (name_s->kind != SYXV_KIND_SYMBOL) RUNTIME_ERROR("Symbol expression expected as name", env);
    value = syx_eval(env, syxv_list_next(&arguments));
  }
  syx_env_define(env, name_s->symbol.name, value);
  return make_syxv_nil();
}

/** Mutate an existing binding or creates new one in current environment. */
SyxV *syx_special_form_set_excl(Syx_Env *env, SyxV *arguments) {
  SyxV *name_s = syxv_list_next(&arguments);
  if (name_s->kind != SYXV_KIND_SYMBOL) RUNTIME_ERROR("Symbol expression expected as name", env);
  SyxV *value = syx_eval(env, syxv_list_next(&arguments));
  syx_env_set(env, name_s->symbol.name, value);
  return make_syxv_nil();
}

/** Create new variable bindings in parallel on new environment and execute a series of forms in that environment. */
SyxV *syx_special_form_let(Syx_Env *env, SyxV *arguments) {
  Syx_Env *body_env = rc_acquire(make_syx_env(env, "let"));
  SyxV *bindings = syxv_list_next(&arguments);
  SyxV **last_binding = NULL;
  syxv_list_for_each(binding, bindings, &last_binding) {
    if ((*binding)->kind != SYXV_KIND_PAIR) RUNTIME_ERROR("malformed let definition, list expected", env);
    if ((*binding)->pair.left->kind != SYXV_KIND_SYMBOL) RUNTIME_ERROR("malformed let definition, symbol as name expected", env);
    if ((*binding)->pair.right->kind == SYXV_KIND_PAIR) {
      if ((*binding)->pair.right->pair.right->kind != SYXV_KIND_NIL) RUNTIME_ERROR("malformed let definition, too long list", env);
      SyxV *value = rc_acquire((*binding)->pair.right->pair.left);
      rc_release((*binding)->pair.right);
      (*binding)->pair.right = value;
    }
    (*binding)->pair.right = syx_eval_with_release(env, (*binding)->pair.right);
  }
  if ((*last_binding)->kind != SYXV_KIND_NIL) RUNTIME_ERROR("malformed let definitions list", env);
  syxv_list_for_each(binding, bindings) {
    SyxV *name = (*binding)->pair.left;
    SyxV *value = (*binding)->pair.right;
    syx_env_define(body_env, name->symbol.name, value);
  }
  return syx_eval_forms_list(body_env, arguments);
}

// /** Evaluates left to right, returns first falsy value or last value if all truthy */
// SyxV *syx_special_form_and(Syx_Env *env, SyxVs *arguments) {
//   da_foreach(SyxV *, argument, arguments) {
//     SyxV *evaluated = rc_acquire(syx_eval(env, *argument));
//     SyxV *cond = rc_acquire(syx_convert_to_bool(env, evaluated));
//     if (!cond->boolean) {
//       rc_release(cond);
//       return evaluated;
//     }
//     rc_release(cond);
//     rc_release(*argument);
//     *argument = evaluated;
//   }
//   return arguments->items[arguments->count - 1];
// }

// /** Evaluates left to right, returns first truthy value or last value if all falsy */
// SyxV *syx_special_form_or(Syx_Env *env, SyxVs *arguments) {
//   da_foreach(SyxV *, argument, arguments) {
//     SyxV *evaluated = rc_acquire(syx_eval(env, *argument));
//     SyxV *cond = rc_acquire(syx_convert_to_bool(env, evaluated));
//     if (cond->boolean) {
//       rc_release(cond);
//       return evaluated;
//     }
//     rc_release(cond);
//     rc_release(*argument);
//     *argument = evaluated;
//   }
//   return arguments->items[arguments->count - 1];
// }

// /** if - Evaluates condition then evaluates only one branch */
// SyxV *syx_special_form_if(Syx_Env *env, SyxVs *arguments) {
//   SYX_EVAL_ARGUMENTS_CLAMP(env, 2, 3);
//   SyxV *cond_eval = rc_acquire(syx_eval(env, arguments->items[0]));
//   SyxV *cond_bool = rc_acquire(syx_convert_to_bool(env, cond_eval));
//   rc_release(cond_eval);
//   if (!cond_bool) RUNTIME_ERROR("illegal if condition value", env);
//   SyxV *then_body = arguments->items[1];
//   SyxV *else_body = arguments->items[2];
//   if (cond_bool->boolean) {
//     rc_release(cond_bool);
//     return syx_eval(env, then_body);
//   } else {
//     rc_release(cond_bool);
//     return syx_eval(env, else_body);
//   }
// }

// /** Multi-branch conditional */
// SyxV *syx_special_form_cond(Syx_Env *env, SyxVs *arguments) {
//   da_foreach(SyxV *, argument, arguments) {
//     if ((*argument)->kind != SYXV_KIND_PAIR) RUNTIME_ERROR("Pair expected as every argument of cond", env);
//     SyxV *cond_uneval = (*argument)->pair.left;
//     SyxV *cond_evaluated;
//     if (cond_uneval->kind == SYXV_KIND_SYMBOL && strcmp(cond_uneval->symbol.name, "else") == 0) {
//       cond_evaluated = rc_acquire(make_syxv_bool(true));
//       goto evaluate_cdr;
//     }
//     cond_evaluated = rc_acquire(syx_eval(env, cond_uneval));
//     SyxV *cond_bool = rc_acquire(syx_convert_to_bool(env, cond_evaluated));
//     if (!cond_bool->boolean) {
//       rc_release(cond_evaluated);
//       rc_release(cond_bool);
//       continue;
//     }
//     rc_release(cond_bool);
//   evaluate_cdr:
//     SyxV *value = (*argument)->pair.right;
//     if (value->kind == SYXV_KIND_NIL) return rc_move(cond_evaluated);
//     if (value->kind != SYXV_KIND_PAIR) {
//       rc_release(cond_evaluated);
//       return syx_eval(env, value);
//     }
//     if (value->pair.left->kind == SYXV_KIND_SYMBOL && strcmp(value->pair.left->symbol.name, "=>") == 0) {
//       SyxV *right = value->pair.right;
//       if (right->kind == SYXV_KIND_PAIR && right->pair.right->kind == SYXV_KIND_NIL) {
//         SyxV *call = rc_acquire(make_syxv_list(right->pair.left, cond_evaluated, NULL));
//         SyxV *result = rc_acquire(syx_eval(env, call));
//         rc_release(call);
//         rc_release(cond_evaluated);
//         return rc_move(result);
//       }
//     }
//     rc_release(cond_evaluated);
//     SyxVs *list = rc_acquire(make_syxvs(env, value));
//     syxvs_eval(env, list);
//     SyxV *result = rc_acquire(list->items[list->count - 1]);
//     rc_release(list);
//     return rc_move(result);
//   }
//   return make_syxv_nil();
// }

void syx_env_define_special_forms(Syx_Env *env) {
  /** Special forms */
  syx_env_define(env, "quote", make_syxv_specialf(NULL, syx_special_form_quote));
  syx_env_define(env, "begin", make_syxv_specialf(NULL, syx_special_form_begin));
  syx_env_define(env, "lambda", make_syxv_specialf(NULL, syx_special_form_lambda));
  syx_env_define(env, "define", make_syxv_specialf(NULL, syx_special_form_define));
  syx_env_define(env, "set!", make_syxv_specialf(NULL, syx_special_form_set_excl));
  syx_env_define(env, "let", make_syxv_specialf(NULL, syx_special_form_let));
  // syx_env_define(env, "and", make_syxv_specialf(NULL, syx_special_form_and));
  // syx_env_define(env, "or", make_syxv_specialf(NULL, syx_special_form_or));
  // syx_env_define(env, "if", make_syxv_specialf(NULL, syx_special_form_if));
  // syx_env_define(env, "cond", make_syxv_specialf(NULL, syx_special_form_cond));
}

#endif // SYX_EVAL_SPECIALF_IMPL
