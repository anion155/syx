#include <stdio.h>

#define SYX_VALUE_IMPL
#include "syx_value.h"

int main() {
  SyxV *s1 = make_syxv_closure(NULL,
                               make_syxv_pair(make_syxv_symbol_cstr("a"), make_syxv_pair(make_syxv_symbol_cstr("b"), make_syxv_nil())),
                               make_syxv_pair(make_syxv_symbol_cstr("a"), make_syxv_nil()),
                               NULL);
  String_Builder sb = stringify_syxv(s1);
  printf("GG %s\n", sb.items);
  return 0;
}
