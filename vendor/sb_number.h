#ifndef SB_NUMBER_H
#define SB_NUMBER_H

#include <limits.h>
#include <sb.h>

#define INTEGER_MIN(type) _Generic((type)0, \
    char: CHAR_MIN,                         \
    signed char: SCHAR_MIN,                 \
    short: SHRT_MIN,                        \
    int: INT_MIN,                           \
    long: LONG_MIN,                         \
    long long: LLONG_MIN,                   \
    unsigned char: 0,                       \
    unsigned short: 0,                      \
    unsigned int: 0,                        \
    unsigned long: 0,                       \
    unsigned long long: 0)
#define INTEGER_DEC_WIDTH(type) _Generic((type)0,                                        \
    char: sizeof(STRINGIFY2(CHAR_MAX)) - 1 /* NULL terminator */ + 1 /* sign */,         \
    signed char: sizeof(STRINGIFY2(SCHAR_MAX)) - 1 /* NULL terminator */ + 1 /* sign */, \
    short: sizeof(STRINGIFY2(SHRT_MAX)) - 1 /* NULL terminator */ + 1 /* sign */,        \
    int: sizeof(STRINGIFY2(INT_MAX)) - 1 /* NULL terminator */ + 1 /* sign */,           \
    long: sizeof(STRINGIFY2(LONG_MAX)) - 1 /* NULL terminator */ + 1 /* sign */,         \
    long long: sizeof(STRINGIFY2(LLONG_MAX)) - 1 /* NULL terminator */ + 1 /* sign */,   \
    unsigned char: sizeof(STRINGIFY2(UCHAR_MAX)) - 1 /* NULL terminator */,              \
    unsigned short: sizeof(STRINGIFY2(USHRT_MAX)) - 1 /* NULL terminator */,             \
    unsigned int: sizeof(STRINGIFY2(UINT_MAX)) - 1 /* NULL terminator */,                \
    unsigned long: sizeof(STRINGIFY2(ULONG_MAX)) - 1 /* NULL terminator */,              \
    unsigned long long: sizeof(STRINGIFY2(ULLONG_MAX)) - 1 /* NULL terminator */)

size_t sb__append_integer(String_Builder *string, intmax_t value, size_t max_width, intmax_t minimum);
size_t sb__append_unsigned_integer(String_Builder *string, uintmax_t value, size_t max_width);
#define sb_append_integer(string, value) ({                        \
  typeof((value)) _value_ = (value);                               \
  typeof(_value_) minimum = INTEGER_MIN(typeof(_value_));          \
  size_t max_width = INTEGER_DEC_WIDTH(typeof(_value_));           \
  minimum < 0                                                      \
      ? sb__append_integer((string), _value_, max_width, minimum)  \
      : sb__append_unsigned_integer((string), _value_, max_width); \
})

size_t sb_append_float(String_Builder *string, float value);
size_t sb_append_double(String_Builder *string, double value);
size_t sb_append_long_double(String_Builder *string, long double value);
#define sb_append_floating(string, value) _Generic((value), \
    float: sb_append_float,                                 \
    double: sb_append_double,                               \
    long double: sb_append_long_double)((string), (value))

#endif // SB_NUMBER_H

#if defined(SB_NUMBER_IMPL) && !defined(SB_NUMBER_IMPL_C)
#define SB_NUMBER_IMPL_C

#define SB_IMPL
#include <sb.h>
#define RYU_IMPL
#include <ryu.h>

void __string_reverse(char *string, size_t width) {
  for (size_t index = width / 2; index > 0; index -= 1) {
    char b = *(string + index - 1);
    size_t right_index = width - index;
    *(string + index - 1) = *(string + right_index);
    *(string + right_index) = b;
  }
}

size_t sb__append_integer(String_Builder *string, intmax_t value, size_t max_width, intmax_t minimum) {
  Stringify_State state = make_stringify_state(string, max_width);
  if (value == 0) return state.count += sb_append(state.sb, '0');
  if (value == minimum) {
    state.count += sb_append(state.sb, '-');
    size_t written = sb__append_unsigned_integer(string, (uintmax_t)0 - (uintmax_t)value, max_width - 1);
    return written + 1;
  }
  if (value < 0) (state.count += sb_append(state.sb, '-'), value *= -1);
  size_t rev_start = state.count;
  for (typeof(value) it = value; it != 0; it /= 10) state.count += sb_append(state.sb, '0' + it % 10);
  if (state.sb) __string_reverse(stringify_ptr(&state, rev_start), state.count - rev_start);
  return state.count;
}

size_t sb__append_unsigned_integer(String_Builder *string, uintmax_t value, size_t max_width) {
  Stringify_State state = make_stringify_state(string, max_width);
  if (value == 0) return state.count += sb_append(state.sb, '0');
  size_t rev_start = state.count;
  for (typeof(value) it = value; it != 0; it /= 10) state.count += sb_append(state.sb, '0' + it % 10);
  if (state.sb) __string_reverse(stringify_ptr(&state, rev_start), state.count - rev_start);
  return state.count;
}

size_t stringify___append_floating_special(Stringify_State *state, bool sign, uint32_t exponent, uint32_t mantissa) {
  if (exponent == ((1u << RYU_F2S_FLOAT_EXPONENT_BITS) - 1u)) {
    if (mantissa) return state->count += sb_append_strlit(state->sb, "NaN");
    if (sign) state->count += sb_append(state->sb, '-');
    state->count += sb_append_strlit(state->sb, "Infinity");
    return state->count;
  }
  if ((exponent == 0 && mantissa == 0)) {
    if (sign) state->count += sb_append(state->sb, '-');
    state->count += sb_append_strlit(state->sb, "0.0");
    return state->count;
  }
  return 0;
}
#define sb__append_floating_special(state, sign, exponent, mantissa) ({                \
  size_t count = stringify___append_floating_special(state, sign, exponent, mantissa); \
  if (count) return count;                                                             \
})

void sb__append_floating(Stringify_State *state, floating_decimal_64 v) {
  if (!v.exponent) {
    state->count += sb_append_integer(state->sb, v.mantissa);
  } else if (v.exponent < 0) {
    size_t exponent = -v.exponent;
    size_t count = ryu_d2s_decimalLength17(v.mantissa);
    if (exponent >= count) state->count += sb_append(state->sb, '0');
    state->count += sb_append_integer(state->sb, v.mantissa);
    state->count += sb_append(state->sb, '.');
    if (state->sb) {
      char *start = state->sb->data + state->sb->count - exponent - 1;
      memmove(start + 1, start, exponent);
      *start = '.';
    }
  } else {
    state->count += sb_append_integer(state->sb, v.mantissa);
    state->count += sb_append_repeat(state->sb, '0', v.exponent);
  }
}

size_t sb_append_float(String_Builder *string, float value) {
  Stringify_State state = make_stringify_state(string, 16);
  uint32_t bits = float_to_bits(value);
  bool ieeeSign = ((bits >> (RYU_F2S_FLOAT_MANTISSA_BITS + RYU_F2S_FLOAT_EXPONENT_BITS)) & 1) != 0;
  uint32_t ieeeMantissa = bits & ((1u << RYU_F2S_FLOAT_MANTISSA_BITS) - 1);
  uint32_t ieeeExponent = (bits >> RYU_F2S_FLOAT_MANTISSA_BITS) & ((1u << RYU_F2S_FLOAT_EXPONENT_BITS) - 1);
  sb__append_floating_special(&state, ieeeSign, ieeeExponent, ieeeMantissa);
  floating_decimal_32 v = ryu_f2s_f2d(ieeeMantissa, ieeeExponent);
  if (ieeeSign) state.count += sb_append(state.sb, '-');
  sb__append_floating(&state, (floating_decimal_64){.mantissa = v.mantissa, .exponent = v.exponent});
  return state.count;
}

size_t sb_append_double(String_Builder *string, double value) {
  Stringify_State state = make_stringify_state(string, 25);
  uint64_t bits = double_to_bits(value);
  bool ieeeSign = ((bits >> (RYU_D2S_DOUBLE_MANTISSA_BITS + RYU_D2S_DOUBLE_EXPONENT_BITS)) & 1) != 0;
  uint64_t ieeeMantissa = bits & ((1ull << RYU_D2S_DOUBLE_MANTISSA_BITS) - 1);
  uint32_t ieeeExponent = (uint32_t)((bits >> RYU_D2S_DOUBLE_MANTISSA_BITS) & ((1u << RYU_D2S_DOUBLE_EXPONENT_BITS) - 1));
  sb__append_floating_special(&state, ieeeSign, ieeeExponent, ieeeMantissa);
  floating_decimal_64 v = ryu_d2s_d2d_optimized(ieeeMantissa, ieeeExponent);
  if (ieeeSign) state.count += sb_append(state.sb, '-');
  sb__append_floating(&state, v);
  return state.count;
}

size_t sb_append_long_double(String_Builder *string, long double value) {
  // ryu does not have separate support for `long double`
  // ryu_generic_128 has two cons:
  // - `long double` is not supported on most platforms (except for x86)
  // - it is performance heavy
  return sb_append_double(string, (double)value);
}

#endif // SB_NUMBER_IMPL_C
