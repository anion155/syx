#ifndef DEFINES_H_
#define DEFINES_H_

#define FE_1(WHAT, X, ...) WHAT(X)
#define FE_2(WHAT, X, ...) WHAT(X), FE_1(WHAT, __VA_ARGS__)
#define FE_3(WHAT, X, ...) WHAT(X), FE_2(WHAT, __VA_ARGS__)
#define FE_4(WHAT, X, ...) WHAT(X), FE_3(WHAT, __VA_ARGS__)
#define FE_5(WHAT, X, ...) WHAT(X), FE_4(WHAT, __VA_ARGS__)
#define FE_6(WHAT, X, ...) WHAT(X), FE_5(WHAT, __VA_ARGS__)
#define FE_GET_MACRO(_1, _2, _3, _4, _5, _6, NAME, ...) NAME
#define FOR_EACH(MACRO, ...) FE_GET_MACRO(__VA_ARGS__, FE_6, FE_5, FE_4, FE_3, FE_2, FE_1)(MACRO, __VA_ARGS__)

#define UNUSED_CAST(var) (void)(var)
#define UNUSED(...) (FOR_EACH(UNUSED_CAST, __VA_ARGS__))

#define STRINGIFY(x) #x
#define STRINGIFY2(x) STRINGIFY(x)

#define EXPAND(...) __VA_ARGS__
#define EXPAND_WITH_COMMA(...) __VA_OPT__(, ) __VA_ARGS__
#define EXPAND_MACRO(MACRO, ...) MACRO(__VA_ARGS__)

#define FIRST_ARG(a, ...) a
#define REST_ARGS(a, ...) __VA_ARGS__
#define SECOND_ARG(a, b, ...) b
#define THIRD_ARG(a, b, c, ...) c
#define FORTH_ARG(a, b, c, d, ...) d

#define WITH_DEFAULT(d, ...) SECOND_ARG(__VA_OPT__(, ) __VA_ARGS__, d)
#define WITH_TWO_DEFAULTS(d1, d2, ...) SECOND_ARG(dummy __VA_OPT__(, __VA_ARGS__), d1), THIRD_ARG(dummy __VA_OPT__(, __VA_ARGS__), d2, d2)

#define MIN(a, b) ((a) < (b) ? (a) : (b))

#define STATIC_ASSERT(condition, msg) sizeof(struct { _Static_assert((condition), msg); int dummy; })
#define TYPE_ASSERT(value, expected, ...) STATIC_ASSERT( \
    _Generic((value), expected: true, default: false),   \
    WITH_DEFAULT("Type mismatch", __VA_ARGS__))

#if defined(__GNUC__) || defined(__clang__)
#  ifdef __MINGW_PRINTF_FORMAT
#    define PRINTF_ATTRIBUTE(STRING_INDEX, FIRST_TO_CHECK) __attribute__((format(__MINGW_PRINTF_FORMAT, STRING_INDEX, FIRST_TO_CHECK)))
#  else
#    define PRINTF_ATTRIBUTE(STRING_INDEX, FIRST_TO_CHECK) __attribute__((format(printf, STRING_INDEX, FIRST_TO_CHECK)))
#  endif // __MINGW_PRINTF_FORMAT
#  define PRINTF_FMT_PARAM
#elif defined(_MSC_VER)
#  include <sal.h>
#  define PRINTF_ATTRIBUTE(STRING_INDEX, FIRST_TO_CHECK)
#  define PRINTF_FMT_PARAM _Printf_format_string_
#else
#  define PRINTF_ATTRIBUTE(STRING_INDEX, FIRST_TO_CHECK)
#  define PRINTF_FMT_PARAM
#endif

#ifndef __STDC_VERSION__
#  define constexpr static const
#elif __STDC_VERSION__ < 202311L
#  define constexpr static const
#endif

#endif // DEFINES_H_
