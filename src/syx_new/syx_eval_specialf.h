#ifndef SYX_EVAL_SPECIALF_H
#define SYX_EVAL_SPECIALF_H

#include <syx_new/syx_eval.h>
#include <syx_new/syx_object.h>

void syx_env_define_special_forms(Syx_Env *env);

#endif // SYX_EVAL_SPECIALF_H

#if defined(SYX_EVAL_SPECIALF_IMPL) && !defined(SYX_EVAL_SPECIALF_IMPL_C)
#define SYX_EVAL_SPECIALF_IMPL_C

/** Special forms */

/** Evaluates forms in order and returns last result. */
Syx_Value *syx_special_form_begin(Syx_Eval_Ctx *ctx, Syx_Pair *arguments) {
  return syx_eval_forms_list(ctx, arguments);
}

Syx_Value *syx__special_form_make_lambda(Syx_Eval_Ctx *ctx, Syx_Symbol *name, Syx_Pair *defines, Syx_Pair *forms) {
  Syx_Value *rest_define = NULL;
  syx_list_for_each(defines, define, &rest_define) {
    if (define->kind == SYX_VALUE_KIND_SYMBOL) continue;
    if (define->kind != SYX_VALUE_KIND_PAIR || !define->pair) SYX_EVAL_THROW(ctx, "malformed lambda arguments definitions list");
    Syx_Value *name = define->pair->left;
    Syx_Value *default_value = define->pair->right;
    if (name->kind != SYX_VALUE_KIND_SYMBOL) SYX_EVAL_THROW(ctx, "malformed lambda arguments definitions list");
    if (default_value->kind != SYX_VALUE_KIND_PAIR) SYX_EVAL_THROW(ctx, "malformed lambda arguments definitions list");
  }
  if (rest_define->kind != SYX_VALUE_KIND_PAIR) {
    if (rest_define->kind != SYX_VALUE_KIND_SYMBOL) SYX_EVAL_THROW(ctx, "malformed lambda rest argument");
  }
  return make_syx_value_closure_lambda(name, (Syx_Closure_Lambda){.env = ctx->env, .defines = defines, .forms = forms});
}

/** Creates a closure that captures current environment. */
Syx_Value *syx_special_form_lambda(Syx_Eval_Ctx *ctx, Syx_Pair *arguments) {
  Syx_Value *first = syx_list_next(&arguments);
  if (first->kind == SYX_VALUE_KIND_SYMBOL) {
    Syx_Value *defines = syx_list_next(&arguments);
    SYX_EVAL_ASSERT(ctx, defines->kind == SYX_VALUE_KIND_PAIR, "malformed lambda rest argument");
    return syx__special_form_make_lambda(ctx, first->symbol, defines->pair, arguments);
  } else if (first->kind == SYX_VALUE_KIND_PAIR) {
    return syx__special_form_make_lambda(ctx, NULL, first->pair, arguments);
  } else {
    SYX_EVAL_THROW(ctx, "malformed lambda rest argument");
  }
}

/** Binds a name in the current environment. */
Syx_Value *syx_special_form_define(Syx_Eval_Ctx *ctx, Syx_Pair *arguments) {
  Syx_Value *name_s = syx_list_next(&arguments);
  Syx_Value *value;
  if (name_s->kind == SYX_VALUE_KIND_PAIR) {
    if (!name_s->pair) SYX_EVAL_THROW(ctx, "malformed lambda definition");
    Syx_Value *defines = name_s->pair->right;
    name_s = name_s->pair->left;
    if (name_s->kind != SYX_VALUE_KIND_SYMBOL) SYX_EVAL_THROW(ctx, "symbol expression expected as lambda name");
    if (defines->kind != SYX_VALUE_KIND_PAIR) SYX_EVAL_THROW(ctx, "malformed lambda rest argument");
    value = rc_acquire(syx__special_form_make_lambda(ctx, name_s->symbol, defines->pair, arguments));
  } else if (name_s->kind != SYX_VALUE_KIND_SYMBOL) {
    SYX_EVAL_THROW(ctx, "Symbol expression expected as name");
  } else {
    value = rc_acquire(syx_eval(ctx, syx_list_next(&arguments)));
    syx_value_early_exit(value);
  }
  syx_env_define(ctx->env, name_s->symbol, rc_move(value));
  return syx_value_nil();
}

/** Mutate an existing binding or creates new one in current environment. */
Syx_Value *syx_special_form_set(Syx_Eval_Ctx *ctx, Syx_Pair *arguments) {
  Syx_Value *target = rc_acquire(syx_eval_unquote(ctx, syx_list_next(&arguments)));
  syx_value_early_exit(target);
  Syx_Value *value = rc_acquire(syx_eval(ctx, syx_list_next(&arguments)));
  syx_value_early_exit(value, (target));
  // if (target->lvalue) {
  //   Syx_Value *result = target->lvalue->callback(ctx, target, target->lvalue->data, value);
  //   if (!result) result = syx_value_nil();
  //   rc_acquire(result);
  //   rc_release(target);
  //   rc_release(value);
  //   return rc_move(result);
  // } else if (target->kind == SYXV_KIND_BOXED) {
  //   Syx_Value *result = syx_boxed_set(ctx, target->boxed, value);
  //   if (!result) result = syx_value_nil();
  //   rc_acquire(result);
  //   rc_release(target);
  //   rc_release(value);
  //   return rc_move(result);
  // } else
  if (target->kind == SYX_VALUE_KIND_SYMBOL) {
    syx_env_set(ctx->env, target->symbol, rc_move(value));
    rc_release(target);
    return syx_value_nil();
  }
  SYX_EVAL_THROW(ctx, "unsupported set expression", (), (target));
}

/** Checks if environment has binding. */
Syx_Value *syx_special_form_is_set(Syx_Eval_Ctx *ctx, Syx_Pair *arguments) {
  Syx_Value *name_s = syx_list_next(&arguments);
  if (name_s->kind != SYX_VALUE_KIND_SYMBOL) SYX_EVAL_THROW(ctx, "Symbol expression expected as name");
  Syx_Value *stored = syx_env_lookup_get(ctx, name_s->symbol);
  return syx_value_bool(stored != NULL);
}

/** Get an existing binding. */
Syx_Value *syx_special_form_get(Syx_Eval_Ctx *ctx, Syx_Pair *arguments) {
  Syx_Value *name_s = syx_list_next(&arguments);
  if (name_s->kind != SYX_VALUE_KIND_SYMBOL) SYX_EVAL_THROW(ctx, "Symbol expression expected as name");
  return syx_env_lookup_get(ctx, name_s->symbol);
}

/** Unset value in current environment. */
Syx_Value *syx_special_form_unset(Syx_Eval_Ctx *ctx, Syx_Pair *arguments) {
  Syx_Value *name_s = syx_list_next(&arguments);
  if (name_s->kind != SYX_VALUE_KIND_SYMBOL) SYX_EVAL_THROW(ctx, "Symbol expression expected as name");
  Syx_Env *env = syx_env_lookup(ctx->env, name_s->symbol);
  if (!env) return NULL;
  Syx_Value **storage = ht_find(&env->symbols, name_s->symbol);
  if (!storage) return NULL;
  Syx_Value *value = *storage;
  ht_delete(&env->symbols, storage);
  rc_release(value);
  return NULL;
}

/** Create new variable bindings in parallel on new environment and execute a series of forms in that environment. */
Syx_Value *syx_special_form_let(Syx_Eval_Ctx *ctx, Syx_Pair *arguments) {
  Syx_Env *body_env = make_syx_env(make_syx_value_symbol_strlit("let")->symbol, ctx->env);
  Syx_Eval_Ctx *body_ctx = rc_acquire(inherit_syx_eval_ctx(ctx, (Syx_Eval_Ctx){.env = body_env}));
  Syx_Value *bindings_src = syx_list_next(&arguments);
  if (bindings_src->kind != SYX_VALUE_KIND_PAIR) SYX_EVAL_THROW(ctx, "List of definitions expected", (), (body_ctx));
  syx_list_for_each(bindings_src->pair, binding) {
    if (binding->kind != SYX_VALUE_KIND_PAIR || !binding->pair) SYX_EVAL_THROW(ctx, "malformed let definition, list expected", (), (body_ctx));
    Syx_Value *name = binding->pair->left;
    if (name->kind != SYX_VALUE_KIND_SYMBOL) SYX_EVAL_THROW(ctx, "malformed let definition, symbol as name expected", (), (body_ctx));
    if (binding->pair->right != SYX_VALUE_KIND_PAIR || !binding->pair->right->pair) SYX_EVAL_THROW(ctx, "malformed let definition, list expected", (), (body_ctx));
    Syx_Value *value = rc_acquire(syx_eval(ctx, binding->pair->right->pair->left));
    syx_value_early_exit(value, (body_ctx));
    syx_env_define(body_ctx->env, name->symbol, rc_move(value));
  }
  Syx_Value *result = rc_acquire(syx_eval_forms_list(body_ctx, arguments));
  rc_release(body_ctx);
  return rc_move(result);
}

Syx_Value *syx_special_form_and_reduce(Syx_Eval_Ctx *ctx, Syx_Value *evaluated) {
  bool value = {0};
  syx_convert_to(ctx, evaluated, &value);
  return syx_value_bool(!value);
}

/** Evaluates left to right, returns first falsy value or last value if all truthy */
Syx_Value *syx_special_form_and(Syx_Eval_Ctx *ctx, Syx_Pair *arguments) {
  return syx_eval_forms_list(ctx, arguments, .should_stop = syx_special_form_and_reduce, .initial = syx_value_nil());
}

Syx_Value *syx_special_form_or_reduce(Syx_Eval_Ctx *ctx, Syx_Value *evaluated) {
  UNUSED(ctx);
  return evaluated;
}

/** Evaluates left to right, returns first truthy value or last value if all falsy */
Syx_Value *syx_special_form_or(Syx_Eval_Ctx *ctx, Syx_Pair *arguments) {
  return syx_eval_forms_list(ctx, arguments, .should_stop = syx_special_form_or_reduce, .initial = syx_value_nil());
}

/** if - Evaluates condition then evaluates only one branch */
Syx_Value *syx_special_form_if(Syx_Eval_Ctx *ctx, Syx_Pair *arguments) {
  bool cond = {0};
  syx_convert_to(ctx, syx_eval(ctx, syx_list_next(&arguments)), &cond);
  Syx_Value *then_body = syx_list_next(&arguments);
  Syx_Value *else_body = syx_list_next(&arguments);
  Syx_Value *result;
  if (cond) result = syx_eval(ctx, then_body);
  else result = syx_eval(ctx, else_body);
  return result;
}

/** Multi-branch conditional */
Syx_Value *syx_special_form_cond(Syx_Eval_Ctx *ctx, Syx_Pair *arguments) {
  Syx_Value *result = NULL;
  Syx_Value *else_symbol = rc_acquire(make_syx_value_symbol_strlit("else"));
  Syx_Value *apply_symbol = rc_acquire(make_syx_value_symbol_strlit("=>"));
  syx_list_for_each(arguments, branch) {
    if (branch->kind != SYX_VALUE_KIND_PAIR || !branch->pair) SYX_EVAL_THROW(ctx, "malformed cond branch, list expected");
    if (branch->pair->left == else_symbol) {
      Syx_Value *forms = branch->pair->right;
      if (forms->kind != SYX_VALUE_KIND_PAIR) SYX_EVAL_THROW(ctx, "malformed else branch, list expected");
      result = syx_eval_forms_list(ctx, forms->pair);
      rc_release_all(else_symbol, apply_symbol);
      return result;
    }
    if (result) rc_release(result);
    result = rc_acquire(syx_eval(ctx, branch->pair->left));
    bool cond = {0};
    syx_convert_to(ctx, result, &cond, (result, else_symbol, apply_symbol));
    if (!cond) continue;
    branch = branch->pair->right;
    if (branch->kind != SYX_VALUE_KIND_PAIR) SYX_EVAL_THROW(ctx, "malformed cond branch, forms list expected");
    if (!branch->pair) {
      rc_release_all(else_symbol, apply_symbol);
      return rc_move(result);
    }
    if (branch->pair->left == apply_symbol) {
      branch = branch->pair->right;
      if (branch->kind != SYX_VALUE_KIND_PAIR || !branch->pair) SYX_EVAL_THROW(ctx, "malformed cond apply branch, apply function expected");
      Syx_Value *fn = branch->pair->left;
      Syx_Value *call = rc_acquire(make_syx_value_list(fn, result, NULL));
      Syx_Value *call_result = rc_acquire(syx_eval(ctx, call));
      syx_value_early_exit(call_result, (call, result, else_symbol, apply_symbol));
      rc_release_all(call, result, else_symbol, apply_symbol);
      return rc_move(call_result);
    }
    rc_release_all(result, else_symbol, apply_symbol);
    return syx_eval_forms_list(ctx, branch->pair);
  }
  if (result == NULL) SYX_EVAL_THROW(ctx, "cond empty branches list");
  rc_release_all(else_symbol, apply_symbol);
  return result;
}

/** Create thrown value. */
Syx_Value *syx_special_form_throw(Syx_Eval_Ctx *ctx, Syx_Pair *arguments) {
  Syx_Value *reason = rc_acquire(syx_eval(ctx, syx_list_next(&arguments)));
  syx_value_early_exit(reason);
  return make_syx_value_exit_thrown(rc_move(reason), ctx->frames_stack->latest);
}

/** Special form for intercepting thrown values and ensuring cleanup logic is executed. */
Syx_Value *syx_special_form_try(Syx_Eval_Ctx *ctx, Syx_Pair *arguments) {
  // `(try <body> ...[(catch (<symbol>)? ...<forms>)|(catch => <fn>)|(finally ...<forms>)])`
  Syx_Value *body = rc_acquire(syx_eval(ctx, syx_list_next(&arguments)));
  Syx_Value *catch_symbol = rc_acquire(make_syx_value_symbol_strlit("catch"));
  Syx_Value *apply_symbol = rc_acquire(make_syx_value_symbol_strlit("=>"));
  Syx_Value *finally_symbol = rc_acquire(make_syx_value_symbol_strlit("finally"));
  Syx_Value *result = NULL;
  syx_list_for_each(arguments, branch) {
    if (branch->kind != SYX_VALUE_KIND_PAIR || !branch->pair) {
      SYX_EVAL_THROW(ctx, "malformed try handlers, list expected", (), (body, catch_symbol, apply_symbol, finally_symbol, result));
    }
    if (branch->pair->left == catch_symbol) {
      if (body->kind != SYX_VALUE_KIND_EXIT || body->exit->kind != SYX_EXIT_KIND_THROWN) continue;
      branch = branch->pair->right;
      if (branch->kind != SYX_VALUE_KIND_PAIR) {
        SYX_EVAL_THROW(ctx, "malformed try's catch, list expected", (), (body, catch_symbol, apply_symbol, finally_symbol, result));
      }
      if (!branch->pair) {
        if (result) rc_release(result);
        result = rc_acquire(syx_value_nil());
        continue;
      }
      if (branch->pair->left == apply_symbol) {
        branch = branch->pair->right;
        if (branch->kind != SYX_VALUE_KIND_PAIR || !branch->pair) {
          SYX_EVAL_THROW(ctx, "malformed try's apply catch, apply function expected", (), (body, catch_symbol, apply_symbol, finally_symbol, result));
        }
        Syx_Value *fn = branch->pair->left;
        Syx_Value *call = rc_acquire(make_syx_value_list(fn, result, NULL));
        Syx_Value *call_result = rc_acquire(syx_eval(ctx, call));
        syx_value_early_exit(call_result, (body, catch_symbol, apply_symbol, finally_symbol, result, call));
        rc_release_all(body, catch_symbol, apply_symbol, finally_symbol, result, call);
        result = call_result;
        continue;
      }
      Syx_Value *error_name = NULL;
      if (branch->pair->left->kind == SYX_VALUE_KIND_PAIR && branch->pair->left->pair) {
        error_name = branch->pair->left->pair->left;
        branch = branch->pair->right;
        if (error_name->kind != SYX_VALUE_KIND_SYMBOL) {
          SYX_EVAL_THROW(ctx, "malformed try's catch handler, error name symbol expected", (), (body, catch_symbol, apply_symbol, finally_symbol, result));
        }
      }
      if (branch->kind != SYX_VALUE_KIND_PAIR) {
        SYX_EVAL_THROW(ctx, "malformed try's catch handler, list expected", (), (body, catch_symbol, apply_symbol, finally_symbol, result));
      }
      if (!error_name) {
        if (result) rc_release(result);
        result = rc_acquire(syx_eval_forms_list(ctx, branch->pair, .initial = syx_value_nil()));
        syx_value_early_exit(result, (body, catch_symbol, apply_symbol, finally_symbol));
        continue;
      }
      Syx_Env *handler_env = make_syx_env(make_syx_value_symbol_strlit("try-catch")->symbol, ctx->env);
      Syx_Eval_Ctx *handler_ctx = inherit_syx_eval_ctx(ctx, (Syx_Eval_Ctx){.env = handler_env});
      syx_env_define(handler_ctx->env, error_name->symbol, body->exit->thrown->reason);
      if (result) rc_release(result);
      result = rc_acquire(syx_eval_forms_list(handler_ctx, branch->pair, .initial = syx_value_nil()));
      syx_value_early_exit(result, (body, catch_symbol, apply_symbol, finally_symbol, handler_ctx));
      rc_release(handler_ctx);
      continue;
    }
    if (branch->pair->left == finally_symbol) {
      branch = branch->pair->right;
      if (branch->kind != SYX_VALUE_KIND_PAIR) {
        SYX_EVAL_THROW(ctx, "malformed try's finally handler, list expected", (), (body, catch_symbol, apply_symbol, finally_symbol));
      }
      Syx_Value *finally_result = rc_acquire(syx_eval_forms_list(ctx, branch->pair));
      syx_value_early_exit(finally_result, (body, catch_symbol, apply_symbol, finally_symbol, result));
      rc_release(finally_result);
      continue;
    }
    SYX_EVAL_THROW(ctx, "malformed try's handlers list", (), (body, catch_symbol, apply_symbol, finally_symbol));
  }
  rc_release_all(catch_symbol, apply_symbol, finally_symbol);
  if (!result) return rc_move(body);
  rc_release(body);
  return rc_move(result);
}

/** Special form to trigger an immediate exit from the current function, carrying a value. */
Syx_Value *syx_special_form_return(Syx_Eval_Ctx *ctx, Syx_Pair *arguments) {
  Syx_Value *value = rc_acquire(syx_eval(ctx, syx_list_next(&arguments)));
  syx_value_early_exit(value);
  return make_syx_value_exit_returned(rc_move(value));
}

/** Creates object with of named fields. */
Syx_Value *syx_special_form_object(Syx_Eval_Ctx *ctx, Syx_Pair *arguments) {
  Syx_Value *value = rc_acquire(make_syx_value_object(NULL));
  Syx_Value *proto_symbol = rc_acquire(make_syx_value_symbol_strlit("proto"));
  while (arguments) {
    Syx_Value *field_name = syx_list_next_nullable(&arguments);
    if (!arguments) SYX_EVAL_THROW(ctx, "malformed object key-value pair", (), (value, proto_symbol));
    if (field_name->kind != SYX_VALUE_KIND_PREFIXED) SYX_EVAL_THROW(ctx, "expected field name prefixed with ':'", (), (value, proto_symbol));
    if (field_name->prefixed->kind != SYX_PREFIXED_KIND_COLON) SYX_EVAL_THROW(ctx, "expected field name prefixed with ':'", (), (value, proto_symbol));
    field_name = field_name->prefixed->value;
    if (field_name == proto_symbol) {
      Syx_Value *proto_form = syx_list_next_nullable(&arguments);
      Syx_Value *proto = rc_acquire(syx_eval(ctx, proto_form));
      syx_value_early_exit(proto, (value, proto_symbol));
      if (proto->kind != SYX_VALUE_KIND_OBJECT) SYX_EVAL_THROW(ctx, "expected object as prototype", (), (value, proto_symbol, proto));
      rc_release(syx_value_from_object(value->object->proto));
      value->object->proto = proto->object;
      continue;
    }
    if (field_name->kind != SYX_VALUE_KIND_SYMBOL) SYX_EVAL_THROW(ctx, "expected field name prefixed with ':'", (), (value, proto_symbol));
    Syx_Value *form = rc_acquire(syx_list_next_nullable(&arguments));
    if (form->kind == SYX_VALUE_KIND_PREFIXED && form->prefixed->kind == SYX_PREFIXED_KIND_UNQUOTE) {
      Syx_Value *result = rc_acquire(syx_eval(ctx, form->prefixed->value));
      syx_value_early_exit(result, (value, proto_symbol, form));
      rc_release(form);
      form = result;
    }
    syx_object_set(value->object, field_name->symbol, rc_move(form));
  }
  rc_release(proto_symbol);
  return rc_move(value);
}

void syx_env_define_special_forms(Syx_Env *env) {
  /** Special forms */
  syx_env_define_strlit(env, "begin", make_syx_value_closure_specialf(NULL, syx_special_form_begin));
  syx_env_define_strlit(env, "lambda", make_syx_value_closure_specialf(NULL, syx_special_form_lambda));

  syx_env_define_strlit(env, "define", make_syx_value_closure_specialf(NULL, syx_special_form_define));
  syx_env_define_strlit(env, "set", make_syx_value_closure_specialf(NULL, syx_special_form_set));
  syx_env_define_strlit(env, "is-set?", make_syx_value_closure_specialf(NULL, syx_special_form_is_set));
  syx_env_define_strlit(env, "get", make_syx_value_closure_specialf(NULL, syx_special_form_get));
  syx_env_define_strlit(env, "unset", make_syx_value_closure_specialf(NULL, syx_special_form_unset));
  syx_env_define_strlit(env, "let", make_syx_value_closure_specialf(NULL, syx_special_form_let));

  syx_env_define_strlit(env, "and", make_syx_value_closure_specialf(NULL, syx_special_form_and));
  syx_env_define_strlit(env, "or", make_syx_value_closure_specialf(NULL, syx_special_form_or));

  syx_env_define_strlit(env, "if", make_syx_value_closure_specialf(NULL, syx_special_form_if));
  syx_env_define_strlit(env, "cond", make_syx_value_closure_specialf(NULL, syx_special_form_cond));

  syx_env_define_strlit(env, "throw", make_syx_value_closure_specialf(NULL, syx_special_form_throw));
  syx_env_define_strlit(env, "try", make_syx_value_closure_specialf(NULL, syx_special_form_try));

  syx_env_define_strlit(env, "return", make_syx_value_closure_specialf(NULL, syx_special_form_return));

  syx_env_define_strlit(env, "object", make_syx_value_closure_specialf(NULL, syx_special_form_object));
}

#endif // SYX_EVAL_SPECIALF_IMPL
