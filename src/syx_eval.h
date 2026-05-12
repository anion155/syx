#ifndef SYX_EVAL_H
#define SYX_EVAL_H

#include <ht.h>
#include <magic.h>
#include <nob.h>
#include <rc.h>

#include "syx_value.h"

#define RUNTIME_ERROR(ctx, message, ...)                         \
  do {                                                           \
    rc_release_all(__VA_ARGS__);                                 \
    SyxV *reason = make_syxv_string_cstr(message);               \
    return make_syxv_thrown((ctx)->frame_stack->latest, reason); \
  } while (0)

typedef Ht(SyxV_Symbol *, SyxV *) Syx_Env_Symbols;

struct Syx_Env {
  Syx_Env *parent;
  Syx_Env_Symbols symbols;
  char *description;
};

struct Syx_Frame {
  Syx_Frame *prev;
  SyxV *callable;
};

typedef struct Syx_Frame_Stack {
  Syx_Frame *latest;
} Syx_Frame_Stack;

struct Syx_Eval_Ctx {
  Syx_Env *env;
  Syx_Frame_Stack *frame_stack;
};

Syx_Eval_Ctx *make_global_syx_eval_ctx();
Syx_Eval_Ctx *inherit_syx_eval_ctx_opt(Syx_Eval_Ctx *parent, Syx_Eval_Ctx opt);
#define inherit_syx_eval_ctx(parent, ...) inherit_syx_eval_ctx_opt((parent), ((Syx_Eval_Ctx){__VA_ARGS__}))
void syx_ctx_push_frame(Syx_Eval_Ctx *ctx, SyxV *callable);
void syx_ctx_pop_frame(Syx_Eval_Ctx *ctx);
syx_string_t stringify_call_stack(Syx_Eval_Ctx *ctx, Syx_Frame *frame);

void syx_env_destructor(void *data);
uintptr_t ht_syxv_symbol_hasheq(Ht_Op op, void const *a_, void const *b_, size_t n);
Syx_Env *make_syx_env(Syx_Env *parent, const char *description);
Syx_Env *syx_env_global(Syx_Env *env);
Syx_Env *syx_env_lookup(Syx_Env *env, SyxV_Symbol *symbol);
SyxV *syx_env_lookup_get(Syx_Env *env, SyxV_Symbol *symbol);
void syx_env_define(Syx_Env *env, SyxV_Symbol *symbol, SyxV *value);
void syx_env_define_cstr(Syx_Env *env, const char *name, SyxV *value);
void syx_env_set(Syx_Env *env, SyxV_Symbol *symbol, SyxV *value);
Syx_Env *make_global_syx_env();

SyxV *syx_eval_specialf(Syx_Eval_Ctx *ctx, Syx_SpecialF *specialf, SyxV *arguments);
SyxV *syx_eval_builtin(Syx_Eval_Ctx *ctx, Syx_Builtin *builtin, SyxV *arguments);
SyxV *syx_eval_closure(Syx_Eval_Ctx *ctx, Syx_Closure *closure, SyxV *arguments);
SyxV *syx_eval(Syx_Eval_Ctx *ctx, SyxV *input);

bool syx_eval_should_early_exit(SyxV *value, const void *items[], size_t count);
#define syx_eval_early_exit(value, ...)                                  \
  do {                                                                   \
    SyxV *_value = (value);                                              \
    const void *items[] = {__VA_ARGS__};                                 \
    const size_t count = sizeof(items) / sizeof(items[0]);               \
    if (syx_eval_should_early_exit(_value, items, count)) return _value; \
  } while (0)
#define syx_eval_assert(ctx, control, reason, ...)                               \
  do {                                                                           \
    if (!(control)) {                                                            \
      rc_release_all(__VA_ARGS__);                                               \
      return make_syxv_thrown((ctx)->frame_stack,                                \
                              _Generic((reason),                                 \
                                  SyxV *: (reason),                              \
                                  const char *: make_syxv_string_cstr(reason))); \
    }                                                                            \
  } while (0)

bool syx_eval_report_error(Syx_Eval_Ctx *ctx, SyxV *value);

typedef struct Syx_Eval_Forms_List_Opt {
  SyxV *(*should_stop)(Syx_Eval_Ctx *ctx, SyxV *evaluated);
  SyxV *default_result;
} Syx_Eval_Forms_List_Opt;

SyxV *syx__eval_forms_list_opt(Syx_Eval_Ctx *ctx, SyxV *forms_list, Syx_Eval_Forms_List_Opt opt);
#define syx_eval_forms_list(env, forms_list, ...) syx__eval_forms_list_opt((env), (forms_list), (Syx_Eval_Forms_List_Opt){__VA_ARGS__})

#define syx_convert_to(ctx, value, storage, ...)                        \
  do {                                                                  \
    SyxV *__value = (value);                                            \
    syx_eval_early_exit(__value __VA_OPT__(, ) __VA_ARGS__);            \
    rc_acquire(__value);                                                \
    SyxV *converted = _Generic(storage,                                 \
        bool *: syx_convert_to_bool,                                    \
        syx_integer_t *: syx_convert_to_integer,                        \
        syx_fractional_t *: syx_convert_to_fractional,                  \
        syx_string_view_t *: syx_convert_to_string,                     \
        syx_string_t *: syx_convert_to_string)((ctx), __value);         \
    syx_eval_early_exit(converted, __value __VA_OPT__(, ) __VA_ARGS__); \
    rc_acquire(converted);                                              \
    rc_release(__value);                                                \
    *(storage) = _Generic(storage,                                      \
        bool *: converted->boolean,                                     \
        syx_integer_t *: converted->integer,                            \
        syx_fractional_t *: converted->fractional,                      \
        syx_string_view_t *: converted->string,                         \
        syx_string_t *: sb_copy_from_sv(converted->string));            \
    rc_release(converted);                                              \
  } while (0)

SyxV *syx_convert_to_bool(Syx_Eval_Ctx *ctx, SyxV *value);
SyxV *syx_convert_to_integer(Syx_Eval_Ctx *ctx, SyxV *value);
SyxV *syx_convert_to_fractional(Syx_Eval_Ctx *ctx, SyxV *value);
SyxV *syx_convert_to_string(Syx_Eval_Ctx *ctx, SyxV *value);

#define PRINT_SYX_ENV_DEEP_CURRENT_ONLY 1
#define PRINT_SYX_ENV_DEEP_ALL 0
#define PRINT_SYX_ENV_DEEP_ALL_WITH_GLOBAL -1

typedef struct Print_Syx_Env_Opt {
  int deep;
  int indent;
} Print_Syx_Env_Opt;

void fprint_syx_env_opt(FILE *f, Syx_Env *env, Print_Syx_Env_Opt opt);
#define fprint_syx_env(f, env, ...) fprint_syx_env_opt((f), (env), (Print_Syx_Env_Opt){__VA_ARGS__})
#define print_syx_env(env, ...) fprint_syx_env_opt(stdout, (env), (Print_Syx_Env_Opt){__VA_ARGS__})

#endif // SYX_EVAL_H

#if defined(SYX_EVAL_IMPL) && !defined(SYX_EVAL_IMPL_C)
#define SYX_EVAL_IMPL_C

#define SYX_VALUE_IMPL
#include "syx_value.h"
#define SYX_EVAL_SPECIALF_IMPL
#include "syx_eval_specialf.h"
#define SYX_EVAL_BUILTINS_IMPL
#include "syx_eval_builtins.h"

void syx_frame_destructor(void *data) {
  Syx_Frame *frame = (Syx_Frame *)data;
  if (frame->callable) rc_release(frame->callable);
  if (frame->prev) rc_release(frame->prev);
}

void syx_frame_stack_destructor(void *data) {
  Syx_Frame_Stack *stack = (Syx_Frame_Stack *)data;
  if (stack->latest) rc_release(stack->latest);
}

void syx_eval_ctx_destructor(void *data) {
  Syx_Eval_Ctx *ctx = (Syx_Eval_Ctx *)data;
  rc_release(ctx->frame_stack);
  rc_release(ctx->env);
}

Syx_Eval_Ctx *make_global_syx_eval_ctx() {
  Syx_Eval_Ctx *ctx = rc_alloc(sizeof(Syx_Eval_Ctx), syx_eval_ctx_destructor);
  ctx->env = rc_acquire(make_global_syx_env());
  ctx->frame_stack = rc_acquire(rc_alloc(sizeof(Syx_Frame_Stack), syx_frame_stack_destructor));
  ctx->frame_stack->latest = NULL;
  return ctx;
}

Syx_Eval_Ctx *inherit_syx_eval_ctx_opt(Syx_Eval_Ctx *parent, Syx_Eval_Ctx opt) {
  Syx_Eval_Ctx *ctx = rc_alloc(sizeof(Syx_Eval_Ctx), syx_eval_ctx_destructor);
  ctx->env = rc_acquire(opt.env ? opt.env : parent->env);
  ctx->frame_stack = rc_acquire(opt.frame_stack ? opt.frame_stack : parent->frame_stack);
  return ctx;
}

void syx_ctx_push_frame(Syx_Eval_Ctx *ctx, SyxV *callable) {
  Syx_Frame *prev = ctx->frame_stack->latest;
  Syx_Frame *next = rc_alloc(sizeof(Syx_Frame), syx_frame_destructor);
  next->callable = rc_acquire(callable);
  if (prev) {
    next->prev = rc_acquire(prev);
    rc_release(prev);
  } else {
    next->prev = NULL;
  }
  ctx->frame_stack->latest = rc_acquire(next);
}

void syx_ctx_pop_frame(Syx_Eval_Ctx *ctx) {
  Syx_Frame *current = ctx->frame_stack->latest;
  ctx->frame_stack->latest = current->prev ? rc_acquire(current->prev) : NULL;
  rc_release(current);
}

syx_string_t stringify_call_stack(Syx_Eval_Ctx *ctx, Syx_Frame *frame) {
  UNUSED(ctx);
  syx_string_t sb = {0};
  while (frame) {
    const char *name;
    switch (frame->callable->kind) {
      case SYXV_KIND_BUILTIN: name = frame->callable->builtin.name; break;
      case SYXV_KIND_CLOSURE: name = frame->callable->closure.name; break;
      default: name = ""; break;
    }
    if (!name) name = "<anonim>";
    sb_appendf(&sb, " at %s\n", name);
    frame = frame->prev;
  }
  return sb;
}

void syx_env_destructor(void *data) {
  Syx_Env *env = data;
  ht_foreach(syxv, &env->symbols) {
    switch ((*syxv)->kind) {
      case SYXV_KIND_CLOSURE: {
        if ((*syxv)->closure.env != env) rc_release((*syxv)->closure.env);
      } break;
      default: break;
    }
    rc_release(*syxv);
    rc_release(get_syxv_from_symbol(ht_key(&env->symbols, syxv)));
  }
  ht_free(&env->symbols);
  if (env->parent) rc_release(env->parent);
  if (env->description) free(env->description);
}

uintptr_t ht_syxv_symbol_hasheq(Ht_Op op, void const *a_, void const *b_, size_t n) {
  UNUSED(n);
  SyxV_Symbol const **a = (SyxV_Symbol const **)a_;
  SyxV_Symbol const **b = (SyxV_Symbol const **)b_;
  switch (op) {
    case HT_HASH: return ht_default_hash((*a)->name, (*a)->length);
    case HT_EQ: return (*a)->length != (*b)->length ? false : memcmp((*a)->name, (*b)->name, (*a)->length) == 0;
  }
  return 0;
}

Syx_Env *make_syx_env(Syx_Env *parent, const char *description) {
  Syx_Env *env = rc_alloc(sizeof(Syx_Env), syx_env_destructor);
  env->parent = parent ? rc_acquire(parent) : NULL;
  env->symbols = (Syx_Env_Symbols){.hasheq = ht_syxv_symbol_hasheq};
  env->description = description ? strdup(description) : NULL;
  return env;
}

Syx_Env *syx_env_global(Syx_Env *env) {
  while (env->parent != NULL) env = env->parent;
  return env;
}

Syx_Env *syx_env_lookup(Syx_Env *env, SyxV_Symbol *symbol) {
  SyxV **item = NULL;
  while (env != NULL) {
    item = ht_find(&env->symbols, symbol);
    if (item != NULL) break;
    env = env->parent;
  }
  return env;
}

SyxV *syx_env_lookup_get(Syx_Env *env, SyxV_Symbol *symbol) {
  env = syx_env_lookup(env, symbol);
  if (env == NULL) return NULL;
  return *ht_find(&env->symbols, symbol);
}

void syx_env_define(Syx_Env *env, SyxV_Symbol *symbol, SyxV *value) {
  SyxV **item = ht_find(&env->symbols, symbol);
  if (item == NULL) {
    rc_acquire(get_syxv_from_symbol(symbol));
    *ht_put(&env->symbols, symbol) = rc_acquire(value);
  } else {
    switch ((*item)->kind) {
      case SYXV_KIND_CLOSURE: {
        if ((*item)->closure.env != env) rc_release((*item)->closure.env);
      } break;
      default: break;
    }
    rc_release(*item);
    *item = rc_acquire(value);
  }
  switch (value->kind) {
    case SYXV_KIND_SPECIALF: {
      if (!value->specialf.name) value->specialf.name = strndup(symbol->name, symbol->length);
    } break;
    case SYXV_KIND_BUILTIN: {
      if (!value->builtin.name) value->builtin.name = strndup(symbol->name, symbol->length);
    } break;
    case SYXV_KIND_CLOSURE: {
      if (!value->closure.name) value->closure.name = strndup(symbol->name, symbol->length);
      if (value->closure.env != env) rc_acquire(value->closure.env);
    } break;
    default: break;
  }
}

void syx_env_define_cstr(Syx_Env *env, const char *name, SyxV *value) {
  syx_env_define(env, &make_syxv_symbol_cstr(name)->symbol, value);
}

void syx_env_set(Syx_Env *env, SyxV_Symbol *symbol, SyxV *value) {
  Syx_Env *container_env = syx_env_lookup(env, symbol);
  syx_env_define(container_env == NULL ? env : container_env, symbol, value);
}

Syx_Env *make_global_syx_env() {
  Syx_Env *env = make_syx_env(NULL, "<global>");
  syx_env_define_special_forms(env);
  syx_env_define_builtins(env);
  //   syx_env_define_arithmetic(env);
  //   syx_env_define_comparison(env);
  //   syx_env_define_equality(env);
  //   syx_env_define_list(env);
  //   syx_env_define_string(env);
  //   syx_env_define_type_predicates(env);
  //   syx_env_define_type_conversion(env);
  //   syx_env_define_io(env);
  return env;
}

SyxV *syx_eval_specialf(Syx_Eval_Ctx *ctx, Syx_SpecialF *specialf, SyxV *arguments) {
  SyxV *result = specialf->eval(ctx, specialf, arguments);
  if (!result) return make_syxv_nil();
  return result;
}

SyxV *syx_eval_builtin(Syx_Eval_Ctx *ctx, Syx_Builtin *builtin, SyxV *arguments) {
  SyxV *evaluated = NULL;
  syxv_list_map(argument, arguments, &evaluated) {
    *argument = syx_eval(ctx, *argument);
    syx_eval_early_exit(*argument, evaluated);
    rc_acquire(*argument);
    rc_move(*argument);
  }
  syx_ctx_push_frame(ctx, (SyxV *)((char *)builtin - offsetof(SyxV, builtin)));
  SyxV *result = builtin->eval(ctx, evaluated);
  if (!result) result = make_syxv_nil();
  rc_acquire(result);
  syx_ctx_pop_frame(ctx);
  rc_release(evaluated);
  return rc_move(result);
}

SyxV *syx_eval_closure(Syx_Eval_Ctx *ctx, Syx_Closure *closure, SyxV *arguments) {
  Syx_Eval_Ctx *call_ctx = rc_acquire(inherit_syx_eval_ctx(ctx, .env = make_syx_env(closure->env, temp_sprintf("call<%s>", closure->name))));
  SyxV *it = arguments;
  SyxV *last_name = NULL;
  syxv_list_for_each(name_v, closure->defines, &last_name) {
    SyxV_Symbol *symbol;
    if (name_v->kind == SYXV_KIND_PAIR) {
      TODO("implement default param values");
      // symbol = &name_v->pair.left->symbol;
    } else {
      symbol = &name_v->symbol;
    }
    // continue_same_param:
    if (ht_find(&call_ctx->env->symbols, symbol)) continue;
    if (it->kind == SYXV_KIND_NIL) TODO("Implement smaller arguments passed or currying");
    if (it->kind != SYXV_KIND_PAIR) RUNTIME_ERROR(ctx, "Malformed arguments list");
    if (it->pair.left->kind == SYXV_KIND_SYMBOL && it->pair.left->symbol.name[0] == ':') {
      TODO("redone named param binding");
      // const char *named_param = it->pair.left->symbol.name + 1;
      // it = it->pair.right;
      // if (it->kind != SYXV_KIND_PAIR) RUNTIME_ERROR(ctx, "Malformed arguments list");
      // SyxV *left = syx_eval(ctx, it->pair.left);
      // syx_eval_early_exit(left, call_ctx);
      // syx_env_define_cstr(call_ctx->env, named_param, left);
      // it = it->pair.right;
      // goto continue_same_param;
    }
    SyxV *value = syx_eval(ctx, it->pair.left);
    syx_eval_early_exit(value, call_ctx);
    syx_env_define(call_ctx->env, symbol, value);
    it = it->pair.right;
  }
  if (last_name->kind != SYXV_KIND_NIL) {
    TODO("implement rest param values");
    // SyxV_Symbol *symbol = &last_name->symbol;
    // SyxV *rest = it;
    // SyxV *evaluated = NULL;
    // SyxV **last_argument = NULL;
    // syxv_list_map(argument, rest, &evaluated, &last_argument) {
    //   *argument = syx_eval(ctx, *argument);
    //   syx_eval_early_exit(*argument, call_ctx, evaluated);
    // }
    // if ((*last_argument)->kind != SYXV_KIND_NIL) {
    //   *last_argument = syx_eval(ctx, *last_argument);
    //   syx_eval_early_exit(*last_argument, call_ctx, evaluated);
    //   rc_acquire(*last_argument);
    // }
    // syx_env_define(call_ctx->env, symbol, rest);
  }
  syxv_list_for_each(name_v, closure->defines) {
    if (name_v->kind != SYXV_KIND_PAIR) continue;
    SyxV **value = ht_find(&call_ctx->env->symbols, &name_v->pair.left->symbol);
    if (value != NULL) continue;
    (*value) = syx_eval(ctx, name_v->pair.right);
    syx_eval_early_exit(*value, call_ctx);
    rc_acquire(*value);
  }
  syx_ctx_push_frame(ctx, (SyxV *)((char *)closure - offsetof(SyxV, closure)));
  SyxV *result = rc_acquire(syx_eval_forms_list(call_ctx, closure->forms));
  syx_ctx_pop_frame(ctx);
  rc_release(call_ctx);
  if (result->kind == SYXV_KIND_RETURN_VALUE) {
    SyxV *return_value = result;
    result = rc_acquire(result->return_value);
    rc_release(return_value);
  }
  return rc_move(result);
}

SyxV *syx_eval(Syx_Eval_Ctx *ctx, SyxV *input) {
  if (input->kind == SYXV_KIND_QUOTE) return input->quote;
  if (input->kind == SYXV_KIND_SYMBOL) {
    SyxV *item = syx_env_lookup_get(ctx->env, &input->symbol);
    if (item == NULL) RUNTIME_ERROR(ctx, temp_sprintf("unbound symbol '%s'", input->symbol.name));
    return item;
  }
  if (input->kind != SYXV_KIND_PAIR) return input;
  if (!input->pair.left) return input;
  SyxV *head = syx_eval(ctx, input->pair.left);
  syx_eval_early_exit(head);
  rc_acquire(head);
  SyxV *arguments = input->pair.right;
  SyxV *result;
  switch (head->kind) {
    case SYXV_KIND_NIL: RUNTIME_ERROR(ctx, "nil is not a procedure");
    case SYXV_KIND_SYMBOL: RUNTIME_ERROR(ctx, "symbol is not a procedure");
    case SYXV_KIND_PAIR: RUNTIME_ERROR(ctx, "pair is not a procedure");
    case SYXV_KIND_BOOL: RUNTIME_ERROR(ctx, "bool is not a procedure");
    case SYXV_KIND_INTEGER: RUNTIME_ERROR(ctx, "integer is not a procedure");
    case SYXV_KIND_FRACTIONAL: RUNTIME_ERROR(ctx, "fractional is not a procedure");
    case SYXV_KIND_STRING: RUNTIME_ERROR(ctx, "string is not a procedure");
    case SYXV_KIND_QUOTE: RUNTIME_ERROR(ctx, "quote is not a procedure");
    case SYXV_KIND_SPECIALF: result = syx_eval_specialf(ctx, &head->specialf, arguments); break;
    case SYXV_KIND_BUILTIN: result = syx_eval_builtin(ctx, &head->builtin, arguments); break;
    case SYXV_KIND_CLOSURE: result = syx_eval_closure(ctx, &head->closure, arguments); break;
    case SYXV_KIND_THROWN: UNREACHABLE("thrown object should not be called");
    case SYXV_KIND_RETURN_VALUE: UNREACHABLE("return value object can't be called");
  }
  rc_acquire(result);
  rc_release(head);
  return rc_move(result);
}

bool syx_eval_should_early_exit(SyxV *value, const void *items[], size_t count) {
  switch (value->kind) {
    case SYXV_KIND_RETURN_VALUE:
    case SYXV_KIND_THROWN: {
      rc_acquire(value);
      rc__release_all(items, count);
      rc_move(value);
      return true;
    } break;
    default: return false;
  }
}

bool syx_eval_report_error(Syx_Eval_Ctx *ctx, SyxV *value) {
  if (value->kind != SYXV_KIND_THROWN) return true;
  fprintf(stderr, "Unhandled exception: ");
  SyxV *reason = rc_acquire(syx_convert_to_string(ctx, value->thrown.reason));
  if (reason->kind == SYXV_KIND_STRING) fprintf(stderr, SV_Fmt "\n", SV_Arg(reason->string));
  else fprintf(stderr, "unknown\n");
  rc_release(reason);
  if (value->thrown.stack_frame) {
    syx_string_t frames_stack = stringify_call_stack(ctx, value->thrown.stack_frame);
    io_puts_n(stderr, frames_stack.items, frames_stack.count);
    sb_free(frames_stack);
  }
  return false;
}

SyxV *syx__eval_forms_list_opt(Syx_Eval_Ctx *ctx, SyxV *forms_list, Syx_Eval_Forms_List_Opt opt) {
  SyxV *result = NULL;
  if (opt.default_result) result = rc_acquire(opt.default_result);
  syxv_list_for_each(form, forms_list) {
    if (result) rc_release(result);
    result = syx_eval(ctx, form);
    syx_eval_early_exit(result);
    rc_acquire(result);
    if (opt.should_stop != NULL) {
      bool should_stop = {0};
      syx_convert_to(ctx, opt.should_stop(ctx, result), &should_stop, result);
      if (should_stop) return rc_move(result);
    }
  }
  if (result == NULL) RUNTIME_ERROR(ctx, "empty forms list");
  return rc_move(result);
}

SyxV *syx_convert_to_bool(Syx_Eval_Ctx *ctx, SyxV *value) {
  switch (value->kind) {
    case SYXV_KIND_NIL: return make_syxv_bool(false);
    case SYXV_KIND_SYMBOL: return make_syxv_bool(true);
    case SYXV_KIND_PAIR: return make_syxv_bool(true);
    case SYXV_KIND_BOOL: return value;
    case SYXV_KIND_INTEGER: return make_syxv_bool((syx_bool_t)value->integer);
    case SYXV_KIND_FRACTIONAL: return make_syxv_bool((syx_bool_t)value->fractional);
    case SYXV_KIND_STRING: return make_syxv_bool((bool)value->string.count);
    case SYXV_KIND_QUOTE: return syx_convert_to_bool(ctx, value->quote);
    case SYXV_KIND_SPECIALF: return make_syxv_bool(true);
    case SYXV_KIND_BUILTIN: return make_syxv_bool(true);
    case SYXV_KIND_CLOSURE: return make_syxv_bool(true);
    case SYXV_KIND_THROWN: RUNTIME_ERROR(ctx, "thrown object can't be converted");
    case SYXV_KIND_RETURN_VALUE: RUNTIME_ERROR(ctx, "return value object can't be converted");
  }
}

SyxV *syx_convert_to_integer(Syx_Eval_Ctx *ctx, SyxV *value) {
  switch (value->kind) {
    case SYXV_KIND_NIL: return make_syxv_integer(0);
    case SYXV_KIND_SYMBOL: RUNTIME_ERROR(ctx, "illegal conversion of symbol to integer number");
    case SYXV_KIND_PAIR: RUNTIME_ERROR(ctx, "illegal conversion of pair to integer number");
    case SYXV_KIND_BOOL: return make_syxv_integer(value->boolean ? 1 : 0);
    case SYXV_KIND_INTEGER: return value;
    case SYXV_KIND_FRACTIONAL: return make_syxv_integer((syx_integer_t)value->fractional);
    case SYXV_KIND_STRING: {
      String_View sv = value->string;
      syx_integer_t result = 0;
      if (!parse_integer(&sv, &result)) RUNTIME_ERROR(ctx, "illegal conversion of string to integer number");
      return make_syxv_integer(result);
    }
    case SYXV_KIND_QUOTE: return syx_convert_to_integer(ctx, value->quote);
    case SYXV_KIND_SPECIALF: RUNTIME_ERROR(ctx, "illegal conversion of special form to integer number");
    case SYXV_KIND_BUILTIN: RUNTIME_ERROR(ctx, "illegal conversion of builtin function to integer number");
    case SYXV_KIND_CLOSURE: RUNTIME_ERROR(ctx, "illegal conversion of closure to integer number");
    case SYXV_KIND_THROWN: RUNTIME_ERROR(ctx, "thrown object can't be converted");
    case SYXV_KIND_RETURN_VALUE: RUNTIME_ERROR(ctx, "return value object can't be converted");
  }
}

SyxV *syx_convert_to_fractional(Syx_Eval_Ctx *ctx, SyxV *value) {
  switch (value->kind) {
    case SYXV_KIND_NIL: return make_syxv_fractional(0.0);
    case SYXV_KIND_SYMBOL: RUNTIME_ERROR(ctx, "illegal conversion of symbol to fractional number");
    case SYXV_KIND_PAIR: RUNTIME_ERROR(ctx, "illegal conversion of pair to fractional number");
    case SYXV_KIND_BOOL: return make_syxv_fractional(value->boolean ? 1.0 : 0.0);
    case SYXV_KIND_INTEGER: return make_syxv_fractional((syx_fractional_t)value->integer);
    case SYXV_KIND_FRACTIONAL: return value;
    case SYXV_KIND_STRING: {
      String_View sv = value->string;
      syx_fractional_t result = 0;
      if (!parse_fractional(&sv, &result)) RUNTIME_ERROR(ctx, "illegal conversion of string to fractional number");
      return make_syxv_fractional(result);
    }
    case SYXV_KIND_QUOTE: return syx_convert_to_fractional(ctx, value->quote);
    case SYXV_KIND_SPECIALF: RUNTIME_ERROR(ctx, "illegal conversion of special form to fractional number");
    case SYXV_KIND_BUILTIN: RUNTIME_ERROR(ctx, "illegal conversion of builtin function to fractional number");
    case SYXV_KIND_CLOSURE: RUNTIME_ERROR(ctx, "illegal conversion of closure to fractional number");
    case SYXV_KIND_THROWN: RUNTIME_ERROR(ctx, "thrown object can't be converted");
    case SYXV_KIND_RETURN_VALUE: RUNTIME_ERROR(ctx, "return value object can't be converted");
  }
}

SyxV *syx_convert_to_string(Syx_Eval_Ctx *ctx, SyxV *value) {
  switch (value->kind) {
    case SYXV_KIND_NIL: return make_syxv_string_cstr("#nil");
    case SYXV_KIND_SYMBOL: RUNTIME_ERROR(ctx, "illegal conversion of symbol to string");
    case SYXV_KIND_PAIR: return make_syxv_string_managed_cstr(stringify_syxv(value));
    case SYXV_KIND_BOOL: return make_syxv_string_cstr(value->boolean ? "#true" : "#false");
    case SYXV_KIND_INTEGER: return make_syxv_string_managed_cstr(stringify_integer(value->integer));
    case SYXV_KIND_FRACTIONAL: return make_syxv_string_managed_cstr(stringify_fractional(value->fractional));
    case SYXV_KIND_STRING: return value;
    case SYXV_KIND_QUOTE: return syx_convert_to_string(ctx, value->quote);
    case SYXV_KIND_SPECIALF: RUNTIME_ERROR(ctx, "illegal conversion of special form to string");
    case SYXV_KIND_BUILTIN: RUNTIME_ERROR(ctx, "illegal conversion of builtin function to string");
    case SYXV_KIND_CLOSURE: RUNTIME_ERROR(ctx, "illegal conversion of closure to string");
    case SYXV_KIND_THROWN: RUNTIME_ERROR(ctx, "thrown object can't be converted");
    case SYXV_KIND_RETURN_VALUE: RUNTIME_ERROR(ctx, "return value object can't be converted");
  }
}

void fprint_syx_env_opt(FILE *f, Syx_Env *env, Print_Syx_Env_Opt opt) {
  if (opt.indent) fprintf(f, "%*c", opt.indent, ' ');
  fprintf(f, "Env: %s\n", env->description ? env->description : "NULL");
  ht_foreach(syxv, &env->symbols) {
    if (opt.indent) fprintf(f, "%*c", opt.indent, ' ');
    fprintf(f, "  \"%s\": ", ht_key(&env->symbols, syxv)->name);
    fprint_syxv(f, *syxv);
    fprintf(f, "\n");
  }
  if (env->parent) {
    if (opt.deep == PRINT_SYX_ENV_DEEP_ALL_WITH_GLOBAL) {
      fprint_syx_env_opt(f, env->parent, opt);
    } else if (opt.deep == PRINT_SYX_ENV_DEEP_ALL && env->parent->parent) {
      fprint_syx_env_opt(f, env->parent, opt);
    } else if (opt.deep > PRINT_SYX_ENV_DEEP_CURRENT_ONLY) {
      fprint_syx_env_opt(f, env->parent, (Print_Syx_Env_Opt){.deep = opt.deep - 1, .indent = opt.indent});
    }
  }
}

#endif // SYX_EVAL_IMPL
