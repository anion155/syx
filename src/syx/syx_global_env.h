#ifndef SYX_GLOBAL_ENV_H
#define SYX_GLOBAL_ENV_H

#include <defines.h>
#include <syx/syx_eval.h>

Syx_Env *make_global_syx_env();
Syx_Eval_Ctx *make_global_syx_eval_ctx(String_View cwd);

#endif // SYX_GLOBAL_ENV_H

#if defined(SYX_GLOBAL_ENV_IMPL) && !defined(SYX_GLOBAL_ENV_IMPL_C)
#define SYX_GLOBAL_ENV_IMPL_C

#define SYX_EVAL_SPECIALF_IMPL
#include <syx/syx_eval_specialf.h>
#define SYX_EVAL_BUILTINS_IMPL
#include <syx/syx_eval_builtins.h>

Syx_Env *make_global_syx_env() {
  Syx_Env *env = make_syx_env(syx_value_symbol_strlit("builtins")->symbol, NULL);
  syx_env_define_special_forms(env);
  syx_env_define_builtins(env);
  syx_env_define_types(env);
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

Syx_Eval_Ctx *make_global_syx_eval_ctx(String_View cwd) {
  return make_syx_eval_ctx((Syx_Eval_Ctx){
      .cwd = cwd,
      .frames_stack = make_syx_frames_stack(),
      .env = make_syx_env(syx_value_symbol_strlit("global")->symbol, make_global_syx_env()),
  });
}

#endif // SYX_GLOBAL_ENV_IMPL
