#ifndef ABORT_H_
#define ABORT_H_

#include <defines.h>

void panicf(const char *file, int line, const char *label, PRINTF_FMT_PARAM const char *format, ...) PRINTF_ATTRIBUTE(4, 5);
#define TODO(format, ...) panicf(__FILE__, __LINE__, "TODO", format __VA_OPT__(, ) __VA_ARGS__)
#define UNREACHABLE(format, ...) panicf(__FILE__, __LINE__, "UNREACHABLE", format __VA_OPT__(, ) __VA_ARGS__)

#endif // ABORT_H_

#if defined(ABORT_IMPL) && !defined(ABORT_IMPL_C)
#define ABORT_IMPL_C

void panicf(const char *file, int line, const char *label, const char *format, ...) {
  fprintf(stderr, "%s:%d: %s: ", file, line, label);
  va_list args;
  va_start(args, format);
  vfprintf(stderr, format, args);
  va_end(args);
  fprintf(stderr, "\n");
  abort();
}

#endif // ABORT_IMPL_C
