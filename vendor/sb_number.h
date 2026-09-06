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

size_t sb__append_integer(String_Builder *sb, intmax_t value, size_t max_width, intmax_t minimum);
size_t sb__append_unsigned_integer(String_Builder *sb, uintmax_t value, size_t max_width);
#define sb_append_integer(sb, value) ({                        \
  typeof((value)) _value_ = (value);                           \
  typeof(_value_) minimum = INTEGER_MIN(typeof(_value_));      \
  size_t max_width = INTEGER_DEC_WIDTH(typeof(_value_));       \
  minimum < 0                                                  \
      ? sb__append_integer((sb), _value_, max_width, minimum)  \
      : sb__append_unsigned_integer((sb), _value_, max_width); \
})

size_t sb_append_float(String_Builder *sb, float value);
size_t sb_append_double(String_Builder *sb, double value);
size_t sb_append_long_double(String_Builder *sb, long double value);
#define sb_append_floating(sb, value) _Generic((value), \
    float: sb_append_float,                             \
    double: sb_append_double,                           \
    long double: sb_append_long_double)((sb), (value))

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

size_t sb__append_integer(String_Builder *sb, intmax_t value, size_t max_width, intmax_t minimum) {
  Stringify_State state = make_stringify_state(sb, max_width);
  if (value == 0) return stringify_append(&state, sb_append, '0');
  if (value == minimum) {
    stringify_append(&state, sb_append, '-');
    size_t written = sb__append_unsigned_integer(sb, (uintmax_t)0 - (uintmax_t)value, max_width - 1);
    return written + 1;
  }
  if (value < 0) (stringify_append(&state, sb_append, '-'), value *= -1);
  size_t rev_start = state.count;
  for (typeof(value) it = value; it != 0; it /= 10) stringify_append(&state, sb_append, '0' + it % 10);
  if (state.sb) __string_reverse(stringify_ptr(&state, rev_start), state.count - rev_start);
  return state.count;
}

size_t sb__append_unsigned_integer(String_Builder *sb, uintmax_t value, size_t max_width) {
  Stringify_State state = make_stringify_state(sb, max_width);
  if (value == 0) return stringify_append(&state, sb_append, '0');
  size_t rev_start = state.count;
  for (typeof(value) it = value; it != 0; it /= 10) stringify_append(&state, sb_append, '0' + it % 10);
  if (state.sb) __string_reverse(stringify_ptr(&state, rev_start), state.count - rev_start);
  return state.count;
}

#define sb__append_floating(state, value, lower, upper) ({                                                             \
  Stringify_State *_state___ = (state);                                                                                \
  RYU_##upper##_MANTISSA_STORAGE bits = ryu_##lower##_to_bits((value));                                                \
  bool sign = ((bits >> (RYU_##upper##_MANTISSA_BITS + RYU_##upper##_EXPONENT_BITS)) & 1) != 0;                        \
  RYU_##upper##_MANTISSA_STORAGE mantissa = bits & ((RYU_##upper##_MANTISSA_BASE << RYU_##upper##_MANTISSA_BITS) - 1); \
  uint32_t exponent = (bits >> RYU_##upper##_MANTISSA_BITS) & ((1u << RYU_##upper##_EXPONENT_BITS) - 1);               \
  if (exponent == ((1u << RYU_##upper##_EXPONENT_BITS) - 1u)) {                                                        \
    if (mantissa) return stringify_append(_state___, sb_append_strlit, "NaN");                                         \
    if (sign) stringify_append(_state___, sb_append, '-');                                                             \
    return stringify_append(_state___, sb_append_strlit, "Infinity");                                                  \
  }                                                                                                                    \
  if ((exponent == 0 && mantissa == 0)) {                                                                              \
    if (sign) stringify_append(_state___, sb_append, '-');                                                             \
    return stringify_append(_state___, sb_append_strlit, "0.0");                                                       \
  }                                                                                                                    \
  floating_decimal_##lower v = ryu_##lower##_parse(mantissa, exponent);                                                \
  if (sign) stringify_append(_state___, sb_append, '-');                                                               \
  if (!v.exponent) {                                                                                                   \
    stringify_append(_state___, sb_append_integer, v.mantissa);                                                        \
  } else if (v.exponent < 0) {                                                                                         \
    size_t exponent = -v.exponent;                                                                                     \
    size_t count = ryu_decimalLength_##lower(v.mantissa);                                                              \
    if (exponent >= count) stringify_append(_state___, sb_append, '0');                                                \
    stringify_append(_state___, sb_append_integer, v.mantissa);                                                        \
    stringify_append(_state___, sb_append, '.');                                                                       \
    if (_state___->sb) {                                                                                               \
      char *start = _state___->sb->data + _state___->sb->count - exponent - 1;                                         \
      memmove(start + 1, start, exponent);                                                                             \
      *start = '.';                                                                                                    \
    }                                                                                                                  \
  } else {                                                                                                             \
    stringify_append(_state___, sb_append_integer, v.mantissa);                                                        \
    stringify_append(_state___, sb_append_repeat, '0', v.exponent);                                                    \
  }                                                                                                                    \
})

size_t sb_append_float(String_Builder *sb, float value) {
  Stringify_State state = make_stringify_state(sb, 16);
  sb__append_floating(&state, value, float, FLOAT);
  return state.count;
}

size_t sb_append_double(String_Builder *sb, double value) {
  Stringify_State state = make_stringify_state(sb, 25);
  sb__append_floating(&state, value, double, DOUBLE);
  return state.count;
}

size_t sb_append_long_double(String_Builder *sb, long double value) {
  // ryu does not have separate support for `long double`
  // ryu_generic_128 has two cons:
  // - `long double` is not supported on most platforms (except for x86)
  // - it is performance heavy
  return sb_append_double(sb, (double)value);
}

#endif // SB_NUMBER_IMPL_C
