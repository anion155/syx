#ifndef MAGIC_H_
#define MAGIC_H_

#define STRINGIFY(x) #x
#define STRINGIFY2(x) STRINGIFY(x)

#define FIRST_ARG(a, ...) a
#define REST_ARGS(a, ...) __VA_ARGS__
#define SECOND_ARG(a, b, ...) b
#define THIRD_ARG(a, b, c, ...) c

#define WITH_DEFAULT(d, ...) SECOND_ARG(__VA_OPT__(, ) __VA_ARGS__, d)
#define WITH_TWO_DEFAULTS(d1, d2, ...) SECOND_ARG(dummy __VA_OPT__(, __VA_ARGS__), d1), THIRD_ARG(dummy __VA_OPT__(, __VA_ARGS__), d2, d2)

#define MIN(a, b) ((a) < (b) ? (a) : (b))

#define STATIC_ASSERT(condition, msg) sizeof(struct { _Static_assert((condition), msg); int dummy; })
#define TYPE_ASSERT(value, expected, ...) STATIC_ASSERT( \
    _Generic((value), expected: true, default: false),   \
    WITH_DEFAULT("Type mismatch", __VA_ARGS__))

#endif // MAGIC_H_
