#ifndef SYX_EVAL_H
#define SYX_EVAL_H

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

#define SYX_THROW(ctx, message, ...)                                        \
  do {                                                                      \
    rc_release_all(__VA_ARGS__);                                            \
    Syx_Value *reason = make_syx_value_string_cstr_dup(message);            \
    return make_syx_value_exit_thrown(reason, (ctx)->frames_stack->latest); \
  } while (0)
#define SYX_ASSERT(ctx, condition, message, ...)            \
  do {                                                      \
    if (!(condition)) SYX_THROW(ctx, message, __VA_ARGS__); \
  } while (0)
#define SYX_TODO(ctx, message, ...) SYX_THROW((ctx), "TODO: " message __VA_OPT__(, ) __VA_ARGS__)

#define syx_eval_early_exit(value, ...)                  \
  do {                                                   \
    Syx_Value *_value = (value);                         \
    if (_value && _value->kind == SYX_VALUE_KIND_EXIT) { \
      rc_release_all(__VA_ARGS__);                       \
      return rc_move(_value);                            \
    }                                                    \
  } while (0)

typedef struct Syx_Eval_Forms_List_Opt {
  // Syx_Value *(*should_stop)(Syx_Eval_Ctx *ctx, Syx_Value *evaluated);
  Syx_Value *initial;
} Syx_Eval_Forms_List_Opt;

Syx_Value *syx_eval_forms_list_opt(Syx_Eval_Ctx *ctx, Syx_Pair *forms, Syx_Eval_Forms_List_Opt opt);
#define syx_eval_forms_list(ctx, forms, ...) syx_eval_forms_list_opt((ctx), (forms), (Syx_Eval_Forms_List_Opt){__VA_ARGS__})

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
  rc__guarded(to_save, count) {
    rc_release(current);
  }
}

void syx_env_destructor(void *data) {
  Syx_Env *env = data;
  ht_foreach(value, &env->symbols) {
    rc_release(*value);
    rc_release(get_syxv_from_symbol(ht_key(&env->symbols, value)));
  }
  ht_free(&env->symbols);
  if (env->parent) rc_release(env->parent);
}

void syx_env_graph_visitor(Rc_Circulars *circulars, const void *data, const void *source) {
  const Syx_Env *env = data;
  // const Syx_Closure_Lambda *lambda = value->closure->lambda;
  ht_foreach(value, &env->symbols) {
    rc_graph_visitor(circulars, (void **)value, source);
  }
  ht_free(&env->symbols);
  if (env->parent) rc_release(env->parent);
}

uintptr_t ht_syxv_symbol_hasheq(Ht_Op op, void const *a_, void const *b_, size_t n) {
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
  env->symbols = (Syx_Env_Symbols){.hasheq = ht_syxv_symbol_hasheq};
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
    rc_acquire(get_syxv_from_symbol(symbol));
    *ht_put(&env->symbols, symbol) = rc_acquire(value);
  } else {
    rc_release(*item);
    *item = rc_acquire(value);
  }
  // TODO: rename anonymous closures
  // if (value->kind == SYX_VALUE_KIND_CLOSURE && value->closure->name.data == NULL) {
  //   value = syx_value_closure_rename(value, symbol, size_t additional_size);
  // }
}

void syx_env_define_cstr(Syx_Env *env, const char *name, Syx_Value *value) {
  syx_env_define(env, &make_syx_value_symbol_cstr(name)->symbol, value);
}

void syx_env_set(Syx_Env *env, Syx_Symbol *symbol, Syx_Value *value) {
  Syx_Env *container_env = syx_env_lookup(env, symbol);
  syx_env_define(container_env == NULL ? env : container_env, symbol, value);
}

Syx_Eval_Ctx *inherit_syx_eval_ctx(Syx_Eval_Ctx *parent, Syx_Eval_Ctx opt) {
  return make_syx_eval_ctx((Syx_Eval_Ctx){
      .frames_stack = opt.frames_stack ? opt.frames_stack : parent->frames_stack,
      .global_env = opt.global_env ? opt.global_env : parent->global_env,
      .env = opt.env ? opt.env : parent->env});
}

Syx_Value *syx_eval_specialf(Syx_Eval_Ctx *ctx, Syx_Closure_Special_Form *specialf, Syx_Pair *arguments) {
  Syx_Value *result = (*specialf)(ctx, arguments);
  if (!result) result = make_syxv_nil();
  return result;
}

Syx_Value *syx_eval_builtin(Syx_Eval_Ctx *ctx, Syx_Closure_Builtin *builtin, Syx_Pair *arguments) {
  Syx_Value *evaluated = rc_acquire(syx_eval_map_list(ctx, arguments));
  syx_eval_early_exit(evaluated);
  Syx_Closure *closure = (Syx_Closure *)(builtin - offsetof(Syx_Closure, builtin));
  syx_ctx_push_frame(ctx, closure->name);
  Syx_Value *result = (*builtin)(ctx, evaluated->pair);
  if (!result) result = make_syxv_nil();
  rc_acquire(result);
  syx_ctx_pop_frame(ctx);
  rc_release(evaluated);
  return rc_move(result);
}

Syx_Value *syx_eval_lambda(Syx_Eval_Ctx *ctx, Syx_Closure_Lambda *lambda, Syx_Pair *arguments) {
  Syx_Closure *closure = (Syx_Closure *)(lambda - offsetof(Syx_Closure, lambda));
  Syx_Eval_Ctx *call_ctx = rc_acquire(inherit_syx_eval_ctx(ctx, (Syx_Eval_Ctx){.env = make_syx_env(lambda->env)}));
  Syx_Value *defines_list = lambda->defines;
  for (Syx_Value *arg_current,
       *arg_next = (Syx_Value *)(arguments - offsetof(Syx_Value, pair)),
       *arg,
       *define = NULL,
       *defines_list = lambda->defines;
       syx_list_for_each_next(&arg_current, &arg_next, &arg, NULL);) {
    if (arg->kind == SYX_VALUE_KIND_SPECIAL && arg->special->kind == SYX_SPECIAL_KIND_COLON) SYX_TODO(ctx, "named para bindings");
    SYX_ASSERT(ctx, defines_list->kind == SYX_VALUE_KIND_PAIR, "list of defines expected");
    define = defines_list->pair->left;
    if (define->kind == SYX_VALUE_KIND_PAIR) define = define->pair->left;
    SYX_ASSERT(ctx, define->kind == SYX_VALUE_KIND_SYMBOL, "argument name expected");
    defines_list = defines_list->pair->right;
    if (defines_list->kind != SYX_VALUE_KIND_PAIR) SYX_TODO(ctx, "implement rest arguments");
    Syx_Value *value = rc_acquire(syx_eval(ctx, arg));
    syx_eval_early_exit(value, call_ctx);
    syx_env_define(call_ctx->env, define, rc_move(value));
  }
  syx_list_for_each(define, lambda->defines) {
    if (define->kind != SYX_VALUE_KIND_PAIR) continue;
    SYX_ASSERT(ctx, define->pair->left->kind == SYX_VALUE_KIND_SYMBOL, "argument name expected");
    Syx_Symbol *symbol = define->pair->left->symbol;
    SYX_ASSERT(ctx, define->pair->right->kind == SYX_VALUE_KIND_PAIR, "default argument value expected");
    Syx_Value *default_arg = define->pair->right->pair->left;
    Syx_Value **stored = ht_find(&call_ctx->env->symbols, symbol);
    if (stored != NULL) continue;
    Syx_Value *value = rc_acquire(syx_eval(ctx, default_arg));
    syx_eval_early_exit(value, call_ctx);
    *ht_put(&call_ctx->env->symbols, symbol) = value;
  }
  syx_ctx_push_frame(ctx, closure->name);
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

Syx_Value *syx_eval(Syx_Eval_Ctx *ctx, Syx_Value *input) {
  if (input->kind == SYX_VALUE_KIND_EXIT) return input;
  if (input->kind == SYX_VALUE_KIND_SYMBOL) {
    Syx_Value *item = syx_env_lookup_get(ctx, &input->symbol);
    SYX_ASSERT(ctx, item, temp_sprintf("unbound symbol '" SV_Fmt "'", SV_Arg(*input->symbol)));
    return item;
  }
  if (input->kind != SYX_VALUE_KIND_PAIR) return input;
  Syx_Value *head = rc_acquire(syx_eval(ctx, input->pair->left));
  SYX_ASSERT(ctx, head->kind == SYX_VALUE_KIND_CLOSURE, "is not a procedure");
  syx_eval_early_exit(head);
  Syx_Value *arguments = input->pair->right;
  SYX_ASSERT(ctx, arguments->kind == SYX_VALUE_KIND_PAIR || arguments->kind == SYX_VALUE_KIND_NIL, "unexpected pair's right value");
  Syx_Value *result;
  switch (head->closure->kind) {
    case SYX_CLOSURE_KIND_SPECIALF: result = syx_eval_specialf(ctx, &head->closure->specialf, arguments->pair); break;
    case SYX_CLOSURE_KIND_BUILTIN: result = syx_eval_builtin(ctx, &head->closure->builtin, arguments->pair); break;
    case SYX_CLOSURE_KIND_LAMBDA: result = syx_eval_lambda(ctx, head->closure->lambda, arguments->pair); break;
  }
  rc_acquire(result);
  rc_release(head);
  return rc_move(result);
}

Syx_Value *syx_eval_map_list(Syx_Eval_Ctx *ctx, Syx_Pair *list) {
  Syx_Value *evaluated = NULL;
  syx_list_map(item, list, &evaluated) {
    *item = syx_eval(ctx, *item);
    if (!*item) *item = make_syxv_nil();
    syx_eval_early_exit(rc_acquire(*item), evaluated);
  }
  return rc_move(evaluated);
}

Syx_Value *syx_eval_forms_list_opt(Syx_Eval_Ctx *ctx, Syx_Pair *forms, Syx_Eval_Forms_List_Opt opt) {
  Syx_Value *result = NULL;
  if (opt.initial) result = rc_acquire(opt.initial);
  syx_list_for_each(form, (Syx_Value *)(forms - offsetof(Syx_Value, pair))) {
    if (result) rc_release(result);
    result = rc_acquire(syx_eval(ctx, form));
    syx_eval_early_exit(result);
    // if (opt.should_stop != NULL) {
    //   bool should_stop = {0};
    //   syx_convert_to(ctx, opt.should_stop(ctx, result), &should_stop, result);
    //   if (should_stop) return rc_move(result);
    // }
  }
  SYX_ASSERT(ctx, result != NULL, "empty forms list");
  return rc_move(result);
}

#endif // SYX_EVAL_IMPL_C
