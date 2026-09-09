#include <stdio.h>
#define SB_NUMBER_IMPL
#include <sb_number.h>

int main(void) {
  {
    String str = stringify(sb_append_number, UINT8_MAX);
    printf("result='" SV_FMT "'\n", sv_fmt_arg(str));
  }
  {
    String str = stringify(sb_append_number, UINT16_MAX);
    printf("result='" SV_FMT "'\n", sv_fmt_arg(str));
  }
  {
    String str = stringify(sb_append_number, UINT32_MAX);
    printf("result='" SV_FMT "'\n", sv_fmt_arg(str));
  }
  {
    String str = stringify(sb_append_number, UINT64_MAX);
    printf("result='" SV_FMT "'\n", sv_fmt_arg(str));
  }
  {
    String str = stringify(sb_append_number, UINT128_MAX);
    printf("result='" SV_FMT "'\n", sv_fmt_arg(str));
  }

  {
    String str = stringify(sb_append_number, (_Float16)0.3);
    printf("result='" SV_FMT "'\n", sv_fmt_arg(str));
  }
  {
    String str = stringify(sb_append_number, (_Float16)0.256);
    printf("result='" SV_FMT "'\n", sv_fmt_arg(str));
  }
  {
    String str = stringify(sb_append_number, (_Float16)10000);
    printf("result='" SV_FMT "'\n", sv_fmt_arg(str));
  }

  {
    String str = stringify(sb_append_number, (float)0.3);
    printf("result='" SV_FMT "'\n", sv_fmt_arg(str));
  }
  {
    String str = stringify(sb_append_number, (float)0.256);
    printf("result='" SV_FMT "'\n", sv_fmt_arg(str));
  }
  {
    String str = stringify(sb_append_number, (float)100000);
    printf("result='" SV_FMT "'\n", sv_fmt_arg(str));
  }

  {
    String str = stringify(sb_append_number, (double)0.3);
    printf("result='" SV_FMT "'\n", sv_fmt_arg(str));
  }
  {
    String str = stringify(sb_append_number, (double)0.256);
    printf("result='" SV_FMT "'\n", sv_fmt_arg(str));
  }
  {
    String str = stringify(sb_append_number, (double)100000);
    printf("result='" SV_FMT "'\n", sv_fmt_arg(str));
  }

  return 0;
}
