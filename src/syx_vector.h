#ifndef SYX_VECTOR_H
#define SYX_VECTOR_H

#include "syx_structure_type_info.h"
#include "syx_value.h"

void syx_env_define_vector(Syx_Env *env);
SyxV *syxv_vector_constructor(Syx_Eval_Ctx *ctx, void *data, SyxV *arguments);
SyxV *syxv_vector_getter(Syx_Eval_Ctx *ctx, void *data, syx_integer_t index);
SyxV *syxv_vector_setter(Syx_Eval_Ctx *ctx, void *data, syx_integer_t index, SyxV *argument);

#endif // SYX_VECTOR_H

#if defined(SYX_VECTOR_IMPL) && !defined(SYX_VECTOR_IMPL_C)
#define SYX_VECTOR_IMPL_C

#define SYX_STRUCTURE_TYPE_INFO_IMPL
#include "syx_structure_type_info.h"

void syx_env_define_vector(Syx_Env *env) {
  // clang-format off
  syx_env_define_cstr(env, "vector", make_syxv_constructor(make_syx_structure_type_info(
    .symbol = (&make_syxv_symbol_cstr("vector")->symbol),
    .size = sizeof(SyxV_Vector),
    .constructor = syxv_vector_constructor,
    .index_getter = syxv_vector_getter,
    .index_setter = syxv_vector_setter,
    .fields = make_syx_structure_type_info_fields(
      {"items", {.typeinfo = make_syx_type_info(.kind = SYX_TYPE_INFO_KIND_PTR, .ptr = make_syx_type_info(
        .kind = SYX_TYPE_INFO_KIND_STRUCTURE,
        .structure = make_syx_structure_type_info(.symbol = (&make_syxv_symbol_cstr("vector")->symbol))
      ))}},
      {"count", {.typeinfo = make_syx_type_info(.kind = SYX_TYPE_INFO_KIND_SIZE)}},
    ),
    .destructor = da_destructor)));
  // clang-format on
}

SyxV *syxv_vector_constructor(Syx_Eval_Ctx *ctx, void *data, SyxV *arguments) {
  SyxV_Vector *vector = data;
  SyxV *last_item;
  syxv_list_for_each(item, arguments, &last_item) {
    da_append(vector, rc_acquire(item));
  }
  if (last_item->kind != SYXV_KIND_NIL) RUNTIME_ERROR(ctx, "vector expects list as arguments");
  return NULL;
}

SyxV *syxv_vector_getter(Syx_Eval_Ctx *ctx, void *data, syx_integer_t index) {
  SyxV_Vector *vector = data;
  if ((syx_integer_t)vector->count <= index) RUNTIME_ERROR(ctx, "index out of bounds");
  if (-(syx_integer_t)vector->count > index) RUNTIME_ERROR(ctx, "index out of bounds");
  if (index < 0) index = vector->count + index;
  return vector->items[index];
}

SyxV *syxv_vector_setter(Syx_Eval_Ctx *ctx, void *data, syx_integer_t index, SyxV *argument) {
  SyxV_Vector *vector = data;
  if ((syx_integer_t)vector->count <= index) RUNTIME_ERROR(ctx, "index out of bounds");
  if (-(syx_integer_t)vector->count > index) RUNTIME_ERROR(ctx, "index out of bounds");
  if (index < 0) index = vector->count + index;
  if (vector->items[index]) rc_release(vector->items[index]);
  vector->items[index] = rc_acquire(argument);
  return NULL;
}

#endif // SYX_VECTOR_IMPL
