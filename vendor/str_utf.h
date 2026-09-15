/**
 * str_utf.h - 0.1.0 - Public Domain - https://github.com/anion155/c-tools
 *
 * UTF string utilities for c23.
 *
 * ## Usage example
 * ```c
  #define STR_UTF_IMPL
  #include "str_utf.h"

  int main(void) {
    ...
    String_View it = source;
    while (it.count) {
      size_t bytes = sv_first_utf_length(it);
      printf("character[%zu]: "SV_FMT"\n", bytes, (int)bytes, it.data);
      sv_chop_left(&it, bytes);
    }
  }
 * ```
 *
 * ## Requirements
 *
 * - C23
 * - GNU statement expressions
 * - [str.h](./str.h)
 * - [defines.h](./defines.h)
 */

#ifndef STR_UTF_H
#define STR_UTF_H

#include <stddef.h>
#include <stdint.h>
#include <str.h>

/** Array that maps every `char` value to utf8 bytes length. */
extern const uint8_t utf8_character_lengths[0x100];

/** Returns bytes length of first character in a `String_View`. */
#define sv_first_utf_length(sv) (utf8_character_lengths[(uint8_t)da_first((sv))])

/** Chop 1 utf character from left. */
#define sv_chop_left_utf(sv) ({                   \
  String_View *_sv_ = (sv);                       \
  sv_chop_left(_sv_, sv_first_utf_length(*_sv_)); \
})

size_t sv__utf_length(String_View sv, size_t *bytes_overrun);
/** Calculate `String_View`'s length in characters. */
#define sv_utf_length(sv, ...) sv__utf_length(sv_from_like(sv), WITH_DEFAULT(NULL, __VA_ARGS__))

#endif // STR_UTF_H

#if defined(STR_UTF_IMPL) && !defined(STR_UTF_IMPL_C)
#define STR_UTF_IMPL_C

#define STR_IMPL
#include <str.h>

// clang-format off
const uint8_t utf8_character_lengths[] = {
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2, 2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
  3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3, 4,4,4,4,4,4,4,4,5,5,5,5,6,6,6,6,
};
// clang-format on

size_t sv__utf_length(String_View sv, size_t *bytes_overrun) {
  size_t count = 0;
  size_t bytes;
  while (sv.count) {
    bytes = sv_first_utf_length(sv);
    if (bytes_overrun && sv.count <= bytes) *bytes_overrun = bytes - sv.count;
    sv_chop_left(&sv, bytes);
    count += 1;
  }
  return count;
}

#endif // STR_UTF_IMPL_C

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
