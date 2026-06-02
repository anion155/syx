#ifndef SYX_BOXED_H
#define SYX_BOXED_H

#include "syx_type_info.h"

typedef struct Syx_Boxed Syx_Boxed;

struct Syx_Boxed {
  Syx_Type_Info *typeinfo;
  void *data;
  Syx_Boxed *parent;
};

Syx_Boxed *make_syx_boxed_opt(Syx_Boxed opt);
#define make_syx_boxed(...) make_syx_boxed_opt((Syx_Boxed){__VA_ARGS__})

SyxV *syx_boxed_set(Syx_Eval_Ctx *ctx, Syx_Boxed *boxed, SyxV *argument);
SyxV *syx_eval_boxed_construct(Syx_Eval_Ctx *ctx, Syx_Type_Info *typeinfo, SyxV *arguments);
SyxV *syx_eval_boxed(Syx_Eval_Ctx *ctx, Syx_Boxed *boxed, SyxV *arguments);

size_t stringify_syx_boxed_n(char *string, Syx_Boxed *boxed);
syx_string_t stringify_syx_boxed(Syx_Boxed *boxed);
void sb_append_syx_boxed(String_Builder *sb, Syx_Boxed *boxed);

SyxV *syx_unbox(Syx_Eval_Ctx *ctx, Syx_Boxed *boxed);
SyxV *syx_convert_boxed_to_bool(Syx_Eval_Ctx *ctx, Syx_Boxed *boxed);
SyxV *syx_convert_boxed_to_number(Syx_Eval_Ctx *ctx, Syx_Boxed *boxed);
SyxV *syx_convert_boxed_to_string(Syx_Eval_Ctx *ctx, Syx_Boxed *boxed);

typedef struct Syx_Boxed_Method {
  Syx_Boxed *boxed;
  Syx_Type_Info_Structure_Field *method_field;
} Syx_Boxed_Method;

Syx_Boxed_Method *make_syx_boxed_method(Syx_Boxed *boxed, Syx_Type_Info_Structure_Field *method_field);
SyxV *syx_eval_boxed_method(Syx_Eval_Ctx *ctx, Syx_Boxed_Method *method, SyxV *arguments);

size_t stringify_syx_boxed_method_n(char *string, Syx_Boxed_Method *method);
syx_string_t stringify_syx_boxed_method(Syx_Boxed_Method *method);
void sb_append_syx_boxed_method(String_Builder *sb, Syx_Boxed_Method *method);

#endif // SYX_BOXED_H

#if defined(SYX_BOXED_IMPL) && !defined(SYX_BOXED_IMPL_C)
#define SYX_BOXED_IMPL_C

#define SYX_EVAL_IMPL
#include "syx_eval.h"
#define SYX_BOXED_IMPL
#include "syx_type_info.h"

void syx_boxed_deconstruct(void *data) {
  Syx_Boxed *boxed = data;
  if (boxed->parent) {
    if (boxed->typeinfo) rc_release(boxed->typeinfo);
    rc_release(boxed->parent);
    return;
  }
  if (boxed->typeinfo && boxed->data) {
    switch (boxed->typeinfo->kind) {
      case SYX_TYPE_INFO_KIND_PTR: {
        Syx_Boxed nested = {.typeinfo = rc_acquire(boxed->typeinfo->ptr), .data = *(void **)boxed->data, .parent = NULL};
        syx_boxed_deconstruct(&nested);
        TODO("need to somehow release and free this pointer");
      } break;
      case SYX_TYPE_INFO_KIND_STRUCTURE: {
        if (boxed->typeinfo->structure.destructor) {
          boxed->typeinfo->structure.destructor(boxed->data);
        }
      } break;
      case SYX_TYPE_INFO_KIND_VALUE_PTR: {
        rc_release(boxed->data);
      } break;
      default:
    }
  }
  if (boxed->typeinfo) rc_release(boxed->typeinfo);
  if (boxed->data) free(boxed->data);
}

Syx_Boxed *make_syx_boxed_opt(Syx_Boxed opt) {
  Syx_Boxed *boxed = rc_malloc(sizeof(Syx_Boxed), .destructor = syx_boxed_deconstruct);
  assert(boxed);
  boxed->typeinfo = opt.typeinfo ? rc_acquire(opt.typeinfo) : NULL;
  boxed->data = opt.data;
  boxed->parent = opt.parent ? rc_acquire(opt.parent) : NULL;
  return boxed;
}

SyxV *syx_boxed_set(Syx_Eval_Ctx *ctx, Syx_Boxed *boxed, SyxV *argument) {
  // TODO: check if readonly
  // TODO: accessor set
  switch (boxed->typeinfo->kind) {
    case SYX_TYPE_INFO_KIND_PTR: {
      if (argument->kind != SYXV_KIND_BOXED) RUNTIME_ERROR(ctx, "boxed pointer expected");
      if (argument->boxed->typeinfo->kind != SYX_TYPE_INFO_KIND_PTR) RUNTIME_ERROR(ctx, "boxed pointer expected");
      void **data = boxed->data;
      if (*data) TODO("need to somehow release this pointer");
      TODO("need to acquire this pointer");
      *data = argument->boxed->data;
    } break;
    case SYX_TYPE_INFO_KIND_FUNCTION_PTR: {
      if (argument->kind != SYXV_KIND_BOXED) RUNTIME_ERROR(ctx, "boxed function expected");
      if (argument->boxed->typeinfo->kind != SYX_TYPE_INFO_KIND_FUNCTION_PTR) RUNTIME_ERROR(ctx, "boxed function expected");
      boxed->data = argument->boxed->data; // no free is needed, cause it's functions
    } break;
    case SYX_TYPE_INFO_KIND_VALUE_PTR: {
      if (boxed->data) rc_release(boxed->data);
      boxed->data = rc_acquire(argument);
    } break;
    case SYX_TYPE_INFO_KIND_STRUCTURE: {
      if (argument->kind != SYXV_KIND_BOXED) RUNTIME_ERROR(ctx, "boxed structure expected");
      if (argument->boxed->typeinfo->kind != SYX_TYPE_INFO_KIND_STRUCTURE) RUNTIME_ERROR(ctx, "boxed structure expected");
      memcpy(boxed->data, argument->boxed->data, boxed->typeinfo->size);
    } break;
    case SYX_TYPE_INFO_KIND_VOID: break;
    default: {
      SyxV *value = syx_convert_to_number(ctx, argument);
      syx_eval_early_exit(value);
      rc_acquire(value);
      if (value->kind != SYXV_KIND_NUMBER) RUNTIME_ERROR(ctx, "number expected");
      switch (boxed->typeinfo->kind) {
        case SYX_TYPE_INFO_KIND_CHAR: (*((SYX_TYPE_CHAR *)boxed->data)) = syx_number_integer_value(value->number); break;
        case SYX_TYPE_INFO_KIND_I8: (*((SYX_TYPE_I8 *)boxed->data)) = syx_number_integer_value(value->number); break;
        case SYX_TYPE_INFO_KIND_I16: (*((SYX_TYPE_I16 *)boxed->data)) = syx_number_integer_value(value->number); break;
        case SYX_TYPE_INFO_KIND_I32: (*((SYX_TYPE_I32 *)boxed->data)) = syx_number_integer_value(value->number); break;
        case SYX_TYPE_INFO_KIND_I64: (*((SYX_TYPE_I64 *)boxed->data)) = syx_number_integer_value(value->number); break;
#ifdef SYX_TYPE_I128_SUPPORTED
        case SYX_TYPE_INFO_KIND_I128: (*((SYX_TYPE_I128 *)boxed->data)) = syx_number_integer_value(value->number); break;
#endif
        case SYX_TYPE_INFO_KIND_U8: (*((SYX_TYPE_U8 *)boxed->data)) = syx_number_integer_value(value->number); break;
        case SYX_TYPE_INFO_KIND_U16: (*((SYX_TYPE_U16 *)boxed->data)) = syx_number_integer_value(value->number); break;
        case SYX_TYPE_INFO_KIND_U32: (*((SYX_TYPE_U32 *)boxed->data)) = syx_number_integer_value(value->number); break;
        case SYX_TYPE_INFO_KIND_U64: (*((SYX_TYPE_U64 *)boxed->data)) = syx_number_integer_value(value->number); break;
#ifdef SYX_TYPE_I128_SUPPORTED
        case SYX_TYPE_INFO_KIND_U128: (*((SYX_TYPE_U128 *)boxed->data)) = syx_number_integer_value(value->number); break;
#endif
        case SYX_TYPE_INFO_KIND_INT: (*((SYX_TYPE_INT *)boxed->data)) = syx_number_integer_value(value->number); break;
        case SYX_TYPE_INFO_KIND_INT_LONG: (*((SYX_TYPE_INT_LONG *)boxed->data)) = syx_number_integer_value(value->number); break;
        case SYX_TYPE_INFO_KIND_INT_LONG_LONG: (*((SYX_TYPE_INT_LONG_LONG *)boxed->data)) = syx_number_integer_value(value->number); break;
        case SYX_TYPE_INFO_KIND_UINT: (*((SYX_TYPE_UINT *)boxed->data)) = syx_number_integer_value(value->number); break;
        case SYX_TYPE_INFO_KIND_UINT_LONG: (*((SYX_TYPE_UINT_LONG *)boxed->data)) = syx_number_integer_value(value->number); break;
        case SYX_TYPE_INFO_KIND_UINT_LONG_LONG: (*((SYX_TYPE_UINT_LONG_LONG *)boxed->data)) = syx_number_integer_value(value->number); break;
#ifdef SYX_TYPE_SIZED_FLOAT_SUPPORTED
        case SYX_TYPE_INFO_KIND_F16: (*((SYX_TYPE_F16 *)boxed->data)) = syx_number_fractional_value(number->number); break;
        case SYX_TYPE_INFO_KIND_F32: (*((SYX_TYPE_F32 *)boxed->data)) = syx_number_fractional_value(number->number); break;
        case SYX_TYPE_INFO_KIND_F64: (*((SYX_TYPE_F64 *)boxed->data)) = syx_number_fractional_value(number->number); break;
        case SYX_TYPE_INFO_KIND_F128: (*((SYX_TYPE_F128 *)boxed->data)) = syx_number_fractional_value(number->number); break;
#endif
        case SYX_TYPE_INFO_KIND_FLOAT: (*((SYX_TYPE_FLOAT *)boxed->data)) = syx_number_fractional_value(value->number); break;
        case SYX_TYPE_INFO_KIND_DOUBLE: (*((SYX_TYPE_DOUBLE *)boxed->data)) = syx_number_fractional_value(value->number); break;
        case SYX_TYPE_INFO_KIND_DOUBLE_LONG: (*((SYX_TYPE_DOUBLE_LONG *)boxed->data)) = syx_number_fractional_value(value->number); break;
        case SYX_TYPE_INFO_KIND_SIZE: (*((SYX_TYPE_SIZE *)boxed->data)) = syx_number_integer_value(value->number); break;
        default: RUNTIME_ERROR(ctx, temp_sprintf("kind is not supported: '%s'", syx_type_info_kind_name(boxed->typeinfo->kind)), value);
      }
      rc_release(value);
    }
  }
  return make_syxv_nil();
}

SyxV *syx_eval_boxed_construct(Syx_Eval_Ctx *ctx, Syx_Type_Info *typeinfo, SyxV *arguments) {
  Syx_Boxed *boxed;
  {
    void *data = malloc(typeinfo->size);
    if (!data) RUNTIME_ERROR(ctx, "failed to allocate boxed value");
    boxed = make_syx_boxed_opt((Syx_Boxed){.typeinfo = typeinfo, .data = data, .parent = NULL});
  }
  rc_acquire(boxed);
  memset(boxed->data, 0, typeinfo->size);

  if (arguments->kind == SYXV_KIND_PAIR) {
    SyxV *argument = arguments->pair.left;
    if (argument && argument->kind == SYXV_KIND_BOXED && argument->boxed->typeinfo->kind == typeinfo->kind) {
      SyxV *result = syx_boxed_set(ctx, boxed, syxv_list_next(&arguments));
      syx_eval_early_exit(result, boxed);
      rc_acquire(result);
      rc_release(result);
      return make_syxv_boxed(rc_move(boxed));
    }
  }
  switch (typeinfo->kind) {
    case SYX_TYPE_INFO_KIND_PTR: {
      SyxV *nested = syx_eval_boxed_construct(ctx, typeinfo->ptr, arguments);
      syx_eval_early_exit(nested, boxed);
      TODO("need to acquire this pointer");
      *(void **)boxed->data = nested->boxed->data;
      nested->boxed->data = NULL;
      rc_release(nested);
    } break;
    case SYX_TYPE_INFO_KIND_FUNCTION_PTR: {
      RUNTIME_ERROR(ctx, "can not instantiate function", boxed);
    } break;
    case SYX_TYPE_INFO_KIND_VALUE_PTR: {
      SyxV *value = arguments->kind != SYXV_KIND_PAIR ? make_syxv_nil() : arguments->pair.left;
      boxed->data = rc_acquire(value);
    } break;
    case SYX_TYPE_INFO_KIND_STRUCTURE: {
      if (typeinfo->structure.constructor) {
        SyxV *result = typeinfo->structure.constructor(ctx, boxed->data, arguments);
        syx_eval_early_exit_temporary(result, boxed);
      }
    }; break;
    case SYX_TYPE_INFO_KIND_VOID: break;
    default: {
      SyxV *result = syx_boxed_set(ctx, boxed, syxv_list_next(&arguments));
      syx_eval_early_exit_temporary(result, boxed);
    }
  }

  return make_syxv_boxed(rc_move(boxed));
}

typedef struct Syx_Boxed_Structure_Index_Setter_Data {
  Syx_Boxed *boxed;
  syx_integer_t index;
} Syx_Boxed_Structure_Index_Setter_Data;

SyxV *syx_boxed_structure_index_setter(Syx_Eval_Ctx *ctx, SyxV *target, void *_data, SyxV *value) {
  UNUSED(target);
  Syx_Boxed_Structure_Index_Setter_Data *data = _data;
  return data->boxed->typeinfo->structure.index_setter(ctx, data->boxed->data, data->index, value);
}

SyxV *syx_boxed_structure_field_data_setter(Syx_Eval_Ctx *ctx, SyxV *target, void *data, SyxV *value) {
  UNUSED(data);
  if (target->kind != SYXV_KIND_BOXED) RUNTIME_ERROR(ctx, "boxed value structure expected");
  return syx_boxed_set(ctx, target->boxed, value);
}

typedef struct Syx_Boxed_Structure_Field_Accessor_Setter_Data {
  Syx_Boxed *boxed;
  Syx_Type_Info_Structure_Accessor_Field *accessor;
} Syx_Boxed_Structure_Field_Accessor_Setter_Data;

SyxV *syx_boxed_structure_field_accessor_setter(Syx_Eval_Ctx *ctx, SyxV *target, void *_data, SyxV *value) {
  UNUSED(target);
  Syx_Boxed_Structure_Field_Accessor_Setter_Data *data = _data;
  return data->accessor->setter(ctx, data->boxed->data, value);
}

typedef struct Syx_Boxed_Structure_Field_Setter_Data {
  Syx_Boxed *boxed;
  const char *field_name;
} Syx_Boxed_Structure_Field_Setter_Data;

SyxV *syx_boxed_structure_field_setter(Syx_Eval_Ctx *ctx, SyxV *target, void *_data, SyxV *value) {
  UNUSED(target);
  Syx_Boxed_Structure_Field_Setter_Data *data = _data;
  return data->boxed->typeinfo->structure.field_setter(ctx, data->boxed->data, data->field_name, value);
}

SyxV *syx_eval_boxed_structure_getter(Syx_Eval_Ctx *ctx, Syx_Boxed *boxed, const char *typeinfo_string, SyxV *field_arg) {
  SyxV *result;
  if (field_arg->kind == SYXV_KIND_NUMBER && field_arg->number.kind == SYX_NUMBER_KIND_INTEGER) {
    syx_integer_t index = field_arg->number.integer;
    if (boxed->typeinfo->structure.index_getter) {
      syx_ctx_push_frame(ctx, temp_sprintf("(%s get %lld)", typeinfo_string, index));
      result = boxed->typeinfo->structure.index_getter(ctx, boxed->data, index);
      if (boxed->typeinfo->structure.index_setter) {
        Syx_Boxed_Structure_Index_Setter_Data *data = malloc(sizeof(Syx_Boxed_Structure_Index_Setter_Data));
        data->boxed = boxed;
        data->index = index;
        result->lvalue = rc_acquire(make_syx_lvalue_closure(syx_boxed_structure_index_setter, data));
      }
      rc_acquire(result);
      syx_ctx_pop_frame(ctx);
      rc_move(result);
    } else {
      RUNTIME_ERROR(ctx, "no index getter");
    }
  } else if (field_arg->kind == SYXV_KIND_SYMBOL) {
    const char *field_name = field_arg->symbol.name;
    Syx_Type_Info_Structure_Field *field = ht_find(&boxed->typeinfo->structure.fields, field_name);
    if (field) {
      switch (field->kind) {
        case SYX_TYPE_INFO_STRUCTURE_FIELD_KIND_DATA: {
          void *field_data = boxed->data + field->data.offset;
          result = make_syxv_boxed(make_syx_boxed(.typeinfo = field->data.typeinfo, .data = field_data, .parent = boxed));
          if (field->data.readonly) {
            void **data = malloc(sizeof(void *));
            *data = &field->data;
            result->lvalue = rc_acquire(make_syx_lvalue_closure(syx_boxed_structure_field_data_setter, data));
          }
        } break;
        case SYX_TYPE_INFO_STRUCTURE_FIELD_KIND_ACCESSOR: {
          if (!field->accessor.getter) RUNTIME_ERROR(ctx, "accessor field has no getter");
          syx_ctx_push_frame(ctx, temp_sprintf("(%s get %s)", typeinfo_string, field_name));
          result = field->accessor.getter(ctx, boxed->data);
          if (field->accessor.setter) {
            Syx_Boxed_Structure_Field_Accessor_Setter_Data *data = malloc(sizeof(Syx_Boxed_Structure_Field_Accessor_Setter_Data));
            data->boxed = boxed;
            data->accessor = &field->accessor;
            result->lvalue = rc_acquire(make_syx_lvalue_closure(syx_boxed_structure_field_accessor_setter, data));
          }
          rc_acquire(result);
          syx_ctx_pop_frame(ctx);
          rc_move(result);
        } break;
        case SYX_TYPE_INFO_STRUCTURE_FIELD_KIND_METHOD: {
          result = make_syxv_boxed_method(make_syx_boxed_method(boxed, field));
        } break;
      }
    } else if (boxed->typeinfo->structure.field_getter) {
      syx_ctx_push_frame(ctx, temp_sprintf("(%s get %s)", typeinfo_string, field_name));
      result = boxed->typeinfo->structure.field_getter(ctx, boxed->data, field_name);
      if (!result) RUNTIME_ERROR(ctx, temp_sprintf("there is no field named '%s'", field_name));
      if (boxed->typeinfo->structure.field_setter) {
        Syx_Boxed_Structure_Field_Setter_Data *data = malloc(sizeof(Syx_Boxed_Structure_Field_Setter_Data));
        data->boxed = boxed;
        data->field_name = field_name;
        result->lvalue = rc_acquire(make_syx_lvalue_closure(syx_boxed_structure_field_setter, data));
      }
      rc_acquire(result);
      syx_ctx_pop_frame(ctx);
      rc_move(result);
    } else {
      RUNTIME_ERROR(ctx, temp_sprintf("there is no field named '%s'", field_name));
    }
  } else {
    syx_string_t field_str = stringify_syxv(field_arg);
    syx_ctx_frame_rename(ctx, temp_sprintf("(%s %s)", typeinfo_string, field_str.items));
    char *message = temp_sprintf("unsupported field kind: %s", field_str.items);
    sb_free(field_str);
    RUNTIME_ERROR(ctx, message);
  }
  return result;
}

SyxV *syx_eval_boxed(Syx_Eval_Ctx *ctx, Syx_Boxed *boxed, SyxV *arguments) {
  Syx_Boxed *current = rc_acquire(boxed);
  SyxV *result = NULL;
  SyxV *unbox_symbol = rc_acquire(make_syxv_symbol_cstr("unbox"));
  SyxV *unref_symbol = rc_acquire(make_syxv_symbol_cstr("unref"));
  syxv_list_for_each(argument, arguments) {
    if (result) rc_release(result);
    if (!current) RUNTIME_ERROR(ctx, "trying to access non boxed value");
    if (!current->data) RUNTIME_ERROR(ctx, "trying to access null boxed value", current);

    SyxV *field_arg = syx_eval_unquote(ctx, argument);
    syx_eval_early_exit(field_arg, unbox_symbol, unref_symbol, current);
    rc_acquire(field_arg);

    if (field_arg == unbox_symbol) {
      result = (SyxV *)current->data;
    } else {
      switch (current->typeinfo->kind) {
        case SYX_TYPE_INFO_KIND_PTR: {
          if (field_arg == unref_symbol) {
            result = make_syxv_boxed(make_syx_boxed(.typeinfo = current->typeinfo->ptr, .data = *(void **)current->data, .parent = current));
          } else if (field_arg->kind == SYXV_KIND_NUMBER && field_arg->number.kind == SYX_NUMBER_KIND_INTEGER) {
            syx_integer_t index = field_arg->number.integer;
            result = make_syxv_boxed(make_syx_boxed(.typeinfo = current->typeinfo->ptr, .data = *(void **)(*(char **)current->data + current->typeinfo->ptr->size * index), .parent = current));
          } else {
            RUNTIME_ERROR(ctx, "can not access inner field of boxed pointer value", unbox_symbol, unref_symbol, field_arg, current);
          }
        } break;
        case SYX_TYPE_INFO_KIND_STRUCTURE: {
          syx_string_t typeinfo_str = stringify_syx_type_info(boxed->typeinfo);
          result = syx_eval_boxed_structure_getter(ctx, current, typeinfo_str.items, field_arg);
          sb_free(typeinfo_str);
        }; break;
        default: {
          const char *kind_name = syx_type_info_kind_name(current->typeinfo->kind);
          RUNTIME_ERROR(ctx, temp_sprintf("can not access inner fields of boxed value: '%s'", kind_name), unbox_symbol, unref_symbol, field_arg, current);
        }
      }
    }
    syx_eval_early_exit(result, unbox_symbol, unref_symbol, field_arg, current);
    Syx_Boxed *prev = current;
    current = NULL;
    if (result) {
      rc_acquire(result);
      if (result->kind == SYXV_KIND_BOXED) current = rc_acquire(result->boxed);
    }
    rc_release(prev);
    rc_release(field_arg);
  }
  if (!result) result = rc_acquire(make_syxv_nil());
  rc_release(unbox_symbol);
  rc_release(unref_symbol);
  if (current) rc_release(current);
  return rc_move(result);
}

size_t stringify_syx_boxed_n(char *string, Syx_Boxed *boxed) {
  __str_it();
  __str_convert(stringify_syx_type_info_n, boxed->typeinfo);
  __str_push('(');
  switch (boxed->typeinfo->kind) {
    case SYX_TYPE_INFO_KIND_PTR: TODO("implement boxed pointer stringify");
    case SYX_TYPE_INFO_KIND_FUNCTION_PTR: TODO("implement boxed function pointer stringify");
    case SYX_TYPE_INFO_KIND_STRUCTURE: TODO("implement boxed structure pointer stringify");
    case SYX_TYPE_INFO_KIND_VALUE_PTR: __str_convert(stringify_syxv_n, (SyxV *)boxed->data); break;
    case SYX_TYPE_INFO_KIND_VOID: break;
    case SYX_TYPE_INFO_KIND_CHAR: __str_push(*(SYX_TYPE_CHAR *)boxed->data); break;
    case SYX_TYPE_INFO_KIND_I8: __str_convert(stringify_integer_n, *(SYX_TYPE_I8 *)boxed->data); break;
    case SYX_TYPE_INFO_KIND_I16: __str_convert(stringify_integer_n, *(SYX_TYPE_I16 *)boxed->data); break;
    case SYX_TYPE_INFO_KIND_I32: __str_convert(stringify_integer_n, *(SYX_TYPE_I32 *)boxed->data); break;
    case SYX_TYPE_INFO_KIND_I64: __str_convert(stringify_integer_n, *(SYX_TYPE_I64 *)boxed->data); break;
#ifdef SYX_TYPE_I128_SUPPORTED
    case SYX_TYPE_INFO_KIND_I128: __str_convert(stringify_integer_n, *(SYX_TYPE_I128 *)boxed->data); break;
#endif
    case SYX_TYPE_INFO_KIND_U8: __str_convert(stringify_integer_n, *(SYX_TYPE_U8 *)boxed->data); break;
    case SYX_TYPE_INFO_KIND_U16: __str_convert(stringify_integer_n, *(SYX_TYPE_U16 *)boxed->data); break;
    case SYX_TYPE_INFO_KIND_U32: __str_convert(stringify_integer_n, *(SYX_TYPE_U32 *)boxed->data); break;
    case SYX_TYPE_INFO_KIND_U64: __str_convert(stringify_integer_n, *(SYX_TYPE_U64 *)boxed->data); break;
#ifdef SYX_TYPE_I128_SUPPORTED
    case SYX_TYPE_INFO_KIND_U128: __str_convert(stringify_integer_n, *(SYX_TYPE_U128 *)boxed->data); break;
#endif
    case SYX_TYPE_INFO_KIND_INT: __str_convert(stringify_integer_n, *(SYX_TYPE_INT *)boxed->data); break;
    case SYX_TYPE_INFO_KIND_INT_LONG: __str_convert(stringify_integer_n, *(SYX_TYPE_INT_LONG *)boxed->data); break;
    case SYX_TYPE_INFO_KIND_INT_LONG_LONG: __str_convert(stringify_integer_n, *(SYX_TYPE_INT_LONG_LONG *)boxed->data); break;
    case SYX_TYPE_INFO_KIND_UINT: __str_convert(stringify_integer_n, *(SYX_TYPE_UINT *)boxed->data); break;
    case SYX_TYPE_INFO_KIND_UINT_LONG: __str_convert(stringify_integer_n, *(SYX_TYPE_UINT_LONG *)boxed->data); break;
    case SYX_TYPE_INFO_KIND_UINT_LONG_LONG: __str_convert(stringify_integer_n, *(SYX_TYPE_UINT_LONG_LONG *)boxed->data); break;
#ifdef SYX_TYPE_SIZED_FLOAT_SUPPORTED
    case SYX_TYPE_INFO_KIND_F16: __str_convert(stringify_fractional_n, *(SYX_TYPE_F16 *)boxed->data); break;
    case SYX_TYPE_INFO_KIND_F32: __str_convert(stringify_fractional_n, *(SYX_TYPE_F32 *)boxed->data); break;
    case SYX_TYPE_INFO_KIND_F64: __str_convert(stringify_fractional_n, *(SYX_TYPE_F64 *)boxed->data); break;
    case SYX_TYPE_INFO_KIND_F128: __str_convert(stringify_fractional_n, *(SYX_TYPE_F128 *)boxed->data); break;
#endif
    case SYX_TYPE_INFO_KIND_FLOAT: __str_convert(stringify_fractional_n, *(SYX_TYPE_FLOAT *)boxed->data); break;
    case SYX_TYPE_INFO_KIND_DOUBLE: __str_convert(stringify_fractional_n, *(SYX_TYPE_DOUBLE *)boxed->data); break;
    case SYX_TYPE_INFO_KIND_DOUBLE_LONG: __str_convert(stringify_fractional_n, *(SYX_TYPE_DOUBLE_LONG *)boxed->data); break;
    case SYX_TYPE_INFO_KIND_SIZE: __str_convert(stringify_integer_n, *(SYX_TYPE_SIZE *)boxed->data); break;
    default: break;
  }
  __str_push(')');
  return __str_width();
}

syx_string_t stringify_syx_boxed(Syx_Boxed *boxed) {
  __stringify_body(stringify_syx_boxed_n, 256, boxed);
}

void sb_append_syx_boxed(String_Builder *sb, Syx_Boxed *boxed) {
  __sb_append_body(stringify_syx_boxed_n, 256, boxed);
}

SyxV *syx_unbox(Syx_Eval_Ctx *ctx, Syx_Boxed *boxed) {
  if (!boxed->data) return make_syxv_nil();
  switch (boxed->typeinfo->kind) {
    case SYX_TYPE_INFO_KIND_PTR: RUNTIME_ERROR(ctx, "illegal unboxing of pointer boxed value");
    case SYX_TYPE_INFO_KIND_FUNCTION_PTR: RUNTIME_ERROR(ctx, "illegal unboxing of function boxed value");
    case SYX_TYPE_INFO_KIND_VALUE_PTR: return (SyxV *)boxed->data;
    case SYX_TYPE_INFO_KIND_STRUCTURE: RUNTIME_ERROR(ctx, "illegal unboxing of structure boxed value");
    case SYX_TYPE_INFO_KIND_VOID: RUNTIME_ERROR(ctx, "illegal unboxing of void boxed value");
    case SYX_TYPE_INFO_KIND_CHAR: return make_syxv_string_n((SYX_TYPE_CHAR *)boxed->data, 1);
    case SYX_TYPE_INFO_KIND_I8: return make_syxv_number_integer(*(SYX_TYPE_I8 *)boxed->data);
    case SYX_TYPE_INFO_KIND_I16: return make_syxv_number_integer(*(SYX_TYPE_I16 *)boxed->data);
    case SYX_TYPE_INFO_KIND_I32: return make_syxv_number_integer(*(SYX_TYPE_I32 *)boxed->data);
    case SYX_TYPE_INFO_KIND_I64: return make_syxv_number_integer(*(SYX_TYPE_I64 *)boxed->data);
#ifdef SYX_TYPE_I128_SUPPORTED
    case SYX_TYPE_INFO_KIND_I128: return make_syxv_number_integer(*(SYX_TYPE_I128 *)boxed->data);
#endif
    case SYX_TYPE_INFO_KIND_U8: return make_syxv_number_integer(*(SYX_TYPE_U8 *)boxed->data);
    case SYX_TYPE_INFO_KIND_U16: return make_syxv_number_integer(*(SYX_TYPE_U16 *)boxed->data);
    case SYX_TYPE_INFO_KIND_U32: return make_syxv_number_integer(*(SYX_TYPE_U32 *)boxed->data);
    case SYX_TYPE_INFO_KIND_U64: return make_syxv_number_integer(*(SYX_TYPE_U64 *)boxed->data);
#ifdef SYX_TYPE_I128_SUPPORTED
    case SYX_TYPE_INFO_KIND_U128: return make_syxv_number_integer(*(SYX_TYPE_U128 *)boxed->data);
#endif
    case SYX_TYPE_INFO_KIND_INT: return make_syxv_number_integer(*(SYX_TYPE_INT *)boxed->data);
    case SYX_TYPE_INFO_KIND_INT_LONG: return make_syxv_number_integer(*(SYX_TYPE_INT_LONG *)boxed->data);
    case SYX_TYPE_INFO_KIND_INT_LONG_LONG: return make_syxv_number_integer(*(SYX_TYPE_INT_LONG_LONG *)boxed->data);
    case SYX_TYPE_INFO_KIND_UINT: return make_syxv_number_integer(*(SYX_TYPE_UINT *)boxed->data);
    case SYX_TYPE_INFO_KIND_UINT_LONG: return make_syxv_number_integer(*(SYX_TYPE_UINT_LONG *)boxed->data);
    case SYX_TYPE_INFO_KIND_UINT_LONG_LONG: return make_syxv_number_integer(*(SYX_TYPE_UINT_LONG_LONG *)boxed->data);
#ifdef SYX_TYPE_SIZED_FLOAT_SUPPORTED
    case SYX_TYPE_INFO_KIND_F16: return make_syxv_number_fractional(*(SYX_TYPE_F16 *)boxed->data);
    case SYX_TYPE_INFO_KIND_F32: return make_syxv_number_fractional(*(SYX_TYPE_F32 *)boxed->data);
    case SYX_TYPE_INFO_KIND_F64: return make_syxv_number_fractional(*(SYX_TYPE_F64 *)boxed->data);
    case SYX_TYPE_INFO_KIND_F128: return make_syxv_number_fractional(*(SYX_TYPE_F128 *)boxed->data);
#endif
    case SYX_TYPE_INFO_KIND_FLOAT: return make_syxv_number_fractional(*(SYX_TYPE_FLOAT *)boxed->data);
    case SYX_TYPE_INFO_KIND_DOUBLE: return make_syxv_number_fractional(*(SYX_TYPE_DOUBLE *)boxed->data);
    case SYX_TYPE_INFO_KIND_DOUBLE_LONG: return make_syxv_number_fractional(*(SYX_TYPE_DOUBLE_LONG *)boxed->data);
    case SYX_TYPE_INFO_KIND_SIZE: return make_syxv_number_integer(*(SYX_TYPE_SIZE *)boxed->data);
    default: RUNTIME_ERROR(ctx, temp_sprintf("illegal unboxing of boxed value (of kind: '%s')", syx_type_info_kind_name(boxed->typeinfo->kind)));
  }
}

SyxV *syx_convert_boxed_to_bool(Syx_Eval_Ctx *ctx, Syx_Boxed *boxed) {
  if (!boxed->data) return make_syxv_bool(false);
  switch (boxed->typeinfo->kind) {
    case SYX_TYPE_INFO_KIND_PTR: return make_syxv_bool(*(void **)boxed->data != NULL);
    case SYX_TYPE_INFO_KIND_FUNCTION_PTR: return make_syxv_bool(true); // false handled by !boxed->data
    case SYX_TYPE_INFO_KIND_VALUE_PTR: return syx_convert_to_bool(ctx, (SyxV *)boxed->data);
    case SYX_TYPE_INFO_KIND_STRUCTURE: TODO("Implement structure conversion to bool");
    case SYX_TYPE_INFO_KIND_VOID: return make_syxv_bool(false);
    case SYX_TYPE_INFO_KIND_CHAR: return make_syxv_bool((syx_bool_t) * (SYX_TYPE_CHAR *)boxed->data);
    case SYX_TYPE_INFO_KIND_I8: return make_syxv_bool((syx_bool_t) * (SYX_TYPE_I8 *)boxed->data);
    case SYX_TYPE_INFO_KIND_I16: return make_syxv_bool((syx_bool_t) * (SYX_TYPE_I16 *)boxed->data);
    case SYX_TYPE_INFO_KIND_I32: return make_syxv_bool((syx_bool_t) * (SYX_TYPE_I32 *)boxed->data);
    case SYX_TYPE_INFO_KIND_I64: return make_syxv_bool((syx_bool_t) * (SYX_TYPE_I64 *)boxed->data);
#ifdef SYX_TYPE_I128_SUPPORTED
    case SYX_TYPE_INFO_KIND_I128: return make_syxv_bool((syx_bool_t) * (SYX_TYPE_I128 *)boxed->data);
#endif
    case SYX_TYPE_INFO_KIND_U8: return make_syxv_bool((syx_bool_t) * (SYX_TYPE_U8 *)boxed->data);
    case SYX_TYPE_INFO_KIND_U16: return make_syxv_bool((syx_bool_t) * (SYX_TYPE_U16 *)boxed->data);
    case SYX_TYPE_INFO_KIND_U32: return make_syxv_bool((syx_bool_t) * (SYX_TYPE_U32 *)boxed->data);
    case SYX_TYPE_INFO_KIND_U64: return make_syxv_bool((syx_bool_t) * (SYX_TYPE_U64 *)boxed->data);
#ifdef SYX_TYPE_I128_SUPPORTED
    case SYX_TYPE_INFO_KIND_U128: return make_syxv_bool((syx_bool_t) * (SYX_TYPE_U128 *)boxed->data);
#endif
    case SYX_TYPE_INFO_KIND_INT: return make_syxv_bool((syx_bool_t) * (SYX_TYPE_INT *)boxed->data);
    case SYX_TYPE_INFO_KIND_INT_LONG: return make_syxv_bool((syx_bool_t) * (SYX_TYPE_INT_LONG *)boxed->data);
    case SYX_TYPE_INFO_KIND_INT_LONG_LONG: return make_syxv_bool((syx_bool_t) * (SYX_TYPE_INT_LONG_LONG *)boxed->data);
    case SYX_TYPE_INFO_KIND_UINT: return make_syxv_bool((syx_bool_t) * (SYX_TYPE_UINT *)boxed->data);
    case SYX_TYPE_INFO_KIND_UINT_LONG: return make_syxv_bool((syx_bool_t) * (SYX_TYPE_UINT_LONG *)boxed->data);
    case SYX_TYPE_INFO_KIND_UINT_LONG_LONG: return make_syxv_bool((syx_bool_t) * (SYX_TYPE_UINT_LONG_LONG *)boxed->data);
#ifdef SYX_TYPE_SIZED_FLOAT_SUPPORTED
    case SYX_TYPE_INFO_KIND_F16: return make_syxv_bool((syx_bool_t) * (SYX_TYPE_F16 *)boxed->data);
    case SYX_TYPE_INFO_KIND_F32: return make_syxv_bool((syx_bool_t) * (SYX_TYPE_F32 *)boxed->data);
    case SYX_TYPE_INFO_KIND_F64: return make_syxv_bool((syx_bool_t) * (SYX_TYPE_F64 *)boxed->data);
    case SYX_TYPE_INFO_KIND_F128: return make_syxv_bool((syx_bool_t) * (SYX_TYPE_F128 *)boxed->data);
#endif
    case SYX_TYPE_INFO_KIND_FLOAT: return make_syxv_bool((syx_bool_t) * (SYX_TYPE_FLOAT *)boxed->data);
    case SYX_TYPE_INFO_KIND_DOUBLE: return make_syxv_bool((syx_bool_t) * (SYX_TYPE_DOUBLE *)boxed->data);
    case SYX_TYPE_INFO_KIND_DOUBLE_LONG: return make_syxv_bool((syx_bool_t) * (SYX_TYPE_DOUBLE_LONG *)boxed->data);
    case SYX_TYPE_INFO_KIND_SIZE: return make_syxv_bool((syx_bool_t) * (SYX_TYPE_SIZE *)boxed->data);
    default: RUNTIME_ERROR(ctx, temp_sprintf("illegal conversion of boxed value (of kind: '%s') to bool", syx_type_info_kind_name(boxed->typeinfo->kind)));
  }
}

SyxV *syx_convert_boxed_to_number(Syx_Eval_Ctx *ctx, Syx_Boxed *boxed) {
  if (!boxed->data) RUNTIME_ERROR(ctx, "illegal conversion of null boxed value to number");
  switch (boxed->typeinfo->kind) {
    // case SYX_TYPE_INFO_KIND_PTR: break;
    // case SYX_TYPE_INFO_KIND_FUNCTION_PTR: break;
    case SYX_TYPE_INFO_KIND_VALUE_PTR: return syx_convert_to_number(ctx, (SyxV *)boxed->data);
    case SYX_TYPE_INFO_KIND_STRUCTURE: TODO("Implement structure conversion to number");
    // case SYX_TYPE_INFO_KIND_VOID: break;
    case SYX_TYPE_INFO_KIND_CHAR: return make_syxv_number_integer(*(SYX_TYPE_CHAR *)boxed->data);
    case SYX_TYPE_INFO_KIND_I8: return make_syxv_number_integer(*(SYX_TYPE_I8 *)boxed->data);
    case SYX_TYPE_INFO_KIND_I16: return make_syxv_number_integer(*(SYX_TYPE_I16 *)boxed->data);
    case SYX_TYPE_INFO_KIND_I32: return make_syxv_number_integer(*(SYX_TYPE_I32 *)boxed->data);
    case SYX_TYPE_INFO_KIND_I64: return make_syxv_number_integer(*(SYX_TYPE_I64 *)boxed->data);
#ifdef SYX_TYPE_I128_SUPPORTED
    case SYX_TYPE_INFO_KIND_I128: return make_syxv_number_integer(*(SYX_TYPE_I128 *)boxed->data);
#endif
    case SYX_TYPE_INFO_KIND_U8: return make_syxv_number_integer(*(SYX_TYPE_U8 *)boxed->data);
    case SYX_TYPE_INFO_KIND_U16: return make_syxv_number_integer(*(SYX_TYPE_U16 *)boxed->data);
    case SYX_TYPE_INFO_KIND_U32: return make_syxv_number_integer(*(SYX_TYPE_U32 *)boxed->data);
    case SYX_TYPE_INFO_KIND_U64: return make_syxv_number_integer(*(SYX_TYPE_U64 *)boxed->data);
#ifdef SYX_TYPE_I128_SUPPORTED
    case SYX_TYPE_INFO_KIND_U128: return make_syxv_number_integer(*(SYX_TYPE_U128 *)boxed->data);
#endif
    case SYX_TYPE_INFO_KIND_INT: return make_syxv_number_integer(*(SYX_TYPE_INT *)boxed->data);
    case SYX_TYPE_INFO_KIND_INT_LONG: return make_syxv_number_integer(*(SYX_TYPE_INT_LONG *)boxed->data);
    case SYX_TYPE_INFO_KIND_INT_LONG_LONG: return make_syxv_number_integer(*(SYX_TYPE_INT_LONG_LONG *)boxed->data);
    case SYX_TYPE_INFO_KIND_UINT: return make_syxv_number_integer(*(SYX_TYPE_UINT *)boxed->data);
    case SYX_TYPE_INFO_KIND_UINT_LONG: return make_syxv_number_integer(*(SYX_TYPE_UINT_LONG *)boxed->data);
    case SYX_TYPE_INFO_KIND_UINT_LONG_LONG: return make_syxv_number_integer(*(SYX_TYPE_UINT_LONG_LONG *)boxed->data);
#ifdef SYX_TYPE_SIZED_FLOAT_SUPPORTED
    case SYX_TYPE_INFO_KIND_F16: return make_syxv_number_fractional(*(SYX_TYPE_F16 *)boxed->data);
    case SYX_TYPE_INFO_KIND_F32: return make_syxv_number_fractional(*(SYX_TYPE_F32 *)boxed->data);
    case SYX_TYPE_INFO_KIND_F64: return make_syxv_number_fractional(*(SYX_TYPE_F64 *)boxed->data);
    case SYX_TYPE_INFO_KIND_F128: return make_syxv_number_fractional(*(SYX_TYPE_F128 *)boxed->data);
#endif
    case SYX_TYPE_INFO_KIND_FLOAT: return make_syxv_number_fractional(*(SYX_TYPE_FLOAT *)boxed->data);
    case SYX_TYPE_INFO_KIND_DOUBLE: return make_syxv_number_fractional(*(SYX_TYPE_DOUBLE *)boxed->data);
    case SYX_TYPE_INFO_KIND_DOUBLE_LONG: return make_syxv_number_fractional(*(SYX_TYPE_DOUBLE_LONG *)boxed->data);
    case SYX_TYPE_INFO_KIND_SIZE: return make_syxv_number_integer(*(SYX_TYPE_SIZE *)boxed->data);
    default: RUNTIME_ERROR(ctx, temp_sprintf("illegal conversion of boxed value (of kind: '%s') to number", syx_type_info_kind_name(boxed->typeinfo->kind)));
  }
}

SyxV *syx_convert_boxed_to_string(Syx_Eval_Ctx *ctx, Syx_Boxed *boxed) {
  if (!boxed->data) RUNTIME_ERROR(ctx, "illegal conversion of null boxed value to string");
  switch (boxed->typeinfo->kind) {
    case SYX_TYPE_INFO_KIND_PTR: {
      if (boxed->typeinfo->ptr->kind == SYX_TYPE_INFO_KIND_CHAR) TODO("implement boxed string to syxv string");
      RUNTIME_ERROR(ctx, "illegal conversion of pointer boxed value to string");
    } break;
    // case SYX_TYPE_INFO_KIND_FUNCTION_PTR: break;
    case SYX_TYPE_INFO_KIND_VALUE_PTR: return syx_convert_to_string(ctx, (SyxV *)boxed->data);
    case SYX_TYPE_INFO_KIND_STRUCTURE: TODO("Implement structure conversion to string");
    // case SYX_TYPE_INFO_KIND_VOID: break;
    case SYX_TYPE_INFO_KIND_CHAR: return make_syxv_string_n((SYX_TYPE_CHAR *)boxed->data, 1);
    case SYX_TYPE_INFO_KIND_I8: return make_syxv_string(stringify_integer(*(SYX_TYPE_I8 *)boxed->data));
    case SYX_TYPE_INFO_KIND_I16: return make_syxv_string(stringify_integer(*(SYX_TYPE_I16 *)boxed->data));
    case SYX_TYPE_INFO_KIND_I32: return make_syxv_string(stringify_integer(*(SYX_TYPE_I32 *)boxed->data));
    case SYX_TYPE_INFO_KIND_I64: return make_syxv_string(stringify_integer(*(SYX_TYPE_I64 *)boxed->data));
#ifdef SYX_TYPE_I128_SUPPORTED
    case SYX_TYPE_INFO_KIND_I128: return make_syxv_string(stringify_integer(*(SYX_TYPE_I128 *)boxed->data));
#endif
    case SYX_TYPE_INFO_KIND_U8: return make_syxv_string(stringify_integer(*(SYX_TYPE_U8 *)boxed->data));
    case SYX_TYPE_INFO_KIND_U16: return make_syxv_string(stringify_integer(*(SYX_TYPE_U16 *)boxed->data));
    case SYX_TYPE_INFO_KIND_U32: return make_syxv_string(stringify_integer(*(SYX_TYPE_U32 *)boxed->data));
    case SYX_TYPE_INFO_KIND_U64: return make_syxv_string(stringify_integer(*(SYX_TYPE_U64 *)boxed->data));
#ifdef SYX_TYPE_I128_SUPPORTED
    case SYX_TYPE_INFO_KIND_U128: return make_syxv_string(stringify_integer(*(SYX_TYPE_U128 *)boxed->data));
#endif
    case SYX_TYPE_INFO_KIND_INT: return make_syxv_string(stringify_integer(*(SYX_TYPE_INT *)boxed->data));
    case SYX_TYPE_INFO_KIND_INT_LONG: return make_syxv_string(stringify_integer(*(SYX_TYPE_INT_LONG *)boxed->data));
    case SYX_TYPE_INFO_KIND_INT_LONG_LONG: return make_syxv_string(stringify_integer(*(SYX_TYPE_INT_LONG_LONG *)boxed->data));
    case SYX_TYPE_INFO_KIND_UINT: return make_syxv_string(stringify_integer(*(SYX_TYPE_UINT *)boxed->data));
    case SYX_TYPE_INFO_KIND_UINT_LONG: return make_syxv_string(stringify_integer(*(SYX_TYPE_UINT_LONG *)boxed->data));
    case SYX_TYPE_INFO_KIND_UINT_LONG_LONG: return make_syxv_string(stringify_integer(*(SYX_TYPE_UINT_LONG_LONG *)boxed->data));
#ifdef SYX_TYPE_SIZED_FLOAT_SUPPORTED
    case SYX_TYPE_INFO_KIND_F16: return make_syxv_string(stringify_fractional(*(SYX_TYPE_F16 *)boxed->data));
    case SYX_TYPE_INFO_KIND_F32: return make_syxv_string(stringify_fractional(*(SYX_TYPE_F32 *)boxed->data));
    case SYX_TYPE_INFO_KIND_F64: return make_syxv_string(stringify_fractional(*(SYX_TYPE_F64 *)boxed->data));
    case SYX_TYPE_INFO_KIND_F128: return make_syxv_string(stringify_fractional(*(SYX_TYPE_F128 *)boxed->data));
#endif
    case SYX_TYPE_INFO_KIND_FLOAT: return make_syxv_string(stringify_fractional(*(SYX_TYPE_FLOAT *)boxed->data));
    case SYX_TYPE_INFO_KIND_DOUBLE: return make_syxv_string(stringify_fractional(*(SYX_TYPE_DOUBLE *)boxed->data));
    case SYX_TYPE_INFO_KIND_DOUBLE_LONG: return make_syxv_string(stringify_fractional(*(SYX_TYPE_DOUBLE_LONG *)boxed->data));
    case SYX_TYPE_INFO_KIND_SIZE: return make_syxv_string(stringify_integer(*(SYX_TYPE_SIZE *)boxed->data));
    default: RUNTIME_ERROR(ctx, temp_sprintf("illegal conversion of boxed value (of kind: '%s') to string", syx_type_info_kind_name(boxed->typeinfo->kind)));
  }
}

void syx_boxed_method_destructor(void *data) {
  Syx_Boxed_Method *method = data;
  rc_release(method->boxed);
}

Syx_Boxed_Method *make_syx_boxed_method(Syx_Boxed *boxed, Syx_Type_Info_Structure_Field *method_field) {
  Syx_Boxed_Method *boxed_method = rc_malloc(sizeof(Syx_Boxed_Method), .destructor = syx_boxed_method_destructor);
  assert(boxed_method);
  boxed_method->boxed = boxed ? rc_acquire(boxed) : NULL;
  boxed_method->method_field = method_field;
  return boxed_method;
}

SyxV *syx_eval_boxed_method(Syx_Eval_Ctx *ctx, Syx_Boxed_Method *method, SyxV *arguments) {
  if (method->method_field->kind != SYX_TYPE_INFO_STRUCTURE_FIELD_KIND_METHOD) RUNTIME_ERROR(ctx, "method expected");
  syx_string_t name = stringify_syx_boxed_method(method);
  syx_ctx_push_frame(ctx, name.items);
  sb_free(name);
  SyxV *result = method->method_field->method(ctx, method->boxed->data, arguments);
  if (!result) result = make_syxv_nil();
  rc_acquire(result);
  syx_ctx_pop_frame(ctx);
  return rc_move(result);
}

size_t stringify_syx_boxed_method_n(char *string, Syx_Boxed_Method *method) {
  __str_it();
  __str_push('(');
  __str_convert(stringify_syx_type_info_n, method->boxed->typeinfo);
  __str_push(' ');
  __str_push('<');
  syx_string_t name = method->method_field->name;
  __str_convert(stringify_string_n, name.items, name.count);
  __str_push('>');
  __str_push(')');
  return __str_width();
}

syx_string_t stringify_syx_boxed_method(Syx_Boxed_Method *method) {
  __stringify_body(stringify_syx_boxed_method_n, 256, method);
}

void sb_append_syx_boxed_method(String_Builder *sb, Syx_Boxed_Method *method) {
  __sb_append_body(stringify_syx_boxed_method_n, 256, method);
}

#endif // SYX_BOXED_IMPL
