#ifndef SB_NUMBER_H
#define SB_NUMBER_H

#include <limits.h>
#include <sb.h>
#include <stdint.h>

#ifndef INT128_MAX
#  define INT128_MAX ((__int128_t) ~(((__uint128_t)1) << 127))
#endif
#ifndef INT128_MIN
#  define INT128_MIN ((__int128_t)1 << 127)
#endif
#ifndef UINT128_MAX
#  define UINT128_MAX (~((__uint128_t)0))
#endif

typedef enum Sb_Integer_Format_Kind {
  SB_INTEGER_FORMAT_KIND_DECIMAL = 0,
  SB_INTEGER_FORMAT_KIND_BINARY,
  SB_INTEGER_FORMAT_KIND_OCTAL,
  SB_INTEGER_FORMAT_KIND_HEX,
  SB_INTEGER_FORMAT_KIND_HEX_BIG,
} Sb_Integer_Format_Kind;

typedef struct Sb_Integer_Format {
  Sb_Integer_Format_Kind kind;
  bool prefix;
  size_t min_width;
} Sb_Integer_Format;

size_t sb__append_integer_i8(String_Builder *sb, int8_t value, Sb_Integer_Format fmt);
size_t sb__append_integer_i16(String_Builder *sb, int16_t value, Sb_Integer_Format fmt);
size_t sb__append_integer_i32(String_Builder *sb, int32_t value, Sb_Integer_Format fmt);
size_t sb__append_integer_i64(String_Builder *sb, int64_t value, Sb_Integer_Format fmt);
size_t sb__append_integer_i128(String_Builder *sb, __int128_t value, Sb_Integer_Format fmt);
size_t sb__append_integer_u8(String_Builder *sb, uint8_t value, Sb_Integer_Format fmt);
size_t sb__append_integer_u16(String_Builder *sb, uint16_t value, Sb_Integer_Format fmt);
size_t sb__append_integer_u32(String_Builder *sb, uint32_t value, Sb_Integer_Format fmt);
size_t sb__append_integer_u64(String_Builder *sb, uint64_t value, Sb_Integer_Format fmt);
size_t sb__append_integer_u128(String_Builder *sb, __uint128_t value, Sb_Integer_Format fmt);
#define sb_append_integer_i8(sb, value, ...) sb__append_integer_i8((sb), (value), (Sb_Integer_Format){__VA_ARGS__})
#define sb_append_integer_i16(sb, value, ...) sb__append_integer_i16((sb), (value), (Sb_Integer_Format){__VA_ARGS__})
#define sb_append_integer_i32(sb, value, ...) sb__append_integer_i32((sb), (value), (Sb_Integer_Format){__VA_ARGS__})
#define sb_append_integer_i64(sb, value, ...) sb__append_integer_i64((sb), (value), (Sb_Integer_Format){__VA_ARGS__})
#define sb_append_integer_i128(sb, value, ...) sb__append_integer_i128((sb), (value), (Sb_Integer_Format){__VA_ARGS__})
#define sb_append_integer_u8(sb, value, ...) sb__append_integer_u8((sb), (value), (Sb_Integer_Format){__VA_ARGS__})
#define sb_append_integer_u16(sb, value, ...) sb__append_integer_u16((sb), (value), (Sb_Integer_Format){__VA_ARGS__})
#define sb_append_integer_u32(sb, value, ...) sb__append_integer_u32((sb), (value), (Sb_Integer_Format){__VA_ARGS__})
#define sb_append_integer_u64(sb, value, ...) sb__append_integer_u64((sb), (value), (Sb_Integer_Format){__VA_ARGS__})
#define sb_append_integer_u128(sb, value, ...) sb__append_integer_u128((sb), (value), (Sb_Integer_Format){__VA_ARGS__})

typedef enum Sb_Floating_Format_Kind {
  SB_FLOATING_FORMAT_KIND_DECIMAL = 0,
  SB_FLOATING_FORMAT_KIND_HEX,
  SB_FLOATING_FORMAT_KIND_HEX_BIG,
} Sb_Floating_Format_Kind;

typedef struct Sb_Floating_Format {
  Sb_Floating_Format_Kind kind;
  size_t min_width;
} Sb_Floating_Format;

size_t sb__append_floating_f16(String_Builder *sb, _Float16 value, Sb_Floating_Format fmt);
size_t sb__append_floating_f32(String_Builder *sb, float value, Sb_Floating_Format fmt);
size_t sb__append_floating_f64(String_Builder *sb, double value, Sb_Floating_Format fmt);
size_t sb__append_floating_f128(String_Builder *sb, long double value, Sb_Floating_Format fmt);
#define sb_append_floating_f16(sb, value, ...) sb__append_floating_f16((sb), (value), (Sb_Floating_Format){__VA_ARGS__})
#define sb_append_floating_f32(sb, value, ...) sb__append_floating_f32((sb), (value), (Sb_Floating_Format){__VA_ARGS__})
#define sb_append_floating_f64(sb, value, ...) sb__append_floating_f64((sb), (value), (Sb_Floating_Format){__VA_ARGS__})
#define sb_append_floating_f128(sb, value, ...) sb__append_floating_f128((sb), (value), (Sb_Floating_Format){__VA_ARGS__})

#if CHAR_MIN < 0
#  define SB_APPEND_NUMBER_FN_CHAR_CASE char : sb__append_integer_i8
#  define SB_APPEND_NUMBER_FN_SCHAR_CASE , SB_APPEND_NUMBER_FN_CHAR_CASE
#  define SB_APPEND_NUMBER_FN_UCHAR_CASE
#else
#  define SB_APPEND_NUMBER_FN_CHAR_CASE char : sb__append_integer_u8
#  define SB_APPEND_NUMBER_FN_SCHAR_CASE
#  define SB_APPEND_NUMBER_FN_UCHAR_CASE , SB_APPEND_NUMBER_FN_CHAR_CASE
#endif

#if SHRT_MAX == 32767
#  define SB_APPEND_NUMBER_FN_SSHORT_CASE signed short : sb__append_integer_i16
#  define SB_APPEND_NUMBER_FN_USHORT_CASE unsigned short : sb__append_integer_u16
#elif SHRT_MAX == 2147483647
#  define SB_APPEND_NUMBER_FN_SSHORT_CASE signed short : sb__append_integer_i32
#  define SB_APPEND_NUMBER_FN_USHORT_CASE unsigned short : sb__append_integer_u32
#else
#  error "short size not supported"
#endif
#define SB_APPEND_NUMBER_FN_SHORT_CASE SB_APPEND_NUMBER_FN_SSHORT_CASE, SB_APPEND_NUMBER_FN_USHORT_CASE

#if INT_MAX == 32767
#  define SB_APPEND_NUMBER_FN_SINT_CASE signed int : sb__append_integer_i16
#  define SB_APPEND_NUMBER_FN_UINT_CASE unsigned int : sb__append_integer_u16
#elif INT_MAX == 2147483647
#  define SB_APPEND_NUMBER_FN_SINT_CASE signed int : sb__append_integer_i32
#  define SB_APPEND_NUMBER_FN_UINT_CASE unsigned int : sb__append_integer_u32
#elif INT_MAX == 9223372036854775807
#  define SB_APPEND_NUMBER_FN_SINT_CASE signed int : sb__append_integer_i64
#  define SB_APPEND_NUMBER_FN_UINT_CASE unsigned int : sb__append_integer_u64
#else
#  error "int size not supported"
#endif
#define SB_APPEND_NUMBER_FN_INT_CASE SB_APPEND_NUMBER_FN_SINT_CASE, SB_APPEND_NUMBER_FN_UINT_CASE

#if LONG_MAX == 2147483647
#  define SB_APPEND_NUMBER_FN_SLONG_CASE signed long : sb__append_integer_i32
#  define SB_APPEND_NUMBER_FN_ULONG_CASE unsigned long : sb__append_integer_u32
#elif LONG_MAX == 9223372036854775807
#  define SB_APPEND_NUMBER_FN_SLONG_CASE signed long : sb__append_integer_i64
#  define SB_APPEND_NUMBER_FN_ULONG_CASE unsigned long : sb__append_integer_u64
#else
#  error "long size not supported"
#endif
#define SB_APPEND_NUMBER_FN_LONG_CASE SB_APPEND_NUMBER_FN_SLONG_CASE, SB_APPEND_NUMBER_FN_ULONG_CASE

#if LLONG_MAX == 9223372036854775807
#  define SB_APPEND_NUMBER_FN_SLLONG_CASE signed long long : sb__append_integer_i64
#  define SB_APPEND_NUMBER_FN_ULLONG_CASE unsigned long long : sb__append_integer_u64
#else
#  error "long size not supported"
#endif
#define SB_APPEND_NUMBER_FN_LLONG_CASE SB_APPEND_NUMBER_FN_SLLONG_CASE, SB_APPEND_NUMBER_FN_ULLONG_CASE

#if defined(__SIZEOF_INT128__)
#  define SB_APPEND_NUMBER_FN_SI128_CASE , __int128_t : sb__append_integer_i128
#  define SB_APPEND_NUMBER_FN_UI128_CASE , __uint128_t : sb__append_integer_u128
#else
#  define SB_APPEND_NUMBER_FN_I128_CASE
#endif
#define SB_APPEND_NUMBER_FN_I128_CASE SB_APPEND_NUMBER_FN_SI128_CASE SB_APPEND_NUMBER_FN_UI128_CASE

#if defined(__FLT16_MAX__)
#  define SB_APPEND_NUMBER_FN_F16_CASE , _Float16 : sb__append_floating_f16
#  define SB_APPEND_NUMBER_FMT_F16_CASE(...) \
    , _Float16 : (Sb_Floating_Format) { __VA_ARGS__ }
#else
#  define SB_APPEND_NUMBER_FN_F16_CASE
#  define SB_APPEND_NUMBER_FMT_F16_CASE(...)
#endif
#if defined(__FLT32_MAX__)
#  define SB_APPEND_NUMBER_FN_F32_CASE , _Float32 : sb__append_floating_f32
#  define SB_APPEND_NUMBER_FMT_F32_CASE(...) \
    , _Float32 : (Sb_Floating_Format) { __VA_ARGS__ }
#else
#  define SB_APPEND_NUMBER_FN_F32_CASE
#  define SB_APPEND_NUMBER_FMT_F32_CASE(...)
#endif
#if defined(__FLT64_MAX__)
#  define SB_APPEND_NUMBER_FN_F64_CASE , _Float64 : sb__append_floating_f64
#  define SB_APPEND_NUMBER_FMT_F64_CASE(...) \
    , _Float64 : (Sb_Floating_Format) { __VA_ARGS__ }
#else
#  define SB_APPEND_NUMBER_FN_F64_CASE
#  define SB_APPEND_NUMBER_FMT_F64_CASE(...)
#endif
#if defined(__FLT128_MAX__)
#  define SB_APPEND_NUMBER_FN_F128_CASE , _Float128 : sb__append_floating_f128
#  define SB_APPEND_NUMBER_FMT_F128_CASE(...) \
    , _Float128 : (Sb_Floating_Format) { __VA_ARGS__ }
#else
#  define SB_APPEND_NUMBER_FN_F128_CASE
#  define SB_APPEND_NUMBER_FMT_F128_CASE(...)
#endif
#define SB_APPEND_NUMBER_FN_FIXED_FLOATS_CASE SB_APPEND_NUMBER_FN_F16_CASE SB_APPEND_NUMBER_FN_F32_CASE SB_APPEND_NUMBER_FN_F64_CASE SB_APPEND_NUMBER_FN_F128_CASE
#define SB_APPEND_NUMBER_FMT_FIXED_FLOATS_CASE(...) SB_APPEND_NUMBER_FMT_F16_CASE(__VA_ARGS__) SB_APPEND_NUMBER_FMT_F32_CASE(__VA_ARGS__) SB_APPEND_NUMBER_FMT_F64_CASE(__VA_ARGS__) SB_APPEND_NUMBER_FMT_F128_CASE(__VA_ARGS__)

// clang-format off
#define sb_append_signed_integer(sb, value, ...) _Generic((value), \
  signed char: sb__append_integer_i8,                        \
  SB_APPEND_NUMBER_FN_SSHORT_CASE,                           \
  SB_APPEND_NUMBER_FN_SINT_CASE,                             \
  SB_APPEND_NUMBER_FN_SLONG_CASE,                            \
  SB_APPEND_NUMBER_FN_SLLONG_CASE                            \
  SB_APPEND_NUMBER_FN_SCHAR_CASE                             \
  SB_APPEND_NUMBER_FN_SI128_CASE)((sb), (value), (Sb_Integer_Format){__VA_ARGS__})

#define sb_append_unsigned_integer(sb, value, ...) _Generic((value), \
  unsigned char: sb__append_integer_u8,                      \
  SB_APPEND_NUMBER_FN_USHORT_CASE,                           \
  SB_APPEND_NUMBER_FN_UINT_CASE,                             \
  SB_APPEND_NUMBER_FN_ULONG_CASE,                            \
  SB_APPEND_NUMBER_FN_ULLONG_CASE                            \
  SB_APPEND_NUMBER_FN_UCHAR_CASE                             \
  SB_APPEND_NUMBER_FN_UI128_CASE)((sb), (value), (Sb_Integer_Format){__VA_ARGS__})

#define sb_append_integer(sb, value, ...) _Generic((value), \
  SB_APPEND_NUMBER_FN_CHAR_CASE,                            \
  signed char: sb__append_integer_i8,                       \
  unsigned char: sb__append_integer_u8,                     \
  SB_APPEND_NUMBER_FN_SHORT_CASE,                           \
  SB_APPEND_NUMBER_FN_INT_CASE,                             \
  SB_APPEND_NUMBER_FN_LONG_CASE,                            \
  SB_APPEND_NUMBER_FN_LLONG_CASE                            \
  SB_APPEND_NUMBER_FN_I128_CASE)((sb), (value), (Sb_Integer_Format){__VA_ARGS__})

#define sb_append_floating(sb, value, ...) _Generic((value), \
  float: sb__append_floating_f32,                            \
  double: sb__append_floating_f64,                           \
  long double: sb__append_floating_f128                      \
  SB_APPEND_NUMBER_FN_FIXED_FLOATS_CASE)((sb), (value), (Sb_Floating_Format){__VA_ARGS__})

#define sb_append_number(sb, value, ...) _Generic((value), \
  SB_APPEND_NUMBER_FN_CHAR_CASE,                           \
  signed char: sb__append_integer_i8,                      \
  unsigned char: sb__append_integer_u8,                    \
  SB_APPEND_NUMBER_FN_SHORT_CASE,                          \
  SB_APPEND_NUMBER_FN_INT_CASE,                            \
  SB_APPEND_NUMBER_FN_LONG_CASE,                           \
  SB_APPEND_NUMBER_FN_LLONG_CASE,                          \
  float: sb__append_floating_f32,                          \
  double: sb__append_floating_f64,                         \
  long double: sb__append_floating_f128                    \
  SB_APPEND_NUMBER_FN_I128_CASE                            \
  SB_APPEND_NUMBER_FN_FIXED_FLOATS_CASE)((sb), (value), _Generic((value), \
    char: (Sb_Integer_Format){__VA_ARGS__},               \
    signed char: (Sb_Integer_Format){__VA_ARGS__},        \
    unsigned char: (Sb_Integer_Format){__VA_ARGS__},      \
    signed short: (Sb_Integer_Format){__VA_ARGS__},       \
    unsigned short: (Sb_Integer_Format){__VA_ARGS__},     \
    signed int: (Sb_Integer_Format){__VA_ARGS__},         \
    unsigned int: (Sb_Integer_Format){__VA_ARGS__},       \
    signed long: (Sb_Integer_Format){__VA_ARGS__},        \
    unsigned long: (Sb_Integer_Format){__VA_ARGS__},      \
    signed long long: (Sb_Integer_Format){__VA_ARGS__},   \
    unsigned long long: (Sb_Integer_Format){__VA_ARGS__}, \
    float: (Sb_Floating_Format){__VA_ARGS__},             \
    double: (Sb_Floating_Format){__VA_ARGS__},            \
    long double: (Sb_Floating_Format){__VA_ARGS__}        \
    SB_APPEND_NUMBER_FMT_FIXED_FLOATS_CASE(__VA_ARGS__)   \
  ))
// clang-format on

#endif // SB_NUMBER_H

#if defined(SB_NUMBER_IMPL) && !defined(SB_NUMBER_IMPL_C)
#define SB_NUMBER_IMPL_C

#define ABORT_IMPL
#include <abort.h>
#define SB_IMPL
#include <sb.h>
#define RYU_IMPL
#include <ryu.h>

void __string_reverse(char *string, size_t width) {
  if (width < 2) return;
  char *left = string;
  char *right = string + width - 1;
  while (left < right) {
    char tmp = *left;
    *left++ = *right;
    *right-- = tmp;
  }
}

#define sb___unsigned_integer_max_width(fmt, decimal_width, binary_width) ({                   \
  fmt.kind == SB_INTEGER_FORMAT_KIND_DECIMAL  ? decimal_width                                  \
  : fmt.kind == SB_INTEGER_FORMAT_KIND_BINARY ? binary_width                                   \
  : fmt.kind == SB_INTEGER_FORMAT_KIND_OCTAL  ? binary_width / 3 + (binary_width % 3 ? 1 : 0)  \
                                              : binary_width / 4 + (binary_width % 4 ? 1 : 0); \
})
#define sb___append_integer_prefix(state, fmt) ({                                                     \
  if (fmt.prefix) {                                                                                   \
    if (state.sb) {                                                                                   \
      switch (fmt.kind) {                                                                             \
        case SB_INTEGER_FORMAT_KIND_BINARY: stringify_append(&state, sb_append_strlit, "0b"); break;  \
        case SB_INTEGER_FORMAT_KIND_OCTAL: stringify_append(&state, sb_append_strlit, "0o"); break;   \
        case SB_INTEGER_FORMAT_KIND_DECIMAL: break;                                                   \
        case SB_INTEGER_FORMAT_KIND_HEX: stringify_append(&state, sb_append_strlit, "0x"); break;     \
        case SB_INTEGER_FORMAT_KIND_HEX_BIG: stringify_append(&state, sb_append_strlit, "0x"); break; \
      }                                                                                               \
    } else {                                                                                          \
      if (fmt.kind != SB_INTEGER_FORMAT_KIND_DECIMAL) state.count += 2;                               \
    }                                                                                                 \
  }                                                                                                   \
})
#define sb___stringify_unsigned_integer(state, value, fmt, digits_start) ({                                              \
  if (state.sb) {                                                                                                        \
    switch (fmt.kind) {                                                                                                  \
      case SB_INTEGER_FORMAT_KIND_BINARY: {                                                                              \
        for (typeof(value) it = value; it != 0; it = it >> 1) stringify_append(&state, sb_append, it & 0b1 ? '1' : '0'); \
      } break;                                                                                                           \
      case SB_INTEGER_FORMAT_KIND_OCTAL: {                                                                               \
        for (typeof(value) it = value; it != 0; it = it >> 3) stringify_append(&state, sb_append, '0' + (it & 0b111));   \
      } break;                                                                                                           \
      case SB_INTEGER_FORMAT_KIND_DECIMAL: {                                                                             \
        for (typeof(value) it = value; it != 0; it /= 10) stringify_append(&state, sb_append, '0' + it % 10);            \
      } break;                                                                                                           \
      case SB_INTEGER_FORMAT_KIND_HEX: {                                                                                 \
        for (typeof(value) it = value; it != 0; it = it >> 4) {                                                          \
          uint8_t digit = it & 0b1111;                                                                                   \
          if (digit < 10) stringify_append(&state, sb_append, '0' + digit);                                              \
          else stringify_append(&state, sb_append, 'a' + (digit - 10));                                                  \
        }                                                                                                                \
      } break;                                                                                                           \
      case SB_INTEGER_FORMAT_KIND_HEX_BIG: {                                                                             \
        for (typeof(value) it = value; it != 0; it = it >> 4) {                                                          \
          uint8_t digit = it & 0b1111;                                                                                   \
          if (digit < 10) stringify_append(&state, sb_append, '0' + digit);                                              \
          else stringify_append(&state, sb_append, 'A' + (digit - 10));                                                  \
        }                                                                                                                \
      } break;                                                                                                           \
    }                                                                                                                    \
    __string_reverse(stringify_ptr(&state, digits_start), state.count - digits_start);                                   \
  } else {                                                                                                               \
    switch (fmt.kind) {                                                                                                  \
      case SB_INTEGER_FORMAT_KIND_BINARY: {                                                                              \
        for (typeof(value) it = value; it != 0; it = it >> 1) state.count += 1;                                          \
      } break;                                                                                                           \
      case SB_INTEGER_FORMAT_KIND_OCTAL: {                                                                               \
        for (typeof(value) it = value; it != 0; it = it >> 3) state.count += 1;                                          \
      } break;                                                                                                           \
      case SB_INTEGER_FORMAT_KIND_DECIMAL: {                                                                             \
        for (typeof(value) it = value; it != 0; it /= 10) state.count += 1;                                              \
      } break;                                                                                                           \
      case SB_INTEGER_FORMAT_KIND_HEX:                                                                                   \
      case SB_INTEGER_FORMAT_KIND_HEX_BIG: {                                                                             \
        for (typeof(value) it = value; it != 0; it = it >> 4) state.count += 1;                                          \
      } break;                                                                                                           \
    }                                                                                                                    \
  }                                                                                                                      \
})

#define sb___format_padding_unsigned_integer(state, value, fmt, digits_start) ({    \
  size_t number_width = state.count - digits_start;                                 \
  if (state.sb) {                                                                   \
    if (fmt.min_width && fmt.min_width > number_width) {                            \
      size_t unfilled = fmt.min_width - number_width;                               \
      stringify_append(&state, sb_append_repeat, '0', unfilled);                    \
      char *ptr = stringify_ptr(&state, digits_start);                              \
      memmove(ptr + unfilled, ptr, number_width);                                   \
      memset(ptr, '0', unfilled);                                                   \
    }                                                                               \
  } else {                                                                          \
    if (fmt.min_width && fmt.min_width > number_width) state.count = fmt.min_width; \
  }                                                                                 \
})

#define sb___append_signed_integer(sb, value, fmt, minimal, binary_width, decimal_width) ({                                \
  Stringify_State state = make_stringify_state(sb, sb___unsigned_integer_max_width(fmt, decimal_width, binary_width) + 1); \
  if (value < 0) stringify_append(&state, sb_append, '-');                                                                 \
  sb___append_integer_prefix(state, fmt);                                                                                  \
  size_t digits_start = state.count;                                                                                       \
  if (value == 0) {                                                                                                        \
    stringify_append(&state, sb_append, '0');                                                                              \
  } else if (value == minimal) {                                                                                           \
    if (state.sb) {                                                                                                        \
      switch (fmt.kind) {                                                                                                  \
        case SB_INTEGER_FORMAT_KIND_BINARY: {                                                                              \
          stringify_append(&state, sb_append, '1');                                                                        \
          for (size_t i = 1; i < binary_width; i += 1) stringify_append(&state, sb_append, '0');                           \
        } break;                                                                                                           \
        case SB_INTEGER_FORMAT_KIND_OCTAL: {                                                                               \
          stringify_append(&state, sb_append, '0' + (1 << ((binary_width - 1) % 3)));                                      \
          for (size_t i = 0, c = binary_width / 3; i < c; i += 1) {                                                        \
            stringify_append(&state, sb_append, '0');                                                                      \
          }                                                                                                                \
        } break;                                                                                                           \
        case SB_INTEGER_FORMAT_KIND_DECIMAL: {                                                                             \
          typeof(value) it = value;                                                                                        \
          stringify_append(&state, sb_append, '0' - (it % 10));                                                            \
          it = -(it / 10);                                                                                                 \
          for (; it != 0; it /= 10) stringify_append(&state, sb_append, '0' + it % 10);                                    \
          __string_reverse(stringify_ptr(&state, digits_start), state.count - digits_start);                               \
        } break;                                                                                                           \
        case SB_INTEGER_FORMAT_KIND_HEX:                                                                                   \
        case SB_INTEGER_FORMAT_KIND_HEX_BIG: {                                                                             \
          stringify_append(&state, sb_append, '8');                                                                        \
          for (size_t i = 0, c = binary_width / 4 - 1; i < c; i += 1) stringify_append(&state, sb_append, '0');            \
        } break;                                                                                                           \
      }                                                                                                                    \
    } else {                                                                                                               \
      switch (fmt.kind) {                                                                                                  \
        case SB_INTEGER_FORMAT_KIND_BINARY: state.count += binary_width; break;                                            \
        case SB_INTEGER_FORMAT_KIND_OCTAL: state.count += binary_width / 3 + (binary_width % 3 ? 1 : 0); break;            \
        case SB_INTEGER_FORMAT_KIND_DECIMAL: state.count += decimal_width; break;                                          \
        case SB_INTEGER_FORMAT_KIND_HEX: state.count += binary_width / 4 + (binary_width % 4 ? 1 : 0); break;              \
        case SB_INTEGER_FORMAT_KIND_HEX_BIG: state.count += binary_width / 4 + (binary_width % 4 ? 1 : 0); break;          \
      }                                                                                                                    \
    }                                                                                                                      \
  } else {                                                                                                                 \
    if (value < 0) value = 0 - value;                                                                                      \
    sb___stringify_unsigned_integer(state, value, fmt, digits_start);                                                      \
  }                                                                                                                        \
  sb___format_padding_unsigned_integer(state, value, fmt, digits_start);                                                   \
  state.count;                                                                                                             \
})
size_t sb__append_integer_i8(String_Builder *sb, int8_t value, Sb_Integer_Format fmt) { return sb___append_signed_integer(sb, value, fmt, INT8_MIN, 8, 3); }
size_t sb__append_integer_i16(String_Builder *sb, int16_t value, Sb_Integer_Format fmt) { return sb___append_signed_integer(sb, value, fmt, INT16_MIN, 16, 5); }
size_t sb__append_integer_i32(String_Builder *sb, int32_t value, Sb_Integer_Format fmt) { return sb___append_signed_integer(sb, value, fmt, INT32_MIN, 32, 10); }
size_t sb__append_integer_i64(String_Builder *sb, int64_t value, Sb_Integer_Format fmt) { return sb___append_signed_integer(sb, value, fmt, INT64_MIN, 64, 19); }
size_t sb__append_integer_i128(String_Builder *sb, __int128_t value, Sb_Integer_Format fmt) { return sb___append_signed_integer(sb, value, fmt, INT128_MIN, 128, 39); }
#undef sb___append_signed_integer

#define sb___append_unsigned_integer(sb, value, fmt, binary_width, decimal_width) ({                                   \
  Stringify_State state = make_stringify_state(sb, sb___unsigned_integer_max_width(fmt, decimal_width, binary_width)); \
  sb___append_integer_prefix(state, fmt);                                                                              \
  size_t digits_start = state.count;                                                                                   \
  if (value == 0) {                                                                                                    \
    stringify_append(&state, sb_append, '0');                                                                          \
  } else {                                                                                                             \
    sb___stringify_unsigned_integer(state, value, fmt, digits_start);                                                  \
  }                                                                                                                    \
  sb___format_padding_unsigned_integer(state, value, fmt, digits_start);                                               \
  state.count;                                                                                                         \
})
size_t sb__append_integer_u8(String_Builder *sb, uint8_t value, Sb_Integer_Format fmt) { return sb___append_unsigned_integer(sb, value, fmt, 8, 3); }
size_t sb__append_integer_u16(String_Builder *sb, uint16_t value, Sb_Integer_Format fmt) { return sb___append_unsigned_integer(sb, value, fmt, 16, 5); }
size_t sb__append_integer_u32(String_Builder *sb, uint32_t value, Sb_Integer_Format fmt) { return sb___append_unsigned_integer(sb, value, fmt, 32, 10); }
size_t sb__append_integer_u64(String_Builder *sb, uint64_t value, Sb_Integer_Format fmt) { return sb___append_unsigned_integer(sb, value, fmt, 64, 19); }
size_t sb__append_integer_u128(String_Builder *sb, __uint128_t value, Sb_Integer_Format fmt) { return sb___append_unsigned_integer(sb, value, fmt, 128, 39); }
#undef sb___append_unsigned_integer

#undef sb___unsigned_integer_max_width
#undef sb___stringify_unsigned_integer
#undef sb___format_padding_unsigned_integer

struct floating_f16 {
  typeof((struct floating_decimal_f16){0}.mantissa) mantissa;
  uint32_t exponent;
  bool sign;
};

struct floating_f32 {
  typeof((struct floating_decimal_f32){0}.mantissa) mantissa;
  uint32_t exponent;
  bool sign;
};

struct floating_f64 {
  typeof((struct floating_decimal_f64){0}.mantissa) mantissa;
  uint32_t exponent;
  bool sign;
};

struct floating_f128 {
  typeof((struct floating_decimal_f128){0}.mantissa) mantissa;
  uint32_t exponent;
  bool sign;
};

#define sb___ryu_floating_to_bits(value, binary_width) ({                                                                                                             \
  typeof((struct floating_decimal_f##binary_width){0}.mantissa) bits = ryu_f##binary_width##_to_bits(value);                                                          \
  bool sign = ((bits >> (RYU_F##binary_width##_MANTISSA_BITS + RYU_F##binary_width##_EXPONENT_BITS)) & 1) != 0;                                                       \
  typeof((struct floating_decimal_f##binary_width){0}.mantissa) mantissa = bits & ((RYU_F##binary_width##_MANTISSA_BASE << RYU_F##binary_width##_MANTISSA_BITS) - 1); \
  uint32_t exponent = (bits >> RYU_F##binary_width##_MANTISSA_BITS) & ((1u << RYU_F##binary_width##_EXPONENT_BITS) - 1);                                              \
  (struct floating_f##binary_width){.mantissa = mantissa, .exponent = exponent, .sign = sign};                                                                        \
})

void sb___floating_format_special(Stringify_State *state, Sb_Floating_Format *fmt, size_t special_start) {
  size_t width = state->count - special_start;
  if (!fmt->min_width || fmt->min_width <= width) return;
  size_t unfilled = fmt->min_width - width;
  if (!state->sb) return (void)(state->count += unfilled);
  stringify_append(state, sb_append_repeat, ' ', unfilled);
  char *start = state->sb->data + state->sb->count - fmt->min_width;
  memmove(start + unfilled, start, width);
  memset(start, ' ', unfilled);
}
#define sb___floating_detect_special(state, f, fmt, binary_width) ({      \
  size_t special_start = state.count;                                     \
  if (f.exponent == ((1u << RYU_F##binary_width##_EXPONENT_BITS) - 1u)) { \
    if (f.mantissa) {                                                     \
      stringify_append(&state, sb_append_strlit, "NaN");                  \
      sb___floating_format_special(&state, &fmt, special_start);          \
      return state.count;                                                 \
    }                                                                     \
    if (f.sign) stringify_append(&state, sb_append, '-');                 \
    stringify_append(&state, sb_append_strlit, "Infinity");               \
    sb___floating_format_special(&state, &fmt, special_start);            \
    return state.count;                                                   \
  }                                                                       \
  if ((f.exponent == 0 && f.mantissa == 0)) {                             \
    if (f.sign) stringify_append(&state, sb_append, '-');                 \
    stringify_append(&state, sb_append_strlit, "0.0");                    \
    sb___floating_format_special(&state, &fmt, special_start);            \
    return state.count;                                                   \
  }                                                                       \
  if (f.sign) stringify_append(&state, sb_append, '-');                   \
  (void)0;                                                                \
})

#define sb___ryu_floating_to_decimal_chars(state, f, binary_width) ({                       \
  floating_decimal_f##binary_width v = ryu_f##binary_width##_parse(f.mantissa, f.exponent); \
  if (!v.exponent) {                                                                        \
    stringify_append(&state, sb_append_unsigned_integer, v.mantissa);                       \
  } else if (v.exponent < 0) {                                                              \
    size_t exponent = -v.exponent;                                                          \
    size_t count = ryu_decimalLength_f##binary_width(v.mantissa);                           \
    if (exponent >= count) stringify_append(&state, sb_append, '0');                        \
    stringify_append(&state, sb_append_unsigned_integer, v.mantissa);                       \
    stringify_append(&state, sb_append, '.');                                               \
    if (state.sb) {                                                                         \
      char *start = state.sb->data + state.sb->count - exponent - 1;                        \
      memmove(start + 1, start, exponent);                                                  \
      *start = '.';                                                                         \
    }                                                                                       \
  } else {                                                                                  \
    stringify_append(&state, sb_append_unsigned_integer, v.mantissa);                       \
    stringify_append(&state, sb_append_repeat, '0', v.exponent);                            \
  }                                                                                         \
  (void)0;                                                                                  \
})

#define sb___ryu_floating_to_hex_chars(state, f, binary_width, kind, fmt_min_width) ({                                                 \
  stringify_append(&state, sb_append_strlit, "0x");                                                                                    \
  stringify_append(&state, sb_append, (f.exponent ? '1' : '0'));                                                                       \
  stringify_append(&state, sb_append, '.');                                                                                            \
  stringify_append(&state, sb_append_unsigned_integer, f.mantissa, .kind = SB_INTEGER_FORMAT_KIND_##kind, .min_width = fmt_min_width); \
  stringify_append(&state, sb_append, 'p');                                                                                            \
  int64_t exponent = (int64_t)f.exponent - (int64_t)(((1u << RYU_F##binary_width##_EXPONENT_BITS) - 1u) >> 1);                         \
  if (exponent > 0) stringify_append(&state, sb_append, '+');                                                                          \
  stringify_append(&state, sb_append_signed_integer, exponent);                                                                        \
  (void)0;                                                                                                                             \
})

#define sb___append_ryu_floating(sb, value, fmt, binary_width, max_width) ({                                                     \
  Stringify_State state = make_stringify_state(sb, max_width);                                                                   \
  struct floating_f##binary_width f = sb___ryu_floating_to_bits(value, binary_width);                                            \
  sb___floating_detect_special(state, f, fmt, binary_width);                                                                     \
  switch (fmt.kind) {                                                                                                            \
    case SB_FLOATING_FORMAT_KIND_DECIMAL: sb___ryu_floating_to_decimal_chars(state, f, binary_width); break;                     \
    case SB_FLOATING_FORMAT_KIND_HEX: sb___ryu_floating_to_hex_chars(state, f, binary_width, HEX, fmt.min_width); break;         \
    case SB_FLOATING_FORMAT_KIND_HEX_BIG: sb___ryu_floating_to_hex_chars(state, f, binary_width, HEX_BIG, fmt.min_width); break; \
  }                                                                                                                              \
  state.count;                                                                                                                   \
})

size_t sb__append_floating_f16(String_Builder *sb, _Float16 value, Sb_Floating_Format fmt) { return sb___append_ryu_floating(sb, value, fmt, 16, 8); }
size_t sb__append_floating_f32(String_Builder *sb, float value, Sb_Floating_Format fmt) { return sb___append_ryu_floating(sb, value, fmt, 32, 16); }
size_t sb__append_floating_f64(String_Builder *sb, double value, Sb_Floating_Format fmt) { return sb___append_ryu_floating(sb, value, fmt, 64, 27); }
// size_t sb__append_floating_f128(String_Builder *sb, long double value, Sb_Floating_Format fmt) { return sb___append_ryu_floating(sb, value, fmt, 128, 36); }
size_t sb__append_floating_f128(String_Builder *sb, long double value, Sb_Floating_Format fmt) { TODO(); }

#undef sb___ryu_floating_to_bits
#undef sb___floating_detect_special
#undef sb___ryu_floating_to_decimal_chars
#undef sb___ryu_floating_to_hex_chars
#undef sb___append_ryu_floating

#endif // SB_NUMBER_IMPL_C
