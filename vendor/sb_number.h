#ifndef SB_NUMBER_H
#define SB_NUMBER_H

#include <defines.h>
#include <floats.h>
#include <limits.h>
#include <ryu.h>
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

size_t sb_append_integer_i8_fmt(String_Builder *sb, int8_t value, Sb_Integer_Format fmt);
size_t sb_append_integer_i16_fmt(String_Builder *sb, int16_t value, Sb_Integer_Format fmt);
size_t sb_append_integer_i32_fmt(String_Builder *sb, int32_t value, Sb_Integer_Format fmt);
size_t sb_append_integer_i64_fmt(String_Builder *sb, int64_t value, Sb_Integer_Format fmt);
size_t sb_append_integer_i128_fmt(String_Builder *sb, __int128_t value, Sb_Integer_Format fmt);
size_t sb_append_integer_u8_fmt(String_Builder *sb, uint8_t value, Sb_Integer_Format fmt);
size_t sb_append_integer_u16_fmt(String_Builder *sb, uint16_t value, Sb_Integer_Format fmt);
size_t sb_append_integer_u32_fmt(String_Builder *sb, uint32_t value, Sb_Integer_Format fmt);
size_t sb_append_integer_u64_fmt(String_Builder *sb, uint64_t value, Sb_Integer_Format fmt);
size_t sb_append_integer_u128_fmt(String_Builder *sb, __uint128_t value, Sb_Integer_Format fmt);
#define sb_append_integer_i8(sb, value, ...) sb_append_integer_i8_fmt((sb), (value), ((Sb_Integer_Format){__VA_ARGS__}))
#define sb_append_integer_i16(sb, value, ...) sb_append_integer_i16_fmt((sb), (value), ((Sb_Integer_Format){__VA_ARGS__}))
#define sb_append_integer_i32(sb, value, ...) sb_append_integer_i32_fmt((sb), (value), ((Sb_Integer_Format){__VA_ARGS__}))
#define sb_append_integer_i64(sb, value, ...) sb_append_integer_i64_fmt((sb), (value), ((Sb_Integer_Format){__VA_ARGS__}))
#define sb_append_integer_i128(sb, value, ...) sb_append_integer_i128_fmt((sb), (value), ((Sb_Integer_Format){__VA_ARGS__}))
#define sb_append_integer_u8(sb, value, ...) sb_append_integer_u8_fmt((sb), (value), ((Sb_Integer_Format){__VA_ARGS__}))
#define sb_append_integer_u16(sb, value, ...) sb_append_integer_u16_fmt((sb), (value), ((Sb_Integer_Format){__VA_ARGS__}))
#define sb_append_integer_u32(sb, value, ...) sb_append_integer_u32_fmt((sb), (value), ((Sb_Integer_Format){__VA_ARGS__}))
#define sb_append_integer_u64(sb, value, ...) sb_append_integer_u64_fmt((sb), (value), ((Sb_Integer_Format){__VA_ARGS__}))
#define sb_append_integer_u128(sb, value, ...) sb_append_integer_u128_fmt((sb), (value), ((Sb_Integer_Format){__VA_ARGS__}))

typedef enum Sb_Floating_Format_Kind {
  SB_FLOATING_FORMAT_KIND_DECIMAL = 0,
  SB_FLOATING_FORMAT_KIND_FIXED,
  SB_FLOATING_FORMAT_KIND_HEX,
  SB_FLOATING_FORMAT_KIND_HEX_BIG,
} Sb_Floating_Format_Kind;

typedef struct Sb_Floating_Format {
  Sb_Floating_Format_Kind kind;
  size_t min_width;
  size_t precision;
} Sb_Floating_Format;

size_t sb_append_floating_f16_fmt(String_Builder *sb, f16_t value, Sb_Floating_Format fmt);
size_t sb_append_floating_f32_fmt(String_Builder *sb, float value, Sb_Floating_Format fmt);
size_t sb_append_floating_f64_fmt(String_Builder *sb, double value, Sb_Floating_Format fmt);
size_t sb_append_floating_f80_fmt(String_Builder *sb, f80_t value, Sb_Floating_Format fmt);
size_t sb_append_floating_f128_fmt(String_Builder *sb, f128_t value, Sb_Floating_Format fmt);
size_t sb_append_floating_f64pair_fmt(String_Builder *sb, f64pair_t value, Sb_Floating_Format fmt);
size_t sb_append_floating_f16_canonical_fmt(String_Builder *sb, f16_canonical_t value, Sb_Floating_Format fmt);
size_t sb_append_floating_f80_canonical_fmt(String_Builder *sb, f80_canonical_t value, Sb_Floating_Format fmt);
size_t sb_append_floating_f128_canonical_fmt(String_Builder *sb, f128_canonical_t value, Sb_Floating_Format fmt);
size_t sb_append_floating_f64pair_canonical_fmt(String_Builder *sb, f64pair_canonical_t value, Sb_Floating_Format fmt);
#define sb_append_floating_f16(sb, value, ...) sb_append_floating_f16_fmt((sb), (value), ((Sb_Floating_Format){__VA_ARGS__}))
#define sb_append_floating_f32(sb, value, ...) sb_append_floating_f32_fmt((sb), (value), ((Sb_Floating_Format){__VA_ARGS__}))
#define sb_append_floating_f64(sb, value, ...) sb_append_floating_f64_fmt((sb), (value), ((Sb_Floating_Format){__VA_ARGS__}))
#define sb_append_floating_f80(sb, value, ...) sb_append_floating_f80_fmt((sb), (value), ((Sb_Floating_Format){__VA_ARGS__}))
#define sb_append_floating_f128(sb, value, ...) sb_append_floating_f128_fmt((sb), (value), ((Sb_Floating_Format){__VA_ARGS__}))
#define sb_append_floating_f64pair(sb, value, ...) sb_append_floating_f64pair_fmt((sb), (value), ((Sb_Floating_Format){__VA_ARGS__}))
#define sb_append_floating_f16_canonical(sb, value, ...) sb_append_floating_f16_canonical_fmt((sb), (value), ((Sb_Floating_Format){__VA_ARGS__}))
#define sb_append_floating_f80_canonical(sb, value, ...) sb_append_floating_f80_canonical_fmt((sb), (value), ((Sb_Floating_Format){__VA_ARGS__}))
#define sb_append_floating_f128_canonical(sb, value, ...) sb_append_floating_f128_canonical_fmt((sb), (value), ((Sb_Floating_Format){__VA_ARGS__}))
#define sb_append_floating_f64pair_canonical(sb, value, ...) sb_append_floating_f64pair_canonical_fmt((sb), (value), ((Sb_Floating_Format){__VA_ARGS__}))

#if CHAR_MIN < 0
#  define SB_APPEND_NUMBER_FN_CHAR_CASE char : sb_append_integer_i8_fmt
#  define SB_APPEND_NUMBER_FN_SCHAR_CASE , SB_APPEND_NUMBER_FN_CHAR_CASE
#  define SB_APPEND_NUMBER_FN_UCHAR_CASE
#else
#  define SB_APPEND_NUMBER_FN_CHAR_CASE char : sb_append_integer_u8_fmt
#  define SB_APPEND_NUMBER_FN_SCHAR_CASE
#  define SB_APPEND_NUMBER_FN_UCHAR_CASE , SB_APPEND_NUMBER_FN_CHAR_CASE
#endif

#if SHRT_MAX == 32767
#  define SB_APPEND_NUMBER_FN_SSHORT_CASE signed short : sb_append_integer_i16_fmt
#  define SB_APPEND_NUMBER_FN_USHORT_CASE unsigned short : sb_append_integer_u16_fmt
#elif SHRT_MAX == 2147483647
#  define SB_APPEND_NUMBER_FN_SSHORT_CASE signed short : sb_append_integer_i32_fmt
#  define SB_APPEND_NUMBER_FN_USHORT_CASE unsigned short : sb_append_integer_u32_fmt
#else
#  error "short size not supported"
#endif
#define SB_APPEND_NUMBER_FN_SHORT_CASE SB_APPEND_NUMBER_FN_SSHORT_CASE, SB_APPEND_NUMBER_FN_USHORT_CASE

#if INT_MAX == 32767
#  define SB_APPEND_NUMBER_FN_SINT_CASE signed int : sb_append_integer_i16_fmt
#  define SB_APPEND_NUMBER_FN_UINT_CASE unsigned int : sb_append_integer_u16_fmt
#elif INT_MAX == 2147483647
#  define SB_APPEND_NUMBER_FN_SINT_CASE signed int : sb_append_integer_i32_fmt
#  define SB_APPEND_NUMBER_FN_UINT_CASE unsigned int : sb_append_integer_u32_fmt
#elif INT_MAX == 9223372036854775807
#  define SB_APPEND_NUMBER_FN_SINT_CASE signed int : sb_append_integer_i64_fmt
#  define SB_APPEND_NUMBER_FN_UINT_CASE unsigned int : sb_append_integer_u64_fmt
#else
#  error "int size not supported"
#endif
#define SB_APPEND_NUMBER_FN_INT_CASE SB_APPEND_NUMBER_FN_SINT_CASE, SB_APPEND_NUMBER_FN_UINT_CASE

#if LONG_MAX == 2147483647
#  define SB_APPEND_NUMBER_FN_SLONG_CASE signed long : sb_append_integer_i32_fmt
#  define SB_APPEND_NUMBER_FN_ULONG_CASE unsigned long : sb_append_integer_u32_fmt
#elif LONG_MAX == 9223372036854775807
#  define SB_APPEND_NUMBER_FN_SLONG_CASE signed long : sb_append_integer_i64_fmt
#  define SB_APPEND_NUMBER_FN_ULONG_CASE unsigned long : sb_append_integer_u64_fmt
#else
#  error "long size not supported"
#endif
#define SB_APPEND_NUMBER_FN_LONG_CASE SB_APPEND_NUMBER_FN_SLONG_CASE, SB_APPEND_NUMBER_FN_ULONG_CASE

#if LLONG_MAX == 9223372036854775807
#  define SB_APPEND_NUMBER_FN_SLLONG_CASE signed long long : sb_append_integer_i64_fmt
#  define SB_APPEND_NUMBER_FN_ULLONG_CASE unsigned long long : sb_append_integer_u64_fmt
#else
#  error "long size not supported"
#endif
#define SB_APPEND_NUMBER_FN_LLONG_CASE SB_APPEND_NUMBER_FN_SLLONG_CASE, SB_APPEND_NUMBER_FN_ULLONG_CASE

#if defined(__SIZEOF_INT128__)
#  define SB_APPEND_NUMBER_FN_SI128_CASE , __int128_t : sb_append_integer_i128_fmt
#  define SB_APPEND_NUMBER_FN_UI128_CASE , __uint128_t : sb_append_integer_u128_fmt
#  define SB_APPEND_NUMBER_FMT_SI128_CASE , __int128_t : ((Sb_Integer_Format){0})
#  define SB_APPEND_NUMBER_FMT_UI128_CASE , __uint128_t : ((Sb_Integer_Format){0})
#else
#  error "128bit integer type not supported"
#endif
#define SB_APPEND_NUMBER_FN_I128_CASE SB_APPEND_NUMBER_FN_SI128_CASE SB_APPEND_NUMBER_FN_UI128_CASE
#define SB_APPEND_NUMBER_FMT_I128_CASE SB_APPEND_NUMBER_FMT_SI128_CASE SB_APPEND_NUMBER_FMT_UI128_CASE

#if defined(__FLT16_MAX__)
#  define SB_APPEND_NUMBER_FN_F16_CASE , _Float16 : sb_append_floating_f16_fmt, f16_canonical_t : sb_append_floating_f16_canonical_fmt
#  define SB_APPEND_NUMBER_FMT_F16_CASE , _Float16 : ((Sb_Floating_Format){0}), f16_canonical_t : ((Sb_Floating_Format){0})
#else
#  define SB_APPEND_NUMBER_FN_F16_CASE , f16_canonical_t : sb_append_floating_f16_canonical_fmt
#  define SB_APPEND_NUMBER_FMT_F16_CASE , f16_canonical_t : ((Sb_Floating_Format){0})
#endif
#if defined(__FLT32_MAX__)
#  define SB_APPEND_NUMBER_FN_F32_CASE , _Float32 : sb_append_floating_f32_fmt
#  define SB_APPEND_NUMBER_FMT_F32_CASE , _Float32 : ((Sb_Floating_Format){0})
#else
#  define SB_APPEND_NUMBER_FN_F32_CASE
#  define SB_APPEND_NUMBER_FMT_F32_CASE
#endif
#if defined(__FLT64_MAX__)
#  define SB_APPEND_NUMBER_FN_F64_CASE , _Float64 : sb_append_floating_f64_fmt
#  define SB_APPEND_NUMBER_FMT_F64_CASE , _Float64 : ((Sb_Floating_Format){0})
#else
#  define SB_APPEND_NUMBER_FN_F64_CASE
#  define SB_APPEND_NUMBER_FMT_F64_CASE
#endif
#if defined(__FLT128_MAX__)
#  define SB_APPEND_NUMBER_FN_F128_CASE , _Float128 : sb_append_floating_f128_fmt
#  define SB_APPEND_NUMBER_FMT_F128_CASE , _Float128 : ((Sb_Floating_Format){0})
#else
#  define SB_APPEND_NUMBER_FN_F128_CASE
#  define SB_APPEND_NUMBER_FMT_F128_CASE
#endif
#define SB_APPEND_NUMBER_FN_FIXED_FLOATS_CASE SB_APPEND_NUMBER_FN_F16_CASE SB_APPEND_NUMBER_FN_F32_CASE SB_APPEND_NUMBER_FN_F64_CASE SB_APPEND_NUMBER_FN_F128_CASE
#define SB_APPEND_NUMBER_FMT_FIXED_FLOATS_CASE SB_APPEND_NUMBER_FMT_F16_CASE SB_APPEND_NUMBER_FMT_F32_CASE SB_APPEND_NUMBER_FMT_F64_CASE SB_APPEND_NUMBER_FMT_F128_CASE

#if FLOATS_LD_KIND == FLOATS_LD_KIND_F64
#  define SB_APPEND_NUMBER_FN_LDOUBLE_CASE long double : sb_append_floating_f64_fmt
#  define SB_APPEND_NUMBER_FMT_LDOUBLE_CASE long double : ((Sb_Floating_Format){0})
#elif FLOATS_LD_KIND == FLOATS_LD_KIND_F80
#  define SB_APPEND_NUMBER_FN_LDOUBLE_CASE \
  f80_canonical_t:                         \
    sb_append_floating_f80_canonical_fmt
#  define SB_APPEND_NUMBER_FMT_LDOUBLE_CASE \
  f80_canonical_t:                          \
    ((Sb_Floating_Format){0})
#elif FLOATS_LD_KIND == FLOATS_LD_KIND_F128
#  define SB_APPEND_NUMBER_FN_LDOUBLE_CASE \
  f128_canonical_t:                        \
    sb_append_floating_f128_canonical_fmt
#  define SB_APPEND_NUMBER_FMT_LDOUBLE_CASE \
  f128_canonical_t:                         \
    ((Sb_Floating_Format){__VA_ARGS__})
#elif FLOATS_LD_KIND == FLOATS_LD_KIND_F64PAIR
#  define SB_APPEND_NUMBER_FN_LDOUBLE_CASE \
  f64pair_canonical_t:                     \
    sb_append_floating_f64pair_canonical_fmt
#  define SB_APPEND_NUMBER_FMT_LDOUBLE_CASE \
  f64pair_canonical_t:                      \
    ((Sb_Floating_Format){__VA_ARGS__})
#else
#  error "Unsupported or unknown long double architecture."
#endif

// clang-format off
#define sb_append_signed_integer_fmt(sb, value, fmt) _Generic((value), \
  signed char: sb_append_integer_i8_fmt,                               \
  SB_APPEND_NUMBER_FN_SSHORT_CASE,                                     \
  SB_APPEND_NUMBER_FN_SINT_CASE,                                       \
  SB_APPEND_NUMBER_FN_SLONG_CASE,                                      \
  SB_APPEND_NUMBER_FN_SLLONG_CASE                                      \
  SB_APPEND_NUMBER_FN_SCHAR_CASE                                       \
  SB_APPEND_NUMBER_FN_SI128_CASE)((sb), (value), (fmt))
#define sb_append_signed_integer(sb, value, ...) sb_append_signed_integer_fmt(sb, value, ((Sb_Integer_Format){__VA_ARGS__}))

#define sb_append_unsigned_integer_fmt(sb, value, fmt) _Generic((value), \
  unsigned char: sb_append_integer_u8_fmt,                               \
  SB_APPEND_NUMBER_FN_USHORT_CASE,                                       \
  SB_APPEND_NUMBER_FN_UINT_CASE,                                         \
  SB_APPEND_NUMBER_FN_ULONG_CASE,                                        \
  SB_APPEND_NUMBER_FN_ULLONG_CASE                                        \
  SB_APPEND_NUMBER_FN_UCHAR_CASE                                         \
  SB_APPEND_NUMBER_FN_UI128_CASE)((sb), (value), (fmt))
#define sb_append_unsigned_integer(sb, value, ...) sb_append_unsigned_integer_fmt(sb, value, ((Sb_Integer_Format){ __VA_ARGS__ }))

#define sb_append_integer_fmt(sb, value, fmt) _Generic((value), \
  SB_APPEND_NUMBER_FN_CHAR_CASE,                                \
  signed char: sb_append_integer_i8_fmt,                        \
  unsigned char: sb_append_integer_u8_fmt,                      \
  SB_APPEND_NUMBER_FN_SHORT_CASE,                               \
  SB_APPEND_NUMBER_FN_INT_CASE,                                 \
  SB_APPEND_NUMBER_FN_LONG_CASE,                                \
  SB_APPEND_NUMBER_FN_LLONG_CASE                                \
  SB_APPEND_NUMBER_FN_I128_CASE)((sb), (value), (fmt))
#define sb_append_integer(sb, value, ...) sb_append_integer_fmt(sb, value, ((Sb_Integer_Format){__VA_ARGS__}))

#define sb_append_floating_fmt(sb, value, fmt) _Generic((value), \
  float: sb_append_floating_f32_fmt,                             \
  double: sb_append_floating_f64_fmt,                            \
  SB_APPEND_NUMBER_FN_LDOUBLE_CASE,                              \
  f80_t: sb_append_floating_f80_fmt,                             \
  f128_t: sb_append_floating_f128_fmt,                           \
  f64pair_t: sb_append_floating_f64pair_fmt                      \
  SB_APPEND_NUMBER_FN_FIXED_FLOATS_CASE)((sb), (value), (fmt))
#define sb_append_floating(sb, value, ...) sb_append_floating_fmt(sb, value, ((Sb_Floating_Format){__VA_ARGS__}))

#define sb_append_number_fmt(sb, value, fmt) _Generic((value), \
  SB_APPEND_NUMBER_FN_CHAR_CASE,                               \
  signed char: sb_append_integer_i8_fmt,                       \
  unsigned char: sb_append_integer_u8_fmt,                     \
  SB_APPEND_NUMBER_FN_SHORT_CASE,                              \
  SB_APPEND_NUMBER_FN_INT_CASE,                                \
  SB_APPEND_NUMBER_FN_LONG_CASE,                               \
  SB_APPEND_NUMBER_FN_LLONG_CASE,                              \
  float: sb_append_floating_f32_fmt,                           \
  double: sb_append_floating_f64_fmt,                          \
  SB_APPEND_NUMBER_FN_LDOUBLE_CASE,                            \
  f80_t: sb_append_floating_f80_fmt,                           \
  f128_t: sb_append_floating_f128_fmt,                         \
  f64pair_t: sb_append_floating_f64pair_fmt                    \
  SB_APPEND_NUMBER_FN_I128_CASE                                \
  SB_APPEND_NUMBER_FN_FIXED_FLOATS_CASE)((sb), (value), (fmt))
#define sb_append_number(sb, value) sb_append_number_fmt(sb, value, _Generic((value), \
    char: ((Sb_Integer_Format){0}),                \
    signed char: ((Sb_Integer_Format){0}),         \
    unsigned char: ((Sb_Integer_Format){0}),       \
    signed short: ((Sb_Integer_Format){0}),        \
    unsigned short: ((Sb_Integer_Format){0}),      \
    signed int: ((Sb_Integer_Format){0}),          \
    unsigned int: ((Sb_Integer_Format){0}),        \
    signed long: ((Sb_Integer_Format){0}),         \
    unsigned long: ((Sb_Integer_Format){0}),       \
    signed long long: ((Sb_Integer_Format){0}),    \
    unsigned long long: ((Sb_Integer_Format){0}),  \
    float: ((Sb_Floating_Format){0}),              \
    double: ((Sb_Floating_Format){0}),             \
    SB_APPEND_NUMBER_FMT_LDOUBLE_CASE,             \
    f80_t: ((Sb_Floating_Format){0}),              \
    f128_t: ((Sb_Floating_Format){0}),             \
    f64pair_t: ((Sb_Floating_Format){0})           \
    SB_APPEND_NUMBER_FMT_FIXED_FLOATS_CASE         \
    SB_APPEND_NUMBER_FMT_I128_CASE                 \
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
#define FLOATS_IMPL
#define FLOATS_IGNORE_F16_WARNINGS
#include <floats.h>

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
size_t sb_append_integer_i8_fmt(String_Builder *sb, int8_t value, Sb_Integer_Format fmt) { return sb___append_signed_integer(sb, value, fmt, INT8_MIN, 8, 3); }
size_t sb_append_integer_i16_fmt(String_Builder *sb, int16_t value, Sb_Integer_Format fmt) { return sb___append_signed_integer(sb, value, fmt, INT16_MIN, 16, 5); }
size_t sb_append_integer_i32_fmt(String_Builder *sb, int32_t value, Sb_Integer_Format fmt) { return sb___append_signed_integer(sb, value, fmt, INT32_MIN, 32, 10); }
size_t sb_append_integer_i64_fmt(String_Builder *sb, int64_t value, Sb_Integer_Format fmt) { return sb___append_signed_integer(sb, value, fmt, INT64_MIN, 64, 19); }
size_t sb_append_integer_i128_fmt(String_Builder *sb, __int128_t value, Sb_Integer_Format fmt) { return sb___append_signed_integer(sb, value, fmt, INT128_MIN, 128, 39); }
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
size_t sb_append_integer_u8_fmt(String_Builder *sb, uint8_t value, Sb_Integer_Format fmt) { return sb___append_unsigned_integer(sb, value, fmt, 8, 3); }
size_t sb_append_integer_u16_fmt(String_Builder *sb, uint16_t value, Sb_Integer_Format fmt) { return sb___append_unsigned_integer(sb, value, fmt, 16, 5); }
size_t sb_append_integer_u32_fmt(String_Builder *sb, uint32_t value, Sb_Integer_Format fmt) { return sb___append_unsigned_integer(sb, value, fmt, 32, 10); }
size_t sb_append_integer_u64_fmt(String_Builder *sb, uint64_t value, Sb_Integer_Format fmt) { return sb___append_unsigned_integer(sb, value, fmt, 64, 19); }
size_t sb_append_integer_u128_fmt(String_Builder *sb, __uint128_t value, Sb_Integer_Format fmt) { return sb___append_unsigned_integer(sb, value, fmt, 128, 39); }
#undef sb___append_unsigned_integer

#undef sb___unsigned_integer_max_width
#undef sb___stringify_unsigned_integer
#undef sb___format_padding_unsigned_integer

#define sb___mantissa_type(binary_width) typeof((struct floating_decimal_f##binary_width){0}.mantissa)

void sb___floating_format_width(Stringify_State *state, Sb_Floating_Format *fmt, size_t number_start) {
  size_t width = state->count - number_start;
  if (!fmt->min_width || fmt->min_width <= width) return;
  size_t unfilled = fmt->min_width - width;
  if (!state->sb) return (void)(state->count += unfilled);
  stringify_append(state, sb_append_repeat, ' ', unfilled);
  char *start = state->sb->data + state->sb->count - fmt->min_width;
  memmove(start + unfilled, start, width);
  memset(start, ' ', unfilled);
}

#define sb___floating_detect_special(state, ieee, fmt, type, number_start) ({ \
  if (ryu_##type##_is_nan(ieee)) {                                            \
    stringify_append(&state, sb_append_strlit, "NaN");                        \
    sb___floating_format_width(&state, &fmt, number_start);                   \
    return state.count;                                                       \
  }                                                                           \
  if (ryu_##type##_is_infinity(ieee)) {                                       \
    if (ieee.sign) stringify_append(&state, sb_append, '-');                  \
    stringify_append(&state, sb_append_strlit, "Infinity");                   \
    sb___floating_format_width(&state, &fmt, number_start);                   \
    return state.count;                                                       \
  }                                                                           \
  if (ryu_##type##_is_zero(ieee)) {                                           \
    if (ieee.sign) stringify_append(&state, sb_append, '-');                  \
    stringify_append(&state, sb_append_strlit, "0.0");                        \
    sb___floating_format_width(&state, &fmt, number_start);                   \
    return state.count;                                                       \
  }                                                                           \
  if (ieee.sign) stringify_append(&state, sb_append, '-');                    \
  (void)0;                                                                    \
})

#define sb___ryu_floating_to_decimal_chars(state, ieee, type) ({                                  \
  floating_decimal_##type decimal = ryu_##type##_parse(ieee);                                     \
  if (!decimal.exponent) {                                                                        \
    stringify_append(&state, sb_append_unsigned_integer, decimal.mantissa);                       \
    stringify_append(&state, sb_append_strlit, ".0");                                             \
  } else if (decimal.exponent < 0) {                                                              \
    size_t exponent = -decimal.exponent;                                                          \
    size_t count = ryu_decimalLength_##type(decimal.mantissa);                                    \
    if (exponent >= count) stringify_append(&state, sb_append_repeat, '0', exponent - count + 1); \
    stringify_append(&state, sb_append_unsigned_integer, decimal.mantissa);                       \
    stringify_append(&state, sb_append, '.');                                                     \
    if (state.sb) {                                                                               \
      char *start = state.sb->data + state.sb->count - exponent - 1;                              \
      memmove(start + 1, start, exponent);                                                        \
      *start = '.';                                                                               \
    }                                                                                             \
  } else {                                                                                        \
    stringify_append(&state, sb_append_unsigned_integer, decimal.mantissa);                       \
    stringify_append(&state, sb_append_repeat, '0', decimal.exponent);                            \
    stringify_append(&state, sb_append_strlit, ".0");                                             \
  }                                                                                               \
  (void)0;                                                                                        \
})

#define sb___ryu_floating_to_hex_chars(state, ieee, binary_width, fmt_kind, fmt_min_width) ({                             \
  stringify_append(&state, sb_append_strlit, "0x");                                                                       \
  stringify_append(&state, sb_append, (ieee.exponent ? '1' : '0'));                                                       \
  stringify_append(&state, sb_append, '.');                                                                               \
  size_t mantissa_width = RYU_F##binary_width##_MANTISSA_BITS / 4 + (RYU_F##binary_width##_MANTISSA_BITS % 4 ? 1 : 0);    \
  sb___mantissa_type(binary_width) mantissa = ieee.mantissa;                                                              \
  if (RYU_F##binary_width##_MANTISSA_BITS % 4 != 0) mantissa = mantissa << (4 - RYU_F##binary_width##_MANTISSA_BITS % 4); \
  Sb_Integer_Format mantissa_fmt = {.kind = SB_INTEGER_FORMAT_KIND_##fmt_kind, .min_width = mantissa_width};              \
  stringify_append(&state, sb_append_unsigned_integer_fmt, mantissa, mantissa_fmt);                                       \
  stringify_append(&state, sb_append, 'p');                                                                               \
  int64_t exponent = (int64_t)ieee.exponent - (int64_t)(((1u << RYU_F##binary_width##_EXPONENT_BITS) - 1u) >> 1);         \
  if (exponent > 0) stringify_append(&state, sb_append, '+');                                                             \
  stringify_append(&state, sb_append_signed_integer, exponent);                                                           \
  (void)0;                                                                                                                \
})

#define sb___append_ryu_floating(sb, value, fmt, binary_width, max_width) ({                                                        \
  Stringify_State state = make_stringify_state(sb, max_width);                                                                      \
  struct floating_ieee_f##binary_width ieee = ryu_f##binary_width##_to_ieee(value);                                                 \
  size_t number_start = state.count;                                                                                                \
  sb___floating_detect_special(state, ieee, fmt, f##binary_width, number_start);                                                    \
  switch (fmt.kind) {                                                                                                               \
    case SB_FLOATING_FORMAT_KIND_DECIMAL: sb___ryu_floating_to_decimal_chars(state, ieee, f##binary_width); break;                  \
    case SB_FLOATING_FORMAT_KIND_FIXED: TODO("TASK(20260913-080043)"); break;                                                       \
    case SB_FLOATING_FORMAT_KIND_HEX: sb___ryu_floating_to_hex_chars(state, ieee, binary_width, HEX, fmt.min_width); break;         \
    case SB_FLOATING_FORMAT_KIND_HEX_BIG: sb___ryu_floating_to_hex_chars(state, ieee, binary_width, HEX_BIG, fmt.min_width); break; \
  }                                                                                                                                 \
  sb___floating_format_width(&state, &fmt, number_start);                                                                           \
  state.count;                                                                                                                      \
})

#define sb___append_ryu_generic_floating(sb, value, fmt, type, max_width) ({                               \
  Stringify_State state = make_stringify_state(sb, max_width);                                             \
  struct floating_ieee_generic ieee = ryu_##type##_to_ieee(value);                                         \
  size_t number_start = state.count;                                                                       \
  sb___floating_detect_special(state, ieee, fmt, generic, number_start);                                   \
  switch (fmt.kind) {                                                                                      \
    case SB_FLOATING_FORMAT_KIND_DECIMAL: sb___ryu_floating_to_decimal_chars(state, ieee, generic); break; \
    case SB_FLOATING_FORMAT_KIND_FIXED: TODO("TASK(20260913-080043)"); break;                              \
    case SB_FLOATING_FORMAT_KIND_HEX: TODO("TASK(20260913-080106)"); break;                                \
    case SB_FLOATING_FORMAT_KIND_HEX_BIG: TODO("TASK(20260913-080106)"); break;                            \
  }                                                                                                        \
  sb___floating_format_width(&state, &fmt, number_start);                                                  \
  state.count;                                                                                             \
})

size_t sb_append_floating_f16_fmt(String_Builder *sb, f16_t value, Sb_Floating_Format fmt) { return sb___append_ryu_generic_floating(sb, value, fmt, f16, 8); }
size_t sb_append_floating_f32_fmt(String_Builder *sb, float value, Sb_Floating_Format fmt) { return sb___append_ryu_floating(sb, value, fmt, 32, 16); }
size_t sb_append_floating_f64_fmt(String_Builder *sb, double value, Sb_Floating_Format fmt) { return sb___append_ryu_floating(sb, value, fmt, 64, 27); }
size_t sb_append_floating_f80_fmt(String_Builder *sb, f80_t value, Sb_Floating_Format fmt) { return sb___append_ryu_generic_floating(sb, value, fmt, f80, 36); }
size_t sb_append_floating_f128_fmt(String_Builder *sb, f128_t value, Sb_Floating_Format fmt) { return sb___append_ryu_generic_floating(sb, value, fmt, f128, 36); }
size_t sb_append_floating_f64pair_fmt(String_Builder *sb, f64pair_t value, Sb_Floating_Format fmt) { return sb___append_ryu_generic_floating(sb, value, fmt, f64pair, 36); }

size_t sb_append_floating_f16_canonical_fmt(String_Builder *sb, f16_canonical_t value, Sb_Floating_Format fmt) {
#if defined(__FLT16_MAX__)
  _Float16 native = 0;
  memcpy(&native, &value, sizeof(native));
  return sb_append_floating_f16_fmt(sb, native, fmt);
#else
  return sb_append_floating_f16_fmt(sb, native, fmt);
#endif
}

size_t sb_append_floating_f80_canonical_fmt(String_Builder *sb, f80_canonical_t value, Sb_Floating_Format fmt) {
#if FLOATS_LD_KIND == FLOATS_LD_KIND_F80
  long double native = 0;
  memcpy(&native, &value, sizeof(native));
  return sb_append_floating_f80_fmt(sb, native, fmt);
#else
  return sb_append_floating_f80_fmt(sb, value, fmt);
#endif
}

size_t sb_append_floating_f128_canonical_fmt(String_Builder *sb, f128_canonical_t value, Sb_Floating_Format fmt) {
#if FLOATS_LD_KIND == FLOATS_LD_KIND_F128
  long double native = 0;
  memcpy(&native, &value, sizeof(native));
  return sb_append_floating_f128_fmt(sb, native, fmt);
#else
  return sb_append_floating_f128_fmt(sb, value, fmt);
#endif
}

size_t sb_append_floating_f64pair_canonical_fmt(String_Builder *sb, f64pair_canonical_t value, Sb_Floating_Format fmt) {
#if FLOATS_LD_KIND == FLOATS_LD_KIND_F64PAIR
  long double native = 0;
  memcpy(&native, &value, sizeof(native));
  return sb_append_floating_f64pair_fmt(sb, native, fmt);
#else
  return sb_append_floating_f64pair_fmt(sb, value, fmt);
#endif
}

#undef sb___mantissa_type
#undef sb___ryu_floating_to_bits
#undef sb___floating_detect_special
#undef sb___ryu_floating_to_decimal_chars
#undef sb___ryu_floating_to_hex_chars
#undef sb___append_ryu_floating

#endif // SB_NUMBER_IMPL_C
