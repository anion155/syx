#include <stdio.h>

#define SYX_VALUE_IMPL
#include "syx_value.h"

int main() {
  SyxV *s1 = make_syxv_closure(NULL,
                               make_syxv_pair(make_syxv_symbol_cstr("a"), make_syxv_pair(make_syxv_symbol_cstr("b"), make_syxv_nil())),
                               make_syxv_pair(make_syxv_symbol_cstr("a"), make_syxv_nil()),
                               NULL);
  String_View sv = stringify_syxv(s1);
  printf("GG " SV_Fmt "\n", SV_Arg(sv));
  return 0;
}
