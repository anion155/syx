#ifndef SYX_IO_H
#define SYX_IO_H

#include <stddef.h>
#include <stdio.h>
#include <str.h>
#include <str_utf.h>

size_t syx_io_putc(FILE *fd, char char_v);
void syx_io_flash(FILE *fd);
ssize_t syx_io_puts_n(FILE *fd, const char *const str, size_t n);
#define syx_io_puts_sv(fd, sv) ({             \
  String_View _sv_ = sv_from_like(sv);        \
  syx_io_puts_n((fd), _sv_.data, _sv_.count); \
})
#define syx_io_puts_cstr(fd, cstr) ({          \
  const char *_cstr_ = (cstr);                 \
  syx_io_puts_n((fd), _cstr_, strlen(_cstr_)); \
})
#define syx_io_puts_strlit(fd, lit) syx_io_puts_n((fd), (lit), sizeof(lit) - 1)

ssize_t syx_io__value_fprint(Syx_Eval_Ctx *ctx, FILE *f, Syx_Value *value, String_Builder *sb);
#define syx_io_value_fprint(ctx, f, value, ...) syx_io__value_fprint((ctx), (f), (value), WITH_DEFAULT(NULL, __VA_ARGS__))
#define syx_io_value_print(ctx, value, ...) syx_io__value_fprint((ctx), stdout, (value), WITH_DEFAULT(NULL, __VA_ARGS__))

ssize_t syx__io_values_fprint(Syx_Eval_Ctx *ctx, FILE *f, Syx_Pair *arguments);
#define syx_io_values_fprint(ctx, f, ...) ({                           \
  Syx_Value *arguments = rc_acquire(make_syx_value_list(__VA_ARGS__)); \
  ssize_t count = syx__io_values_fprint((ctx), (f), argumens->pair);   \
  rc_release(arguments);                                               \
  count;                                                               \
})
#define syx_io_values_print(ctx, ...) syx_io_values_fprint((ctx), stdout __VA_OPT__(, ) __VA_ARGS__)

ssize_t syx__io_values_fprintln(Syx_Eval_Ctx *ctx, FILE *f, Syx_Pair *arguments);
#define syx_io_values_fprintln(ctx, f, ...) ({                         \
  Syx_Value *arguments = rc_acquire(make_syx_value_list(__VA_ARGS__)); \
  ssize_t count = syx__io_values_fprintln((ctx), (f), argumens->pair); \
  rc_release(arguments);                                               \
  count;                                                               \
})
#define syx_io_values_println(ctx, ...) syx_io_values_fprintln((ctx), stdout __VA_OPT__(, ) __VA_ARGS__)

ssize_t syx__io_values_fprintf(Syx_Eval_Ctx *ctx, FILE *f, String fmt, Syx_Pair *arguments);
#define syx_io_values_fprintf(ctx, f, fmt, ...) ({                           \
  Syx_Value *arguments = rc_acquire(make_syx_value_list(__VA_ARGS__));       \
  ssize_t count = syx__io_values_fprintf((ctx), (f), (fmt), argumens->pair); \
  rc_release(arguments);                                                     \
  count;                                                                     \
})
#define syx_io_values_printf(ctx, ...) syx_io_values_fprintf((ctx), stdout, (fmt)__VA_OPT__(, ) __VA_ARGS__)
#define syx_io_values_fprintf_strlit(ctx, f, fmt, ...) syx_io_values_fprintf((ctx), (f), string_from_strlit(fmt) __VA_OPT__(, ) __VA_ARGS__)
#define syx_io_values_printf_strlit(ctx, fmt, ...) syx_io_values_fprintf((ctx), stdout, string_from_strlit(fmt) __VA_OPT__(, ) __VA_ARGS__)

#endif // SYX_IO_H

#if defined(SYX_IO_IMPL) && !defined(SYX_IO_IMPL_C)
#define SYX_IO_IMPL_C

#define STR_IMPL
#include <str.h>

size_t syx_io_putc(FILE *fd, char c) {
  if (fputc(c, fd) < 0) return 0;
  if (c == '\n') syx_io_flash(fd);
  return 1;
}

void syx_io_flash(FILE *fd) {
  fflush(fd);
}

ssize_t syx_io_puts_n(FILE *fd, const char *const str, size_t n) {
  for (size_t index = 0; index < n; index += 1) {
    if (!syx_io_putc(fd, str[index])) return -(ssize_t)index;
  }
  return n;
}

ssize_t syx_io__value_fprint(Syx_Eval_Ctx *ctx, FILE *f, Syx_Value *value, String_Builder *sb) {
  if (value->kind == SYX_VALUE_KIND_EXIT) return 0;
  Syx_Value *converted = rc_acquire(syx_convert_to_string(ctx, value));
  if (converted->kind == SYX_VALUE_KIND_EXIT) return (rc_release(value), 0);
  rc_release(value);
  sb_append_sv(sb, *converted->string);
  rc_release(converted);
  return syx_io_puts_n(f, sb->data, sb->count);
}

ssize_t syx__io_values_fprint(Syx_Eval_Ctx *ctx, FILE *f, Syx_Pair *arguments) {
  bool first = true;
  ssize_t count = 0;
  String_Builder sb = {0};
  while (arguments) {
    if (!first) {
      if (!syx_io_putc(f, ' ')) goto error;
      count += 1;
    }
    first = false;
    Syx_Value *value = rc_acquire(syx_list_next(&arguments));
    size_t value_count = syx_io__value_fprint(ctx, f, value, &sb);
    if (!value_count) goto error;
    count += value_count;
    if (value_count < 0) goto error;
  }
  sb_free(&sb);
  return count;
error:
  sb_free(&sb);
  return -count;
}

ssize_t syx__io_values_fprintln(Syx_Eval_Ctx *ctx, FILE *f, Syx_Pair *arguments) {
  ssize_t count = syx__io_values_fprint(ctx, f, arguments);
  if (count <= 0) return count;
  if (!syx_io_putc(f, '\n')) return count;
  return count + 1;
}

ssize_t syx__io_values_fprintf(Syx_Eval_Ctx *ctx, FILE *f, String fmt, Syx_Pair *arguments) {
  ssize_t count = 0;
  String_Builder sb = {0};
  for (size_t index = 0; index < fmt.count; index += utf8_character_lengths[(uint8_t)fmt.data[index]]) {
    if (fmt.data[index] != '%') goto put_char;
    if (index + 1 < fmt.count && fmt.data[index + 1] == '%') goto put_char;
    Syx_Value *value = rc_acquire(syx_list_next(&arguments));
    ssize_t value_count = syx_io__value_fprint(ctx, f, value, &sb);
    if (!value_count) goto error;
    count += value_count;
    if (value_count < 0) goto error;
    continue;
  put_char:
    if (!syx_io_putc(f, fmt.data[index])) goto error;
    count += 1;
  }
  sb_free(&sb);
  return count;
error:
  sb_free(&sb);
  return -count;
}

#endif // SYX_IO_IMPL
