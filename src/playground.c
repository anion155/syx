#include <stdio.h>

#define SYX_VALUE_IMPL
#include "syx_value.h"

int main() {
  SyxV *s1 = make_syxv_symbol_cstr("test");
  printf("GG %zu\n", get_syxv_string_width(s1));
  return 0;
}
