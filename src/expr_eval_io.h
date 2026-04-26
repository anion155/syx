#ifndef EXPR_EVAL_IO_H
#define EXPR_EVAL_IO_H

#include "expr_ast.h"
#include "expr_eval.h"

void expr_env_init_io(Expr_Env *env);

#endif // EXPR_EVAL_IO_H

#if defined(EXPR_EVAL_IO_IMPLEMENTATION) && !defined(EXPR_EVAL_IO_IMPLEMENTATION_C)
#define EXPR_EVAL_IO_IMPLEMENTATION_C

void expr_env_init_io(Expr_Env *env) {
  UNUSED(env);
  /** Builtins I/O */
  // expr_env_put_symbol(env, "display", (Expr_Value){.kind = EXPR_VALUE_KIND_BUILTIN, .special = expr_builtin_io_display});
  // expr_env_put_symbol(env, "newline", (Expr_Value){.kind = EXPR_VALUE_KIND_BUILTIN, .special = expr_builtin_io_newline});
  // expr_env_put_symbol(env, "read", (Expr_Value){.kind = EXPR_VALUE_KIND_BUILTIN, .special = expr_builtin_io_read});
  // expr_env_put_symbol(env, "write", (Expr_Value){.kind = EXPR_VALUE_KIND_BUILTIN, .special = expr_builtin_io_write});
}

#endif // EXPR_EVAL_IO_IMPLEMENTATION
