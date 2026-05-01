#ifndef SYX_EVAL_SPECIALF_H
#define SYX_EVAL_SPECIALF_H

#include "syx_eval.h"

void syx_env_define_special_forms(Syx_Env *env);

#endif // SYX_EVAL_SPECIALF_H

#if defined(SYX_EVAL_SPECIALF_IMPL) && !defined(SYX_EVAL_SPECIALF_IMPL_C)
#define SYX_EVAL_SPECIALF_IMPL_C

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

SyxV *syx__special_form_make_lambda(Syx_Env *env, const char *name, SyxV *defines, SyxV *forms) {
  if (defines->kind == SYXV_KIND_NIL) RUNTIME_ERROR("malformed lambda arguments definitions expected", env);
  SyxV **rest_define = NULL;
  syxv_list_for_each(env, define, defines, &rest_define) {
    if (define->kind == SYXV_KIND_SYMBOL) continue;
    if (define->kind != SYXV_KIND_PAIR) RUNTIME_ERROR("malformed lambda arguments definitions list", env);
    if (define->pair.left->kind != SYXV_KIND_SYMBOL) RUNTIME_ERROR("malformed lambda arguments definitions list", env);
    if (define->pair.right->kind == SYXV_KIND_PAIR) {
      if (define->pair.right->pair.right->kind != SYXV_KIND_NIL) RUNTIME_ERROR("malformed lambda arguments definitions list", env);
    }
  }
  if ((*rest_define)->kind != SYXV_KIND_NIL) {
    if ((*rest_define)->kind != SYXV_KIND_SYMBOL) RUNTIME_ERROR("malformed lambda rest argument", env);
  }
  if (forms->kind != SYXV_KIND_PAIR) RUNTIME_ERROR("malformed lambda body", env);
  return make_syxv_closure(name, defines, forms, env);
}

/** Creates a closure that captures current environment. */
SyxV *syx_special_form_lambda(Syx_Env *env, SyxV *arguments) {
  return syx__special_form_make_lambda(env, NULL, syxv_list_next(&arguments), arguments);
}

/** Binds a name in the current environment. */
SyxV *syx_special_form_define(Syx_Env *env, SyxV *arguments) {
  SyxV *name_s = syxv_list_next(&arguments);
  SyxV *value;
  if (name_s->kind == SYXV_KIND_PAIR) {
    SyxV *defines = name_s->pair.right;
    name_s = name_s->pair.left;
    if (name_s->kind != SYXV_KIND_SYMBOL) RUNTIME_ERROR("Symbol expression expected as name", env);
    value = syx__special_form_make_lambda(env, name_s->symbol.name, defines, arguments);
  } else if (name_s->kind != SYXV_KIND_SYMBOL) {
    RUNTIME_ERROR("Symbol expression expected as name", env);
  } else {
    value = syx_eval(env, syxv_list_next(&arguments));
  }
  syx_env_define(env, &name_s->symbol, value);
  return make_syxv_nil();
}

/** Mutate an existing binding or creates new one in current environment. */
SyxV *syx_special_form_set(Syx_Env *env, SyxV *arguments) {
  SyxV *name_s = syxv_list_next(&arguments);
  if (name_s->kind != SYXV_KIND_SYMBOL) RUNTIME_ERROR("Symbol expression expected as name", env);
  SyxV *value = syx_eval(env, syxv_list_next(&arguments));
  syx_env_set(env, &name_s->symbol, value);
  return make_syxv_nil();
}

/** Create new variable bindings in parallel on new environment and execute a series of forms in that environment. */
SyxV *syx_special_form_let(Syx_Env *env, SyxV *arguments) {
  Syx_Env *body_env = rc_acquire(make_syx_env(env, NULL));
  body_env->description = strdup(temp_sprintf("let<%p>", body_env));
  SyxV *bindings_src = syxv_list_next(&arguments);
  if (bindings_src->kind != SYXV_KIND_PAIR) RUNTIME_ERROR("List of definitions expected", env);
  SyxV *bindings = NULL;
  syxv_list_map(env, binding, bindings_src, &bindings) {
    if ((*binding)->kind != SYXV_KIND_PAIR) RUNTIME_ERROR("malformed let definition, list expected", env);
    if ((*binding)->pair.left->kind != SYXV_KIND_SYMBOL) RUNTIME_ERROR("malformed let definition, symbol as name expected", env);
    if ((*binding)->pair.right->kind == SYXV_KIND_PAIR) {
      if ((*binding)->pair.right->pair.right->kind != SYXV_KIND_NIL) RUNTIME_ERROR("malformed let definition, too long list", env);
    }
    (*binding) = make_syxv_pair(
        (*binding)->pair.left,
        syx_eval(env, (*binding)->pair.right->pair.left));
  }
  syxv_list_for_each(env, binding, bindings) {
    SyxV *name = binding->pair.left;
    SyxV *value = binding->pair.right;
    syx_env_define(body_env, &name->symbol, value);
  }
  return syx_eval_forms_list(body_env, arguments);
}

bool syx_special_form_and_reduce(Syx_Env *env, SyxV *evaluated) {
  return !syx_convert_to_bool_v(env, evaluated);
}

/** Evaluates left to right, returns first falsy value or last value if all truthy */
SyxV *syx_special_form_and(Syx_Env *env, SyxV *arguments) {
  return syx_eval_forms_list(env, arguments, .should_stop = syx_special_form_and_reduce, .default_result = make_syxv_nil());
}

bool syx_special_form_or_reduce(Syx_Env *env, SyxV *evaluated) {
  return syx_convert_to_bool_v(env, evaluated);
}

/** Evaluates left to right, returns first truthy value or last value if all falsy */
SyxV *syx_special_form_or(Syx_Env *env, SyxV *arguments) {
  return syx_eval_forms_list(env, arguments, .should_stop = syx_special_form_or_reduce, .default_result = make_syxv_nil());
}

/** if - Evaluates condition then evaluates only one branch */
SyxV *syx_special_form_if(Syx_Env *env, SyxV *arguments) {
  SyxV *result = rc_acquire(syx_eval(env, syxv_list_next(&arguments)));
  bool cond = syx_convert_to_bool_v(env, result);
  SyxV *then_body = syxv_list_next(&arguments);
  SyxV *else_body = syxv_list_next(&arguments);
  if (cond) return syx_eval(env, then_body);
  else return syx_eval(env, else_body);
}

/** Multi-branch conditional */
SyxV *syx_special_form_cond(Syx_Env *env, SyxV *arguments) {
  SyxV *result = NULL;
  syxv_list_for_each(env, branch, arguments) {
    if (branch->kind != SYXV_KIND_PAIR) RUNTIME_ERROR("malformed cond branch, list expected", env);
    if (branch->pair.left->kind == SYXV_KIND_SYMBOL && strcmp(branch->pair.left->symbol.name, "else") == 0) {
      return syx_eval_forms_list(env, branch->pair.right);
    }
    if (result) rc_release(result);
    result = rc_acquire(syx_eval(env, branch->pair.left));
    bool cond = syx_convert_to_bool_v(env, result);
    if (!cond) continue;
    SyxV *right = branch->pair.right;
    if (right->kind == SYXV_KIND_NIL) return rc_move(result);
    if (right->pair.left->kind == SYXV_KIND_SYMBOL && strcmp(right->pair.left->symbol.name, "=>") == 0) {
      SyxV *apply_right = right->pair.right;
      if (apply_right->kind == SYXV_KIND_PAIR && apply_right->pair.right->kind == SYXV_KIND_NIL) {
        SyxV *call = rc_acquire(make_syxv_list(apply_right->pair.left, result, NULL));
        SyxV *call_result = rc_acquire(syx_eval(env, call));
        rc_release(call);
        rc_release(result);
        return rc_move(call_result);
      }
    }
    rc_release(result);
    return syx_eval_forms_list(env, right);
  }
  if (result == NULL) RUNTIME_ERROR("cond empty branches list", env);
  return result;
}

void syx_env_define_special_forms(Syx_Env *env) {
  /** Special forms */
  syx_env_define_cstr(env, "quote", make_syxv_specialf(NULL, syx_special_form_quote));
  syx_env_define_cstr(env, "begin", make_syxv_specialf(NULL, syx_special_form_begin));
  syx_env_define_cstr(env, "lambda", make_syxv_specialf(NULL, syx_special_form_lambda));
  syx_env_define_cstr(env, "define", make_syxv_specialf(NULL, syx_special_form_define));
  syx_env_define_cstr(env, "set", make_syxv_specialf(NULL, syx_special_form_set));
  syx_env_define_cstr(env, "let", make_syxv_specialf(NULL, syx_special_form_let));
  syx_env_define_cstr(env, "and", make_syxv_specialf(NULL, syx_special_form_and));
  syx_env_define_cstr(env, "or", make_syxv_specialf(NULL, syx_special_form_or));
  syx_env_define_cstr(env, "if", make_syxv_specialf(NULL, syx_special_form_if));
  syx_env_define_cstr(env, "cond", make_syxv_specialf(NULL, syx_special_form_cond));
}

#endif // SYX_EVAL_SPECIALF_IMPL
