#ifndef SYX_PARSER_NATIVE_H
#define SYX_PARSER_NATIVE_H

#include <syx/syx_lexer.h>
#include <syx/syx_parser.h>

Syx_Value *syx_parse_native_value(Syx_Type *type, Syx_Tokens *tokens);

#endif // SYX_PARSER_NATIVE_H

#if defined(SYX_PARSER_NATIVE_IMPL) && !defined(SYX_PARSER_NATIVE_IMPL_C)
#define SYX_PARSER_NATIVE_IMPL_C

#define PARSER_IMPL
#include <parser.h>

Syx_Value *syx_parse_native_value(Syx_Type *type, Syx_Tokens *tokens) {
  Syx_Token token = da_slice_shift(tokens);
  SYX_ASSERT(token.kind == SYX_TOKEN_KIND_PREFIX && *token.source.data == ':', "type and value must be separated with :");
  switch (type->kind) {
    case SYX_TYPE_KIND_VOID: return make_syx_value_native_instance(type);
    case SYX_TYPE_KIND_PRIMITIVE: {
      Syx_Value *value = rc_acquire(make_syx_value_native_instance(type));
      token = da_slice_shift(tokens);
#define parse_int(base, type_name, prefix_check) ({         \
  bool negative = false;                                    \
  if (sv.data[0] == '-' || sv.data[0] == '+') {             \
    sv.data += 1;                                           \
    sv.count -= 1;                                          \
    negative = sv.data[0] == '-';                           \
  }                                                         \
  prefix_check;                                             \
  type_name number = 0;                                     \
  parse__number_unsigned_integer_value(                     \
      sv,                                                   \
      number,                                               \
      parse__number_##base##_shift,                         \
      parse__number_##base##_map,                           \
      SYX_THROW("expected integer number", (), (value)), ); \
  if (negative) number *= -1;                               \
  number;                                                   \
})
#define parse_frac(base, type_name, prefix_check) ({ \
  bool negative = false;                             \
  if (sv.data[0] == '-' || sv.data[0] == '+') {      \
    sv.data += 1;                                    \
    sv.count -= 1;                                   \
    negative = sv.data[0] == '-';                    \
  }                                                  \
  prefix_check;                                      \
  type_name number = {0};                            \
  parse__number_fractional_value(                    \
      sv,                                            \
      number,                                        \
      negative,                                      \
      parse__number_##base##_map,                    \
      SYX_THROW("expected fractional number"));      \
  number;                                            \
})
#define X(type_name) ({                                                                                                   \
  type_name *data = value->native->data;                                                                                  \
  String_View sv = token.source;                                                                                          \
  switch (token.kind) {                                                                                                   \
    case SYX_TOKEN_KIND_NAN: *data = NAN; break;                                                                          \
    case SYX_TOKEN_KIND_INFINITY_POSITIVE: *data = INFINITY; break;                                                       \
    case SYX_TOKEN_KIND_INFINITY_NEGATIVE: *data = (-INFINITY); break;                                                    \
    case SYX_TOKEN_KIND_BIN_INT_LIT: *data = parse_int(binary, type_name, sv_chop_left(&sv, 2)); break;                   \
    case SYX_TOKEN_KIND_OCT_INT_LIT: *data = parse_int(octal, type_name, sv_chop_left(&sv, 2)); break;                    \
    case SYX_TOKEN_KIND_DEC_INT_LIT: *data = parse_int(decimal, type_name, ); break;                                      \
    case SYX_TOKEN_KIND_HEX_INT_LIT: *data = parse_int(hex, type_name, sv_chop_left(&sv, 2)); break;                      \
    case SYX_TOKEN_KIND_BIN_FRC_LIT: *data = parse_frac(binary, double, sv_chop_left(&sv, 2)); break;                     \
    case SYX_TOKEN_KIND_OCT_FRC_LIT: *data = parse_frac(octal, double, sv_chop_left(&sv, 2)); break;                      \
    case SYX_TOKEN_KIND_DEC_FRC_LIT: SYX_TODO("SYX_TOKEN_KIND_DEC_FRC_LIT to native TASK(20260923-105431)", (), (value)); \
    case SYX_TOKEN_KIND_HEX_FRC_LIT: *data = parse_frac(hex, double, sv_chop_left(&sv, 2)); break;                        \
    default: SYX_THROW("expected number", (), (value));                                                                   \
  }                                                                                                                       \
})
#define Y(type_name) ({                                                                                                                \
  type_name *data = value->native->data;                                                                                               \
  String_View sv = token.source;                                                                                                       \
  switch (token.kind) {                                                                                                                \
    case SYX_TOKEN_KIND_NAN: *data = f_canonical_from_native(NAN, type_name); break;                                                   \
    case SYX_TOKEN_KIND_INFINITY_POSITIVE: *data = f_canonical_from_native(INFINITY, type_name); break;                                \
    case SYX_TOKEN_KIND_INFINITY_NEGATIVE: *data = f_canonical_from_native((-INFINITY), type_name); break;                             \
    case SYX_TOKEN_KIND_BIN_INT_LIT: *data = f_canonical_from_native(parse_int(binary, long, sv_chop_left(&sv, 2)), type_name); break; \
    case SYX_TOKEN_KIND_OCT_INT_LIT: *data = f_canonical_from_native(parse_int(octal, long, sv_chop_left(&sv, 2)), type_name); break;  \
    case SYX_TOKEN_KIND_DEC_INT_LIT: *data = f_canonical_from_native(parse_int(decimal, long, ), type_name); break;                    \
    case SYX_TOKEN_KIND_HEX_INT_LIT: *data = f_canonical_from_native(parse_int(hex, long, sv_chop_left(&sv, 2)), type_name); break;    \
    case SYX_TOKEN_KIND_BIN_FRC_LIT: *data = parse_frac(binary, type_name, sv_chop_left(&sv, 2)); break;                               \
    case SYX_TOKEN_KIND_OCT_FRC_LIT: *data = parse_frac(octal, type_name, sv_chop_left(&sv, 2)); break;                                \
    case SYX_TOKEN_KIND_DEC_FRC_LIT: SYX_TODO("SYX_TOKEN_KIND_DEC_FRC_LIT to native TASK(20260923-105431)", (), (value));              \
    case SYX_TOKEN_KIND_HEX_FRC_LIT: *data = parse_frac(hex, type_name, sv_chop_left(&sv, 2)); break;                                  \
    default: SYX_THROW("expected number", (), (value));                                                                                \
  }                                                                                                                                    \
})
      syx_native_primitive_xy_macro(type, X, Y);
#undef Y
#undef X
#undef parse_int
#undef parse_frac
      return rc_move(value);
    } break;
    case SYX_TYPE_KIND_STRUCTURE: {
      Syx_Value *value = rc_acquire(make_syx_value_native_instance(type));
      token = da_slice_shift(tokens);
      SYX_ASSERT(token.kind == SYX_TOKEN_KIND_LPAREN, "list of structure fields expected");
      Syx_Value *init_list = rc_acquire(parse_syx_list_values(tokens, SYX_TOKEN_KIND_RPAREN));
      syx_value_early_exit(init_list, (value));
      Syx_Pair *arguments = init_list->pair;
      Syx_Value *result = rc_acquire(syx_native_structure_update(NULL, value->native, type->structure, &arguments));
      rc_release(init_list);
      syx_value_early_exit(result, (value));
      rc_release(result);
      return rc_move(value);
    } break;
    case SYX_TYPE_KIND_PTR: return make_syx_value_native_instance(type);
    case SYX_TYPE_KIND_FUNCTION_PTR: return make_syx_value_native_instance(type);
  }
}

#endif // SYX_PARSER_NATIVE_IMPL_C
