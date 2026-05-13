#ifndef SYX_STRUCTURE_H
#define SYX_STRUCTURE_H

#include "syx_value.h"

typedef SyxV *(*Syx_Structure_Type_Info_Constructor)(Syx_Eval_Ctx *ctx, void *data, SyxV *arguments);
typedef SyxV *(*Syx_Structure_Type_Info_Index_Getter)(Syx_Eval_Ctx *ctx, void *data, syx_integer_t index);
typedef SyxV *(*Syx_Structure_Type_Info_Index_Setter)(Syx_Eval_Ctx *ctx, void *data, syx_integer_t index, SyxV *argument);
typedef void (*Syx_Structure_Type_Info_Desstructor)(void *data);

struct Syx_Structure_Type_Info {
  const char *id;
  SyxV_Symbol *symbol;
  size_t size;
  Syx_Structure_Type_Info_Constructor constructor;
  Syx_Structure_Type_Info_Index_Getter index_getter;
  Syx_Structure_Type_Info_Index_Setter index_setter;
  Syx_Structure_Type_Info_Desstructor destructor;
};

Syx_Structure_Type_Info *make_syxv_structure_info_opt(Syx_Structure_Type_Info opt);
#define make_syxv_structure_info(...) make_syxv_structure_info_opt((Syx_Structure_Type_Info){__VA_ARGS__})

SyxV *syxv_eval_instantiate_structure(Syx_Eval_Ctx *ctx, Syx_Constructor *constructor, SyxV *arguments);
SyxV *syxv_eval_structure(Syx_Eval_Ctx *ctx, SyxV_Structure *structure, SyxV *arguments);

#endif // SYX_STRUCTURE_H

#if defined(SYX_STRUCTURE_IMPL) && !defined(SYX_STRUCTURE_IMPL_C)
#define SYX_STRUCTURE_IMPL_C

#define SYX_VALUE_IMPL
#include "syx_value.h"
#define SYX_EVAL_IMPL
#include "syx_eval.h"

void syx_structure_type_info_destructor(void *data) {
  Syx_Structure_Type_Info *typeinfo = data;
  free((char *)typeinfo->id);
  rc_release(typeinfo->symbol);
}

Syx_Structure_Type_Info *make_syxv_structure_info_opt(Syx_Structure_Type_Info opt) {
  Syx_Structure_Type_Info *info = rc_alloc(sizeof(Syx_Structure_Type_Info), syx_structure_type_info_destructor);
  memset(info, 0, sizeof(Syx_Structure_Type_Info));
  *info = opt;
  if (!opt.id) info->id = nanoid("info", 20);
  rc_acquire(get_syxv_from_symbol(opt.symbol));
  return info;
}

SyxV *syxv_eval_instantiate_structure(Syx_Eval_Ctx *ctx, Syx_Constructor *constructor, SyxV *arguments) {
  void *data = malloc(constructor->typeinfo->size);
  if (!data) RUNTIME_ERROR(ctx, "failed to allocate structure");
  memset(data, 0, constructor->typeinfo->size);
  SyxV *structure = make_syxv_structure(constructor->typeinfo, data);
  if (constructor->typeinfo->constructor) {
    syx_ctx_push_frame(ctx, (SyxV *)((char *)constructor - offsetof(SyxV, constructor)));
    SyxV *result = constructor->typeinfo->constructor(ctx, data, arguments);
    syx_ctx_pop_frame(ctx);
    syx_eval_early_exit(result, structure);
    if (result) rc_release(result);
  }
  return structure;
}

SyxV *syxv_eval_structure(Syx_Eval_Ctx *ctx, SyxV_Structure *structure, SyxV *arguments) {
  SyxV *field = syx_eval(ctx, syxv_list_next(&arguments));
  syx_eval_early_exit(field);
  rc_acquire(field);
  if (field->kind == SYXV_KIND_NUMBER && field->number.kind == SYX_NUMBER_KIND_INTEGER) {
    syx_integer_t index = field->number.integer;
    rc_release(field);
    // SyxV *argument = syxv_list_next_nullable(&arguments);
    if (arguments->kind == SYXV_KIND_NIL) {
      if (!structure->typeinfo->index_getter) RUNTIME_ERROR(ctx, "structure does not implement index getter");
      syx_ctx_push_frame(ctx, NULL); // TODO index getter as callable
      SyxV *result = rc_acquire(structure->typeinfo->index_getter(ctx, structure->data, index));
      syx_ctx_pop_frame(ctx);
      return rc_move(result);
    } else {
      if (!structure->typeinfo->index_setter) RUNTIME_ERROR(ctx, "structure does not implement index setter");
      syx_ctx_push_frame(ctx, NULL); // TODO index setter as callable
      SyxV *result = structure->typeinfo->index_setter(ctx, structure->data, index, arguments->pair.left);
      if (!result) result = rc_acquire(make_syxv_nil());
      else rc_acquire(result);
      syx_ctx_pop_frame(ctx);
      return rc_move(result);
    }
  } else if (field->kind == SYXV_KIND_SYMBOL) {
    TODO("methods and named fields are not implemented yet");
  }
  rc_release(field);
  return make_syxv_nil();
}

#endif // SYX_STRUCTURE_IMPL
