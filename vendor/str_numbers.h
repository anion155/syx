/**
 * str_floats.h - 0.3.0 - Public Domain - https://github.com/anion155/c-tools
 *
 * Utilities to format floating point numbers to strings for c23.
 *
 * ## Usage example
 * ```c
  #define STR_NUMBERS_IMPL
  #include "str_numbers.h"

  int main(void) {
    String_Builder sb = {0};
    sb_append_number(&sb, (long)120349);
    sb_append_number_fmt(&sb, (long long)120349, ((Sb_Integer_Format){.kind = SB_INTEGER_FORMAT_KIND_HEX}));
    sb_append_number_fmt(&sb, (char)126, ((Sb_Integer_Format){.kind = SB_INTEGER_FORMAT_KIND_BINARY}));
    sb_append_number(&sb, (long)120349);
    sb_append_number_fmt(&sb, (long long)120349, ((Sb_Floating_Format){.kind = SB_FLOATING_FORMAT_KIND_HEX}));
    sb_append_number_fmt(&sb, (char)126, ((Sb_Floating_Format){.kind = SB_FLOATING_FORMAT_KIND_FIXED, .precision = 2}));
  }
 * ```
 *
 * ## Requirements
 *
 * - C23
 * - GNU statement expressions
 * - [str.h](./str.h)
 * - [str_floats.h](./str_floats.h)
 * - [str_integers.h](./str_integers.h)
 */

#ifndef STR_NUMBERS_H
#define STR_NUMBERS_H

#include <str.h>
#include <str_floats.h>
#include <str_integers.h>

// clang-format off
#define sb_append_number_fmt(sb, value, fmt) _Generic((value), \
  SB_APPEND_NUMBER_FN_CHAR_CASE,             \
  signed char: sb_append_i8_number_fmt,      \
  unsigned char: sb_append_u8_number_fmt,    \
  SB_APPEND_NUMBER_FN_SHORT_CASE,            \
  SB_APPEND_NUMBER_FN_INT_CASE,              \
  SB_APPEND_NUMBER_FN_LONG_CASE,             \
  SB_APPEND_NUMBER_FN_LLONG_CASE,            \
  float: sb_append_f32_number_fmt,         \
  double: sb_append_f64_number_fmt,        \
  SB_APPEND_NUMBER_FN_LDOUBLE_CASE,          \
  f80_t: sb_append_f80_number_fmt,         \
  f128_t: sb_append_f128_number_fmt,       \
  f64pair_t: sb_append_f64pair_number_fmt  \
  SB_APPEND_NUMBER_FN_I128_CASE              \
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

#endif // STR_NUMBERS_H

#if defined(STR_NUMBERS_IMPL) && !defined(STR_NUMBERS_IMPL_C)
#define STR_NUMBERS_IMPL_C

#define STR_INTEGERS_IMPL
#include <str_integers.h>
#define STR_FLOATS_IMPL
#include <str_floats.h>

#endif // STR_NUMBERS_IMPL_C

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
