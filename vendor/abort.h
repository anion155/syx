/**
 * abort.h - 0.1.0 - Public Domain - https://github.com/anion155/c-tools
 *
 * Abort utilities for c23.
 *
 * ## Requirements
 *
 * - C23
 * - GNU statement expressions
 */

#ifndef ABORT_H
#define ABORT_H

#include <defines.h>

[[noreturn]] void panicf(const char *file, int line, const char *label, PRINTF_FMT_PARAM const char *format, ...) PRINTF_ATTRIBUTE(4, 5);

#define TODO_DEFAULT_MESSAGE "not implemented yet"
/** Panics with "TODO" message */
#define TODO(...) panicf(__FILE__, __LINE__, "TODO", WITH_DEFAULT(TODO_DEFAULT_MESSAGE, __VA_ARGS__) EXPAND_WITH_COMMA(REST_ARGS(__VA_ARGS__)))

#define UNREACHABLE_DEFAULT_MESSAGE "should never happen"
/** Panics with "UNREACHABLE" message */
#define UNREACHABLE(...) panicf(__FILE__, __LINE__, "UNREACHABLE", WITH_DEFAULT(UNREACHABLE_DEFAULT_MESSAGE, __VA_ARGS__) EXPAND_WITH_COMMA(REST_ARGS(__VA_ARGS__)))

/** If not `condition` panics with "UNREACHABLE" message */
#define ASSERT(condition, format, ...) ({ if (!(condition)) UNREACHABLE(format __VA_OPT__(, ) __VA_ARGS__); })
/** Static assert as an expression. */
#define STATIC_ASSERT(condition, msg) sizeof(struct { _Static_assert((condition), msg); int dummy; })
/** Static types are compatible assert. */
#define TYPE_ASSERT(value, expected, ...) STATIC_ASSERT(_Generic((value), expected: true, default: false), WITH_DEFAULT("Type mismatch", __VA_ARGS__))

#endif // ABORT_H

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
