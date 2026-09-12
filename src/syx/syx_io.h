#ifndef SYX_IO_H
#define SYX_IO_H

#include <sb.h>
#include <stddef.h>
#include <stdio.h>

size_t syx_io_putc(FILE *fd, char char_v);
void syx_io_flash(FILE *fd);
size_t syx_io_puts_n(FILE *fd, const char *str, size_t n);
#define syx_io_puts_sv(fd, sv) ({             \
  String_View _sv_ = sv_from_like(sv);        \
  syx_io_puts_n((fd), _sv_.data, _sv_.count); \
})
#define syx_io_puts_cstr(fd, cstr) ({          \
  const char *_cstr_ = (cstr);                 \
  syx_io_puts_n((fd), _cstr_, strlen(_cstr_)); \
})
#define syx_io_puts_strlit(fd, lit) syx_io_puts_n((fd), (lit), sizeof(lit) - 1)

#endif // SYX_IO_H

#if defined(SYX_IO_IMPL) && !defined(SYX_IO_IMPL_C)
#define SYX_IO_IMPL_C

#define SB_IMPL
#include <sb.h>

size_t syx_io_putc(FILE *fd, char c) {
  if (fputc(c, fd) < 0) return 0;
  if (c == '\n') syx_io_flash(fd);
  return 1;
}

void syx_io_flash(FILE *fd) {
  fflush(fd);
}

size_t syx_io_puts_n(FILE *fd, const char *str, size_t n) {
  for (size_t index = 0; index < n; index += 1) {
    if (!syx_io_putc(fd, str[index])) return index;
  }
  return n;
}

#endif // SYX_IO_IMPL
