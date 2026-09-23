#ifndef PARSER_H
#define PARSER_H

#include <stdint.h>

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
#define parse__number_fractional_value(sv, number, negative, map, INVALID) ({                              \
  size_t last_non_zero = 0;                                                                                \
  __uint128_t significand = 0;                                                                             \
  int32_t exponent = 0;                                                                                    \
  while (sv.count && *sv.data != '.') {                                                                    \
    int8_t digit = map[(uint8_t)*sv.data];                                                                 \
    if (digit < 0) INVALID;                                                                                \
    significand = (significand << map[0]) + digit;                                                         \
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
    significand = (significand << map[0]) + digit;                                                         \
    sv_chop_left(&sv);                                                                                     \
    exponent -= 1;                                                                                         \
    size_t index = 0;                                                                                      \
    while (index < sv.count && sv.data[index] == '0') index += 1;                                          \
    if (index < sv.count) continue;                                                                        \
    break;                                                                                                 \
  }                                                                                                        \
  exponent *= map[0];                                                                                      \
  typedef typeof(_Generic(number,                                                                          \
                     float: (uint32_t)0,                                                                   \
                     double: (uint64_t)0,                                                                  \
                     long double: (__uint128_t)0,                                                          \
                     f16_canonical_t: (uint16_t)0,                                                         \
                     f80_canonical_t: (__uint128_t)0,                                                      \
                     f128_canonical_t: (__uint128_t)0,                                                     \
                     f64pair_canonical_t: (__uint128_t)0)) number_bits_t;                                  \
  if (significand) {                                                                                       \
    size_t bits_width = sizeof(significand) * 8;                                                           \
    size_t leading_zeros = 0;                                                                              \
    constexpr typeof(significand) ONE = 1;                                                                 \
    while (leading_zeros < bits_width && (significand & (ONE << (bits_width - 1 - leading_zeros))) == 0) { \
      leading_zeros += 1;                                                                                  \
    }                                                                                                      \
    size_t current_bit_pos = bits_width - 1 - leading_zeros;                                               \
    constexpr size_t MANTISSA_BITS = _Generic(number,                                                      \
        float: RYU_F32_MANTISSA_BITS,                                                                      \
        double: RYU_F64_MANTISSA_BITS,                                                                     \
        long double: RYU_LD_MANTISSA_BITS,                                                                 \
        f16_canonical_t: RYU_F16_MANTISSA_BITS,                                                            \
        f80_canonical_t: RYU_F80_MANTISSA_BITS,                                                            \
        f128_canonical_t: RYU_F128_MANTISSA_BITS,                                                          \
        f64pair_canonical_t: RYU_F64PAIR_MANTISSA_BITS);                                                   \
    constexpr size_t EXPONENT_BITS = _Generic(number,                                                      \
        float: RYU_F32_EXPONENT_BITS,                                                                      \
        double: RYU_F64_EXPONENT_BITS,                                                                     \
        long double: RYU_LD_EXPONENT_BITS,                                                                 \
        f16_canonical_t: RYU_F16_EXPONENT_BITS,                                                            \
        f80_canonical_t: RYU_F80_EXPONENT_BITS,                                                            \
        f128_canonical_t: RYU_F128_EXPONENT_BITS,                                                          \
        f64pair_canonical_t: RYU_F64PAIR_EXPONENT_BITS);                                                   \
    if (MANTISSA_BITS >= current_bit_pos) significand <<= MANTISSA_BITS - current_bit_pos;                 \
    else significand >>= (current_bit_pos - MANTISSA_BITS);                                                \
    exponent += current_bit_pos + ((ONE << (EXPONENT_BITS - 1)) - 1);                                      \
    number_bits_t bits;                                                                                    \
    bits = (significand & ((ONE << (MANTISSA_BITS)) - 1));                                                 \
    bits |= ((exponent & ((ONE << EXPONENT_BITS) - 1)) << MANTISSA_BITS);                                  \
    bits |= (negative ? (ONE << (MANTISSA_BITS + EXPONENT_BITS)) : 0);                                     \
    memcpy(&number, (char *)(&bits) + (sizeof(bits) - sizeof(number)) / sizeof(char), sizeof(number));     \
  } else if (negative) {                                                                                   \
    number = f_canonical_from_native(-0, typeof(number));                                                  \
  } else {                                                                                                 \
    number = f_canonical_from_native(0, typeof(number));                                                   \
  }                                                                                                        \
})

#endif // PARSER_H
