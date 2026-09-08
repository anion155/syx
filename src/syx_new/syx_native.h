#ifndef SYX_NATIVE_H
#define SYX_NATIVE_H

#include <syx_new/syx_value.h>

Syx_Value *syx_eval_construct_native(Syx_Eval_Ctx *ctx, Syx_Type *type, Syx_Pair *arguments);

#endif // SYX_NATIVE_H

#if defined(SYX_NATIVE_IMPL) && !defined(SYX_NATIVE_IMPL_C)
#define SYX_NATIVE_IMPL_C

Syx_Value *syx_eval_construct_native(Syx_Eval_Ctx *ctx, Syx_Type *type, Syx_Pair *arguments) {
  Syx_Value *value = make_syx_value_native(type, 0);
  void *data = value->native->data;
  switch (type->kind) {
    case SYX_TYPE_KIND_PRIMITIVE: {
      if (type->primitive == SYX_PRIMITIVE_TYPE_KIND_VOID) SYX_EVAL_THROW(ctx, "primitive native void value is not constructible", (), (value));
      Syx_Value *argument = syx_list_next(&arguments);
      if (argument->kind != SYX_VALUE_KIND_PAIR || argument->pair) {
        Syx_Value *evaluated = rc_acquire(syx_eval(ctx, argument));
        syx_value_early_exit(evaluated);
        Syx_Value *converted = NULL;
        if (evaluated->kind == SYX_VALUE_KIND_NATIVE && evaluated->native->type->kind == SYX_TYPE_KIND_PRIMITIVE && evaluated->native->type->primitive == type->primitive) {
          converted = rc_acquire(evaluated);
        } else {
          converted = rc_acquire(syx_convert_to_number(ctx, evaluated));
          syx_value_early_exit(converted, (evaluated));
        }
        rc_release(evaluated);
        switch (type->primitive) {
          case SYX_PRIMITIVE_TYPE_KIND_VOID: UNREACHABLE("should be filtered already");
          case SYX_PRIMITIVE_TYPE_KIND_CHAR: *(char *)data = syx_number_get(converted->number); break;
          case SYX_PRIMITIVE_TYPE_KIND_I8: *(int8_t *)data = syx_number_get(converted->number); break;
          case SYX_PRIMITIVE_TYPE_KIND_I16: *(int16_t *)data = syx_number_get(converted->number); break;
          case SYX_PRIMITIVE_TYPE_KIND_I32: *(int32_t *)data = syx_number_get(converted->number); break;
          case SYX_PRIMITIVE_TYPE_KIND_I64: *(int64_t *)data = syx_number_get(converted->number); break;
          case SYX_PRIMITIVE_TYPE_KIND_I128: *(__int128_t *)data = syx_number_get(converted->number); break;
          case SYX_PRIMITIVE_TYPE_KIND_U8: *(uint8_t *)data = syx_number_get(converted->number); break;
          case SYX_PRIMITIVE_TYPE_KIND_U16: *(uint16_t *)data = syx_number_get(converted->number); break;
          case SYX_PRIMITIVE_TYPE_KIND_U32: *(uint32_t *)data = syx_number_get(converted->number); break;
          case SYX_PRIMITIVE_TYPE_KIND_U64: *(uint64_t *)data = syx_number_get(converted->number); break;
          case SYX_PRIMITIVE_TYPE_KIND_U128: *(__uint128_t *)data = syx_number_get(converted->number); break;
          case SYX_PRIMITIVE_TYPE_KIND_INT: *(int *)data = syx_number_get(converted->number); break;
          case SYX_PRIMITIVE_TYPE_KIND_LONG: *(long *)data = syx_number_get(converted->number); break;
          case SYX_PRIMITIVE_TYPE_KIND_LLONG: *(long long *)data = syx_number_get(converted->number); break;
          case SYX_PRIMITIVE_TYPE_KIND_UINT: *(unsigned int *)data = syx_number_get(converted->number); break;
          case SYX_PRIMITIVE_TYPE_KIND_ULONG: *(unsigned long *)data = syx_number_get(converted->number); break;
          case SYX_PRIMITIVE_TYPE_KIND_ULLONG: *(unsigned long long *)data = syx_number_get(converted->number); break;
          case SYX_PRIMITIVE_TYPE_KIND_FLOAT: *(float *)data = syx_number_get(converted->number); break;
          case SYX_PRIMITIVE_TYPE_KIND_DOUBLE: *(double *)data = syx_number_get(converted->number); break;
          case SYX_PRIMITIVE_TYPE_KIND_SIZE: *(size_t *)data = syx_number_get(converted->number); break;
        }
      }
    } break;
    case SYX_TYPE_KIND_STRUCTURE: {
      if (type->structure->constructor) {
        Syx_Value *result = rc_acquire(type->structure->constructor(ctx, data, arguments));
        syx_value_early_exit(result, (value));
        rc_release(result);
      }
      rc_get(value)->methods.destructor = syx_value_native_structure_destructor;
    } break;
    case SYX_TYPE_KIND_PTR:
    case SYX_TYPE_KIND_FUNCTION_PTR:
    case SYX_TYPE_KIND_VALUE_PTR: {
      *(void **)data = NULL;
      Syx_Value *argument = syx_list_next(&arguments);
      switch (argument->kind) {
        case SYX_VALUE_KIND_NUMBER: {
          switch (argument->number->kind) {
            case SYX_NUMBER_KIND_INTEGER: *(void **)data = (void *)(uintptr_t)argument->number->integer; break;
            case SYX_NUMBER_KIND_FRACTIONAL: SYX_EVAL_THROW(ctx, "invalid argument type", (), (value));
          }
        } break;
        case SYX_VALUE_KIND_NATIVE: {
          Syx_Type *argument_type = argument->native->type;
          switch (argument_type->kind) {
            case SYX_TYPE_KIND_PRIMITIVE: {
#define CONVERT(type) ({                                                                               \
  SYX_EVAL_ASSERT(ctx, sizeof(type) <= sizeof(void *), "cast from smaller integer type", (), (value)); \
  *(void **)data = (void *)(uintptr_t)*(type *)argument->native->data;                                 \
})
              switch (argument_type->primitive) {
                case SYX_PRIMITIVE_TYPE_KIND_VOID: SYX_EVAL_THROW(ctx, "invalid argument type", (), (value));
                case SYX_PRIMITIVE_TYPE_KIND_CHAR: CONVERT(char);
                case SYX_PRIMITIVE_TYPE_KIND_I8: CONVERT(int8_t); break;
                case SYX_PRIMITIVE_TYPE_KIND_I16: CONVERT(int16_t); break;
                case SYX_PRIMITIVE_TYPE_KIND_I32: CONVERT(int32_t); break;
                case SYX_PRIMITIVE_TYPE_KIND_I64: CONVERT(int64_t); break;
                case SYX_PRIMITIVE_TYPE_KIND_I128: CONVERT(__int128_t); break;
                case SYX_PRIMITIVE_TYPE_KIND_U8: CONVERT(uint8_t); break;
                case SYX_PRIMITIVE_TYPE_KIND_U16: CONVERT(uint16_t); break;
                case SYX_PRIMITIVE_TYPE_KIND_U32: CONVERT(uint32_t); break;
                case SYX_PRIMITIVE_TYPE_KIND_U64: CONVERT(uint64_t); break;
                case SYX_PRIMITIVE_TYPE_KIND_U128: CONVERT(__uint128_t); break;
                case SYX_PRIMITIVE_TYPE_KIND_INT: CONVERT(int); break;
                case SYX_PRIMITIVE_TYPE_KIND_LONG: CONVERT(long); break;
                case SYX_PRIMITIVE_TYPE_KIND_LLONG: CONVERT(long long); break;
                case SYX_PRIMITIVE_TYPE_KIND_UINT: CONVERT(unsigned int); break;
                case SYX_PRIMITIVE_TYPE_KIND_ULONG: CONVERT(unsigned long); break;
                case SYX_PRIMITIVE_TYPE_KIND_ULLONG: CONVERT(unsigned long long); break;
                case SYX_PRIMITIVE_TYPE_KIND_FLOAT: SYX_EVAL_THROW(ctx, "invalid argument type", (), (value));
                case SYX_PRIMITIVE_TYPE_KIND_DOUBLE: SYX_EVAL_THROW(ctx, "invalid argument type", (), (value));
                case SYX_PRIMITIVE_TYPE_KIND_SIZE: CONVERT(size_t); break;
              }
#undef CONVERT
            } break;
            case SYX_TYPE_KIND_STRUCTURE: SYX_EVAL_THROW(ctx, "invalid argument type", (), (value));
            case SYX_TYPE_KIND_PTR:
            case SYX_TYPE_KIND_FUNCTION_PTR:
            case SYX_TYPE_KIND_VALUE_PTR: {
              *(void **)data = *(void **)argument->native->data;
            } break;
          }
        } break;
        default: SYX_EVAL_THROW(ctx, "invalid argument type", (), (value));
      }
    } break;
  }
  return value;
}

#endif // SYX_NATIVE_IMPL_C
