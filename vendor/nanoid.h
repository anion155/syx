#ifndef NANOID_H
#define NANOID_H

#include <stdio.h>

const char *nanoid_alph(Nob_String_View alphabet, const char *prefix, size_t size);
const char *nanoid(const char *prefix, size_t size);

#endif // NANOID_H

#if defined(NANOID_IMPL) && !defined(NANOID_IMPL_C)
#define NANOID_IMPL_C

#include <stdlib.h>
#include <magic.h>
#include <nob.h>

const char *nanoid_alph(Nob_String_View alphabet, const char *prefix, size_t size) {
  size_t prefix_size = prefix ? strlen(prefix) : 0;
  char *buf = malloc((prefix_size + size + 1) * sizeof(char));
  buf[prefix_size + size] = 0;
  if (prefix) memcpy(buf, prefix, prefix_size);
  for (size_t index = 0; index < size; index += 1) {
    buf[prefix_size + index] = alphabet.data[rand() % alphabet.count];
  }
  return buf;
}
Nob_String_View NANOID_ALPHABET = {0};
const char *nanoid(const char *prefix, size_t size) {
  if (!NANOID_ALPHABET.data) NANOID_ALPHABET = sv_from_cstr("useandom-26T198340PX75pxJACKVERYMINDBUSHWOLF_GQZbfghjklqvwyzrict");
  return nanoid_alph(NANOID_ALPHABET, prefix, size);
}

#endif // NANOID_IMPL
