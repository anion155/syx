#ifndef SYX_PARSER_H
#define SYX_PARSER_H

#include <str.h>
#include <syx/syx_lexer.h>

Syx_Value *parse_syx(String_View source, bool ignore_errors);

#endif // SYX_PARSER_H

#if defined(SYX_PARSER_IMPL) && !defined(SYX_PARSER_IMPL_C)
#define SYX_PARSER_IMPL_C

#include <stdint.h>
#include <wchar.h>

#define STR_IMPL
#include <str.h>
#define SYX_LEXER_IMPL
#include <syx/syx_lexer.h>
#define STR_FLOATS_IMPL
#include <str_floats.h>

Syx_Value *parse_syx_value(Syx_Tokens *tokens);

Syx_Value *parse_syx_list_values(Syx_Tokens *tokens, Syx_Token_Kind closing_token) {
  if (da_first(*tokens).kind == closing_token) {
    da_slice_chop_left(tokens);
    return syx_value_nil();
  }
  Syx_Value *list = NULL;
  Syx_Value **pair_value = &list;
  while (tokens->count) {
    Syx_Token first = da_first(*tokens);
    if (first.kind == SYX_TOKEN_KIND_SYMBOL && sv_eq(first.source, sv_from_strlit("."))) {
      Syx_Value *value = rc_acquire(parse_syx_value(tokens));
      syx_value_early_exit(value, (list));
      *pair_value = value;
      SYX_ASSERT(da_first(*tokens).kind == closing_token, "expected end token", (), (list));
      goto without_nil;
    }
    Syx_Value *value = rc_acquire(parse_syx_value(tokens));
    syx_value_early_exit(value, (list));
    *pair_value = rc_acquire(make_syx_value_pair(value, NULL));
    pair_value = &(*pair_value)->pair->right;
    if (da_first(*tokens).kind == closing_token) break;
  }
  *pair_value = rc_acquire(syx_value_nil());
without_nil:
  SYX_ASSERT(da_first(*tokens).kind == closing_token, "expected end token", (), (list));
  da_slice_chop_left(tokens);
  return list;
}

uint16_t syx_parser_utf_4_chars_to_codepoint(char chars[4]) {
  return (syx_utils_hex_to_decimal(chars[0]) << 12) |
         (syx_utils_hex_to_decimal(chars[1]) << 8) |
         (syx_utils_hex_to_decimal(chars[2]) << 4) |
         syx_utils_hex_to_decimal(chars[3]);
}

bool syx_parser_utf_codepoint_to_string(uint32_t codepoint, String_Builder *string) {
  if (codepoint <= 0x007F) {
    sb_append(string, (uint8_t)codepoint);
  } else if (codepoint <= 0x07FF) {
    sb_append(string, (uint8_t)(0xC0 | (codepoint >> 6)));
    sb_append(string, (uint8_t)(0x80 | (codepoint & 0x3F)));
  } else if (codepoint <= 0xFFFF) {
    sb_append(string, (uint8_t)(0xE0 | (codepoint >> 12)));
    sb_append(string, (uint8_t)(0x80 | ((codepoint >> 6) & 0x3F)));
    sb_append(string, (uint8_t)(0x80 | (codepoint & 0x3F)));
  } else if (codepoint <= 0x10FFFF) {
    sb_append(string, (uint8_t)(0xF0 | (codepoint >> 18)));
    sb_append(string, (uint8_t)(0x80 | ((codepoint >> 12) & 0x3F)));
    sb_append(string, (uint8_t)(0x80 | ((codepoint >> 6) & 0x3F)));
    sb_append(string, (uint8_t)(0x80 | (codepoint & 0x3F)));
  } else {
    return false;
  }
  return true;
}

uint32_t syx_parser_utf_string_to_codepoint(String_View string) {
  if (string.count < sv_first_utf_length(string)) return 0;
#define get(index, mask, ...) ((__VA_OPT__((uint32_t)(uint8_t)) string.data[index] & mask) __VA_OPT__(<< __VA_ARGS__))
  if (get(0, 0b10000000) == 0) {
    return string.data[0];
  } else if (get(0, 0b11100000) == 0b11000000) {
    if (get(1, 0b11000000) != 0b10000000) return 0;
    return get(0, 0b00011111, 6) | get(1, 0b00111111);
  } else if (get(0, 0b11110000) == 0b11100000) {
    if (get(1, 0b11000000) != 0b10000000) return 0;
    if (get(2, 0b11000000) != 0b10000000) return 0;
    return get(0, 0b00001111, 12) | get(1, 0b00111111, 6) | get(2, 0b00111111);
  } else if (get(0, 0b11111000) == 0b11110000) {
    if (get(1, 0b11000000) != 0b10000000) return 0;
    if (get(2, 0b11000000) != 0b10000000) return 0;
    if (get(3, 0b11000000) != 0b10000000) return 0;
    return get(0, 0b00000111, 18) | get(1, 0b00111111, 12) | get(2, 0b00111111, 6) | get(3, 0b00111111);
  } else {
    return 0;
  }
#undef get
}

bool syx_parser_validate_utf_bytes(char *chars, size_t bytes_count) {
  for (size_t index = 0; index < (bytes_count); index += 1) {
    if (!syx_utils_is_hex_digit(chars[index])) return false;
  }
  return true;
}

Syx_Value *parse_syx_string_value(Syx_Token token) {
  SYX_ASSERT(token.kind == SYX_TOKEN_KIND_STRLIT, "expected string literal token");
  SYX_ASSERT(token.source.data[0] == '"' && token.source.data[token.source.count - 1] == '"', "invalid string literal");
  token.source.data += 1;
  token.source.count -= 2;
  String_Builder literal = {0};
  da_reserve(&literal, token.source.count);
  for (size_t tindex = 0; tindex < token.source.count; tindex += 1) {
    if (token.source.data[tindex] == '\\') {
      tindex += 1;
      if (token.source.count <= tindex) {
        sb_append(&literal, '\\');
        continue;
      }

#define handle_error()                              \
  {                                                 \
    sb_append(&literal, token.source.data[tindex]); \
    continue;                                       \
  }
#define utf_bytes_from_string(bytes_count)                             \
  if (token.source.count - tindex - 1 < (bytes_count)) handle_error(); \
  char chars[(bytes_count)];                                           \
  memcpy(chars, token.source.data + tindex + 1, (bytes_count));        \
  if (!syx_parser_validate_utf_bytes(chars, (bytes_count))) handle_error();

      switch (token.source.data[tindex]) {
        case 'b': sb_append(&literal, '\b'); break;
        case 'f': sb_append(&literal, '\f'); break;
        case 'v': sb_append(&literal, '\v'); break;
        case 'a': sb_append(&literal, '\a'); break;
        case 'e': sb_append(&literal, '\e'); break;
        case 'n': sb_append(&literal, '\n'); break;
        case 'r': sb_append(&literal, '\r'); break;
        case 't': sb_append(&literal, '\t'); break;
        case '0': sb_append(&literal, '\0'); break;
        case 'x': {
          utf_bytes_from_string(2);
          uint8_t byte = (syx_utils_hex_to_decimal(chars[0]) << 4) + syx_utils_hex_to_decimal(chars[1]);
          sb_append(&literal, byte);
          tindex += 2;
        } break;
        case 'u': {
          if (token.source.count > tindex + 3 && token.source.data[tindex + 1] == '{') {
            size_t count = 0;
            while (true) {
              size_t index = tindex + count + 2;
              if (token.source.count <= index) {
                count = 0;
                break;
              }
              if (token.source.data[index] == '}') break;
              if (!syx_utils_is_hex_digit(token.source.data[index])) {
                count = 0;
                break;
              }
              count += 1;
            }
            if (!count || count > 8) handle_error();
            char chars[8] = {};
            memset(chars, '0', 8 - count);
            memcpy(chars + (8 - count), token.source.data + tindex + 2, count);
            uint16_t high = syx_parser_utf_4_chars_to_codepoint(chars);
            uint16_t low = syx_parser_utf_4_chars_to_codepoint(chars + 4);
            if (!syx_parser_utf_codepoint_to_string(((uint32_t)high << 16) | low, &literal)) handle_error();
            tindex += count + 2;
            continue;
          }
          utf_bytes_from_string(4);
          if (!syx_parser_utf_codepoint_to_string(syx_parser_utf_4_chars_to_codepoint(chars), &literal)) handle_error();
          tindex += 4;
        } break;
        case 'U': {
          utf_bytes_from_string(8);
          uint16_t high = syx_parser_utf_4_chars_to_codepoint(chars);
          uint16_t low = syx_parser_utf_4_chars_to_codepoint(chars + 4);
          if (!syx_parser_utf_codepoint_to_string(((uint32_t)high << 16) | low, &literal)) handle_error();
          tindex += 8;
        } break;
        default: sb_append(&literal, token.source.data[tindex]); break;
      }
      continue;
    }
    size_t width = sv_first_utf_length(token.source);
    for (size_t lindex = 0; lindex < width; lindex += 1) {
      sb_append(&literal, token.source.data[tindex + lindex]);
    }
  }
#undef handle_error
#undef utf_bytes_from_string
  Syx_Value *value = make_syx_value_string_n_dup(literal.data, literal.count);
  sb_free(&literal);
  return value;
}

// clang-format off
#define I -1
constexpr int8_t parse__number_binary_map[256] = {
  2,I,I,I,I,I,I,I,I,I,I,I,I,I,I,I, I,I,I,I,I,I,I,I,I,I,I,I,I,I,I,I,
  I,I,I,I,I,I,I,I,I,I,I,I,I,I,I,I, 0,1,I,I,I,I,I,I,I,I,I,I,I,I,I,I,
  I,I,I,I,I,I,I,I,I,I,I,I,I,I,I,I, I,I,I,I,I,I,I,I,I,I,I,I,I,I,I,I,
  I,I,I,I,I,I,I,I,I,I,I,I,I,I,I,I, I,I,I,I,I,I,I,I,I,I,I,I,I,I,I,I,
  I,I,I,I,I,I,I,I,I,I,I,I,I,I,I,I, I,I,I,I,I,I,I,I,I,I,I,I,I,I,I,I,
  I,I,I,I,I,I,I,I,I,I,I,I,I,I,I,I, I,I,I,I,I,I,I,I,I,I,I,I,I,I,I,I,
  I,I,I,I,I,I,I,I,I,I,I,I,I,I,I,I, I,I,I,I,I,I,I,I,I,I,I,I,I,I,I,I,
  I,I,I,I,I,I,I,I,I,I,I,I,I,I,I,I, I,I,I,I,I,I,I,I,I,I,I,I,I,I,I,I,
};
constexpr int8_t parse__number_octal_map[256] = {
  3,I,I,I,I,I,I,I,I,I,I,I,I,I,I,I, I,I,I,I,I,I,I,I,I,I,I,I,I,I,I,I,
  I,I,I,I,I,I,I,I,I,I,I,I,I,I,I,I, 0,1,2,3,4,5,6,7,I,I,I,I,I,I,I,I,
  I,I,I,I,I,I,I,I,I,I,I,I,I,I,I,I, I,I,I,I,I,I,I,I,I,I,I,I,I,I,I,I,
  I,I,I,I,I,I,I,I,I,I,I,I,I,I,I,I, I,I,I,I,I,I,I,I,I,I,I,I,I,I,I,I,
  I,I,I,I,I,I,I,I,I,I,I,I,I,I,I,I, I,I,I,I,I,I,I,I,I,I,I,I,I,I,I,I,
  I,I,I,I,I,I,I,I,I,I,I,I,I,I,I,I, I,I,I,I,I,I,I,I,I,I,I,I,I,I,I,I,
  I,I,I,I,I,I,I,I,I,I,I,I,I,I,I,I, I,I,I,I,I,I,I,I,I,I,I,I,I,I,I,I,
  I,I,I,I,I,I,I,I,I,I,I,I,I,I,I,I, I,I,I,I,I,I,I,I,I,I,I,I,I,I,I,I,
};
constexpr int8_t parse__number_decimal_map[256] = {
  I,I,I,I,I,I,I,I,I,I,I,I,I,I,I,I, I,I,I,I,I,I,I,I,I,I,I,I,I,I,I,I,
  I,I,I,I,I,I,I,I,I,I,I,I,I,I,I,I, 0,1,2,3,4,5,6,7,8,9,I,I,I,I,I,I,
  I,I,I,I,I,I,I,I,I,I,I,I,I,I,I,I, I,I,I,I,I,I,I,I,I,I,I,I,I,I,I,I,
  I,I,I,I,I,I,I,I,I,I,I,I,I,I,I,I, I,I,I,I,I,I,I,I,I,I,I,I,I,I,I,I,
  I,I,I,I,I,I,I,I,I,I,I,I,I,I,I,I, I,I,I,I,I,I,I,I,I,I,I,I,I,I,I,I,
  I,I,I,I,I,I,I,I,I,I,I,I,I,I,I,I, I,I,I,I,I,I,I,I,I,I,I,I,I,I,I,I,
  I,I,I,I,I,I,I,I,I,I,I,I,I,I,I,I, I,I,I,I,I,I,I,I,I,I,I,I,I,I,I,I,
  I,I,I,I,I,I,I,I,I,I,I,I,I,I,I,I, I,I,I,I,I,I,I,I,I,I,I,I,I,I,I,I,
};
constexpr int8_t parse__number_hex_map[256] = {
  4, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I,
  I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, I, I, I, I, I, I,
  I,10,11,12,13,14,15, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I,
  I,10,11,12,13,14,15, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I,
  I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I,
  I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I,
  I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I,
  I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I,
};
#undef I
// clang-format on
#define parse__number_binary_shift(value) (value << 1)
#define parse__number_octal_shift(value) (value << 3)
#define parse__number_decimal_shift(value) (value * 10)
#define parse__number_hex_shift(value) (value << 4)
#define parse__number_unsigned_integer_value(sv, number, shift, map, INVALID, on_digit) \
  while (sv.count) {                                                                    \
    int8_t digit = map[(uint8_t)*sv.data];                                              \
    if (digit < 0) INVALID;                                                             \
    number = shift(number) + digit;                                                     \
    sv_chop_left(&sv, 1);                                                               \
    on_digit;                                                                           \
  }

struct parsed_fractional {
  __uint128_t significand;
  int32_t exponent;
};
#define parse__number_fractional_value(sv, number, shift, map, INVALID, MANTISSA_BITS, EXPONENT_BITS) ({   \
  size_t last_non_zero = 0;                                                                                \
  __uint128_t significand = 0;                                                                             \
  int32_t exponent = 0;                                                                                    \
  while (sv.count && *sv.data != '.') {                                                                    \
    int8_t digit = map[(uint8_t)*sv.data];                                                                 \
    if (digit < 0) INVALID;                                                                                \
    significand = (significand << shift) + digit;                                                          \
    sv_chop_left(&sv);                                                                                     \
    if (last_non_zero > 0) {                                                                               \
      last_non_zero -= 1;                                                                                  \
      continue;                                                                                            \
    }                                                                                                      \
    if (*sv.data == '.') break;                                                                            \
    size_t index = 0;                                                                                      \
    while (sv.data[index] == '0') index += 1;                                                              \
    if (sv.data[index] != '.') {                                                                           \
      last_non_zero = index;                                                                               \
      continue;                                                                                            \
    }                                                                                                      \
    size_t maybe_exponent = index;                                                                         \
    index += 1;                                                                                            \
    while (index < sv.count && sv.data[index] == '0') index += 1;                                          \
    if (index != sv.count) {                                                                               \
      last_non_zero = index;                                                                               \
      continue;                                                                                            \
    }                                                                                                      \
    exponent = maybe_exponent;                                                                             \
    sv_chop_left(&sv, exponent);                                                                           \
    break;                                                                                                 \
  }                                                                                                        \
  if (*sv.data != '.') INVALID;                                                                            \
  sv_chop_left(&sv);                                                                                       \
  while (sv.count) {                                                                                       \
    int8_t digit = map[(uint8_t)*sv.data];                                                                 \
    if (digit < 0) INVALID;                                                                                \
    significand = (significand << shift) + digit;                                                          \
    sv_chop_left(&sv);                                                                                     \
    exponent -= 1;                                                                                         \
    size_t index = 0;                                                                                      \
    while (index < sv.count && sv.data[index] == '0') index += 1;                                          \
    if (index < sv.count) continue;                                                                        \
    break;                                                                                                 \
  }                                                                                                        \
  exponent *= shift;                                                                                       \
  if (significand) {                                                                                       \
    size_t bits_width = sizeof(significand) * 8;                                                           \
    size_t leading_zeros = 0;                                                                              \
    constexpr typeof(significand) ONE = 1;                                                                 \
    while (leading_zeros < bits_width && (significand & (ONE << (bits_width - 1 - leading_zeros))) == 0) { \
      leading_zeros += 1;                                                                                  \
    }                                                                                                      \
    size_t current_bit_pos = bits_width - 1 - leading_zeros;                                               \
    if (MANTISSA_BITS >= current_bit_pos) significand <<= MANTISSA_BITS - current_bit_pos;                 \
    else significand >>= (current_bit_pos - MANTISSA_BITS);                                                \
    exponent += current_bit_pos + ((ONE << (EXPONENT_BITS - 1)) - 1);                                      \
    typeof(_Generic(number,                                                                                \
               float: (uint32_t)0,                                                                         \
               double: (uint64_t)0,                                                                        \
               long double: (__uint128_t)0)) bits;                                                         \
    bits = (significand & ((ONE << (MANTISSA_BITS)) - 1));                                                 \
    bits |= ((exponent & ((ONE << EXPONENT_BITS) - 1)) << MANTISSA_BITS);                                  \
    bits |= (negative ? (ONE << (MANTISSA_BITS + EXPONENT_BITS)) : 0);                                     \
    memcpy(&number, &bits, sizeof(number));                                                                \
  } else {                                                                                                 \
    number = 0;                                                                                            \
  }                                                                                                        \
})

#define parse__syx_number_integer_value(base, sv) ({ \
  String_View _sv_ = sv_from_like(sv);               \
  bool negative = false;                             \
  if (_sv_.data[0] == '-' || _sv_.data[0] == '+') {  \
    _sv_.data += 1;                                  \
    _sv_.count -= 1;                                 \
    negative = _sv_.data[0] == '-';                  \
  }                                                  \
  syx_integer_t number = 0;                          \
  parse__number_unsigned_integer_value(              \
      _sv_,                                          \
      number,                                        \
      parse__number_##base##_shift,                  \
      parse__number_##base##_map,                    \
      SYX_THROW("expected integer number"), );       \
  if (negative) number *= -1;                        \
  make_syx_value_number_integer(number);             \
})

Syx_Value *parse_syx_number_binary_integer_value(Syx_Token token) {
  return parse__syx_number_integer_value(binary, token.source);
}

Syx_Value *parse_syx_number_octal_integer_value(Syx_Token token) {
  return parse__syx_number_integer_value(octal, token.source);
}

Syx_Value *parse_syx_number_decimal_integer_value(Syx_Token token) {
  return parse__syx_number_integer_value(decimal, token.source);
}

Syx_Value *parse_syx_number_hex_integer_value(Syx_Token token) {
  return parse__syx_number_integer_value(hex, token.source);
}

#define parse__syx_number_fractional_value(base, sv) ({ \
  String_View _sv_ = sv_from_like(sv);                  \
  bool negative = false;                                \
  if (_sv_.data[0] == '-' || _sv_.data[0] == '+') {     \
    _sv_.data += 1;                                     \
    _sv_.count -= 1;                                    \
    negative = _sv_.data[0] == '-';                     \
  }                                                     \
  double number = 0;                                    \
  parse__number_fractional_value(                       \
      _sv_,                                             \
      number,                                           \
      parse__number_##base##_map[0],                    \
      parse__number_##base##_map,                       \
      SYX_THROW("expected integer number"),             \
      RYU_F64_MANTISSA_BITS,                            \
      RYU_F64_EXPONENT_BITS);                           \
  make_syx_value_number_fractional(number);             \
})

Syx_Value *parse_syx_number_binary_fractional_value(Syx_Token token) {
  return parse__syx_number_fractional_value(binary, token.source);
}

Syx_Value *parse_syx_number_octal_fractional_value(Syx_Token token) {
  return parse__syx_number_fractional_value(octal, token.source);
}

Syx_Value *parse_syx_number_decimal_fractional_value(Syx_Token token) {
  UNUSED(token);
  SYX_TODO();
}

Syx_Value *parse_syx_number_hex_fractional_value(Syx_Token token) {
  return parse__syx_number_fractional_value(hex, token.source);
}

Syx_Value *parse_syx_symbol_value(Syx_Token token) {
  if (token.source.data[0] == '|' && token.source.data[token.source.count - 1] == '|') {
    return make_syx_value_symbol_n(token.source.data + 1, token.source.count - 2);
  } else {
    return make_syx_value_symbol_n(token.source.data, token.source.count);
  }
}

Syx_Value *parse_syx_prefix(Syx_Token token, Syx_Tokens *tokens) {
  SYX_ASSERT(token.kind == SYX_TOKEN_KIND_PREFIX, "prefix expected");
  uint32_t type = syx_parser_utf_string_to_codepoint(sv_from_like(token.source));
  switch (type) {
    case '\'':
    case ',': {
      return make_syx_value_prefixed((Syx_Prefixed_Kind)type, parse_syx_value(tokens));
    }
    case ':':
    case '$': {
      Syx_Token symbol = da_slice_shift(tokens);
      SYX_ASSERT(symbol.kind == SYX_TOKEN_KIND_SYMBOL, "symbol expected");
      return make_syx_value_prefixed((Syx_Prefixed_Kind)type, parse_syx_symbol_value(symbol));
    }
    default: SYX_THROW("unexpected prefix type");
  }
}

Syx_Value *parse_syx_dispatch(Syx_Token token, Syx_Tokens *tokens) {
  SYX_ASSERT(token.kind == SYX_TOKEN_KIND_DISPATCH && token.source.count > 1, "dispatch expected");
  uint32_t type = syx_parser_utf_string_to_codepoint(sv_from_parts((char *)token.source.data + 1, token.source.count - 1));
  switch (type) {
    case 'n': return syx_value_nil();
    case 't': return syx_value_bool_true();
    case 'f': return syx_value_bool_false();
    case 'R': {
      SYX_ASSERT(tokens->count >= 1, "expected string literal");
      token = da_slice_shift(tokens);
      SYX_ASSERT(token.kind == SYX_TOKEN_KIND_STRLIT, "expected string literal");
      return make_syx_value_string_n_dup(token.source.data, token.source.count);
    }
    case '{': {
      Syx_Value *fields = rc_acquire(parse_syx_list_values(tokens, SYX_TOKEN_KIND_RCURLY));
      syx_value_early_exit(fields);
      return make_syx_value_pair(make_syx_value_symbol_strlit("object"), rc_move(fields));
    }
    case 'c': {
      SYX_ASSERT(tokens->count >= 1, "expected symbol");
      token = da_slice_shift(tokens);
      SYX_ASSERT(token.kind == SYX_TOKEN_KIND_SYMBOL && token.source.count > 1 && token.source.data[0] == '_', "expected c type symbol");
      sv_chop_left(&token.source);
      Syx_Type **type = ht_find(&SYX_KNOWN_TYPES()->registry, token.source);
      SYX_ASSERT(type != NULL, "c type " SV_FMT, (sv_fmt_arg(token.source)));
      token = da_slice_shift(tokens);
      switch ((*type)->kind) {
        case SYX_TYPE_KIND_VOID: SYX_THROW("native void value is not constructible");
        case SYX_TYPE_KIND_PRIMITIVE: {

        } break;
        case SYX_TYPE_KIND_STRUCTURE: break;
        case SYX_TYPE_KIND_PTR: break;
        case SYX_TYPE_KIND_FUNCTION_PTR: break;
      }
    }
    default: SYX_THROW("unexpected dispatch type");
  }
}

Syx_Value *parse__syx_value_from_token(Syx_Token first, Syx_Tokens *tokens) {
  switch (first.kind) {
    case SYX_TOKEN_KIND_NULL: return parse_syx_value(tokens);
    case SYX_TOKEN_KIND_LPAREN: return parse_syx_list_values(tokens, SYX_TOKEN_KIND_RPAREN);
    case SYX_TOKEN_KIND_STRLIT: return parse_syx_string_value(first);
    case SYX_TOKEN_KIND_NAN: return make_syx_value_number_fractional(NAN);
    case SYX_TOKEN_KIND_INFINITY_POSITIVE: return make_syx_value_number_fractional(INFINITY);
    case SYX_TOKEN_KIND_INFINITY_NEGATIVE: return make_syx_value_number_fractional(-INFINITY);
    case SYX_TOKEN_KIND_BIN_INT_LIT: return parse_syx_number_binary_integer_value(first);
    case SYX_TOKEN_KIND_OCT_INT_LIT: return parse_syx_number_octal_integer_value(first);
    case SYX_TOKEN_KIND_DEC_INT_LIT: return parse_syx_number_decimal_integer_value(first);
    case SYX_TOKEN_KIND_HEX_INT_LIT: return parse_syx_number_hex_integer_value(first);
    case SYX_TOKEN_KIND_BIN_FRC_LIT: return parse_syx_number_binary_fractional_value(first);
    case SYX_TOKEN_KIND_OCT_FRC_LIT: return parse_syx_number_octal_fractional_value(first);
    case SYX_TOKEN_KIND_DEC_FRC_LIT: return parse_syx_number_decimal_fractional_value(first);
    case SYX_TOKEN_KIND_HEX_FRC_LIT: return parse_syx_number_hex_fractional_value(first);
    case SYX_TOKEN_KIND_SYMBOL: return parse_syx_symbol_value(first);
    case SYX_TOKEN_KIND_PREFIX: return parse_syx_prefix(first, tokens);
    case SYX_TOKEN_KIND_DISPATCH: return parse_syx_dispatch(first, tokens);
    default: SYX_THROW("unexpected token");
  }
}

Syx_Value *parse_syx_value(Syx_Tokens *tokens) {
  SYX_ASSERT(tokens->count, "expected value");
  Syx_Token first = da_slice_shift(tokens);
  return parse__syx_value_from_token(first, tokens);
}

Syx_Value *parse_syx(String_View source, bool ignore_errors) {
  Syx_Tokens tokens = syx_lexer_tokenize(source);
  SYX_ASSERT(tokens.count, "Failed to parse syx script");
  SYX_ASSERT(tokens.data[tokens.count - 1].kind == SYX_TOKEN_KIND_EOF, "Failed to parse syx script");
  tokens.count -= 1;
  Syx_Value *list = NULL;
  Syx_Value **pair_value = &list;
  for (Syx_Tokens it = tokens; it.count;) {
    Syx_Value *value = rc_acquire(parse_syx_value(&it));
    if (!ignore_errors) syx_value_early_exit(value, (list));
    *pair_value = rc_acquire(make_syx_value_pair(value, NULL));
    pair_value = &(*pair_value)->pair->right;
  }
  *pair_value = rc_acquire(syx_value_nil());
  return rc_move(list);
}

#undef tokens_chop_left

#endif // SYX_PARSER_IMPL_C
