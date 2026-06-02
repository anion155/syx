#ifndef SYX_GLOBAL_ENV_H
#define SYX_GLOBAL_ENV_H

#include <ht.h>
#include <magic.h>
#include <nob.h>
#include <rc.h>

#include "syx_eval.h"

Syx_Env *make_global_syx_env();

#endif // SYX_GLOBAL_ENV_H

#if defined(SYX_GLOBAL_ENV_IMPL) && !defined(SYX_GLOBAL_ENV_IMPL_C)
#define SYX_GLOBAL_ENV_IMPL_C

#define SYX_EVAL_SPECIALF_IMPL
#include "syx_eval_specialf.h"
#define SYX_EVAL_BUILTINS_IMPL
#include "syx_eval_builtins.h"
#define SYX_VECTOR_IMPL
#include "syx_vector.h"
#define SYX_TEST_VECTOR_IMPL
#include "syx_test_vector.h"

Syx_Env *make_global_syx_env() {
  Syx_Env *env = make_syx_env(NULL, "<global>");
  syx_env_define_special_forms(env);
  syx_env_define_builtins(env);
  syx_env_define_vector(env);
  syx_env_define_test_vector(env);
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

#endif // SYX_GLOBAL_ENV_IMPL
