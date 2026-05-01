#ifndef SYX_EVAL_CONSTANTS_H
#define SYX_EVAL_CONSTANTS_H

#include "syx_eval.h"

void syx_env_define_constants(Syx_Env *env);

#endif // SYX_EVAL_CONSTANTS_H

#if defined(SYX_EVAL_CONSTANTS_IMPL) && !defined(SYX_EVAL_CONSTANTS_IMPL_C)
#define SYX_EVAL_CONSTANTS_IMPL_C

#define NANOID_IMPL
#include <nanoid.h>

void syx_env_define_constants(Syx_Env *env) {
  syx_env_define_cstr(env, "nil", make_syxv_nil());
  syx_env_define_cstr(env, "null", make_syxv_nil());
  syx_env_define_cstr(env, "true", make_syxv_bool(true));
  syx_env_define_cstr(env, "#t", make_syxv_bool(true));
  syx_env_define_cstr(env, "false", make_syxv_bool(false));
  syx_env_define_cstr(env, "#f", make_syxv_bool(false));
}

#endif // SYX_EVAL_CONSTANTS_IMPL
