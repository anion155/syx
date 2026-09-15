/**
 * floats.h - 0.1.0 - Public Domain - https://github.com/anion155/c-tools
 *
 * Fixed width float types for c23.
 *
 * ## Usage example
 * ```c
  #define FLOATS_IMPL
  #include "floats.h"

  #if FLOATS_LD_KIND == FLOATS_LD_KIND_F64
  #  message "long double is IEEE 754 Double Precision"
  #elif FLOATS_LD_KIND == FLOATS_LD_KIND_F80
  #  message "long double is IEEE 754 Extended Precision"
  #elif FLOATS_LD_KIND == FLOATS_LD_KIND_F128
  #  message "long double is IEEE 754 Quadruple Precision"
  #elif FLOATS_LD_KIND == FLOATS_LD_KIND_F64PAIR
  #  message "long double is IBM double-double"
  #else
  #  error "Unsupported or unknown long double architecture."
  #endif
 * ```
 *
 * ## Requirements
 *
 * - C23
 * - GNU statement expressions
 * - [defines.h](./defines.h)
 * - [abort.h](./abort.h)
 */

#ifndef FLOATS_H
#define FLOATS_H

#include <defines.h>
#include <float.h>
#include <stdint.h>
#include <string.h>

#define FLOATS_LD_KIND_F64 0
#define FLOATS_LD_KIND_F80 1
#define FLOATS_LD_KIND_F128 2
#define FLOATS_LD_KIND_F64PAIR 3

#if defined(_MSC_VER) || defined(_WIN32)
#  define FLOATS_LD_KIND FLOATS_LD_KIND_F64
#elif defined(__APPLE__) && (defined(__arm64__) || defined(__aarch64__))
#  define FLOATS_LD_KIND FLOATS_LD_KIND_F64
#elif defined(__ppc64__) || defined(__PPC64__) || defined(_ARCH_PPC)
#  if defined(__LONG_DOUBLE_128__) && !defined(__IEEE_FLOAT__)
#    define FLOATS_LD_KIND FLOATS_LD_KIND_F64PAIR
#  elif defined(__IEEE_FLOAT__)
#    define FLOATS_LD_KIND FLOATS_LD_KIND_F128
#  else
#    error "Unsupported or unknown long double architecture."
#  endif
#elif defined(__i386__) || defined(__x86_64__)
#  define FLOATS_LD_KIND FLOATS_LD_KIND_F80
#elif defined(__aarch64__) || defined(__riscv) || defined(__sparc__)
#  define FLOATS_LD_KIND FLOATS_LD_KIND_F128
#elif LDBL_MANT_DIG == 53
#  define FLOATS_LD_KIND FLOATS_LD_KIND_F64
#elif LDBL_MANT_DIG == 64
#  define FLOATS_LD_KIND FLOATS_LD_KIND_F80
#elif LDBL_MANT_DIG == 113
#  define FLOATS_LD_KIND FLOATS_LD_KIND_F128
#elif LDBL_MANT_DIG == 106
#  define FLOATS_LD_KIND FLOATS_LD_KIND_F64PAIR
#else
#  error "Unsupported or unknown long double architecture."
#endif

#pragma pack(push, 1)
typedef struct f16_canonical_t {
  uint16_t bits;
} f16_canonical_t;
typedef struct f80_canonical_t {
  uint64_t mantissa;
  uint16_t exponent_sign;
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
typedef float f32_t;
typedef double f64_t;
#if FLOATS_LD_KIND == FLOATS_LD_KIND_F64
typedef f80_canonical_t f80_t;
typedef f128_canonical_t f128_t;
typedef f64pair_canonical_t f64pair_t;
typedef double fld_canonical_t;
#elif FLOATS_LD_KIND == FLOATS_LD_KIND_F80
typedef long double f80_t;
typedef f128_canonical_t f128_t;
typedef f64pair_canonical_t f64pair_t;
typedef f80_canonical_t fld_canonical_t;
#elif FLOATS_LD_KIND == FLOATS_LD_KIND_F128
typedef f80_canonical_t f80_t;
typedef long double f128_t;
typedef f64pair_canonical_t f64pair_t;
typedef f128_canonical_t fld_canonical_t;
#elif FLOATS_LD_KIND == FLOATS_LD_KIND_F64PAIR
typedef f80_canonical_t f80_t;
typedef f128_canonical_t f128_t;
typedef long double f64pair_t;
typedef f64pair_canonical_t fld_canonical_t;
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
_Float16 f16_canonical_to_half(f16_canonical_t value);
#  define F16__CANONICAL_TO_HALF_CASE _Float16 : f16_canonical_to_half,
#else
#  define F16__CANONICAL_TO_HALF_CASE
#endif
float f16_canonical_to_float(f16_canonical_t value);
double f16_canonical_to_double(f16_canonical_t value);
long double f16_canonical_to_long_double(f16_canonical_t value);
// clang-format off
#define f16_canonical_to(value, type) _Generic((type){0}, \
    F16__CANONICAL_TO_HALF_CASE                           \
    float: f16_canonical_to_float,                        \
    double: f16_canonical_to_double,                      \
    long double: f16_canonical_to_long_double,            \
    f16_canonical_t: f16_canonical_identity,              \
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
    F80__CANONICAL_TO_HALF_CASE                              \
    float: f80_canonical_to_float,                           \
    double: f80_canonical_to_double,                         \
    long double: f80_canonical_to_long_double,               \
    f16_canonical_t: f16_canonical_from_f80_canonical,       \
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
    F128__CANONICAL_TO_HALF_CASE                              \
    float: f128_canonical_to_float,                           \
    double: f128_canonical_to_double,                         \
    long double: f128_canonical_to_long_double,               \
    f16_canonical_t: f16_canonical_from_f128_canonical,       \
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
    F64PAIR__CANONICAL_TO_HALF_CASE                              \
    float: f64pair_canonical_to_float,                           \
    double: f64pair_canonical_to_double,                         \
    long double: f64pair_canonical_to_long_double,               \
    f16_canonical_t: f16_canonical_from_f64pair_canonical,       \
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
uint32_t f32_to_bits(f32_t value);
uint64_t f64_to_bits(f64_t value);
__uint128_t f80_to_bits(f80_t value);
__uint128_t f128_to_bits(f128_t value);
__uint128_t f64pair_to_bits(f64pair_t value);

#endif // FLOATS_H

#if defined(FLOATS_IMPL) && !defined(FLOATS_IMPL_C)
#define FLOATS_IMPL_C

#define ABORT_IMPL
#include <abort.h>

#define FLOATS_MEMORYCOPY(value, dest_type) ({             \
  dest_type dest = {0};                                    \
  memcpy(&dest, &value, MIN(sizeof(value), sizeof(dest))); \
  dest;                                                    \
})

#if defined(__FLT16_MAX__)
f16_canonical_t f16_canonical_from_native(_Float16 value) {
  return FLOATS_MEMORYCOPY(value, f16_canonical_t);
}
#else
f16_canonical_t f16_canonical_from_native(float value) {
  UNUSED(value);
  TODO("TASK(20260913-080138)");
}
#endif

f16_canonical_t f16_canonical_from_float(float value) {
#if defined(__FLT16_MAX__)
  return f16_canonical_from_native(value);
#else
  TODO("TASK(20260913-080156)");
#endif
  UNUSED(value);
}

f16_canonical_t f16_canonical_from_double(double value) {
#if defined(__FLT16_MAX__)
  return f16_canonical_from_native(value);
#else
  TODO("TASK(20260913-080204)");
#endif
  UNUSED(value);
}

f16_canonical_t f16_canonical_from_long_double(long double value) {
#if defined(__FLT16_MAX__)
  return f16_canonical_from_native(value);
#elif FLOATS_LD_KIND == FLOATS_LD_KIND_F64
  TODO("TASK(20260913-080213)");
#elif FLOATS_LD_KIND == FLOATS_LD_KIND_F80
  TODO("TASK(20260913-080213)");
#elif FLOATS_LD_KIND == FLOATS_LD_KIND_F128
  TODO("TASK(20260913-080213)");
#elif FLOATS_LD_KIND == FLOATS_LD_KIND_F64PAIR
  TODO("TASK(20260913-080213)");
#else
#  error "Unsupported or unknown long double architecture."
#endif
  UNUSED(value);
}

f16_canonical_t f16_canonical_from_f80_canonical(f80_canonical_t value) {
#if !defined(__FLT16_MAX__)
  TODO("TASK(20260913-080222)");
#elif FLOATS_LD_KIND == FLOATS_LD_KIND_F64
  TODO("TASK(20260913-080222)");
#elif FLOATS_LD_KIND == FLOATS_LD_KIND_F80
  return f16_canonical_from_native(FLOATS_MEMORYCOPY(value, long double));
#elif FLOATS_LD_KIND == FLOATS_LD_KIND_F128
  TODO("TASK(20260913-080222)");
#elif FLOATS_LD_KIND == FLOATS_LD_KIND_F64PAIR
  TODO("TASK(20260913-080222)");
#else
#  error "Unsupported or unknown long double architecture."
#endif
  UNUSED(value);
}

f16_canonical_t f16_canonical_from_f128_canonical(f128_canonical_t value) {
#if !defined(__FLT16_MAX__)
  TODO("TASK(20260913-080229)");
#elif FLOATS_LD_KIND == FLOATS_LD_KIND_F64
  TODO("TASK(20260913-080229)");
#elif FLOATS_LD_KIND == FLOATS_LD_KIND_F80
  TODO("TASK(20260913-080229)");
#elif FLOATS_LD_KIND == FLOATS_LD_KIND_F128
  return f16_canonical_from_native(FLOATS_MEMORYCOPY(value, long double));
#elif FLOATS_LD_KIND == FLOATS_LD_KIND_F64PAIR
  TODO("TASK(20260913-080229)");
#else
#  error "Unsupported or unknown long double architecture."
#endif
  UNUSED(value);
}

f16_canonical_t f16_canonical_from_f64pair_canonical(f64pair_canonical_t value) {
#if !defined(__FLT16_MAX__)
  TODO("TASK(20260913-080235)");
#elif FLOATS_LD_KIND == FLOATS_LD_KIND_F64
  TODO("TASK(20260913-080235)");
#elif FLOATS_LD_KIND == FLOATS_LD_KIND_F80
  TODO("TASK(20260913-080235)");
#elif FLOATS_LD_KIND == FLOATS_LD_KIND_F128
  TODO("TASK(20260913-080235)");
#elif FLOATS_LD_KIND == FLOATS_LD_KIND_F64PAIR
  return f16_canonical_from_native(FLOATS_MEMORYCOPY(value, long double));
#else
#  error "Unsupported or unknown long double architecture."
#endif
  UNUSED(value);
}

#if defined(__FLT16_MAX__)
_Float16 f16_canonical_to_half(f16_canonical_t value) {
  return FLOATS_MEMORYCOPY(value, _Float16);
}
#endif

float f16_canonical_to_float(f16_canonical_t value) {
#if defined(__FLT16_MAX__)
  return f16_canonical_to_half(value);
#else
  TODO("TASK(20260913-080244)");
#endif
  UNUSED(value);
}

double f16_canonical_to_double(f16_canonical_t value) {
#if defined(__FLT16_MAX__)
  return f16_canonical_to_half(value);
#else
  TODO("TASK(20260913-080251)");
#endif
  UNUSED(value);
}

long double f16_canonical_to_long_double(f16_canonical_t value) {
#if defined(__FLT16_MAX__)
  return f16_canonical_to_half(value);
#else
  TODO("TASK(20260913-080300)");
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
#if FLOATS_LD_KIND == FLOATS_LD_KIND_F64
  TODO("TASK(20260913-080306)");
#elif FLOATS_LD_KIND == FLOATS_LD_KIND_F80
  return FLOATS_MEMORYCOPY(value, f80_canonical_t);
#elif FLOATS_LD_KIND == FLOATS_LD_KIND_F128
  TODO("TASK(20260913-080306)");
#elif FLOATS_LD_KIND == FLOATS_LD_KIND_F64PAIR
  TODO("TASK(20260913-080306)");
#else
#  error "Unsupported or unknown long double architecture."
#endif
  UNUSED(value);
}

f80_canonical_t f80_canonical_from_f16_canonical(f16_canonical_t value) {
#if defined(__FLT16_MAX__) && FLOATS_LD_KIND == FLOATS_LD_KIND_F80
  return f80_canonical_from_long_double(FLOATS_MEMORYCOPY(value, _Float16));
#else
  TODO("TASK(20260913-080313)");
#endif
  UNUSED(value);
}

f80_canonical_t f80_canonical_from_f128_canonical(f128_canonical_t value) {
  TODO("TASK(20260913-080325)");
  UNUSED(value);
}

f80_canonical_t f80_canonical_from_f64pair_canonical(f64pair_canonical_t value) {
  TODO("TASK(20260913-080339)");
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
#if FLOATS_LD_KIND == FLOATS_LD_KIND_F80
  return FLOATS_MEMORYCOPY(value, long double);
#else
  TODO("TASK(20260913-080347)");
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
#if FLOATS_LD_KIND == FLOATS_LD_KIND_F64
  TODO("TASK(20260913-080353)");
#elif FLOATS_LD_KIND == FLOATS_LD_KIND_F80
  TODO("TASK(20260913-080353)");
#elif FLOATS_LD_KIND == FLOATS_LD_KIND_F128
  return FLOATS_MEMORYCOPY(value, f128_canonical_t);
#elif FLOATS_LD_KIND == FLOATS_LD_KIND_F64PAIR
  TODO("TASK(20260913-080353)");
#else
#  error "Unsupported or unknown long double architecture."
#endif
  UNUSED(value);
}

f128_canonical_t f128_canonical_from_f16_canonical(f16_canonical_t value) {
#if defined(__FLT16_MAX__) && FLOATS_LD_KIND == FLOATS_LD_KIND_F128
  return f128_canonical_from_long_double(FLOATS_MEMORYCOPY(value, _Float16));
#else
  TODO("TASK(20260913-080414)");
#endif
  UNUSED(value);
}

f128_canonical_t f128_canonical_from_f80_canonical(f80_canonical_t value) {
  TODO("TASK(20260913-080420)");
  UNUSED(value);
}

f128_canonical_t f128_canonical_from_f64pair_canonical(f64pair_canonical_t value) {
  TODO("TASK(20260913-080424)");
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
#if FLOATS_LD_KIND == FLOATS_LD_KIND_F128
  return FLOATS_MEMORYCOPY(value, long double);
#else
  TODO("TASK(20260913-080430)");
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
#if FLOATS_LD_KIND == FLOATS_LD_KIND_F64
  TODO("TASK(20260913-080437)");
#elif FLOATS_LD_KIND == FLOATS_LD_KIND_F80
  TODO("TASK(20260913-080437)");
#elif FLOATS_LD_KIND == FLOATS_LD_KIND_F128
  TODO("TASK(20260913-080437)");
#elif FLOATS_LD_KIND == FLOATS_LD_KIND_F64PAIR
  return FLOATS_MEMORYCOPY(value, f64pair_canonical_t);
#else
#  error "Unsupported or unknown long double architecture."
#endif
  UNUSED(value);
}

f64pair_canonical_t f64pair_canonical_from_f16_canonical(f16_canonical_t value) {
#if defined(__FLT16_MAX__) && FLOATS_LD_KIND == FLOATS_LD_KIND_F64PAIR
  return f64pair_canonical_from_long_double(FLOATS_MEMORYCOPY(value, _Float16));
#else
  TODO("TASK(20260913-080443)");
#endif
  UNUSED(value);
}

f64pair_canonical_t f64pair_canonical_from_f80_canonical(f80_canonical_t value) {
  TODO("TASK(20260913-080448)");
  UNUSED(value);
}

f64pair_canonical_t f64pair_canonical_from_f128_canonical(f128_canonical_t value) {
  TODO("TASK(20260913-080453)");
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
#if FLOATS_LD_KIND == FLOATS_LD_KIND_F64PAIR
  return FLOATS_MEMORYCOPY(value, long double);
#else
  TODO("TASK(20260913-080500)");
#endif
  UNUSED(value);
}

uint16_t f16_to_bits(f16_t value) {
#if defined(__FLT16_MAX__)
  return FLOATS_MEMORYCOPY(value, uint16_t);
#else
  return value.bits;
#endif
}

uint32_t f32_to_bits(f32_t value) {
  return FLOATS_MEMORYCOPY(value, uint32_t);
}

uint64_t f64_to_bits(f64_t value) {
  return FLOATS_MEMORYCOPY(value, uint64_t);
}

__uint128_t f80_to_bits(f80_t value) {
#if FLOATS_LD_KIND == FLOATS_LD_KIND_F80
  return FLOATS_MEMORYCOPY(FLOATS_MEMORYCOPY(value, f80_canonical_t), __uint128_t);
#else
  return FLOATS_MEMORYCOPY(value, __uint128_t);
#endif
}

__uint128_t f128_to_bits(f128_t value) {
#if FLOATS_LD_KIND == FLOATS_LD_KIND_F128
  return FLOATS_MEMORYCOPY(value, __uint128_t);
#else
  return value.bits;
#endif
}

__uint128_t f64pair_to_bits(f64pair_t value) {
#if FLOATS_LD_KIND == FLOATS_LD_KIND_F64PAIR
  return FLOATS_MEMORYCOPY(value, __uint128_t);
#else
  return value.bits;
#endif
}

#ifdef FLOATS_IGNORE_F16_WARNINGS
#  define FLOATS_IGNORE_FLOATTIHF_WARNING
#  define FLOATS_IGNORE_FLOATUNTIHF_WARNING
#endif

#if !(defined(__has_builtin) && __has_builtin(__builtin_floattihf))
#  ifndef FLOATS_IGNORE_FLOATTIHF_WARNING
#    if defined(_MSC_VER)
#      pragma message("floats.h: ___floattihf fallback provided for int128 -> _Float16 conversion; this is double-rounded and not guaranteed correctly-rounded")
#    elif defined(__GNUC__) || defined(__clang__)
#      warning "___floattihf fallback provided for int128 -> _Float16 conversion; this is double-rounded and not guaranteed correctly-rounded"
#    endif
#  endif
_Float16 ___floattihf(__int128_t a) __asm__("___floattihf");
__attribute__((used, noinline, visibility("default"))) _Float16 ___floattihf(__int128_t a) {
  return (_Float16)(double)a;
}
#endif

#if !(defined(__has_builtin) && __has_builtin(__builtin_floatuntihf))
#  ifndef FLOATS_IGNORE_FLOATUNTIHF_WARNING
#    if defined(_MSC_VER)
#      pragma message("floats.h: ___floatuntihf fallback provided for uint128 -> _Float16 conversion; this is double-rounded and not guaranteed correctly-rounded")
#    elif defined(__GNUC__) || defined(__clang__)
#      warning "___floatuntihf fallback provided for uint128 -> _Float16 conversion; this is double-rounded and not guaranteed correctly-rounded"
#    endif
#  endif
_Float16 ___floatuntihf(__uint128_t a) __asm__("___floatuntihf");
__attribute__((used, noinline, visibility("default"))) _Float16 ___floatuntihf(__uint128_t a) {
  return (_Float16)(double)a;
}
#endif

#undef FLOATS_MEMORYCOPY

#endif // FLOATS_IMPL_C

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
