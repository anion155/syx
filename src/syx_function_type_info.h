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

void syx_function_type_info_destructor(void *data) {
  Syx_Function_Type_Info *function = data;
  if (function->return_type) rc_release(function->return_type);
  for (size_t index = 0; index < function->argc; index += 1) {
    if (function->argv[index]) rc_release(function->argv[index]);
  }
}

void syx_function_type_info_graph_visitor(Rc_Circulars *circulars, const void *data, const void *source) {
  const Syx_Function_Type_Info *function = data;
  rc_graph_visitor(circulars, (void **)&function->return_type, source);
  for (size_t index = 0; index < function->argc; index += 1) {
    rc_graph_visitor(circulars, (void **)&function->argv[index], source);
  }
}

Syx_Function_Type_Info *make_syx_function_type_info_opt(Syx_Function_Type_Info opt) {
  Syx_Function_Type_Info *function = rc_malloc(sizeof(Syx_Function_Type_Info), .destructor = syx_function_type_info_destructor, .graph_visitor = syx_function_type_info_graph_visitor);
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
