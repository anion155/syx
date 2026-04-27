#ifndef SYX_EVAL_SPECIALF_H
#define SYX_EVAL_SPECIALF_H

#include "syx_eval.h"

void syx_env_put_special_forms(Syx_Env *env);

#endif // SYX_EVAL_SPECIALF_H

#if defined(SYX_EVAL_SPECIALF_IMPL) && !defined(SYX_EVAL_SPECIALF_IMPL_C)
#define SYX_EVAL_SPECIALF_IMPL_C

/** Special forms */
/** quote - Returns argument unevaluated */
SyxV *syx_special_form_quote(Syx_Env *env, Syx_Arguments *arguments) {
  SYX_EVAL_ARGUMENTS_CLAMP(env, 1);
  return arguments->items[0];
}
/** if - Evaluates condition then evaluates only one branch */
SyxV *syx_special_form_if(Syx_Env *env, Syx_Arguments *arguments) {
  SYX_EVAL_ARGUMENTS_CLAMP(env, 2, 3);
  SyxV *cond_value = syx_eval(env, arguments->items[0]);
  SyxV *then_value = arguments->items[1];
  SyxV *else_value = arguments->items[2];
  // TODO convert to bool
  if (cond_value->kind != SYXV_KIND_BOOL) UNREACHABLE("illegal if condition value");
  return cond_value->boolean ? syx_eval(env, then_value) : syx_eval(env, else_value);
}
/** lambda - Creates a closure and captures current environment */
SyxV *syx_special_form_lambda(Syx_Env *env, Syx_Arguments *arguments) {
  TODO("syx_special_form_lambda"); UNUSED(env); UNUSED(arguments);
  // Expr_Value arguments_val = arguments.items[0];
  // if (arguments_val.kind != EXPR_VALUE_KIND_EXPR) UNREACHABLE("Arguments expected to be a list");
  // Expr_Value body_val = arguments.items[1];
  // if (body_val.kind != EXPR_VALUE_KIND_EXPR) UNREACHABLE("Body expected to be a body");
  // return (Expr_Value){
  //   .kind = EXPR_VALUE_KIND_CLOSURE,
  //   .closure = {.env = env, .arguments = arguments_val.expr, .body = body_val.expr},
  // };
}
/** define - Binds a name in the current environment */
SyxV *syx_special_form_define(Syx_Env *env, Syx_Arguments *arguments) {
  TODO("syx_special_form_define"); UNUSED(env); UNUSED(arguments);
  // if (arguments.count != 2) UNREACHABLE("Incorrect amount of arguments");
  // Expr_Value name_val = arguments.items[0];
  // if (name_val.kind != EXPR_VALUE_KIND_EXPR) UNREACHABLE("Name expected to be a symbol");
  // Expr_Value value_val = arguments.items[1];
  // if (name_val.expr->kind == EXPR_KIND_PAIR) {
  //   Expr *arguments_expr = name_val.expr->pair.right;
  //   if (arguments_expr->kind != EXPR_KIND_PAIR) UNREACHABLE("Arguments expected to be a list");
  //   name_val.expr = name_val.expr->pair.left;
  //   if (value_val.kind != EXPR_VALUE_KIND_EXPR) UNREACHABLE("Body expected to be a list");
  //   value_val = expr_special_form_lambda(env, expr_arguments(make_expr_list(
  //     arguments_expr,
  //     value_val.expr,
  //     NULL
  //   )));
  // } else {
  //   value_val = expr_eval(env, value_val);
  // }
  // if (name_val.expr->kind != EXPR_KIND_SYMBOL) UNREACHABLE("Symbol expression expected as name");
  // expr_env_put_symbol(env, name_val.expr->symbol.name, value_val);
  // return (Expr_Value){.kind = EXPR_VALUE_KIND_EXPR, .expr = &EXPR_NIL};
}
/** Mutates an existing binding */
SyxV *syx_special_form_set_excl(Syx_Env *env, Syx_Arguments *arguments) {
  TODO("syx_special_form_set_excl"); UNUSED(env); UNUSED(arguments);
}
/** Evaluates expressions in order and returns last */
SyxV *syx_special_form_begin(Syx_Env *env, Syx_Arguments *arguments) {
  TODO("syx_special_form_begin"); UNUSED(env); UNUSED(arguments);
  // da_foreach(Expr_Value, argument, &arguments) {
  //   if (argument->kind != EXPR_VALUE_KIND_EXPR) UNREACHABLE("Every begin item supposed to be expressions");
  //   *argument = expr_eval(env, *argument);
  // }
  // return arguments.items[arguments.count - 1];
}
/** Binds names in a new environment with all RHS evaluated in current env */
SyxV *syx_special_form_let(Syx_Env *env, Syx_Arguments *arguments) {
  TODO("syx_special_form_let"); UNUSED(env); UNUSED(arguments);
}
/** Like let but each binding sees previous ones */
SyxV *syx_special_form_let_star(Syx_Env *env, Syx_Arguments *arguments) {
  TODO("syx_special_form_let_star"); UNUSED(env); UNUSED(arguments);
}
/** Like let but bindings can refer to each other for mutual recursion */
SyxV *syx_special_form_letrec(Syx_Env *env, Syx_Arguments *arguments) {
  TODO("syx_special_form_letrec"); UNUSED(env); UNUSED(arguments);
}
/** Evaluates left to right and stops at first falsy and returns last */
SyxV *syx_special_form_and(Syx_Env *env, Syx_Arguments *arguments) {
  TODO("syx_special_form_and"); UNUSED(env); UNUSED(arguments);
}
/** Evaluates left to right and stops at first truthy and returns it */
SyxV *syx_special_form_or(Syx_Env *env, Syx_Arguments *arguments) {
  TODO("syx_special_form_or"); UNUSED(env); UNUSED(arguments);
}
/** Chain of (test expr) clauses that evaluates first matching branch */
SyxV *syx_special_form_cond(Syx_Env *env, Syx_Arguments *arguments) {
  TODO("syx_special_form_cond"); UNUSED(env); UNUSED(arguments);
}
/** If condition is true evaluates body else returns nil */
SyxV *syx_special_form_when(Syx_Env *env, Syx_Arguments *arguments) {
  TODO("syx_special_form_when"); UNUSED(env); UNUSED(arguments);
}
/** If condition is false evaluates body else returns nil */
SyxV *syx_special_form_unless(Syx_Env *env, Syx_Arguments *arguments) {
  TODO("syx_special_form_unless"); UNUSED(env); UNUSED(arguments);
}
/** Iterative loop with step expressions and exit condition */
SyxV *syx_special_form_do(Syx_Env *env, Syx_Arguments *arguments) {
  TODO("syx_special_form_do"); UNUSED(env); UNUSED(arguments);
}
/** Wraps expression in a promise without evaluating it */
SyxV *syx_special_form_delay(Syx_Env *env, Syx_Arguments *arguments) {
  TODO("syx_special_form_delay"); UNUSED(env); UNUSED(arguments);
}
/** Evaluates a delayed promise and caches result */
SyxV *syx_special_form_force(Syx_Env *env, Syx_Arguments *arguments) {
  TODO("syx_special_form_force"); UNUSED(env); UNUSED(arguments);
}
/** Defines a macro transformation rule */
SyxV *syx_special_form_define_syntax(Syx_Env *env, Syx_Arguments *arguments) {
  TODO("syx_special_form_define_syntax"); UNUSED(env); UNUSED(arguments);
}
/** Locally scoped macro definitions */
SyxV *syx_special_form_let_syntax(Syx_Env *env, Syx_Arguments *arguments) {
  TODO("syx_special_form_let_syntax"); UNUSED(env); UNUSED(arguments);
}
/** Pattern-based macro expansion engine */
SyxV *syx_special_form_syntax_rules(Syx_Env *env, Syx_Arguments *arguments) {
  TODO("syx_special_form_syntax_rules"); UNUSED(env); UNUSED(arguments);
}

void syx_env_put_special_forms(Syx_Env *env) {
  /** Special forms */
  syx_env_put(env, "quote",         make_syxv_specialf("quote",         syx_special_form_quote        ));
  syx_env_put(env, "if",            make_syxv_specialf("if",            syx_special_form_if           ));
  syx_env_put(env, "lambda",        make_syxv_specialf("lambda",        syx_special_form_lambda       ));
  syx_env_put(env, "define",        make_syxv_specialf("define",        syx_special_form_define       ));
  syx_env_put(env, "set!",          make_syxv_specialf("set",           syx_special_form_set_excl     ));
  syx_env_put(env, "begin",         make_syxv_specialf("begin",         syx_special_form_begin        ));
  syx_env_put(env, "let",           make_syxv_specialf("let",           syx_special_form_let          ));
  syx_env_put(env, "let*",          make_syxv_specialf("let",           syx_special_form_let_star     ));
  syx_env_put(env, "letrec",        make_syxv_specialf("letrec",        syx_special_form_letrec       ));
  syx_env_put(env, "and",           make_syxv_specialf("and",           syx_special_form_and          ));
  syx_env_put(env, "or",            make_syxv_specialf("or",            syx_special_form_or           ));
  syx_env_put(env, "cond",          make_syxv_specialf("cond",          syx_special_form_cond         ));
  syx_env_put(env, "when",          make_syxv_specialf("when",          syx_special_form_when         ));
  syx_env_put(env, "unless",        make_syxv_specialf("unless",        syx_special_form_unless       ));
  syx_env_put(env, "do",            make_syxv_specialf("do",            syx_special_form_do           ));
  syx_env_put(env, "delay",         make_syxv_specialf("delay",         syx_special_form_delay        ));
  syx_env_put(env, "force",         make_syxv_specialf("force",         syx_special_form_force        ));
  syx_env_put(env, "define-syntax", make_syxv_specialf("define-syntax", syx_special_form_define_syntax));
  syx_env_put(env, "let-syntax",    make_syxv_specialf("let-syntax",    syx_special_form_let_syntax   ));
  syx_env_put(env, "syntax-rules",  make_syxv_specialf("syntax-rules",  syx_special_form_syntax_rules ));
}

#endif // SYX_EVAL_SPECIALF_IMPL
