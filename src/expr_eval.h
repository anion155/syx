#ifndef EXPR_EVAL_H
#define EXPR_EVAL_H

#include <nob.h>
#include <ht.h>
#include "expr_ast.h"

typedef struct Expr_Env Expr_Env;
typedef struct Expr_Value Expr_Value;
typedef struct Expr_Arguments {
  Expr_Value *items;
  size_t count;
  size_t capacity;
  Expr *expr;
} Expr_Arguments;
typedef Expr_Value (*Expr_Evaluator)(Expr_Env *env, Expr_Arguments arguments);
typedef struct Expr_Closure {
  Expr_Env *env;
  Exprs arguments;
  Expr *body;
} Expr_Closure;
typedef enum {
  EXPR_VALUE_KIND_EXPR,
  EXPR_VALUE_KIND_SPECIAL_FORM,
  EXPR_VALUE_KIND_BUILTIN,
  EXPR_VALUE_KIND_CLOSURE,
} Expr_Value_Kind;
struct Expr_Value {
  Expr_Value_Kind kind;
  union {
    Expr *expr;
    Expr_Evaluator special;
    Expr_Evaluator builtin;
    Expr_Closure closure;
  };
};
typedef Ht(const char *, Expr_Value, Expr_Env_Symbols) Expr_Env_Symbols;
struct Expr_Env {
  Expr_Env *parent;
  Expr_Env_Symbols symbols;
};

void expr_env_put_symbol(Expr_Env *env, const char *name, Expr_Value value);
void init_expr_global_env(Expr_Env *env);
Expr_Value *expr_env_lookup(Expr_Env *env, const char *name);


/** Special forms */
/** quote         - Returns argument unevaluated */
Expr_Value expr_evaluate_quote(Expr_Env *env, Expr_Arguments arguments);
/** if            - Evaluates condition then evaluates only one branch */
Expr_Value expr_evaluate_if(Expr_Env *env, Expr_Arguments arguments);
/** lambda        - Creates a closure and captures current environment */
Expr_Value expr_evaluate_lambda(Expr_Env *env, Expr_Arguments arguments);
/** define        - Binds a name in the current environment */
Expr_Value expr_evaluate_define(Expr_Env *env, Expr_Arguments arguments);
/** set!          - Mutates an existing binding */
Expr_Value expr_evaluate_set_excl(Expr_Env *env, Expr_Arguments arguments);
/** begin         - Evaluates expressions in order and returns last */
Expr_Value expr_evaluate_begin(Expr_Env *env, Expr_Arguments arguments);
/** let           - Binds names in a new environment with all RHS evaluated in current env */
Expr_Value expr_evaluate_let(Expr_Env *env, Expr_Arguments arguments);
/** let*          - Like let but each binding sees previous ones */
Expr_Value expr_evaluate_let_star(Expr_Env *env, Expr_Arguments arguments);
/** letrec        - Like let but bindings can refer to each other for mutual recursion */
Expr_Value expr_evaluate_letrec(Expr_Env *env, Expr_Arguments arguments);
/** and           - Evaluates left to right and stops at first falsy and returns last */
Expr_Value expr_evaluate_and(Expr_Env *env, Expr_Arguments arguments);
/** or            - Evaluates left to right and stops at first truthy and returns it */
Expr_Value expr_evaluate_or(Expr_Env *env, Expr_Arguments arguments);
/** cond          - Chain of (test expr) clauses that evaluates first matching branch */
Expr_Value expr_evaluate_cond(Expr_Env *env, Expr_Arguments arguments);
/** when          - If condition is true evaluates body else returns nil */
Expr_Value expr_evaluate_when(Expr_Env *env, Expr_Arguments arguments);
/** unless        - If condition is false evaluates body else returns nil */
Expr_Value expr_evaluate_unless(Expr_Env *env, Expr_Arguments arguments);
/** do            - Iterative loop with step expressions and exit condition */
Expr_Value expr_evaluate_do(Expr_Env *env, Expr_Arguments arguments);
/** delay         - Wraps expression in a promise without evaluating it */
Expr_Value expr_evaluate_delay(Expr_Env *env, Expr_Arguments arguments);
/** force         - Evaluates a delayed promise and caches result */
Expr_Value expr_evaluate_force(Expr_Env *env, Expr_Arguments arguments);
/** define-syntax - Defines a macro transformation rule */
Expr_Value expr_evaluate_define_syntax(Expr_Env *env, Expr_Arguments arguments);
/** let-syntax    - Locally scoped macro definitions */
Expr_Value expr_evaluate_let_syntax(Expr_Env *env, Expr_Arguments arguments);
/** syntax-rules  - Pattern-based macro expansion engine */
Expr_Value expr_evaluate_syntax_rules(Expr_Env *env, Expr_Arguments arguments);


void expr_eval_arguments(Expr_Env *env, Exprs *arguments);
Expr_Value expr_eval(Expr_Env *env, Expr_Value input);

#endif // EXPR_EVAL_H

#ifdef EXPR_EVAL_IMPLEMENTATION
#undef EXPR_EVAL_IMPLEMENTATION

#include "expr_utils.h"

void expr_env_put_symbol(Expr_Env *env, const char *name, Expr_Value value) {
  Expr_Value *item = expr_env_lookup(env, name);
  if (item != NULL) {
    switch (item->kind) {
      case EXPR_VALUE_KIND_EXPR: break;
      case EXPR_VALUE_KIND_SPECIAL_FORM: UNREACHABLE("trying to redefine special form");
      case EXPR_VALUE_KIND_BUILTIN: UNREACHABLE("trying to redefine builtin");
      case EXPR_VALUE_KIND_CLOSURE: break;
    }
  } else {
    item = ht_put(&env->symbols, name);
  }
  *item = value;
}
void init_expr_global_env(Expr_Env *env) {
  env->symbols.hasheq = ht_cstr_hasheq;
  expr_env_put_symbol(env, "quote",         (Expr_Value){.kind = EXPR_VALUE_KIND_SPECIAL_FORM, .special = expr_evaluate_quote        });
  expr_env_put_symbol(env, "if",            (Expr_Value){.kind = EXPR_VALUE_KIND_SPECIAL_FORM, .special = expr_evaluate_if           });
  expr_env_put_symbol(env, "lambda",        (Expr_Value){.kind = EXPR_VALUE_KIND_SPECIAL_FORM, .special = expr_evaluate_lambda       });
  expr_env_put_symbol(env, "define",        (Expr_Value){.kind = EXPR_VALUE_KIND_SPECIAL_FORM, .special = expr_evaluate_define       });
  expr_env_put_symbol(env, "set!",          (Expr_Value){.kind = EXPR_VALUE_KIND_SPECIAL_FORM, .special = expr_evaluate_set_excl     });
  expr_env_put_symbol(env, "begin",         (Expr_Value){.kind = EXPR_VALUE_KIND_SPECIAL_FORM, .special = expr_evaluate_let          });
  expr_env_put_symbol(env, "let",           (Expr_Value){.kind = EXPR_VALUE_KIND_SPECIAL_FORM, .special = expr_evaluate_begin        });
  expr_env_put_symbol(env, "let*",          (Expr_Value){.kind = EXPR_VALUE_KIND_SPECIAL_FORM, .special = expr_evaluate_let_star     });
  expr_env_put_symbol(env, "letrec",        (Expr_Value){.kind = EXPR_VALUE_KIND_SPECIAL_FORM, .special = expr_evaluate_letrec       });
  expr_env_put_symbol(env, "and",           (Expr_Value){.kind = EXPR_VALUE_KIND_SPECIAL_FORM, .special = expr_evaluate_and          });
  expr_env_put_symbol(env, "or",            (Expr_Value){.kind = EXPR_VALUE_KIND_SPECIAL_FORM, .special = expr_evaluate_or           });
  expr_env_put_symbol(env, "cond",          (Expr_Value){.kind = EXPR_VALUE_KIND_SPECIAL_FORM, .special = expr_evaluate_cond         });
  expr_env_put_symbol(env, "when",          (Expr_Value){.kind = EXPR_VALUE_KIND_SPECIAL_FORM, .special = expr_evaluate_when         });
  expr_env_put_symbol(env, "unless",        (Expr_Value){.kind = EXPR_VALUE_KIND_SPECIAL_FORM, .special = expr_evaluate_unless       });
  expr_env_put_symbol(env, "do",            (Expr_Value){.kind = EXPR_VALUE_KIND_SPECIAL_FORM, .special = expr_evaluate_do           });
  expr_env_put_symbol(env, "delay",         (Expr_Value){.kind = EXPR_VALUE_KIND_SPECIAL_FORM, .special = expr_evaluate_delay        });
  expr_env_put_symbol(env, "force",         (Expr_Value){.kind = EXPR_VALUE_KIND_SPECIAL_FORM, .special = expr_evaluate_force        });
  expr_env_put_symbol(env, "define-syntax", (Expr_Value){.kind = EXPR_VALUE_KIND_SPECIAL_FORM, .special = expr_evaluate_define_syntax});
  expr_env_put_symbol(env, "let-syntax",    (Expr_Value){.kind = EXPR_VALUE_KIND_SPECIAL_FORM, .special = expr_evaluate_let_syntax   });
  expr_env_put_symbol(env, "syntax-rules",  (Expr_Value){.kind = EXPR_VALUE_KIND_SPECIAL_FORM, .special = expr_evaluate_syntax_rules });
}
Expr_Value *expr_env_lookup(Expr_Env *env, const char *name) {
  Expr_Value *item = NULL;
  while (env != NULL && item == NULL) {
    item = ht_find(&env->symbols, name);
    env = env->parent;
  }
  return item;
}

Expr_Arguments expr_arguments(Expr *expr) {
  Exprs arguments_exprs = exprs_from_list(expr);
  arguments_exprs.count -= 1;
  if (arguments_exprs.items[arguments_exprs.count] != NULL) UNREACHABLE("malformed arguments list");
  Expr_Arguments arguments = {.expr = expr};
  da_reserve(&arguments, arguments_exprs.count);
  da_foreach(Expr *, arg_expr, &arguments_exprs) {
    da_append(&arguments, ((Expr_Value){.kind = EXPR_VALUE_KIND_EXPR, .expr = arg_expr}));
  }
  return arguments;
}
void eval_expr_arguments(Expr_Env *env, Expr_Arguments *arguments) {
  da_foreach(Expr_Value, arg, arguments) {
    *arg = expr_eval(env, (Expr_Value){.kind = EXPR_VALUE_KIND_EXPR, .expr = arg->expr});
  }
}
Expr_Value expr_eval(Expr_Env *env, Expr_Value input) {
  if (input.kind != EXPR_VALUE_KIND_EXPR) return input;
  if (input.expr->kind == EXPR_KIND_QUOTE) return (Expr_Value){.kind = EXPR_VALUE_KIND_EXPR, .expr = input.expr->quote};
  if (input.expr->kind == EXPR_KIND_SYMBOL) {
    Expr_Value *item = expr_env_lookup(env, input.expr->symbol.name);
    if (item == NULL) UNREACHABLE("unbound symbol");
    return *item;
  }
  if (input.expr->kind != EXPR_KIND_PAIR) return input;
  if (!input.expr->pair.left) return input;
  Expr_Value head = expr_eval(env, (Expr_Value){.kind = EXPR_VALUE_KIND_EXPR, .expr = input.expr->pair.left});
  switch (head.kind) {
    case EXPR_VALUE_KIND_EXPR: UNREACHABLE("is not a procedure");
    case EXPR_VALUE_KIND_SPECIAL_FORM: {
      Expr_Arguments arguments = expr_arguments(input.expr->pair.right);
      Expr_Value result = head.special(env, arguments);
      da_free(arguments);
      return result;
    }
    case EXPR_VALUE_KIND_BUILTIN: {
      Expr_Arguments arguments = expr_arguments(input.expr->pair.right);
      eval_expr_arguments(env, &arguments);
      Expr_Value result = head.special(env, arguments);
      da_free(arguments);
      return result;
    }
    case EXPR_VALUE_KIND_CLOSURE: {
      TODO("closure evaluation");
      // Expr_Arguments arguments = expr_arguments(input.expr->pair.right);
      // eval_expr_arguments(env, &arguments);
      // Expr_Value result = head.closure.evaluator(head.closure.env, arguments);
      // da_free(arguments);
      // return result;
    }
  }
}

Expr_Value expr_evaluate_quote(Expr_Env *env, Expr_Arguments arguments) {
  UNUSED(env);
  if (arguments.count != 1) UNREACHABLE("Incorrect amount of arguments");
  return arguments.items[0];
}
Expr_Value expr_evaluate_if(Expr_Env *env, Expr_Arguments arguments) {
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
Expr_Value expr_evaluate_lambda(Expr_Env *env, Expr_Arguments arguments) {
  Expr_Value arguments_val = arguments.items[0];
  if (arguments_val.kind != EXPR_VALUE_KIND_EXPR) UNREACHABLE("Arguments expected to be a list");
  Expr_Value body_val = arguments.items[1];
  if (body_val.kind != EXPR_VALUE_KIND_EXPR) UNREACHABLE("Body expected to be a body");
  return (Expr_Value){.kind = EXPR_VALUE_KIND_CLOSURE, .closure = {
    .env = env,
    .arguments = arguments_val.expr,
    .body = body_val.expr
  }};
}
Expr_Value expr_evaluate_define(Expr_Env *env, Expr_Arguments arguments) {
  if (arguments.count != 2) UNREACHABLE("Incorrect amount of arguments");
  Expr_Value name_val = arguments.items[0];
  if (name_val.kind != EXPR_VALUE_KIND_EXPR) UNREACHABLE("Name expected to be a symbol");
  Expr_Value value_val = arguments.items[1];
  if (name_val.expr->kind == EXPR_KIND_PAIR) {
    Expr *arguments_expr = name_val.expr->pair.right;
    if (arguments_expr->kind != EXPR_KIND_PAIR) UNREACHABLE("Arguments expected to be a list");
    name_val.expr = name_val.expr->pair.left;
    if (value_val.kind != EXPR_VALUE_KIND_EXPR) UNREACHABLE("Body expected to be a list");
    value_val = expr_evaluate_lambda(env, expr_arguments(make_expr_list(
      arguments_expr,
      value_val.expr,
      NULL
    )));
  }
  if (name_val.expr->kind != EXPR_KIND_SYMBOL) UNREACHABLE("Symbol expression expected as name");
  expr_env_put_symbol(env, name_val.expr->symbol.name, value_val);
  return (Expr_Value){.kind = EXPR_VALUE_KIND_EXPR, .expr = &EXPR_NIL};
}
Expr_Value expr_evaluate_set_excl(Expr_Env *env, Expr_Arguments arguments) {
  UNUSED(env); UNUSED(arguments); TODO("expr_evaluate_set_excl");
}
Expr_Value expr_evaluate_begin(Expr_Env *env, Expr_Arguments arguments) {
  UNUSED(env); UNUSED(arguments); TODO("expr_evaluate_begin");
}
Expr_Value expr_evaluate_let(Expr_Env *env, Expr_Arguments arguments) {
  UNUSED(env); UNUSED(arguments); TODO("expr_evaluate_let");
}
Expr_Value expr_evaluate_let_star(Expr_Env *env, Expr_Arguments arguments) {
  UNUSED(env); UNUSED(arguments); TODO("expr_evaluate_let_star");
}
Expr_Value expr_evaluate_letrec(Expr_Env *env, Expr_Arguments arguments) {
  UNUSED(env); UNUSED(arguments); TODO("expr_evaluate_letrec");
}
Expr_Value expr_evaluate_and(Expr_Env *env, Expr_Arguments arguments) {
  UNUSED(env); UNUSED(arguments); TODO("expr_evaluate_and");
}
Expr_Value expr_evaluate_or(Expr_Env *env, Expr_Arguments arguments) {
  UNUSED(env); UNUSED(arguments); TODO("expr_evaluate_or");
}
Expr_Value expr_evaluate_cond(Expr_Env *env, Expr_Arguments arguments) {
  UNUSED(env); UNUSED(arguments); TODO("expr_evaluate_cond");
}
Expr_Value expr_evaluate_when(Expr_Env *env, Expr_Arguments arguments) {
  UNUSED(env); UNUSED(arguments); TODO("expr_evaluate_when");
}
Expr_Value expr_evaluate_unless(Expr_Env *env, Expr_Arguments arguments) {
  UNUSED(env); UNUSED(arguments); TODO("expr_evaluate_unless");
}
Expr_Value expr_evaluate_do(Expr_Env *env, Expr_Arguments arguments) {
  UNUSED(env); UNUSED(arguments); TODO("expr_evaluate_do");
}
Expr_Value expr_evaluate_delay(Expr_Env *env, Expr_Arguments arguments) {
  UNUSED(env); UNUSED(arguments); TODO("expr_evaluate_delay");
}
Expr_Value expr_evaluate_force(Expr_Env *env, Expr_Arguments arguments) {
  UNUSED(env); UNUSED(arguments); TODO("expr_evaluate_force");
}
Expr_Value expr_evaluate_define_syntax(Expr_Env *env, Expr_Arguments arguments) {
  UNUSED(env); UNUSED(arguments); TODO("expr_evaluate_define_syntax");
}
Expr_Value expr_evaluate_let_syntax(Expr_Env *env, Expr_Arguments arguments) {
  UNUSED(env); UNUSED(arguments); TODO("expr_evaluate_let_syntax");
}
Expr_Value expr_evaluate_syntax_rules(Expr_Env *env, Expr_Arguments arguments) {
  UNUSED(env); UNUSED(arguments); TODO("expr_evaluate_syntax_rules");
}

#endif // EXPR_EVAL_IMPLEMENTATION
