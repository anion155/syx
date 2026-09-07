#include <stdio.h>
#define SB_NUMBER_IMPL
#include <sb_number.h>

int main(void) {
  String_Builder sb;
  sb = stringify(sb_append_double_hex, (float)0.3);
  printf("result='" SV_FMT "'\tactual='%a'\n", sv_fmt_arg(sb), (float)0.3);
  sb = stringify(sb_append_float_hex, (float)0.256);
  printf("result='" SV_FMT "'\tactual='%a'\n", sv_fmt_arg(sb), (float)0.256);
  sb = stringify(sb_append_float_hex, (float)100000);
  printf("result='" SV_FMT "'\tactual='%a'\n", sv_fmt_arg(sb), (float)100000);
  sb = stringify(sb_append_double_hex, (double)0.3);
  printf("result='" SV_FMT "'\tactual='%a'\n", sv_fmt_arg(sb), (double)0.3);
  sb = stringify(sb_append_double_hex, (double)0.256);
  printf("result='" SV_FMT "'\tactual='%a'\n", sv_fmt_arg(sb), (double)0.256);
  sb = stringify(sb_append_double_hex, (double)100000);
  printf("result='" SV_FMT "'\tactual='%a'\n", sv_fmt_arg(sb), (double)100000);
  return 0;
}
