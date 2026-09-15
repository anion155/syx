#ifndef STR_UTF_H
#define STR_UTF_H

#include <stddef.h>
#include <stdint.h>
#include <str.h>

extern const uint8_t utf8_character_lengths[0x100];

#define sv_first_utf_length(sv) (utf8_character_lengths[(uint8_t)da_first((sv))])

size_t sv__utf_length(String_View sv, size_t *bytes_overrun);
#define sv_utf_length(sv, ...) sv__utf_length(sv_from_like(sv), WITH_DEFAULT(NULL, __VA_ARGS__))

#endif // STR_UTF_H

#if defined(STR_UTF_IMPL) && !defined(STR_UTF_IMPL_C)
#define STR_UTF_IMPL_C

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
