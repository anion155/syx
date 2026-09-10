#ifndef SYX_EVAL_H
#define SYX_EVAL_H

#include <da.h>
#include <defines.h>
#include <sb.h>
#include <syx_new/syx_value.h>

typedef struct Syx_Frame {
  Syx_Frame *prev;
  String_View trace;
} Syx_Frame;

typedef struct Syx_Frames_Stack {
  Syx_Frame *latest;
} Syx_Frames_Stack;

Syx_Frames_Stack *make_syx_frames_stack();
void syx_frames_stack_push(Syx_Frames_Stack *frames_stack, String_View trace);
void syx_frames_stack_push_f(Syx_Frames_Stack *frames_stack, PRINTF_FMT_PARAM const char *format, ...) PRINTF_ATTRIBUTE(2, 3);
void syx_frames_stack__pop(Syx_Frames_Stack *frames_stack, Syx_Value **to_save, size_t count);
#define syx_frames_stack_pop(frames_stack, ...) \
  syx_frames_stack__pop((frames_stack), (Syx_Value *[]){__VA_ARGS__}, sizeof((Syx_Value *[]){__VA_ARGS__}) / sizeof(Syx_Value *))

typedef struct Syx_Env {
  Syx_Symbol *name;
  Syx_Env *parent;
  Syx_Symbols_Ht symbols;
} Syx_Env;

typedef struct Syx_Eval_Ctx {
  Syx_Frames_Stack *frames_stack;
  Syx_Env *global_env;
  Syx_Env *env;
} Syx_Eval_Ctx;

#define syx_ctx_push_frame(ctx, trace) syx_frames_stack_push((ctx)->frames_stack, trace)
#define syx_ctx_push_frame_f(ctx, format, ...) syx_frames_stack_push_f((ctx)->frames_stack, format __VA_OPT__(, ) __VA_ARGS__)
#define syx_ctx_pop_frame(ctx, ...) syx_frames_stack_pop((ctx)->frames_stack __VA_OPT__(, ) __VA_ARGS__)

Syx_Env *make_syx_env(Syx_Symbol *name, Syx_Env *parent);
Syx_Env *syx_env_global(Syx_Env *env);
Syx_Env *syx_env_lookup(Syx_Env *env, Syx_Symbol *symbol);
Syx_Value *syx_env_lookup_get(Syx_Eval_Ctx *ctx, Syx_Symbol *symbol);
void syx_env_define(Syx_Env *env, Syx_Symbol *symbol, Syx_Value *value);
void syx_env_define_cstr(Syx_Env *env, const char *name, Syx_Value *value);
#define syx_env_define_strlit(env, name, value) syx_env_define((env), make_syx_value_symbol_strlit(name)->symbol, (value))
void syx_env_set(Syx_Env *env, Syx_Symbol *symbol, Syx_Value *value);

Syx_Eval_Ctx *make_syx_eval_ctx(Syx_Eval_Ctx opt);
Syx_Eval_Ctx *make_global_syx_eval_ctx();
Syx_Eval_Ctx *inherit_syx_eval_ctx(Syx_Eval_Ctx *parent, Syx_Eval_Ctx opt);

#define SYX_EVAL_THROW(ctx, message, ...) SYX_THROW(message, WITH_DEFAULT((), FIRST_ARG(__VA_ARGS__)), WITH_DEFAULT((), SECOND_ARG(__VA_ARGS__, )), (ctx)->frames_stack->latest)
#define SYX_EVAL_TODO(ctx, message, ...) SYX_TODO(message, WITH_DEFAULT((), FIRST_ARG(__VA_ARGS__)), WITH_DEFAULT((), SECOND_ARG(__VA_ARGS__, )), (ctx)->frames_stack->latest)
#define SYX_EVAL_ASSERT(ctx, condition, message, ...) SYX_ASSERT((condition), message, WITH_DEFAULT((), FIRST_ARG(__VA_ARGS__)), WITH_DEFAULT((), SECOND_ARG(__VA_ARGS__, )), (ctx)->frames_stack->latest)

Syx_Value *syx_eval(Syx_Eval_Ctx *ctx, Syx_Value *input);
Syx_Value *syx_eval_unquote(Syx_Eval_Ctx *ctx, Syx_Value *unevaluated);
Syx_Value *syx_eval_map_list(Syx_Eval_Ctx *ctx, Syx_Pair *list);

typedef struct Syx_Eval_Forms_List_Opt {
  Syx_Value *(*should_stop)(Syx_Eval_Ctx *ctx, Syx_Value *evaluated);
  Syx_Value *initial;
} Syx_Eval_Forms_List_Opt;

Syx_Value *syx_eval_forms_list_opt(Syx_Eval_Ctx *ctx, Syx_Pair *forms, Syx_Eval_Forms_List_Opt opt);
#define syx_eval_forms_list(ctx, forms, ...) syx_eval_forms_list_opt((ctx), (forms), (Syx_Eval_Forms_List_Opt){__VA_ARGS__})

Syx_Value *syx_convert_to_bool(Syx_Eval_Ctx *ctx, Syx_Value *value);
Syx_Value *syx_convert_to_number(Syx_Eval_Ctx *ctx, Syx_Value *value);
Syx_Value *syx_convert_to_string(Syx_Eval_Ctx *ctx, Syx_Value *value);
#define syx_convert_to(ctx, value, storage, ...) ({                                                             \
  Syx_Value *__value = rc_acquire((value));                                                                     \
  syx_value_early_exit(__value __VA_OPT__(, ) __VA_ARGS__);                                                     \
  Syx_Value *converted = _Generic(storage,                                                                      \
      bool *: syx_convert_to_bool,                                                                              \
      Syx_Number *: syx_convert_to_number,                                                                      \
      syx_integer_t *: syx_convert_to_number,                                                                   \
      syx_fractional_t *: syx_convert_to_number,                                                                \
      String_Builder *: syx_convert_to_string,                                                                  \
      String_View *: syx_convert_to_string,                                                                     \
      String *: syx_convert_to_string)((ctx), __value);                                                         \
  rc_acquire(converted);                                                                                        \
  syx_value_early_exit(converted, (__value EXPAND_WITH_COMMA WITH_DEFAULT((), __VA_ARGS__)));                   \
  rc_release(__value);                                                                                          \
  _Generic(storage,                                                                                             \
      bool *: *((bool *)storage) = syx_boolean_get(converted),                                                  \
      Syx_Number *: *((Syx_Number *)storage) = *converted->number,                                              \
      syx_integer_t *: *((syx_integer_t *)storage) = syx_number_get(converted->number),                         \
      syx_fractional_t *: *((syx_fractional_t *)storage) = syx_number_get(converted->number),                   \
      String_Builder *: sb_append_sv((String_Builder *)storage, *converted->string),                            \
      String_View *: *((String_View *)storage) = sv_from_like(*converted->string),                              \
      String *: string_assign((String *)storage, string_from(sb_append_sv, sv_from_like(*converted->string)))); \
  rc_release(converted);                                                                                        \
})

#endif // SYX_EVAL_H

#if defined(SYX_EVAL_IMPL) && !defined(SYX_EVAL_IMPL_C)
#define SYX_EVAL_IMPL_C

#define HT_IMPL
#include <ht.h>
#define RC_IMPL
#include <rc.h>
#define SYX_VALUE_IMPL
#include <syx_new/syx_value.h>
#define SYX_OBJECT_IMPL
#include <syx_new/syx_object.h>
#define SYX_NATIVE_IMPL
#include <syx_new/syx_native.h>

void syx_frames_stack_destructor(void *data) {
  Syx_Frames_Stack *stack = data;
  rc_release(stack->latest);
}

Syx_Frames_Stack *make_syx_frames_stack() {
  Syx_Frames_Stack *frames_stack = rc_acquire(rc_malloc(sizeof(Syx_Frames_Stack), .destructor = syx_frames_stack_destructor));
  assert(frames_stack);
  frames_stack->latest = NULL;
  return frames_stack;
}

void syx_frame_destructor(void *data) {
  Syx_Frame *frame = data;
  rc_release(frame->prev);
}

void syx_frames_stack_push(Syx_Frames_Stack *frames_stack, String_View trace) {
  Syx_Frame *prev = frames_stack->latest;
  Syx_Frame *next = rc_acquire(rc_malloc(sizeof(Syx_Frame) + sizeof(char) * trace.count, .destructor = syx_frame_destructor));
  next->trace.data = (char *)next + 1;
  assert(next);
  memcpy((char *)next->trace.data, trace.data, trace.count);
  if (prev) {
    next->prev = rc_acquire(prev);
    rc_release(prev);
  } else {
    next->prev = NULL;
  }
  frames_stack->latest = (next);
}

void syx_frames_stack_push_f(Syx_Frames_Stack *frames_stack, const char *format, ...) {
  String_Builder sb = {0};
  va_list args;
  va_start(args, format);
  sb_vappendf(&sb, format, args);
  va_end(args);
  syx_frames_stack_push(frames_stack, sv_from_like(sb));
  sb_free(&sb);
}

void syx_frames_stack__pop(Syx_Frames_Stack *frames_stack, Syx_Value **to_save, size_t count) {
  Syx_Frame *current = frames_stack->latest;
  frames_stack->latest = current->prev ? rc_acquire(current->prev) : NULL;
  rc__guarded((void **)to_save, count) {
    rc_release(current);
  }
}

void syx_env_destructor(void *data) {
  Syx_Env *env = data;
  ht_foreach(value, &env->symbols) {
    rc_release(*value);
    Syx_Symbol *symbol = ht_key(&env->symbols, value);
    rc_release(syx_value_from_symbol(symbol));
  }
  ht_free(&env->symbols);
  if (env->parent) rc_release(env->parent);
}

void syx_env_graph_visitor(Rc_Circulars *circulars, const void *data, const void *source) {
  const Syx_Env *env = data;
  ht_foreach(value, &env->symbols) {
    rc_graph_visitor(circulars, (void **)value, source);
  }
  ht_free(&env->symbols);
  if (env->parent) rc_release(env->parent);
}

Syx_Env *make_syx_env(Syx_Symbol *name, Syx_Env *parent) {
  Syx_Env *env = rc_malloc(sizeof(Syx_Env), .destructor = syx_env_destructor);
  assert(env);
  if (name) {
    rc_acquire(syx_value_from_symbol(name));
    env->name = name;
  }
  env->parent = rc_acquire(parent);
  env->symbols.hasheq = ht_syx_symbol_hasheq;
  return env;
}

Syx_Env *syx_env_global(Syx_Env *env) {
  while (env->parent != NULL) env = env->parent;
  return env;
}

Syx_Env *syx_env_lookup(Syx_Env *env, Syx_Symbol *symbol) {
  Syx_Value **item = NULL;
  while (env != NULL) {
    item = ht_find(&env->symbols, symbol);
    if (item != NULL) break;
    env = env->parent;
  }
  return env;
}

Syx_Value *syx_env_lookup_get(Syx_Eval_Ctx *ctx, Syx_Symbol *symbol) {
  Syx_Env *env = syx_env_lookup(ctx->env, symbol);
  if (env == NULL) env = syx_env_lookup(ctx->global_env, symbol);
  if (env == NULL) return NULL;
  return *ht_find(&env->symbols, symbol);
}

void syx_env_define(Syx_Env *env, Syx_Symbol *symbol, Syx_Value *value) {
  Syx_Value **item = ht_find(&env->symbols, symbol);
  if (item == NULL) {
    rc_acquire(syx_value_from_symbol(symbol));
    *ht_put(&env->symbols, symbol) = rc_acquire(value);
  } else {
    rc_release(*item);
    *item = rc_acquire(value);
  }
  switch (value->kind) {
    case SYX_VALUE_KIND_CLOSURE: {
      if (!value->closure->name) syx_value_closure_rename(value->closure, symbol);
    } break;
    default:
  }
}

void syx_env_define_cstr(Syx_Env *env, const char *name, Syx_Value *value) {
  syx_env_define(env, make_syx_value_symbol_cstr(name)->symbol, value);
}

void syx_env_set(Syx_Env *env, Syx_Symbol *symbol, Syx_Value *value) {
  Syx_Env *container_env = syx_env_lookup(env, symbol);
  syx_env_define(container_env == NULL ? env : container_env, symbol, value);
}

void syx_eval_ctx_destructor(void *_data) {
  Syx_Eval_Ctx *ctx = _data;
  rc_release(ctx->frames_stack);
  rc_release(ctx->env);
  rc_release(ctx->global_env);
}

Syx_Eval_Ctx *make_syx_eval_ctx(Syx_Eval_Ctx opt) {
  Syx_Eval_Ctx *ctx = rc_malloc(sizeof(Syx_Eval_Ctx), .destructor = syx_eval_ctx_destructor);
  assert(ctx);
  *ctx = opt;
  rc_acquire(ctx->frames_stack);
  rc_acquire(ctx->global_env);
  rc_acquire(ctx->env);
  return ctx;
}

Syx_Eval_Ctx *inherit_syx_eval_ctx(Syx_Eval_Ctx *parent, Syx_Eval_Ctx opt) {
  return make_syx_eval_ctx((Syx_Eval_Ctx){
      .frames_stack = opt.frames_stack ? opt.frames_stack : parent->frames_stack,
      .global_env = opt.global_env ? opt.global_env : parent->global_env,
      .env = opt.env ? opt.env : parent->env});
}

Syx_Value *syx_eval_closure_specialf(Syx_Eval_Ctx *ctx, Syx_Closure_Special_Form *specialf, Syx_Pair *arguments) {
  Syx_Value *result = (*specialf)(ctx, arguments);
  if (!result) result = syx_value_nil();
  return result;
}

Syx_Value *syx_eval_closure_builtin(Syx_Eval_Ctx *ctx, Syx_Closure_Builtin *builtin, Syx_Pair *arguments) {
  Syx_Value *evaluated = rc_acquire(syx_eval_map_list(ctx, arguments));
  syx_value_early_exit(evaluated);
  Syx_Symbol *name = syx_closure_from_builtin(builtin)->name;
  if (name) syx_ctx_push_frame_f(ctx, SV_FMT "()", sv_fmt_arg(*name));
  else syx_ctx_push_frame(ctx, sv_from_strlit("<anonim>()"));
  Syx_Value *result = (*builtin)(ctx, evaluated->pair);
  if (!result) result = syx_value_nil();
  rc_acquire(result);
  syx_ctx_pop_frame(ctx);
  rc_release(evaluated);
  return rc_move(result);
}

Syx_Value *syx_eval_closure_lambda(Syx_Eval_Ctx *ctx, Syx_Closure_Lambda *lambda, Syx_Pair *arguments) {
  Syx_Symbol *name = syx_closure_from_lambda(lambda)->name;
  Syx_Env *call_env = make_syx_env(name, lambda->env);
  Syx_Eval_Ctx *call_ctx = rc_acquire(inherit_syx_eval_ctx(ctx, (Syx_Eval_Ctx){.env = call_env}));
  for (Syx_Value *arg_current,
       *arg_next = syx_value_from_pair(arguments),
       *arg,
       *define = NULL,
       *defines_list = syx_value_from_pair(lambda->defines);
       syx_list_for_each_next(&arg_current, &arg_next, &arg, NULL);) {
    if (arg->kind == SYX_VALUE_KIND_PREFIXED && arg->prefixed->kind == SYX_PREFIXED_KIND_COLON) SYX_EVAL_TODO(ctx, "named param bindings");
    SYX_EVAL_ASSERT(ctx, defines_list->kind == SYX_VALUE_KIND_PAIR && defines_list->pair, "list of defines expected");
    define = defines_list->pair->left;
    if (define->kind == SYX_VALUE_KIND_PAIR && define->pair) define = define->pair->left;
    SYX_EVAL_ASSERT(ctx, define->kind == SYX_VALUE_KIND_SYMBOL, "argument name expected");
    defines_list = defines_list->pair->right;
    if (defines_list->kind != SYX_VALUE_KIND_PAIR) SYX_EVAL_TODO(ctx, "implement rest arguments");
    Syx_Value *value = rc_acquire(syx_eval(ctx, arg));
    syx_value_early_exit(value, (call_ctx));
    syx_env_define(call_ctx->env, define->symbol, rc_move(value));
  }
  syx_list_for_each(lambda->defines, define) {
    if (define->kind != SYX_VALUE_KIND_PAIR) continue;
    SYX_EVAL_ASSERT(ctx, define->pair, "argument definition expected");
    SYX_EVAL_ASSERT(ctx, define->pair->left->kind == SYX_VALUE_KIND_SYMBOL, "argument name expected");
    Syx_Symbol *symbol = define->pair->left->symbol;
    SYX_EVAL_ASSERT(ctx, define->pair->right->kind == SYX_VALUE_KIND_PAIR, "default argument value expected");
    if (!define->pair->right->pair) continue;
    Syx_Value *default_arg = define->pair->right->pair->left;
    Syx_Value **stored = ht_find(&call_ctx->env->symbols, symbol);
    if (stored != NULL) continue;
    Syx_Value *value = rc_acquire(syx_eval(ctx, default_arg));
    syx_value_early_exit(value, (call_ctx));
    *ht_put(&call_ctx->env->symbols, symbol) = value;
  }
  syx_ctx_push_frame_f(ctx, SV_FMT "()", sv_fmt_arg(*name));
  Syx_Value *result = rc_acquire(syx_eval_forms_list(call_ctx, lambda->forms));
  syx_ctx_pop_frame(ctx);
  rc_release(call_ctx);
  if (result->kind == SYX_VALUE_KIND_EXIT && result->exit->kind == SYX_EXIT_KIND_RETURNED) {
    Syx_Value *return_value = result;
    result = rc_acquire(result->exit->returned);
    rc_release(return_value);
  }
  return rc_move(result);
}

Syx_Value *syx_eval_closure(Syx_Eval_Ctx *ctx, Syx_Closure *closure, Syx_Pair *arguments) {
  Syx_Value *result;
  switch (closure->kind) {
    case SYX_CLOSURE_KIND_SPECIALF: result = syx_eval_closure_specialf(ctx, &closure->specialf, arguments); break;
    case SYX_CLOSURE_KIND_BUILTIN: result = syx_eval_closure_builtin(ctx, &closure->builtin, arguments); break;
    case SYX_CLOSURE_KIND_LAMBDA: result = syx_eval_closure_lambda(ctx, closure->lambda, arguments); break;
    case SYX_CLOSURE_KIND_NATIVE_CONSTRUCTOR: result = syx_eval_construct_native(ctx, closure->native, arguments); break;
  }
  return result;
}

Syx_Value *syx_eval_in_environment(Syx_Eval_Ctx *ctx, Syx_Symbol *env_name, Syx_Pair *arguments) {
  Syx_Env *env = ctx->env;
  while (env && env->name != env_name) env = env->parent;
  if (!env) SYX_EVAL_THROW(ctx, "environment not found");
  Syx_Eval_Ctx *eval_ctx = inherit_syx_eval_ctx(ctx, (Syx_Eval_Ctx){.env = env});
  Syx_Value *result = rc_acquire(syx_eval_forms_list(eval_ctx, arguments, .initial = syx_value_nil()));
  syx_value_early_exit(result, (eval_ctx));
  rc_release(eval_ctx);
  return result;
}

Syx_Value *syx_eval_object(Syx_Eval_Ctx *ctx, Syx_Object *object, Syx_Pair *arguments) {
  Syx_Value *result = NULL;
  while (arguments) {
    if (!object) break;
    Syx_Value *argument = syx_list_next_nullable(&arguments);
    if (argument->kind != SYX_VALUE_KIND_PREFIXED) break;
    if (argument->prefixed->kind != SYX_PREFIXED_KIND_COLON) break;
    if (argument->prefixed->value->kind != SYX_VALUE_KIND_SYMBOL) break;
    Syx_Symbol *field_name = argument->prefixed->value->symbol;
    rc_release(result);
    result = syx_object_get(ctx, object, field_name);
    if (result->kind == SYX_VALUE_KIND_OBJECT) object = result->object;
    else object = NULL;
  }
  if (arguments) SYX_EVAL_THROW(ctx, "field getter expected", (), (result));
  return rc_move(result);
}

Syx_Value *syx_eval_pair(Syx_Eval_Ctx *ctx, Syx_Pair *arguments) {
  Syx_Value *head = rc_acquire(syx_eval(ctx, syx_list_next(&arguments)));
  syx_value_early_exit(head);
  switch (head->kind) {
    case SYX_VALUE_KIND_CLOSURE: {
      Syx_Value *result = rc_acquire(syx_eval_closure(ctx, head->closure, arguments));
      rc_release(head);
      return rc_move(result);
    }
    case SYX_VALUE_KIND_PREFIXED: {
      switch (head->prefixed->kind) {
        case SYX_PREFIXED_KIND_DOLLAR: {
          if (head->prefixed->value->kind != SYX_VALUE_KIND_SYMBOL) SYX_EVAL_THROW(ctx, "is not callable");
          return syx_eval_in_environment(ctx, head->prefixed->value->symbol, arguments);
        }
        default:;
      }
    }
    case SYX_VALUE_KIND_OBJECT: return syx_eval_object(ctx, head->object, arguments);
    case SYX_VALUE_KIND_NATIVE: {
      switch (head->native->type->kind) {
        // case SYX_TYPE_KIND_PRIMITIVE:
        // case SYX_TYPE_KIND_STRUCTURE:
        // case SYX_TYPE_KIND_PTR:
        // case SYX_TYPE_KIND_FUNCTION_PTR:
        // case SYX_TYPE_KIND_VALUE_PTR:
        default:
      }
    }
    default:;
  }
  SYX_EVAL_THROW(ctx, "is not callable");
}

Syx_Value *syx_eval(Syx_Eval_Ctx *ctx, Syx_Value *input) {
  switch (input->kind) {
    case SYX_VALUE_KIND_EXIT: return input;
    case SYX_VALUE_KIND_SYMBOL: {
      Syx_Value *item = syx_env_lookup_get(ctx, input->symbol);
      SYX_EVAL_ASSERT(ctx, item, "unbound symbol '" SV_FMT "'", (sv_fmt_arg(*input->symbol)));
      return item;
    }
    case SYX_VALUE_KIND_PREFIXED: {
      switch (input->prefixed->kind) {
        case SYX_PREFIXED_KIND_QUOTE: return input->prefixed->value;
        default: return input;
      }
    }
    case SYX_VALUE_KIND_PAIR: {
      if (!input->pair) return input;
      return syx_eval_pair(ctx, input->pair);
    }
    default: return input;
  }
}

Syx_Value *syx_eval_unquote(Syx_Eval_Ctx *ctx, Syx_Value *unevaluated) {
  if (unevaluated->kind != SYX_VALUE_KIND_PREFIXED) return unevaluated;
  if (unevaluated->prefixed->kind != SYX_PREFIXED_KIND_UNQUOTE) return unevaluated;
  return syx_eval(ctx, unevaluated->prefixed->value);
}

Syx_Value *syx_eval_map_list(Syx_Eval_Ctx *ctx, Syx_Pair *list) {
  Syx_Value *evaluated = NULL;
  syx_list_map(list, item, &evaluated) {
    *item = syx_eval(ctx, *item);
    if (!*item) *item = syx_value_nil();
    syx_value_early_exit(rc_acquire(*item), (evaluated));
  }
  return rc_move(evaluated);
}

Syx_Value *syx_eval_forms_list_opt(Syx_Eval_Ctx *ctx, Syx_Pair *forms, Syx_Eval_Forms_List_Opt opt) {
  Syx_Value *result = NULL;
  if (opt.initial) result = rc_acquire(opt.initial);
  syx_list_for_each(forms, form) {
    if (result) rc_release(result);
    result = rc_acquire(syx_eval(ctx, form));
    syx_value_early_exit(result);
    if (opt.should_stop != NULL) {
      bool should_stop = {0};
      syx_convert_to(ctx, opt.should_stop(ctx, result), &should_stop, (result));
      if (should_stop) return rc_move(result);
    }
  }
  SYX_EVAL_ASSERT(ctx, result != NULL, "empty forms list");
  return rc_move(result);
}

Syx_Value *syx_convert_to_bool(Syx_Eval_Ctx *ctx, Syx_Value *value) {
  switch (value->kind) {
    case SYX_VALUE_KIND_PAIR: return syx_value_bool(value->pair);
    case SYX_VALUE_KIND_CONST: {
      if (value == syx_value_bool_true() || value == syx_value_bool_false()) return value;
      SYX_EVAL_THROW(ctx, "constant can't be converted to bool");
    }
    case SYX_VALUE_KIND_SYMBOL: SYX_EVAL_THROW(ctx, "symbol can't be converted to bool");
    case SYX_VALUE_KIND_NUMBER: return syx_value_bool(syx_number_get(value->number));
    case SYX_VALUE_KIND_STRING: SYX_EVAL_THROW(ctx, "string can't be converted to bool");
    case SYX_VALUE_KIND_OBJECT: SYX_EVAL_TODO(ctx, "object converted to bool");
    case SYX_VALUE_KIND_CLOSURE: SYX_EVAL_THROW(ctx, "closure can't be converted to bool");
    case SYX_VALUE_KIND_NATIVE: {
      Syx_Native *native = value->native;
      switch (native->type->kind) {
        case SYX_TYPE_KIND_PRIMITIVE: {
          switch (native->type->primitive) {
            case SYX_PRIMITIVE_TYPE_KIND_VOID: SYX_EVAL_THROW(ctx, "primitive native void can't be converted to bool");
            case SYX_PRIMITIVE_TYPE_KIND_CHAR: return syx_value_bool(*(char *)native->data);
            case SYX_PRIMITIVE_TYPE_KIND_I8: return syx_value_bool(*(int8_t *)native->data);
            case SYX_PRIMITIVE_TYPE_KIND_I16: return syx_value_bool(*(int16_t *)native->data);
            case SYX_PRIMITIVE_TYPE_KIND_I32: return syx_value_bool(*(int32_t *)native->data);
            case SYX_PRIMITIVE_TYPE_KIND_I64: return syx_value_bool(*(int64_t *)native->data);
            case SYX_PRIMITIVE_TYPE_KIND_I128: return syx_value_bool(*(__int128_t *)native->data);
            case SYX_PRIMITIVE_TYPE_KIND_U8: return syx_value_bool(*(uint8_t *)native->data);
            case SYX_PRIMITIVE_TYPE_KIND_U16: return syx_value_bool(*(uint16_t *)native->data);
            case SYX_PRIMITIVE_TYPE_KIND_U32: return syx_value_bool(*(uint32_t *)native->data);
            case SYX_PRIMITIVE_TYPE_KIND_U64: return syx_value_bool(*(uint64_t *)native->data);
            case SYX_PRIMITIVE_TYPE_KIND_U128: return syx_value_bool(*(__uint128_t *)native->data);
            case SYX_PRIMITIVE_TYPE_KIND_F16: return syx_value_bool(f16_canonical_to_float(*(f16_canonical_t *)native->data));
            case SYX_PRIMITIVE_TYPE_KIND_F32: return syx_value_bool(*(float *)native->data);
            case SYX_PRIMITIVE_TYPE_KIND_F64: return syx_value_bool(*(double *)native->data);
            case SYX_PRIMITIVE_TYPE_KIND_F80: return syx_value_bool(f80_canonical_to_long_double(*(f80_canonical_t *)native->data));
            case SYX_PRIMITIVE_TYPE_KIND_F128: return syx_value_bool(f128_canonical_to_long_double(*(f128_canonical_t *)native->data));
            case SYX_PRIMITIVE_TYPE_KIND_F64PAIR: return syx_value_bool(f64pair_canonical_to_long_double(*(f64pair_canonical_t *)native->data));
          }
        }
        case SYX_TYPE_KIND_STRUCTURE: SYX_EVAL_THROW(ctx, "native structure can't be converted to bool");
        case SYX_TYPE_KIND_PTR:
        case SYX_TYPE_KIND_FUNCTION_PTR:
        case SYX_TYPE_KIND_VALUE_PTR:
          return syx_value_bool(*(void **)native->data);
      }
    }
    case SYX_VALUE_KIND_EXIT: SYX_EVAL_THROW(ctx, "exit value can't be converted to bool");
    case SYX_VALUE_KIND_PREFIXED: SYX_EVAL_THROW(ctx, "prefixed value can't be converted to bool");
  }
}

Syx_Value *syx_convert_to_number(Syx_Eval_Ctx *ctx, Syx_Value *value) {
  switch (value->kind) {
    case SYX_VALUE_KIND_PAIR: SYX_EVAL_THROW(ctx, "pair can't be converted to number");
    case SYX_VALUE_KIND_CONST: {
      if (value == syx_value_bool_true()) return make_syx_value_number_integer(1);
      if (value == syx_value_bool_false()) return make_syx_value_number_integer(0);
      SYX_EVAL_THROW(ctx, "constant can't be converted to number");
    }
    case SYX_VALUE_KIND_SYMBOL: SYX_EVAL_THROW(ctx, "symbol can't be converted to number");
    case SYX_VALUE_KIND_NUMBER: return value;
    case SYX_VALUE_KIND_STRING: SYX_EVAL_THROW(ctx, "string can't be converted to number");
    case SYX_VALUE_KIND_OBJECT: SYX_EVAL_TODO(ctx, "object converted to number");
    case SYX_VALUE_KIND_CLOSURE: SYX_EVAL_THROW(ctx, "closure can't be converted to number");
    case SYX_VALUE_KIND_NATIVE: {
      Syx_Native *native = value->native;
      switch (native->type->kind) {
        case SYX_TYPE_KIND_PRIMITIVE: {
          switch (native->type->primitive) {
            case SYX_PRIMITIVE_TYPE_KIND_VOID: SYX_EVAL_THROW(ctx, "primitive native void can't be converted to number");
            case SYX_PRIMITIVE_TYPE_KIND_CHAR: return make_syx_value_number_integer(*(char *)native->data);
            case SYX_PRIMITIVE_TYPE_KIND_I8: return make_syx_value_number_integer(*(int8_t *)native->data);
            case SYX_PRIMITIVE_TYPE_KIND_I16: return make_syx_value_number_integer(*(int16_t *)native->data);
            case SYX_PRIMITIVE_TYPE_KIND_I32: return make_syx_value_number_integer(*(int32_t *)native->data);
            case SYX_PRIMITIVE_TYPE_KIND_I64: return make_syx_value_number_integer(*(int64_t *)native->data);
            case SYX_PRIMITIVE_TYPE_KIND_I128: return make_syx_value_number_integer(*(__int128_t *)native->data);
            case SYX_PRIMITIVE_TYPE_KIND_U8: return make_syx_value_number_integer(*(uint8_t *)native->data);
            case SYX_PRIMITIVE_TYPE_KIND_U16: return make_syx_value_number_integer(*(uint16_t *)native->data);
            case SYX_PRIMITIVE_TYPE_KIND_U32: return make_syx_value_number_integer(*(uint32_t *)native->data);
            case SYX_PRIMITIVE_TYPE_KIND_U64: return make_syx_value_number_integer(*(uint64_t *)native->data);
            case SYX_PRIMITIVE_TYPE_KIND_U128: return make_syx_value_number_integer(*(__uint128_t *)native->data);
            case SYX_PRIMITIVE_TYPE_KIND_F16: return make_syx_value_number_fractional(f16_canonical_to_float(*(f16_canonical_t *)native->data));
            case SYX_PRIMITIVE_TYPE_KIND_F32: return make_syx_value_number_fractional(*(float *)native->data);
            case SYX_PRIMITIVE_TYPE_KIND_F64: return make_syx_value_number_fractional(*(double *)native->data);
            case SYX_PRIMITIVE_TYPE_KIND_F80: return make_syx_value_number_fractional(f80_canonical_to_float(*(f80_canonical_t *)native->data));
            case SYX_PRIMITIVE_TYPE_KIND_F128: return make_syx_value_number_fractional(f128_canonical_to_float(*(f128_canonical_t *)native->data));
            case SYX_PRIMITIVE_TYPE_KIND_F64PAIR: return make_syx_value_number_fractional(f64pair_canonical_to_float(*(f64pair_canonical_t *)native->data));
          }
        }
        case SYX_TYPE_KIND_STRUCTURE: SYX_EVAL_THROW(ctx, "native structure can't be converted to number");
        case SYX_TYPE_KIND_PTR:
        case SYX_TYPE_KIND_FUNCTION_PTR:
          return make_syx_value_number_integer((uintptr_t)*(void **)native->data);
        case SYX_TYPE_KIND_VALUE_PTR: return syx_convert_to_number(ctx, *(Syx_Value **)native->data);
      }
    }
    case SYX_VALUE_KIND_EXIT: SYX_EVAL_THROW(ctx, "exit value can't be converted to number");
    case SYX_VALUE_KIND_PREFIXED: SYX_EVAL_THROW(ctx, "prefixed value can't be converted to number");
  }
}

Syx_Value *syx_convert_to_string(Syx_Eval_Ctx *ctx, Syx_Value *value) {
  switch (value->kind) {
    case SYX_VALUE_KIND_PAIR: SYX_EVAL_THROW(ctx, "pair can't be converted to string");
    case SYX_VALUE_KIND_CONST: SYX_EVAL_THROW(ctx, "constant can't be converted to string");
    case SYX_VALUE_KIND_SYMBOL: SYX_EVAL_THROW(ctx, "symbol can't be converted to string");
    case SYX_VALUE_KIND_NUMBER: {
      String str = string_from(sb_append_number, syx_number_get(value->number));
      return make_syx_value_string_dup(str);
    }
    case SYX_VALUE_KIND_STRING: return value;
    case SYX_VALUE_KIND_OBJECT: SYX_EVAL_TODO(ctx, "object converted to string");
    case SYX_VALUE_KIND_CLOSURE: SYX_EVAL_THROW(ctx, "closure can't be converted to string");
    case SYX_VALUE_KIND_NATIVE: {
      Syx_Native *native = value->native;
      switch (native->type->kind) {
        case SYX_TYPE_KIND_PRIMITIVE: SYX_EVAL_THROW(ctx, "native can't be converted to string");
        case SYX_TYPE_KIND_STRUCTURE: SYX_EVAL_THROW(ctx, "native structure can't be converted to string");
        case SYX_TYPE_KIND_PTR: {
          if (native->type) {
            if (native->type == SYX_KNOWN_TYPES()->c_str) {
              return make_syx_value_string_cstr_dup(*(char **)native->data);
            }
            if (native->type == SYX_KNOWN_TYPES()->c_string) {
              return make_syx_value_string((String *)native->data);
            }
          }
          SYX_EVAL_THROW(ctx, "native pointer can't be converted to string");
        }
        case SYX_TYPE_KIND_FUNCTION_PTR: SYX_EVAL_THROW(ctx, "native function pointer can't be converted to string");
        case SYX_TYPE_KIND_VALUE_PTR: return syx_convert_to_string(ctx, *(Syx_Value **)native->data);
      }
    }
    case SYX_VALUE_KIND_EXIT: SYX_EVAL_THROW(ctx, "exit value can't be converted to string");
    case SYX_VALUE_KIND_PREFIXED: SYX_EVAL_THROW(ctx, "prefixed value can't be converted to string");
  }
}

#endif // SYX_EVAL_IMPL_C
