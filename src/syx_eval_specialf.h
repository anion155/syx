#ifndef SYX_EVAL_SPECIALF_H
#define SYX_EVAL_SPECIALF_H

#include "syx_eval.h"

void syx_env_define_special_forms(Syx_Env *env);

#endif // SYX_EVAL_SPECIALF_H

#if defined(SYX_EVAL_SPECIALF_IMPL) && !defined(SYX_EVAL_SPECIALF_IMPL_C)
#define SYX_EVAL_SPECIALF_IMPL_C

/** Special forms */

/** Returns first argument unevaluated */
SyxV *syx_special_form_quote(Syx_Eval_Ctx *ctx, Syx_SpecialF *callable, SyxV *arguments) {
  UNUSED(ctx);
  UNUSED(callable);
  return syxv_list_next(&arguments);
}

/** Evaluates forms in order and returns last result. */
SyxV *syx_special_form_begin(Syx_Eval_Ctx *ctx, Syx_SpecialF *callable, SyxV *arguments) {
  UNUSED(callable);
  return syx_eval_forms_list(ctx, arguments);
}

SyxV *syx__special_form_make_lambda(Syx_Eval_Ctx *ctx, const char *name, SyxV *defines, SyxV *forms) {
  SyxV **rest_define = NULL;
  syxv_list_for_each(define, defines, &rest_define) {
    if (define->kind == SYXV_KIND_SYMBOL) continue;
    if (define->kind != SYXV_KIND_PAIR) RUNTIME_ERROR("malformed lambda arguments definitions list", ctx);
    if (define->pair.left->kind != SYXV_KIND_SYMBOL) RUNTIME_ERROR("malformed lambda arguments definitions list", ctx);
    if (define->pair.right->kind == SYXV_KIND_PAIR) {
      if (define->pair.right->pair.right->kind != SYXV_KIND_NIL) RUNTIME_ERROR("malformed lambda arguments definitions list", ctx);
    }
  }
  if ((*rest_define)->kind != SYXV_KIND_NIL) {
    if ((*rest_define)->kind != SYXV_KIND_SYMBOL) RUNTIME_ERROR("malformed lambda rest argument", ctx);
  }
  if (forms->kind != SYXV_KIND_PAIR) RUNTIME_ERROR("malformed lambda body", ctx);
  return make_syxv_closure(name, defines, forms, ctx->env);
}

/** Creates a closure that captures current environment. */
SyxV *syx_special_form_lambda(Syx_Eval_Ctx *ctx, Syx_SpecialF *callable, SyxV *arguments) {
  UNUSED(callable);
  SyxV *first = syxv_list_next(&arguments);
  if (first->kind == SYXV_KIND_SYMBOL) {
    return syx__special_form_make_lambda(ctx, first->symbol.name, syxv_list_next(&arguments), arguments);
  } else {
    return syx__special_form_make_lambda(ctx, NULL, first, arguments);
  }
}

/** Binds a name in the current environment. */
SyxV *syx_special_form_define(Syx_Eval_Ctx *ctx, Syx_SpecialF *callable, SyxV *arguments) {
  UNUSED(callable);
  SyxV *name_s = syxv_list_next(&arguments);
  SyxV *value;
  if (name_s->kind == SYXV_KIND_PAIR) {
    SyxV *defines = name_s->pair.right;
    name_s = name_s->pair.left;
    if (name_s->kind != SYXV_KIND_SYMBOL) RUNTIME_ERROR("Symbol expression expected as name", ctx);
    value = syx__special_form_make_lambda(ctx, name_s->symbol.name, defines, arguments);
  } else if (name_s->kind != SYXV_KIND_SYMBOL) {
    RUNTIME_ERROR("Symbol expression expected as name", ctx);
  } else {
    value = syx_eval(ctx, syxv_list_next(&arguments));
    syx_eval_early_exit(value);
  }
  syx_env_define(ctx->env, &name_s->symbol, value);
  return make_syxv_nil();
}

/** Mutate an existing binding or creates new one in current environment. */
SyxV *syx_special_form_set(Syx_Eval_Ctx *ctx, Syx_SpecialF *callable, SyxV *arguments) {
  UNUSED(callable);
  SyxV *name_s = syxv_list_next(&arguments);
  if (name_s->kind != SYXV_KIND_SYMBOL) RUNTIME_ERROR("Symbol expression expected as name", ctx);
  SyxV *value = syx_eval(ctx, syxv_list_next(&arguments));
  syx_eval_early_exit(value);
  syx_env_set(ctx->env, &name_s->symbol, value);
  return make_syxv_nil();
}

/** Create new variable bindings in parallel on new environment and execute a series of forms in that environment. */
SyxV *syx_special_form_let(Syx_Eval_Ctx *ctx, Syx_SpecialF *callable, SyxV *arguments) {
  UNUSED(callable);
  Syx_Eval_Ctx *body_ctx = rc_acquire(inherit_syx_eval_ctx(ctx, .env = make_syx_env(ctx->env, NULL)));
  body_ctx->env->description = strdup(temp_sprintf("let<%p>", body_ctx->env));
  SyxV *bindings_src = syxv_list_next(&arguments);
  if (bindings_src->kind != SYXV_KIND_PAIR) RUNTIME_ERROR("List of definitions expected", ctx);
  SyxV *bindings = NULL;
  syxv_list_map(binding, bindings_src, &bindings) {
    if ((*binding)->kind != SYXV_KIND_PAIR) RUNTIME_ERROR("malformed let definition, list expected", ctx);
    if ((*binding)->pair.left->kind != SYXV_KIND_SYMBOL) RUNTIME_ERROR("malformed let definition, symbol as name expected", ctx);
    if ((*binding)->pair.right->kind == SYXV_KIND_PAIR) {
      if ((*binding)->pair.right->pair.right->kind != SYXV_KIND_NIL) RUNTIME_ERROR("malformed let definition, too long list", ctx);
    }
    (*binding) = make_syxv_pair(
        (*binding)->pair.left,
        syx_eval(ctx, (*binding)->pair.right->pair.left));
    syx_eval_early_exit((*binding), bindings);
  }
  syxv_list_for_each(binding, bindings) {
    SyxV *name = binding->pair.left;
    SyxV *value = binding->pair.right;
    syx_env_define(body_ctx->env, &name->symbol, value);
  }
  SyxV *result = syx_eval_forms_list(body_ctx, arguments);
  rc_release(body_ctx);
  return result;
}

bool syx_special_form_and_reduce(Syx_Eval_Ctx *ctx, SyxV *evaluated) {
  return !syx_convert_to_bool_v(ctx, evaluated);
}

/** Evaluates left to right, returns first falsy value or last value if all truthy */
SyxV *syx_special_form_and(Syx_Eval_Ctx *ctx, Syx_SpecialF *callable, SyxV *arguments) {
  UNUSED(callable);
  return syx_eval_forms_list(ctx, arguments, .should_stop = syx_special_form_and_reduce, .default_result = make_syxv_nil());
}

bool syx_special_form_or_reduce(Syx_Eval_Ctx *ctx, SyxV *evaluated) {
  return syx_convert_to_bool_v(ctx, evaluated);
}

/** Evaluates left to right, returns first truthy value or last value if all falsy */
SyxV *syx_special_form_or(Syx_Eval_Ctx *ctx, Syx_SpecialF *callable, SyxV *arguments) {
  UNUSED(callable);
  return syx_eval_forms_list(ctx, arguments, .should_stop = syx_special_form_or_reduce, .default_result = make_syxv_nil());
}

/** if - Evaluates condition then evaluates only one branch */
SyxV *syx_special_form_if(Syx_Eval_Ctx *ctx, Syx_SpecialF *callable, SyxV *arguments) {
  UNUSED(callable);
  SyxV *result = syx_eval(ctx, syxv_list_next(&arguments));
  syx_eval_early_exit(result);
  bool cond = syx_convert_to_bool_v(ctx, result);
  rc_release(result);
  SyxV *then_body = syxv_list_next(&arguments);
  SyxV *else_body = syxv_list_next(&arguments);
  if (cond) result = syx_eval(ctx, then_body);
  else result = syx_eval(ctx, else_body);
  syx_eval_early_exit(result);
  return result;
}

/** Multi-branch conditional */
SyxV *syx_special_form_cond(Syx_Eval_Ctx *ctx, Syx_SpecialF *callable, SyxV *arguments) {
  UNUSED(callable);
  SyxV *result = NULL;
  SyxV *else_symbol = make_syxv_symbol_cstr("else");
  SyxV *apply_symbol = make_syxv_symbol_cstr("=>");
  syxv_list_for_each(branch, arguments) {
    if (branch->kind != SYXV_KIND_PAIR) RUNTIME_ERROR("malformed cond branch, list expected", ctx);
    if (branch->pair.left == else_symbol) return syx_eval_forms_list(ctx, branch->pair.right);
    if (result) rc_release(result);
    result = rc_acquire(syx_eval(ctx, branch->pair.left));
    syx_eval_early_exit(result, result);
    bool cond = syx_convert_to_bool_v(ctx, result);
    if (!cond) continue;
    SyxV *right = branch->pair.right;
    if (right->kind == SYXV_KIND_NIL) return rc_move(result);
    if (right->pair.left == apply_symbol) {
      SyxV *apply_right = right->pair.right;
      if (apply_right->kind == SYXV_KIND_PAIR && apply_right->pair.right->kind == SYXV_KIND_NIL) {
        SyxV *call = rc_acquire(make_syxv_list(apply_right->pair.left, result, NULL));
        SyxV *call_result = syx_eval(ctx, call);
        syx_eval_early_exit(call_result, call, result);
        rc_acquire(call_result);
        rc_release(call);
        rc_release(result);
        return rc_move(call_result);
      }
    }
    rc_release(result);
    return syx_eval_forms_list(ctx, right);
  }
  if (result == NULL) RUNTIME_ERROR("cond empty branches list", ctx);
  return result;
}

/** Create thrown value. */
SyxV *syx_special_form_throw(Syx_Eval_Ctx *ctx, Syx_SpecialF *callable, SyxV *arguments) {
  UNUSED(callable);
  SyxV *reason = syx_eval(ctx, syxv_list_next(&arguments));
  syx_eval_early_exit(reason);
  return make_syxv_throw(ctx->frame_stack->latest, reason);
}

/** Special form for intercepting thrown values and ensuring cleanup logic is executed. */
SyxV *syx_special_form_try(Syx_Eval_Ctx *ctx, Syx_SpecialF *callable, SyxV *arguments) {
  UNUSED(callable);
  SyxV *body = rc_acquire(syx_eval(ctx, syxv_list_next(&arguments)));
  SyxV *catch_symbol = make_syxv_symbol_cstr("catch");
  SyxV *finally_symbol = make_syxv_symbol_cstr("finally");
  SyxV *result = NULL;
  syxv_list_for_each(branch, arguments) {
    if (branch->kind != SYXV_KIND_PAIR) RUNTIME_ERROR("malformed try handlers, list expected", ctx);
    if (branch->pair.left == catch_symbol) {
      if (body->kind != SYXV_KIND_THROWN) continue;
      SyxV *list = branch->pair.right;
      if (list->kind == SYXV_KIND_NIL) {
        if (result) rc_release(result);
        result = rc_acquire(make_syxv_nil());
        continue;
      }
      if (list->kind != SYXV_KIND_PAIR) RUNTIME_ERROR("malformed try's catch handler, list expected", ctx);
      if (list->pair.left->kind != SYXV_KIND_SYMBOL) {
        if (result) rc_release(result);
        result = rc_acquire(syx_eval_forms_list(ctx, list));
        syx_eval_early_exit(result, body);
        continue;
      }
      SyxV *error_name = list->pair.left;
      SyxV *handler = list->pair.right;
      Syx_Eval_Ctx *handler_ctx = inherit_syx_eval_ctx(ctx, .env = make_syx_env(ctx->env, NULL));
      handler_ctx->env->description = strdup(temp_sprintf("try-catch<%p>", handler_ctx->env));
      syx_env_define(handler_ctx->env, &error_name->symbol, body->thrown.reason);
      if (result) rc_release(result);
      result = rc_acquire(syx_eval_forms_list(handler_ctx, handler, .default_result = make_syxv_nil()));
      syx_eval_early_exit(result, body);
      rc_release(handler_ctx);
      continue;
    }
    if (branch->pair.left == finally_symbol) {
      SyxV *list = branch->pair.right;
      if (list->kind != SYXV_KIND_PAIR) RUNTIME_ERROR("malformed try's finally handler, list expected", ctx);
      SyxV *finally_result = rc_acquire(syx_eval_forms_list(ctx, list));
      syx_eval_early_exit(finally_result, body, result);
      rc_release(finally_result);
      continue;
    }
    RUNTIME_ERROR("malformed try's handlers list", ctx);
  }
  if (!result) return rc_move(body);
  return rc_move(result);
}

/** Special form to trigger an immediate exit from the current function, carrying a value. */
SyxV *syx_special_form_return(Syx_Eval_Ctx *ctx, Syx_SpecialF *callable, SyxV *arguments) {
  UNUSED(callable);
  SyxV *value = syx_eval(ctx, syxv_list_next(&arguments));
  syx_eval_early_exit(value);
  return make_syxv_return_value(value);
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
  syx_env_define_cstr(env, "thrown", make_syxv_specialf(NULL, syx_special_form_throw));
  syx_env_define_cstr(env, "try", make_syxv_specialf(NULL, syx_special_form_try));
  syx_env_define_cstr(env, "return", make_syxv_specialf(NULL, syx_special_form_return));
}

#endif // SYX_EVAL_SPECIALF_IMPL
