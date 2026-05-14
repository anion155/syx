#ifndef SYX_FUNCTION_TYPE_INFO_H
#define SYX_FUNCTION_TYPE_INFO_H

#include "syx_type_info.h"

struct Syx_Function_Type_Info {
  size_t size;
  Syx_Type_Info *return_type;
  size_t argc;
  Syx_Type_Info **argv;
  bool vaargs;
};

#endif // SYX_FUNCTION_TYPE_INFO_H

#if defined(SYX_FUNCTION_TYPE_INFO_IMPL) && !defined(SYX_FUNCTION_TYPE_INFO_IMPL_C)
#define SYX_FUNCTION_TYPE_INFO_IMPL_C

#define SYX_VALUE_IMPL
#include "syx_value.h"
#define SYX_EVAL_IMPL
#include "syx_eval.h"

void syx_function_type_info_rc_count_circular(RC_Circulars *circulars, Syx_Function_Type_Info *function, void *parent_type) {
  rc_count_circular(syx_type_info_rc_count_circular, circulars, &function->return_type, parent_type);
  for (size_t index = 0; index < function->argc; index += 1) {
    rc_count_circular(syx_type_info_rc_count_circular, circulars, &function->argv[index], parent_type);
  }
}

void syx_function_type_info_destructor(void *data) {
  Syx_Function_Type_Info *function = data;
  if (function->return_type) rc_release_circular(syx_type_info_rc_count_circular, function->return_type);
  for (size_t index = 0; index < function->argc; index += 1) {
    if (function->argv[index]) rc_release_circular(syx_type_info_rc_count_circular, function->argv[index]);
  }
}

Syx_Function_Type_Info *make_syx_function_type_info_opt(Syx_Function_Type_Info opt) {
  Syx_Function_Type_Info *function = rc_alloc(sizeof(Syx_Function_Type_Info), syx_function_type_info_destructor);
  memset(function, 0, sizeof(Syx_Function_Type_Info));
  *function = opt;
  rc_acquire(function->return_type);
  for (size_t index = 0; index < function->argc; index += 1) {
    rc_acquire(function->argv[index]);
  }
  for (size_t index = 0; index < function->argc; index += 1) rc_acquire(function->argv[index]);
  return function;
}

#endif // SYX_FUNCTION_TYPE_INFO_IMPL
