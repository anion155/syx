#ifndef SYX_EVAL_H
#define SYX_EVAL_H

#include <general_utils.h>
#include <ht.h>
#include <rc.h>
#include <syx_new/syx_value.h>

typedef struct Syx_Frame {
  Syx_Frame *prev;
  syx_string_view trace;
} Syx_Frame;

typedef struct Syx_Frames_Stack {
  Syx_Frame *latest;
} Syx_Frames_Stack;

Syx_Frames_Stack *make_syx_frames_stack();
void syx_frames_stack_push(Syx_Frames_Stack *frames_stack, syx_string_view trace);
void syx_frames_stack__pop(Syx_Frames_Stack *frames_stack, Syx_Value **to_save, size_t count);
#define syx_frames_stack_pop(frames_stack, ...) \
  syx_frames_stack__pop((frames_stack), (Syx_Value *[]){__VA_ARGS__}, sizeof((Syx_Value *[]){__VA_ARGS__}) / sizeof(Syx_Value *))

typedef Ht(Syx_Symbol *, Syx_Value *) Syx_Env_Symbols;

typedef struct Syx_Env {
  Syx_Env *parent;
  Syx_Env_Symbols symbols;
} Syx_Env;

typedef struct Syx_Eval_Ctx {
  Syx_Frames_Stack *frames_stack;
  Syx_Env *global_env;
  Syx_Env *env;
} Syx_Eval_Ctx;

#define syx_ctx_push_frame(ctx, trace) syx_frames_stack_push((ctx)->frames_stack, trace)
#define syx_ctx_pop_frame(ctx, ...) syx_frames_stack_pop((ctx)->frames_stack __VA_OPT__(, ) __VA_ARGS__)

Syx_Env *make_syx_env(Syx_Env *parent);
Syx_Env *syx_env_global(Syx_Env *env);
Syx_Env *syx_env_lookup(Syx_Env *env, Syx_Symbol *symbol);
Syx_Value *syx_env_lookup_get(Syx_Eval_Ctx *ctx, Syx_Symbol *symbol);
void syx_env_define(Syx_Env *env, Syx_Symbol *symbol, Syx_Value *value);
void syx_env_define_cstr(Syx_Env *env, const char *name, Syx_Value *value);
#define syx_env_define_strlit(env, name, value) syx_env_define((env), make_syx_value_symbol_strlit(name), (value))
void syx_env_set(Syx_Env *env, Syx_Symbol *symbol, Syx_Value *value);

Syx_Eval_Ctx *make_syx_eval_ctx(Syx_Eval_Ctx opt);
Syx_Eval_Ctx *make_global_syx_eval_ctx();
Syx_Eval_Ctx *inherit_syx_eval_ctx(Syx_Eval_Ctx *parent, Syx_Eval_Ctx opt);

#define SYX_EVAL_THROW(ctx, message, ...) SYX_THROW(message, (ctx)->frames_stack->latest __VA_OPT__(, ) __VA_ARGS__)
#define SYX_EVAL_ASSERT(ctx, condition, message, ...) SYX_ASSERT((condition), message, (ctx)->frames_stack->latest __VA_OPT__(, ) __VA_ARGS__)
#define SYX_EVAL_TODO(ctx, message, ...) SYX_TODO(message, (ctx)->frames_stack->latest __VA_OPT__(, ) __VA_ARGS__)

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
#define syx_convert_to(ctx, value, storage, ...) ({                     \
  Syx_Value *__value = rc_acquire((value));                             \
  syx_value_early_exit(__value __VA_OPT__(, ) __VA_ARGS__);             \
  Syx_Value *converted = _Generic(storage,                              \
      bool *: syx_convert_to_bool,                                      \
      Syx_Number *: syx_convert_to_number,                              \
      syx_integer_t *: syx_convert_to_number,                           \
      syx_fractional_t *: syx_convert_to_number,                        \
      syx_string_view *: syx_convert_to_string,                         \
      syx_string *: syx_convert_to_string)((ctx), __value);             \
  rc_acquire(converted);                                                \
  syx_value_early_exit(converted, __value __VA_OPT__(, ) __VA_ARGS__);  \
  rc_release(__value);                                                  \
  *(storage) = _Generic(storage,                                        \
      bool *: syx_boolean_get(converted),                               \
      Syx_Number *: *converted->number,                                 \
      syx_integer_t *: syx_number_get(converted->number),               \
      syx_fractional_t *: syx_number_get(converted->number),            \
      syx_string_view *: sv_from_like(*converted->string),              \
      syx_string *: sb_copy_from_sv(sv_from_like(*converted->string))); \
  rc_release(converted);                                                \
})

#endif // SYX_EVAL_H

#define SYX_EVAL_IMPL
#if defined(SYX_EVAL_IMPL) && !defined(SYX_EVAL_IMPL_C)
#define SYX_EVAL_IMPL_C

#define HT_IMPL
#include <ht.h>
#define RC_IMPL
#include <rc.h>
#define SYX_VALUE_IMPL
#include <syx_new/syx_value.h>
#define GENERAL_UTILS_IMPL
#include <general_utils.h>

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

void syx_frames_stack_push(Syx_Frames_Stack *frames_stack, syx_string_view trace) {
  Syx_Frame *prev = frames_stack->latest;
  Syx_Frame *next = rc_acquire(rc_malloc(sizeof(Syx_Frame) + sizeof(char) * trace.count, .destructor = syx_frame_destructor));
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

uintptr_t ht_syx_symbol_hasheq(Ht_Op op, void const *a_, void const *b_, size_t n) {
  UNUSED(n);
  Syx_Symbol const **a = (Syx_Symbol const **)a_;
  Syx_Symbol const **b = (Syx_Symbol const **)b_;
  switch (op) {
    case HT_HASH: return ht_default_hash((*a)->data, (*a)->count);
    case HT_EQ: return (*a)->count != (*b)->count ? false : memcmp((*a)->data, (*b)->data, (*a)->count) == 0;
  }
  return 0;
}

Syx_Env *make_syx_env(Syx_Env *parent) {
  Syx_Env *env = rc_malloc(sizeof(Syx_Env), .destructor = syx_env_destructor);
  assert(env);
  env->parent = parent ? rc_acquire(parent) : NULL;
  env->symbols = (Syx_Env_Symbols){.hasheq = ht_syx_symbol_hasheq};
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
      if (value->closure->name) syx_value_closure_rename(value->closure, symbol);
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
  syx_ctx_push_frame(ctx, syx_closure_from_builtin(builtin)->name);
  Syx_Value *result = (*builtin)(ctx, evaluated->pair);
  if (!result) result = syx_value_nil();
  rc_acquire(result);
  syx_ctx_pop_frame(ctx);
  rc_release(evaluated);
  return rc_move(result);
}

Syx_Value *syx_eval_closure_lambda(Syx_Eval_Ctx *ctx, Syx_Closure_Lambda *lambda, Syx_Pair *arguments) {
  Syx_Eval_Ctx *call_ctx = rc_acquire(inherit_syx_eval_ctx(ctx, (Syx_Eval_Ctx){.env = make_syx_env(lambda->env)}));
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
    syx_value_early_exit(value, call_ctx);
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
    syx_value_early_exit(value, call_ctx);
    *ht_put(&call_ctx->env->symbols, symbol) = value;
  }
  syx_ctx_push_frame(ctx, syx_closure_from_lambda(lambda)->name);
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
  }
  return result;
}

Syx_Value *syx_eval_pair(Syx_Eval_Ctx *ctx, Syx_Pair *arguments) {
  Syx_Value *head = rc_acquire(syx_eval(ctx, syx_list_next(arguments)));
  syx_value_early_exit(head);
  switch (head->kind) {
    case SYX_VALUE_KIND_CLOSURE: {
      Syx_Value *result = rc_acquire(syx_eval_closure(ctx, head->closure, arguments));
      rc_release(head);
      return rc_move(result);
    }
    default: SYX_EVAL_THROW(ctx, "is not callable");
  }
}

Syx_Value *syx_eval(Syx_Eval_Ctx *ctx, Syx_Value *input) {
  switch (input->kind) {
    case SYX_VALUE_KIND_EXIT: return input;
    case SYX_VALUE_KIND_SYMBOL: {
      Syx_Value *item = syx_env_lookup_get(ctx, input->symbol);
      SYX_EVAL_ASSERT(ctx, item, temp_sprintf("unbound symbol '" SV_Fmt "'", SV_Arg(*input->symbol)));
      return item;
    }
    case SYX_VALUE_KIND_PREFIXED: {
      if (input->prefixed->kind != SYX_PREFIXED_KIND_QUOTE) return input->prefixed->value;
      return input;
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
    syx_value_early_exit(rc_acquire(*item), evaluated);
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
      syx_convert_to(ctx, opt.should_stop(ctx, result), &should_stop, result);
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
    case SYX_VALUE_KIND_CLOSURE: SYX_EVAL_THROW(ctx, "closure can't be converted to bool");
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
    case SYX_VALUE_KIND_CLOSURE: SYX_EVAL_THROW(ctx, "closure can't be converted to number");
    case SYX_VALUE_KIND_EXIT: SYX_EVAL_THROW(ctx, "exit value can't be converted to number");
    case SYX_VALUE_KIND_PREFIXED: SYX_EVAL_THROW(ctx, "prefixed value can't be converted to number");
  }
}

Syx_Value *syx_convert_to_string(Syx_Eval_Ctx *ctx, Syx_Value *value) {
  switch (value->kind) {
    case SYX_VALUE_KIND_PAIR: SYX_EVAL_THROW(ctx, "pair can't be converted to string");
    case SYX_VALUE_KIND_CONST: SYX_EVAL_THROW(ctx, "constant can't be converted to string");
    case SYX_VALUE_KIND_SYMBOL: SYX_EVAL_THROW(ctx, "symbol can't be converted to string");
    case SYX_VALUE_KIND_NUMBER: SYX_EVAL_THROW(ctx, "number can't be converted to string");
    case SYX_VALUE_KIND_STRING: return value;
    case SYX_VALUE_KIND_CLOSURE: SYX_EVAL_THROW(ctx, "closure can't be converted to string");
    case SYX_VALUE_KIND_EXIT: SYX_EVAL_THROW(ctx, "exit value can't be converted to string");
    case SYX_VALUE_KIND_PREFIXED: SYX_EVAL_THROW(ctx, "prefixed value can't be converted to string");
  }
}

#endif // SYX_EVAL_IMPL_C
