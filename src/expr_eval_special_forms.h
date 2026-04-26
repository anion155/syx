#ifndef EXPR_EVAL_SPECIAL_FORMS_H
#define EXPR_EVAL_SPECIAL_FORMS_H

#include "expr_ast.h"
#include "expr_eval.h"

void expr_env_init_special_forms(Expr_Env *env);

#endif // EXPR_EVAL_SPECIAL_FORMS_H

#if defined(EXPR_EVAL_SPECIAL_FORMS_IMPLEMENTATION) && !defined(EXPR_EVAL_SPECIAL_FORMS_IMPLEMENTATION_C)
#define EXPR_EVAL_SPECIAL_FORMS_IMPLEMENTATION_C

/** Special forms */
/** quote - Returns argument unevaluated */
Expr_Value expr_special_form_quote(Expr_Env *env, Expr_Arguments arguments) {
  UNUSED(env);
  if (arguments.count != 1) UNREACHABLE("Incorrect amount of arguments");
  return arguments.items[0];
}
/** if - Evaluates condition then evaluates only one branch */
Expr_Value expr_special_form_if(Expr_Env *env, Expr_Arguments arguments) {
  if (arguments.count < 2) UNREACHABLE("Too few arguments");
  if (arguments.count > 3) UNREACHABLE("Too many arguments");
  Expr_Value cond_val = arguments.items[0];
  Expr_Value then_val = arguments.items[1];
  Expr_Value else_val = arguments.items[2];
  if (cond_val.kind == EXPR_VALUE_KIND_EXPR) cond_val = expr_eval(env, cond_val);
  switch (cond_val.kind) {
    case EXPR_VALUE_KIND_EXPR: break;
    case EXPR_VALUE_KIND_SPECIAL_FORM:
    case EXPR_VALUE_KIND_BUILTIN:
    case EXPR_VALUE_KIND_CLOSURE: cond_val = (Expr_Value){.kind = EXPR_VALUE_KIND_EXPR, .expr = &EXPR_TRUE}; break;
  }
  if (cond_val.kind != EXPR_VALUE_KIND_EXPR) UNREACHABLE("illegal if condition value");
  cond_val.expr = expr_convert_to_bool(cond_val.expr);
  if (cond_val.expr->kind != EXPR_KIND_BOOL) UNREACHABLE("illegal if condition expression kind");
  if (cond_val.expr->boolean) return expr_eval(env, then_val);
  return expr_eval(env, else_val);
}
/** lambda - Creates a closure and captures current environment */
Expr_Value expr_special_form_lambda(Expr_Env *env, Expr_Arguments arguments) {
  Expr_Value arguments_val = arguments.items[0];
  if (arguments_val.kind != EXPR_VALUE_KIND_EXPR) UNREACHABLE("Arguments expected to be a list");
  Expr_Value body_val = arguments.items[1];
  if (body_val.kind != EXPR_VALUE_KIND_EXPR) UNREACHABLE("Body expected to be a body");
  return (Expr_Value){
    .kind = EXPR_VALUE_KIND_CLOSURE,
    .closure = {.env = env, .arguments = arguments_val.expr, .body = body_val.expr},
  };
}
/** define - Binds a name in the current environment */
Expr_Value expr_special_form_define(Expr_Env *env, Expr_Arguments arguments) {
  if (arguments.count != 2) UNREACHABLE("Incorrect amount of arguments");
  Expr_Value name_val = arguments.items[0];
  if (name_val.kind != EXPR_VALUE_KIND_EXPR) UNREACHABLE("Name expected to be a symbol");
  Expr_Value value_val = arguments.items[1];
  if (name_val.expr->kind == EXPR_KIND_PAIR) {
    Expr *arguments_expr = name_val.expr->pair.right;
    if (arguments_expr->kind != EXPR_KIND_PAIR) UNREACHABLE("Arguments expected to be a list");
    name_val.expr = name_val.expr->pair.left;
    if (value_val.kind != EXPR_VALUE_KIND_EXPR) UNREACHABLE("Body expected to be a list");
    value_val = expr_special_form_lambda(env, expr_arguments(make_expr_list(
      arguments_expr,
      value_val.expr,
      NULL
    )));
  } else {
    value_val = expr_eval(env, value_val);
  }
  if (name_val.expr->kind != EXPR_KIND_SYMBOL) UNREACHABLE("Symbol expression expected as name");
  expr_env_put_symbol(env, name_val.expr->symbol.name, value_val);
  return (Expr_Value){.kind = EXPR_VALUE_KIND_EXPR, .expr = &EXPR_NIL};
}
/** Mutates an existing binding */
Expr_Value expr_special_form_set_excl(Expr_Env *env, Expr_Arguments arguments) {
  UNUSED(env); UNUSED(arguments); TODO("expr_evaluate_set_excl");
}
/** Evaluates expressions in order and returns last */
Expr_Value expr_special_form_begin(Expr_Env *env, Expr_Arguments arguments) {
  da_foreach(Expr_Value, argument, &arguments) {
    if (argument->kind != EXPR_VALUE_KIND_EXPR) UNREACHABLE("Every begin item supposed to be expressions");
    *argument = expr_eval(env, *argument);
  }
  return arguments.items[arguments.count - 1];
}
/** Binds names in a new environment with all RHS evaluated in current env */
Expr_Value expr_special_form_let(Expr_Env *env, Expr_Arguments arguments) {
  UNUSED(env); UNUSED(arguments); TODO("expr_special_form_let");
}
/** Like let but each binding sees previous ones */
Expr_Value expr_special_form_let_star(Expr_Env *env, Expr_Arguments arguments) {
  UNUSED(env); UNUSED(arguments); TODO("expr_special_form_let_star");
}
/** Like let but bindings can refer to each other for mutual recursion */
Expr_Value expr_special_form_letrec(Expr_Env *env, Expr_Arguments arguments) {
  UNUSED(env); UNUSED(arguments); TODO("expr_special_form_letrec");
}
/** Evaluates left to right and stops at first falsy and returns last */
Expr_Value expr_special_form_and(Expr_Env *env, Expr_Arguments arguments) {
  UNUSED(env); UNUSED(arguments); TODO("expr_special_form_and");
}
/** Evaluates left to right and stops at first truthy and returns it */
Expr_Value expr_special_form_or(Expr_Env *env, Expr_Arguments arguments) {
  UNUSED(env); UNUSED(arguments); TODO("expr_special_form_or");
}
/** Chain of (test expr) clauses that evaluates first matching branch */
Expr_Value expr_special_form_cond(Expr_Env *env, Expr_Arguments arguments) {
  UNUSED(env); UNUSED(arguments); TODO("expr_special_form_cond");
}
/** If condition is true evaluates body else returns nil */
Expr_Value expr_special_form_when(Expr_Env *env, Expr_Arguments arguments) {
  UNUSED(env); UNUSED(arguments); TODO("expr_special_form_when");
}
/** If condition is false evaluates body else returns nil */
Expr_Value expr_special_form_unless(Expr_Env *env, Expr_Arguments arguments) {
  UNUSED(env); UNUSED(arguments); TODO("expr_special_form_unless");
}
/** Iterative loop with step expressions and exit condition */
Expr_Value expr_special_form_do(Expr_Env *env, Expr_Arguments arguments) {
  UNUSED(env); UNUSED(arguments); TODO("expr_special_form_do");
}
/** Wraps expression in a promise without evaluating it */
Expr_Value expr_special_form_delay(Expr_Env *env, Expr_Arguments arguments) {
  UNUSED(env); UNUSED(arguments); TODO("expr_special_form_delay");
}
/** Evaluates a delayed promise and caches result */
Expr_Value expr_special_form_force(Expr_Env *env, Expr_Arguments arguments) {
  UNUSED(env); UNUSED(arguments); TODO("expr_special_form_force");
}
/** Defines a macro transformation rule */
Expr_Value expr_special_form_define_syntax(Expr_Env *env, Expr_Arguments arguments) {
  UNUSED(env); UNUSED(arguments); TODO("expr_special_form_define_syntax");
}
/** Locally scoped macro definitions */
Expr_Value expr_special_form_let_syntax(Expr_Env *env, Expr_Arguments arguments) {
  UNUSED(env); UNUSED(arguments); TODO("expr_special_form_let_syntax");
}
/** Pattern-based macro expansion engine */
Expr_Value expr_special_form_syntax_rules(Expr_Env *env, Expr_Arguments arguments) {
  UNUSED(env); UNUSED(arguments); TODO("expr_special_form_syntax_rules");
}

void expr_env_init_special_forms(Expr_Env *env) {
  /** Special forms */
  expr_env_put_symbol(env, "quote",         (Expr_Value){.kind = EXPR_VALUE_KIND_SPECIAL_FORM, .special = expr_special_form_quote        });
  expr_env_put_symbol(env, "if",            (Expr_Value){.kind = EXPR_VALUE_KIND_SPECIAL_FORM, .special = expr_special_form_if           });
  expr_env_put_symbol(env, "lambda",        (Expr_Value){.kind = EXPR_VALUE_KIND_SPECIAL_FORM, .special = expr_special_form_lambda       });
  expr_env_put_symbol(env, "define",        (Expr_Value){.kind = EXPR_VALUE_KIND_SPECIAL_FORM, .special = expr_special_form_define       });
  expr_env_put_symbol(env, "set!",          (Expr_Value){.kind = EXPR_VALUE_KIND_SPECIAL_FORM, .special = expr_special_form_set_excl     });
  expr_env_put_symbol(env, "begin",         (Expr_Value){.kind = EXPR_VALUE_KIND_SPECIAL_FORM, .special = expr_special_form_begin        });
  expr_env_put_symbol(env, "let",           (Expr_Value){.kind = EXPR_VALUE_KIND_SPECIAL_FORM, .special = expr_special_form_let          });
  expr_env_put_symbol(env, "let*",          (Expr_Value){.kind = EXPR_VALUE_KIND_SPECIAL_FORM, .special = expr_special_form_let_star     });
  expr_env_put_symbol(env, "letrec",        (Expr_Value){.kind = EXPR_VALUE_KIND_SPECIAL_FORM, .special = expr_special_form_letrec       });
  expr_env_put_symbol(env, "and",           (Expr_Value){.kind = EXPR_VALUE_KIND_SPECIAL_FORM, .special = expr_special_form_and          });
  expr_env_put_symbol(env, "or",            (Expr_Value){.kind = EXPR_VALUE_KIND_SPECIAL_FORM, .special = expr_special_form_or           });
  expr_env_put_symbol(env, "cond",          (Expr_Value){.kind = EXPR_VALUE_KIND_SPECIAL_FORM, .special = expr_special_form_cond         });
  expr_env_put_symbol(env, "when",          (Expr_Value){.kind = EXPR_VALUE_KIND_SPECIAL_FORM, .special = expr_special_form_when         });
  expr_env_put_symbol(env, "unless",        (Expr_Value){.kind = EXPR_VALUE_KIND_SPECIAL_FORM, .special = expr_special_form_unless       });
  expr_env_put_symbol(env, "do",            (Expr_Value){.kind = EXPR_VALUE_KIND_SPECIAL_FORM, .special = expr_special_form_do           });
  expr_env_put_symbol(env, "delay",         (Expr_Value){.kind = EXPR_VALUE_KIND_SPECIAL_FORM, .special = expr_special_form_delay        });
  expr_env_put_symbol(env, "force",         (Expr_Value){.kind = EXPR_VALUE_KIND_SPECIAL_FORM, .special = expr_special_form_force        });
  expr_env_put_symbol(env, "define-syntax", (Expr_Value){.kind = EXPR_VALUE_KIND_SPECIAL_FORM, .special = expr_special_form_define_syntax});
  expr_env_put_symbol(env, "let-syntax",    (Expr_Value){.kind = EXPR_VALUE_KIND_SPECIAL_FORM, .special = expr_special_form_let_syntax   });
  expr_env_put_symbol(env, "syntax-rules",  (Expr_Value){.kind = EXPR_VALUE_KIND_SPECIAL_FORM, .special = expr_special_form_syntax_rules });
}

#endif // EXPR_EVAL_SPECIAL_FORMS_IMPLEMENTATION
