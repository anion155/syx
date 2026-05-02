#ifndef MAGIC_H_
#define MAGIC_H_

#define STRINGIFY(x) #x

#define SECOND_ARG(a, b, ...) b
#define THIRD_ARG(a, b, c, ...) c

#define WITH_DEFAULT(d, ...) SECOND_ARG(__VA_OPT__(, ) __VA_ARGS__, d)
#define WITH_TWO_DEFAULTS(d1, d2, ...) SECOND_ARG(dummy __VA_OPT__(, __VA_ARGS__), d1), THIRD_ARG(dummy __VA_OPT__(, __VA_ARGS__), d2, d2)

#define MIN(a, b) ((a) < (b) ? (a) : (b))

#endif // MAGIC_H_
