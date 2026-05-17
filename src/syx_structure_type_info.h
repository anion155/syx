#ifndef SYX_STRUCTURE_TYPE_INFO_H
#define SYX_STRUCTURE_TYPE_INFO_H

#include <limits.h>
#include <stdint.h>

#include "syx_type_info.h"
#include "syx_value.h"

typedef SyxV *(*Syx_Structure_Type_Info_Constructor)(Syx_Eval_Ctx *ctx, void *data, SyxV *arguments);
typedef SyxV *(*Syx_Structure_Type_Info_Index_Getter)(Syx_Eval_Ctx *ctx, void *data, syx_integer_t index);
typedef SyxV *(*Syx_Structure_Type_Info_Index_Setter)(Syx_Eval_Ctx *ctx, void *data, syx_integer_t index, SyxV *argument);
typedef void (*Syx_Structure_Type_Info_Destructor)(void *data);

typedef struct Syx_Structure_Type_Info_Field {
  Syx_Type_Info *typeinfo;
  size_t offset;
} Syx_Structure_Type_Info_Field;

typedef Ht(const char *, Syx_Structure_Type_Info_Field, Syx_Structure_Type_Info_Fields) Syx_Structure_Type_Info_Fields;

typedef struct Syx_Field_Pair {
  const char *key;
  Syx_Structure_Type_Info_Field field;
} Syx_Field_Pair;

Syx_Structure_Type_Info_Fields make_syx_structure_type_info_fields_opt(Syx_Field_Pair *pairs, size_t count);
#define make_syx_structure_type_info_fields(...) \
  make_syx_structure_type_info_fields_opt(       \
      (Syx_Field_Pair[]){__VA_ARGS__},           \
      sizeof((Syx_Field_Pair[]){__VA_ARGS__}) / sizeof(Syx_Field_Pair))

struct Syx_Structure_Type_Info {
  size_t size;
  SyxV_Symbol *symbol;
  Syx_Structure_Type_Info_Constructor constructor;
  Syx_Structure_Type_Info_Index_Getter index_getter;
  Syx_Structure_Type_Info_Index_Setter index_setter;
  Syx_Structure_Type_Info_Fields fields;
  Syx_Structure_Type_Info_Destructor destructor;
};

SyxV *syxv_eval_instantiate_structure(Syx_Eval_Ctx *ctx, Syx_Constructor *constructor, SyxV *arguments);
SyxV *syxv_eval_structure(Syx_Eval_Ctx *ctx, SyxV_Structure *structure, SyxV *arguments);

#endif // SYX_STRUCTURE_TYPE_INFO_H

#if defined(SYX_STRUCTURE_TYPE_INFO_IMPL) && !defined(SYX_STRUCTURE_TYPE_INFO_IMPL_C)
#define SYX_STRUCTURE_TYPE_INFO_IMPL_C

#define SYX_VALUE_IMPL
#include "syx_value.h"
#define SYX_EVAL_IMPL
#include "syx_eval.h"

Syx_Structure_Type_Info_Fields make_syx_structure_type_info_fields_opt(Syx_Field_Pair *pairs, size_t count) {
  Syx_Structure_Type_Info_Fields fields = {0};
  fields.hasheq = ht_cstr_hasheq;
  size_t offset = 0;
  for (size_t index = 0; index < count; index += 1) {
    if (!pairs[index].field.offset) pairs[index].field.offset = offset;
    *ht_put(&fields, pairs[index].key) = pairs[index].field;
    offset += pairs[index].field.typeinfo->size;
  }
  return fields;
}

void syx_structure_type_info_destructor(void *data) {
  Syx_Structure_Type_Info *typeinfo = data;
  if (typeinfo->symbol) rc_release(get_syxv_from_symbol(typeinfo->symbol));
  ht_foreach(field, &typeinfo->fields) {
    if (field) rc_release(field->typeinfo);
    free((char *)ht_key(&typeinfo->fields, field));
  }
  ht_free(&typeinfo->fields);
}

void syx_structure_type_info_graph_visitor(Rc_Circulars *circulars, const void *data, const void *parent_type) {
  const Syx_Structure_Type_Info *structure = data;
  ht_foreach(field, &structure->fields) {
    rc_graph_visitor(circulars, (void **)&field->typeinfo, parent_type);
  }
}

Syx_Structure_Type_Info *make_syx_structure_type_info_opt(Syx_Structure_Type_Info opt) {
  Syx_Structure_Type_Info *info = rc_malloc(sizeof(Syx_Structure_Type_Info), .destructor = syx_structure_type_info_destructor);
  memset(info, 0, sizeof(Syx_Structure_Type_Info));
  *info = opt;
  if (info->symbol) rc_acquire(get_syxv_from_symbol(info->symbol));
  return info;
}

SyxV *syxv_eval_instantiate_structure(Syx_Eval_Ctx *ctx, Syx_Constructor *constructor, SyxV *arguments) {
  void *data = malloc(constructor->typeinfo->size);
  if (!data) RUNTIME_ERROR(ctx, "failed to allocate structure");
  memset(data, 0, constructor->typeinfo->size);
  SyxV *structure = make_syxv_structure(constructor->typeinfo, data);
  if (constructor->typeinfo->constructor) {
    if (constructor->typeinfo->symbol) syx_ctx_push_frame(ctx, constructor->typeinfo->symbol->name);
    else syx_ctx_push_frame(ctx, "constructor");
    SyxV *result = constructor->typeinfo->constructor(ctx, data, arguments);
    syx_ctx_pop_frame(ctx);
    syx_eval_early_exit(result, structure);
    if (result) rc_release(result);
  }
  return structure;
}

SyxV *syxv_eval_structure_indexed(Syx_Eval_Ctx *ctx, SyxV_Structure *structure, syx_integer_t index, SyxV *arguments) {
  if (arguments->kind == SYXV_KIND_NIL) {
    if (!structure->typeinfo->index_getter) RUNTIME_ERROR(ctx, "structure does not implement index getter");
  } else {
    if (!structure->typeinfo->index_setter) RUNTIME_ERROR(ctx, "structure does not implement index setter");
  }
  const char *function_name;
  if (structure->typeinfo->symbol) function_name = strdup(temp_sprintf("(#.%s %lld)", structure->typeinfo->symbol->name, index));
  else function_name = strdup(temp_sprintf("(#.<anonim> %lld)", index));
  syx_ctx_push_frame(ctx, function_name);
  SyxV *result;
  if (arguments->kind == SYXV_KIND_NIL) {
    result = structure->typeinfo->index_getter(ctx, structure->data, index);
  } else {
    result = structure->typeinfo->index_setter(ctx, structure->data, index, arguments->pair.left);
    if (!result) result = make_syxv_nil();
  }
  rc_acquire(result);
  syx_ctx_pop_frame(ctx);
  return rc_move(result);
}

void __gg__(Syx_Structure_Type_Info_Fields *fields) {
  ht_foreach(field, fields) {
    printf("'%s'[%zu]: %u\n", ht_key(fields, field), field->offset, field->typeinfo->kind);
  }
}

SyxV *syxv_eval_structure(Syx_Eval_Ctx *ctx, SyxV_Structure *structure, SyxV *arguments) {
  SyxV *field_name = syx_eval(ctx, syxv_list_next(&arguments));
  syx_eval_early_exit(field_name);
  rc_acquire(field_name);
  if (field_name->kind == SYXV_KIND_NUMBER && field_name->number.kind == SYX_NUMBER_KIND_INTEGER) {
    SyxV *result = rc_acquire(syxv_eval_structure_indexed(ctx, structure, field_name->number.integer, arguments));
    rc_release(field_name);
    return rc_move(result);
  } else if (field_name->kind == SYXV_KIND_SYMBOL) {
    Syx_Structure_Type_Info_Field *field = ht_find(&structure->typeinfo->fields, field_name->symbol.name);
    if (!field) RUNTIME_ERROR(ctx, temp_sprintf("structure does not have '%s' field", field_name->symbol.name));
    void *data = structure->data + field->offset;
    UNUSED(data);
    if (field->typeinfo->kind == SYX_TYPE_INFO_KIND_FUNCTION) TODO("implement method call");
    if (field->typeinfo->kind == SYX_TYPE_INFO_KIND_PTR && field->typeinfo->ptr->kind == SYX_TYPE_INFO_KIND_FUNCTION) TODO("implement method call");
    if (arguments->kind == SYXV_KIND_NIL) {
      TODO("implement field getter");
    } else {
      TODO("implement field setter");
    }
  }
  rc_release(field_name);
  return make_syxv_nil();
}

#endif // SYX_STRUCTURE_TYPE_INFO_IMPL
