#include <stdio.h>
#define SB_NUMBER_IMPL
#include <sb_number.h>

int main(void) {
  {
    String str = stringify(sb_append_integer_u8, UINT8_MAX);
    printf("result='" SV_FMT "'\n", sv_fmt_arg(str));
  }
  {
    String str = stringify(sb_append_integer_u16, UINT16_MAX);
    printf("result='" SV_FMT "'\n", sv_fmt_arg(str));
  }
  {
    String str = stringify(sb_append_integer_u32, UINT32_MAX);
    printf("result='" SV_FMT "'\n", sv_fmt_arg(str));
  }
  {
    String str = stringify(sb_append_integer_u64, UINT64_MAX);
    printf("result='" SV_FMT "'\n", sv_fmt_arg(str));
  }
  {
    String str = stringify(sb_append_integer_u128, UINT128_MAX);
    printf("result='" SV_FMT "'\n", sv_fmt_arg(str));
  }
  // {
  //   String str = stringify(sb_append_double_hex, (float)0.3);
  //   printf("result='" SV_FMT "'\tactual='%a'\n", sv_fmt_arg(str), (float)0.3);
  // }
  // {
  //   String str = stringify(sb_append_float_hex, (float)0.256);
  //   printf("result='" SV_FMT "'\tactual='%a'\n", sv_fmt_arg(str), (float)0.256);
  // }
  // {
  //   String str = stringify(sb_append_float_hex, (float)100000);
  //   printf("result='" SV_FMT "'\tactual='%a'\n", sv_fmt_arg(str), (float)100000);
  // }
  // {
  //   String str = stringify(sb_append_double_hex, (double)0.3);
  //   printf("result='" SV_FMT "'\tactual='%a'\n", sv_fmt_arg(str), (double)0.3);
  // }
  // {
  //   String str = stringify(sb_append_double_hex, (double)0.256);
  //   printf("result='" SV_FMT "'\tactual='%a'\n", sv_fmt_arg(str), (double)0.256);
  // }
  // {
  //   String str = stringify(sb_append_double_hex, (double)100000);
  //   printf("result='" SV_FMT "'\tactual='%a'\n", sv_fmt_arg(str), (double)100000);
  // }
  return 0;
}
