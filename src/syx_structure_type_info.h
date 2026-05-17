#ifndef SYX_STRUCTURE_TYPE_INFO_H
#define SYX_STRUCTURE_TYPE_INFO_H

#include <limits.h>
#include <stdint.h>

#include "syx_type_info.h"
#include "syx_type_info_utils.h"
#include "syx_value.h"

typedef struct Syx_Structure_Type_Info_Data_Field {
  Syx_Type_Info *typeinfo;
  size_t offset;
  bool readonly;
} Syx_Structure_Type_Info_Data_Field;

typedef SyxV *(*Syx_Structure_Type_Info_Accessor_Getter)(Syx_Eval_Ctx *ctx, void *data);
typedef SyxV *(*Syx_Structure_Type_Info_Accessor_Setter)(Syx_Eval_Ctx *ctx, void *data, SyxV *argument);

typedef struct Syx_Structure_Type_Info_Accessor_Field {
  Syx_Structure_Type_Info_Accessor_Getter getter;
  Syx_Structure_Type_Info_Accessor_Setter setter;
} Syx_Structure_Type_Info_Accessor_Field;

typedef SyxV *(*Syx_Structure_Type_Info_Method)(Syx_Eval_Ctx *ctx, void *data, SyxV *arguments);

typedef enum Syx_Structure_Type_Info_Field_Kind {
  SYX_STRUCTURE_TYPE_INFO_FIELD_KIND_DATA,
  SYX_STRUCTURE_TYPE_INFO_FIELD_KIND_ACCESSOR,
} Syx_Structure_Type_Info_Field_Kind;

typedef struct Syx_Structure_Type_Info_Field {
  Syx_Structure_Type_Info_Field_Kind kind;

  union {
    Syx_Structure_Type_Info_Data_Field data;
    Syx_Structure_Type_Info_Accessor_Field accessor;
  };
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

typedef SyxV *(*Syx_Structure_Type_Info_Constructor)(Syx_Eval_Ctx *ctx, void *data, SyxV *arguments);
typedef SyxV *(*Syx_Structure_Type_Info_Index_Getter)(Syx_Eval_Ctx *ctx, void *data, syx_integer_t index);
typedef SyxV *(*Syx_Structure_Type_Info_Index_Setter)(Syx_Eval_Ctx *ctx, void *data, syx_integer_t index, SyxV *argument);
typedef SyxV *(*Syx_Structure_Type_Info_Field_Getter)(Syx_Eval_Ctx *ctx, void *data, const char *field_name);
typedef SyxV *(*Syx_Structure_Type_Info_Field_Setter)(Syx_Eval_Ctx *ctx, void *data, const char *field_name, SyxV *argument);
typedef void (*Syx_Structure_Type_Info_Destructor)(void *data);

struct Syx_Structure_Type_Info {
  size_t size;
  SyxV_Symbol *symbol;
  Syx_Structure_Type_Info_Constructor constructor;
  Syx_Structure_Type_Info_Index_Getter index_getter;
  Syx_Structure_Type_Info_Index_Setter index_setter;
  Syx_Structure_Type_Info_Fields fields;
  Syx_Structure_Type_Info_Field_Getter field_getter;
  Syx_Structure_Type_Info_Field_Setter field_setter;
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
#define SYX_TYPE_INFO_UTILS_IMPL
#include "syx_type_info_utils.h"

Syx_Structure_Type_Info_Fields make_syx_structure_type_info_fields_opt(Syx_Field_Pair *pairs, size_t count) {
  Syx_Structure_Type_Info_Fields fields = {0};
  fields.hasheq = ht_cstr_hasheq;
  size_t offset = 0;
  for (size_t index = 0; index < count; index += 1) {
    Syx_Structure_Type_Info_Field field = pairs[index].field;
    if (field.kind == SYX_STRUCTURE_TYPE_INFO_FIELD_KIND_DATA && !field.data.offset) field.data.offset = offset;
    char *key = strdup(pairs[index].key);
    *ht_put(&fields, key) = field;
    if (field.kind == SYX_STRUCTURE_TYPE_INFO_FIELD_KIND_DATA && field.data.typeinfo) offset = field.data.offset + field.data.typeinfo->size;
  }
  return fields;
}

void syx_structure_type_info_destructor(void *data) {
  Syx_Structure_Type_Info *typeinfo = data;
  if (typeinfo->symbol) rc_release(get_syxv_from_symbol(typeinfo->symbol));
  ht_foreach(field, &typeinfo->fields) {
    switch (field->kind) {
      case SYX_STRUCTURE_TYPE_INFO_FIELD_KIND_DATA: {
        if (field->data.typeinfo) rc_release(field->data.typeinfo);
      } break;
      case SYX_STRUCTURE_TYPE_INFO_FIELD_KIND_ACCESSOR: break;
    }
    free((char *)ht_key(&typeinfo->fields, field));
  }
  ht_free(&typeinfo->fields);
}

void syx_structure_type_info_graph_visitor(Rc_Circulars *circulars, const void *data, const void *parent_type) {
  const Syx_Structure_Type_Info *structure = data;
  ht_foreach(field, &structure->fields) {
    switch (field->kind) {
      case SYX_STRUCTURE_TYPE_INFO_FIELD_KIND_DATA: {
        if (field->data.typeinfo) rc_graph_visitor(circulars, (void **)&field->data.typeinfo, parent_type);
      } break;
      case SYX_STRUCTURE_TYPE_INFO_FIELD_KIND_ACCESSOR: break;
    }
  }
}

Syx_Structure_Type_Info *make_syx_structure_type_info_opt(Syx_Structure_Type_Info opt) {
  Syx_Structure_Type_Info *info = rc_malloc(sizeof(Syx_Structure_Type_Info), .destructor = syx_structure_type_info_destructor);
  memset(info, 0, sizeof(Syx_Structure_Type_Info));
  *info = opt;
  if (info->symbol) rc_acquire(get_syxv_from_symbol(info->symbol));
  ht_foreach(field, &info->fields) {
    switch (field->kind) {
      case SYX_STRUCTURE_TYPE_INFO_FIELD_KIND_DATA: {
        if (field->data.typeinfo) rc_acquire(field->data.typeinfo);
      } break;
      case SYX_STRUCTURE_TYPE_INFO_FIELD_KIND_ACCESSOR: break;
    }
  }
  return info;
}

SyxV *syxv_eval_instantiate_structure(Syx_Eval_Ctx *ctx, Syx_Constructor *constructor, SyxV *arguments) {
  void *data = malloc(constructor->typeinfo->size);
  if (!data) RUNTIME_ERROR(ctx, "failed to allocate structure");
  memset(data, 0, constructor->typeinfo->size);
  SyxV *structure = make_syxv_structure(constructor->typeinfo, data);
  if (constructor->typeinfo->constructor) {
    const char *function_name;
    if (constructor->typeinfo->symbol) function_name = constructor->typeinfo->symbol->name;
    else function_name = "constructor";
    syx_ctx_push_frame(ctx, function_name);
    SyxV *result = constructor->typeinfo->constructor(ctx, data, arguments);
    syx_ctx_pop_frame(ctx, result);
    syx_eval_early_exit(result, structure);
    if (result) rc_release(result);
  }
  return structure;
}

SyxV *syxv_eval_structure_getter(Syx_Eval_Ctx *ctx, SyxV_Structure *structure, SyxV *field_arg) {
  if (field_arg->kind == SYXV_KIND_NUMBER && field_arg->number.kind == SYX_NUMBER_KIND_INTEGER) {
    if (!structure->typeinfo->index_getter) RUNTIME_ERROR(ctx, "structure does not implement index getter");
    return structure->typeinfo->index_getter(ctx, structure->data, field_arg->number.integer);
  } else if (field_arg->kind == SYXV_KIND_SYMBOL) {
    const char *field_name = field_arg->symbol.name;
    Syx_Structure_Type_Info_Field *field;
    if (structure->typeinfo->field_getter) {
      return structure->typeinfo->field_getter(ctx, structure->data, field_name);
    } else if (!(field = ht_find(&structure->typeinfo->fields, field_name))) {
      RUNTIME_ERROR(ctx, temp_sprintf("structure does not have '%s' field", field_name));
    } else if (field->kind == SYX_STRUCTURE_TYPE_INFO_FIELD_KIND_DATA) {
      return syx_type_info_to_syxv(ctx, field->data.typeinfo, structure->data + field->data.offset);
    } else if (field->kind == SYX_STRUCTURE_TYPE_INFO_FIELD_KIND_ACCESSOR) {
      if (!field->accessor.getter) RUNTIME_ERROR(ctx, temp_sprintf("structure does not implement '%s' field getter", field_name));
      return field->accessor.getter(ctx, structure->data);
    }
  }
  RUNTIME_ERROR(ctx, "unsupported type of field");
}

SyxV *syxv_eval_structure__setter(Syx_Eval_Ctx *ctx, SyxV_Structure *structure, SyxV *field_arg, SyxV *argument) {
  if (field_arg->kind == SYXV_KIND_NUMBER && field_arg->number.kind == SYX_NUMBER_KIND_INTEGER) {
    if (!structure->typeinfo->index_setter) RUNTIME_ERROR(ctx, "structure does not implement index setter");
    return structure->typeinfo->index_setter(ctx, structure->data, field_arg->number.integer, argument);
  } else if (field_arg->kind == SYXV_KIND_SYMBOL) {
    const char *field_name = field_arg->symbol.name;
    Syx_Structure_Type_Info_Field *field;
    if (structure->typeinfo->field_setter) {
      return structure->typeinfo->field_setter(ctx, structure->data, field_name, argument);
    } else if (!(field = ht_find(&structure->typeinfo->fields, field_name))) {
      RUNTIME_ERROR(ctx, temp_sprintf("structure does not have '%s' field", field_name));
    } else if (field->kind == SYX_STRUCTURE_TYPE_INFO_FIELD_KIND_DATA) {
      return syx_type_info_from_syxv(ctx, field->data.typeinfo, structure->data + field->data.offset, argument);
    } else if (field->kind == SYX_STRUCTURE_TYPE_INFO_FIELD_KIND_ACCESSOR) {
      if (!field->accessor.setter) RUNTIME_ERROR(ctx, temp_sprintf("structure does not implement '%s' field getter", field_name));
      return field->accessor.setter(ctx, structure->data, argument);
    }
  }
  RUNTIME_ERROR(ctx, "unsupported type of field");
}

SyxV *syxv_eval_structure_setter(Syx_Eval_Ctx *ctx, SyxV_Structure *structure, SyxV *field_arg, SyxV *argument) {
  argument = syx_eval(ctx, argument);
  syx_eval_early_exit(argument);
  rc_acquire(argument);
  SyxV *result = syxv_eval_structure__setter(ctx, structure, field_arg, argument);
  if (!result) result = make_syxv_nil();
  rc_acquire(result);
  rc_release(argument);
  return rc_move(result);
}

SyxV *syxv_eval_structure(Syx_Eval_Ctx *ctx, SyxV_Structure *structure, SyxV *arguments) {
  SyxV *field_arg = syx_eval(ctx, syxv_list_next(&arguments));
  syx_eval_early_exit(field_arg);
  rc_acquire(field_arg);

  String_Builder field_str = stringify_syxv(field_arg);
  const char *function_name;
  if (!structure->typeinfo->symbol) function_name = temp_sprintf("(#.<anonim> %s)", field_str.items);
  else if (structure->typeinfo->symbol->guarded) function_name = temp_sprintf("(#.|%s| %s)", structure->typeinfo->symbol->name, field_str.items);
  else function_name = temp_sprintf("(#.%s %s)", structure->typeinfo->symbol->name, field_str.items);
  sb_free(field_str);

  SyxV *result;
  syx_ctx_push_frame(ctx, function_name);
  if (arguments->kind == SYXV_KIND_NIL) {
    result = syxv_eval_structure_getter(ctx, structure, field_arg);
  } else {
    result = syxv_eval_structure_setter(ctx, structure, field_arg, syxv_list_next(&arguments));
  }
  if (!result) result = make_syxv_nil();
  rc_acquire(result);
  syx_ctx_pop_frame(ctx);
  rc_release(field_arg);
  return rc_move(result);
}

#endif // SYX_STRUCTURE_TYPE_INFO_IMPL
