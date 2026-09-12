#ifndef SYX_NATIVE_H
#define SYX_NATIVE_H

#include <syx/syx_value.h>

Syx_Value *syx_eval_construct_native(Syx_Eval_Ctx *ctx, Syx_Type *type, Syx_Pair *arguments);
Syx_Value *syx_native_set(Syx_Eval_Ctx *ctx, Syx_Type *type, void *data, Syx_Value *argument);
Syx_Value *syx_native_structure_set(Syx_Eval_Ctx *ctx, Syx_Type_Structure *structure, void *data, Syx_Symbol *field, Syx_Value *argument);
Syx_Value *syx_native_structure_update(Syx_Eval_Ctx *ctx, Syx_Native *native, Syx_Type_Structure *structure, Syx_Pair **arguments);

#define syx_native_primitive_xy_macro(type, X, Y) ({                     \
  switch (type->primitive) {                                             \
    case SYX_PRIMITIVE_TYPE_KIND_CHAR: X(char); break;                   \
    case SYX_PRIMITIVE_TYPE_KIND_I8: X(int8_t); break;                   \
    case SYX_PRIMITIVE_TYPE_KIND_I16: X(int16_t); break;                 \
    case SYX_PRIMITIVE_TYPE_KIND_I32: X(int32_t); break;                 \
    case SYX_PRIMITIVE_TYPE_KIND_I64: X(int64_t); break;                 \
    case SYX_PRIMITIVE_TYPE_KIND_I128: X(__int128_t); break;             \
    case SYX_PRIMITIVE_TYPE_KIND_U8: X(uint8_t); break;                  \
    case SYX_PRIMITIVE_TYPE_KIND_U16: X(uint16_t); break;                \
    case SYX_PRIMITIVE_TYPE_KIND_U32: X(uint32_t); break;                \
    case SYX_PRIMITIVE_TYPE_KIND_U64: X(uint64_t); break;                \
    case SYX_PRIMITIVE_TYPE_KIND_U128: X(__uint128_t); break;            \
    case SYX_PRIMITIVE_TYPE_KIND_F16: Y(f16_canonical_t); break;         \
    case SYX_PRIMITIVE_TYPE_KIND_F32: Y(float); break;                   \
    case SYX_PRIMITIVE_TYPE_KIND_F64: Y(double); break;                  \
    case SYX_PRIMITIVE_TYPE_KIND_F80: Y(f80_canonical_t); break;         \
    case SYX_PRIMITIVE_TYPE_KIND_F128: Y(f128_canonical_t); break;       \
    case SYX_PRIMITIVE_TYPE_KIND_F64PAIR: Y(f64pair_canonical_t); break; \
  };                                                                     \
})
#define syx_native_primitive_xy_macro2(type, X, Y) ({                    \
  switch (type->primitive) {                                             \
    case SYX_PRIMITIVE_TYPE_KIND_CHAR: X(char); break;                   \
    case SYX_PRIMITIVE_TYPE_KIND_I8: X(int8_t); break;                   \
    case SYX_PRIMITIVE_TYPE_KIND_I16: X(int16_t); break;                 \
    case SYX_PRIMITIVE_TYPE_KIND_I32: X(int32_t); break;                 \
    case SYX_PRIMITIVE_TYPE_KIND_I64: X(int64_t); break;                 \
    case SYX_PRIMITIVE_TYPE_KIND_I128: X(__int128_t); break;             \
    case SYX_PRIMITIVE_TYPE_KIND_U8: X(uint8_t); break;                  \
    case SYX_PRIMITIVE_TYPE_KIND_U16: X(uint16_t); break;                \
    case SYX_PRIMITIVE_TYPE_KIND_U32: X(uint32_t); break;                \
    case SYX_PRIMITIVE_TYPE_KIND_U64: X(uint64_t); break;                \
    case SYX_PRIMITIVE_TYPE_KIND_U128: X(__uint128_t); break;            \
    case SYX_PRIMITIVE_TYPE_KIND_F16: Y(f16_canonical_t); break;         \
    case SYX_PRIMITIVE_TYPE_KIND_F32: Y(float); break;                   \
    case SYX_PRIMITIVE_TYPE_KIND_F64: Y(double); break;                  \
    case SYX_PRIMITIVE_TYPE_KIND_F80: Y(f80_canonical_t); break;         \
    case SYX_PRIMITIVE_TYPE_KIND_F128: Y(f128_canonical_t); break;       \
    case SYX_PRIMITIVE_TYPE_KIND_F64PAIR: Y(f64pair_canonical_t); break; \
  };                                                                     \
})
#endif // SYX_NATIVE_H

#if defined(SYX_NATIVE_IMPL) && !defined(SYX_NATIVE_IMPL_C)
#define SYX_NATIVE_IMPL_C

#include <syx/syx_object.h>

Syx_Value *syx_eval_construct_native(Syx_Eval_Ctx *ctx, Syx_Type *type, Syx_Pair *arguments) {
  Syx_Value *value = make_syx_value_native_instance(type);
  void *data = value->native->data;
  switch (type->kind) {
    case SYX_TYPE_KIND_VOID: {
      SYX_EVAL_THROW(ctx, "native void value is not constructible", (), (value));
    } break;
    case SYX_TYPE_KIND_STRUCTURE: {
      if (type->structure->constructor) {
        Syx_Value *result = rc_acquire(type->structure->constructor(ctx, data, arguments));
        syx_value_early_exit(result, (value));
        rc_release(result);
        break;
      }
    }
    default: {
      Syx_Value *result = rc_acquire(syx_native_set(ctx, type, data, syx_list_next(&arguments)));
      syx_value_early_exit(result, (value));
      rc_release(result);
    }
  }
  return value;
}

Syx_Value *syx_native_set(Syx_Eval_Ctx *ctx, Syx_Type *type, void *data, Syx_Value *argument) {
  Syx_Value *evaluated = rc_acquire(syx_eval(ctx, argument));
  syx_value_early_exit(evaluated);
  switch (type->kind) {
    case SYX_TYPE_KIND_VOID: SYX_EVAL_THROW(ctx, "native void can't be set", (), (evaluated));
    case SYX_TYPE_KIND_PRIMITIVE: {
      switch (evaluated->kind) {
        case SYX_VALUE_KIND_PAIR: SYX_EVAL_THROW(ctx, "pair can't be converted to native primitive", (), (evaluated));
        case SYX_VALUE_KIND_CONST: {
          bool value;
          if (evaluated == syx_value_bool_true()) {
            value = true;
          } else if (evaluated == syx_value_bool_false()) {
            value = false;
          } else {
            SYX_EVAL_THROW(ctx, "const can't be converted to native pointer", (), (evaluated));
          }
#define X(type) *(type *)data = value
#define Y(type) *(type *)data = f_canonical_from_native(value, type)
          syx_native_primitive_xy_macro(type, X, Y);
#undef Y
#undef X
        } break;
        case SYX_VALUE_KIND_SYMBOL: SYX_EVAL_THROW(ctx, "symbol can't be converted to native primitive", (), (evaluated));
        case SYX_VALUE_KIND_NUMBER: {
#define X(type) *(type *)data = syx_number_get(evaluated->number)
#define Y(type) *(type *)data = f_canonical_from_native(syx_number_get(evaluated->number), type)
          syx_native_primitive_xy_macro(type, X, Y);
#undef Y
#undef X
        } break;
        case SYX_VALUE_KIND_STRING: SYX_EVAL_THROW(ctx, "string can't be converted to native primitive", (), (evaluated));
        case SYX_VALUE_KIND_OBJECT: SYX_EVAL_THROW(ctx, "object can't be converted to native primitive", (), (evaluated));
        case SYX_VALUE_KIND_CLOSURE: SYX_EVAL_THROW(ctx, "closure can't be converted to native primitive", (), (evaluated));
        case SYX_VALUE_KIND_NATIVE: {
          switch (evaluated->native->type->kind) {
            case SYX_TYPE_KIND_VOID: SYX_EVAL_THROW(ctx, "native void can't be converted to native primitive", (), (evaluated));
            case SYX_TYPE_KIND_PRIMITIVE: {
#define J(etype) *(typeof(tmp) *)data = *(etype *)evaluated->native->data
#define K(etype) *(typeof(tmp) *)data = f_canonical_to_native(*(etype *)evaluated->native->data)
#define X(ttype) ({                                              \
  ttype tmp;                                                     \
  UNUSED(tmp);                                                   \
  syx_native_primitive_xy_macro2(evaluated->native->type, J, K); \
})
#define N(etype) *(typeof(tmp) *)data = f_canonical_from_native(*(etype *)evaluated->native->data, typeof(tmp))
#define M(etype) *(typeof(tmp) *)data = f_canonical_from_native(f_canonical_to_native(*(etype *)evaluated->native->data), typeof(tmp))
#define Y(ttype) ({                                              \
  ttype tmp;                                                     \
  syx_native_primitive_xy_macro2(evaluated->native->type, N, M); \
})
              syx_native_primitive_xy_macro(type, X, Y);
#undef J
#undef K
#undef X
#undef Y
#undef N
#undef M
            } break;
            case SYX_TYPE_KIND_STRUCTURE: {
              if (type->size == evaluated->native->type->size) {
                memcpy(data, evaluated->native->data, type->size);
              } else {
                SYX_EVAL_THROW(ctx, "native structure can't be converted to native primitive", (), (evaluated));
              }
            } break;
            case SYX_TYPE_KIND_PTR:
            case SYX_TYPE_KIND_FUNCTION_PTR: {
#define X(type) ({                                                                                    \
  SYX_EVAL_ASSERT(ctx, sizeof(void *) >= sizeof(type), "native pointer can not be stored in " #type); \
  *(type *)data = (uintptr_t)*(void **)evaluated->native->data;                                       \
})
#define Y(type) SYX_EVAL_THROW(ctx, "native pointer value can't be converted to native primitive", (), (evaluated));
              syx_native_primitive_xy_macro(type, X, Y);
#undef X
#undef Y
            } break;
          }
        } break;
        case SYX_VALUE_KIND_EXIT: SYX_EVAL_THROW(ctx, "exit can't be converted to native primitive", (), (evaluated));
        case SYX_VALUE_KIND_PREFIXED: SYX_EVAL_THROW(ctx, "prefixed can't be converted to native primitive", (), (evaluated));
      }
    } break;
    case SYX_TYPE_KIND_STRUCTURE: {
      switch (evaluated->kind) {
        case SYX_VALUE_KIND_PAIR: SYX_EVAL_THROW(ctx, "pair can't be converted to native structure", (), (evaluated));
        case SYX_VALUE_KIND_CONST: SYX_EVAL_THROW(ctx, "const can't be converted to native structure", (), (evaluated));
        case SYX_VALUE_KIND_SYMBOL: SYX_EVAL_THROW(ctx, "symbol can't be converted to native structure", (), (evaluated));
        case SYX_VALUE_KIND_NUMBER: SYX_EVAL_THROW(ctx, "number can't be converted to native structure", (), (evaluated));
        case SYX_VALUE_KIND_STRING: SYX_EVAL_THROW(ctx, "string can't be converted to native structure", (), (evaluated));
        case SYX_VALUE_KIND_OBJECT: {
          ht_foreach(field, &evaluated->object->fields) {
            Syx_Symbol *symbol = ht_key(&evaluated->object->fields, *field);
            Syx_Value *result = rc_acquire(syx_native_structure_set(ctx, type->structure, data, symbol, *field));
            syx_value_early_exit(result, (evaluated));
          }
        } break;
        case SYX_VALUE_KIND_CLOSURE: SYX_EVAL_THROW(ctx, "closure can't be converted to native structure", (), (evaluated));
        case SYX_VALUE_KIND_NATIVE: {
          switch (evaluated->native->type->kind) {
            case SYX_TYPE_KIND_VOID: SYX_EVAL_THROW(ctx, "native void can't be converted to native structure", (), (evaluated));
            case SYX_TYPE_KIND_PRIMITIVE: {
              if (type->size == evaluated->native->type->size) {
                memcpy(data, evaluated->native->data, type->size);
              } else {
                SYX_EVAL_THROW(ctx, "native primitive can't be converted to native structure", (), (evaluated));
              }
            } break;
            case SYX_TYPE_KIND_STRUCTURE: {
              if (type->size == evaluated->native->type->size) {
                memcpy(data, evaluated->native->data, type->size);
              } else {
                SYX_EVAL_THROW(ctx, "native structure can't be converted to native structure", (), (evaluated));
              }
            } break;
            case SYX_TYPE_KIND_PTR: SYX_EVAL_THROW(ctx, "native pointer can't be converted to native structure", (), (evaluated));
            case SYX_TYPE_KIND_FUNCTION_PTR: SYX_EVAL_THROW(ctx, "native function can't be converted to native structure", (), (evaluated));
          }
        } break;
        case SYX_VALUE_KIND_EXIT: SYX_EVAL_THROW(ctx, "exit can't be converted to native structure", (), (evaluated));
        case SYX_VALUE_KIND_PREFIXED: SYX_EVAL_THROW(ctx, "prefixed can't be converted to native structure", (), (evaluated));
      }
    } break;
    case SYX_TYPE_KIND_PTR:
    case SYX_TYPE_KIND_FUNCTION_PTR: {
      switch (evaluated->kind) {
        case SYX_VALUE_KIND_PAIR: SYX_EVAL_THROW(ctx, "pair can't be converted to native pointer", (), (evaluated));
        case SYX_VALUE_KIND_CONST: SYX_EVAL_THROW(ctx, "const can't be converted to native pointer", (), (evaluated));
        case SYX_VALUE_KIND_SYMBOL: SYX_EVAL_THROW(ctx, "symbol can't be converted to native pointer", (), (evaluated));
        case SYX_VALUE_KIND_NUMBER: {
          switch (evaluated->number->kind) {
            case SYX_NUMBER_KIND_INTEGER: *(void **)data = (void *)(uintptr_t)evaluated->number->integer; break;
            case SYX_NUMBER_KIND_FRACTIONAL: SYX_EVAL_THROW(ctx, "floating number can't be converted to native pointer", (), (evaluated));
          }
        } break;
        case SYX_VALUE_KIND_STRING: SYX_EVAL_THROW(ctx, "string can't be converted to native pointer", (), (evaluated));
        case SYX_VALUE_KIND_OBJECT: SYX_EVAL_THROW(ctx, "object can't be converted to native pointer", (), (evaluated));
        case SYX_VALUE_KIND_CLOSURE: SYX_EVAL_THROW(ctx, "closure can't be converted to native pointer", (), (evaluated));
        case SYX_VALUE_KIND_NATIVE: {
          switch (evaluated->native->type->kind) {
            case SYX_TYPE_KIND_VOID: SYX_EVAL_THROW(ctx, "native void can't be converted to native pointer", (), (evaluated));
            case SYX_TYPE_KIND_PRIMITIVE: {
#define X(type) ({                                                                                    \
  SYX_EVAL_ASSERT(ctx, sizeof(void *) >= sizeof(type), #type " can not be stored in native pointer"); \
  *(void **)data = (void *)(uintptr_t)*(type *)evaluated->native->data;                               \
})
#define Y(type) SYX_EVAL_THROW(ctx, "native floating value can't be converted to native pointer", (), (evaluated))
              syx_native_primitive_xy_macro(evaluated->native->type, X, Y);
#undef X
#undef Y
            } break;
            case SYX_TYPE_KIND_STRUCTURE: SYX_EVAL_THROW(ctx, "native structure can't be converted to native pointer", (), (evaluated));
            case SYX_TYPE_KIND_PTR:
            case SYX_TYPE_KIND_FUNCTION_PTR: {
              *(void **)data = *(void **)evaluated->native->data;
            } break;
          }
        } break;
        case SYX_VALUE_KIND_EXIT: SYX_EVAL_THROW(ctx, "exit can't be converted to native pointer", (), (evaluated));
        case SYX_VALUE_KIND_PREFIXED: SYX_EVAL_THROW(ctx, "prefixed can't be converted to native pointer", (), (evaluated));
      }
    } break;
    default: SYX_EVAL_TODO(ctx, TODO_DEFAULT_MESSAGE, (), (evaluated));
  }
  rc_release(evaluated);
  return syx_value_nil();
}

Syx_Value *syx_native_structure_set(Syx_Eval_Ctx *ctx, Syx_Type_Structure *structure, void *data, Syx_Symbol *field, Syx_Value *argument) {
  Syx_Type_Structure_Field *field_desc = syx_native_structure_get_field(structure, field);
  SYX_EVAL_ASSERT(ctx, field_desc, "native structure has no such field");
  Syx_Value *result = rc_acquire(syx_native_set(ctx, field_desc->type, (void *)((char *)data + field_desc->offset), argument));
  syx_value_early_exit(result);
  rc_release(result);
  return syx_value_nil();
}

Syx_Value *syx_native_structure_update(Syx_Eval_Ctx *ctx, Syx_Native *native, Syx_Type_Structure *structure, Syx_Pair **arguments) {
  while (*arguments) {
    Syx_Value *field = rc_acquire(syx_eval_unquote(ctx, syx_list_next(arguments)));
    syx_value_early_exit(field);
    SYX_EVAL_ASSERT(ctx, field->kind == SYX_VALUE_KIND_PREFIXED, "colon prefixed symbol expected", (), (field));
    SYX_EVAL_ASSERT(ctx, field->prefixed->kind == SYX_PREFIXED_KIND_COLON, "colon prefixed symbol expected", (), (field));
    SYX_EVAL_ASSERT(ctx, field->prefixed->value->kind == SYX_VALUE_KIND_SYMBOL, "colon prefixed symbol expected", (), (field));
    Syx_Type_Structure_Field *field_desc = syx_native_structure_get_field(structure, field->prefixed->value->symbol);
    SYX_EVAL_ASSERT(ctx, field_desc, "native structure has no such field");

    Syx_Value *value = rc_acquire(syx_eval_unquote(ctx, syx_list_next(arguments)));
    syx_value_early_exit(value, (field));

    Syx_Value *result = rc_acquire(syx_native_structure_set(ctx, structure, native->data, field->symbol, value));
    syx_value_early_exit(result, (field, value));
    rc_release_all(result, field, value);
  }
  return syx_value_nil();
}

#endif // SYX_NATIVE_IMPL_C
