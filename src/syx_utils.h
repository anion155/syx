#ifndef SYX_UTILS_H
#define SYX_UTILS_H

#include <ht.h>
#include <magic.h>
#include <nob.h>
#include <rc.h>

int islineend(int c);
int issymbol_special(int c);
int issymbol(int c);
int isoctal(int c);
int ishex(int c);
int hex_to_int(int c);

void da_destructor(void *data);

typedef bool syx_bool_t;
typedef long long int syx_integer_t;
typedef long double syx_fractional_t;

typedef String_View syx_string_view_t;
typedef String_Builder syx_string_t;

bool parse_integer(String_View *sv, syx_integer_t *result);
bool parse_fractions(String_View *sv, syx_fractional_t *result);
bool parse_fractional(String_View *sv, syx_fractional_t *result);

size_t get_integer_string_width(syx_integer_t value);
void stringify_integer_n(syx_integer_t value, size_t length, char *string);
syx_string_t stringify_integer(syx_integer_t value);

#define FRAC_MINIMAL_DIFFERENCE 1e-9
#define MAX_FRAC_FRACTIONAL_WIDTH 15

size_t get_fractions_string_width(syx_fractional_t value);
size_t fractions__precision(syx_fractional_t value, ssize_t precision);
#define fractions_precision(value, ...) fractions__precision((value), WITH_DEFAULT(-MAX_FRAC_FRACTIONAL_WIDTH, __VA_ARGS__))

size_t get_fractional_string_width(syx_fractional_t value, size_t precision);
void stringify_fractional_n(syx_fractional_t value, size_t integer_width, size_t precision, char *string);
syx_string_t stringify__fractional(syx_fractional_t value, ssize_t precision);
#define stringify_fractional(value, ...) stringify__fractional((value), WITH_DEFAULT(-MAX_FRAC_FRACTIONAL_WIDTH, __VA_ARGS__))

#define define_constant(type, name)                \
  typedef type name##_t;                           \
  typedef struct {                                 \
    size_t initialized;                            \
    name##_t data;                                 \
  } name##_w;                                      \
  name##_w _##name = {0};                          \
  void make_##name(name##_t *name);                \
  name##_t *name() {                               \
    if (_##name.initialized) return &_##name.data; \
    _##name.initialized = 1;                       \
    make_##name(&_##name.data);                    \
    return &_##name.data;                          \
  }                                                \
  void make_##name(name##_t *name)

struct escape_char_print {
  void *data;
  int (*get_next_char)(void *data);
  int (*revert_next_char)(void *data, size_t n);
};

size_t io_putc(FILE *fd, char char_v);
size_t io_putc_escaped(FILE *fd, struct escape_char_print escape);
int io_char_n(void *_data);
size_t io_puts_n(FILE *fd, const char *str, size_t n);
size_t io_puts(FILE *fd, String_View sv);
size_t io_puts_cstr(FILE *fd, const char *str);

String_Builder sb_copy_from_cstr(const char *string);
String_Builder sb_copy_from_sv(String_View sv);

#endif // SYX_UTILS_H

#if defined(SYX_UTILS_IMPL) && !defined(SYX_UTILS_IMPL_C)
#define SYX_UTILS_IMPL_C

int islineend(int c) {
  return (c == '\n' || c == '\r');
}

int issymbol_special(int c) {
  return (
      c == '#' || c == '?' || c == '@' || c == '!' || c == '$' || c == '+' || c == '-' || c == '*' || c == '/' || c == '=' || c == '<' || c == '>');
}

int issymbol(int c) {
  return c == '-' || issymbol_special(c) || isalnum(c);
}

int isoctal(int c) {
  return c >= '0' && c < '8';
}

int ishex(int c) {
  return isdigit(c) || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
}

int hex_to_int(int c) {
  if (isdigit(c)) return c - '0';
  else if (c >= 'a' && c <= 'f') return c - 'a';
  else if (c >= 'A' && c <= 'F') return c - 'A';
  return -1;
}

struct Nob_Dynamic_Array__Abstract {
  void **items;
  size_t count;
  size_t capacity;
};

void da_destructor(void *data) {
  struct Nob_Dynamic_Array__Abstract *da = data;
  da_foreach(void *, it, da) rc_release(*it);
  da_free(*da);
}

bool parse_integer(String_View *sv, syx_integer_t *result) {
  bool is_negative = sv->data[0] == '-';
  if (is_negative) sv_chop_left(sv, 1);
  if (!sv->count) return false;
  *result = 0;
  size_t i = 0;
  while (isdigit(sv->data[i])) {
    *result = *result * 10 + (sv->data[i] - '0');
    i += 1;
    if (i >= sv->count) break;
    // TODO: implement number separators: 100_000_000;
  }
  sv->count -= i;
  sv->data += i;
  if (is_negative) *result *= -1;
  return true;
}

bool parse_fractions(String_View *sv, syx_fractional_t *result) {
  if (sv->data[0] != '.') return false;
  sv_chop_left(sv, 1);
  syx_integer_t fractional_part = 0;
  syx_integer_t exponent = 1;
  size_t i = 0;
  while (i < sv->count && isdigit(sv->data[i])) {
    fractional_part = fractional_part * 10 + (sv->data[i] - '0');
    exponent *= 10;
    i += 1;
  }
  sv->count -= i;
  sv->data += i;
  *result = (syx_fractional_t)fractional_part / (syx_fractional_t)exponent;
  return true;
}

bool parse_fractional(String_View *sv, syx_fractional_t *result) {
  syx_integer_t integer_part = 0;
  if (!parse_integer(sv, &integer_part)) return false;
  if (sv->data[0] != '.') {
    *result = (syx_fractional_t)integer_part;
    return true;
  }
  if (!parse_fractions(sv, result)) return false;
  if (integer_part < 0) *result = integer_part - *result;
  else *result = integer_part + *result;
  return true;
}

size_t get_integer_string_width(syx_integer_t value) {
  size_t size = 0;
  for (syx_integer_t it = value; it != 0; it = it / 10)
    size += 1;
  if (value == 0) size = 1;
  else if (value < 0) size += 1;
  return size;
}

void stringify_integer_n(syx_integer_t value, size_t length, char *string) {
  if (value < 0) {
    length -= 1;
    string[0] = '-';
    string += 1;
  }
  size_t index = 0;
  syx_integer_t it = value;
  for (; index < length; index += 1, it /= 10) {
    string[length - index - 1] = '0' + (value < 0 ? -(it % 10) : it % 10);
  }
}

syx_string_t stringify_integer(syx_integer_t value) {
  size_t length = get_integer_string_width(value);
  char *string = malloc(length + 1);
  syx_string_t result = {.items = string, .count = length, .capacity = length + 1};
  stringify_integer_n(value, length, string);
  return result;
}

size_t get_fractions_string_width(syx_fractional_t value) {
  if (value < 0) value *= -1;
  value -= (syx_integer_t)value;
  size_t width = 0;
  while (width < MAX_FRAC_FRACTIONAL_WIDTH) {
    if (value < FRAC_MINIMAL_DIFFERENCE) break;
    if (value > 1.0 - FRAC_MINIMAL_DIFFERENCE) break;
    value *= 10;
    value -= (syx_integer_t)value;
    width++;
  }
  return width;
}

size_t fractions__precision(syx_fractional_t value, ssize_t precision) {
  if (precision >= 0) return precision;
  size_t fractional_width = get_fractions_string_width(value);
  size_t exponenta = -precision;
  return fractional_width > exponenta ? exponenta : fractional_width;
}

size_t get_fractional_string_width(syx_fractional_t value, size_t precision) {
  size_t integer_width = get_integer_string_width(value);
  if (precision == 0) return integer_width;
  return integer_width + 1 + precision;
}

void stringify_fractional_n(syx_fractional_t value, size_t integer_width, size_t precision, char *string) {
  syx_fractional_t round_const = value < 0 ? -0.5 : 0.5;
  if (precision == 0) {
    stringify_integer_n((syx_integer_t)value + round_const, integer_width, string);
    return;
  }
  size_t width = integer_width + 1 + precision;
  stringify_integer_n(value, integer_width, string);
  string[integer_width] = '.';
  string[width] = 0;
  syx_integer_t exponent = 1;
  for (size_t index = 0; index < precision; index += 1)
    exponent *= 10;
  syx_integer_t fractions = (syx_integer_t)((value < 0 ? -value : value) * exponent + 0.5);
  stringify_integer_n(fractions, precision, string + integer_width + 1);
  return;
}

syx_string_t stringify__fractional(syx_fractional_t value, ssize_t precision) {
  syx_fractional_t round_const = value < 0 ? -0.5 : 0.5;
  if (precision == 0) return stringify_integer((syx_integer_t)value + round_const);
  size_t _precision = fractions__precision(value, precision);
  syx_string_t res = {.count = get_fractional_string_width(value, _precision)};
  res.capacity = res.count + 1;
  res.items = malloc(res.count + 1);
  stringify_fractional_n(value, get_integer_string_width(value), _precision, res.items);
  return res;
}

size_t io_putc(FILE *fd, char char_v) {
  if (fputc(char_v, fd) < 0) return 0;
  return 1;
}

size_t syx_put_octal_char(FILE *fd, char first, struct escape_char_print escape) {
  char output = first - '0';
  char c = escape.get_next_char(escape.data);
  for (size_t count = 1; c > 0 && isoctal(c) && count <= 3; c = escape.get_next_char(escape.data), count += 1) {
    output = (output << 3) + (c - '0');
  }
  io_putc(fd, output);
  return 1;
}

size_t syx_put_hex_char(FILE *fd, struct escape_char_print escape) {
  char c = escape.get_next_char(escape.data);
  if (c < 0 || !ishex(c)) return escape.revert_next_char(escape.data, 1);
  char output = 0;
  while (ishex(c)) {
    output = (output << 4) + hex_to_int(c);
    c = escape.get_next_char(escape.data);
    if (c < 0) return io_putc(fd, output);
  }
  return io_putc(fd, output);
}

size_t syx_put_unicode_char(FILE *fd, char base, struct escape_char_print escape) {
  char u[4] = {0};
  u[0] = escape.get_next_char(escape.data);
  if (u[0] < 0 || !ishex(u[0])) return escape.revert_next_char(escape.data, 2);
  u[1] = escape.get_next_char(escape.data);
  if (u[1] < 0 || !ishex(u[1])) return escape.revert_next_char(escape.data, 3);
  u[2] = escape.get_next_char(escape.data);
  if (u[2] < 0 || !ishex(u[2])) return escape.revert_next_char(escape.data, 4);
  u[3] = escape.get_next_char(escape.data);
  if (u[3] < 0 || !ishex(u[3])) return escape.revert_next_char(escape.data, 5);
  if (base == 'u') goto print_u;
  if (base == 'U') {
    char U[4] = {0};
    U[0] = escape.get_next_char(escape.data);
    if (U[0] < 0 || !ishex(U[0])) {
      escape.revert_next_char(escape.data, 1);
      goto print_u;
    }
    U[1] = escape.get_next_char(escape.data);
    if (U[1] < 0 || !ishex(U[1])) {
      escape.revert_next_char(escape.data, 2);
      goto print_u;
    }
    U[2] = escape.get_next_char(escape.data);
    if (U[2] < 0 || !ishex(U[2])) {
      escape.revert_next_char(escape.data, 3);
      goto print_u;
    }
    U[3] = escape.get_next_char(escape.data);
    if (U[3] < 0 || !ishex(U[3])) {
      escape.revert_next_char(escape.data, 4);
      goto print_u;
    }
    uint16_t high = (hex_to_int(u[0]) << 12) | (hex_to_int(u[1]) << 8) | (hex_to_int(u[2]) << 4) | hex_to_int(u[3]);
    uint16_t low = (hex_to_int(U[0]) << 12) | (hex_to_int(U[1]) << 8) | (hex_to_int(U[2]) << 4) | hex_to_int(U[3]);
    uint32_t codepoint = ((uint32_t)high << 16) | low;
    if (codepoint < 0x80) {
      return io_putc(fd, codepoint);
    } else if (codepoint < 0x800) {
      return io_putc(fd, 0xC0 | (codepoint >> 6)) && io_putc(fd, 0x80 | (codepoint & 0x3F));
    } else if (codepoint < 0x10000) {
      return io_putc(fd, 0xE0 | (codepoint >> 12)) && io_putc(fd, 0x80 | ((codepoint >> 6) & 0x3F)) && io_putc(fd, 0x80 | (codepoint & 0x3F));
    } else {
      return io_putc(fd, 0xF0 | (codepoint >> 18)) && io_putc(fd, 0x80 | ((codepoint >> 12) & 0x3F)) && io_putc(fd, 0x80 | ((codepoint >> 6) & 0x3F)) && io_putc(fd, 0x80 | (codepoint & 0x3F));
    }
  }
  UNREACHABLE("unknown base");
print_u:
  uint16_t codepoint = (hex_to_int(u[0]) << 12) | (hex_to_int(u[1]) << 8) | (hex_to_int(u[2]) << 4) | hex_to_int(u[3]);
  if (codepoint < 0x80) {
    return io_putc(fd, codepoint);
  } else if (codepoint < 0x800) {
    return io_putc(fd, 0xC0 | (codepoint >> 6)) && io_putc(fd, 0x80 | (codepoint & 0x3F));
  } else {
    return io_putc(fd, 0xE0 | (codepoint >> 12)) && io_putc(fd, 0x80 | ((codepoint >> 6) & 0x3F)) && io_putc(fd, 0x80 | (codepoint & 0x3F));
  }
}

size_t io_putc_escaped(FILE *fd, struct escape_char_print escape) {
  int c = escape.get_next_char(escape.data);
  if (c < 0) return 0;
  if (c != '\\') return io_putc(fd, c);
  c = escape.get_next_char(escape.data);
  if (c < 0) return io_putc(fd, '\\');
  switch (c) {
    case 'a': return io_putc(fd, '\a');
    case 'b': return io_putc(fd, '\b');
    case 'e': return io_putc(fd, '\e');
    case 'f': return io_putc(fd, '\f');
    case 'n': return io_putc(fd, '\n');
    case 'r': return io_putc(fd, '\r');
    case 't': return io_putc(fd, '\t');
    case 'v': return io_putc(fd, '\v');
    case '\\': return io_putc(fd, '\\');
    case '\'': return io_putc(fd, '\'');
    case '"': return io_putc(fd, '\"');
    case '?': return io_putc(fd, '\?');
    case 'x': return syx_put_hex_char(fd, escape);
    case 'u': return syx_put_unicode_char(fd, 'u', escape);
    case 'U': return syx_put_unicode_char(fd, 'U', escape);
  }
  if (isoctal(c)) return syx_put_octal_char(fd, c, escape);
  return io_putc(fd, c);
}

struct gat_char_data {
  const char *data;
  size_t count;
  size_t index;
};

int io_char_n(void *_data) {
  struct gat_char_data *data = _data;
  if (data->index >= data->count) return -1;
  return data->data[data->index++];
}

int revert_char_n(void *_data, size_t n) {
  struct gat_char_data *data = _data;
  data->index -= n;
  return 1;
}

size_t io_puts_n(FILE *fd, const char *str, size_t n) {
  struct gat_char_data data = {.data = str, .count = n, .index = 0};
  struct escape_char_print escape = {.data = &data, .get_next_char = io_char_n, .revert_next_char = revert_char_n};
  while (data.index < n) {
    if (!io_putc_escaped(fd, escape)) return data.index;
  }
  return n;
}

size_t io_puts(FILE *fd, String_View sv) {
  struct gat_char_data data = {.data = sv.data, .count = sv.count, .index = 0};
  struct escape_char_print escape = {.data = &data, .get_next_char = io_char_n, .revert_next_char = revert_char_n};
  while (data.index < sv.count) {
    if (!io_putc_escaped(fd, escape)) return data.index;
  }
  return data.count;
}

struct get_char_cstr_data {
  const char *cstr;
  size_t index;
};

int get_char_cstr(void *_data) {
  struct get_char_cstr_data *data = _data;
  char c = data->cstr[data->index];
  if (c == 0) return -1;
  return c;
}

int revert_char_cstr(void *_data, size_t n) {
  struct get_char_cstr_data *data = _data;
  data->index -= n;
  return 1;
}

size_t io_puts_cstr(FILE *fd, const char *str) {
  struct get_char_cstr_data data = {.cstr = str, .index = 0};
  struct escape_char_print escape = {.data = &data, .get_next_char = get_char_cstr, .revert_next_char = revert_char_cstr};
  while (str[data.index]) {
    if (!io_putc_escaped(fd, escape)) return data.index;
  }
  return data.index;
}

String_Builder sb_copy_from_cstr(const char *string) {
  String_Builder sb = {0};
  sb_append_cstr(&sb, string);
  return sb;
}

String_Builder sb_copy_from_sv(String_View sv) {
  String_Builder sb = {0};
  sb_append_sv(&sb, sv);
  return sb;
}

#endif // SYX_UTILS_IMPL
