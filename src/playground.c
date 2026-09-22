#include <cli.h>
#include <stdio.h>
#define FLOATS_IGNORE_F16_WARNINGS
#define STR_NUMBERS_IMPL
#include <str_numbers.h>
#define SYX_PARSER_IMPL
#include <syx/syx_parser.h>

// bool print_test_result(const char *test, const char *expected, String actual) {
//   bool passed = strlen(expected) == actual.count && strncmp(actual.data, expected, actual.count) == 0;
//   if (passed) {
//     printf(CLI_BG_GREEN CLI_BOLD CLI_FG_WHITE "[PASS]" CLI_RESET " %-12s => '" SV_FMT "'\n", test, sv_fmt_arg(actual));
//   } else {
//     printf(CLI_BG_RED CLI_BOLD CLI_FG_WHITE "[FAIL]" CLI_RESET " %-12s => Got: '" SV_FMT "' | Expected: '%s'\n", test, sv_fmt_arg(actual), expected);
//   }
//   return passed;
// }

// void test_floating_formatting(void) {
// #define VAL(val) val##L
// #define TEST(val, expected) ({                                               \
//   String actual = stringify(sb_append_floating, val);                        \
//   if (!print_test_result(EXPAND_MACRO(STRINGIFY2, val), expected, actual)) { \
//     printf("printf: '%f'\n", (float)(_Float16)val);                          \
//   }                                                                          \
// })
//   printf("=== Starting Stringify Tests ===\n\n");
//   // 1. Basic Integers & Zeroes
//   TEST(VAL(0.0), "0.0");
//   TEST(VAL(-0.0), "-0.0");
//   TEST(VAL(1.0), "1.0");
//   TEST(VAL(-1.0), "-1.0");
//   TEST(VAL(256.345), "256.345");
//   TEST(VAL(0.256), "0.256");
//   // 2. IEEE 754 Half-Precision Boundaries
//   TEST(VAL(65504.0), "65504.0");                                       // Max positive normal
//   TEST(VAL(-65504.0), "-65504.0");                                     // Max negative normal
//   TEST(VAL(0.00006103515625), "0.00006103515625");                     // Min positive normal (2^-14)
//   TEST(VAL(0.000000059604644775390625), "0.000000059604644775390625"); // Min positive subnormal (2^-24)
//   // 4. Floating Point Rounding & Precision Limits
//   TEST(VAL(0.1), "0.1");
//   TEST(VAL(0.3), "0.3");
//   TEST((VAL(1.0) / VAL(3.0)), "0.3333"); // 11-bit mantissa precision truncation
//   // 5. Special IEEE 754 Values (Inf, -Inf, NaN)
//   TEST((VAL(1.0) / VAL(0.0)), "Infinity");
//   TEST((VAL(-1.0) / VAL(0.0)), "-Infinity");
//   TEST((VAL(0.0) / VAL(0.0)), "NaN");
// #undef SUFFIX
// #undef TEST

//   /* f16
//   [PASS] VAL(0.0)     => '0.0'
//   [PASS] VAL(-0.0)    => '-0.0'
//   [PASS] VAL(1.0)     => '1.0'
//   [PASS] VAL(-1.0)    => '-1.0'
//   [FAIL] VAL(256.345) => Got: '256.2' | Expected: '256.345'
//   [PASS] VAL(0.256)   => '0.256'
//   [FAIL] VAL(65504.0) => Got: '65500.0' | Expected: '65504.0'
//   [FAIL] VAL(-65504.0) => Got: '-65500.0' | Expected: '-65504.0'
//   [FAIL] VAL(0.00006103515625) => Got: '0.00006104' | Expected: '0.00006103515625'
//   [FAIL] VAL(0.000000059604644775390625) => Got: '0.00000006' | Expected: '0.000000059604644775390625'
//   [PASS] VAL(0.1)     => '0.1'
//   [PASS] VAL(0.3)     => '0.3'
//   [FAIL] (VAL(1.0) / VAL(3.0)) => Got: '0.3333' | Expected: '0.3333333333333333'
//   [PASS] (VAL(1.0) / VAL(0.0)) => 'Infinity'
//   [PASS] (VAL(-1.0) / VAL(0.0)) => '-Infinity'
//   [PASS] (VAL(0.0) / VAL(0.0)) => 'NaN'*/

//   /* f32
//   [PASS] VAL(0.0)     => '0.0'
//   [PASS] VAL(-0.0)    => '-0.0'
//   [PASS] VAL(1.0)     => '1.0'
//   [PASS] VAL(-1.0)    => '-1.0'
//   [PASS] VAL(256.345) => '256.345'
//   [PASS] VAL(0.256)   => '0.256'
//   [PASS] VAL(65504.0) => '65504.0'
//   [PASS] VAL(-65504.0) => '-65504.0'
//   [FAIL] VAL(0.00006103515625) => Got: '0.000061035156' | Expected: '0.00006103515625'
//   [FAIL] VAL(0.000000059604644775390625) => Got: '0.000000059604645' | Expected: '0.000000059604644775390625'
//   [PASS] VAL(0.1)     => '0.1'
//   [PASS] VAL(0.3)     => '0.3'
//   [FAIL] (VAL(1.0) / VAL(3.0)) => Got: '0.33333334' | Expected: '0.3333333333333333'
//   [PASS] (VAL(1.0) / VAL(0.0)) => 'Infinity'
//   [PASS] (VAL(-1.0) / VAL(0.0)) => '-Infinity'
//   [PASS] (VAL(0.0) / VAL(0.0)) => 'NaN'*/

//   /* f64
//   [PASS] 0.0          => '0.0'
//   [PASS] -0.0         => '-0.0'
//   [PASS] 1.0          => '1.0'
//   [PASS] -1.0         => '-1.0'
//   [PASS] 256.345      => '256.345'
//   [PASS] 0.256        => '0.256'
//   [PASS] 65504.0      => '65504.0'
//   [PASS] -65504.0     => '-65504.0'
//   [PASS] 0.00006103515625 => '0.00006103515625'
//   [FAIL] 0.000000059604644775390625 => Got: '0.00000005960464477539063' | Expected: '0.000000059604644775390625'
//   [PASS] 0.1          => '0.1'
//   [PASS] 0.3          => '0.3'
//   [PASS] 1 / 3.0      => '0.3333333333333333'
//   [PASS] (1.0 / 0.0)  => 'Infinity'
//   [PASS] (-1.0 / 0.0) => '-Infinity'
//   [PASS] (0.0 / 0.0)  => 'NaN'*/
// }

int main(void) {
  // test_floating_formatting();

  Syx_Value *value = parse__syx_value_from_token((Syx_Token){.kind = SYX_TOKEN_KIND_BIN_FRC_LIT, .source = (String_View){.data = "10.10", .count = 6}}, NULL);
  // Syx_Value *value = parse__syx_value_from_token((Syx_Token){.kind = SYX_TOKEN_KIND_OCT_FRC_LIT, .source = (String_View){.data = "127.40", .count = 6}}, NULL);
  // Syx_Value *value = parse__syx_value_from_token((Syx_Token){.kind = SYX_TOKEN_KIND_DEC_FRC_LIT, .source = (String_View){.data = "128.50", .count = 6}}, NULL);
  // Syx_Value *value = parse__syx_value_from_token((Syx_Token){.kind = SYX_TOKEN_KIND_HEX_FRC_LIT, .source = (String_View){.data = "1F.80", .count = 6}}, NULL);

  return 0;
}
