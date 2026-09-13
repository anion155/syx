// https://github.com/ulfjack/ryu
// Copyright 2018 Ulf Adams
//
// The contents of this file may be used under the terms of the Apache License,
// Version 2.0.
//
//    (See accompanying file LICENSE-Apache or copy at
//     http://www.apache.org/licenses/LICENSE-2.0)
//
// Alternatively, the contents of this file may be used under the terms of
// the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE-Boost or copy at
//     https://www.boost.org/LICENSE_1_0.txt)
//
// Unless required by applicable law or agreed to in writing, this software
// is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied.

// Runtime compiler options:
// -DRYU_DEBUG Generate verbose debugging output to stdout.
//
// -DRYU_ONLY_64_BIT_OPS Avoid using __uint128_t or 64-bit intrinsics. Slower,
//     depending on your compiler.
//
// -DRYU_OPTIMIZE_SIZE Use smaller lookup tables. Instead of storing every
//     required power of 5, only store every 26th entry, and compute
//     intermediate values with a multiplication. This reduces the lookup table
//     size by about 10x (only one case, and only double) at the cost of some
//     performance. Currently requires MSVC intrinsics.

#ifndef RYU_H
#define RYU_H

#include <floats.h>
#include <inttypes.h>

typedef struct floating_ieee_f32 {
  uint32_t mantissa;
  int32_t exponent;
  bool sign;
} floating_ieee_f32;

typedef struct floating_decimal_f32 {
  uint32_t mantissa;
  int32_t exponent;
} floating_decimal_f32;

typedef struct floating_ieee_f64 {
  uint64_t mantissa;
  int32_t exponent;
  bool sign;
} floating_ieee_f64;

typedef struct floating_decimal_f64 {
  uint64_t mantissa;
  int32_t exponent;
} floating_decimal_f64;

typedef struct floating_ieee_generic {
  __uint128_t mantissa;
  int64_t exponent;
  bool sign;
  const uint32_t mantissaBits;
  const uint32_t exponentBits;
  const bool explicitLeadingBit;
} floating_ieee_generic;

typedef struct floating_decimal_generic {
  __uint128_t mantissa;
  int64_t exponent;
} floating_decimal_generic;

#endif // RYU_H

#if defined(RYU_IMPL) && !defined(RYU_IMPL_C)
#define RYU_IMPL_C
#define RYU_COMMON_INCL
#define RYU_DIGIT_TABLE_INCL
#define RYU_D2S_INTRINSICS_INCL
#if defined(RYU_OPTIMIZE_SIZE)
#  define RYU_D2S_SMALL_TABLE_INCL
#else
#  define RYU_D2S_FULL_TABLE_INCL
#endif
#define RYU_D2S_IMPL
#define RYU_F2S_INTRINSICS_INCL
#if defined(RYU_FLOAT_FULL_TABLE)
#  define RYU_F2S_FULL_TABLE_INCL
#elif defined(RYU_OPTIMIZE_SIZE)
#  define RYU_D2S_SMALL_TABLE_INCL
#else
#  define RYU_D2S_FULL_TABLE_INCL
#endif
#define RYU_F2S_IMPL
#define RYU_GENERIC_INTRINSICS_IMPL
#define RYU_GENERIC_IMPL
#endif // RYU_IMPL_C

#if defined(RYU_COMMON_INCL) && !defined(RYU_COMMON_H)
#define RYU_COMMON_H

#include <assert.h>
#include <stdint.h>
#include <string.h>

#if defined(_M_IX86) || defined(_M_ARM)
#  define RYU_32_BIT_PLATFORM
#endif

// Returns the number of decimal digits in v, which must not contain more than 9 digits.
static inline uint32_t ryu_decimalLength_f32(const uint32_t v) {
  // Function precondition: v is not a 10-digit number.
  // (f2s: 9 digits are sufficient for round-tripping.)
  // (d2fixed: We print 9-digit blocks.)
  assert(v < 1000000000);
  if (v >= 100000000) {
    return 9;
  }
  if (v >= 10000000) {
    return 8;
  }
  if (v >= 1000000) {
    return 7;
  }
  if (v >= 100000) {
    return 6;
  }
  if (v >= 10000) {
    return 5;
  }
  if (v >= 1000) {
    return 4;
  }
  if (v >= 100) {
    return 3;
  }
  if (v >= 10) {
    return 2;
  }
  return 1;
}

// Returns e == 0 ? 1 : [log_2(5^e)]; requires 0 <= e <= 3528.
static inline int32_t ryu_log2pow5(const int32_t e) {
  // This approximation works up to the point that the multiplication overflows at e = 3529.
  // If the multiplication were done in 64 bits, it would fail at 5^4004 which is just greater
  // than 2^9297.
  assert(e >= 0);
  assert(e <= 3528);
  return (int32_t)((((uint32_t)e) * 1217359) >> 19);
}

// Returns e == 0 ? 1 : ceil(log_2(5^e)); requires 0 <= e <= 3528.
static inline int32_t ryu_pow5bits(const int32_t e) {
  // This approximation works up to the point that the multiplication overflows at e = 3529.
  // If the multiplication were done in 64 bits, it would fail at 5^4004 which is just greater
  // than 2^9297.
  assert(e >= 0);
  assert(e <= 3528);
  return (int32_t)(((((uint32_t)e) * 1217359) >> 19) + 1);
}

// Returns e == 0 ? 1 : ceil(log_2(5^e)); requires 0 <= e <= 3528.
static inline int32_t ryu_ceil_log2pow5(const int32_t e) {
  return ryu_log2pow5(e) + 1;
}

// Returns floor(log_10(2^e)); requires 0 <= e <= 1650.
static inline uint32_t ryu_log10Pow2(const int32_t e) {
  // The first value this approximation fails for is 2^1651 which is just greater than 10^297.
  assert(e >= 0);
  assert(e <= 1650);
  return (((uint32_t)e) * 78913) >> 18;
}

// Returns floor(log_10(5^e)); requires 0 <= e <= 2620.
static inline uint32_t ryu_log10Pow5(const int32_t e) {
  // The first value this approximation fails for is 5^2621 which is just greater than 10^1832.
  assert(e >= 0);
  assert(e <= 2620);
  return (((uint32_t)e) * 732923) >> 20;
}

static inline uint32_t ryu_f32_to_bits(const float f) {
  uint32_t bits = 0;
  memcpy(&bits, &f, sizeof(float));
  return bits;
}

static inline uint64_t ryu_f64_to_bits(const double d) {
  uint64_t bits = 0;
  memcpy(&bits, &d, sizeof(double));
  return bits;
}

#endif // RYU_COMMON_H

#if defined(RYU_DIGIT_TABLE_INCL) && !defined(RYU_DIGIT_TABLE_H)
#define RYU_DIGIT_TABLE_H

// A table of all two-digit numbers. This is used to speed up decimal digit
// generation by copying pairs of digits into the final output.
static const char RYU_DIGIT_TABLE[200] = {
    '0', '0', '0', '1', '0', '2', '0', '3', '0', '4', '0', '5', '0', '6', '0', '7', '0', '8', '0', '9',
    '1', '0', '1', '1', '1', '2', '1', '3', '1', '4', '1', '5', '1', '6', '1', '7', '1', '8', '1', '9',
    '2', '0', '2', '1', '2', '2', '2', '3', '2', '4', '2', '5', '2', '6', '2', '7', '2', '8', '2', '9',
    '3', '0', '3', '1', '3', '2', '3', '3', '3', '4', '3', '5', '3', '6', '3', '7', '3', '8', '3', '9',
    '4', '0', '4', '1', '4', '2', '4', '3', '4', '4', '4', '5', '4', '6', '4', '7', '4', '8', '4', '9',
    '5', '0', '5', '1', '5', '2', '5', '3', '5', '4', '5', '5', '5', '6', '5', '7', '5', '8', '5', '9',
    '6', '0', '6', '1', '6', '2', '6', '3', '6', '4', '6', '5', '6', '6', '6', '7', '6', '8', '6', '9',
    '7', '0', '7', '1', '7', '2', '7', '3', '7', '4', '7', '5', '7', '6', '7', '7', '7', '8', '7', '9',
    '8', '0', '8', '1', '8', '2', '8', '3', '8', '4', '8', '5', '8', '6', '8', '7', '8', '8', '8', '9',
    '9', '0', '9', '1', '9', '2', '9', '3', '9', '4', '9', '5', '9', '6', '9', '7', '9', '8', '9', '9'};

#endif // RYU_DIGIT_TABLE_H

#if defined(RYU_D2S_INTRINSICS_INCL) && !defined(RYU_D2S_INTRINSICS_H)
#define RYU_D2S_INTRINSICS_H

#include <assert.h>
#include <stdint.h>

// ABSL avoids __uint128_t on Win32 even if __SIZEOF_INT128__ is defined.
// Let's do the same for now.
#if defined(__SIZEOF_INT128__) && !defined(_MSC_VER) && !defined(RYU_ONLY_64_BIT_OPS)
#  define HAS_UINT128
#elif defined(_MSC_VER) && !defined(RYU_ONLY_64_BIT_OPS) && defined(_M_X64)
#  define HAS_64_BIT_INTRINSICS
#endif

#if defined(HAS_64_BIT_INTRINSICS)

#  include <intrin.h>

static inline uint64_t ryu_umul128(const uint64_t a, const uint64_t b, uint64_t *const productHi) {
  return _umul128(a, b, productHi);
}

// Returns the lower 64 bits of (hi*2^64 + lo) >> dist, with 0 < dist < 64.
static inline uint64_t ryu_shiftright128(const uint64_t lo, const uint64_t hi, const uint32_t dist) {
  // For the __shiftright128 intrinsic, the shift value is always
  // modulo 64.
  // In the current implementation of the double-precision version
  // of Ryu, the shift value is always < 64. (In the case
  // RYU_OPTIMIZE_SIZE == 0, the shift value is in the range [49, 58].
  // Otherwise in the range [2, 59].)
  // However, this function is now also called by s2d, which requires supporting
  // the larger shift range (TODO: what is the actual range?).
  // Check this here in case a future change requires larger shift
  // values. In this case this function needs to be adjusted.
  assert(dist < 64);
  return __shiftright128(lo, hi, (unsigned char)dist);
}

#else // defined(HAS_64_BIT_INTRINSICS)

static inline uint64_t ryu_umul128(const uint64_t a, const uint64_t b, uint64_t *const productHi) {
  // The casts here help MSVC to avoid calls to the __allmul library function.
  const uint32_t aLo = (uint32_t)a;
  const uint32_t aHi = (uint32_t)(a >> 32);
  const uint32_t bLo = (uint32_t)b;
  const uint32_t bHi = (uint32_t)(b >> 32);

  const uint64_t b00 = (uint64_t)aLo * bLo;
  const uint64_t b01 = (uint64_t)aLo * bHi;
  const uint64_t b10 = (uint64_t)aHi * bLo;
  const uint64_t b11 = (uint64_t)aHi * bHi;

  const uint32_t b00Lo = (uint32_t)b00;
  const uint32_t b00Hi = (uint32_t)(b00 >> 32);

  const uint64_t mid1 = b10 + b00Hi;
  const uint32_t mid1Lo = (uint32_t)(mid1);
  const uint32_t mid1Hi = (uint32_t)(mid1 >> 32);

  const uint64_t mid2 = b01 + mid1Lo;
  const uint32_t mid2Lo = (uint32_t)(mid2);
  const uint32_t mid2Hi = (uint32_t)(mid2 >> 32);

  const uint64_t pHi = b11 + mid1Hi + mid2Hi;
  const uint64_t pLo = ((uint64_t)mid2Lo << 32) | b00Lo;

  *productHi = pHi;
  return pLo;
}

static inline uint64_t ryu_shiftright128(const uint64_t lo, const uint64_t hi, const uint32_t dist) {
  // We don't need to handle the case dist >= 64 here (see above).
  assert(dist < 64);
  assert(dist > 0);
  return (hi << (64 - dist)) | (lo >> dist);
}

#endif // defined(HAS_64_BIT_INTRINSICS)

#if defined(RYU_32_BIT_PLATFORM)

// Returns the high 64 bits of the 128-bit product of a and b.
static inline uint64_t ryu_umulh(const uint64_t a, const uint64_t b) {
  // Reuse the ryu_umul128 implementation.
  // Optimizers will likely eliminate the instructions used to compute the
  // low part of the product.
  uint64_t hi;
  ryu_umul128(a, b, &hi);
  return hi;
}

// On 32-bit platforms, compilers typically generate calls to library
// functions for 64-bit divisions, even if the divisor is a constant.
//
// E.g.:
// https://bugs.llvm.org/show_bug.cgi?id=37932
// https://gcc.gnu.org/bugzilla/show_bug.cgi?id=17958
// https://gcc.gnu.org/bugzilla/show_bug.cgi?id=37443
//
// The functions here perform division-by-constant using multiplications
// in the same way as 64-bit compilers would do.
//
// NB:
// The multipliers and shift values are the ones generated by clang x64
// for expressions like x/5, x/10, etc.

static inline uint64_t ryu_div5(const uint64_t x) {
  return ryu_umulh(x, 0xCCCCCCCCCCCCCCCDu) >> 2;
}

static inline uint64_t ryu_div10(const uint64_t x) {
  return ryu_umulh(x, 0xCCCCCCCCCCCCCCCDu) >> 3;
}

static inline uint64_t ryu_div100(const uint64_t x) {
  return ryu_umulh(x >> 2, 0x28F5C28F5C28F5C3u) >> 2;
}

static inline uint64_t ryu_div1e8(const uint64_t x) {
  return ryu_umulh(x, 0xABCC77118461CEFDu) >> 26;
}

static inline uint64_t ryu_div1e9(const uint64_t x) {
  return ryu_umulh(x >> 9, 0x44B82FA09B5A53u) >> 11;
}

static inline uint32_t ryu_mod1e9(const uint64_t x) {
  // Avoid 64-bit math as much as possible.
  // Returning (uint32_t) (x - 1000000000 * ryu_div1e9(x)) would
  // perform 32x64-bit multiplication and 64-bit subtraction.
  // x and 1000000000 * ryu_div1e9(x) are guaranteed to differ by
  // less than 10^9, so their highest 32 bits must be identical,
  // so we can truncate both sides to uint32_t before subtracting.
  // We can also simplify (uint32_t) (1000000000 * ryu_div1e9(x)).
  // We can truncate before multiplying instead of after, as multiplying
  // the highest 32 bits of ryu_div1e9(x) can't affect the lowest 32 bits.
  return ((uint32_t)x) - 1000000000 * ((uint32_t)ryu_div1e9(x));
}

#else // defined(RYU_32_BIT_PLATFORM)

static inline uint64_t ryu_div5(const uint64_t x) {
  return x / 5;
}

static inline uint64_t ryu_div10(const uint64_t x) {
  return x / 10;
}

static inline uint64_t ryu_div100(const uint64_t x) {
  return x / 100;
}

static inline uint64_t ryu_div1e8(const uint64_t x) {
  return x / 100000000;
}

static inline uint64_t ryu_div1e9(const uint64_t x) {
  return x / 1000000000;
}

static inline uint32_t ryu_mod1e9(const uint64_t x) {
  return (uint32_t)(x - 1000000000 * ryu_div1e9(x));
}

#endif // defined(RYU_32_BIT_PLATFORM)

static inline uint32_t ryu_pow5Factor(uint64_t value) {
  const uint64_t m_inv_5 = 14757395258967641293u; // 5 * m_inv_5 = 1 (mod 2^64)
  const uint64_t n_div_5 = 3689348814741910323u;  // #{ n | n = 0 (mod 2^64) } = 2^64 / 5
  uint32_t count = 0;
  for (;;) {
    assert(value != 0);
    value *= m_inv_5;
    if (value > n_div_5)
      break;
    ++count;
  }
  return count;
}

// Returns true if value is divisible by 5^p.
static inline bool ryu_multipleOfPowerOf5(const uint64_t value, const uint32_t p) {
  // I tried a case distinction on p, but there was no performance difference.
  return ryu_pow5Factor(value) >= p;
}

// Returns true if value is divisible by 2^p.
static inline bool ryu_multipleOfPowerOf2(const uint64_t value, const uint32_t p) {
  assert(value != 0);
  assert(p < 64);
  // __builtin_ctzll doesn't appear to be faster here.
  return (value & ((1ull << p) - 1)) == 0;
}

// We need a 64x128-bit multiplication and a subsequent 128-bit shift.
// Multiplication:
//   The 64-bit factor is variable and passed in, the 128-bit factor comes
//   from a lookup table. We know that the 64-bit factor only has 55
//   significant bits (i.e., the 9 topmost bits are zeros). The 128-bit
//   factor only has 124 significant bits (i.e., the 4 topmost bits are
//   zeros).
// Shift:
//   In principle, the multiplication result requires 55 + 124 = 179 bits to
//   represent. However, we then shift this value to the right by j, which is
//   at least j >= 115, so the result is guaranteed to fit into 179 - 115 = 64
//   bits. This means that we only need the topmost 64 significant bits of
//   the 64x128-bit multiplication.
//
// There are several ways to do this:
// 1. Best case: the compiler exposes a 128-bit type.
//    We perform two 64x64-bit multiplications, add the higher 64 bits of the
//    lower result to the higher result, and shift by j - 64 bits.
//
//    We explicitly cast from 64-bit to 128-bit, so the compiler can tell
//    that these are only 64-bit inputs, and can map these to the best
//    possible sequence of assembly instructions.
//    x64 machines happen to have matching assembly instructions for
//    64x64-bit multiplications and 128-bit shifts.
//
// 2. Second best case: the compiler exposes intrinsics for the x64 assembly
//    instructions mentioned in 1.
//
// 3. We only have 64x64 bit instructions that return the lower 64 bits of
//    the result, i.e., we have to use plain C.
//    Our inputs are less than the full width, so we have three options:
//    a. Ignore this fact and just implement the intrinsics manually.
//    b. Split both into 31-bit pieces, which guarantees no internal overflow,
//       but requires extra work upfront (unless we change the lookup table).
//    c. Split only the first factor into 31-bit pieces, which also guarantees
//       no internal overflow, but requires extra work since the intermediate
//       results are not perfectly aligned.
#if defined(HAS_UINT128)

// Best case: use 128-bit type.
static inline uint64_t ryu_mulShift64(const uint64_t m, const uint64_t *const mul, const int32_t j) {
  const __uint128_t b0 = ((__uint128_t)m) * mul[0];
  const __uint128_t b2 = ((__uint128_t)m) * mul[1];
  return (uint64_t)(((b0 >> 64) + b2) >> (j - 64));
}

static inline uint64_t ryu_mulShiftAll64(const uint64_t m, const uint64_t *const mul, const int32_t j,
                                         uint64_t *const vp, uint64_t *const vm, const uint32_t mmShift) {
  //  m <<= 2;
  //  __uint128_t b0 = ((__uint128_t) m) * mul[0]; // 0
  //  __uint128_t b2 = ((__uint128_t) m) * mul[1]; // 64
  //
  //  __uint128_t hi = (b0 >> 64) + b2;
  //  __uint128_t lo = b0 & 0xffffffffffffffffull;
  //  __uint128_t factor = (((__uint128_t) mul[1]) << 64) + mul[0];
  //  __uint128_t vpLo = lo + (factor << 1);
  //  *vp = (uint64_t) ((hi + (vpLo >> 64)) >> (j - 64));
  //  __uint128_t vmLo = lo - (factor << mmShift);
  //  *vm = (uint64_t) ((hi + (vmLo >> 64) - (((__uint128_t) 1ull) << 64)) >> (j - 64));
  //  return (uint64_t) (hi >> (j - 64));
  *vp = ryu_mulShift64(4 * m + 2, mul, j);
  *vm = ryu_mulShift64(4 * m - 1 - mmShift, mul, j);
  return ryu_mulShift64(4 * m, mul, j);
}

#elif defined(HAS_64_BIT_INTRINSICS)

static inline uint64_t ryu_mulShift64(const uint64_t m, const uint64_t *const mul, const int32_t j) {
  // m is maximum 55 bits
  uint64_t high1;                                       // 128
  const uint64_t low1 = ryu_umul128(m, mul[1], &high1); // 64
  uint64_t high0;                                       // 64
  ryu_umul128(m, mul[0], &high0);                       // 0
  const uint64_t sum = high0 + low1;
  if (sum < high0) {
    ++high1; // overflow into high1
  }
  return ryu_shiftright128(sum, high1, j - 64);
}

static inline uint64_t ryu_mulShiftAll64(const uint64_t m, const uint64_t *const mul, const int32_t j,
                                         uint64_t *const vp, uint64_t *const vm, const uint32_t mmShift) {
  *vp = ryu_mulShift64(4 * m + 2, mul, j);
  *vm = ryu_mulShift64(4 * m - 1 - mmShift, mul, j);
  return ryu_mulShift64(4 * m, mul, j);
}

#else // !defined(HAS_UINT128) && !defined(HAS_64_BIT_INTRINSICS)

static inline uint64_t ryu_mulShift64(const uint64_t m, const uint64_t *const mul, const int32_t j) {
  // m is maximum 55 bits
  uint64_t high1;                                       // 128
  const uint64_t low1 = ryu_umul128(m, mul[1], &high1); // 64
  uint64_t high0;                                       // 64
  ryu_umul128(m, mul[0], &high0);                       // 0
  const uint64_t sum = high0 + low1;
  if (sum < high0) {
    ++high1; // overflow into high1
  }
  return ryu_shiftright128(sum, high1, j - 64);
}

// This is faster if we don't have a 64x64->128-bit multiplication.
static inline uint64_t ryu_mulShiftAll64(uint64_t m, const uint64_t *const mul, const int32_t j,
                                         uint64_t *const vp, uint64_t *const vm, const uint32_t mmShift) {
  m <<= 1;
  // m is maximum 55 bits
  uint64_t tmp;
  const uint64_t lo = ryu_umul128(m, mul[0], &tmp);
  uint64_t hi;
  const uint64_t mid = tmp + ryu_umul128(m, mul[1], &hi);
  hi += mid < tmp; // overflow into hi

  const uint64_t lo2 = lo + mul[0];
  const uint64_t mid2 = mid + mul[1] + (lo2 < lo);
  const uint64_t hi2 = hi + (mid2 < mid);
  *vp = ryu_shiftright128(mid2, hi2, (uint32_t)(j - 64 - 1));

  if (mmShift == 1) {
    const uint64_t lo3 = lo - mul[0];
    const uint64_t mid3 = mid - mul[1] - (lo3 > lo);
    const uint64_t hi3 = hi - (mid3 > mid);
    *vm = ryu_shiftright128(mid3, hi3, (uint32_t)(j - 64 - 1));
  } else {
    const uint64_t lo3 = lo + lo;
    const uint64_t mid3 = mid + mid + (lo3 < lo);
    const uint64_t hi3 = hi + hi + (mid3 < mid);
    const uint64_t lo4 = lo3 - mul[0];
    const uint64_t mid4 = mid3 - mul[1] - (lo4 > lo3);
    const uint64_t hi4 = hi3 - (mid4 > mid3);
    *vm = ryu_shiftright128(mid4, hi4, (uint32_t)(j - 64));
  }

  return ryu_shiftright128(mid, hi, (uint32_t)(j - 64 - 1));
}

#endif // HAS_64_BIT_INTRINSICS

#endif // RYU_D2S_INTRINSICS_H

#if defined(RYU_D2S_SMALL_TABLE_INCL) && !defined(RYU_D2S_SMALL_TABLE_H)
#define RYU_D2S_SMALL_TABLE_H

// These tables are generated by PrintDoubleLookupTable.
#define RYU_DOUBLE_POW5_INV_BITCOUNT 125
#define RYU_DOUBLE_POW5_BITCOUNT 125

static const uint64_t RYU_DOUBLE_POW5_INV_SPLIT2[15][2] = {
    {1u, 2305843009213693952u},
    {5955668970331000884u, 1784059615882449851u},
    {8982663654677661702u, 1380349269358112757u},
    {7286864317269821294u, 2135987035920910082u},
    {7005857020398200553u, 1652639921975621497u},
    {17965325103354776697u, 1278668206209430417u},
    {8928596168509315048u, 1978643211784836272u},
    {10075671573058298858u, 1530901034580419511u},
    {597001226353042382u, 1184477304306571148u},
    {1527430471115325346u, 1832889850782397517u},
    {12533209867169019542u, 1418129833677084982u},
    {5577825024675947042u, 2194449627517475473u},
    {11006974540203867551u, 1697873161311732311u},
    {10313493231639821582u, 1313665730009899186u},
    {12701016819766672773u, 2032799256770390445u}};
static const uint32_t RYU_POW5_INV_OFFSETS[19] = {
    0x54544554, 0x04055545, 0x10041000, 0x00400414, 0x40010000, 0x41155555,
    0x00000454, 0x00010044, 0x40000000, 0x44000041, 0x50454450, 0x55550054,
    0x51655554, 0x40004000, 0x01000001, 0x00010500, 0x51515411, 0x05555554,
    0x00000000};

static const uint64_t RYU_DOUBLE_POW5_SPLIT2[13][2] = {
    {0u, 1152921504606846976u},
    {0u, 1490116119384765625u},
    {1032610780636961552u, 1925929944387235853u},
    {7910200175544436838u, 1244603055572228341u},
    {16941905809032713930u, 1608611746708759036u},
    {13024893955298202172u, 2079081953128979843u},
    {6607496772837067824u, 1343575221513417750u},
    {17332926989895652603u, 1736530273035216783u},
    {13037379183483547984u, 2244412773384604712u},
    {1605989338741628675u, 1450417759929778918u},
    {9630225068416591280u, 1874621017369538693u},
    {665883850346957067u, 1211445438634777304u},
    {14931890668723713708u, 1565756531257009982u}};
static const uint32_t RYU_POW5_OFFSETS[21] = {
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x40000000, 0x59695995,
    0x55545555, 0x56555515, 0x41150504, 0x40555410, 0x44555145, 0x44504540,
    0x45555550, 0x40004000, 0x96440440, 0x55565565, 0x54454045, 0x40154151,
    0x55559155, 0x51405555, 0x00000105};

#define RYU_POW5_TABLE_SIZE 26
static const uint64_t RYU_DOUBLE_POW5_TABLE[RYU_POW5_TABLE_SIZE] = {
    1ull, 5ull, 25ull, 125ull, 625ull, 3125ull, 15625ull, 78125ull, 390625ull,
    1953125ull, 9765625ull, 48828125ull, 244140625ull, 1220703125ull, 6103515625ull,
    30517578125ull, 152587890625ull, 762939453125ull, 3814697265625ull,
    19073486328125ull, 95367431640625ull, 476837158203125ull,
    2384185791015625ull, 11920928955078125ull, 59604644775390625ull,
    298023223876953125ull //, 1490116119384765625ull
};

#if defined(HAS_UINT128)

// Computes 5^i in the form required by Ryu, and stores it in the given pointer.
static inline void ryu_double_computePow5(const uint32_t i, uint64_t *const result) {
  const uint32_t base = i / RYU_POW5_TABLE_SIZE;
  const uint32_t base2 = base * RYU_POW5_TABLE_SIZE;
  const uint32_t offset = i - base2;
  const uint64_t *const mul = RYU_DOUBLE_POW5_SPLIT2[base];
  if (offset == 0) {
    result[0] = mul[0];
    result[1] = mul[1];
    return;
  }
  const uint64_t m = RYU_DOUBLE_POW5_TABLE[offset];
  const __uint128_t b0 = ((__uint128_t)m) * mul[0];
  const __uint128_t b2 = ((__uint128_t)m) * mul[1];
  const uint32_t delta = ryu_pow5bits(i) - ryu_pow5bits(base2);
  const __uint128_t shiftedSum = (b0 >> delta) + (b2 << (64 - delta)) + ((RYU_POW5_OFFSETS[i / 16] >> ((i % 16) << 1)) & 3);
  result[0] = (uint64_t)shiftedSum;
  result[1] = (uint64_t)(shiftedSum >> 64);
}

// Computes 5^-i in the form required by Ryu, and stores it in the given pointer.
static inline void ryu_double_computeInvPow5(const uint32_t i, uint64_t *const result) {
  const uint32_t base = (i + RYU_POW5_TABLE_SIZE - 1) / RYU_POW5_TABLE_SIZE;
  const uint32_t base2 = base * RYU_POW5_TABLE_SIZE;
  const uint32_t offset = base2 - i;
  const uint64_t *const mul = RYU_DOUBLE_POW5_INV_SPLIT2[base]; // 1/5^base2
  if (offset == 0) {
    result[0] = mul[0];
    result[1] = mul[1];
    return;
  }
  const uint64_t m = RYU_DOUBLE_POW5_TABLE[offset]; // 5^offset
  const __uint128_t b0 = ((__uint128_t)m) * (mul[0] - 1);
  const __uint128_t b2 = ((__uint128_t)m) * mul[1]; // 1/5^base2 * 5^offset = 1/5^(base2-offset) = 1/5^i
  const uint32_t delta = ryu_pow5bits(base2) - ryu_pow5bits(i);
  const __uint128_t shiftedSum =
      ((b0 >> delta) + (b2 << (64 - delta))) + 1 + ((RYU_POW5_INV_OFFSETS[i / 16] >> ((i % 16) << 1)) & 3);
  result[0] = (uint64_t)shiftedSum;
  result[1] = (uint64_t)(shiftedSum >> 64);
}

#else // defined(HAS_UINT128)

// Computes 5^i in the form required by Ryu, and stores it in the given pointer.
static inline void ryu_double_computePow5(const uint32_t i, uint64_t *const result) {
  const uint32_t base = i / RYU_POW5_TABLE_SIZE;
  const uint32_t base2 = base * RYU_POW5_TABLE_SIZE;
  const uint32_t offset = i - base2;
  const uint64_t *const mul = RYU_DOUBLE_POW5_SPLIT2[base];
  if (offset == 0) {
    result[0] = mul[0];
    result[1] = mul[1];
    return;
  }
  const uint64_t m = RYU_DOUBLE_POW5_TABLE[offset];
  uint64_t high1;
  const uint64_t low1 = ryu_umul128(m, mul[1], &high1);
  uint64_t high0;
  const uint64_t low0 = ryu_umul128(m, mul[0], &high0);
  const uint64_t sum = high0 + low1;
  if (sum < high0) {
    ++high1; // overflow into high1
  }
  // high1 | sum | low0
  const uint32_t delta = ryu_pow5bits(i) - ryu_pow5bits(base2);
  result[0] = ryu_shiftright128(low0, sum, delta) + ((RYU_POW5_OFFSETS[i / 16] >> ((i % 16) << 1)) & 3);
  result[1] = ryu_shiftright128(sum, high1, delta);
}

// Computes 5^-i in the form required by Ryu, and stores it in the given pointer.
static inline void ryu_double_computeInvPow5(const uint32_t i, uint64_t *const result) {
  const uint32_t base = (i + RYU_POW5_TABLE_SIZE - 1) / RYU_POW5_TABLE_SIZE;
  const uint32_t base2 = base * RYU_POW5_TABLE_SIZE;
  const uint32_t offset = base2 - i;
  const uint64_t *const mul = RYU_DOUBLE_POW5_INV_SPLIT2[base]; // 1/5^base2
  if (offset == 0) {
    result[0] = mul[0];
    result[1] = mul[1];
    return;
  }
  const uint64_t m = RYU_DOUBLE_POW5_TABLE[offset];
  uint64_t high1;
  const uint64_t low1 = ryu_umul128(m, mul[1], &high1);
  uint64_t high0;
  const uint64_t low0 = ryu_umul128(m, mul[0] - 1, &high0);
  const uint64_t sum = high0 + low1;
  if (sum < high0) {
    ++high1; // overflow into high1
  }
  // high1 | sum | low0
  const uint32_t delta = ryu_pow5bits(base2) - ryu_pow5bits(i);
  result[0] = ryu_shiftright128(low0, sum, delta) + 1 + ((RYU_POW5_INV_OFFSETS[i / 16] >> ((i % 16) << 1)) & 3);
  result[1] = ryu_shiftright128(sum, high1, delta);
}

#endif // defined(HAS_UINT128)

#endif // RYU_D2S_SMALL_TABLE_H

#if defined(RYU_D2S_FULL_TABLE_INCL) && !defined(RYU_D2S_FULL_TABLE_H)
#define RYU_D2S_FULL_TABLE_H

// These tables are generated by PrintDoubleLookupTable.
#define RYU_DOUBLE_POW5_INV_BITCOUNT 125
#define RYU_DOUBLE_POW5_BITCOUNT 125

#define RYU_DOUBLE_POW5_INV_TABLE_SIZE 342
#define RYU_DOUBLE_RYU_POW5_TABLE_SIZE 326

static const uint64_t RYU_DOUBLE_POW5_INV_SPLIT[RYU_DOUBLE_POW5_INV_TABLE_SIZE][2] = {
    {1u, 2305843009213693952u}, {11068046444225730970u, 1844674407370955161u}, {5165088340638674453u, 1475739525896764129u}, {7821419487252849886u, 1180591620717411303u}, {8824922364862649494u, 1888946593147858085u}, {7059937891890119595u, 1511157274518286468u}, {13026647942995916322u, 1208925819614629174u}, {9774590264567735146u, 1934281311383406679u}, {11509021026396098440u, 1547425049106725343u}, {16585914450600699399u, 1237940039285380274u}, {15469416676735388068u, 1980704062856608439u}, {16064882156130220778u, 1584563250285286751u}, {9162556910162266299u, 1267650600228229401u}, {7281393426775805432u, 2028240960365167042u}, {16893161185646375315u, 1622592768292133633u}, {2446482504291369283u, 1298074214633706907u}, {7603720821608101175u, 2076918743413931051u}, {2393627842544570617u, 1661534994731144841u}, {16672297533003297786u, 1329227995784915872u}, {11918280793837635165u, 2126764793255865396u}, {5845275820328197809u, 1701411834604692317u}, {15744267100488289217u, 1361129467683753853u}, {3054734472329800808u, 2177807148294006166u}, {17201182836831481939u, 1742245718635204932u}, {6382248639981364905u, 1393796574908163946u}, {2832900194486363201u, 2230074519853062314u}, {5955668970331000884u, 1784059615882449851u}, {1075186361522890384u, 1427247692705959881u}, {12788344622662355584u, 2283596308329535809u}, {13920024512871794791u, 1826877046663628647u}, {3757321980813615186u, 1461501637330902918u}, {10384555214134712795u, 1169201309864722334u}, {5547241898389809503u, 1870722095783555735u}, {4437793518711847602u, 1496577676626844588u}, {10928932444453298728u, 1197262141301475670u}, {17486291911125277965u, 1915619426082361072u}, {6610335899416401726u, 1532495540865888858u}, {12666966349016942027u, 1225996432692711086u}, {12888448528943286597u, 1961594292308337738u}, {17689456452638449924u, 1569275433846670190u}, {14151565162110759939u, 1255420347077336152u}, {7885109000409574610u, 2008672555323737844u}, {9997436015069570011u, 1606938044258990275u}, {7997948812055656009u, 1285550435407192220u}, {12796718099289049614u, 2056880696651507552u}, {2858676849947419045u, 1645504557321206042u}, {13354987924183666206u, 1316403645856964833u}, {17678631863951955605u, 2106245833371143733u}, {3074859046935833515u, 1684996666696914987u}, {13527933681774397782u, 1347997333357531989u}, {10576647446613305481u, 2156795733372051183u}, {15840015586774465031u, 1725436586697640946u}, {8982663654677661702u, 1380349269358112757u}, {18061610662226169046u, 2208558830972980411u}, {10759939715039024913u, 1766847064778384329u}, {12297300586773130254u, 1413477651822707463u}, {15986332124095098083u, 2261564242916331941u}, {9099716884534168143u, 1809251394333065553u}, {14658471137111155161u, 1447401115466452442u}, {4348079280205103483u, 1157920892373161954u}, {14335624477811986218u, 1852673427797059126u}, {7779150767507678651u, 1482138742237647301u}, {2533971799264232598u, 1185710993790117841u}, {15122401323048503126u, 1897137590064188545u}, {12097921058438802501u, 1517710072051350836u}, {5988988032009131678u, 1214168057641080669u}, {16961078480698431330u, 1942668892225729070u}, {13568862784558745064u, 1554135113780583256u}, {7165741412905085728u, 1243308091024466605u}, {11465186260648137165u, 1989292945639146568u}, {16550846638002330379u, 1591434356511317254u}, {16930026125143774626u, 1273147485209053803u}, {4951948911778577463u, 2037035976334486086u}, {272210314680951647u, 1629628781067588869u}, {3907117066486671641u, 1303703024854071095u}, {6251387306378674625u, 2085924839766513752u}, {16069156289328670670u, 1668739871813211001u}, {9165976216721026213u, 1334991897450568801u}, {7286864317269821294u, 2135987035920910082u}, {16897537898041588005u, 1708789628736728065u}, {13518030318433270404u, 1367031702989382452u}, {6871453250525591353u, 2187250724783011924u}, {9186511415162383406u, 1749800579826409539u}, {11038557946871817048u, 1399840463861127631u}, {10282995085511086630u, 2239744742177804210u}, {8226396068408869304u, 1791795793742243368u}, {13959814484210916090u, 1433436634993794694u}, {11267656730511734774u, 2293498615990071511u}, {5324776569667477496u, 1834798892792057209u}, {7949170070475892320u, 1467839114233645767u}, {17427382500606444826u, 1174271291386916613u}, {5747719112518849781u, 1878834066219066582u}, {15666221734240810795u, 1503067252975253265u}, {12532977387392648636u, 1202453802380202612u}, {5295368560860596524u, 1923926083808324180u}, {4236294848688477220u, 1539140867046659344u}, {7078384693692692099u, 1231312693637327475u}, {11325415509908307358u, 1970100309819723960u}, {9060332407926645887u, 1576080247855779168u}, {14626963555825137356u, 1260864198284623334u}, {12335095245094488799u, 2017382717255397335u}, {9868076196075591040u, 1613906173804317868u}, {15273158586344293478u, 1291124939043454294u}, {13369007293925138595u, 2065799902469526871u}, {7005857020398200553u, 1652639921975621497u}, {16672732060544291412u, 1322111937580497197u}, {11918976037903224966u, 2115379100128795516u}, {5845832015580669650u, 1692303280103036413u}, {12055363241948356366u, 1353842624082429130u}, {841837113407818570u, 2166148198531886609u}, {4362818505468165179u, 1732918558825509287u}, {14558301248600263113u, 1386334847060407429u}, {12225235553534690011u, 2218135755296651887u}, {2401490813343931363u, 1774508604237321510u}, {1921192650675145090u, 1419606883389857208u}, {17831303500047873437u, 2271371013423771532u}, {6886345170554478103u, 1817096810739017226u}, {1819727321701672159u, 1453677448591213781u}, {16213177116328979020u, 1162941958872971024u}, {14873036941900635463u, 1860707134196753639u}, {15587778368262418694u, 1488565707357402911u}, {8780873879868024632u, 1190852565885922329u}, {2981351763563108441u, 1905364105417475727u}, {13453127855076217722u, 1524291284333980581u}, {7073153469319063855u, 1219433027467184465u}, {11317045550910502167u, 1951092843947495144u}, {12742985255470312057u, 1560874275157996115u}, {10194388204376249646u, 1248699420126396892u}, {1553625868034358140u, 1997919072202235028u}, {8621598323911307159u, 1598335257761788022u}, {17965325103354776697u, 1278668206209430417u}, {13987124906400001422u, 2045869129935088668u}, {121653480894270168u, 1636695303948070935u}, {97322784715416134u, 1309356243158456748u}, {14913111714512307107u, 2094969989053530796u}, {8241140556867935363u, 1675975991242824637u}, {17660958889720079260u, 1340780792994259709u}, {17189487779326395846u, 2145249268790815535u}, {13751590223461116677u, 1716199415032652428u}, {18379969808252713988u, 1372959532026121942u}, {14650556434236701088u, 2196735251241795108u}, {652398703163629901u, 1757388200993436087u}, {11589965406756634890u, 1405910560794748869u}, {7475898206584884855u, 2249456897271598191u}, {2291369750525997561u, 1799565517817278553u}, {9211793429904618695u, 1439652414253822842u}, {18428218302589300235u, 2303443862806116547u}, {7363877012587619542u, 1842755090244893238u}, {13269799239553916280u, 1474204072195914590u}, {10615839391643133024u, 1179363257756731672u}, {2227947767661371545u, 1886981212410770676u}, {16539753473096738529u, 1509584969928616540u}, {13231802778477390823u, 1207667975942893232u}, {6413489186596184024u, 1932268761508629172u}, {16198837793502678189u, 1545815009206903337u}, {5580372605318321905u, 1236652007365522670u}, {8928596168509315048u, 1978643211784836272u}, {18210923379033183008u, 1582914569427869017u}, {7190041073742725760u, 1266331655542295214u}, {436019273762630246u, 2026130648867672343u}, {7727513048493924843u, 1620904519094137874u}, {9871359253537050198u, 1296723615275310299u}, {4726128361433549347u, 2074757784440496479u}, {7470251503888749801u, 1659806227552397183u}, {13354898832594820487u, 1327844982041917746u}, {13989140502667892133u, 2124551971267068394u}, {14880661216876224029u, 1699641577013654715u}, {11904528973500979224u, 1359713261610923772u}, {4289851098633925465u, 2175541218577478036u}, {18189276137874781665u, 1740432974861982428u}, {3483374466074094362u, 1392346379889585943u}, {1884050330976640656u, 2227754207823337509u}, {5196589079523222848u, 1782203366258670007u}, {15225317707844309248u, 1425762693006936005u}, {5913764258841343181u, 2281220308811097609u}, {8420360221814984868u, 1824976247048878087u}, {17804334621677718864u, 1459980997639102469u}, {17932816512084085415u, 1167984798111281975u}, {10245762345624985047u, 1868775676978051161u}, {4507261061758077715u, 1495020541582440929u}, {7295157664148372495u, 1196016433265952743u}, {7982903447895485668u, 1913626293225524389u}, {10075671573058298858u, 1530901034580419511u}, {4371188443704728763u, 1224720827664335609u}, {14372599139411386667u, 1959553324262936974u}, {15187428126271019657u, 1567642659410349579u}, {15839291315758726049u, 1254114127528279663u}, {3206773216762499739u, 2006582604045247462u}, {13633465017635730761u, 1605266083236197969u}, {14596120828850494932u, 1284212866588958375u}, {4907049252451240275u, 2054740586542333401u}, {236290587219081897u, 1643792469233866721u}, {14946427728742906810u, 1315033975387093376u}, {16535586736504830250u, 2104054360619349402u}, {5849771759720043554u, 1683243488495479522u}, {15747863852001765813u, 1346594790796383617u}, {10439186904235184007u, 2154551665274213788u}, {15730047152871967852u, 1723641332219371030u}, {12584037722297574282u, 1378913065775496824u}, {9066413911450387881u, 2206260905240794919u}, {10942479943902220628u, 1765008724192635935u}, {8753983955121776503u, 1412006979354108748u}, {10317025513452932081u, 2259211166966573997u}, {874922781278525018u, 1807368933573259198u}, {8078635854506640661u, 1445895146858607358u}, {13841606313089133175u, 1156716117486885886u}, {14767872471458792434u, 1850745787979017418u}, {746251532941302978u, 1480596630383213935u}, {597001226353042382u, 1184477304306571148u}, {15712597221132509104u, 1895163686890513836u}, {8880728962164096960u, 1516130949512411069u}, {10793931984473187891u, 1212904759609928855u}, {17270291175157100626u, 1940647615375886168u}, {2748186495899949531u, 1552518092300708935u}, {2198549196719959625u, 1242014473840567148u}, {18275073973719576693u, 1987223158144907436u}, {10930710364233751031u, 1589778526515925949u}, {12433917106128911148u, 1271822821212740759u}, {8826220925580526867u, 2034916513940385215u}, {7060976740464421494u, 1627933211152308172u}, {16716827836597268165u, 1302346568921846537u}, {11989529279587987770u, 2083754510274954460u}, {9591623423670390216u, 1667003608219963568u}, {15051996368420132820u, 1333602886575970854u}, {13015147745246481542u, 2133764618521553367u}, {3033420566713364587u, 1707011694817242694u}, {6116085268112601993u, 1365609355853794155u}, {9785736428980163188u, 2184974969366070648u}, {15207286772667951197u, 1747979975492856518u}, {1097782973908629988u, 1398383980394285215u}, {1756452758253807981u, 2237414368630856344u}, {5094511021344956708u, 1789931494904685075u}, {4075608817075965366u, 1431945195923748060u}, {6520974107321544586u, 2291112313477996896u}, {1527430471115325346u, 1832889850782397517u}, {12289990821117991246u, 1466311880625918013u}, {17210690286378213644u, 1173049504500734410u}, {9090360384495590213u, 1876879207201175057u}, {18340334751822203140u, 1501503365760940045u}, {14672267801457762512u, 1201202692608752036u}, {16096930852848599373u, 1921924308174003258u}, {1809498238053148529u, 1537539446539202607u}, {12515645034668249793u, 1230031557231362085u}, {1578287981759648052u, 1968050491570179337u}, {12330676829633449412u, 1574440393256143469u}, {13553890278448669853u, 1259552314604914775u}, {3239480371808320148u, 2015283703367863641u}, {17348979556414297411u, 1612226962694290912u}, {6500486015647617283u, 1289781570155432730u}, {10400777625036187652u, 2063650512248692368u}, {15699319729512770768u, 1650920409798953894u}, {16248804598352126938u, 1320736327839163115u}, {7551343283653851484u, 2113178124542660985u}, {6041074626923081187u, 1690542499634128788u}, {12211557331022285596u, 1352433999707303030u}, {1091747655926105338u, 2163894399531684849u}, {4562746939482794594u, 1731115519625347879u}, {7339546366328145998u, 1384892415700278303u}, {8053925371383123274u, 2215827865120445285u}, {6443140297106498619u, 1772662292096356228u}, {12533209867169019542u, 1418129833677084982u}, {5295740528502789974u, 2269007733883335972u}, {15304638867027962949u, 1815206187106668777u}, {4865013464138549713u, 1452164949685335022u}, {14960057215536570740u, 1161731959748268017u}, {9178696285890871890u, 1858771135597228828u}, {14721654658196518159u, 1487016908477783062u}, {4398626097073393881u, 1189613526782226450u}, {7037801755317430209u, 1903381642851562320u}, {5630241404253944167u, 1522705314281249856u}, {814844308661245011u, 1218164251424999885u}, {1303750893857992017u, 1949062802279999816u}, {15800395974054034906u, 1559250241823999852u}, {5261619149759407279u, 1247400193459199882u}, {12107939454356961969u, 1995840309534719811u}, {5997002748743659252u, 1596672247627775849u}, {8486951013736837725u, 1277337798102220679u}, {2511075177753209390u, 2043740476963553087u}, {13076906586428298482u, 1634992381570842469u}, {14150874083884549109u, 1307993905256673975u}, {4194654460505726958u, 2092790248410678361u}, {18113118827372222859u, 1674232198728542688u}, {3422448617672047318u, 1339385758982834151u}, {16543964232501006678u, 2143017214372534641u}, {9545822571258895019u, 1714413771498027713u}, {15015355686490936662u, 1371531017198422170u}, {5577825024675947042u, 2194449627517475473u}, {11840957649224578280u, 1755559702013980378u}, {16851463748863483271u, 1404447761611184302u}, {12204946739213931940u, 2247116418577894884u}, {13453306206113055875u, 1797693134862315907u}, {3383947335406624054u, 1438154507889852726u}, {16482362180876329456u, 2301047212623764361u}, {9496540929959153242u, 1840837770099011489u}, {11286581558709232917u, 1472670216079209191u}, {5339916432225476010u, 1178136172863367353u}, {4854517476818851293u, 1885017876581387765u}, {3883613981455081034u, 1508014301265110212u}, {14174937629389795797u, 1206411441012088169u}, {11611853762797942306u, 1930258305619341071u}, {5600134195496443521u, 1544206644495472857u}, {15548153800622885787u, 1235365315596378285u}, {6430302007287065643u, 1976584504954205257u}, {16212288050055383484u, 1581267603963364205u}, {12969830440044306787u, 1265014083170691364u}, {9683682259845159889u, 2024022533073106183u}, {15125643437359948558u, 1619218026458484946u}, {8411165935146048523u, 1295374421166787957u}, {17147214310975587960u, 2072599073866860731u}, {10028422634038560045u, 1658079259093488585u}, {8022738107230848036u, 1326463407274790868u}, {9147032156827446534u, 2122341451639665389u}, {11006974540203867551u, 1697873161311732311u}, {5116230817421183718u, 1358298529049385849u}, {15564666937357714594u, 2173277646479017358u}, {1383687105660440706u, 1738622117183213887u}, {12174996128754083534u, 1390897693746571109u}, {8411947361780802685u, 2225436309994513775u}, {6729557889424642148u, 1780349047995611020u}, {5383646311539713719u, 1424279238396488816u}, {1235136468979721303u, 2278846781434382106u}, {15745504434151418335u, 1823077425147505684u}, {16285752362063044992u, 1458461940118004547u}, {5649904260166615347u, 1166769552094403638u}, {5350498001524674232u, 1866831283351045821u}, {591049586477829062u, 1493465026680836657u}, {11540886113407994219u, 1194772021344669325u}, {18673707743239135u, 1911635234151470921u}, {14772334225162232601u, 1529308187321176736u}, {8128518565387875758u, 1223446549856941389u}, {1937583260394870242u, 1957514479771106223u}, {8928764237799716840u, 1566011583816884978u}, {14521709019723594119u, 1252809267053507982u}, {8477339172590109297u, 2004494827285612772u}, {17849917782297818407u, 1603595861828490217u}, {6901236596354434079u, 1282876689462792174u}, {18420676183650915173u, 2052602703140467478u}, {3668494502695001169u, 1642082162512373983u}, {10313493231639821582u, 1313665730009899186u}, {9122891541139893884u, 2101865168015838698u}, {14677010862395735754u, 1681492134412670958u}, {673562245690857633u, 1345193707530136767u}};

static const uint64_t RYU_DOUBLE_POW5_SPLIT[RYU_DOUBLE_RYU_POW5_TABLE_SIZE][2] = {
    {0u, 1152921504606846976u}, {0u, 1441151880758558720u}, {0u, 1801439850948198400u}, {0u, 2251799813685248000u}, {0u, 1407374883553280000u}, {0u, 1759218604441600000u}, {0u, 2199023255552000000u}, {0u, 1374389534720000000u}, {0u, 1717986918400000000u}, {0u, 2147483648000000000u}, {0u, 1342177280000000000u}, {0u, 1677721600000000000u}, {0u, 2097152000000000000u}, {0u, 1310720000000000000u}, {0u, 1638400000000000000u}, {0u, 2048000000000000000u}, {0u, 1280000000000000000u}, {0u, 1600000000000000000u}, {0u, 2000000000000000000u}, {0u, 1250000000000000000u}, {0u, 1562500000000000000u}, {0u, 1953125000000000000u}, {0u, 1220703125000000000u}, {0u, 1525878906250000000u}, {0u, 1907348632812500000u}, {0u, 1192092895507812500u}, {0u, 1490116119384765625u}, {4611686018427387904u, 1862645149230957031u}, {9799832789158199296u, 1164153218269348144u}, {12249790986447749120u, 1455191522836685180u}, {15312238733059686400u, 1818989403545856475u}, {14528612397897220096u, 2273736754432320594u}, {13692068767113150464u, 1421085471520200371u}, {12503399940464050176u, 1776356839400250464u}, {15629249925580062720u, 2220446049250313080u}, {9768281203487539200u, 1387778780781445675u}, {7598665485932036096u, 1734723475976807094u}, {274959820560269312u, 2168404344971008868u}, {9395221924704944128u, 1355252715606880542u}, {2520655369026404352u, 1694065894508600678u}, {12374191248137781248u, 2117582368135750847u}, {14651398557727195136u, 1323488980084844279u}, {13702562178731606016u, 1654361225106055349u}, {3293144668132343808u, 2067951531382569187u}, {18199116482078572544u, 1292469707114105741u}, {8913837547316051968u, 1615587133892632177u}, {15753982952572452864u, 2019483917365790221u}, {12152082354571476992u, 1262177448353618888u}, {15190102943214346240u, 1577721810442023610u}, {9764256642163156992u, 1972152263052529513u}, {17631875447420442880u, 1232595164407830945u}, {8204786253993389888u, 1540743955509788682u}, {1032610780636961552u, 1925929944387235853u}, {2951224747111794922u, 1203706215242022408u}, {3689030933889743652u, 1504632769052528010u}, {13834660704216955373u, 1880790961315660012u}, {17870034976990372916u, 1175494350822287507u}, {17725857702810578241u, 1469367938527859384u}, {3710578054803671186u, 1836709923159824231u}, {26536550077201078u, 2295887403949780289u}, {11545800389866720434u, 1434929627468612680u}, {14432250487333400542u, 1793662034335765850u}, {8816941072311974870u, 2242077542919707313u}, {17039803216263454053u, 1401298464324817070u}, {12076381983474541759u, 1751623080406021338u}, {5872105442488401391u, 2189528850507526673u}, {15199280947623720629u, 1368455531567204170u}, {9775729147674874978u, 1710569414459005213u}, {16831347453020981627u, 2138211768073756516u}, {1296220121283337709u, 1336382355046097823u}, {15455333206886335848u, 1670477943807622278u}, {10095794471753144002u, 2088097429759527848u}, {6309871544845715001u, 1305060893599704905u}, {12499025449484531656u, 1631326116999631131u}, {11012095793428276666u, 2039157646249538914u}, {11494245889320060820u, 1274473528905961821u}, {532749306367912313u, 1593091911132452277u}, {5277622651387278295u, 1991364888915565346u}, {7910200175544436838u, 1244603055572228341u}, {14499436237857933952u, 1555753819465285426u}, {8900923260467641632u, 1944692274331606783u}, {12480606065433357876u, 1215432671457254239u}, {10989071563364309441u, 1519290839321567799u}, {9124653435777998898u, 1899113549151959749u}, {8008751406574943263u, 1186945968219974843u}, {5399253239791291175u, 1483682460274968554u}, {15972438586593889776u, 1854603075343710692u}, {759402079766405302u, 1159126922089819183u}, {14784310654990170340u, 1448908652612273978u}, {9257016281882937117u, 1811135815765342473u}, {16182956370781059300u, 2263919769706678091u}, {7808504722524468110u, 1414949856066673807u}, {5148944884728197234u, 1768687320083342259u}, {1824495087482858639u, 2210859150104177824u}, {1140309429676786649u, 1381786968815111140u}, {1425386787095983311u, 1727233711018888925u}, {6393419502297367043u, 2159042138773611156u}, {13219259225790630210u, 1349401336733506972u}, {16524074032238287762u, 1686751670916883715u}, {16043406521870471799u, 2108439588646104644u}, {803757039314269066u, 1317774742903815403u}, {14839754354425000045u, 1647218428629769253u}, {4714634887749086344u, 2059023035787211567u}, {9864175832484260821u, 1286889397367007229u}, {16941905809032713930u, 1608611746708759036u}, {2730638187581340797u, 2010764683385948796u}, {10930020904093113806u, 1256727927116217997u}, {18274212148543780162u, 1570909908895272496u}, {4396021111970173586u, 1963637386119090621u}, {5053356204195052443u, 1227273366324431638u}, {15540067292098591362u, 1534091707905539547u}, {14813398096695851299u, 1917614634881924434u}, {13870059828862294966u, 1198509146801202771u}, {12725888767650480803u, 1498136433501503464u}, {15907360959563101004u, 1872670541876879330u}, {14553786618154326031u, 1170419088673049581u}, {4357175217410743827u, 1463023860841311977u}, {10058155040190817688u, 1828779826051639971u}, {7961007781811134206u, 2285974782564549964u}, {14199001900486734687u, 1428734239102843727u}, {13137066357181030455u, 1785917798878554659u}, {11809646928048900164u, 2232397248598193324u}, {16604401366885338411u, 1395248280373870827u}, {16143815690179285109u, 1744060350467338534u}, {10956397575869330579u, 2180075438084173168u}, {6847748484918331612u, 1362547148802608230u}, {17783057643002690323u, 1703183936003260287u}, {17617136035325974999u, 2128979920004075359u}, {17928239049719816230u, 1330612450002547099u}, {17798612793722382384u, 1663265562503183874u}, {13024893955298202172u, 2079081953128979843u}, {5834715712847682405u, 1299426220705612402u}, {16516766677914378815u, 1624282775882015502u}, {11422586310538197711u, 2030353469852519378u}, {11750802462513761473u, 1268970918657824611u}, {10076817059714813937u, 1586213648322280764u}, {12596021324643517422u, 1982767060402850955u}, {5566670318688504437u, 1239229412751781847u}, {2346651879933242642u, 1549036765939727309u}, {7545000868343941206u, 1936295957424659136u}, {4715625542714963254u, 1210184973390411960u}, {5894531928393704067u, 1512731216738014950u}, {16591536947346905892u, 1890914020922518687u}, {17287239619732898039u, 1181821263076574179u}, {16997363506238734644u, 1477276578845717724u}, {2799960309088866689u, 1846595723557147156u}, {10973347230035317489u, 1154122327223216972u}, {13716684037544146861u, 1442652909029021215u}, {12534169028502795672u, 1803316136286276519u}, {11056025267201106687u, 2254145170357845649u}, {18439230838069161439u, 1408840731473653530u}, {13825666510731675991u, 1761050914342066913u}, {3447025083132431277u, 2201313642927583642u}, {6766076695385157452u, 1375821026829739776u}, {8457595869231446815u, 1719776283537174720u}, {10571994836539308519u, 2149720354421468400u}, {6607496772837067824u, 1343575221513417750u}, {17482743002901110588u, 1679469026891772187u}, {17241742735199000331u, 2099336283614715234u}, {15387775227926763111u, 1312085177259197021u}, {5399660979626290177u, 1640106471573996277u}, {11361262242960250625u, 2050133089467495346u}, {11712474920277544544u, 1281333180917184591u}, {10028907631919542777u, 1601666476146480739u}, {7924448521472040567u, 2002083095183100924u}, {14176152362774801162u, 1251301934489438077u}, {3885132398186337741u, 1564127418111797597u}, {9468101516160310080u, 1955159272639746996u}, {15140935484454969608u, 1221974545399841872u}, {479425281859160394u, 1527468181749802341u}, {5210967620751338397u, 1909335227187252926u}, {17091912818251750210u, 1193334516992033078u}, {12141518985959911954u, 1491668146240041348u}, {15176898732449889943u, 1864585182800051685u}, {11791404716994875166u, 1165365739250032303u}, {10127569877816206054u, 1456707174062540379u}, {8047776328842869663u, 1820883967578175474u}, {836348374198811271u, 2276104959472719343u}, {7440246761515338900u, 1422565599670449589u}, {13911994470321561530u, 1778206999588061986u}, {8166621051047176104u, 2222758749485077483u}, {2798295147690791113u, 1389224218428173427u}, {17332926989895652603u, 1736530273035216783u}, {17054472718942177850u, 2170662841294020979u}, {8353202440125167204u, 1356664275808763112u}, {10441503050156459005u, 1695830344760953890u}, {3828506775840797949u, 2119787930951192363u}, {86973725686804766u, 1324867456844495227u}, {13943775212390669669u, 1656084321055619033u}, {3594660960206173375u, 2070105401319523792u}, {2246663100128858359u, 1293815875824702370u}, {12031700912015848757u, 1617269844780877962u}, {5816254103165035138u, 2021587305976097453u}, {5941001823691840913u, 1263492066235060908u}, {7426252279614801142u, 1579365082793826135u}, {4671129331091113523u, 1974206353492282669u}, {5225298841145639904u, 1233878970932676668u}, {6531623551432049880u, 1542348713665845835u}, {3552843420862674446u, 1927935892082307294u}, {16055585193321335241u, 1204959932551442058u}, {10846109454796893243u, 1506199915689302573u}, {18169322836923504458u, 1882749894611628216u}, {11355826773077190286u, 1176718684132267635u}, {9583097447919099954u, 1470898355165334544u}, {11978871809898874942u, 1838622943956668180u}, {14973589762373593678u, 2298278679945835225u}, {2440964573842414192u, 1436424174966147016u}, {3051205717303017741u, 1795530218707683770u}, {13037379183483547984u, 2244412773384604712u}, {8148361989677217490u, 1402757983365377945u}, {14797138505523909766u, 1753447479206722431u}, {13884737113477499304u, 2191809349008403039u}, {15595489723564518921u, 1369880843130251899u}, {14882676136028260747u, 1712351053912814874u}, {9379973133180550126u, 2140438817391018593u}, {17391698254306313589u, 1337774260869386620u}, {3292878744173340370u, 1672217826086733276u}, {4116098430216675462u, 2090272282608416595u}, {266718509671728212u, 1306420176630260372u}, {333398137089660265u, 1633025220787825465u}, {5028433689789463235u, 2041281525984781831u}, {10060300083759496378u, 1275800953740488644u}, {12575375104699370472u, 1594751192175610805u}, {1884160825592049379u, 1993438990219513507u}, {17318501580490888525u, 1245899368887195941u}, {7813068920331446945u, 1557374211108994927u}, {5154650131986920777u, 1946717763886243659u}, {915813323278131534u, 1216698602428902287u}, {14979824709379828129u, 1520873253036127858u}, {9501408849870009354u, 1901091566295159823u}, {12855909558809837702u, 1188182228934474889u}, {2234828893230133415u, 1485227786168093612u}, {2793536116537666769u, 1856534732710117015u}, {8663489100477123587u, 1160334207943823134u}, {1605989338741628675u, 1450417759929778918u}, {11230858710281811652u, 1813022199912223647u}, {9426887369424876662u, 2266277749890279559u}, {12809333633531629769u, 1416423593681424724u}, {16011667041914537212u, 1770529492101780905u}, {6179525747111007803u, 2213161865127226132u}, {13085575628799155685u, 1383226165704516332u}, {16356969535998944606u, 1729032707130645415u}, {15834525901571292854u, 2161290883913306769u}, {2979049660840976177u, 1350806802445816731u}, {17558870131333383934u, 1688508503057270913u}, {8113529608884566205u, 2110635628821588642u}, {9682642023980241782u, 1319147268013492901u}, {16714988548402690132u, 1648934085016866126u}, {11670363648648586857u, 2061167606271082658u}, {11905663298832754689u, 1288229753919426661u}, {1047021068258779650u, 1610287192399283327u}, {15143834390605638274u, 2012858990499104158u}, {4853210475701136017u, 1258036869061940099u}, {1454827076199032118u, 1572546086327425124u}, {1818533845248790147u, 1965682607909281405u}, {3442426662494187794u, 1228551629943300878u}, {13526405364972510550u, 1535689537429126097u}, {3072948650933474476u, 1919611921786407622u}, {15755650962115585259u, 1199757451116504763u}, {15082877684217093670u, 1499696813895630954u}, {9630225068416591280u, 1874621017369538693u}, {8324733676974063502u, 1171638135855961683u}, {5794231077790191473u, 1464547669819952104u}, {7242788847237739342u, 1830684587274940130u}, {18276858095901949986u, 2288355734093675162u}, {16034722328366106645u, 1430222333808546976u}, {1596658836748081690u, 1787777917260683721u}, {6607509564362490017u, 2234722396575854651u}, {1823850468512862308u, 1396701497859909157u}, {6891499104068465790u, 1745876872324886446u}, {17837745916940358045u, 2182346090406108057u}, {4231062170446641922u, 1363966306503817536u}, {5288827713058302403u, 1704957883129771920u}, {6611034641322878003u, 2131197353912214900u}, {13355268687681574560u, 1331998346195134312u}, {16694085859601968200u, 1664997932743917890u}, {11644235287647684442u, 2081247415929897363u}, {4971804045566108824u, 1300779634956185852u}, {6214755056957636030u, 1625974543695232315u}, {3156757802769657134u, 2032468179619040394u}, {6584659645158423613u, 1270292612261900246u}, {17454196593302805324u, 1587865765327375307u}, {17206059723201118751u, 1984832206659219134u}, {6142101308573311315u, 1240520129162011959u}, {3065940617289251240u, 1550650161452514949u}, {8444111790038951954u, 1938312701815643686u}, {665883850346957067u, 1211445438634777304u}, {832354812933696334u, 1514306798293471630u}, {10263815553021896226u, 1892883497866839537u}, {17944099766707154901u, 1183052186166774710u}, {13206752671529167818u, 1478815232708468388u}, {16508440839411459773u, 1848519040885585485u}, {12623618533845856310u, 1155324400553490928u}, {15779523167307320387u, 1444155500691863660u}, {1277659885424598868u, 1805194375864829576u}, {1597074856780748586u, 2256492969831036970u}, {5609857803915355770u, 1410308106144398106u}, {16235694291748970521u, 1762885132680497632u}, {1847873790976661535u, 2203606415850622041u}, {12684136165428883219u, 1377254009906638775u}, {11243484188358716120u, 1721567512383298469u}, {219297180166231438u, 2151959390479123087u}, {7054589765244976505u, 1344974619049451929u}, {13429923224983608535u, 1681218273811814911u}, {12175718012802122765u, 2101522842264768639u}, {14527352785642408584u, 1313451776415480399u}, {13547504963625622826u, 1641814720519350499u}, {12322695186104640628u, 2052268400649188124u}, {16925056528170176201u, 1282667750405742577u}, {7321262604930556539u, 1603334688007178222u}, {18374950293017971482u, 2004168360008972777u}, {4566814905495150320u, 1252605225005607986u}, {14931890668723713708u, 1565756531257009982u}, {9441491299049866327u, 1957195664071262478u}, {1289246043478778550u, 1223247290044539049u}, {6223243572775861092u, 1529059112555673811u}, {3167368447542438461u, 1911323890694592264u}, {1979605279714024038u, 1194577431684120165u}, {7086192618069917952u, 1493221789605150206u}, {18081112809442173248u, 1866527237006437757u}, {13606538515115052232u, 1166579523129023598u}, {7784801107039039482u, 1458224403911279498u}, {507629346944023544u, 1822780504889099373u}, {5246222702107417334u, 2278475631111374216u}, {3278889188817135834u, 1424047269444608885u}, {8710297504448807696u, 1780059086805761106u}};

#endif // RYU_D2S_FULL_TABLE_H

#if defined(RYU_D2S_IMPL) && !defined(RYU_D2S_IMPL_C)
#define RYU_D2S_IMPL_C

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifdef RYU_DEBUG
#  include <inttypes.h>
#  include <stdio.h>
#endif

#define RYU_F64_MANTISSA_BASE 1ull
#define RYU_F64_MANTISSA_BITS 52
#define RYU_F64_EXPONENT_BITS 11
#define RYU_F64_BIAS 1023

static inline uint32_t ryu_decimalLength_f64(const uint64_t v) {
  // This is slightly faster than a loop.
  // The average output length is 16.38 digits, so we check high-to-low.
  // Function precondition: v is not an 18, 19, or 20-digit number.
  // (17 digits are sufficient for round-tripping.)
  assert(v < 100000000000000000L);
  if (v >= 10000000000000000L) {
    return 17;
  }
  if (v >= 1000000000000000L) {
    return 16;
  }
  if (v >= 100000000000000L) {
    return 15;
  }
  if (v >= 10000000000000L) {
    return 14;
  }
  if (v >= 1000000000000L) {
    return 13;
  }
  if (v >= 100000000000L) {
    return 12;
  }
  if (v >= 10000000000L) {
    return 11;
  }
  if (v >= 1000000000L) {
    return 10;
  }
  if (v >= 100000000L) {
    return 9;
  }
  if (v >= 10000000L) {
    return 8;
  }
  if (v >= 1000000L) {
    return 7;
  }
  if (v >= 100000L) {
    return 6;
  }
  if (v >= 10000L) {
    return 5;
  }
  if (v >= 1000L) {
    return 4;
  }
  if (v >= 100L) {
    return 3;
  }
  if (v >= 10L) {
    return 2;
  }
  return 1;
}

static inline floating_ieee_f64 ryu_f64_to_ieee(const double value) {
  uint64_t bits = ryu_f64_to_bits(value);
  bool sign = ((bits >> (RYU_F64_MANTISSA_BITS + RYU_F64_EXPONENT_BITS)) & 1) != 0;
  uint64_t mantissa = bits & ((RYU_F64_MANTISSA_BASE << RYU_F64_MANTISSA_BITS) - 1);
  uint32_t exponent = (bits >> RYU_F64_MANTISSA_BITS) & ((1u << RYU_F64_EXPONENT_BITS) - 1);
  return (floating_ieee_f64){.mantissa = mantissa, .exponent = exponent, .sign = sign};
}

static inline bool ryu_f64_is_nan(floating_ieee_f64 ieee) {
  return (ieee.exponent == ((1u << RYU_F64_EXPONENT_BITS) - 1u)) && ieee.mantissa;
}

static inline bool ryu_f64_is_infinity(floating_ieee_f64 ieee) {
  return (ieee.exponent == ((1u << RYU_F64_EXPONENT_BITS) - 1u)) && !ieee.mantissa;
}

static inline bool ryu_f64_is_zero(floating_ieee_f64 ieee) {
  return ieee.exponent == 0 && ieee.mantissa == 0;
}

static inline floating_decimal_f64 ryu_f64_parse(floating_ieee_f64 ieee) {
  uint64_t m2 = (1ull << RYU_F64_MANTISSA_BITS) | ieee.mantissa;
  int32_t e2 = (int32_t)ieee.exponent - RYU_F64_BIAS - RYU_F64_MANTISSA_BITS;

  // f = m2 * 2^e2 >= 2^53 is an integer.
  // Ignore this case for now.
  if (e2 <= 0 &&
      // f < 1.
      e2 >= -52 && ({
                     // Since 2^52 <= m2 < 2^53 and 0 <= -e2 <= 52: 1 <= f = m2 / 2^-e2 < 2^53.
                     // Test if the lower -e2 bits of the significand are 0, i.e. whether the fraction is 0.
                     const uint64_t mask = (1ull << -e2) - 1;
                     const uint64_t fraction = m2 & mask;
                     fraction;
                   }) == 0) {
    floating_decimal_f64 v = {.mantissa = m2 >> -e2};
    for (;;) {
      uint64_t q = ryu_div10(v.mantissa);
      uint32_t r = ((uint32_t)v.mantissa) - 10 * ((uint32_t)q);
      if (r != 0) break;
      v.mantissa = q;
      ++v.exponent;
    }
    return v;
  }

  if (ieee.exponent == 0) {
    // We subtract 2 so that the bounds computation has 2 additional bits.
    e2 = 1 - RYU_F64_BIAS - RYU_F64_MANTISSA_BITS - 2;
    m2 = ieee.mantissa;
  } else {
    e2 = (int32_t)ieee.exponent - RYU_F64_BIAS - RYU_F64_MANTISSA_BITS - 2;
    m2 = (1ull << RYU_F64_MANTISSA_BITS) | ieee.mantissa;
  }
  const bool even = (m2 & 1) == 0;
  const bool acceptBounds = even;

#ifdef RYU_DEBUG
  printf("-> %" PRIu64 " * 2^%d\n", m2, e2 + 2);
#endif

  // Step 2: Determine the interval of valid decimal representations.
  const uint64_t mv = 4 * m2;
  // Implicit bool -> int conversion. True is 1, false is 0.
  const uint32_t mmShift = ieee.mantissa != 0 || ieee.exponent <= 1;
  // We would compute mp and mm like this:
  // uint64_t mp = 4 * m2 + 2;
  // uint64_t mm = mv - 1 - mmShift;

  // Step 3: Convert to a decimal power base using 128-bit arithmetic.
  uint64_t vr, vp, vm;
  int32_t e10;
  bool vmIsTrailingZeros = false;
  bool vrIsTrailingZeros = false;
  if (e2 >= 0) {
    // I tried special-casing q == 0, but there was no effect on performance.
    // This expression is slightly faster than max(0, ryu_log10Pow2(e2) - 1).
    const uint32_t q = ryu_log10Pow2(e2) - (e2 > 3);
    e10 = (int32_t)q;
    const int32_t k = RYU_DOUBLE_POW5_INV_BITCOUNT + ryu_pow5bits((int32_t)q) - 1;
    const int32_t i = -e2 + (int32_t)q + k;
#if defined(RYU_OPTIMIZE_SIZE)
    uint64_t pow5[2];
    ryu_double_computeInvPow5(q, pow5);
    vr = ryu_mulShiftAll64(m2, pow5, i, &vp, &vm, mmShift);
#else
    vr = ryu_mulShiftAll64(m2, RYU_DOUBLE_POW5_INV_SPLIT[q], i, &vp, &vm, mmShift);
#endif
#ifdef RYU_DEBUG
    printf("%" PRIu64 " * 2^%d / 10^%u\n", mv, e2, q);
    printf("V+=%" PRIu64 "\nV =%" PRIu64 "\nV-=%" PRIu64 "\n", vp, vr, vm);
#endif
    if (q <= 21) {
      // This should use q <= 22, but I think 21 is also safe. Smaller values
      // may still be safe, but it's more difficult to reason about them.
      // Only one of mp, mv, and mm can be a multiple of 5, if any.
      const uint32_t mvMod5 = ((uint32_t)mv) - 5 * ((uint32_t)ryu_div5(mv));
      if (mvMod5 == 0) {
        vrIsTrailingZeros = ryu_multipleOfPowerOf5(mv, q);
      } else if (acceptBounds) {
        // Same as min(e2 + (~mm & 1), ryu_pow5Factor(mm)) >= q
        // <=> e2 + (~mm & 1) >= q && ryu_pow5Factor(mm) >= q
        // <=> true && ryu_pow5Factor(mm) >= q, since e2 >= q.
        vmIsTrailingZeros = ryu_multipleOfPowerOf5(mv - 1 - mmShift, q);
      } else {
        // Same as min(e2 + 1, ryu_pow5Factor(mp)) >= q.
        vp -= ryu_multipleOfPowerOf5(mv + 2, q);
      }
    }
  } else {
    // This expression is slightly faster than max(0, ryu_log10Pow5(-e2) - 1).
    const uint32_t q = ryu_log10Pow5(-e2) - (-e2 > 1);
    e10 = (int32_t)q + e2;
    const int32_t i = -e2 - (int32_t)q;
    const int32_t k = ryu_pow5bits(i) - RYU_DOUBLE_POW5_BITCOUNT;
    const int32_t j = (int32_t)q - k;
#if defined(RYU_OPTIMIZE_SIZE)
    uint64_t pow5[2];
    ryu_double_computePow5(i, pow5);
    vr = ryu_mulShiftAll64(m2, pow5, j, &vp, &vm, mmShift);
#else
    vr = ryu_mulShiftAll64(m2, RYU_DOUBLE_POW5_SPLIT[i], j, &vp, &vm, mmShift);
#endif
#ifdef RYU_DEBUG
    printf("%" PRIu64 " * 5^%d / 10^%u\n", mv, -e2, q);
    printf("%u %d %d %d\n", q, i, k, j);
    printf("V+=%" PRIu64 "\nV =%" PRIu64 "\nV-=%" PRIu64 "\n", vp, vr, vm);
#endif
    if (q <= 1) {
      // {vr,vp,vm} is trailing zeros if {mv,mp,mm} has at least q trailing 0 bits.
      // mv = 4 * m2, so it always has at least two trailing 0 bits.
      vrIsTrailingZeros = true;
      if (acceptBounds) {
        // mm = mv - 1 - mmShift, so it has 1 trailing 0 bit iff mmShift == 1.
        vmIsTrailingZeros = mmShift == 1;
      } else {
        // mp = mv + 2, so it always has at least one trailing 0 bit.
        --vp;
      }
    } else if (q < 63) { // TODO(ulfjack): Use a tighter bound here.
      // We want to know if the full product has at least q trailing zeros.
      // We need to compute min(p2(mv), p5(mv) - e2) >= q
      // <=> p2(mv) >= q && p5(mv) - e2 >= q
      // <=> p2(mv) >= q (because -e2 >= q)
      vrIsTrailingZeros = ryu_multipleOfPowerOf2(mv, q);
#ifdef RYU_DEBUG
      printf("vr is trailing zeros=%s\n", vrIsTrailingZeros ? "true" : "false");
#endif
    }
  }
#ifdef RYU_DEBUG
  printf("e10=%d\n", e10);
  printf("V+=%" PRIu64 "\nV =%" PRIu64 "\nV-=%" PRIu64 "\n", vp, vr, vm);
  printf("vm is trailing zeros=%s\n", vmIsTrailingZeros ? "true" : "false");
  printf("vr is trailing zeros=%s\n", vrIsTrailingZeros ? "true" : "false");
#endif

  // Step 4: Find the shortest decimal representation in the interval of valid representations.
  int32_t removed = 0;
  uint8_t lastRemovedDigit = 0;
  uint64_t output;
  // On average, we remove ~2 digits.
  if (vmIsTrailingZeros || vrIsTrailingZeros) {
    // General case, which happens rarely (~0.7%).
    for (;;) {
      const uint64_t vpDiv10 = ryu_div10(vp);
      const uint64_t vmDiv10 = ryu_div10(vm);
      if (vpDiv10 <= vmDiv10) {
        break;
      }
      const uint32_t vmMod10 = ((uint32_t)vm) - 10 * ((uint32_t)vmDiv10);
      const uint64_t vrDiv10 = ryu_div10(vr);
      const uint32_t vrMod10 = ((uint32_t)vr) - 10 * ((uint32_t)vrDiv10);
      vmIsTrailingZeros &= vmMod10 == 0;
      vrIsTrailingZeros &= lastRemovedDigit == 0;
      lastRemovedDigit = (uint8_t)vrMod10;
      vr = vrDiv10;
      vp = vpDiv10;
      vm = vmDiv10;
      ++removed;
    }
#ifdef RYU_DEBUG
    printf("V+=%" PRIu64 "\nV =%" PRIu64 "\nV-=%" PRIu64 "\n", vp, vr, vm);
    printf("d-10=%s\n", vmIsTrailingZeros ? "true" : "false");
#endif
    if (vmIsTrailingZeros) {
      for (;;) {
        const uint64_t vmDiv10 = ryu_div10(vm);
        const uint32_t vmMod10 = ((uint32_t)vm) - 10 * ((uint32_t)vmDiv10);
        if (vmMod10 != 0) {
          break;
        }
        const uint64_t vpDiv10 = ryu_div10(vp);
        const uint64_t vrDiv10 = ryu_div10(vr);
        const uint32_t vrMod10 = ((uint32_t)vr) - 10 * ((uint32_t)vrDiv10);
        vrIsTrailingZeros &= lastRemovedDigit == 0;
        lastRemovedDigit = (uint8_t)vrMod10;
        vr = vrDiv10;
        vp = vpDiv10;
        vm = vmDiv10;
        ++removed;
      }
    }
#ifdef RYU_DEBUG
    printf("%" PRIu64 " %d\n", vr, lastRemovedDigit);
    printf("vr is trailing zeros=%s\n", vrIsTrailingZeros ? "true" : "false");
#endif
    if (vrIsTrailingZeros && lastRemovedDigit == 5 && vr % 2 == 0) {
      // Round even if the exact number is .....50..0.
      lastRemovedDigit = 4;
    }
    // We need to take vr + 1 if vr is outside bounds or we need to round up.
    output = vr + ((vr == vm && (!acceptBounds || !vmIsTrailingZeros)) || lastRemovedDigit >= 5);
  } else {
    // Specialized for the common case (~99.3%). Percentages below are relative to this.
    bool roundUp = false;
    const uint64_t vpDiv100 = ryu_div100(vp);
    const uint64_t vmDiv100 = ryu_div100(vm);
    if (vpDiv100 > vmDiv100) { // Optimization: remove two digits at a time (~86.2%).
      const uint64_t vrDiv100 = ryu_div100(vr);
      const uint32_t vrMod100 = ((uint32_t)vr) - 100 * ((uint32_t)vrDiv100);
      roundUp = vrMod100 >= 50;
      vr = vrDiv100;
      vp = vpDiv100;
      vm = vmDiv100;
      removed += 2;
    }
    // Loop iterations below (approximately), without optimization above:
    // 0: 0.03%, 1: 13.8%, 2: 70.6%, 3: 14.0%, 4: 1.40%, 5: 0.14%, 6+: 0.02%
    // Loop iterations below (approximately), with optimization above:
    // 0: 70.6%, 1: 27.8%, 2: 1.40%, 3: 0.14%, 4+: 0.02%
    for (;;) {
      const uint64_t vpDiv10 = ryu_div10(vp);
      const uint64_t vmDiv10 = ryu_div10(vm);
      if (vpDiv10 <= vmDiv10) {
        break;
      }
      const uint64_t vrDiv10 = ryu_div10(vr);
      const uint32_t vrMod10 = ((uint32_t)vr) - 10 * ((uint32_t)vrDiv10);
      roundUp = vrMod10 >= 5;
      vr = vrDiv10;
      vp = vpDiv10;
      vm = vmDiv10;
      ++removed;
    }
#ifdef RYU_DEBUG
    printf("%" PRIu64 " roundUp=%s\n", vr, roundUp ? "true" : "false");
    printf("vr is trailing zeros=%s\n", vrIsTrailingZeros ? "true" : "false");
#endif
    // We need to take vr + 1 if vr is outside bounds or we need to round up.
    output = vr + (vr == vm || roundUp);
  }
  const int32_t exp = e10 + removed;

#ifdef RYU_DEBUG
  printf("V+=%" PRIu64 "\nV =%" PRIu64 "\nV-=%" PRIu64 "\n", vp, vr, vm);
  printf("O=%" PRIu64 "\n", output);
  printf("EXP=%d\n", exp);
#endif

  floating_decimal_f64 fd;
  fd.exponent = exp;
  fd.mantissa = output;
  return fd;
}

#endif // RYU_D2S_IMPL_C

#if defined(RYU_F2S_FULL_TABLE_INCL_INCL) && !defined(RYU_F2S_FULL_TABLE_INCL_H)
#define RYU_F2S_FULL_TABLE_INCL_H

// This table is generated by PrintFloatLookupTable.
#define RYU_FLOAT_POW5_INV_BITCOUNT 59
#define RYU_FLOAT_POW5_BITCOUNT 61

static uint64_t RYU_FLOAT_POW5_INV_SPLIT[55] = {
    576460752303423489u, 461168601842738791u, 368934881474191033u, 295147905179352826u,
    472236648286964522u, 377789318629571618u, 302231454903657294u, 483570327845851670u,
    386856262276681336u, 309485009821345069u, 495176015714152110u, 396140812571321688u,
    316912650057057351u, 507060240091291761u, 405648192073033409u, 324518553658426727u,
    519229685853482763u, 415383748682786211u, 332306998946228969u, 531691198313966350u,
    425352958651173080u, 340282366920938464u, 544451787073501542u, 435561429658801234u,
    348449143727040987u, 557518629963265579u, 446014903970612463u, 356811923176489971u,
    570899077082383953u, 456719261665907162u, 365375409332725730u, 292300327466180584u,
    467680523945888934u, 374144419156711148u, 299315535325368918u, 478904856520590269u,
    383123885216472215u, 306499108173177772u, 490398573077084435u, 392318858461667548u,
    313855086769334039u, 502168138830934462u, 401734511064747569u, 321387608851798056u,
    514220174162876889u, 411376139330301511u, 329100911464241209u, 526561458342785934u,
    421249166674228747u, 336999333339382998u, 539198933343012796u, 431359146674410237u,
    345087317339528190u, 552139707743245103u, 441711766194596083u};
static const uint64_t RYU_FLOAT_POW5_SPLIT[47] = {
    1152921504606846976u, 1441151880758558720u, 1801439850948198400u, 2251799813685248000u,
    1407374883553280000u, 1759218604441600000u, 2199023255552000000u, 1374389534720000000u,
    1717986918400000000u, 2147483648000000000u, 1342177280000000000u, 1677721600000000000u,
    2097152000000000000u, 1310720000000000000u, 1638400000000000000u, 2048000000000000000u,
    1280000000000000000u, 1600000000000000000u, 2000000000000000000u, 1250000000000000000u,
    1562500000000000000u, 1953125000000000000u, 1220703125000000000u, 1525878906250000000u,
    1907348632812500000u, 1192092895507812500u, 1490116119384765625u, 1862645149230957031u,
    1164153218269348144u, 1455191522836685180u, 1818989403545856475u, 2273736754432320594u,
    1421085471520200371u, 1776356839400250464u, 2220446049250313080u, 1387778780781445675u,
    1734723475976807094u, 2168404344971008868u, 1355252715606880542u, 1694065894508600678u,
    2117582368135750847u, 1323488980084844279u, 1654361225106055349u, 2067951531382569187u,
    1292469707114105741u, 1615587133892632177u, 2019483917365790221u};

#endif // RYU_F2S_FULL_TABLE_INCL_H

#if defined(RYU_F2S_INTRINSICS_INCL) && !defined(RYU_F2S_INTRINSICS_H)
#define RYU_F2S_INTRINSICS_H

#ifndef RYU_FLOAT_FULL_TABLE
#  define RYU_FLOAT_POW5_INV_BITCOUNT (RYU_DOUBLE_POW5_INV_BITCOUNT - 64)
#  define RYU_FLOAT_POW5_BITCOUNT (RYU_DOUBLE_POW5_BITCOUNT - 64)
#endif

static inline uint32_t ryu_pow5factor_32(uint32_t value) {
  uint32_t count = 0;
  for (;;) {
    assert(value != 0);
    const uint32_t q = value / 5;
    const uint32_t r = value % 5;
    if (r != 0) {
      break;
    }
    value = q;
    ++count;
  }
  return count;
}

// Returns true if value is divisible by 5^p.
static inline bool ryu_multipleOfPowerOf5_32(const uint32_t value, const uint32_t p) {
  return ryu_pow5factor_32(value) >= p;
}

// Returns true if value is divisible by 2^p.
static inline bool ryu_multipleOfPowerOf2_32(const uint32_t value, const uint32_t p) {
  // __builtin_ctz doesn't appear to be faster here.
  return (value & ((1u << p) - 1)) == 0;
}

// It seems to be slightly faster to avoid __uint128_t here, although the
// generated code for __uint128_t looks slightly nicer.
static inline uint32_t ryu_mulShift32(const uint32_t m, const uint64_t factor, const int32_t shift) {
  assert(shift > 32);

  // The casts here help MSVC to avoid calls to the __allmul library
  // function.
  const uint32_t factorLo = (uint32_t)(factor);
  const uint32_t factorHi = (uint32_t)(factor >> 32);
  const uint64_t bits0 = (uint64_t)m * factorLo;
  const uint64_t bits1 = (uint64_t)m * factorHi;

#if defined(RYU_32_BIT_PLATFORM)
  // On 32-bit platforms we can avoid a 64-bit shift-right since we only
  // need the upper 32 bits of the result and the shift value is > 32.
  const uint32_t bits0Hi = (uint32_t)(bits0 >> 32);
  uint32_t bits1Lo = (uint32_t)(bits1);
  uint32_t bits1Hi = (uint32_t)(bits1 >> 32);
  bits1Lo += bits0Hi;
  bits1Hi += (bits1Lo < bits0Hi);
  if (shift >= 64) {
    // s2f can call this with a shift value >= 64, which we have to handle.
    // This could now be slower than the !defined(RYU_32_BIT_PLATFORM) case.
    return (uint32_t)(bits1Hi >> (shift - 64));
  } else {
    const int32_t s = shift - 32;
    return (bits1Hi << (32 - s)) | (bits1Lo >> s);
  }
#else  // RYU_32_BIT_PLATFORM
  const uint64_t sum = (bits0 >> 32) + bits1;
  const uint64_t shiftedSum = sum >> (shift - 32);
  assert(shiftedSum <= UINT32_MAX);
  return (uint32_t)shiftedSum;
#endif // RYU_32_BIT_PLATFORM
}

static inline uint32_t ryu_mulPow5InvDivPow2(const uint32_t m, const uint32_t q, const int32_t j) {
#if defined(RYU_FLOAT_FULL_TABLE)
  return ryu_mulShift32(m, RYU_FLOAT_POW5_INV_SPLIT[q], j);
#elif defined(RYU_OPTIMIZE_SIZE)
  // The inverse multipliers are defined as [2^x / 5^y] + 1; the upper 64 bits from the double lookup
  // table are the correct bits for [2^x / 5^y], so we have to add 1 here. Note that we rely on the
  // fact that the added 1 that's already stored in the table never overflows into the upper 64 bits.
  uint64_t pow5[2];
  ryu_double_computeInvPow5(q, pow5);
  return ryu_mulShift32(m, pow5[1] + 1, j);
#else
  return ryu_mulShift32(m, RYU_DOUBLE_POW5_INV_SPLIT[q][1] + 1, j);
#endif
}

static inline uint32_t ryu_mulPow5divPow2(const uint32_t m, const uint32_t i, const int32_t j) {
#if defined(RYU_FLOAT_FULL_TABLE)
  return ryu_mulShift32(m, RYU_FLOAT_POW5_SPLIT[i], j);
#elif defined(RYU_OPTIMIZE_SIZE)
  uint64_t pow5[2];
  ryu_double_computePow5(i, pow5);
  return ryu_mulShift32(m, pow5[1], j);
#else
  return ryu_mulShift32(m, RYU_DOUBLE_POW5_SPLIT[i][1], j);
#endif
}

#endif // RYU_F2S_INTRINSICS_H

#if defined(RYU_F2S_IMPL) && !defined(RYU_F2S_IMPL_C)
#define RYU_F2S_IMPL_C

#include <assert.h>
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifdef RYU_DEBUG
#  include <stdio.h>
#endif

#define RYU_F32_MANTISSA_BASE 1u
#define RYU_F32_MANTISSA_BITS 23
#define RYU_F32_EXPONENT_BITS 8
#define RYU_F32_BIAS 127

static inline floating_ieee_f32 ryu_f32_to_ieee(const float value) {
  uint64_t bits = ryu_f32_to_bits(value);
  bool sign = ((bits >> (RYU_F32_MANTISSA_BITS + RYU_F32_EXPONENT_BITS)) & 1) != 0;
  uint64_t mantissa = bits & ((RYU_F32_MANTISSA_BASE << RYU_F32_MANTISSA_BITS) - 1);
  uint32_t exponent = (bits >> RYU_F32_MANTISSA_BITS) & ((1u << RYU_F32_EXPONENT_BITS) - 1);
  return (floating_ieee_f32){.mantissa = mantissa, .exponent = exponent, .sign = sign};
}

static inline bool ryu_f32_is_nan(floating_ieee_f32 ieee) {
  return (ieee.exponent == ((1u << RYU_F32_EXPONENT_BITS) - 1u)) && ieee.mantissa;
}

static inline bool ryu_f32_is_infinity(floating_ieee_f32 ieee) {
  return (ieee.exponent == ((1u << RYU_F32_EXPONENT_BITS) - 1u)) && !ieee.mantissa;
}

static inline bool ryu_f32_is_zero(floating_ieee_f32 ieee) {
  return ieee.exponent == 0 && ieee.mantissa == 0;
}

static inline floating_decimal_f32 ryu_f32_parse(floating_ieee_f32 ieee) {
  int32_t e2;
  uint32_t m2;
  if (ieee.exponent == 0) {
    // We subtract 2 so that the bounds computation has 2 additional bits.
    e2 = 1 - RYU_F32_BIAS - RYU_F32_MANTISSA_BITS - 2;
    m2 = ieee.mantissa;
  } else {
    e2 = (int32_t)ieee.exponent - RYU_F32_BIAS - RYU_F32_MANTISSA_BITS - 2;
    m2 = (1u << RYU_F32_MANTISSA_BITS) | ieee.mantissa;
  }
  const bool even = (m2 & 1) == 0;
  const bool acceptBounds = even;

#ifdef RYU_DEBUG
  printf("-> %u * 2^%d\n", m2, e2 + 2);
#endif

  // Step 2: Determine the interval of valid decimal representations.
  const uint32_t mv = 4 * m2;
  const uint32_t mp = 4 * m2 + 2;
  // Implicit bool -> int conversion. True is 1, false is 0.
  const uint32_t mmShift = ieee.mantissa != 0 || ieee.exponent <= 1;
  const uint32_t mm = 4 * m2 - 1 - mmShift;

  // Step 3: Convert to a decimal power base using 64-bit arithmetic.
  uint32_t vr, vp, vm;
  int32_t e10;
  bool vmIsTrailingZeros = false;
  bool vrIsTrailingZeros = false;
  uint8_t lastRemovedDigit = 0;
  if (e2 >= 0) {
    const uint32_t q = ryu_log10Pow2(e2);
    e10 = (int32_t)q;
    const int32_t k = RYU_FLOAT_POW5_INV_BITCOUNT + ryu_pow5bits((int32_t)q) - 1;
    const int32_t i = -e2 + (int32_t)q + k;
    vr = ryu_mulPow5InvDivPow2(mv, q, i);
    vp = ryu_mulPow5InvDivPow2(mp, q, i);
    vm = ryu_mulPow5InvDivPow2(mm, q, i);
#ifdef RYU_DEBUG
    printf("%u * 2^%d / 10^%u\n", mv, e2, q);
    printf("V+=%u\nV =%u\nV-=%u\n", vp, vr, vm);
#endif
    if (q != 0 && (vp - 1) / 10 <= vm / 10) {
      // We need to know one removed digit even if we are not going to loop below. We could use
      // q = X - 1 above, except that would require 33 bits for the result, and we've found that
      // 32-bit arithmetic is faster even on 64-bit machines.
      const int32_t l = RYU_FLOAT_POW5_INV_BITCOUNT + ryu_pow5bits((int32_t)(q - 1)) - 1;
      lastRemovedDigit = (uint8_t)(ryu_mulPow5InvDivPow2(mv, q - 1, -e2 + (int32_t)q - 1 + l) % 10);
    }
    if (q <= 9) {
      // The largest power of 5 that fits in 24 bits is 5^10, but q <= 9 seems to be safe as well.
      // Only one of mp, mv, and mm can be a multiple of 5, if any.
      if (mv % 5 == 0) {
        vrIsTrailingZeros = ryu_multipleOfPowerOf5_32(mv, q);
      } else if (acceptBounds) {
        vmIsTrailingZeros = ryu_multipleOfPowerOf5_32(mm, q);
      } else {
        vp -= ryu_multipleOfPowerOf5_32(mp, q);
      }
    }
  } else {
    const uint32_t q = ryu_log10Pow5(-e2);
    e10 = (int32_t)q + e2;
    const int32_t i = -e2 - (int32_t)q;
    const int32_t k = ryu_pow5bits(i) - RYU_FLOAT_POW5_BITCOUNT;
    int32_t j = (int32_t)q - k;
    vr = ryu_mulPow5divPow2(mv, (uint32_t)i, j);
    vp = ryu_mulPow5divPow2(mp, (uint32_t)i, j);
    vm = ryu_mulPow5divPow2(mm, (uint32_t)i, j);
#ifdef RYU_DEBUG
    printf("%u * 5^%d / 10^%u\n", mv, -e2, q);
    printf("%u %d %d %d\n", q, i, k, j);
    printf("V+=%u\nV =%u\nV-=%u\n", vp, vr, vm);
#endif
    if (q != 0 && (vp - 1) / 10 <= vm / 10) {
      j = (int32_t)q - 1 - (ryu_pow5bits(i + 1) - RYU_FLOAT_POW5_BITCOUNT);
      lastRemovedDigit = (uint8_t)(ryu_mulPow5divPow2(mv, (uint32_t)(i + 1), j) % 10);
    }
    if (q <= 1) {
      // {vr,vp,vm} is trailing zeros if {mv,mp,mm} has at least q trailing 0 bits.
      // mv = 4 * m2, so it always has at least two trailing 0 bits.
      vrIsTrailingZeros = true;
      if (acceptBounds) {
        // mm = mv - 1 - mmShift, so it has 1 trailing 0 bit iff mmShift == 1.
        vmIsTrailingZeros = mmShift == 1;
      } else {
        // mp = mv + 2, so it always has at least one trailing 0 bit.
        --vp;
      }
    } else if (q < 31) { // TODO(ulfjack): Use a tighter bound here.
      vrIsTrailingZeros = ryu_multipleOfPowerOf2_32(mv, q - 1);
#ifdef RYU_DEBUG
      printf("vr is trailing zeros=%s\n", vrIsTrailingZeros ? "true" : "false");
#endif
    }
  }
#ifdef RYU_DEBUG
  printf("e10=%d\n", e10);
  printf("V+=%u\nV =%u\nV-=%u\n", vp, vr, vm);
  printf("vm is trailing zeros=%s\n", vmIsTrailingZeros ? "true" : "false");
  printf("vr is trailing zeros=%s\n", vrIsTrailingZeros ? "true" : "false");
#endif

  // Step 4: Find the shortest decimal representation in the interval of valid representations.
  int32_t removed = 0;
  uint32_t output;
  if (vmIsTrailingZeros || vrIsTrailingZeros) {
    // General case, which happens rarely (~4.0%).
    while (vp / 10 > vm / 10) {
#ifdef __clang__ // https://bugs.llvm.org/show_bug.cgi?id=23106
      // The compiler does not realize that vm % 10 can be computed from vm / 10
      // as vm - (vm / 10) * 10.
      vmIsTrailingZeros &= vm - (vm / 10) * 10 == 0;
#else
      vmIsTrailingZeros &= vm % 10 == 0;
#endif
      vrIsTrailingZeros &= lastRemovedDigit == 0;
      lastRemovedDigit = (uint8_t)(vr % 10);
      vr /= 10;
      vp /= 10;
      vm /= 10;
      ++removed;
    }
#ifdef RYU_DEBUG
    printf("V+=%u\nV =%u\nV-=%u\n", vp, vr, vm);
    printf("d-10=%s\n", vmIsTrailingZeros ? "true" : "false");
#endif
    if (vmIsTrailingZeros) {
      while (vm % 10 == 0) {
        vrIsTrailingZeros &= lastRemovedDigit == 0;
        lastRemovedDigit = (uint8_t)(vr % 10);
        vr /= 10;
        vp /= 10;
        vm /= 10;
        ++removed;
      }
    }
#ifdef RYU_DEBUG
    printf("%u %d\n", vr, lastRemovedDigit);
    printf("vr is trailing zeros=%s\n", vrIsTrailingZeros ? "true" : "false");
#endif
    if (vrIsTrailingZeros && lastRemovedDigit == 5 && vr % 2 == 0) {
      // Round even if the exact number is .....50..0.
      lastRemovedDigit = 4;
    }
    // We need to take vr + 1 if vr is outside bounds or we need to round up.
    output = vr + ((vr == vm && (!acceptBounds || !vmIsTrailingZeros)) || lastRemovedDigit >= 5);
  } else {
    // Specialized for the common case (~96.0%). Percentages below are relative to this.
    // Loop iterations below (approximately):
    // 0: 13.6%, 1: 70.7%, 2: 14.1%, 3: 1.39%, 4: 0.14%, 5+: 0.01%
    while (vp / 10 > vm / 10) {
      lastRemovedDigit = (uint8_t)(vr % 10);
      vr /= 10;
      vp /= 10;
      vm /= 10;
      ++removed;
    }
#ifdef RYU_DEBUG
    printf("%u %d\n", vr, lastRemovedDigit);
    printf("vr is trailing zeros=%s\n", vrIsTrailingZeros ? "true" : "false");
#endif
    // We need to take vr + 1 if vr is outside bounds or we need to round up.
    output = vr + (vr == vm || lastRemovedDigit >= 5);
  }
  const int32_t exp = e10 + removed;

#ifdef RYU_DEBUG
  printf("V+=%u\nV =%u\nV-=%u\n", vp, vr, vm);
  printf("O=%u\n", output);
  printf("EXP=%d\n", exp);
#endif

  floating_decimal_f32 fd;
  fd.exponent = exp;
  fd.mantissa = output;
  return fd;
}

#endif // RYU_F2S_IMPL_C

#if defined(RYU_GENERIC_INTRINSICS_IMPL) && !defined(RYU_GENERIC_INTRINSICS_IMPL_C)
#define RYU_GENERIC_INTRINSICS_IMPL_C

#define RYU_FLOAT_128_POW5_INV_BITCOUNT 249
#define RYU_FLOAT_128_POW5_BITCOUNT 249
#define RYU_GENERIC_POW5_TABLE_SIZE 56

static const uint64_t RYU_GENERIC_POW5_TABLE[RYU_GENERIC_POW5_TABLE_SIZE][2] = {
    {1u, 0u},
    {5u, 0u},
    {25u, 0u},
    {125u, 0u},
    {625u, 0u},
    {3125u, 0u},
    {15625u, 0u},
    {78125u, 0u},
    {390625u, 0u},
    {1953125u, 0u},
    {9765625u, 0u},
    {48828125u, 0u},
    {244140625u, 0u},
    {1220703125u, 0u},
    {6103515625u, 0u},
    {30517578125u, 0u},
    {152587890625u, 0u},
    {762939453125u, 0u},
    {3814697265625u, 0u},
    {19073486328125u, 0u},
    {95367431640625u, 0u},
    {476837158203125u, 0u},
    {2384185791015625u, 0u},
    {11920928955078125u, 0u},
    {59604644775390625u, 0u},
    {298023223876953125u, 0u},
    {1490116119384765625u, 0u},
    {7450580596923828125u, 0u},
    {359414837200037393u, 2u},
    {1797074186000186965u, 10u},
    {8985370930000934825u, 50u},
    {8033366502585570893u, 252u},
    {3273344365508751233u, 1262u},
    {16366721827543756165u, 6310u},
    {8046632842880574361u, 31554u},
    {3339676066983768573u, 157772u},
    {16698380334918842865u, 788860u},
    {9704925379756007861u, 3944304u},
    {11631138751360936073u, 19721522u},
    {2815461535676025517u, 98607613u},
    {14077307678380127585u, 493038065u},
    {15046306170771983077u, 2465190328u},
    {1444554559021708921u, 12325951644u},
    {7222772795108544605u, 61629758220u},
    {17667119901833171409u, 308148791101u},
    {14548623214327650581u, 1540743955509u},
    {17402883850509598057u, 7703719777548u},
    {13227442957709783821u, 38518598887744u},
    {10796982567420264257u, 192592994438723u},
    {17091424689682218053u, 962964972193617u},
    {11670147153572883801u, 4814824860968089u},
    {3010503546735764157u, 24074124304840448u},
    {15052517733678820785u, 120370621524202240u},
    {1475612373555897461u, 601853107621011204u},
    {7378061867779487305u, 3009265538105056020u},
    {18443565265187884909u, 15046327690525280101u}};

static const uint64_t RYU_GENERIC_POW5_SPLIT[89][4] = {
    {0u, 0u, 0u, 72057594037927936u},
    {0u, 5206161169240293376u, 4575641699882439235u, 73468396926392969u},
    {3360510775605221349u, 6983200512169538081u, 4325643253124434363u, 74906821675075173u},
    {11917660854915489451u, 9652941469841108803u, 946308467778435600u, 76373409087490117u},
    {1994853395185689235u, 16102657350889591545u, 6847013871814915412u, 77868710555449746u},
    {958415760277438274u, 15059347134713823592u, 7329070255463483331u, 79393288266368765u},
    {2065144883315240188u, 7145278325844925976u, 14718454754511147343u, 80947715414629833u},
    {8980391188862868935u, 13709057401304208685u, 8230434828742694591u, 82532576417087045u},
    {432148644612782575u, 7960151582448466064u, 12056089168559840552u, 84148467132788711u},
    {484109300864744403u, 15010663910730448582u, 16824949663447227068u, 85795995087002057u},
    {14793711725276144220u, 16494403799991899904u, 10145107106505865967u, 87475779699624060u},
    {15427548291869817042u, 12330588654550505203u, 13980791795114552342u, 89188452518064298u},
    {9979404135116626552u, 13477446383271537499u, 14459862802511591337u, 90934657454687378u},
    {12385121150303452775u, 9097130814231585614u, 6523855782339765207u, 92715051028904201u},
    {1822931022538209743u, 16062974719797586441u, 3619180286173516788u, 94530302614003091u},
    {12318611738248470829u, 13330752208259324507u, 10986694768744162601u, 96381094688813589u},
    {13684493829640282333u, 7674802078297225834u, 15208116197624593182u, 98268123094297527u},
    {5408877057066295332u, 6470124174091971006u, 15112713923117703147u, 100192097295163851u},
    {11407083166564425062u, 18189998238742408185u, 4337638702446708282u, 102153740646605557u},
    {4112405898036935485u, 924624216579956435u, 14251108172073737125u, 104153790666259019u},
    {16996739107011444789u, 10015944118339042475u, 2395188869672266257u, 106192999311487969u},
    {4588314690421337879u, 5339991768263654604u, 15441007590670620066u, 108272133262096356u},
    {2286159977890359825u, 14329706763185060248u, 5980012964059367667u, 110391974208576409u},
    {9654767503237031099u, 11293544302844823188u, 11739932712678287805u, 112553319146000238u},
    {11362964448496095896u, 7990659682315657680u, 251480263940996374u, 114756980673665505u},
    {1423410421096377129u, 14274395557581462179u, 16553482793602208894u, 117003787300607788u},
    {2070444190619093137u, 11517140404712147401u, 11657844572835578076u, 119294583757094535u},
    {7648316884775828921u, 15264332483297977688u, 247182277434709002u, 121630231312217685u},
    {17410896758132241352u, 10923914482914417070u, 13976383996795783649u, 124011608097704390u},
    {9542674537907272703u, 3079432708831728956u, 14235189590642919676u, 126439609438067572u},
    {10364666969937261816u, 8464573184892924210u, 12758646866025101190u, 128915148187220428u},
    {14720354822146013883u, 11480204489231511423u, 7449876034836187038u, 131439155071681461u},
    {1692907053653558553u, 17835392458598425233u, 1754856712536736598u, 134012579040499057u},
    {5620591334531458755u, 11361776175667106627u, 13350215315297937856u, 136636387622027174u},
    {17455759733928092601u, 10362573084069962561u, 11246018728801810510u, 139311567287686283u},
    {2465404073814044982u, 17694822665274381860u, 1509954037718722697u, 142039123822846312u},
    {2152236053329638369u, 11202280800589637091u, 16388426812920420176u, 72410041352485523u},
    {17319024055671609028u, 10944982848661280484u, 2457150158022562661u, 73827744744583080u},
    {17511219308535248024u, 5122059497846768077u, 2089605804219668451u, 75273205100637900u},
    {10082673333144031533u, 14429008783411894887u, 12842832230171903890u, 76746965869337783u},
    {16196653406315961184u, 10260180891682904501u, 10537411930446752461u, 78249581139456266u},
    {15084422041749743389u, 234835370106753111u, 16662517110286225617u, 79781615848172976u},
    {8199644021067702606u, 3787318116274991885u, 7438130039325743106u, 81343645993472659u},
    {12039493937039359765u, 9773822153580393709u, 5945428874398357806u, 82936258850702722u},
    {984543865091303961u, 7975107621689454830u, 6556665988501773347u, 84560053193370726u},
    {9633317878125234244u, 16099592426808915028u, 9706674539190598200u, 86215639518264828u},
    {6860695058870476186u, 4471839111886709592u, 7828342285492709568u, 87903640274981819u},
    {14583324717644598331u, 4496120889473451238u, 5290040788305728466u, 89624690099949049u},
    {18093669366515003715u, 12879506572606942994u, 18005739787089675377u, 91379436055028227u},
    {17997493966862379937u, 14646222655265145582u, 10265023312844161858u, 93168537870790806u},
    {12283848109039722318u, 11290258077250314935u, 9878160025624946825u, 94992668194556404u},
    {8087752761883078164u, 5262596608437575693u, 11093553063763274413u, 96852512843287537u},
    {15027787746776840781u, 12250273651168257752u, 9290470558712181914u, 98748771061435726u},
    {15003915578366724489u, 2937334162439764327u, 5404085603526796602u, 100682155783835929u},
    {5225610465224746757u, 14932114897406142027u, 2774647558180708010u, 102653393903748137u},
    {17112957703385190360u, 12069082008339002412u, 3901112447086388439u, 104663226546146909u},
    {4062324464323300238u, 3992768146772240329u, 15757196565593695724u, 106712409346361594u},
    {5525364615810306701u, 11855206026704935156u, 11344868740897365300u, 108801712734172003u},
    {9274143661888462646u, 4478365862348432381u, 18010077872551661771u, 110931922223466333u},
    {12604141221930060148u, 8930937759942591500u, 9382183116147201338u, 113103838707570263u},
    {14513929377491886653u, 1410646149696279084u, 587092196850797612u, 115318278760358235u},
    {2226851524999454362u, 7717102471110805679u, 7187441550995571734u, 117576074943260147u},
    {5527526061344932763u, 2347100676188369132u, 16976241418824030445u, 119878076118278875u},
    {6088479778147221611u, 17669593130014777580u, 10991124207197663546u, 122225147767136307u},
    {11107734086759692041u, 3391795220306863431u, 17233960908859089158u, 124618172316667879u},
    {7913172514655155198u, 17726879005381242552u, 641069866244011540u, 127058049470587962u},
    {12596991768458713949u, 15714785522479904446u, 6035972567136116512u, 129545696547750811u},
    {16901996933781815980u, 4275085211437148707u, 14091642539965169063u, 132082048827034281u},
    {7524574627987869240u, 15661204384239316051u, 2444526454225712267u, 134668059898975949u},
    {8199251625090479942u, 6803282222165044067u, 16064817666437851504u, 137304702024293857u},
    {4453256673338111920u, 15269922543084434181u, 3139961729834750852u, 139992966499426682u},
    {15841763546372731299u, 3013174075437671812u, 4383755396295695606u, 142733864029230733u},
    {9771896230907310329u, 4900659362437687569u, 12386126719044266361u, 72764212553486967u},
    {9420455527449565190u, 1859606122611023693u, 6555040298902684281u, 74188850200884818u},
    {5146105983135678095u, 2287300449992174951u, 4325371679080264751u, 75641380576797959u},
    {11019359372592553360u, 8422686425957443718u, 7175176077944048210u, 77122349788024458u},
    {11005742969399620716u, 4132174559240043701u, 9372258443096612118u, 78632314633490790u},
    {8887589641394725840u, 8029899502466543662u, 14582206497241572853u, 80171842813591127u},
    {360247523705545899u, 12568341805293354211u, 14653258284762517866u, 81741513143625247u},
    {12314272731984275834u, 4740745023227177044u, 6141631472368337539u, 83341915771415304u},
    {441052047733984759u, 7940090120939869826u, 11750200619921094248u, 84973652399183278u},
    {3436657868127012749u, 9187006432149937667u, 16389726097323041290u, 86637336509772529u},
    {13490220260784534044u, 15339072891382896702u, 8846102360835316895u, 88333593597298497u},
    {4125672032094859833u, 158347675704003277u, 10592598512749774447u, 90063061402315272u},
    {12189928252974395775u, 2386931199439295891u, 7009030566469913276u, 91826390151586454u},
    {9256479608339282969u, 2844900158963599229u, 11148388908923225596u, 93624242802550437u},
    {11584393507658707408u, 2863659090805147914u, 9873421561981063551u, 95457295292572042u},
    {13984297296943171390u, 1931468383973130608u, 12905719743235082319u, 97326236793074198u},
    {5837045222254987499u, 10213498696735864176u, 14893951506257020749u, 99231769968645227u}};

// Unfortunately, the results are sometimes off by one or two. We use an additional
// lookup table to store those cases and adjust the result.
static const uint64_t RYU_GENERIC_POW5_ERRORS[156] = {
    0x0000000000000000u, 0x0000000000000000u, 0x0000000000000000u, 0x9555596400000000u,
    0x65a6569525565555u, 0x4415551445449655u, 0x5105015504144541u, 0x65a69969a6965964u,
    0x5054955969959656u, 0x5105154515554145u, 0x4055511051591555u, 0x5500514455550115u,
    0x0041140014145515u, 0x1005440545511051u, 0x0014405450411004u, 0x0414440010500000u,
    0x0044000440010040u, 0x5551155000004001u, 0x4554555454544114u, 0x5150045544005441u,
    0x0001111400054501u, 0x6550955555554554u, 0x1504159645559559u, 0x4105055141454545u,
    0x1411541410405454u, 0x0415555044545555u, 0x0014154115405550u, 0x1540055040411445u,
    0x0000000500000000u, 0x5644000000000000u, 0x1155555591596555u, 0x0410440054569565u,
    0x5145100010010005u, 0x0555041405500150u, 0x4141450455140450u, 0x0000000144000140u,
    0x5114004001105410u, 0x4444100404005504u, 0x0414014410001015u, 0x5145055155555015u,
    0x0141041444445540u, 0x0000100451541414u, 0x4105041104155550u, 0x0500501150451145u,
    0x1001050000004114u, 0x5551504400141045u, 0x5110545410151454u, 0x0100001400004040u,
    0x5040010111040000u, 0x0140000150541100u, 0x4400140400104110u, 0x5011014405545004u,
    0x0000000044155440u, 0x0000000010000000u, 0x1100401444440001u, 0x0040401010055111u,
    0x5155155551405454u, 0x0444440015514411u, 0x0054505054014101u, 0x0451015441115511u,
    0x1541411401140551u, 0x4155104514445110u, 0x4141145450145515u, 0x5451445055155050u,
    0x4400515554110054u, 0x5111145104501151u, 0x565a655455500501u, 0x5565555555525955u,
    0x0550511500405695u, 0x4415504051054544u, 0x6555595965555554u, 0x0100915915555655u,
    0x5540001510001001u, 0x5450051414000544u, 0x1405010555555551u, 0x5555515555644155u,
    0x5555055595496555u, 0x5451045004415000u, 0x5450510144040144u, 0x5554155555556455u,
    0x5051555495415555u, 0x5555554555555545u, 0x0000000010005455u, 0x4000005000040000u,
    0x5565555555555954u, 0x5554559555555505u, 0x9645545495552555u, 0x4000400055955564u,
    0x0040000000000001u, 0x4004100100000000u, 0x5540040440000411u, 0x4565555955545644u,
    0x1140659549651556u, 0x0100000410010000u, 0x5555515400004001u, 0x5955545555155255u,
    0x5151055545505556u, 0x5051454510554515u, 0x0501500050415554u, 0x5044154005441005u,
    0x1455445450550455u, 0x0010144055144545u, 0x0000401100000004u, 0x1050145050000010u,
    0x0415004554011540u, 0x1000510100151150u, 0x0100040400001144u, 0x0000000000000000u,
    0x0550004400000100u, 0x0151145041451151u, 0x0000400400005450u, 0x0000100044010004u,
    0x0100054100050040u, 0x0504400005410010u, 0x4011410445500105u, 0x0000404000144411u,
    0x0101504404500000u, 0x0000005044400400u, 0x0000000014000100u, 0x0404440414000000u,
    0x5554100410000140u, 0x4555455544505555u, 0x5454105055455455u, 0x0115454155454015u,
    0x4404110000045100u, 0x4400001100101501u, 0x6596955956966a94u, 0x0040655955665965u,
    0x5554144400100155u, 0xa549495401011041u, 0x5596555565955555u, 0x5569965959549555u,
    0x969565a655555456u, 0x0000001000000000u, 0x0000000040000140u, 0x0000040100000000u,
    0x1415454400000000u, 0x5410415411454114u, 0x0400040104000154u, 0x0504045000000411u,
    0x0000001000000010u, 0x5554000000001040u, 0x5549155551556595u, 0x1455541055515555u,
    0x0510555454554541u, 0x9555555555540455u, 0x6455456555556465u, 0x4524565555654514u,
    0x5554655255559545u, 0x9555455441155556u, 0x0000000051515555u, 0x0010005040000550u,
    0x5044044040000000u, 0x1045040440010500u, 0x0000400000040000u, 0x0000000000000000u};

static const uint64_t RYU_GENERIC_POW5_INV_SPLIT[89][4] = {
    {0u, 0u, 0u, 144115188075855872u},
    {1573859546583440065u, 2691002611772552616u, 6763753280790178510u, 141347765182270746u},
    {12960290449513840412u, 12345512957918226762u, 18057899791198622765u, 138633484706040742u},
    {7615871757716765416u, 9507132263365501332u, 4879801712092008245u, 135971326161092377u},
    {7869961150745287587u, 5804035291554591636u, 8883897266325833928u, 133360288657597085u},
    {2942118023529634767u, 15128191429820565086u, 10638459445243230718u, 130799390525667397u},
    {14188759758411913794u, 5362791266439207815u, 8068821289119264054u, 128287668946279217u},
    {7183196927902545212u, 1952291723540117099u, 12075928209936341512u, 125824179589281448u},
    {5672588001402349748u, 17892323620748423487u, 9874578446960390364u, 123407996258356868u},
    {4442590541217566325u, 4558254706293456445u, 10343828952663182727u, 121038210542800766u},
    {3005560928406962566u, 2082271027139057888u, 13961184524927245081u, 118713931475986426u},
    {13299058168408384786u, 17834349496131278595u, 9029906103900731664u, 116434285200389047u},
    {5414878118283973035u, 13079825470227392078u, 17897304791683760280u, 114198414639042157u},
    {14609755883382484834u, 14991702445765844156u, 3269802549772755411u, 112005479173303009u},
    {15967774957605076027u, 2511532636717499923u, 16221038267832563171u, 109854654326805788u},
    {9269330061621627145u, 3332501053426257392u, 16223281189403734630u, 107745131455483836u},
    {16739559299223642282u, 1873986623300664530u, 6546709159471442872u, 105676117443544318u},
    {17116435360051202055u, 1359075105581853924u, 2038341371621886470u, 103646834405281051u},
    {17144715798009627550u, 3201623802661132408u, 9757551605154622431u, 101656519392613377u},
    {17580479792687825857u, 6546633380567327312u, 15099972427870912398u, 99704424108241124u},
    {9726477118325522902u, 14578369026754005435u, 11728055595254428803u, 97789814624307808u},
    {134593949518343635u, 5715151379816901985u, 1660163707976377376u, 95911971106466306u},
    {5515914027713859358u, 7124354893273815720u, 5548463282858794077u, 94070187543243255u},
    {6188403395862945512u, 5681264392632320838u, 15417410852121406654u, 92263771480600430u},
    {15908890877468271457u, 10398888261125597540u, 4817794962769172309u, 90492043761593298u},
    {1413077535082201005u, 12675058125384151580u, 7731426132303759597u, 88754338271028867u},
    {1486733163972670293u, 11369385300195092554u, 11610016711694864110u, 87050001685026843u},
    {8788596583757589684u, 3978580923851924802u, 9255162428306775812u, 85378393225389919u},
    {7203518319660962120u, 15044736224407683725u, 2488132019818199792u, 83738884418690858u},
    {4004175967662388707u, 18236988667757575407u, 15613100370957482671u, 82130858859985791u},
    {18371903370586036463u, 53497579022921640u, 16465963977267203307u, 80553711981064899u},
    {10170778323887491315u, 1999668801648976001u, 10209763593579456445u, 79006850823153334u},
    {17108131712433974546u, 16825784443029944237u, 2078700786753338945u, 77489693813976938u},
    {17221789422665858532u, 12145427517550446164u, 5391414622238668005u, 76001670549108934u},
    {4859588996898795878u, 1715798948121313204u, 3950858167455137171u, 74542221577515387u},
    {13513469241795711526u, 631367850494860526u, 10517278915021816160u, 73110798191218799u},
    {11757513142672073111u, 2581974932255022228u, 17498959383193606459u, 143413724438001539u},
    {14524355192525042817u, 5640643347559376447u, 1309659274756813016u, 140659771648132296u},
    {2765095348461978538u, 11021111021896007722u, 3224303603779962366u, 137958702611185230u},
    {12373410389187981037u, 13679193545685856195u, 11644609038462631561u, 135309501808182158u},
    {12813176257562780151u, 3754199046160268020u, 9954691079802960722u, 132711173221007413u},
    {17557452279667723458u, 3237799193992485824u, 17893947919029030695u, 130162739957935629u},
    {14634200999559435155u, 4123869946105211004u, 6955301747350769239u, 127663243886350468u},
    {2185352760627740240u, 2864813346878886844u, 13049218671329690184u, 125211745272516185u},
    {6143438674322183002u, 10464733336980678750u, 6982925169933978309u, 122807322428266620u},
    {1099509117817174576u, 10202656147550524081u, 754997032816608484u, 120449071364478757u},
    {2410631293559367023u, 17407273750261453804u, 15307291918933463037u, 118136105451200587u},
    {12224968375134586697u, 1664436604907828062u, 11506086230137787358u, 115867555084305488u},
    {3495926216898000888u, 18392536965197424288u, 10992889188570643156u, 113642567358547782u},
    {8744506286256259680u, 3966568369496879937u, 18342264969761820037u, 111460305746896569u},
    {7689600520560455039u, 5254331190877624630u, 9628558080573245556u, 109319949786027263u},
    {11862637625618819436u, 3456120362318976488u, 14690471063106001082u, 107220694767852583u},
    {5697330450030126444u, 12424082405392918899u, 358204170751754904u, 105161751436977040u},
    {11257457505097373622u, 15373192700214208870u, 671619062372033814u, 103142345693961148u},
    {16850355018477166700u, 1913910419361963966u, 4550257919755970531u, 101161718304283822u},
    {9670835567561997011u, 10584031339132130638u, 3060560222974851757u, 99219124612893520u},
    {7698686577353054710u, 11689292838639130817u, 11806331021588878241u, 97313834264240819u},
    {12233569599615692137u, 3347791226108469959u, 10333904326094451110u, 95445130927687169u},
    {13049400362825383933u, 17142621313007799680u, 3790542585289224168u, 93612312028186576u},
    {12430457242474442072u, 5625077542189557960u, 14765055286236672238u, 91814688482138969u},
    {4759444137752473128u, 2230562561567025078u, 4954443037339580076u, 90051584438315940u},
    {7246913525170274758u, 8910297835195760709u, 4015904029508858381u, 88322337023761438u},
    {12854430245836432067u, 8135139748065431455u, 11548083631386317976u, 86626296094571907u},
    {4848827254502687803u, 4789491250196085625u, 3988192420450664125u, 84962823991462151u},
    {7435538409611286684u, 904061756819742353u, 14598026519493048444u, 83331295300025028u},
    {11042616160352530997u, 8948390828345326218u, 10052651191118271927u, 81731096615594853u},
    {11059348291563778943u, 11696515766184685544u, 3783210511290897367u, 80161626312626082u},
    {7020010856491885826u, 5025093219346041680u, 8960210401638911765u, 78622294318500592u},
    {17732844474490699984u, 7820866704994446502u, 6088373186798844243u, 77112521891678506u},
    {688278527545590501u, 3045610706602776618u, 8684243536999567610u, 75631741404109150u},
    {2734573255120657297u, 3903146411440697663u, 9470794821691856713u, 74179396127820347u},
    {15996457521023071259u, 4776627823451271680u, 12394856457265744744u, 72754940025605801u},
    {13492065758834518331u, 7390517611012222399u, 1630485387832860230u, 142715675091463768u},
    {13665021627282055864u, 9897834675523659302u, 17907668136755296849u, 139975126841173266u},
    {9603773719399446181u, 10771916301484339398u, 10672699855989487527u, 137287204938390542u},
    {3630218541553511265u, 8139010004241080614u, 2876479648932814543u, 134650898807055963u},
    {8318835909686377084u, 9525369258927993371u, 2796120270400437057u, 132065217277054270u},
    {11190003059043290163u, 12424345635599592110u, 12539346395388933763u, 129529188211565064u},
    {8701968833973242276u, 820569587086330727u, 2315591597351480110u, 127041858141569228u},
    {5115113890115690487u, 16906305245394587826u, 9899749468931071388u, 124602291907373862u},
    {15543535488939245974u, 10945189844466391399u, 3553863472349432246u, 122209572307020975u},
    {7709257252608325038u, 1191832167690640880u, 15077137020234258537u, 119862799751447719u},
    {7541333244210021737u, 9790054727902174575u, 5160944773155322014u, 117561091926268545u},
    {12297384708782857832u, 1281328873123467374u, 4827925254630475769u, 115303583460052092u},
    {13243237906232367265u, 15873887428139547641u, 3607993172301799599u, 113089425598968120u},
    {11384616453739611114u, 15184114243769211033u, 13148448124803481057u, 110917785887682141u},
    {17727970963596660683u, 1196965221832671990u, 14537830463956404138u, 108787847856377790u},
    {17241367586707330931u, 8880584684128262874u, 11173506540726547818u, 106698810713789254u},
    {7184427196661305643u, 14332510582433188173u, 14230167953789677901u, 104649889046128358u}};

static const uint64_t RYU_GENERIC_POW5_INV_ERRORS[154] = {
    0x1144155514145504u,
    0x0000541555401141u,
    0x0000000000000000u,
    0x0154454000000000u,
    0x4114105515544440u,
    0x0001001111500415u,
    0x4041411410011000u,
    0x5550114515155014u,
    0x1404100041554551u,
    0x0515000450404410u,
    0x5054544401140004u,
    0x5155501005555105u,
    0x1144141000105515u,
    0x0541500000500000u,
    0x1104105540444140u,
    0x4000015055514110u,
    0x0054010450004005u,
    0x4155515404100005u,
    0x5155145045155555u,
    0x1511555515440558u,
    0x5558544555515555u,
    0x0000000000000010u,
    0x5004000000000050u,
    0x1415510100000010u,
    0x4545555444514500u,
    0x5155151555555551u,
    0x1441540144044554u,
    0x5150104045544400u,
    0x5450545401444040u,
    0x5554455045501400u,
    0x4655155555555145u,
    0x1000010055455055u,
    0x1000004000055004u,
    0x4455405104000005u,
    0x4500114504150545u,
    0x0000000014000000u,
    0x5450000000000000u,
    0x5514551511445555u,
    0x4111501040555451u,
    0x4515445500054444u,
    0x5101500104100441u,
    0x1545115155545055u,
    0x0000000000000000u,
    0x1554000000100000u,
    0x5555545595551555u,
    0x5555051851455955u,
    0x5555555555555559u,
    0x0000400011001555u,
    0x0000004400040000u,
    0x5455511555554554u,
    0x5614555544115445u,
    0x6455156145555155u,
    0x5455855455415455u,
    0x5515555144555545u,
    0x0114400000145155u,
    0x0000051000450511u,
    0x4455154554445100u,
    0x4554150141544455u,
    0x65955555559a5965u,
    0x5555555854559559u,
    0x9569654559616595u,
    0x1040044040005565u,
    0x1010010500011044u,
    0x1554015545154540u,
    0x4440555401545441u,
    0x1014441450550105u,
    0x4545400410504145u,
    0x5015111541040151u,
    0x5145051154000410u,
    0x1040001044545044u,
    0x4001400000151410u,
    0x0540000044040000u,
    0x0510555454411544u,
    0x0400054054141550u,
    0x1001041145001100u,
    0x0000000140000000u,
    0x0000000014100000u,
    0x1544005454000140u,
    0x4050055505445145u,
    0x0011511104504155u,
    0x5505544415045055u,
    0x1155154445515554u,
    0x0000000000004555u,
    0x0000000000000000u,
    0x5101010510400004u,
    0x1514045044440400u,
    0x5515519555515555u,
    0x4554545441555545u,
    0x1551055955551515u,
    0x0150000011505515u,
    0x0044005040400000u,
    0x0004001004010050u,
    0x0000051004450414u,
    0x0114001101001144u,
    0x0401000001000001u,
    0x4500010001000401u,
    0x0004100000005000u,
    0x0105000441101100u,
    0x0455455550454540u,
    0x5404050144105505u,
    0x4101510540555455u,
    0x1055541411451555u,
    0x5451445110115505u,
    0x1154110010101545u,
    0x1145140450054055u,
    0x5555565415551554u,
    0x1550559555555555u,
    0x5555541545045141u,
    0x4555455450500100u,
    0x5510454545554555u,
    0x1510140115045455u,
    0x1001050040111510u,
    0x5555454555555504u,
    0x9954155545515554u,
    0x6596656555555555u,
    0x0140410051555559u,
    0x0011104010001544u,
    0x965669659a680501u,
    0x5655a55955556955u,
    0x4015111014404514u,
    0x1414155554505145u,
    0x0540040011051404u,
    0x1010000000015005u,
    0x0010054050004410u,
    0x5041104014000100u,
    0x4440010500100001u,
    0x1155510504545554u,
    0x0450151545115541u,
    0x4000100400110440u,
    0x1004440010514440u,
    0x0000115050450000u,
    0x0545404455541500u,
    0x1051051555505101u,
    0x5505144554544144u,
    0x4550545555515550u,
    0x0015400450045445u,
    0x4514155400554415u,
    0x4555055051050151u,
    0x1511441450001014u,
    0x4544554510404414u,
    0x4115115545545450u,
    0x5500541555551555u,
    0x5550010544155015u,
    0x0144414045545500u,
    0x4154050001050150u,
    0x5550511111000145u,
    0x1114504055000151u,
    0x5104041101451040u,
    0x0010501401051441u,
    0x0010501450504401u,
    0x4554585440044444u,
    0x5155555951450455u,
    0x0040000400105555u,
    0x0000000000000001u,
};

// Returns e == 0 ? 1 : ceil(log_2(5^e)); requires 0 <= e <= 32768.
static inline uint32_t ryu_generic_pow5bits(const int32_t e) {
  assert(e >= 0);
  assert(e <= 1 << 15);
  return (uint32_t)(((e * 163391164108059ull) >> 46) + 1);
}

static inline void ryu_generic_mul_128_256_shift(const uint64_t *const a, const uint64_t *const b, const uint32_t shift, const uint32_t corr, uint64_t *const result) {
  assert(shift > 0);
  assert(shift < 256);
  const __uint128_t b00 = ((__uint128_t)a[0]) * b[0]; // 0
  const __uint128_t b01 = ((__uint128_t)a[0]) * b[1]; // 64
  const __uint128_t b02 = ((__uint128_t)a[0]) * b[2]; // 128
  const __uint128_t b03 = ((__uint128_t)a[0]) * b[3]; // 196
  const __uint128_t b10 = ((__uint128_t)a[1]) * b[0]; // 64
  const __uint128_t b11 = ((__uint128_t)a[1]) * b[1]; // 128
  const __uint128_t b12 = ((__uint128_t)a[1]) * b[2]; // 196
  const __uint128_t b13 = ((__uint128_t)a[1]) * b[3]; // 256

  const __uint128_t s0 = b00;       // 0   x
  const __uint128_t s1 = b01 + b10; // 64  x
  const __uint128_t c1 = s1 < b01;  // 196 x
  const __uint128_t s2 = b02 + b11; // 128 x
  const __uint128_t c2 = s2 < b02;  // 256 x
  const __uint128_t s3 = b03 + b12; // 196 x
  const __uint128_t c3 = s3 < b03;  // 324 x

  const __uint128_t p0 = s0 + (s1 << 64);                              // 0
  const __uint128_t d0 = p0 < b00;                                     // 128
  const __uint128_t q1 = s2 + (s1 >> 64) + (s3 << 64);                 // 128
  const __uint128_t d1 = q1 < s2;                                      // 256
  const __uint128_t p1 = q1 + (c1 << 64) + d0;                         // 128
  const __uint128_t d2 = p1 < q1;                                      // 256
  const __uint128_t p2 = b13 + (s3 >> 64) + c2 + (c3 << 64) + d1 + d2; // 256

  if (shift < 128) {
    const __uint128_t r0 = corr + ((p0 >> shift) | (p1 << (128 - shift)));
    const __uint128_t r1 = ((p1 >> shift) | (p2 << (128 - shift))) + (r0 < corr);
    result[0] = (uint64_t)r0;
    result[1] = (uint64_t)(r0 >> 64);
    result[2] = (uint64_t)r1;
    result[3] = (uint64_t)(r1 >> 64);
  } else if (shift == 128) {
    const __uint128_t r0 = corr + p1;
    const __uint128_t r1 = p2 + (r0 < corr);
    result[0] = (uint64_t)r0;
    result[1] = (uint64_t)(r0 >> 64);
    result[2] = (uint64_t)r1;
    result[3] = (uint64_t)(r1 >> 64);
  } else {
    const __uint128_t r0 = corr + ((p1 >> (shift - 128)) | (p2 << (256 - shift)));
    const __uint128_t r1 = (p2 >> (shift - 128)) + (r0 < corr);
    result[0] = (uint64_t)r0;
    result[1] = (uint64_t)(r0 >> 64);
    result[2] = (uint64_t)r1;
    result[3] = (uint64_t)(r1 >> 64);
  }
}

// Computes 5^i in the form required by Ryu, and stores it in the given pointer.
static inline void ryu_generic_computePow5(const uint32_t i, uint64_t *const result) {
  const uint32_t base = i / RYU_GENERIC_POW5_TABLE_SIZE;
  const uint32_t base2 = base * RYU_GENERIC_POW5_TABLE_SIZE;
  const uint64_t *const mul = RYU_GENERIC_POW5_SPLIT[base];
  if (i == base2) {
    result[0] = mul[0];
    result[1] = mul[1];
    result[2] = mul[2];
    result[3] = mul[3];
  } else {
    const uint32_t offset = i - base2;
    const uint64_t *const m = RYU_GENERIC_POW5_TABLE[offset];
    const uint32_t delta = ryu_generic_pow5bits(i) - ryu_generic_pow5bits(base2);
    const uint32_t corr = (uint32_t)((RYU_GENERIC_POW5_ERRORS[i / 32] >> (2 * (i % 32))) & 3);
    ryu_generic_mul_128_256_shift(m, mul, delta, corr, result);
  }
}

// Computes 5^-i in the form required by Ryu, and stores it in the given pointer.
static inline void ryu_generic_computeInvPow5(const uint32_t i, uint64_t *const result) {
  const uint32_t base = (i + RYU_GENERIC_POW5_TABLE_SIZE - 1) / RYU_GENERIC_POW5_TABLE_SIZE;
  const uint32_t base2 = base * RYU_GENERIC_POW5_TABLE_SIZE;
  const uint64_t *const mul = RYU_GENERIC_POW5_INV_SPLIT[base]; // 1/5^base2
  if (i == base2) {
    result[0] = mul[0] + 1;
    result[1] = mul[1];
    result[2] = mul[2];
    result[3] = mul[3];
  } else {
    const uint32_t offset = base2 - i;
    const uint64_t *const m = RYU_GENERIC_POW5_TABLE[offset]; // 5^offset
    const uint32_t delta = ryu_generic_pow5bits(base2) - ryu_generic_pow5bits(i);
    const uint32_t corr = (uint32_t)((RYU_GENERIC_POW5_INV_ERRORS[i / 32] >> (2 * (i % 32))) & 3) + 1;
    ryu_generic_mul_128_256_shift(m, mul, delta, corr, result);
  }
}

static inline uint32_t ryu_generic_pow5Factor(__uint128_t value) {
  for (uint32_t count = 0; value > 0; ++count) {
    if (value % 5 != 0) {
      return count;
    }
    value /= 5;
  }
  return 0;
}

// Returns true if value is divisible by 5^p.
static inline bool ryu_generic_multipleOfPowerOf5(const __uint128_t value, const uint32_t p) {
  // I tried a case distinction on p, but there was no performance difference.
  return ryu_generic_pow5Factor(value) >= p;
}

// Returns true if value is divisible by 2^p.
static inline bool ryu_generic_multipleOfPowerOf2(const __uint128_t value, const uint32_t p) {
  return (value & ((((__uint128_t)1) << p) - 1)) == 0;
}

static inline __uint128_t ryu_generic_mulShift(const __uint128_t m, const uint64_t *const mul, const int32_t j) {
  assert(j > 128);
  uint64_t a[2];
  a[0] = (uint64_t)m;
  a[1] = (uint64_t)(m >> 64);
  uint64_t result[4];
  ryu_generic_mul_128_256_shift(a, mul, j, 0, result);
  return (((__uint128_t)result[1]) << 64) | result[0];
}

#endif // RYU_GENERIC_INTRINSICS_IMPL_C

#if defined(RYU_GENERIC_IMPL) && !defined(RYU_GENERIC_IMPL_C)
#define RYU_GENERIC_IMPL_C

#define ONE ((__uint128_t)1)
static inline floating_ieee_generic ryu_generic_to_ieee(const __uint128_t bits, const uint32_t mantissaBits, const uint32_t exponentBits, const bool explicitLeadingBit) {
  const bool sign = ((bits >> (mantissaBits + exponentBits)) & 1) != 0;
  const __uint128_t mantissa = bits & ((ONE << mantissaBits) - 1);
  const uint32_t exponent = (uint32_t)((bits >> mantissaBits) & ((ONE << exponentBits) - 1u));
  return (floating_ieee_generic){.mantissa = mantissa, .exponent = exponent, .sign = sign, .mantissaBits = mantissaBits, .exponentBits = exponentBits, .explicitLeadingBit = explicitLeadingBit};
}
static inline floating_ieee_generic ryu_f16_to_ieee(const f16_t value) {
  return ryu_generic_to_ieee(f16_to_bits(value), 10, 5, false);
}
static inline floating_ieee_generic ryu_f80_to_ieee(const f80_t value) {
  return ryu_generic_to_ieee(f80_to_bits(value), 64, 15, true);
}
static inline floating_ieee_generic ryu_f128_to_ieee(const f128_t value) {
  return ryu_generic_to_ieee(f128_to_bits(value), 112, 15, false);
}
static inline floating_ieee_generic ryu_f64pair_to_ieee(const f64pair_t value) {
  f64pair_canonical_t f64pair = f64pair_canonical_from(value);
  bool sign = ((f64pair.high >> 63) & 1) != 0;
  uint32_t exponent = (uint32_t)((f64pair.high >> 52) & 0x7FF);
  __uint128_t mantissa = ((__uint128_t)(f64pair.high & 0xFFFFFFFFFFFFF) << 52) | (f64pair.low & 0xFFFFFFFFFFFFF);
  return (floating_ieee_generic){.mantissa = mantissa, .exponent = exponent, .sign = sign, .mantissaBits = 104, .exponentBits = 11, .explicitLeadingBit = false};
}

static inline bool ryu_generic_is_nan(floating_ieee_generic ieee) {
  return (ieee.exponent == ((1u << ieee.exponentBits) - 1u)) && ieee.mantissa;
}

static inline bool ryu_generic_is_infinity(floating_ieee_generic ieee) {
  return (ieee.exponent == ((1u << ieee.exponentBits) - 1u)) && !ieee.mantissa;
}

static inline bool ryu_generic_is_zero(floating_ieee_generic ieee) {
  return ieee.exponent == 0 && ieee.mantissa == 0;
}

static inline uint32_t ryu_decimalLength_generic(const __uint128_t v) {
  static __uint128_t LARGEST_POW10 = (((__uint128_t)5421010862427522170ull) << 64) | 687399551400673280ull;
  __uint128_t p10 = LARGEST_POW10;
  for (uint32_t i = 39; i > 0; i--) {
    if (v >= p10) {
      return i;
    }
    p10 /= 10;
  }
  return 1;
}

// Returns floor(log_10(2^e)).
static inline uint32_t ryu_generic_log10Pow2(const int32_t e) {
  // The first value this approximation fails for is 2^1651 which is just greater than 10^297.
  assert(e >= 0);
  assert(e <= 1 << 15);
  return (uint32_t)((((uint64_t)e) * 169464822037455ull) >> 49);
}

// Returns floor(log_10(5^e)).
static inline uint32_t ryu_generic_log10Pow5(const int32_t e) {
  // The first value this approximation fails for is 5^2621 which is just greater than 10^1832.
  assert(e >= 0);
  assert(e <= 1 << 15);
  return (uint32_t)((((uint64_t)e) * 196742565691928ull) >> 48);
}

static inline floating_decimal_generic ryu_generic_parse(floating_ieee_generic ieee) {
  const uint32_t bias = (1u << (ieee.exponentBits - 1)) - 1;

  int32_t e2;
  __uint128_t m2;
  // We subtract 2 in all cases so that the bounds computation has 2 additional bits.
  if (ieee.explicitLeadingBit) {
    // mantissaBits includes the explicit leading bit, so we need to correct for that here.
    if (ieee.exponent == 0) {
      e2 = 1 - bias - ieee.mantissaBits + 1 - 2;
    } else {
      e2 = ieee.exponent - bias - ieee.mantissaBits + 1 - 2;
    }
    m2 = ieee.mantissa;
  } else {
    if (ieee.exponent == 0) {
      e2 = 1 - bias - ieee.mantissaBits - 2;
      m2 = ieee.mantissa;
    } else {
      e2 = ieee.exponent - bias - ieee.mantissaBits - 2;
      m2 = (ONE << ieee.mantissaBits) | ieee.mantissa;
    }
  }
  const bool even = (m2 & 1) == 0;
  const bool acceptBounds = even;

#ifdef RYU_DEBUG
  printf("-> %s %s * 2^%d\n", ieeeSign ? "-" : "+", s(m2), e2 + 2);
#endif

  // Step 2: Determine the interval of legal decimal representations.
  const __uint128_t mv = 4 * m2;
  // Implicit bool -> int conversion. True is 1, false is 0.
  const uint32_t mmShift = (ieee.mantissa != (ieee.explicitLeadingBit ? ONE << (ieee.mantissaBits - 1) : 0)) || (ieee.exponent == 0);

  // Step 3: Convert to a decimal power base using 128-bit arithmetic.
  __uint128_t vr, vp, vm;
  int32_t e10;
  bool vmIsTrailingZeros = false;
  bool vrIsTrailingZeros = false;
  if (e2 >= 0) {
    // I tried special-casing q == 0, but there was no effect on performance.
    // This expression is slightly faster than max(0, ryu_generic_log10Pow2(e2) - 1).
    const uint32_t q = ryu_generic_log10Pow2(e2) - (e2 > 3);
    e10 = q;
    const int32_t k = RYU_FLOAT_128_POW5_INV_BITCOUNT + ryu_generic_pow5bits(q) - 1;
    const int32_t i = -e2 + q + k;
    uint64_t pow5[4];
    ryu_generic_computeInvPow5(q, pow5);
    vr = ryu_generic_mulShift(4 * m2, pow5, i);
    vp = ryu_generic_mulShift(4 * m2 + 2, pow5, i);
    vm = ryu_generic_mulShift(4 * m2 - 1 - mmShift, pow5, i);
#ifdef RYU_DEBUG
    printf("%s * 2^%d / 10^%d\n", s(mv), e2, q);
    printf("V+=%s\nV =%s\nV-=%s\n", s(vp), s(vr), s(vm));
#endif
    // floor(log_5(2^128)) = 55, this is very conservative
    if (q <= 55) {
      // Only one of mp, mv, and mm can be a multiple of 5, if any.
      if (mv % 5 == 0) {
        vrIsTrailingZeros = ryu_generic_multipleOfPowerOf5(mv, q - 1);
      } else if (acceptBounds) {
        // Same as min(e2 + (~mm & 1), ryu_generic_pow5Factor(mm)) >= q
        // <=> e2 + (~mm & 1) >= q && ryu_generic_pow5Factor(mm) >= q
        // <=> true && ryu_generic_pow5Factor(mm) >= q, since e2 >= q.
        vmIsTrailingZeros = ryu_generic_multipleOfPowerOf5(mv - 1 - mmShift, q);
      } else {
        // Same as min(e2 + 1, ryu_generic_pow5Factor(mp)) >= q.
        vp -= ryu_generic_multipleOfPowerOf5(mv + 2, q);
      }
    }
  } else {
    // This expression is slightly faster than max(0, ryu_generic_log10Pow5(-e2) - 1).
    const uint32_t q = ryu_generic_log10Pow5(-e2) - (-e2 > 1);
    e10 = q + e2;
    const int32_t i = -e2 - q;
    const int32_t k = ryu_generic_pow5bits(i) - RYU_FLOAT_128_POW5_BITCOUNT;
    const int32_t j = q - k;
    uint64_t pow5[4];
    ryu_generic_computePow5(i, pow5);
    vr = ryu_generic_mulShift(4 * m2, pow5, j);
    vp = ryu_generic_mulShift(4 * m2 + 2, pow5, j);
    vm = ryu_generic_mulShift(4 * m2 - 1 - mmShift, pow5, j);
#ifdef RYU_DEBUG
    printf("%s * 5^%d / 10^%d\n", s(mv), -e2, q);
    printf("%d %d %d %d\n", q, i, k, j);
    printf("V+=%s\nV =%s\nV-=%s\n", s(vp), s(vr), s(vm));
#endif
    if (q <= 1) {
      // {vr,vp,vm} is trailing zeros if {mv,mp,mm} has at least q trailing 0 bits.
      // mv = 4 m2, so it always has at least two trailing 0 bits.
      vrIsTrailingZeros = true;
      if (acceptBounds) {
        // mm = mv - 1 - mmShift, so it has 1 trailing 0 bit iff mmShift == 1.
        vmIsTrailingZeros = mmShift == 1;
      } else {
        // mp = mv + 2, so it always has at least one trailing 0 bit.
        --vp;
      }
    } else if (q < 127) { // TODO(ulfjack): Use a tighter bound here.
      // We need to compute min(ntz(mv), ryu_generic_pow5Factor(mv) - e2) >= q-1
      // <=> ntz(mv) >= q-1  &&  ryu_generic_pow5Factor(mv) - e2 >= q-1
      // <=> ntz(mv) >= q-1    (e2 is negative and -e2 >= q)
      // <=> (mv & ((1 << (q-1)) - 1)) == 0
      // We also need to make sure that the left shift does not overflow.
      vrIsTrailingZeros = ryu_generic_multipleOfPowerOf2(mv, q - 1);
#ifdef RYU_DEBUG
      printf("vr is trailing zeros=%s\n", vrIsTrailingZeros ? "true" : "false");
#endif
    }
  }
#ifdef RYU_DEBUG
  printf("e10=%d\n", e10);
  printf("V+=%s\nV =%s\nV-=%s\n", s(vp), s(vr), s(vm));
  printf("vm is trailing zeros=%s\n", vmIsTrailingZeros ? "true" : "false");
  printf("vr is trailing zeros=%s\n", vrIsTrailingZeros ? "true" : "false");
#endif

  // Step 4: Find the shortest decimal representation in the interval of legal representations.
  uint32_t removed = 0;
  uint8_t lastRemovedDigit = 0;
  __uint128_t output;

  while (vp / 10 > vm / 10) {
    vmIsTrailingZeros &= vm % 10 == 0;
    vrIsTrailingZeros &= lastRemovedDigit == 0;
    lastRemovedDigit = (uint8_t)(vr % 10);
    vr /= 10;
    vp /= 10;
    vm /= 10;
    ++removed;
  }
#ifdef RYU_DEBUG
  printf("V+=%s\nV =%s\nV-=%s\n", s(vp), s(vr), s(vm));
  printf("d-10=%s\n", vmIsTrailingZeros ? "true" : "false");
#endif
  if (vmIsTrailingZeros) {
    while (vm % 10 == 0) {
      vrIsTrailingZeros &= lastRemovedDigit == 0;
      lastRemovedDigit = (uint8_t)(vr % 10);
      vr /= 10;
      vp /= 10;
      vm /= 10;
      ++removed;
    }
  }
#ifdef RYU_DEBUG
  printf("%s %d\n", s(vr), lastRemovedDigit);
  printf("vr is trailing zeros=%s\n", vrIsTrailingZeros ? "true" : "false");
#endif
  if (vrIsTrailingZeros && (lastRemovedDigit == 5) && (vr % 2 == 0)) {
    // Round even if the exact numbers is .....50..0.
    lastRemovedDigit = 4;
  }
  // We need to take vr+1 if vr is outside bounds or we need to round up.
  output = vr +
           ((vr == vm && (!acceptBounds || !vmIsTrailingZeros)) || (lastRemovedDigit >= 5));
  const int32_t exp = e10 + removed;

#ifdef RYU_DEBUG
  printf("V+=%s\nV =%s\nV-=%s\n", s(vp), s(vr), s(vm));
  printf("O=%s\n", s(output));
  printf("EXP=%d\n", exp);
#endif

  struct floating_decimal_generic fd;
  fd.mantissa = output;
  fd.exponent = exp;
  return fd;
}

#undef ONE

#endif // RYU_GENERIC_IMPL_C
