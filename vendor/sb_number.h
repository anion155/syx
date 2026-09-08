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

size_t sb__append_signed_integer(String_Builder *sb, intmax_t value, size_t max_width, intmax_t minimum);
#define sb_append_signed_integer(sb, value) ({                                                                \
  typeof((value)) _value_ = (value);                                                                          \
  sb__append_signed_integer((sb), _value_, INTEGER_DEC_WIDTH(typeof(_value_)), INTEGER_MIN(typeof(_value_))); \
})
size_t sb__append_unsigned_integer(String_Builder *sb, uintmax_t value, size_t max_width);
#define sb_append_unsigned_integer(sb, value) ({                                  \
  typeof((value)) _value_ = (value);                                              \
  sb__append_unsigned_integer((sb), _value_, INTEGER_DEC_WIDTH(typeof(_value_))); \
})
#define sb_append_integer(sb, value) ({                              \
  typeof((value)) _value_ = (value);                                 \
  typeof(_value_) minimum = INTEGER_MIN(typeof(_value_));            \
  size_t max_width = INTEGER_DEC_WIDTH(typeof(_value_));             \
  minimum < 0                                                        \
      ? sb__append_signed_integer((sb), _value_, max_width, minimum) \
      : sb__append_unsigned_integer((sb), _value_, max_width);       \
})
static inline size_t sb_append_integer_char(String_Builder *sb, char value) { return sb_append_signed_integer(sb, value); }
static inline size_t sb_append_integer_signed_char(String_Builder *sb, signed char value) { return sb_append_signed_integer(sb, value); }
static inline size_t sb_append_integer_short(String_Builder *sb, short value) { return sb_append_signed_integer(sb, value); }
static inline size_t sb_append_integer_int(String_Builder *sb, int value) { return sb_append_signed_integer(sb, value); }
static inline size_t sb_append_integer_long(String_Builder *sb, long value) { return sb_append_signed_integer(sb, value); }
static inline size_t sb_append_integer_long_long(String_Builder *sb, long long value) { return sb_append_signed_integer(sb, value); }
static inline size_t sb_append_integer_unsigned_char(String_Builder *sb, unsigned char value) { return sb_append_unsigned_integer(sb, value); }
static inline size_t sb_append_integer_unsigned_short(String_Builder *sb, unsigned short value) { return sb_append_unsigned_integer(sb, value); }
static inline size_t sb_append_integer_unsigned_int(String_Builder *sb, unsigned int value) { return sb_append_unsigned_integer(sb, value); }
static inline size_t sb_append_integer_unsigned_long(String_Builder *sb, unsigned long value) { return sb_append_unsigned_integer(sb, value); }
static inline size_t sb_append_integer_unsigned_long_long(String_Builder *sb, unsigned long long value) { return sb_append_unsigned_integer(sb, value); }

size_t sb_append_float(String_Builder *sb, float value);
size_t sb_append_double(String_Builder *sb, double value);
size_t sb_append_long_double(String_Builder *sb, long double value);
#define sb_append_floating(sb, value) _Generic((value), \
    float: sb_append_float,                             \
    double: sb_append_double,                           \
    long double: sb_append_long_double)((sb), (value))

size_t sb__append_float_hex(String_Builder *sb, float value, bool uppercase);
#define sb_append_float_hex(sb, value, ...) sb__append_float_hex((sb), (value), WITH_DEFAULT(false, __VA_ARGS__))
size_t sb__append_double_hex(String_Builder *sb, double value, bool uppercase);
#define sb_append_double_hex(sb, value, ...) sb__append_double_hex((sb), (value), WITH_DEFAULT(false, __VA_ARGS__))

#define sb_append_number(sb, value) _Generic((value),         \
    char: sb_append_integer_char,                             \
    signed char: sb_append_integer_signed_char,               \
    short: sb_append_integer_short,                           \
    int: sb_append_integer_int,                               \
    long: sb_append_integer_long,                             \
    long long: sb_append_integer_long_long,                   \
    unsigned char: sb_append_integer_unsigned_char,           \
    unsigned short: sb_append_integer_unsigned_short,         \
    unsigned int: sb_append_integer_unsigned_int,             \
    unsigned long: sb_append_integer_unsigned_long,           \
    unsigned long long: sb_append_integer_unsigned_long_long, \
    float: sb_append_float,                                   \
    double: sb_append_double,                                 \
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

size_t sb__append_signed_integer(String_Builder *sb, intmax_t value, size_t max_width, intmax_t minimum) {
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

typedef struct floating_double {
  RYU_DOUBLE_MANTISSA_STORAGE mantissa;
  uint32_t exponent;
  bool sign;
} floating_double;
typedef struct floating_float {
  RYU_FLOAT_MANTISSA_STORAGE mantissa;
  uint32_t exponent;
  bool sign;
} floating_float;

#define sb__floating_bits(value, lower, upper) ({                                                                      \
  RYU_##upper##_MANTISSA_STORAGE bits = ryu_##lower##_to_bits((value));                                                \
  bool sign = ((bits >> (RYU_##upper##_MANTISSA_BITS + RYU_##upper##_EXPONENT_BITS)) & 1) != 0;                        \
  RYU_##upper##_MANTISSA_STORAGE mantissa = bits & ((RYU_##upper##_MANTISSA_BASE << RYU_##upper##_MANTISSA_BITS) - 1); \
  uint32_t exponent = (bits >> RYU_##upper##_MANTISSA_BITS) & ((1u << RYU_##upper##_EXPONENT_BITS) - 1);               \
  (floating_##lower){.mantissa = mantissa, .exponent = exponent, .sign = sign};                                        \
})
#define sb__append_floating_special(state, f, lower, upper) ({                      \
  Stringify_State *_state___ = (state);                                             \
  floating_##lower *_f_ = (f);                                                      \
  if (_f_->exponent == ((1u << RYU_##upper##_EXPONENT_BITS) - 1u)) {                \
    if (_f_->mantissa) return stringify_append(_state___, sb_append_strlit, "NaN"); \
    if (_f_->sign) stringify_append(_state___, sb_append, '-');                     \
    return stringify_append(_state___, sb_append_strlit, "Infinity");               \
  }                                                                                 \
  if ((_f_->exponent == 0 && _f_->mantissa == 0)) {                                 \
    if (_f_->sign) stringify_append(_state___, sb_append, '-');                     \
    return stringify_append(_state___, sb_append_strlit, "0.0");                    \
  }                                                                                 \
  if (_f_->sign) stringify_append(_state___, sb_append, '-');                       \
  (void)0;                                                                          \
})
#define sb__append_floating_to_chars(state, v, lower, upper) ({                \
  Stringify_State *_state___ = (state);                                        \
  floating_decimal_##lower *_v_ = (v);                                         \
  if (!_v_->exponent) {                                                        \
    stringify_append(_state___, sb_append_unsigned_integer, _v_->mantissa);    \
  } else if (_v_->exponent < 0) {                                              \
    size_t exponent = -_v_->exponent;                                          \
    size_t count = ryu_decimalLength_##lower(_v_->mantissa);                   \
    if (exponent >= count) stringify_append(_state___, sb_append, '0');        \
    stringify_append(_state___, sb_append_unsigned_integer, _v_->mantissa);    \
    stringify_append(_state___, sb_append, '.');                               \
    if (_state___->sb) {                                                       \
      char *start = _state___->sb->data + _state___->sb->count - exponent - 1; \
      memmove(start + 1, start, exponent);                                     \
      *start = '.';                                                            \
    }                                                                          \
  } else {                                                                     \
    stringify_append(_state___, sb_append_unsigned_integer, _v_->mantissa);    \
    stringify_append(_state___, sb_append_repeat, '0', _v_->exponent);         \
  }                                                                            \
  (void)0;                                                                     \
})

size_t sb_append_float(String_Builder *sb, float value) {
  Stringify_State state = make_stringify_state(sb, 16);
  floating_float f = sb__floating_bits(value, float, FLOAT);
  sb__append_floating_special(&state, &f, float, FLOAT);
  floating_decimal_float v = ryu_float_parse(f.mantissa, f.exponent);
  sb__append_floating_to_chars(&state, &v, float, FLOAT);
  return state.count;
}

size_t sb_append_double(String_Builder *sb, double value) {
  Stringify_State state = make_stringify_state(sb, 25);
  floating_double f = sb__floating_bits(value, double, DOUBLE);
  sb__append_floating_special(&state, &f, double, DOUBLE);
  floating_decimal_double v = ryu_double_parse(f.mantissa, f.exponent);
  sb__append_floating_to_chars(&state, &v, double, DOUBLE);
  return state.count;
}

size_t sb_append_long_double(String_Builder *sb, long double value) {
  // ryu does not have separate support for `long double`
  // ryu_generic_128 has two cons:
  // - `long double` is not supported on most platforms (except for x86)
  // - it is performance heavy
  return sb_append_double(sb, (double)value);
}

#define sb__append_floating_hex_to_chars(state, f, lower, upper) ({                                    \
  Stringify_State *_state__ = (state);                                                                 \
  floating_##lower *_f_ = (f);                                                                         \
  stringify_append(_state__, sb_append_strlit, "0x");                                                  \
  stringify_append(_state__, sb_append, (_f_->exponent ? '1' : '0'));                                  \
  stringify_append(_state__, sb_append, '.');                                                          \
  size_t rev_start = _state__->count;                                                                  \
  bool print = false;                                                                                  \
  uint64_t it = _f_->mantissa;                                                                         \
  if (RYU_##upper##_MANTISSA_BITS % 4 != 0) it = it << (4 - RYU_##upper##_MANTISSA_BITS % 4);          \
  size_t length = (RYU_##upper##_MANTISSA_BITS >> 2) + (RYU_##upper##_MANTISSA_BITS & 0b11 ? 1 : 0);   \
  for (size_t index = 0; index < length; index += 1, it = it >> 4) {                                   \
    uint8_t digit = it & 0b1111;                                                                       \
    if (digit) print = true;                                                                           \
    else if (!print) continue;                                                                         \
    if (digit < 10) stringify_append(_state__, sb_append, '0' + digit);                                \
    else if (uppercase) stringify_append(_state__, sb_append, 'A' + digit - 10);                       \
    else stringify_append(_state__, sb_append, 'a' + digit - 10);                                      \
  }                                                                                                    \
  if (_state__->sb) __string_reverse(stringify_ptr(_state__, rev_start), _state__->count - rev_start); \
  stringify_append(_state__, sb_append, 'p');                                                          \
  int exponent = (int)_f_->exponent - (int)(((1u << RYU_##upper##_EXPONENT_BITS) - 1u) >> 1);          \
  if (exponent > 0) stringify_append(_state__, sb_append, '+');                                        \
  stringify_append(_state__, sb_append_signed_integer, exponent);                                      \
  (void)0;                                                                                             \
})

size_t sb__append_float_hex(String_Builder *sb, float value, bool uppercase) {
  Stringify_State state = make_stringify_state(sb, (RYU_FLOAT_MANTISSA_BITS >> 2) + 11);
  floating_float f = sb__floating_bits(value, float, FLOAT);
  sb__append_floating_special(&state, &f, float, FLOAT);
  sb__append_floating_hex_to_chars(&state, &f, float, FLOAT);
  return state.count;
}

size_t sb__append_double_hex(String_Builder *sb, double value, bool uppercase) {
  Stringify_State state = make_stringify_state(sb, (RYU_DOUBLE_MANTISSA_BITS >> 2) + 11);
  floating_double f = sb__floating_bits(value, double, DOUBLE);
  sb__append_floating_special(&state, &f, double, DOUBLE);
  sb__append_floating_hex_to_chars(&state, &f, double, DOUBLE);
  return state.count;
}

#endif // SB_NUMBER_IMPL_C
