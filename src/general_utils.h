#ifndef GENERAL_UTILS_H
#define GENERAL_UTILS_H

#include <nob.h>

#define sv_from_like(sv) nob_sv_from_parts((sv).data, (sv).count)
#define sv_like_eq(a, b) ({                                          \
  typeof((a)) ac = (a);                                              \
  typeof((b)) bc = (b);                                              \
  ac.count == bc.count && (memcmp(ac.data, bc.data, ac.count) == 0); \
})

String_Builder sb_copy_from_sv(String_View sv);

syx_string_view temp_view_vsprintf(const char *format, va_list ap);
syx_string_view temp_view_sprintf(const char *format, ...) NOB_PRINTF_FORMAT(1, 2);

#endif // GENERAL_UTILS_H

#define GENERAL_UTILS_IMPL
#if defined(GENERAL_UTILS_IMPL) && !defined(GENERAL_UTILS_IMPL_C)
#define GENERAL_UTILS_IMPL_C

#define NOB_IMPL
#include <nob.h>

String_Builder sb_copy_from_sv(String_View sv) {
  String_Builder sb = {0};
  da_realloc_capacity(&sb, sv.count + 1);
  sb_append_sv(&sb, sv);
  sb_append(&sb, 0);
  return sb;
}

syx_string_view temp_view_vsprintf(const char *format, va_list ap) {
  syx_string_view sv = {0};
  va_list args;

  va_copy(args, ap);
  sv.count = vsnprintf(NULL, 0, format, args);
  va_end(args);
  assert(sv.count >= 0);

  sv.data = nob_temp_alloc(sv.count + 1);
  assert(sv.data != NULL && "Extend the size of the temporary allocator");

  va_copy(args, ap);
  vsnprintf(sv.data, sv.count + 1, format, args);
  va_end(args);

  return sv;
}

syx_string_view temp_view_sprintf(const char *format, ...) {
  va_list args;
  va_start(args, format);
  syx_string_view sv = temp_view_vsprintf(format, args);
  va_end(args);
  return sv;
}

#endif // GENERAL_UTILS_IMPL_C
