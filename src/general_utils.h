#ifndef GENERAL_UTILS_H
#define GENERAL_UTILS_H

syx_string_view temp_view_vsprintf(const char *format, va_list ap);
syx_string_view temp_view_sprintf(const char *format, ...) NOB_PRINTF_FORMAT(1, 2);

#endif // GENERAL_UTILS_H

#if defined(GENERAL_UTILS_IMPL) && !defined(GENERAL_UTILS_IMPL_C)
#define GENERAL_UTILS_IMPL_C

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
  vsnprintf((char *)sv.data, sv.count + 1, format, args);
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
