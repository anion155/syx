#ifndef SYX_TYPE_EXPRESSIONS_H
#define SYX_TYPE_EXPRESSIONS_H

#include <syx/syx_parser.h>
#include <syx/syx_value.h>

Syx_Value *syx__parse_type(Syx_Parser_Ctx *ctx, Syx_Type **type);
#define syx_parse_type(ctx, ...) syx_get_non_value_with_early_exit(Syx_Type, syx__parse_type, ((ctx)), WITH_DEFAULT((), __VA_ARGS__))
Syx_Value *syx__parse_type_expression(Syx_Parser_Ctx *ctx, Syx_Type **type);
#define syx_parse_type_expression(ctx, ...) syx_get_non_value_with_early_exit(Syx_Type, syx__parse_type_expression, ((ctx)), WITH_DEFAULT((), __VA_ARGS__))

Syx_Value *syx__list_next_type(Syx_Eval_Ctx *ctx, Syx_Pair **list, Syx_Type **type);
#define syx_list_next_type(ctx, list, ...) syx_get_non_value_with_early_exit(Syx_Type, syx__list_next_type, ((ctx), (list)), WITH_DEFAULT((), __VA_ARGS__))
Syx_Value *syx__eval_type_expression(Syx_Eval_Ctx *ctx, Syx_Pair **expression, Syx_Type **type);
#define syx_eval_type_expression(ctx, expression, ...) syx_get_non_value_with_early_exit(Syx_Type, syx__eval_type_expression, ((ctx), (expression)), WITH_DEFAULT((), __VA_ARGS__))

#endif // SYX_TYPE_EXPRESSIONS_H

#if defined(SYX_TYPE_EXPRESSIONS_IMPL) && !defined(SYX_TYPE_EXPRESSIONS_IMPL_C)
#define SYX_TYPE_EXPRESSIONS_IMPL_C

#define SYX_PARSER_IMPL
#include <syx/syx_parser.h>
#define SYX_VALUE_IMPL
#include <syx/syx_value.h>

Syx_Value *syx__parse_type(Syx_Parser_Ctx *ctx, Syx_Type **type) {
  if (ctx->tokens.data[0].kind == SYX_TOKEN_KIND_SYMBOL) {
    Syx_Token token = da_slice_shift(&ctx->tokens);
    Syx_Value *name = rc_acquire(syx_value_symbol_n(token.source.data, token.source.count));
    *type = syx_env_get_type(ctx->env, name->symbol);
    SYX_ASSERT(*type, "unknown type '" SV_FMT "'", (sv_fmt_arg(token.source)));
  } else if (ctx->tokens.data[0].kind == SYX_TOKEN_KIND_LPAREN) {
    *type = syx_parse_type_expression(ctx);
  } else {
    SYX_THROW("expected type definition");
  }
  return NULL;
}
Syx_Value *syx__parse_type_expression(Syx_Parser_Ctx *ctx, Syx_Type **type) {
  Syx_Token token = da_slice_shift(&ctx->tokens);
  SYX_ASSERT(token.kind == SYX_TOKEN_KIND_LPAREN, "expected type expression start");
  token = da_slice_shift(&ctx->tokens);
  SYX_ASSERT(token.kind == SYX_TOKEN_KIND_SYMBOL, "expected type kind");
  if (sv_eq(token.source, sv_from_strlit("ref"))) {
    Syx_Type *target = syx_parse_type(ctx);
    *type = make_syx_type_pointer(NULL, target);
  } else if (sv_eq(token.source, sv_from_strlit("struct"))) {
    SYX_TODO();
  } else if (sv_eq(token.source, sv_from_strlit("fn"))) {
    Syx_Type *return_type = syx_parse_type(ctx);
    rc_acquire(return_type);
    Syx_Types_Array *args_types = rc_acquire(rc_malloc(sizeof(Syx_Types_Array)));
    memset(args_types, 0, sizeof(Syx_Types_Array));
    rc_get(args_types)->methods.destructor = syx_types_da_descructor;
    token = da_slice_shift(&ctx->tokens);
    SYX_ASSERT(token.kind == SYX_TOKEN_KIND_LPAREN, "expected arguments definition start", (), (return_type, args_types));
    while (ctx->tokens.data[0].kind != SYX_TOKEN_KIND_RPAREN) {
      Syx_Type *arg_type = syx_parse_type(ctx, (return_type, args_types));
      da_append(args_types, rc_acquire(arg_type));
    }
    token = da_slice_shift(&ctx->tokens);
    SYX_ASSERT(token.kind == SYX_TOKEN_KIND_RPAREN, "expected arguments definition end", (), (return_type, args_types));
    *type = make_syx_type_function(NULL, (Syx_Type_Function){.args_types = da_slice(*args_types, Syx_Types), .return_type = rc_move(return_type), .vaargs = false});
    rc_acquire(*type);
    rc_release(args_types);
    rc_move(*type);
  } else {
    SYX_THROW("expected type kind: '" SV_FMT "'", (sv_fmt_arg(token.source)));
  }
  token = da_slice_shift(&ctx->tokens);
  SYX_ASSERT(token.kind == SYX_TOKEN_KIND_RPAREN, "expected type expression end");
  return NULL;
}

Syx_Value *syx__list_next_type(Syx_Eval_Ctx *ctx, Syx_Pair **list, Syx_Type **type) {
  Syx_Value *value = syx_list_next(list);
  if (value->kind == SYX_VALUE_KIND_SYMBOL) {
    *type = syx_env_get_type(ctx->env, value->symbol);
    SYX_EVAL_ASSERT(ctx, *type, "unknown type '" SV_FMT "'", (sv_fmt_arg(*value->symbol)));
  } else if (value->kind == SYX_VALUE_KIND_PAIR) {
    Syx_Pair *expression = value->pair;
    *type = syx_eval_type_expression(ctx, &expression);
  } else {
    SYX_EVAL_THROW(ctx, "unsupported type expression");
  }
  return NULL;
}

Syx_Value *syx__eval_type_expression(Syx_Eval_Ctx *ctx, Syx_Pair **expression, Syx_Type **type) {
  Syx_Value *kind = syx_list_next(expression);
  SYX_EVAL_ASSERT(ctx, kind->kind == SYX_VALUE_KIND_SYMBOL, "expected type kind");
  static Syx_Value *ref_s = NULL;
  if (!ref_s) ref_s = rc_acquire(syx_value_symbol_strlit("ref"));
  if (kind->symbol == ref_s->symbol) {
    Syx_Type *target = syx_list_next_type(ctx, expression);
    *type = make_syx_type_pointer(NULL, target);
    return NULL;
  }
  static Syx_Value *struct_s = NULL;
  if (!struct_s) struct_s = rc_acquire(syx_value_symbol_strlit("struct"));
  if (kind->symbol == struct_s->symbol) {
    SYX_EVAL_TODO(ctx);
    // return NULL;
  }
  static Syx_Value *fn_s = NULL;
  if (!fn_s) fn_s = rc_acquire(syx_value_symbol_strlit("fn"));
  if (kind->symbol == fn_s->symbol) {
    Syx_Type *return_type = syx_list_next_type(ctx, expression);
    rc_acquire(return_type);
    Syx_Types_Array *args_types = rc_acquire(rc_malloc(sizeof(Syx_Types_Array)));
    memset(args_types, 0, sizeof(Syx_Types_Array));
    rc_get(args_types)->methods.destructor = syx_types_da_descructor;
    Syx_Value *arg_definition_value = syx_list_next(expression);
    SYX_EVAL_ASSERT(ctx, arg_definition_value->kind == SYX_VALUE_KIND_PAIR, "expected arguments definition", (), (return_type, args_types));
    Syx_Pair *arg_definition = arg_definition_value->pair;
    while (arg_definition) {
      Syx_Type *arg_type = syx_list_next_type(ctx, &arg_definition, (return_type, args_types));
      da_append(args_types, rc_acquire(arg_type));
    }
    *type = make_syx_type_function(NULL, (Syx_Type_Function){.args_types = da_slice(*args_types, Syx_Types), .return_type = rc_move(return_type), .vaargs = false});
    rc_acquire(*type);
    rc_release(args_types);
    rc_move(*type);
    return NULL;
  }
  SYX_EVAL_THROW(ctx, "expected type kind: '" SV_FMT "'", (sv_fmt_arg(*kind->symbol)));
}

#endif // SYX_TYPE_EXPRESSIONS_IMPL
