#include <stdio.h>

#define SYX_VALUE_IMPL
#include "syx_value.h"

void test_integer_stringify(syx_integer_t value);
void test_fractional_stringify(syx_fractional_t value, ssize_t precision);

int main() {
  test_integer_stringify(1234);
  test_integer_stringify(-1234);

  test_fractional_stringify(1234, -MAX_FRAC_FRACTIONAL_WIDTH);
  test_fractional_stringify(123400, -MAX_FRAC_FRACTIONAL_WIDTH);
  test_fractional_stringify(1234.5678, -MAX_FRAC_FRACTIONAL_WIDTH);
  test_fractional_stringify(-1234.5678, -MAX_FRAC_FRACTIONAL_WIDTH);
  test_fractional_stringify(1000000.000001, -MAX_FRAC_FRACTIONAL_WIDTH);

  test_fractional_stringify(1234, 5);
  test_fractional_stringify(123400, 5);
  test_fractional_stringify(1234.5678, 5);
  test_fractional_stringify(-1234.5678, 5);
  test_fractional_stringify(1000000.000001, 5);
  test_fractional_stringify(-1234.5678, 2);

  test_fractional_stringify(-1234.5678, -4);
  test_fractional_stringify(-1234.5678, -5);
  test_fractional_stringify(-1234.5678, -2);
  test_fractional_stringify(-1234.0, -2);

  return 0;
}

void test_integer_stringify(syx_integer_t value) {
  printf("GG test_integer_stringify %lld\n", value);
  {
    printf("GG NULL count: %zu\n", str_append_integer(NULL, value));
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

void test_fractional_stringify(syx_fractional_t value, ssize_t precision) {
  printf("GG test_fractional_stringify %f %zd\n", value, precision);
  {
    printf("GG NULL count: %zu\n", str__append_fractional(NULL, value, precision));
  }
  {
    syx_string_t sb = {0};
    size_t count = str__append_fractional(&sb, value, precision);
    printf("GG {.count = %zu, .capacity = %zu, .items = \"%s\"} count = %zu\n", sb.count, sb.capacity, sb.items, count);
  }
  {
    syx_string_t sb = stringify(str__append_fractional, value, precision);
    printf("GG {.count = %zu, .capacity = %zu, .items = \"%s\"}\n", sb.count, sb.capacity, sb.items);
  }
  {
    syx_string_view_t sv = stringify_temp(str__append_fractional, value, precision);
    printf("GG {.count = %zu, .data = \"%s\"}\n", sv.count, sv.data);
  }
}
