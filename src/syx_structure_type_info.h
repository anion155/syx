#ifndef SYX_STRUCTURE_TYPE_INFO_H
#define SYX_STRUCTURE_TYPE_INFO_H

#include <limits.h>
#include <stdint.h>

#include "syx_type_info.h"
#include "syx_value.h"

typedef SyxV *(*Syx_Structure_Type_Info_Constructor)(Syx_Eval_Ctx *ctx, void *data, SyxV *arguments);
typedef SyxV *(*Syx_Structure_Type_Info_Index_Getter)(Syx_Eval_Ctx *ctx, void *data, syx_integer_t index);
typedef SyxV *(*Syx_Structure_Type_Info_Index_Setter)(Syx_Eval_Ctx *ctx, void *data, syx_integer_t index, SyxV *argument);
typedef void (*Syx_Structure_Type_Info_Desstructor)(void *data);

typedef struct Syx_Structure_Type_Info_Field {
  size_t offset;
  Syx_Type_Info *typeinfo;
} Syx_Structure_Type_Info_Field;

typedef Ht(const char *, Syx_Structure_Type_Info_Field, Syx_Structure_Type_Info_Fields) Syx_Structure_Type_Info_Fields;

struct Syx_Structure_Type_Info {
  size_t size;
  SyxV_Symbol *symbol;
  Syx_Structure_Type_Info_Constructor constructor;
  Syx_Structure_Type_Info_Index_Getter index_getter;
  Syx_Structure_Type_Info_Index_Setter index_setter;
  Syx_Structure_Type_Info_Fields fields;
  Syx_Structure_Type_Info_Desstructor destructor;
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

void syx_structure_type_info_rc_count_circular(RC_Circulars *circulars, Syx_Structure_Type_Info *structure, void *parent_type) {
  ht_foreach(field, &structure->fields) {
    rc_count_circular(syx_type_info_rc_count_circular, circulars, &field->typeinfo, parent_type);
  }
}

void syx_structure_type_info_destructor(void *data) {
  Syx_Structure_Type_Info *typeinfo = data;
  rc_release(typeinfo->symbol);
  ht_foreach(field, &typeinfo->fields) {
    if (field) rc_release_circular(syx_type_info_rc_count_circular, field->typeinfo);
    free((char *)ht_key(&typeinfo->fields, field));
  }
  ht_free(&typeinfo->fields);
}

Syx_Structure_Type_Info *make_syx_structure_type_info_opt(Syx_Structure_Type_Info opt) {
  Syx_Structure_Type_Info *info = rc_alloc(sizeof(Syx_Structure_Type_Info), syx_structure_type_info_destructor);
  memset(info, 0, sizeof(Syx_Structure_Type_Info));
  *info = opt;
  rc_acquire(get_syxv_from_symbol(opt.symbol));
  return info;
}

SyxV *syxv_eval_instantiate_structure(Syx_Eval_Ctx *ctx, Syx_Constructor *constructor, SyxV *arguments) {
  void *data = malloc(constructor->typeinfo->size);
  if (!data) RUNTIME_ERROR(ctx, "failed to allocate structure");
  memset(data, 0, constructor->typeinfo->size);
  SyxV *structure = make_syxv_structure(constructor->typeinfo, data);
  if (constructor->typeinfo->constructor) {
    syx_ctx_push_frame(ctx, constructor->typeinfo->symbol->name);
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
    const char *function_name = strdup(temp_sprintf("(#.%s %lld)", structure->typeinfo->symbol->name, index));
    syx_ctx_push_frame(ctx, function_name);
    SyxV *result = rc_acquire(structure->typeinfo->index_getter(ctx, structure->data, index));
    syx_ctx_pop_frame(ctx);
    return rc_move(result);
  } else {
    if (!structure->typeinfo->index_setter) RUNTIME_ERROR(ctx, "structure does not implement index setter");
    const char *function_name = strdup(temp_sprintf("(#.%s %lld <value>)", structure->typeinfo->symbol->name, index));
    syx_ctx_push_frame(ctx, function_name);
    SyxV *result = structure->typeinfo->index_setter(ctx, structure->data, index, arguments->pair.left);
    if (!result) result = rc_acquire(make_syxv_nil());
    else rc_acquire(result);
    syx_ctx_pop_frame(ctx);
    return rc_move(result);
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
