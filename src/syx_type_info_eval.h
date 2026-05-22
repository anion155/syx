#ifndef SYX_TYPE_INFO_EVAL_H
#define SYX_TYPE_INFO_EVAL_H

#include "syx_type_info.h"

typedef struct Syx_Boxed {
  Syx_Type_Info *typeinfo;
  void *data;
  Syx_Boxed *parent;
} Syx_Boxed;

Syx_Boxed *make_syx_boxed_opt(Syx_Boxed opt);
#define make_syx_boxed(...) make_syx_boxed_opt((Syx_Boxed){__VA_ARGS__})
SyxV *syx_boxed_set(Syx_Eval_Ctx *ctx, Syx_Boxed *boxed, SyxV *argument);
SyxV *syxv_eval_boxed_construct(Syx_Eval_Ctx *ctx, Syx_Type_Info *typeinfo, SyxV *arguments);
SyxV *syxv_eval_boxed(Syx_Eval_Ctx *ctx, Syx_Boxed *boxed, SyxV *arguments);

size_t stringify_syx_boxed_n(char *string, Syx_Boxed *boxed);
String_Builder stringify_syx_type_info_instance(Syx_Boxed *boxed);
void sb_append_syx_type_info_instance(String_Builder *sb, Syx_Boxed *boxed);

#endif // SYX_TYPE_INFO_EVAL_H

#if defined(SYX_TYPE_INFO_EVAL_IMPL) && !defined(SYX_TYPE_INFO_EVAL_IMPL_C)
#define SYX_TYPE_INFO_EVAL_IMPL_C

#define SYX_EVAL_IMPL
#include "syx_eval.h"
#define SYX_TYPE_INFO_EVAL_IMPL
#include "syx_type_info.h"

void syxv_eval_boxed_deconstruct(void *data) {
  Syx_Boxed *boxed = data;
  if (boxed->parent) {
    rc_release(boxed->parent);
    return;
  }
  if (boxed->typeinfo && boxed->data) {
    switch (boxed->typeinfo->kind) {
      case SYX_TYPE_INFO_KIND_PTR: {
        Syx_Boxed nested = {.typeinfo = rc_acquire(boxed->typeinfo->ptr), .data = *(void **)boxed->data, .parent = NULL};
        syxv_eval_boxed_deconstruct(&nested);
        TODO("need to somehow release and free this pointer");
      } break;
      case SYX_TYPE_INFO_KIND_STRUCTURE: {
        if (boxed->typeinfo->structure.destructor) {
          boxed->typeinfo->structure.destructor(boxed->data);
        }
      } break;
      default:
    }
  }
  if (boxed->typeinfo) rc_release(boxed->typeinfo);
  if (boxed->data) free(boxed->data);
}

Syx_Boxed *make_syx_boxed_opt(Syx_Boxed opt) {
  Syx_Boxed *boxed = rc_malloc(sizeof(Syx_Boxed), .destructor = syxv_eval_boxed_deconstruct);
  assert(boxed);
  boxed->typeinfo = opt.typeinfo ? rc_acquire(opt.typeinfo) : NULL;
  boxed->data = opt.data;
  boxed->parent = opt.parent ? rc_acquire(opt.parent) : NULL;
  return boxed;
}

SyxV *syx_boxed_set(Syx_Eval_Ctx *ctx, Syx_Boxed *boxed, SyxV *argument) {
  switch (boxed->typeinfo->kind) {
    case SYX_TYPE_INFO_KIND_PTR: {
      if (argument->kind != SYXV_KIND_BOXED) RUNTIME_ERROR(ctx, "boxed pointer expected");
      if (argument->boxed->typeinfo->kind != SYX_TYPE_INFO_KIND_PTR) RUNTIME_ERROR(ctx, "boxed pointer expected");
      if (*(void **)boxed->data) TODO("need to somehow release this pointer");
      TODO("need to acquire this pointer");
      *(void **)boxed->data = argument->boxed->data;
    } break;
    case SYX_TYPE_INFO_KIND_FUNCTION_PTR: {
      if (argument->kind != SYXV_KIND_BOXED) RUNTIME_ERROR(ctx, "boxed function expected");
      if (argument->boxed->typeinfo->kind != SYX_TYPE_INFO_KIND_FUNCTION_PTR) RUNTIME_ERROR(ctx, "boxed function expected");
      boxed->data = argument->boxed->data;
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

SyxV *syxv_eval_boxed_construct(Syx_Eval_Ctx *ctx, Syx_Type_Info *typeinfo, SyxV *arguments) {
  Syx_Boxed *boxed;
  {
    void *data = malloc(typeinfo->size);
    if (!data) RUNTIME_ERROR(ctx, "failed to allocate boxed value");
    boxed = make_syx_boxed_opt((Syx_Boxed){.typeinfo = typeinfo, .data = data, .parent = NULL});
  }
  rc_acquire(boxed);
  memset(boxed->data, 0, typeinfo->size);

  SyxV *argument = arguments->pair.left;
  if (argument->kind == SYXV_KIND_BOXED && argument->boxed->typeinfo->kind == typeinfo->kind) {
    SyxV *result = syx_boxed_set(ctx, boxed, syxv_list_next(&arguments));
    syx_eval_early_exit(result, boxed);
    rc_acquire(result);
    rc_release(result);
  } else {
    switch (typeinfo->kind) {
      case SYX_TYPE_INFO_KIND_PTR: {
        SyxV *nested = syxv_eval_boxed_construct(ctx, typeinfo->ptr, arguments);
        syx_eval_early_exit(nested, boxed);
        TODO("need to acquire this pointer");
        *(void **)boxed->data = nested->boxed->data;
        nested->boxed->data = NULL;
        rc_release(nested);
      } break;
      case SYX_TYPE_INFO_KIND_FUNCTION_PTR: {
        RUNTIME_ERROR(ctx, "can not instantiate function", boxed);
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
  }

  return make_syxv_boxed(rc_move(boxed));
}

SyxV *syxv_eval_boxed_structure_getter(Syx_Eval_Ctx *ctx, Syx_Boxed *boxed, const char *typeinfo_string, SyxV *field_arg, SyxV **arguments) {
  SyxV *result;
  if (field_arg->kind == SYXV_KIND_NUMBER && field_arg->number.kind == SYX_NUMBER_KIND_INTEGER) {
    syx_integer_t index = field_arg->number.integer;
    if (!boxed->typeinfo->structure.index_getter) RUNTIME_ERROR(ctx, "no index getter");
    syx_ctx_push_frame(ctx, temp_sprintf("(%s get %lld)", typeinfo_string, index));
    result = boxed->typeinfo->structure.index_getter(ctx, boxed->data, index);
    rc_acquire(result);
    syx_ctx_pop_frame(ctx);
    rc_move(result);
  } else if (field_arg->kind == SYXV_KIND_SYMBOL) {
    const char *field_name = field_arg->symbol.name;
    Syx_Type_Info_Structure_Field *field = ht_find(&boxed->typeinfo->structure.fields, field_name);
    if (field) {
      switch (field->kind) {
        case SYX_TYPE_INFO_STRUCTURE_FIELD_KIND_DATA: {
          void *field_data = boxed->data + field->data.offset;
          result = make_syxv_boxed(make_syx_boxed_opt((Syx_Boxed){.typeinfo = field->data.typeinfo, .data = field_data, .parent = boxed}));
        } break;
        case SYX_TYPE_INFO_STRUCTURE_FIELD_KIND_ACCESSOR: {
          if (!field->accessor.getter) RUNTIME_ERROR(ctx, "no accessor field getter");
          syx_ctx_push_frame(ctx, temp_sprintf("(%s get %s)", typeinfo_string, field_name));
          result = field->accessor.getter(ctx, boxed->data);
          rc_acquire(result);
          syx_ctx_pop_frame(ctx);
          rc_move(result);
        } break;
        case SYX_TYPE_INFO_STRUCTURE_FIELD_KIND_METHOD: {
          SyxV *evaluated = syx_eval_list(ctx, *arguments);
          syx_eval_early_exit(evaluated);
          rc_acquire(evaluated);
          *arguments = make_syxv_nil();
          syx_ctx_push_frame(ctx, temp_sprintf("(%s %s)", typeinfo_string, field_name));
          result = field->method(ctx, boxed->data, evaluated);
          rc_acquire(result);
          syx_ctx_pop_frame(ctx);
          rc_release(evaluated);
          rc_move(result);
        } break;
      }
    } else if (boxed->typeinfo->structure.field_getter) {
      syx_ctx_push_frame(ctx, temp_sprintf("(%s get %s)", typeinfo_string, field_name));
      result = boxed->typeinfo->structure.field_getter(ctx, boxed->data, field_name);
      rc_acquire(result);
      syx_ctx_pop_frame(ctx);
      rc_release(result);
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

SyxV *syxv_eval_boxed(Syx_Eval_Ctx *ctx, Syx_Boxed *boxed, SyxV *arguments) {
  Syx_Boxed *current = rc_acquire(boxed);
  SyxV *result = NULL;
  SyxV *unref_symbol = rc_acquire(make_syxv_symbol_cstr("unref"));
  syxv_list_for_each(argument, arguments) {
    if (result) rc_release(result);
    if (!current) RUNTIME_ERROR(ctx, "trying to access non boxed value", result);
    if (!current->data) RUNTIME_ERROR(ctx, "trying to access null boxed value", current, result);

    SyxV *field_arg = syx_eval_unquote(ctx, argument);
    syx_eval_early_exit(field_arg, unref_symbol, current, result);
    rc_acquire(field_arg);

    switch (current->typeinfo->kind) {
      case SYX_TYPE_INFO_KIND_PTR: {
        if (field_arg != unref_symbol) RUNTIME_ERROR(ctx, "can not access inner field of boxed ref value", unref_symbol, field_arg, current, result);
        result = make_syxv_boxed(make_syx_boxed_opt((Syx_Boxed){.typeinfo = current->typeinfo->ptr, .data = *(void **)current->data, .parent = current}));
      } break;
      case SYX_TYPE_INFO_KIND_STRUCTURE: {
        syx_string_t typeinfo_str = stringify_syx_type_info(boxed->typeinfo);
        result = syxv_eval_boxed_structure_getter(ctx, current, typeinfo_str.items, field_arg, &argument_next);
        sb_free(typeinfo_str);
      }; break;
      default: {
        const char *kind_name = syx_type_info_kind_name(current->typeinfo->kind);
        RUNTIME_ERROR(ctx, temp_sprintf("can not access inner fields of boxed value: '%s'", kind_name), unref_symbol, field_arg, current, result);
      }
    }
    syx_eval_early_exit(result, unref_symbol, field_arg, current);
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
  rc_release(unref_symbol);
  if (current) rc_release(current);
  return rc_move(result);
}

// unbox
// switch (current->typeinfo->kind) {
//           case SYX_TYPE_INFO_KIND_I8: result = make_syxv_number_integer(*(SYX_TYPE_I8 *)current->data); break;
//           case SYX_TYPE_INFO_KIND_I16: result = make_syxv_number_integer(*(SYX_TYPE_I16 *)current->data); break;
//           case SYX_TYPE_INFO_KIND_I32: result = make_syxv_number_integer(*(SYX_TYPE_I32 *)current->data); break;
//           case SYX_TYPE_INFO_KIND_I64: result = make_syxv_number_integer(*(SYX_TYPE_I64 *)current->data); break;
// #ifdef SYX_TYPE_I128_SUPPORTED
//           case SYX_TYPE_INFO_KIND_I128: result = make_syxv_number_integer(*(SYX_TYPE_I128 *)current->data); break;
// #endif
//           case SYX_TYPE_INFO_KIND_U8: result = make_syxv_number_integer(*(SYX_TYPE_U8 *)current->data); break;
//           case SYX_TYPE_INFO_KIND_U16: result = make_syxv_number_integer(*(SYX_TYPE_U16 *)current->data); break;
//           case SYX_TYPE_INFO_KIND_U32: result = make_syxv_number_integer(*(SYX_TYPE_U32 *)current->data); break;
//           case SYX_TYPE_INFO_KIND_U64: result = make_syxv_number_integer(*(SYX_TYPE_U64 *)current->data); break;
// #ifdef SYX_TYPE_I128_SUPPORTED
//           case SYX_TYPE_INFO_KIND_U128: result = make_syxv_number_integer(*(SYX_TYPE_U128 *)current->data); break;
// #endif
//           case SYX_TYPE_INFO_KIND_INT: result = make_syxv_number_integer(*(SYX_TYPE_INT *)current->data); break;
//           case SYX_TYPE_INFO_KIND_INT_LONG: result = make_syxv_number_integer(*(SYX_TYPE_INT_LONG *)current->data); break;
//           case SYX_TYPE_INFO_KIND_INT_LONG_LONG: result = make_syxv_number_integer(*(SYX_TYPE_INT_LONG_LONG *)current->data); break;
//           case SYX_TYPE_INFO_KIND_UINT: result = make_syxv_number_integer(*(SYX_TYPE_UINT *)current->data); break;
//           case SYX_TYPE_INFO_KIND_UINT_LONG: result = make_syxv_number_integer(*(SYX_TYPE_UINT_LONG *)current->data); break;
//           case SYX_TYPE_INFO_KIND_UINT_LONG_LONG: result = make_syxv_number_integer(*(SYX_TYPE_UINT_LONG_LONG *)current->data); break;
// #ifdef SYX_TYPE_SIZED_FLOAT_SUPPORTED
//           case SYX_TYPE_INFO_KIND_F16: result = make_syxv_number_fractional(*(SYX_TYPE_F16 *)current->data); break;
//           case SYX_TYPE_INFO_KIND_F32: result = make_syxv_number_fractional(*(SYX_TYPE_F32 *)current->data); break;
//           case SYX_TYPE_INFO_KIND_F64: result = make_syxv_number_fractional(*(SYX_TYPE_F64 *)current->data); break;
//           case SYX_TYPE_INFO_KIND_F128: result = make_syxv_number_fractional(*(SYX_TYPE_F128 *)current->data); break;
// #endif
//           case SYX_TYPE_INFO_KIND_FLOAT: result = make_syxv_number_fractional(*(SYX_TYPE_FLOAT *)current->data); break;
//           case SYX_TYPE_INFO_KIND_DOUBLE: result = make_syxv_number_fractional(*(SYX_TYPE_DOUBLE *)current->data); break;
//           case SYX_TYPE_INFO_KIND_DOUBLE_LONG: result = make_syxv_number_fractional(*(SYX_TYPE_DOUBLE_LONG *)current->data); break;
//           case SYX_TYPE_INFO_KIND_SIZE: result = make_syxv_number_integer(*(SYX_TYPE_SIZE *)current->data); break;
//         }

size_t stringify_syx_boxed_n(char *string, Syx_Boxed *boxed) {
  __str_it();
  __str_convert(stringify_syx_type_info_n, boxed->typeinfo);
  __str_push('(');
  switch (boxed->typeinfo->kind) {
    case SYX_TYPE_INFO_KIND_PTR: TODO("implement boxed pointer stringify");
    case SYX_TYPE_INFO_KIND_FUNCTION_PTR: TODO("implement boxed function pointer stringify");
    case SYX_TYPE_INFO_KIND_STRUCTURE: TODO("implement boxed structure pointer stringify");
    case SYX_TYPE_INFO_KIND_VOID: break;
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

String_Builder stringify_syx_type_info_instance(Syx_Boxed *boxed) {
  __stringify_body(stringify_syx_boxed_n, 256, boxed);
}

void sb_append_syx_type_info_instance(String_Builder *sb, Syx_Boxed *boxed) {
  __sb_append_body(stringify_syx_boxed_n, 256, boxed);
}

#endif // SYX_TYPE_INFO_EVAL_IMPL
