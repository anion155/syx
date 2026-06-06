#include <stdio.h>

#define SYX_VALUE_IMPL
#include "syx_value.h"

void test_fractional_stringify(syx_fractional_t value) {
  {
    printf("GG %zu\n", str_append_fractional(NULL, value));
  }
  {
    syx_string_t sb = {0};
    size_t count = str_append_fractional(&sb, value);
    printf("GG {.count = %zu, .capacity = %zu, .items = \"%s\"} count = %zu\n", sb.count, sb.capacity, sb.items, count);
  }
  {
    syx_string_t sb = stringify(str_append_fractional, value);
    printf("GG {.count = %zu, .capacity = %zu, .items = \"%s\"}\n", sb.count, sb.capacity, sb.items);
  }
  {
    syx_string_view_t sv = stringify_temp(str_append_fractional, value);
    printf("GG {.count = %zu, .data = \"%s\"}\n", sv.count, sv.data);
  }
}

void test_integer_stringify(syx_integer_t value) {
  {
    printf("GG %zu\n", str_append_integer(NULL, value));
  }
  {
    syx_string_t sb = {0};
    size_t count = str_append_integer(&sb, value);
    printf("GG {.count = %zu, .capacity = %zu, .items = \"%s\"} count = %zu\n", sb.count, sb.capacity, sb.items, count);
  }
  {
    syx_string_t sb = stringify(str_append_integer, value);
    printf("GG {.count = %zu, .capacity = %zu, .items = \"%s\"}\n", sb.count, sb.capacity, sb.items);
  }
  {
    syx_string_view_t sv = stringify_temp(str_append_integer, value);
    printf("GG {.count = %zu, .data = \"%s\"}\n", sv.count, sv.data);
  }
}

int main() {
  // test_integer_stringify(1234);
  // test_integer_stringify(-1234);
  // test_fractional_stringify(1234.5678);
  // test_fractional_stringify(-1234.5678);
  return 0;
}
