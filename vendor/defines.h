/**
 * defines.h - 0.4.0 - Public Domain - https://github.com/anion155/c-tools
 *
 * Preprocessor utilities for c.
 */

#ifndef DEFINES_H
#define DEFINES_H

#define FE_1(WHAT, X, ...) WHAT(X)
#define FE_2(WHAT, X, ...) WHAT(X), FE_1(WHAT, __VA_ARGS__)
#define FE_3(WHAT, X, ...) WHAT(X), FE_2(WHAT, __VA_ARGS__)
#define FE_4(WHAT, X, ...) WHAT(X), FE_3(WHAT, __VA_ARGS__)
#define FE_5(WHAT, X, ...) WHAT(X), FE_4(WHAT, __VA_ARGS__)
#define FE_6(WHAT, X, ...) WHAT(X), FE_5(WHAT, __VA_ARGS__)
#define FE_7(WHAT, X, ...) WHAT(X), FE_6(WHAT, __VA_ARGS__)
#define FE_8(WHAT, X, ...) WHAT(X), FE_7(WHAT, __VA_ARGS__)
#define FE_9(WHAT, X, ...) WHAT(X), FE_8(WHAT, __VA_ARGS__)
#define FE_GET_MACRO(_1, _2, _3, _4, _5, _6, _7, _8, _9, NAME, ...) NAME
/** Macro to run macro for each argument separated by comma. */
#define FOR_EACH(MACRO, ...) FE_GET_MACRO(__VA_ARGS__, FE_9, FE_8, FE_7, FE_6, FE_5, FE_4, FE_3, FE_2, FE_1)(MACRO, __VA_ARGS__)

#define UNUSED_CAST(var) (void)(var)
/** Variadic UNUSED macro, that marks all passed variables as used. */
#define UNUSED(...) (FOR_EACH(UNUSED_CAST, __VA_ARGS__))

/** 'No operation' macro. */
#define NOOP() (void)0

#define _STRINGIFY(x) #x
/** Macro to create string literal from tokens. */
#define STRINGIFY(x) _STRINGIFY(x)

/** Macro that expands to __VA_ARGS__. */
#define EXPAND(...) __VA_ARGS__
/** Macro that expands to __VA_ARGS__ with comma before it. */
#define EXPAND_WITH_COMMA(...) __VA_OPT__(, ) __VA_ARGS__
/** Macro that removes parentheses from `args`. */
#define EXPAND_PARENS(args) EXPAND args
/** Macro that expands to MACRO call with __VA_ARGS__. */
#define EXPAND_MACRO(MACRO, ...) MACRO(__VA_ARGS__)

/** Macro that removes first argument. */
#define REST_ARGS(a, ...) __VA_ARGS__

/** Macro that expands to first argument. */
#define FIRST_ARG(a, ...) a
/** Macro that expands to second argument. */
#define SECOND_ARG(a, b, ...) b
/** Macro that expands to third argument. */
#define THIRD_ARG(a, b, c, ...) c
/** Macro that expands to forth argument. */
#define FORTH_ARG(a, b, c, d, ...) d

/** Macro that expands to first argument in __VA_ARGS__, or to d. */
#define WITH_DEFAULT(d, ...) SECOND_ARG(__VA_OPT__(, ) __VA_ARGS__, d)
/** Macro that expands to first two arguments in __VA_ARGS__, or to d1, d2. */
#define WITH_TWO_DEFAULTS(d1, d2, ...) SECOND_ARG(dummy __VA_OPT__(, __VA_ARGS__), d1), THIRD_ARG(dummy __VA_OPT__(, __VA_ARGS__), d2, d2)

/** Macro that expands to `yes` if it was provided with __VA_ARGS__ and to `no` if not. */
#define IF_VA_OPT(yes, no, ...) SECOND_ARG(__VA_OPT__(, ) yes, no)

/** Macro that compares two values and returns minimal. */
#define MIN(a, b) ((a) < (b) ? (a) : (b))

#if defined(__GNUC__) || defined(__clang__)
#  ifdef __MINGW_PRINTF_FORMAT
/** Marks function with printf static check. */
#    define PRINTF_ATTRIBUTE(STRING_INDEX, FIRST_TO_CHECK) __attribute__((format(__MINGW_PRINTF_FORMAT, STRING_INDEX, FIRST_TO_CHECK)))
#  else
/** Marks function with printf static check. */
#    define PRINTF_ATTRIBUTE(STRING_INDEX, FIRST_TO_CHECK) __attribute__((format(printf, STRING_INDEX, FIRST_TO_CHECK)))
#  endif // __MINGW_PRINTF_FORMAT
/** Marks function parameter as printf format argument. */
#  define PRINTF_FMT_PARAM
#elif defined(_MSC_VER)
#  include <sal.h>
/** Marks function with printf static check. */
#  define PRINTF_ATTRIBUTE(STRING_INDEX, FIRST_TO_CHECK)
/** Marks function parameter as printf format argument. */
#  define PRINTF_FMT_PARAM _Printf_format_string_
#else
/** Marks function with printf static check. */
#  define PRINTF_ATTRIBUTE(STRING_INDEX, FIRST_TO_CHECK)
/** Marks function parameter as printf format argument. */
#  define PRINTF_FMT_PARAM
#endif

#endif // DEFINES_H

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
