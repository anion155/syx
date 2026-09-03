#ifndef SYX_GLOBAL_ENV_H
#define SYX_GLOBAL_ENV_H

#include <ht.h>
#include <magic.h>
#include <nob.h>
#include <rc.h>
#include <syx_new/syx_eval.h>

Syx_Env *make_global_syx_env();
Syx_Eval_Ctx *make_global_syx_eval_ctx();

#endif // SYX_GLOBAL_ENV_H

#if defined(SYX_GLOBAL_ENV_IMPL) && !defined(SYX_GLOBAL_ENV_IMPL_C)
#define SYX_GLOBAL_ENV_IMPL_C

#define SYX_EVAL_IMPL
#include <syx_new/syx_eval.h>
#define SYX_EVAL_SPECIALF_IMPL
#include <syx_new/syx_eval_specialf.h>
#define SYX_EVAL_BUILTINS_IMPL
#include <syx_new/syx_eval_builtins.h>

Syx_Env *make_global_syx_env() {
  Syx_Env *env = make_syx_env(make_syx_value_symbol_strlit("<builtins-global>"), NULL);
  syx_env_define_special_forms(env);
  syx_env_define_builtins(env);
  // syx_env_define_boxed(env);
  // syx_env_define_vector(env);
  // syx_env_define_test_vector(env);
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

Syx_Eval_Ctx *make_global_syx_eval_ctx() {
  return make_syx_eval_ctx((Syx_Eval_Ctx){
      .frames_stack = make_syx_frames_stack(),
      .global_env = make_global_syx_env(),
      .env = make_syx_env(make_syx_value_symbol_strlit("<global>"), NULL),
  });
}

#endif // SYX_GLOBAL_ENV_IMPL
