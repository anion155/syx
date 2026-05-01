#include <stdio.h>

#define SYX_VALUE_IMPL
#include "syx_value.h"

int main() {
  UNUSED(ht__put);
  UNUSED(ht__find);
  UNUSED(ht__find_or_put);
  UNUSED(ht__delete);
  UNUSED(ht__find_and_delete);
  UNUSED(ht__key);
  UNUSED(ht__next);
  UNUSED(ht__reset);
  UNUSED(ht__free);
  UNUSED(ht__find_slot);
  UNUSED(ht__put_no_expand);
  UNUSED(ht__expand);
  UNUSED(ht__strlen);
  UNUSED(ht__strcmp);
  UNUSED(ht__memcpy);
  UNUSED(ht__memcmp);

  printf("%s\n", stringify_syxv(make_syxv_nil()).data);
  printf("%s\n", stringify_syxv(make_syxv_symbol_cstr("test")).data);
  printf("%s\n", stringify_syxv(make_syxv_pair(make_syxv_symbol_cstr("test"), make_syxv_nil())).data);
  printf("%s\n", stringify_syxv(make_syxv_pair(make_syxv_symbol_cstr("test"), make_syxv_integer(99))).data);
  printf("%s\n", stringify_syxv(make_syxv_bool(true)).data);
  printf("%s\n", stringify_syxv(make_syxv_integer(123)).data);
  printf("%s\n", stringify_syxv(make_syxv_integer(-123)).data);
  printf("%s\n", stringify_syxv(make_syxv_fractional(123.4560)).data);
  printf("%s\n", stringify_syxv(make_syxv_fractional(-123.456)).data);
  printf("%s\n", stringify_syxv(make_syxv_string_cstr("test")).data);
  printf("%s\n", stringify_syxv(make_syxv_quote(make_syxv_string_cstr("test"))).data);
  printf("%s\n", stringify_syxv(make_syxv_specialf("test", NULL)).data);
  printf("%s\n", stringify_syxv(make_syxv_builtin("test", NULL)).data);
  printf("%s\n", stringify_syxv(make_syxv_closure("test",
                                                  make_syxv_nil(),
                                                  make_syxv_nil(),
                                                  NULL))
                     .data);
  printf("%s\n", stringify_syxv(make_syxv_closure("test",
                                                  make_syxv_pair(
                                                      make_syxv_symbol_cstr("a"),
                                                      make_syxv_pair(
                                                          make_syxv_symbol_cstr("b"),
                                                          make_syxv_nil())),
                                                  make_syxv_pair(
                                                      make_syxv_integer(5),
                                                      make_syxv_pair(
                                                          make_syxv_pair(
                                                              make_syxv_symbol_cstr("a"),
                                                              make_syxv_pair(
                                                                  make_syxv_symbol_cstr("b"),
                                                                  make_syxv_nil())),
                                                          make_syxv_nil())),
                                                  NULL))
                     .data);
  return 0;
}
