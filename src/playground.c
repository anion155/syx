#include <stdio.h>
#define SB_NUMBER_IMPL
#include <sb_number.h>

int main(void) {
  String_Builder sb;
  sb = stringify(sb_append_floating, (double)0.256);
  printf("result='" SV_FMT "'\n", sv_fmt_arg(sb));
  sb = stringify(sb_append_floating, (double)100000);
  printf("result='" SV_FMT "'\n", sv_fmt_arg(sb));
  return 0;
}
