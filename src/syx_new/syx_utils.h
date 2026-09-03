#ifndef SYX_UTILS_H
#define SYX_UTILS_H

#define syx_define_constant(type, name)              \
  typedef type name##_t;                             \
  typedef struct {                                   \
    size_t initialized;                              \
    name##_t data;                                   \
  } name##_w;                                        \
  name##_w w_##name = {0};                           \
  void make_##name(name##_t *name);                  \
  inline name##_t *name() {                          \
    if (w_##name.initialized) return &w_##name.data; \
    w_##name.initialized = 1;                        \
    make_##name(&w_##name.data);                     \
    return &w_##name.data;                           \
  }                                                  \
  void make_##name(name##_t *name)

int syx_utils_is_decimal_digit(int character);
int syx_utils_is_hex_digit(int character);
int syx_utils_hex_to_decimal(int c);

#endif // SYX_UTILS_H

#define SYX_UTILS_IMPL
#if defined(SYX_UTILS_IMPL) && !defined(SYX_UTILS_IMPL_C)
#define SYX_UTILS_IMPL_C

int syx_utils_is_decimal_digit(int character) {
  switch (character) {
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
      return true;
    default: return false;
  }
}

int syx_utils_is_hex_digit(int character) {
  switch (character) {
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
    case 'a':
    case 'A':
    case 'b':
    case 'B':
    case 'c':
    case 'C':
    case 'd':
    case 'D':
    case 'e':
    case 'E':
    case 'f':
    case 'F':
      return true;
    default: return false;
  }
}

int syx_utils_hex_to_decimal(int c) {
  if (syx_utils_is_decimal_digit(c)) return c - '0';
  else if (c >= 'a' && c <= 'f') return (c - 'a') + 10;
  else if (c >= 'A' && c <= 'F') return (c - 'A') + 10;
  return -1;
}

#endif // SYX_UTILS_IMPL_C
