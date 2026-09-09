#include <cli.h>
#include <stdio.h>
#define SB_NUMBER_IMPL
#include <sb_number.h>

void test_floating_formatting(void) {
#define VAL(val) val
#define TEST(val, expected_str) ({                                                                                                                     \
  String str = stringify(sb_append_floating_f16_fmt, val, (Sb_Floating_Format){0});                                                                    \
  if (strncmp(str.data, expected_str, str.count) == 0 && strlen(expected_str) == str.count) {                                                          \
    printf(CLI_BG_GREEN CLI_BOLD CLI_FG_WHITE "[PASS]" CLI_RESET " %-12s => '" SV_FMT "'\n", #val, sv_fmt_arg(str));                                   \
  } else {                                                                                                                                             \
    printf(CLI_BG_RED CLI_BOLD CLI_FG_WHITE "[FAIL]" CLI_RESET " %-12s => Got: '" SV_FMT "' | Expected: '%s'\n", #val, sv_fmt_arg(str), expected_str); \
  }                                                                                                                                                    \
})
  printf("=== Starting Stringify Tests ===\n\n");
  // 1. Basic Integers & Zeroes
  TEST(VAL(0.0), "0.0");
  TEST(VAL(-0.0), "-0.0");
  TEST(VAL(1.0), "1.0");
  TEST(VAL(-1.0), "-1.0");
  TEST(VAL(256.345), "256.345");
  TEST(VAL(0.256), "0.256");
  // 2. IEEE 754 Half-Precision Boundaries
  TEST(VAL(65504.0), "65504.0");                   // Max positive normal
  TEST(VAL(-65504.0), "-65504.0");                 // Max negative normal
  TEST(VAL(0.00006103515625), "0.00006103515625"); // Min positive normal (2^-14)
  // 3. Subnormals (Denormals)
  TEST(VAL(0.000000059604644775390625), "0.000000059604644775390625"); // Min positive subnormal (2^-24)
  // 4. Floating Point Rounding & Precision Limits
  TEST(VAL(0.1), "0.1");
  TEST(VAL(0.3), "0.3");
  TEST((VAL(1.0) / VAL(3.0)), "0.3333333333333333"); // 11-bit mantissa precision truncation
  // 5. Special IEEE 754 Values (Inf, -Inf, NaN)
  TEST((VAL(1.0) / VAL(0.0)), "Infinity");
  TEST((VAL(-1.0) / VAL(0.0)), "-Infinity");
  TEST((VAL(0.0) / VAL(0.0)), "NaN");
#undef SUFFIX
#undef TEST

  /* f16
  [PASS] VAL(0.0)     => '0.0'
  [PASS] VAL(-0.0)    => '-0.0'
  [PASS] VAL(1.0)     => '1.0'
  [PASS] VAL(-1.0)    => '-1.0'
  [FAIL] VAL(256.345) => Got: '256.2' | Expected: '256.345'
  [PASS] VAL(0.256)   => '0.256'
  [FAIL] VAL(65504.0) => Got: '65500.0' | Expected: '65504.0'
  [FAIL] VAL(-65504.0) => Got: '-65500.0' | Expected: '-65504.0'
  [FAIL] VAL(0.00006103515625) => Got: '0.00006104' | Expected: '0.00006103515625'
  [FAIL] VAL(0.000000059604644775390625) => Got: '0.00000006' | Expected: '0.000000059604644775390625'
  [PASS] VAL(0.1)     => '0.1'
  [PASS] VAL(0.3)     => '0.3'
  [FAIL] (VAL(1.0) / VAL(3.0)) => Got: '0.3333' | Expected: '0.3333333333333333'
  [PASS] (VAL(1.0) / VAL(0.0)) => 'Infinity'
  [PASS] (VAL(-1.0) / VAL(0.0)) => '-Infinity'
  [PASS] (VAL(0.0) / VAL(0.0)) => 'NaN'*/

  /* f32
  [PASS] VAL(0.0)     => '0.0'
  [PASS] VAL(-0.0)    => '-0.0'
  [PASS] VAL(1.0)     => '1.0'
  [PASS] VAL(-1.0)    => '-1.0'
  [PASS] VAL(256.345) => '256.345'
  [PASS] VAL(0.256)   => '0.256'
  [PASS] VAL(65504.0) => '65504.0'
  [PASS] VAL(-65504.0) => '-65504.0'
  [FAIL] VAL(0.00006103515625) => Got: '0.000061035156' | Expected: '0.00006103515625'
  [FAIL] VAL(0.000000059604644775390625) => Got: '0.000000059604645' | Expected: '0.000000059604644775390625'
  [PASS] VAL(0.1)     => '0.1'
  [PASS] VAL(0.3)     => '0.3'
  [FAIL] (VAL(1.0) / VAL(3.0)) => Got: '0.33333334' | Expected: '0.3333333333333333'
  [PASS] (VAL(1.0) / VAL(0.0)) => 'Infinity'
  [PASS] (VAL(-1.0) / VAL(0.0)) => '-Infinity'
  [PASS] (VAL(0.0) / VAL(0.0)) => 'NaN'*/

  /* f64
  [PASS] 0.0          => '0.0'
  [PASS] -0.0         => '-0.0'
  [PASS] 1.0          => '1.0'
  [PASS] -1.0         => '-1.0'
  [PASS] 256.345      => '256.345'
  [PASS] 0.256        => '0.256'
  [PASS] 65504.0      => '65504.0'
  [PASS] -65504.0     => '-65504.0'
  [PASS] 0.00006103515625 => '0.00006103515625'
  [FAIL] 0.000000059604644775390625 => Got: '0.00000005960464477539063' | Expected: '0.000000059604644775390625'
  [PASS] 0.1          => '0.1'
  [PASS] 0.3          => '0.3'
  [PASS] 1 / 3.0      => '0.3333333333333333'
  [PASS] (1.0 / 0.0)  => 'Infinity'
  [PASS] (-1.0 / 0.0) => '-Infinity'
  [PASS] (0.0 / 0.0)  => 'NaN'*/
}

int main(void) {
  test_floating_formatting();

  return 0;
}
