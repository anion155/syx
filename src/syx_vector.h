#ifndef SYX_VECTOR_H
#define SYX_VECTOR_H

#include "syx_value.h"

void syx_env_define_vector(Syx_Env *env);

#endif // SYX_VECTOR_H

#if defined(SYX_VECTOR_IMPL) && !defined(SYX_VECTOR_IMPL_C)
#define SYX_VECTOR_IMPL_C

void syxv_vector_zero_init_rest(SyxV_Vector *vector) {
  if (vector->capacity > vector->count) {
    memset(vector->items + vector->count, 0, sizeof(vector->items[0]) * (vector->capacity - vector->count));
  }
}

SyxV *syxv_vector_constructor(Syx_Eval_Ctx *ctx, void *data, SyxV *arguments) {
  SyxV_Vector *vector = data;
  size_t size = 0;
  SyxV *last_item;
  syxv_list_for_each(item, arguments, &last_item) size += 1;
  if (last_item->kind != SYXV_KIND_NIL) RUNTIME_ERROR(ctx, "vector expects list as arguments");
  da_realloc_capacity(vector, size);
  syxv_list_for_each(item, arguments) da_append(vector, rc_acquire(item));
  syxv_vector_zero_init_rest(vector);
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
  rc_acquire(argument);
  if (vector->items[index]) rc_release(vector->items[index]);
  vector->items[index] = argument;
  return NULL;
}

SyxV *syxv_vector_count_getter(Syx_Eval_Ctx *ctx, void *data) {
  UNUSED(ctx);
  SyxV_Vector *vector = data;
  return make_syxv_number_integer(vector->count);
}

SyxV *syxv_vector_count_setter(Syx_Eval_Ctx *ctx, void *data, SyxV *argument) {
  SyxV_Vector *vector = data;
  Syx_Number number = {0};
  syx_convert_to(ctx, argument, &number);
  size_t new_count = syx_number_integer_value(number);
  if (new_count > vector->count) {
    da_realloc_capacity(vector, new_count);
    syxv_vector_zero_init_rest(vector);
    vector->count = new_count;
  } else if (new_count < vector->count) {
    for (size_t index = new_count; index < vector->count; index += 1) {
      if (vector->items[index]) rc_release(vector->items[index]);
    }
    vector->count = new_count;
  }
  return NULL;
}

SyxV *syxv_vector_append(Syx_Eval_Ctx *ctx, void *data, SyxV *arguments) {
  SyxV_Vector *vector = data;
  SyxV *last_argument = NULL;
  syxv_list_for_each(argument, arguments, &last_argument) {
    da_append(vector, rc_acquire(argument));
  }
  if (last_argument->kind != SYXV_KIND_NIL) RUNTIME_ERROR(ctx, "list expected as arguments");
  return make_syxv_number_integer(vector->count);
}

void syx_env_define_vector(Syx_Env *env) {
  // clang-format off
  syx_env_define_cstr(env, "vector", make_syxv_constructor(make_syx_type_info_opt((Syx_Type_Info){
    .size = sizeof(SyxV_Vector),
    .symbol = (&make_syxv_symbol_cstr("vector")->symbol),
    .kind = SYX_TYPE_INFO_KIND_STRUCTURE,
    .structure = {
      .constructor = syxv_vector_constructor,
      .index_getter = syxv_vector_getter,
      .index_setter = syxv_vector_setter,
      .fields = make_syx_type_info_structure_fields(
        // {"items", {.kind = SYX_TYPE_INFO_STRUCTURE_FIELD_KIND_DATA, .data = {
        //   .typeinfo = make_syx_type_info_opt((Syx_Type_Info){
        //     .kind = SYX_TYPE_INFO_KIND_PTR,
        //     .ptr = make_syx_type_info_opt((Syx_Type_Info){
        //       .symbol = (&make_syxv_symbol_cstr("vector")->symbol),
        //       .kind = SYX_TYPE_INFO_KIND_STRUCTURE,
        //       .structure = {},
        //     }),
        //   }),
        //   .readonly = true,
        // }}},
        // {"count", {.kind = SYX_TYPE_INFO_STRUCTURE_FIELD_KIND_DATA, .data = {
        //   .offset = offsetof(SyxV_Vector, count),
        //   .typeinfo = make_syx_type_info(.kind = SYX_TYPE_INFO_KIND_SIZE),
        //   .readonly = true,
        // }}},
        {"count", {.kind = SYX_TYPE_INFO_STRUCTURE_FIELD_KIND_ACCESSOR, .accessor = {
          .getter = syxv_vector_count_getter,
          .setter = syxv_vector_count_setter,
        }}},
        {"capacity", {.kind = SYX_TYPE_INFO_STRUCTURE_FIELD_KIND_DATA, .data = {
          .offset = offsetof(SyxV_Vector, capacity),
          .typeinfo = make_syx_type_info(.kind = SYX_TYPE_INFO_KIND_SIZE),
          .readonly = true,
        }}},
        {"append", {.kind = SYX_TYPE_INFO_STRUCTURE_FIELD_KIND_METHOD, .method = syxv_vector_append}},
      ),
      .destructor = da_destructor
    },
  })));
  // clang-format on
}

#endif // SYX_VECTOR_IMPL
