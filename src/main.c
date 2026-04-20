#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#define NOB_IMPLEMENTATION
#include <nob.h>
#undef NOB_IMPLEMENTATION
#define HT_IMPLEMENTATION
#include <ht.h>
#undef HT_IMPLEMENTATION

#define EXPR_AST_IMPLEMENTATION
#include "expr_ast.h"
#define EXPR_PARSER_IMPLEMENTATION
#include "expr_parser.h"

typedef enum {
  EXPR_ENV_KIND_BUILTIN,
  EXPR_ENV_KIND_VARIABLE,
} Expr_Env_Kind;
typedef struct Expr_Run_Env Expr_Run_Env;
typedef Expr *(*Expr_Builtin)(Expr_List *arguments, Expr_Run_Env *env);

Expr *expr_eval(Expr *input, Expr_Run_Env *env);

Expr *expr_eval_sum_real(Expr_List *arguments, Expr_Run_Env *env, expr_real_t initial, size_t from) {
  expr_real_t real_result = initial;
  for (size_t index = from, count = arguments->count; index < count; index += 1) {
    Expr *arg = expr_eval(arguments->items[index], env);
    switch (arg->kind) {
    case EXPR_KIND_INTEGER: real_result += arg->integer; break;
    case EXPR_KIND_REAL: real_result += arg->real; break;
    default: UNREACHABLE("number arguments exprected");
    }
  }
  return make_expr_real(real_result);
}
Expr *expr_eval_sum(Expr_List *arguments, Expr_Run_Env *env) {
  if (!arguments->count) return make_expr_integer(0);
  expr_int_t integer_result = 0;
  for (size_t index = 0, count = arguments->count; index < count; index += 1) {
    Expr *arg = expr_eval(arguments->items[index], env);
    switch (arg->kind) {
    case EXPR_KIND_INTEGER: integer_result += arg->integer; break;
    case EXPR_KIND_REAL: return expr_eval_sum_real(arguments, env, (expr_real_t)integer_result + arg->real, index + 1);
    default: UNREACHABLE("number arguments exprected");
    }
  }
  return make_expr_integer(integer_result);
}

Expr *expr_eval_mul_real(Expr_List *arguments, Expr_Run_Env *env, expr_real_t initial, size_t from) {
  expr_real_t real_result = initial;
  for (size_t index = from, count = arguments->count; index < count; index += 1) {
    Expr *arg = expr_eval(arguments->items[index], env);
    switch (arg->kind) {
    case EXPR_KIND_INTEGER: real_result *= arg->integer; break;
    case EXPR_KIND_REAL: real_result *= arg->real; break;
    default: UNREACHABLE("number arguments exprected");
    }
  }
  return make_expr_real(real_result);
}
Expr *expr_eval_mul(Expr_List *arguments, Expr_Run_Env *env) {
  if (!arguments->count) return make_expr_integer(1);
  expr_int_t integer_result = 1;
  for (size_t index = 0, count = arguments->count; index < count; index += 1) {
    Expr *arg = expr_eval(arguments->items[index], env);
    switch (arg->kind) {
    case EXPR_KIND_INTEGER: integer_result *= arg->integer; break;
    case EXPR_KIND_REAL: return expr_eval_mul_real(arguments, env, (expr_real_t)integer_result * arg->real, index + 1);
    default: UNREACHABLE("number arguments exprected");
    }
  }
  return make_expr_integer(integer_result);
}

Expr *expr_eval_sub_real(Expr_List *arguments, Expr_Run_Env *env, expr_real_t initial, size_t from) {
  expr_real_t real_result = initial;
  for (size_t index = from, count = arguments->count; index < count; index += 1) {
    Expr *arg = expr_eval(arguments->items[index], env);
    switch (arg->kind) {
    case EXPR_KIND_INTEGER: real_result -= arg->integer; break;
    case EXPR_KIND_REAL: real_result -= arg->real; break;
    default: UNREACHABLE("number arguments exprected");
    }
  }
  return make_expr_real(real_result);
}
Expr *expr_eval_sub(Expr_List *arguments, Expr_Run_Env *env) {
  if (!arguments->count) UNREACHABLE("too few arguments");
  Expr *first_arg = expr_eval(arguments->items[0], env);
  expr_int_t integer_result;
  switch (first_arg->kind) {
  case EXPR_KIND_INTEGER: integer_result = first_arg->integer; break;
  case EXPR_KIND_REAL: return expr_eval_sub_real(arguments, env, first_arg->real, 1);
  default: UNREACHABLE("number arguments exprected");
  }
  for (size_t index = 1, count = arguments->count; index < count; index += 1) {
    Expr *arg = expr_eval(arguments->items[index], env);
    switch (arg->kind) {
    case EXPR_KIND_INTEGER: integer_result -= arg->integer; break;
    case EXPR_KIND_REAL: return expr_eval_sub_real(arguments, env, (expr_real_t)integer_result - arg->real, index + 1);
    default: UNREACHABLE("number arguments exprected");
    }
  }
  return make_expr_integer(integer_result);
}

Expr *expr_eval_div(Expr_List *arguments, Expr_Run_Env *env) {
  if (!arguments->count) UNREACHABLE("too few arguments");
  Expr *first_arg = expr_eval(arguments->items[0], env);
  expr_real_t result;
  if (arguments->count == 1) {
    result = 1;
    switch (first_arg->kind) {
    case EXPR_KIND_INTEGER: result /= first_arg->integer; break;
    case EXPR_KIND_REAL: result /= first_arg->real; break;
    default: UNREACHABLE("number arguments exprected");
    }
    return make_expr_real(result);
  }
  switch (first_arg->kind) {
  case EXPR_KIND_INTEGER: result = first_arg->integer; break;
  case EXPR_KIND_REAL: result = first_arg->real; break;
  default: UNREACHABLE("number arguments exprected");
  }
  for (size_t index = 1, count = arguments->count; index < count; index += 1) {
    Expr *arg = expr_eval(arguments->items[index], env);
    switch (arg->kind) {
    case EXPR_KIND_INTEGER: result /= arg->integer; break;
    case EXPR_KIND_REAL: result /= arg->real; break;
    default: UNREACHABLE("number arguments exprected");
    }
  }
  return make_expr_real(result);
}

#define _expr_numbers_compare(left, right, comparison, result)                     \
  bool result;                                                                     \
  do {                                                                             \
    if ((left)->kind == (right)->kind) {                                           \
      switch ((left)->kind) {                                                      \
      case EXPR_KIND_INTEGER: result = (left)->integer == (right)->integer; break; \
      case EXPR_KIND_REAL: result = (left)->real == (right)->real; break;          \
      default: UNREACHABLE("number arguments exprected");                          \
      }                                                                            \
    } else {                                                                       \
      switch ((left)->kind) {                                                      \
      case EXPR_KIND_INTEGER: result = (left)->integer == (right)->real; break;    \
      case EXPR_KIND_REAL: result = (left)->real == (right)->integer; break;       \
      default: UNREACHABLE("number arguments exprected");                          \
      }                                                                            \
    }                                                                              \
  } while(false)

Expr *expr_eval_equal_numbers(Expr_List *arguments, Expr_Run_Env *env) {
  if (!arguments->count) UNREACHABLE("too few arguments");
  if (arguments->count == 1) return make_expr_bool(true);
  Expr *first_arg = expr_eval(arguments->items[0], env);
  for (size_t index = 1, count = arguments->count; index < count; index += 1) {
    Expr *arg = expr_eval(arguments->items[index], env);
    _expr_numbers_compare(first_arg, arg, ==, result);
    if (!result) return make_expr_bool(false);
  }
  return make_expr_bool(true);
}

Expr *expr_eval_lower_than(Expr_List *arguments, Expr_Run_Env *env) {
  if (!arguments->count) UNREACHABLE("too few arguments");
  if (arguments->count == 1) return make_expr_bool(true);
  for (size_t index = 0, count = arguments->count - 1; index < count; index += 1) {
    Expr *left = expr_eval(arguments->items[index], env);
    Expr *right = expr_eval(arguments->items[index + 1], env);
    _expr_numbers_compare(left, right, <, result);
    if (!result) return make_expr_bool(false);
  }
  return make_expr_bool(true);
}

Expr *expr_eval_lower_or_equal(Expr_List *arguments, Expr_Run_Env *env) {
  if (!arguments->count) UNREACHABLE("too few arguments");
  if (arguments->count == 1) return make_expr_bool(true);
  for (size_t index = 0, count = arguments->count - 1; index < count; index += 1) {
    Expr *left = expr_eval(arguments->items[index], env);
    Expr *right = expr_eval(arguments->items[index + 1], env);
    _expr_numbers_compare(left, right, <=, result);
    if (!result) return make_expr_bool(false);
  }
  return make_expr_bool(true);
}

Expr *expr_eval_greater_than(Expr_List *arguments, Expr_Run_Env *env) {
  if (!arguments->count) UNREACHABLE("too few arguments");
  if (arguments->count == 1) return make_expr_bool(true);
  for (size_t index = 0, count = arguments->count - 1; index < count; index += 1) {
    Expr *left = expr_eval(arguments->items[index], env);
    Expr *right = expr_eval(arguments->items[index + 1], env);
    _expr_numbers_compare(left, right, >, result);
    if (!result) return make_expr_bool(false);
  }
  return make_expr_bool(true);
}

Expr *expr_eval_greater_or_equal(Expr_List *arguments, Expr_Run_Env *env) {
  if (!arguments->count) UNREACHABLE("too few arguments");
  if (arguments->count == 1) return make_expr_bool(true);
  for (size_t index = 0, count = arguments->count - 1; index < count; index += 1) {
    Expr *left = expr_eval(arguments->items[index], env);
    Expr *right = expr_eval(arguments->items[index + 1], env);
    _expr_numbers_compare(left, right, >=, result);
    if (!result) return make_expr_bool(false);
  }
  return make_expr_bool(true);
}

Expr *expr_eval_condition(Expr_List *arguments, Expr_Run_Env *env) {
  if (!arguments->count) UNREACHABLE("too few arguments");
  // if (arguments->count == 1) return make_expr_bool(true);
  TODO('if');
  // for (size_t index = 0, count = arguments->count - 1; index < count; index += 1) {
  //   Expr *left = expr_eval(arguments->items[index], env);
  //   Expr *right = expr_eval(arguments->items[index + 1], env);
  //   _expr_numbers_compare(left, right, >=, result);
  //   if (!result) return make_expr_bool(false);
  // }
  // return make_expr_bool(true);
}

typedef struct {
  Expr_Env_Kind kind;
  union {
    Expr_Builtin builtin;
    Expr *variable; // literals
  };
} Expr_Env_Item;
Ht(const char *, Expr_Env_Item, Expr_Run_Env);
void init_expr_env_builtin(Expr_Run_Env *env, const char *name, Expr_Builtin builtin) {
  Expr_Env_Item *item = ht_put(env, name);
  item->kind = EXPR_ENV_KIND_BUILTIN;
  item->builtin = builtin;
}
void init_expr_env(Expr_Run_Env *env) {
  env->hasheq = ht_cstr_hasheq;
  init_expr_env_builtin(env, "+", expr_eval_sum);
  init_expr_env_builtin(env, "*", expr_eval_mul);
  init_expr_env_builtin(env, "-", expr_eval_sub);
  init_expr_env_builtin(env, "/", expr_eval_div);
  init_expr_env_builtin(env, "=", expr_eval_equal_numbers);
  init_expr_env_builtin(env, "<", expr_eval_lower_than);
  init_expr_env_builtin(env, "<=", expr_eval_lower_or_equal);
  init_expr_env_builtin(env, ">", expr_eval_greater_than);
  init_expr_env_builtin(env, ">=", expr_eval_greater_or_equal);
  init_expr_env_builtin(env, "if", expr_eval_condition);
}
// // if (strcmp(symbol_name, "quote")) return expr_eval_quote(input, env);
// // if (strcmp(symbol_name, "car")) return expr_eval_quote(input, env);
// // if (strcmp(symbol_name, "cdr")) return expr_eval_quote(input, env);
// if (strcmp(symbol_name, "if")) return expr_eval_if(input, env);
// // if (strcmp(symbol_name, "define")) return expr_eval_define(input, env);
// // if (strcmp(symbol_name, "lambda")) return expr_eval_lambda(input, env);
// if (strcmp(symbol_name, "begin")) return expr_eval_sequential(input, env);

Expr *expr_eval(Expr *input, Expr_Run_Env *env) {
  if (input->kind == EXPR_KIND_SYMBOL) {
    Expr_Env_Item *item = ht_find(env, input->symbol.name);
    if (item && item->kind == EXPR_ENV_KIND_VARIABLE) return item->variable;
    return input;
  }
  if (input->kind != EXPR_KIND_LIST) return input;
  if (!input->list.count) return input;
  if (input->list.items[input->list.count - 1]) return input;
  Expr *first_expr = input->list.items[0];
  if (!first_expr) return input;
  first_expr = expr_eval(first_expr, env);
  if (first_expr->kind != EXPR_KIND_SYMBOL) return input;
  Expr_Env_Item *item = ht_find(env, first_expr->symbol.name);
  if (!item) UNREACHABLE("undefined symbol");
  if (item->kind == EXPR_ENV_KIND_VARIABLE) return item->variable;
  if (item->kind != EXPR_ENV_KIND_BUILTIN) UNREACHABLE("unknown type");
  Expr_List arguments = {.items = input->list.items + 1, .count = input->list.count - 2 };
  return item->builtin(&arguments, env);
}

int main(int argc, char **argv) {
  UNUSED(ht__find_or_put);
  UNUSED(ht__find_and_delete);
  UNUSED(*ht__key);
  UNUSED(ht__next);
  UNUSED(ht__reset);
  UNUSED(ht__free);

  Nob_String_View source; {
    Nob_String_Builder sb = {0};
    for (int index = 1; index < argc; index += 1) {
      nob_sb_append_cstr(&sb, " ");
      nob_sb_append_cstr(&sb, argv[index]);
    }
    source = nob_sb_to_sv(sb);
    nob_sv_chop_left(&source, 1);
  }
  Expr *input = parse_expr(source);
  // dump_expr(input, 0);
  print_expr(input);
  Expr_Run_Env env={0};
  init_expr_env(&env);
  Expr *result = expr_eval(input, &env);
  print_expr(result);

  return 0;
}
