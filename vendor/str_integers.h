/**
 * str_integers.h - 0.2.0 - Public Domain - https://github.com/anion155/c-tools
 *
 * Utilities to format integer numbers to strings for c23.
 *
 * ## Usage example
 * ```c
  #define STR_INTEGERS_IMPL
  #include "str_integers.h"

  int main(void) {
    String_Builder sb = {0};
    sb_append_integer_number(&sb, (long)120349);
    sb_append_integer_number(&sb, (long long)120349, .kind = SB_INTEGER_FORMAT_KIND_HEX);
    sb_append_integer_number(&sb, (char)126, .kind = SB_INTEGER_FORMAT_KIND_BINARY);
  }
 * ```
 *
 * ## Requirements
 *
 * - C23
 * - GNU statement expressions
 * - [str.h](./str.h)
 * - [defines.h](./defines.h)
 * - [abort.h](./abort.h)
 */

#ifndef STR_INTEGERS_H
#define STR_INTEGERS_H

#include <defines.h>
#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <str.h>

#ifndef __INT128_MAX__
#  define __INT128_MAX__ ((__int128_t) ~(((__uint128_t)1) << 127))
#endif
#ifndef __INT128_MIN__
#  define __INT128_MIN__ ((__int128_t)1 << 127)
#endif
#ifndef __UINT128_MAX__
#  define __UINT128_MAX__ (~((__uint128_t)0))
#endif

/** Integer format kinds. */
typedef enum Sb_Integer_Format_Kind {
  SB_INTEGER_FORMAT_KIND_DECIMAL = 0, /** regular decimal format */
  SB_INTEGER_FORMAT_KIND_BINARY,      /** binary number format, provides default prefix `0b` */
  SB_INTEGER_FORMAT_KIND_OCTAL,       /** octal number format, provides default prefix `0o` */
  SB_INTEGER_FORMAT_KIND_HEX,         /** hex number format, using a-f alphadigits, provides default prefix `0x` */
  SB_INTEGER_FORMAT_KIND_HEX_BIG,     /** hex number format, using A-F alphadigits, provides default prefix `0x` */
} Sb_Integer_Format_Kind;

/** Represents how integer must be formated in the string. */
typedef struct Sb_Integer_Format {
  Sb_Integer_Format_Kind kind;
  bool hide_prefix;   /** hides prefix */
  String_View prefix; /** custom prefix */
  size_t min_width;   /** minimal width of whole resulting string, if too small will padd with spaces at string begining  */
} Sb_Integer_Format;

/** Signed formatter functions. */
size_t sb_append_i8_number_fmt(String_Builder *sb, int8_t value, Sb_Integer_Format fmt);
size_t sb_append_i16_number_fmt(String_Builder *sb, int16_t value, Sb_Integer_Format fmt);
size_t sb_append_i32_number_fmt(String_Builder *sb, int32_t value, Sb_Integer_Format fmt);
size_t sb_append_i64_number_fmt(String_Builder *sb, int64_t value, Sb_Integer_Format fmt);
size_t sb_append_i128_number_fmt(String_Builder *sb, __int128_t value, Sb_Integer_Format fmt);
/** Signed formatter functions with format options accepted as __VA_ARGS__. */
#define sb_append_i8_number(sb, value, ...) sb_append_i8_number_fmt((sb), (value), ((Sb_Integer_Format){__VA_ARGS__}))
#define sb_append_i16_number(sb, value, ...) sb_append_i16_number_fmt((sb), (value), ((Sb_Integer_Format){__VA_ARGS__}))
#define sb_append_i32_number(sb, value, ...) sb_append_i32_number_fmt((sb), (value), ((Sb_Integer_Format){__VA_ARGS__}))
#define sb_append_i64_number(sb, value, ...) sb_append_i64_number_fmt((sb), (value), ((Sb_Integer_Format){__VA_ARGS__}))
#define sb_append_i128_number(sb, value, ...) sb_append_i128_number_fmt((sb), (value), ((Sb_Integer_Format){__VA_ARGS__}))

/** Unsigned formatter functions. */
size_t sb_append_u8_number_fmt(String_Builder *sb, uint8_t value, Sb_Integer_Format fmt);
size_t sb_append_u16_number_fmt(String_Builder *sb, uint16_t value, Sb_Integer_Format fmt);
size_t sb_append_u32_number_fmt(String_Builder *sb, uint32_t value, Sb_Integer_Format fmt);
size_t sb_append_u64_number_fmt(String_Builder *sb, uint64_t value, Sb_Integer_Format fmt);
size_t sb_append_u128_number_fmt(String_Builder *sb, __uint128_t value, Sb_Integer_Format fmt);
/** Unsigned formatter functions with format options accepted as __VA_ARGS__. */
#define sb_append_u8_number(sb, value, ...) sb_append_u8_number_fmt((sb), (value), ((Sb_Integer_Format){__VA_ARGS__}))
#define sb_append_u16_number(sb, value, ...) sb_append_u16_number_fmt((sb), (value), ((Sb_Integer_Format){__VA_ARGS__}))
#define sb_append_u32_number(sb, value, ...) sb_append_u32_number_fmt((sb), (value), ((Sb_Integer_Format){__VA_ARGS__}))
#define sb_append_u64_number(sb, value, ...) sb_append_u64_number_fmt((sb), (value), ((Sb_Integer_Format){__VA_ARGS__}))
#define sb_append_u128_number(sb, value, ...) sb_append_u128_number_fmt((sb), (value), ((Sb_Integer_Format){__VA_ARGS__}))

#if CHAR_MIN < 0
#  define SB_APPEND_NUMBER_FN_CHAR_CASE char : sb_append_i8_number_fmt
#  define SB_APPEND_NUMBER_FN_SCHAR_CASE , SB_APPEND_NUMBER_FN_CHAR_CASE
#  define SB_APPEND_NUMBER_FN_UCHAR_CASE
#else
#  define SB_APPEND_NUMBER_FN_CHAR_CASE char : sb_append_u8_number_fmt
#  define SB_APPEND_NUMBER_FN_SCHAR_CASE
#  define SB_APPEND_NUMBER_FN_UCHAR_CASE , SB_APPEND_NUMBER_FN_CHAR_CASE
#endif

#if SHRT_MAX == 32767
#  define SB_APPEND_NUMBER_FN_SSHORT_CASE signed short : sb_append_i16_number_fmt
#  define SB_APPEND_NUMBER_FN_USHORT_CASE unsigned short : sb_append_u16_number_fmt
#elif SHRT_MAX == 2147483647
#  define SB_APPEND_NUMBER_FN_SSHORT_CASE signed short : sb_append_i32_number_fmt
#  define SB_APPEND_NUMBER_FN_USHORT_CASE unsigned short : sb_append_u32_number_fmt
#else
#  error "short size not supported"
#endif
#define SB_APPEND_NUMBER_FN_SHORT_CASE SB_APPEND_NUMBER_FN_SSHORT_CASE, SB_APPEND_NUMBER_FN_USHORT_CASE

#if INT_MAX == 32767
#  define SB_APPEND_NUMBER_FN_SINT_CASE signed int : sb_append_i16_number_fmt
#  define SB_APPEND_NUMBER_FN_UINT_CASE unsigned int : sb_append_u16_number_fmt
#elif INT_MAX == 2147483647
#  define SB_APPEND_NUMBER_FN_SINT_CASE signed int : sb_append_i32_number_fmt
#  define SB_APPEND_NUMBER_FN_UINT_CASE unsigned int : sb_append_u32_number_fmt
#elif INT_MAX == 9223372036854775807
#  define SB_APPEND_NUMBER_FN_SINT_CASE signed int : sb_append_i64_number_fmt
#  define SB_APPEND_NUMBER_FN_UINT_CASE unsigned int : sb_append_u64_number_fmt
#else
#  error "int size not supported"
#endif
#define SB_APPEND_NUMBER_FN_INT_CASE SB_APPEND_NUMBER_FN_SINT_CASE, SB_APPEND_NUMBER_FN_UINT_CASE

#if LONG_MAX == 2147483647
#  define SB_APPEND_NUMBER_FN_SLONG_CASE signed long : sb_append_i32_number_fmt
#  define SB_APPEND_NUMBER_FN_ULONG_CASE unsigned long : sb_append_u32_number_fmt
#elif LONG_MAX == 9223372036854775807
#  define SB_APPEND_NUMBER_FN_SLONG_CASE signed long : sb_append_i64_number_fmt
#  define SB_APPEND_NUMBER_FN_ULONG_CASE unsigned long : sb_append_u64_number_fmt
#else
#  error "long size not supported"
#endif
#define SB_APPEND_NUMBER_FN_LONG_CASE SB_APPEND_NUMBER_FN_SLONG_CASE, SB_APPEND_NUMBER_FN_ULONG_CASE

#if LLONG_MAX == 9223372036854775807
#  define SB_APPEND_NUMBER_FN_SLLONG_CASE signed long long : sb_append_i64_number_fmt
#  define SB_APPEND_NUMBER_FN_ULLONG_CASE unsigned long long : sb_append_u64_number_fmt
#else
#  error "long size not supported"
#endif
#define SB_APPEND_NUMBER_FN_LLONG_CASE SB_APPEND_NUMBER_FN_SLLONG_CASE, SB_APPEND_NUMBER_FN_ULLONG_CASE

#if defined(__SIZEOF_INT128__)
#  define SB_APPEND_NUMBER_FN_SI128_CASE , __int128_t : sb_append_i128_number_fmt
#  define SB_APPEND_NUMBER_FN_UI128_CASE , __uint128_t : sb_append_u128_number_fmt
#  define SB_APPEND_NUMBER_FMT_SI128_CASE , __int128_t : ((Sb_Integer_Format){0})
#  define SB_APPEND_NUMBER_FMT_UI128_CASE , __uint128_t : ((Sb_Integer_Format){0})
#else
#  error "128bit integer type not supported"
#endif
#define SB_APPEND_NUMBER_FN_I128_CASE SB_APPEND_NUMBER_FN_SI128_CASE SB_APPEND_NUMBER_FN_UI128_CASE
#define SB_APPEND_NUMBER_FMT_I128_CASE SB_APPEND_NUMBER_FMT_SI128_CASE SB_APPEND_NUMBER_FMT_UI128_CASE

// clang-format off
/** Polymorphic signed integers formatter. */
#define sb_append_signed_integer_number_fmt(sb, value, fmt) _Generic((value), \
  signed char: sb_append_i8_number_fmt,    \
  SB_APPEND_NUMBER_FN_SSHORT_CASE,         \
  SB_APPEND_NUMBER_FN_SINT_CASE,           \
  SB_APPEND_NUMBER_FN_SLONG_CASE,          \
  SB_APPEND_NUMBER_FN_SLLONG_CASE          \
  SB_APPEND_NUMBER_FN_SCHAR_CASE           \
  SB_APPEND_NUMBER_FN_SI128_CASE)((sb), (value), (fmt))
/** Polymorphic signed integers formatter with format options accepted as `__VA_ARGS__`. */
#define sb_append_signed_integer_number(sb, value, ...) sb_append_signed_integer_number_fmt(sb, value, ((Sb_Integer_Format){__VA_ARGS__}))

/** Polymorphic unsigned integers formatter. */
#define sb_append_unsigned_integer_number_fmt(sb, value, fmt) _Generic((value), \
  unsigned char: sb_append_u8_number_fmt,  \
  SB_APPEND_NUMBER_FN_USHORT_CASE,         \
  SB_APPEND_NUMBER_FN_UINT_CASE,           \
  SB_APPEND_NUMBER_FN_ULONG_CASE,          \
  SB_APPEND_NUMBER_FN_ULLONG_CASE          \
  SB_APPEND_NUMBER_FN_UCHAR_CASE           \
  SB_APPEND_NUMBER_FN_UI128_CASE)((sb), (value), (fmt))
/** Polymorphic unsigned integers formatter with format options accepted as `__VA_ARGS__`. */
#define sb_append_unsigned_integer_number(sb, value, ...) sb_append_unsigned_integer_number_fmt(sb, value, ((Sb_Integer_Format){ __VA_ARGS__ }))

/** Polymorphic integers formatter. */
#define sb_append_integer_number_fmt(sb, value, fmt) _Generic((value), \
  SB_APPEND_NUMBER_FN_CHAR_CASE,           \
  signed char: sb_append_i8_number_fmt,    \
  unsigned char: sb_append_u8_number_fmt,  \
  SB_APPEND_NUMBER_FN_SHORT_CASE,          \
  SB_APPEND_NUMBER_FN_INT_CASE,            \
  SB_APPEND_NUMBER_FN_LONG_CASE,           \
  SB_APPEND_NUMBER_FN_LLONG_CASE           \
  SB_APPEND_NUMBER_FN_I128_CASE)((sb), (value), (fmt))
/** Polymorphic integers formatter with format options accepted as `__VA_ARGS__`. */
#define sb_append_integer_number(sb, value, ...) sb_append_integer_number_fmt(sb, value, ((Sb_Integer_Format){__VA_ARGS__}))
// clang-format on

#endif // STR_INTEGERS_H

#if defined(STR_INTEGERS_IMPL) && !defined(STR_INTEGERS_IMPL_C)
#define STR_INTEGERS_IMPL_C

#define ABORT_IMPL
#include <abort.h>
#define STR_IMPL
#include <str.h>

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
#define sb___append_integer_prefix(state, fmt) ({                                                       \
  if (!fmt.hide_prefix) {                                                                               \
    if (state.sb) {                                                                                     \
      if (fmt.prefix.data) {                                                                            \
        stringify_append(&state, sb_append_sv, fmt.prefix);                                             \
      } else {                                                                                          \
        switch (fmt.kind) {                                                                             \
          case SB_INTEGER_FORMAT_KIND_BINARY: stringify_append(&state, sb_append_strlit, "0b"); break;  \
          case SB_INTEGER_FORMAT_KIND_OCTAL: stringify_append(&state, sb_append_strlit, "0o"); break;   \
          case SB_INTEGER_FORMAT_KIND_DECIMAL: break;                                                   \
          case SB_INTEGER_FORMAT_KIND_HEX: stringify_append(&state, sb_append_strlit, "0x"); break;     \
          case SB_INTEGER_FORMAT_KIND_HEX_BIG: stringify_append(&state, sb_append_strlit, "0x"); break; \
        }                                                                                               \
      }                                                                                                 \
    } else {                                                                                            \
      if (fmt.prefix.data) {                                                                            \
        state.count += fmt.prefix.count;                                                                \
      } else if (fmt.kind != SB_INTEGER_FORMAT_KIND_DECIMAL) {                                          \
        state.count += 2;                                                                               \
      }                                                                                                 \
    }                                                                                                   \
  }                                                                                                     \
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
size_t sb_append_i8_number_fmt(String_Builder *sb, int8_t value, Sb_Integer_Format fmt) { return sb___append_signed_integer(sb, value, fmt, INT8_MIN, 8, 3); }
size_t sb_append_i16_number_fmt(String_Builder *sb, int16_t value, Sb_Integer_Format fmt) { return sb___append_signed_integer(sb, value, fmt, INT16_MIN, 16, 5); }
size_t sb_append_i32_number_fmt(String_Builder *sb, int32_t value, Sb_Integer_Format fmt) { return sb___append_signed_integer(sb, value, fmt, INT32_MIN, 32, 10); }
size_t sb_append_i64_number_fmt(String_Builder *sb, int64_t value, Sb_Integer_Format fmt) { return sb___append_signed_integer(sb, value, fmt, INT64_MIN, 64, 19); }
size_t sb_append_i128_number_fmt(String_Builder *sb, __int128_t value, Sb_Integer_Format fmt) { return sb___append_signed_integer(sb, value, fmt, __INT128_MIN__, 128, 39); }
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
size_t sb_append_u8_number_fmt(String_Builder *sb, uint8_t value, Sb_Integer_Format fmt) { return sb___append_unsigned_integer(sb, value, fmt, 8, 3); }
size_t sb_append_u16_number_fmt(String_Builder *sb, uint16_t value, Sb_Integer_Format fmt) { return sb___append_unsigned_integer(sb, value, fmt, 16, 5); }
size_t sb_append_u32_number_fmt(String_Builder *sb, uint32_t value, Sb_Integer_Format fmt) { return sb___append_unsigned_integer(sb, value, fmt, 32, 10); }
size_t sb_append_u64_number_fmt(String_Builder *sb, uint64_t value, Sb_Integer_Format fmt) { return sb___append_unsigned_integer(sb, value, fmt, 64, 19); }
size_t sb_append_u128_number_fmt(String_Builder *sb, __uint128_t value, Sb_Integer_Format fmt) { return sb___append_unsigned_integer(sb, value, fmt, 128, 39); }
#undef sb___append_unsigned_integer
#undef sb___unsigned_integer_max_width
#undef sb___stringify_unsigned_integer
#undef sb___format_padding_unsigned_integer

#endif // STR_INTEGERS_IMPL_C

/**
 * This is free and unencumbered software released into the public domain.
 *
 * Anyone is free to copy, modify, publish, use, compile, sell, or
 * distribute this software, either in source code form or as a compiled
 * binary, for any purpose, commercial or non-commercial, and by any
 * means.
 *
 * In jurisdictions that recognize copyright laws, the author or authors
 * of this software dedicate any and all copyright interest in the
 * software to the public domain. We make this dedication for the benefit
 * of the public at large and to the detriment of our heirs and
 * successors. We intend this dedication to be an overt act of
 * relinquishment in perpetuity of all present and future rights to this
 * software under copyright law.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * For more information, please refer to <https://unlicense.org/>
 */
