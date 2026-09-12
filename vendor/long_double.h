#ifndef LONG_DOUBLE_H
#define LONG_DOUBLE_H

#include <defines.h>

#define LD_KIND_F64 0
#define LD_KIND_F80 1
#define LD_KIND_F128 2
#define LD_KIND_F64PAIR 2

#if defined(_MSC_VER) || defined(_WIN32)
#  define LD_KIND LD_KIND_F64
#elif defined(__APPLE__) && (defined(__arm64__) || defined(__aarch64__))
#  define LD_KIND LD_KIND_F64
#elif defined(__ppc64__) || defined(__PPC64__) || defined(_ARCH_PPC)
#  if defined(__LONG_DOUBLE_128__) && !defined(__IEEE_FLOAT__)
#    define LD_KIND LD_KIND_F64PAIR
#  elif defined(__IEEE_FLOAT__)
#    define LD_KIND LD_KIND_F128
#  else
#    error "Unsupported or unknown long double architecture."
#  endif
#elif defined(__i386__) || defined(__x86_64__) || defined(_M_IX86) || defined(_M_X64)
#  define LD_KIND LD_KIND_F80
#elif defined(__aarch64__) || defined(__riscv) || defined(__sparc__)
#  define LD_KIND LD_KIND_F128
#elif LDBL_MANT_DIG == 53
#  define LD_KIND LD_KIND_F64
#elif LDBL_MANT_DIG == 64
#  define LD_KIND LD_KIND_F80
#elif LDBL_MANT_DIG == 113
#  define LD_KIND LD_KIND_F128
#elif LDBL_MANT_DIG == 106
#  define LD_KIND LD_KIND_F64PAIR
#else
#  error "Unsupported or unknown long double architecture."
#endif

#pragma pack(push, 1)
typedef struct f16_canonical_t {
  uint16_t bits;
} f16_canonical_t;

typedef union f80_canonical_t {
  struct {
    uint64_t mantissa;
    uint16_t exponent_sign;
  };
} f80_canonical_t;

typedef union f128_canonical_t {
  __uint128_t bits;
  struct {
    uint64_t low;
    uint64_t high;
  };
} f128_canonical_t;

typedef union f64pair_canonical_t {
  struct {
    double head;
    double tail;
  };
  struct {
    uint64_t low;
    uint64_t high;
  };
  __uint128_t bits;
} f64pair_canonical_t;
#pragma pack(pop)
_Static_assert(sizeof(f16_canonical_t) == 2, "Canonical f16 struct must be 2 bytes");
_Static_assert(sizeof(f80_canonical_t) == 10, "Canonical f80 struct must be 10 bytes");
_Static_assert(sizeof(f128_canonical_t) == 16, "Canonical f128 struct must be 16 bytes");
_Static_assert(sizeof(f64pair_canonical_t) == 16, "Canonical f64pair struct must be 16 bytes");

#if defined(__FLT16_MAX__)
typedef _Float16 f16_t;
#else
typedef f16_canonical_t f16_t;
#endif
#if LD_KIND == LD_KIND_F64
typedef f80_canonical_t f80_t;
typedef f128_canonical_t f128_t;
typedef f64pair_canonical_t f64pair_t;
#elif LD_KIND == LD_KIND_F80
typedef long double f80_t;
typedef f128_canonical_t f128_t;
typedef f64pair_canonical_t f64pair_t;
#elif LD_KIND == LD_KIND_F128
typedef f80_canonical_t f80_t;
typedef long double f128_t;
typedef f64pair_canonical_t f64pair_t;
#elif LD_KIND == LD_KIND_F64PAIR
typedef f80_canonical_t f80_t;
typedef f128_canonical_t f128_t;
typedef long double f64pair_t;
#else
#  error "Unsupported or unknown long double architecture."
#endif

static inline f16_canonical_t f16_canonical_identity(f16_canonical_t value) { return value; }
#if defined(__FLT16_MAX__)
f16_canonical_t f16_canonical_from_native(_Float16 value);
#  define F16__CANONICAL_FROM_NATIVE_CASE _Float16 : f16_canonical_from_native,
#else
f16_canonical_t f16_canonical_from_native(float value);
#  define F16__CANONICAL_FROM_NATIVE_CASE
#endif
f16_canonical_t f16_canonical_from_float(float value);
f16_canonical_t f16_canonical_from_double(double value);
f16_canonical_t f16_canonical_from_long_double(long double value);
f16_canonical_t f16_canonical_from_f80_canonical(f80_canonical_t value);
f16_canonical_t f16_canonical_from_f128_canonical(f128_canonical_t value);
f16_canonical_t f16_canonical_from_f64pair_canonical(f64pair_canonical_t value);
// clang-format off
#define f16_canonical_from(value) _Generic((value),            \
    F16__CANONICAL_FROM_NATIVE_CASE                            \
    float: f16_canonical_from_float,                           \
    double: f16_canonical_from_double,                         \
    long double: f16_canonical_from_long_double,               \
    f16_canonical_t: f16_canonical_identity,                   \
    f80_canonical_t: f16_canonical_from_f80_canonical,         \
    f128_canonical_t: f16_canonical_from_f128_canonical,       \
    f64pair_canonical_t: f16_canonical_from_f64pair_canonical)((value))
// clang-format on
#if defined(__FLT16_MAX__)
_Float16 f16_canonical_to_native(f16_canonical_t value);
#  define F16__CANONICAL_TO_NATIVE_CASE _Float16 : f16_canonical_to_native,
#else
#  define F16__CANONICAL_TO_NATIVE_CASE
#endif
float f16_canonical_to_float(f16_canonical_t value);
double f16_canonical_to_double(f16_canonical_t value);
long double f16_canonical_to_long_double(f16_canonical_t value);
// clang-format off
#define f16_canonical_to(value, type) _Generic((type){0}, \
    F16__CANONICAL_TO_NATIVE_CASE                         \
    float: f16_canonical_to_float,                        \
    double: f16_canonical_to_double,                      \
    long double: f16_canonical_to_long_double,            \
    f16_canonical_t: f16_identity,                        \
    f80_canonical_t: f80_canonical_from_f16_canonical,    \
    f128_canonical_t: f128_canonical_from_f16_canonical,  \
    f64pair_canonical_t: f64pair_canonical_from_f16_canonical)((value))
// clang-format on

static inline f80_canonical_t f80_canonical_identity(f80_canonical_t value) { return value; }
f80_canonical_t f80_canonical_from_float(float value);
f80_canonical_t f80_canonical_from_double(double value);
f80_canonical_t f80_canonical_from_long_double(long double value);
f80_canonical_t f80_canonical_from_f16_canonical(f16_canonical_t value);
f80_canonical_t f80_canonical_from_f128_canonical(f128_canonical_t value);
f80_canonical_t f80_canonical_from_f64pair_canonical(f64pair_canonical_t value);
// clang-format off
#define f80_canonical_from(value) _Generic((value),            \
    float: f80_canonical_from_float,                           \
    double: f80_canonical_from_double,                         \
    long double: f80_canonical_from_long_double,               \
    f16_canonical_t: f80_canonical_from_f16_canonical,         \
    f80_canonical_t: f80_canonical_identity,                   \
    f128_canonical_t: f80_canonical_from_f128_canonical,       \
    f64pair_canonical_t: f80_canonical_from_f64pair_canonical)((value))
// clang-format on
#if defined(__FLT16_MAX__)
_Float16 f80_canonical_to_half(f80_canonical_t value);
#  define F80__CANONICAL_TO_HALF_CASE _Float16 : f80_canonical_to_half,
#else
#  define F80__CANONICAL_TO_HALF_CASE
#endif
float f80_canonical_to_float(f80_canonical_t value);
double f80_canonical_to_double(f80_canonical_t value);
long double f80_canonical_to_long_double(f80_canonical_t value);
// clang-format off
#define f80_canonical_to(value, type) _Generic((type){0},    \
    F80__CANONICAL_TO_NATIVE_CASE                            \
    float: f80_canonical_to_float,                           \
    double: f80_canonical_to_double,                         \
    long double: f80_canonical_to_long_double,               \
    f16_canonical_t: f80_canonical_from_f16_canonical,       \
    f80_canonical_t: f80_canonical_identity,                 \
    f128_canonical_t: f128_canonical_from_f80_canonical,     \
    f64pair_canonical_t: f64pair_canonical_from_f80_canonical)((value))
// clang-format on

static inline f128_canonical_t f128_canonical_identity(f128_canonical_t value) { return value; }
f128_canonical_t f128_canonical_from_float(float value);
f128_canonical_t f128_canonical_from_double(double value);
f128_canonical_t f128_canonical_from_long_double(long double value);
f128_canonical_t f128_canonical_from_f16_canonical(f16_canonical_t value);
f128_canonical_t f128_canonical_from_f80_canonical(f80_canonical_t value);
f128_canonical_t f128_canonical_from_f64pair_canonical(f64pair_canonical_t value);
// clang-format off
#define f128_canonical_from(value) _Generic((value),            \
    float: f128_canonical_from_float,                           \
    double: f128_canonical_from_double,                         \
    long double: f128_canonical_from_long_double,               \
    f16_canonical_t: f128_canonical_from_f16_canonical,         \
    f80_canonical_t: f128_canonical_from_f80_canonical,         \
    f128_canonical_t: f128_canonical_identity,                  \
    f64pair_canonical_t: f128_canonical_from_f64pair_canonical)((value))
// clang-format on
#if defined(__FLT16_MAX__)
_Float16 f128_canonical_to_half(f128_canonical_t value);
#  define F128__CANONICAL_TO_HALF_CASE _Float16 : f128_canonical_to_half,
#else
#  define F128__CANONICAL_TO_HALF_CASE
#endif
float f128_canonical_to_float(f128_canonical_t value);
double f128_canonical_to_double(f128_canonical_t value);
long double f128_canonical_to_long_double(f128_canonical_t value);
// clang-format off
#define f128_canonical_to(value, type) _Generic((type){0},    \
    F128__CANONICAL_TO_NATIVE_CASE                            \
    float: f128_canonical_to_float,                           \
    double: f128_canonical_to_double,                         \
    long double: f128_canonical_to_long_double,               \
    f16_canonical_t: f128_canonical_from_f16_canonical,       \
    f80_canonical_t: f80_canonical_from_f128_canonical,       \
    f128_canonical_t: f128_canonical_identity,                \
    f64pair_canonical_t: f64pair_canonical_from_f128_canonical)((value))
// clang-format on

static inline f64pair_canonical_t f64pair_canonical_identity(f64pair_canonical_t value) { return value; }
f64pair_canonical_t f64pair_canonical_from_float(float value);
f64pair_canonical_t f64pair_canonical_from_double(double value);
f64pair_canonical_t f64pair_canonical_from_long_double(long double value);
f64pair_canonical_t f64pair_canonical_from_f16_canonical(f16_canonical_t value);
f64pair_canonical_t f64pair_canonical_from_f80_canonical(f80_canonical_t value);
f64pair_canonical_t f64pair_canonical_from_f128_canonical(f128_canonical_t value);
// clang-format off
#define f64pair_canonical_from(value) _Generic((value),      \
    float: f64pair_canonical_from_float,                     \
    double: f64pair_canonical_from_double,                   \
    long double: f64pair_canonical_from_long_double,         \
    f16_canonical_t: f64pair_canonical_from_f16_canonical,   \
    f80_canonical_t: f64pair_canonical_from_f80_canonical,   \
    f128_canonical_t: f64pair_canonical_from_f128_canonical, \
    f64pair_canonical_t: f64pair_canonical_identity)((value))
// clang-format on
#if defined(__FLT16_MAX__)
_Float16 f64pair_canonical_to_half(f64pair_canonical_t value);
#  define F64PAIR__CANONICAL_TO_HALF_CASE _Float16 : f64pair_canonical_to_half,
#else
#  define F64PAIR__CANONICAL_TO_HALF_CASE
#endif
float f64pair_canonical_to_float(f64pair_canonical_t value);
double f64pair_canonical_to_double(f64pair_canonical_t value);
long double f64pair_canonical_to_long_double(f64pair_canonical_t value);
// clang-format off
#define f64pair_canonical_to(value, type) _Generic((type){0},    \
    F64PAIR__CANONICAL_TO_NATIVE_CASE                            \
    float: f64pair_canonical_to_float,                           \
    double: f64pair_canonical_to_double,                         \
    long double: f64pair_canonical_to_long_double,               \
    f16_canonical_t: f64pair_canonical_from_f16_canonical,       \
    f80_canonical_t: f80_canonical_from_f64pair_canonical,       \
    f128_canonical_t: f128_canonical_from_f64pair_canonical,     \
    f64pair_canonical_t: f64pair_canonical_identity)((value))
// clang-format on

static inline float float_identity(float value) { return value; }
static inline double double_identity(double value) { return value; }
static inline long double long_double_identity(long double value) { return value; }
#define f_canonical_to_native(value) _Generic((value), \
    float: float_identity,                             \
    double: double_identity,                           \
    long double: long_double_identity,                 \
    f16_canonical_t: f16_canonical_to_float,           \
    f80_canonical_t: f80_canonical_to_long_double,     \
    f128_canonical_t: f128_canonical_to_long_double,   \
    f64pair_canonical_t: f64pair_canonical_to_long_double)((value))

#define f_canonical_from_native(value, type) _Generic((type){0}, \
    float: float_identity,                                       \
    double: double_identity,                                     \
    long double: long_double_identity,                           \
    f16_canonical_t: f16_canonical_from_native,                  \
    f80_canonical_t: f80_canonical_from_long_double,             \
    f128_canonical_t: f128_canonical_from_long_double,           \
    f64pair_canonical_t: f64pair_canonical_from_long_double)((value))

uint16_t f16_to_bits(f16_t value);
__uint128_t f80_to_bits(f80_t value);
__uint128_t f128_to_bits(f128_t value);
__uint128_t f64pair_to_bits(f64pair_t value);

#endif // LONG_DOUBLE_H

#define LONG_DOUBLE_IMPL
#if defined(LONG_DOUBLE_IMPL) && !defined(LONG_DOUBLE_IMPL_C)
#define LONG_DOUBLE_IMPL_C

#define MEMORYCOPY(value, dest_type) ({                    \
  dest_type dest = {0};                                    \
  memcpy(&dest, &value, MIN(sizeof(value), sizeof(dest))); \
  dest;                                                    \
})

#if defined(__FLT16_MAX__)
f16_canonical_t f16_canonical_from_native(_Float16 value) {
  return MEMORYCOPY(value, f16_canonical_t);
}
#else
f16_canonical_t f16_canonical_from_native(float value) {
  UNUSED(value);
  TODO();
}
#endif

f16_canonical_t f16_canonical_from_float(float value) {
#if defined(__FLT16_MAX__)
  return f16_canonical_from_native(value);
#else
  TODO();
#endif
  UNUSED(value);
}

f16_canonical_t f16_canonical_from_double(double value) {
#if defined(__FLT16_MAX__)
  return f16_canonical_from_native(value);
#else
  TODO();
#endif
  UNUSED(value);
}

f16_canonical_t f16_canonical_from_long_double(long double value) {
#if defined(__FLT16_MAX__)
  return f16_canonical_from_native(value);
#elif LD_KIND == LD_KIND_F64
  TODO();
#elif LD_KIND == LD_KIND_F80
  TODO();
#elif LD_KIND == LD_KIND_F128
  TODO();
#elif LD_KIND == LD_KIND_F64PAIR
  TODO();
#else
#  error "Unsupported or unknown long double architecture."
#endif
  UNUSED(value);
}

f16_canonical_t f16_canonical_from_f80_canonical(f80_canonical_t value) {
#if !defined(__FLT16_MAX__)
  TODO();
#elif LD_KIND == LD_KIND_F64
  TODO();
#elif LD_KIND == LD_KIND_F80
  return MEMORYCOPY(value, long double);
#elif LD_KIND == LD_KIND_F128
  TODO();
#elif LD_KIND == LD_KIND_F64PAIR
  TODO();
#else
#  error "Unsupported or unknown long double architecture."
#endif
  UNUSED(value);
}

f16_canonical_t f16_canonical_from_f128_canonical(f128_canonical_t value) {
#if !defined(__FLT16_MAX__)
  TODO();
#elif LD_KIND == LD_KIND_F64
  TODO();
#elif LD_KIND == LD_KIND_F80
  TODO();
#elif LD_KIND == LD_KIND_F128
  return MEMORYCOPY(value, long double);
#elif LD_KIND == LD_KIND_F64PAIR
  TODO();
#else
#  error "Unsupported or unknown long double architecture."
#endif
  UNUSED(value);
}

f16_canonical_t f16_canonical_from_f64pair_canonical(f64pair_canonical_t value) {
#if !defined(__FLT16_MAX__)
  TODO();
#elif LD_KIND == LD_KIND_F64
  TODO();
#elif LD_KIND == LD_KIND_F80
  TODO();
#elif LD_KIND == LD_KIND_F128
  TODO();
#elif LD_KIND == LD_KIND_F64PAIR
  return MEMORYCOPY(value, long double);
#else
#  error "Unsupported or unknown long double architecture."
#endif
  UNUSED(value);
}

#if defined(__FLT16_MAX__)
_Float16 f16_canonical_to_native(f16_canonical_t value) {
  return MEMORYCOPY(value, _Float16);
}
#endif

float f16_canonical_to_float(f16_canonical_t value) {
#if defined(__FLT16_MAX__)
  return f16_canonical_to_native(value);
#else
  TODO();
#endif
  UNUSED(value);
}

double f16_canonical_to_double(f16_canonical_t value) {
#if defined(__FLT16_MAX__)
  return f16_canonical_to_native(value);
#else
  TODO();
#endif
  UNUSED(value);
}

long double f16_canonical_to_long_double(f16_canonical_t value) {
#if defined(__FLT16_MAX__)
  return f16_canonical_to_native(value);
#else
  TODO();
#endif
  UNUSED(value);
}

f80_canonical_t f80_canonical_from_float(float value) {
  return f80_canonical_from_long_double(value);
}

f80_canonical_t f80_canonical_from_double(double value) {
  return f80_canonical_from_long_double(value);
}

f80_canonical_t f80_canonical_from_long_double(long double value) {
#if LD_KIND == LD_KIND_F64
  TODO();
#elif LD_KIND == LD_KIND_F80
  return MEMORYCOPY(value, f80_canonical_t);
#elif LD_KIND == LD_KIND_F128
  TODO();
#elif LD_KIND == LD_KIND_F64PAIR
  TODO();
#else
#  error "Unsupported or unknown long double architecture."
#endif
  UNUSED(value);
}

f80_canonical_t f80_canonical_from_f16_canonical(f16_canonical_t value) {
#if defined(__FLT16_MAX__) && LD_KIND == LD_KIND_F80
  return f80_canonical_from_long_double(MEMORYCOPY(value, _Float16));
#else
  TODO();
#endif
  UNUSED(value);
}

f80_canonical_t f80_canonical_from_f128_canonical(f128_canonical_t value) {
  TODO();
  UNUSED(value);
}

f80_canonical_t f80_canonical_from_f64pair_canonical(f64pair_canonical_t value) {
  TODO();
  UNUSED(value);
}

#if defined(__FLT16_MAX__)
_Float16 f80_canonical_to_half(f80_canonical_t value) {
  return f80_canonical_to_long_double(value);
}
#endif

float f80_canonical_to_float(f80_canonical_t value) {
  return f80_canonical_to_long_double(value);
}

double f80_canonical_to_double(f80_canonical_t value) {
  return f80_canonical_to_long_double(value);
}

long double f80_canonical_to_long_double(f80_canonical_t value) {
#if LD_KIND == LD_KIND_F80
  return MEMORYCOPY(value, long double);
#else
  TODO();
#endif
  UNUSED(value);
}

f128_canonical_t f128_canonical_from_float(float value) {
  return f128_canonical_from_long_double(value);
}

f128_canonical_t f128_canonical_from_double(double value) {
  return f128_canonical_from_long_double(value);
}

f128_canonical_t f128_canonical_from_long_double(long double value) {
#if LD_KIND == LD_KIND_F64
  TODO();
#elif LD_KIND == LD_KIND_F80
  TODO();
#elif LD_KIND == LD_KIND_F128
  return MEMORYCOPY(value, f128_canonical_t);
#elif LD_KIND == LD_KIND_F64PAIR
  TODO();
#else
#  error "Unsupported or unknown long double architecture."
#endif
  UNUSED(value);
}

f128_canonical_t f128_canonical_from_f16_canonical(f16_canonical_t value) {
#if defined(__FLT16_MAX__) && LD_KIND == LD_KIND_F128
  return f128_canonical_from_long_double(MEMORYCOPY(value, _Float16));
#else
  TODO();
#endif
  UNUSED(value);
}

f128_canonical_t f128_canonical_from_f80_canonical(f80_canonical_t value) {
  TODO();
  UNUSED(value);
}

f128_canonical_t f128_canonical_from_f64pair_canonical(f64pair_canonical_t value) {
  TODO();
  UNUSED(value);
}

#if defined(__FLT16_MAX__)
_Float16 f128_canonical_to_half(f128_canonical_t value) {
  return f128_canonical_to_long_double(value);
}
#endif

float f128_canonical_to_float(f128_canonical_t value) {
  return f128_canonical_to_long_double(value);
}

double f128_canonical_to_double(f128_canonical_t value) {
  return f128_canonical_to_long_double(value);
}

long double f128_canonical_to_long_double(f128_canonical_t value) {
#if LD_KIND == LD_KIND_F128
  return MEMORYCOPY(value, long double);
#else
  TODO();
#endif
  UNUSED(value);
}

f64pair_canonical_t f64pair_canonical_from_float(float value) {
  return f64pair_canonical_from_long_double(value);
}

f64pair_canonical_t f64pair_canonical_from_double(double value) {
  return f64pair_canonical_from_long_double(value);
}

f64pair_canonical_t f64pair_canonical_from_long_double(long double value) {
#if LD_KIND == LD_KIND_F64
  TODO();
#elif LD_KIND == LD_KIND_F80
  TODO();
#elif LD_KIND == LD_KIND_F128
  TODO();
#elif LD_KIND == LD_KIND_F64PAIR
  return MEMORYCOPY(value, f64pair_canonical_t);
#else
#  error "Unsupported or unknown long double architecture."
#endif
  UNUSED(value);
}

f64pair_canonical_t f64pair_canonical_from_f16_canonical(f16_canonical_t value) {
#if defined(__FLT16_MAX__) && LD_KIND == LD_KIND_F64PAIR
  return f64pair_canonical_from_long_double(MEMORYCOPY(value, _Float16));
#else
  TODO();
#endif
  UNUSED(value);
}

f64pair_canonical_t f64pair_canonical_from_f80_canonical(f80_canonical_t value) {
  TODO();
  UNUSED(value);
}

f64pair_canonical_t f64pair_canonical_from_f128_canonical(f128_canonical_t value) {
  TODO();
  UNUSED(value);
}

#if defined(__FLT16_MAX__)
_Float16 f64pair_canonical_to_half(f64pair_canonical_t value) {
  return f64pair_canonical_to_long_double(value);
}
#endif

float f64pair_canonical_to_float(f64pair_canonical_t value) {
  return f64pair_canonical_to_long_double(value);
}

double f64pair_canonical_to_double(f64pair_canonical_t value) {
  return f64pair_canonical_to_long_double(value);
}

long double f64pair_canonical_to_long_double(f64pair_canonical_t value) {
#if LD_KIND == LD_KIND_F64PAIR
  return MEMORYCOPY(value, long double);
#else
  TODO();
#endif
  UNUSED(value);
}

uint16_t f16_to_bits(f16_t value) {
  return MEMORYCOPY(value, uint16_t);
}

__uint128_t f80_to_bits(f80_t value) {
  return MEMORYCOPY(value, __uint128_t);
}

__uint128_t f128_to_bits(f128_t value) {
  return MEMORYCOPY(value, __uint128_t);
}

__uint128_t f64pair_to_bits(f64pair_t value) {
  return MEMORYCOPY(value, __uint128_t);
}

#endif // LONG_DOUBLE_IMPL_C
