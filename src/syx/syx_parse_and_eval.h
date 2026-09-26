#ifndef SYX_PARSE_AND_EVAL_H
#define SYX_PARSE_AND_EVAL_H

#include <syx/syx_eval.h>
#include <syx/syx_parser.h>
#include <syx/syx_value.h>

Syx_Value *syx_parse_and_eval(Syx_Eval_Ctx *ctx, String_View source);

#endif // SYX_PARSE_AND_EVAL_H

#if defined(SYX_PARSE_AND_EVAL_IMPL) && !defined(SYX_PARSE_AND_EVAL_IMPL_C)
#define SYX_PARSE_AND_EVAL_IMPL_C

#define SYX_VALUE_IMPL
#include <syx/syx_value.h>
#define SYX_EVAL_IMPL
#include <syx/syx_eval.h>
#define SYX_PARSER_IMPL
#include <syx/syx_parser.h>

Syx_Value *syx_parse_and_eval(Syx_Eval_Ctx *ctx, String_View source) {
  Syx_Value *expressions = rc_acquire(parse_syx(ctx->env, source, true));
  Syx_Value *result = NULL;
  syx_list_for_each(expressions->pair, expression) {
    syx_value_early_exit(expression);
    if (result) rc_release(result);
    result = rc_acquire(syx_eval(ctx, expression));
    syx_value_early_exit(result);
  }
  return rc_move(result);
}

#endif // SYX_PARSE_AND_EVAL_IMPL_C
