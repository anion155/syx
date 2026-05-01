#include <stdio.h>

#define SYX_VALUE_IMPL
#include "syx_value.h"

int main() {
  SyxV *s1 = make_syxv_symbol_cstr("test");
  SyxV *s2 = make_syxv_symbol_cstr("test");
  printf("GG %s\n", s1 == s2 ? "true" : "false");
  return 0;
}
