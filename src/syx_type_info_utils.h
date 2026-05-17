#ifndef SYX_TYPE_INFO_UTILS_H
#define SYX_TYPE_INFO_UTILS_H

#include "syx_type_info.h"
#include "syx_utils.h"
#include "syx_value.h"

SyxV *syx_type_info_to_syxv(Syx_Eval_Ctx *ctx, Syx_Type_Info *typeinfo, void *data);
SyxV *syx_type_info_from_syxv(Syx_Eval_Ctx *ctx, Syx_Type_Info *typeinfo, void *data, SyxV *argument);

#endif // SYX_TYPE_INFO_UTILS_H

#if defined(SYX_TYPE_INFO_UTILS_IMPL) && !defined(SYX_TYPE_INFO_UTILS_IMPL_C)
#define SYX_TYPE_INFO_UTILS_IMPL_C

SyxV *syx_type_info_to_syxv(Syx_Eval_Ctx *ctx, Syx_Type_Info *typeinfo, void *data) {
  switch (typeinfo->kind) {
    case SYX_TYPE_INFO_KIND_PTR: TODO("implement ptr getter");
    case SYX_TYPE_INFO_KIND_STRUCTURE: TODO("implement structure getter");
    case SYX_TYPE_INFO_KIND_FUNCTION: TODO("implement function getter");
    case SYX_TYPE_INFO_KIND_VOID: RUNTIME_ERROR(ctx, "can not get void value");
    case SYX_TYPE_INFO_KIND_I8: return make_syxv_number_integer(*((SYX_TYPE_I8 *)data));
    case SYX_TYPE_INFO_KIND_I16: return make_syxv_number_integer(*((SYX_TYPE_I16 *)data));
    case SYX_TYPE_INFO_KIND_I32: return make_syxv_number_integer(*((SYX_TYPE_I32 *)data));
    case SYX_TYPE_INFO_KIND_I64: return make_syxv_number_integer(*((SYX_TYPE_I64 *)data));
#ifdef __SIZEOF_INT128__
    case SYX_TYPE_INFO_KIND_I128: return make_syxv_number_integer(*((SYX_TYPE_I128 *)data));
#endif
    case SYX_TYPE_INFO_KIND_U8: return make_syxv_number_integer(*((SYX_TYPE_U8 *)data));
    case SYX_TYPE_INFO_KIND_U16: return make_syxv_number_integer(*((SYX_TYPE_U16 *)data));
    case SYX_TYPE_INFO_KIND_U32: return make_syxv_number_integer(*((SYX_TYPE_U32 *)data));
    case SYX_TYPE_INFO_KIND_U64: return make_syxv_number_integer(*((SYX_TYPE_U64 *)data));
#ifdef __SIZEOF_INT128__
    case SYX_TYPE_INFO_KIND_U128: return make_syxv_number_integer(*((SYX_TYPE_U128 *)data));
#endif
    case SYX_TYPE_INFO_KIND_INT: return make_syxv_number_integer(*((SYX_TYPE_INT *)data));
    case SYX_TYPE_INFO_KIND_INT_LONG: return make_syxv_number_integer(*((SYX_TYPE_INT_LONG *)data));
    case SYX_TYPE_INFO_KIND_INT_LONG_LONG: return make_syxv_number_integer(*((SYX_TYPE_INT_LONG_LONG *)data));
    case SYX_TYPE_INFO_KIND_UINT: return make_syxv_number_integer(*((SYX_TYPE_UINT *)data));
    case SYX_TYPE_INFO_KIND_UINT_LONG: return make_syxv_number_integer(*((SYX_TYPE_UINT_LONG *)data));
    case SYX_TYPE_INFO_KIND_UINT_LONG_LONG: return make_syxv_number_integer(*((SYX_TYPE_UINT_LONG_LONG *)data));
#if defined(__STDC_IEC_60559_TYPES__) || defined(__clang__) && defined(__is_identifier) && !__is_identifier(_Float32)
    case SYX_TYPE_INFO_KIND_F16: return make_syxv_number_fractional(*((SYX_TYPE_F16 *)data));
    case SYX_TYPE_INFO_KIND_F32: return make_syxv_number_fractional(*((SYX_TYPE_F32 *)data));
    case SYX_TYPE_INFO_KIND_F64: return make_syxv_number_fractional(*((SYX_TYPE_F64 *)data));
    case SYX_TYPE_INFO_KIND_F128: return make_syxv_number_fractional(*((SYX_TYPE_F128 *)data));
#endif
    case SYX_TYPE_INFO_KIND_FLOAT: return make_syxv_number_fractional(*((SYX_TYPE_FLOAT *)data));
    case SYX_TYPE_INFO_KIND_DOUBLE: return make_syxv_number_fractional(*((SYX_TYPE_DOUBLE *)data));
    case SYX_TYPE_INFO_KIND_DOUBLE_LONG: return make_syxv_number_fractional(*((SYX_TYPE_DOUBLE_LONG *)data));
#ifdef __SIZEOF_SIZE_T__
    case SYX_TYPE_INFO_KIND_SIZE: return make_syxv_number_integer(*((SYX_TYPE_SIZE *)data));
#endif
    default: RUNTIME_ERROR(ctx, temp_sprintf("kind is not supported: %u '%s'", typeinfo->kind, syx_type_info_kind_name(typeinfo->kind)));
  }
}

SyxV *syx_type_info_from_syxv(Syx_Eval_Ctx *ctx, Syx_Type_Info *typeinfo, void *data, SyxV *argument) {
  switch (typeinfo->kind) {
    case SYX_TYPE_INFO_KIND_PTR: TODO("implement ptr setter");
    case SYX_TYPE_INFO_KIND_STRUCTURE: TODO("implement structure setter");
    case SYX_TYPE_INFO_KIND_FUNCTION: TODO("implement function setter");
    case SYX_TYPE_INFO_KIND_VOID: RUNTIME_ERROR(ctx, "can not set to void value");
    default: {
      Syx_Number number = {0};
      syx_convert_to(ctx, argument, &number);
      switch (typeinfo->kind) {
        case SYX_TYPE_INFO_KIND_I8: (*((SYX_TYPE_I8 *)data)) = syx_number_integer_value(number); break;
        case SYX_TYPE_INFO_KIND_I16: (*((SYX_TYPE_I16 *)data)) = syx_number_integer_value(number); break;
        case SYX_TYPE_INFO_KIND_I32: (*((SYX_TYPE_I32 *)data)) = syx_number_integer_value(number); break;
        case SYX_TYPE_INFO_KIND_I64: (*((SYX_TYPE_I64 *)data)) = syx_number_integer_value(number); break;
#ifdef __SIZEOF_INT128__
        case SYX_TYPE_INFO_KIND_I128: (*((SYX_TYPE_I128 *)data)) = syx_number_integer_value(number); break;
#endif
        case SYX_TYPE_INFO_KIND_U8: (*((SYX_TYPE_U8 *)data)) = syx_number_integer_value(number); break;
        case SYX_TYPE_INFO_KIND_U16: (*((SYX_TYPE_U16 *)data)) = syx_number_integer_value(number); break;
        case SYX_TYPE_INFO_KIND_U32: (*((SYX_TYPE_U32 *)data)) = syx_number_integer_value(number); break;
        case SYX_TYPE_INFO_KIND_U64: (*((SYX_TYPE_U64 *)data)) = syx_number_integer_value(number); break;
#ifdef __SIZEOF_INT128__
        case SYX_TYPE_INFO_KIND_U128: (*((SYX_TYPE_U128 *)data)) = syx_number_integer_value(number); break;
#endif
        case SYX_TYPE_INFO_KIND_INT: (*((SYX_TYPE_INT *)data)) = syx_number_integer_value(number); break;
        case SYX_TYPE_INFO_KIND_INT_LONG: (*((SYX_TYPE_INT_LONG *)data)) = syx_number_integer_value(number); break;
        case SYX_TYPE_INFO_KIND_INT_LONG_LONG: (*((SYX_TYPE_INT_LONG_LONG *)data)) = syx_number_integer_value(number); break;
        case SYX_TYPE_INFO_KIND_UINT: (*((SYX_TYPE_UINT *)data)) = syx_number_integer_value(number); break;
        case SYX_TYPE_INFO_KIND_UINT_LONG: (*((SYX_TYPE_UINT_LONG *)data)) = syx_number_integer_value(number); break;
        case SYX_TYPE_INFO_KIND_UINT_LONG_LONG: (*((SYX_TYPE_UINT_LONG_LONG *)data)) = syx_number_integer_value(number); break;
#if defined(__STDC_IEC_60559_TYPES__) || defined(__clang__) && defined(__is_identifier) && !__is_identifier(_Float32)
        case SYX_TYPE_INFO_KIND_F16: (*((SYX_TYPE_F16 *)data)) = syx_number_fractional_value(number); break;
        case SYX_TYPE_INFO_KIND_F32: (*((SYX_TYPE_F32 *)data)) = syx_number_fractional_value(number); break;
        case SYX_TYPE_INFO_KIND_F64: (*((SYX_TYPE_F64 *)data)) = syx_number_fractional_value(number); break;
        case SYX_TYPE_INFO_KIND_F128: (*((SYX_TYPE_F128 *)data)) = syx_number_fractional_value(number); break;
#endif
        case SYX_TYPE_INFO_KIND_FLOAT: (*((SYX_TYPE_FLOAT *)data)) = syx_number_fractional_value(number); break;
        case SYX_TYPE_INFO_KIND_DOUBLE: (*((SYX_TYPE_DOUBLE *)data)) = syx_number_fractional_value(number); break;
        case SYX_TYPE_INFO_KIND_DOUBLE_LONG: (*((SYX_TYPE_DOUBLE_LONG *)data)) = syx_number_fractional_value(number); break;
#ifdef __SIZEOF_SIZE_T__
        case SYX_TYPE_INFO_KIND_SIZE: (*((SYX_TYPE_SIZE *)data)) = syx_number_integer_value(number); break;
#endif
        default: RUNTIME_ERROR(ctx, temp_sprintf("kind is not supported: %u '%s'", typeinfo->kind, syx_type_info_kind_name(typeinfo->kind)));
      }
    }
  }
  return NULL;
}

#endif // SYX_TYPE_INFO_UTILS_IMPL
