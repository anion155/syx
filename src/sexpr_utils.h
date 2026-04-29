#ifndef SEXPR_UTILS_H
#define SEXPR_UTILS_H

#include <ht.h>
#include <magic.h>
#include <nob.h>

int islineend(int c);
int issymbol_special(int c);
int issymbol(int c);
int isoctal(int c);
int ishex(int c);
int hex_to_int(int c);

void da_destructor(void *data);

typedef bool sexpr_bool_t;
typedef long long int sexpr_int_t;
typedef long double sexpr_real_t;
typedef char *sexpr_string_t;

bool parse_integer(String_View *sv, sexpr_int_t *result);
bool parse_fractions(String_View *sv, sexpr_real_t *result);
bool parse_real(String_View *sv, sexpr_real_t *result);

size_t int_width(sexpr_int_t value);
void stringify_int_n(sexpr_int_t value, size_t length, char *string);
String_View stringify_int(sexpr_int_t value);

#define REAL_MINIMAL_DIFFERENCE 1e-9
#define MAX_REAL_FRACTIONAL_WIDTH 15

size_t real_fraction_width(sexpr_real_t value);
size_t real__precision(sexpr_real_t value, ssize_t precision);
#define real_precision(value, ...) real__precision((value), WITH_DEFAULT(-MAX_REAL_FRACTIONAL_WIDTH, __VA_ARGS__))
size_t real_width(sexpr_real_t value, size_t precision);
void stringify_real_n(sexpr_real_t value, size_t integer_width, size_t precision, char *string);
String_View stringify__real(sexpr_real_t value, ssize_t precision);
#define stringify_real(value, ...) stringify__real((value), WITH_DEFAULT(-MAX_REAL_FRACTIONAL_WIDTH, __VA_ARGS__))

#define define_constants_ht(name, data_type)        \
  typedef Ht(char *, data_type, name##_t) name##_t; \
  name##_t _##name = {.hasheq = ht_cstr_hasheq};    \
  void make_##name(name##_t *name);                 \
  name##_t *name() {                                \
    if (_##name.count) return &_##name;             \
    make_##name(&_##name);                          \
    return &_##name;                                \
  }                                                 \
  void make_##name(name##_t *name)

struct escape_char_print {
  void *data;
  int (*get_next_char)(void *data);
  int (*revert_next_char)(void *data);
};

size_t syx_putc(FILE *fd, char char_v);
size_t syx_putc_escaped(FILE *fd, struct escape_char_print escape);
int gat_char_n(void *_data);
size_t syx_puts_n(FILE *fd, const char *str, size_t n);
size_t syx_puts(FILE *fd, String_View sv);
int gat_char_cstr(void *_data);
size_t syx_puts_cstr(FILE *fd, const char *str);
ssize_t syx_put_sv_diff(FILE *fd, String_View *base, String_View *offset);

#endif // SEXPR_UTILS_H

#if defined(SEXPR_UTILS_IMPL) && !defined(SEXPR_UTILS_IMPL_C)
#define SEXPR_UTILS_IMPL_C

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

bool parse_integer(String_View *sv, sexpr_int_t *result) {
  bool is_negative = sv->data[0] == '-';
  if (is_negative) sv_chop_left(sv, 1);
  if (!sv->count) return false;
  *result = 0;
  size_t i = 0;
  while (isdigit(sv->data[i])) {
    *result = *result * 10 + (sv->data[i] - '0');
    i += 1;
    if (i >= sv->count) break;
  }
  sv->count -= i;
  sv->data += i;
  if (is_negative) *result *= -1;
  return true;
}

bool parse_fractions(String_View *sv, sexpr_real_t *result) {
  if (sv->data[0] != '.') return false;
  sv_chop_left(sv, 1);
  sexpr_int_t fractional_part = 0;
  sexpr_int_t exponent = 1;
  size_t i = 0;
  while (i < sv->count && isdigit(sv->data[i])) {
    fractional_part = fractional_part * 10 + (sv->data[i] - '0');
    exponent *= 10;
    i += 1;
  }
  sv->count -= i;
  sv->data += i;
  *result = (sexpr_real_t)fractional_part / (sexpr_real_t)exponent;
  return true;
}

bool parse_real(String_View *sv, sexpr_real_t *result) {
  sexpr_int_t integer_part = 0;
  if (!parse_integer(sv, &integer_part)) return false;
  if (sv->data[0] != '.') {
    *result = (sexpr_real_t)integer_part;
    return true;
  }
  if (!parse_fractions(sv, result)) return false;
  if (integer_part < 0) *result = integer_part - *result;
  else *result = integer_part + *result;
  return true;
}

size_t int_width(sexpr_int_t value) {
  size_t size = 0;
  for (sexpr_int_t it = value; it != 0; it = it / 10)
    size += 1;
  if (value == 0) size = 1;
  else if (value < 0) size += 1;
  return size;
}

void stringify_int_n(sexpr_int_t value, size_t length, char *string) {
  if (value < 0) {
    length -= 1;
    string[0] = '-';
    string += 1;
  }
  for (struct { size_t index; sexpr_int_t it; } s = {0, value}; s.index < length; s.index += 1, s.it /= 10) {
    string[length - s.index - 1] = '0' + (value < 0 ? -(s.it % 10) : s.it % 10);
  }
}

String_View stringify_int(sexpr_int_t value) {
  size_t length = int_width(value);
  char *string = malloc(length + 1);
  String_View result = {.data = string, .count = length};
  stringify_int_n(value, length, string);
  return result;
}

size_t real_fraction_width(sexpr_real_t value) {
  if (value < 0) value *= -1;
  value -= (sexpr_int_t)value;
  size_t width = 0;
  while (width < MAX_REAL_FRACTIONAL_WIDTH) {
    if (value < REAL_MINIMAL_DIFFERENCE) break;
    if (value > 1.0 - REAL_MINIMAL_DIFFERENCE) break;
    value *= 10;
    value -= (sexpr_int_t)value;
    width++;
  }
  return width;
}

size_t real__precision(sexpr_real_t value, ssize_t precision) {
  if (precision >= 0) return precision;
  size_t fractional_width = real_fraction_width(value);
  size_t exponenta = -precision;
  return fractional_width > exponenta ? exponenta : fractional_width;
}

size_t real_width(sexpr_real_t value, size_t precision) {
  size_t integer_width = int_width(value);
  if (precision == 0) return integer_width;
  return integer_width + 1 + precision;
}

void stringify_real_n(sexpr_real_t value, size_t integer_width, size_t precision, char *string) {
  sexpr_real_t round_const = value < 0 ? -0.5 : 0.5;
  if (precision == 0) {
    stringify_int_n((sexpr_int_t)value + round_const, integer_width, string);
    return;
  }
  size_t width = integer_width + 1 + precision;
  stringify_int_n(value, integer_width, string);
  string[integer_width] = '.';
  string[width] = 0;
  sexpr_int_t exponent = 1;
  for (size_t index = 0; index < precision; index += 1)
    exponent *= 10;
  sexpr_int_t fractions = (sexpr_int_t)((value < 0 ? -value : value) * exponent + 0.5);
  stringify_int_n(fractions, precision, string + integer_width + 1);
  return;
}

String_View stringify__real(sexpr_real_t value, ssize_t precision) {
  sexpr_real_t round_const = value < 0 ? -0.5 : 0.5;
  if (precision == 0) return stringify_int((sexpr_int_t)value + round_const);
  size_t _precision = real__precision(value, precision);
  String_View res = {.count = real_width(value, _precision)};
  char *string = malloc(res.count + 1);
  res.data = string;
  stringify_real_n(value, int_width(value), _precision, string);
  return res;
}

size_t syx_putc(FILE *fd, char char_v) {
  if (fputc(char_v, fd) < 0) return 0;
  return 1;
}

size_t syx_put_octal_char(FILE *fd, char first, struct escape_char_print escape) {
  char output = first - '0';
  char c = escape.get_next_char(escape.data);
  for (size_t count = 1; c > 0 && isoctal(c) && count <= 3; c = escape.get_next_char(escape.data), count += 1) {
    output = (output << 3) + (c - '0');
  }
  syx_putc(fd, output);
  return 1;
}

size_t syx_put_hex_char(FILE *fd, struct escape_char_print escape) {
  char c = escape.get_next_char(escape.data);
  if (c < 0) return syx_putc(fd, 'x');
  if (!ishex(c)) return escape.revert_next_char(escape.data);
  char output = 0;
  while (ishex(c)) {
    output = (output << 4) + hex_to_int(c);
    c = escape.get_next_char(escape.data);
    if (c < 0) return syx_putc(fd, output);
  }
  return escape.revert_next_char(escape.data);
}

size_t syx_put_unicode_char(FILE *fd, char base, struct escape_char_print escape) {
  char u[4] = {0};
  u[0] = escape.get_next_char(escape.data);
  if (u[0] < 0) return syx_putc(fd, base);
  if (!ishex(u[0])) return (escape.revert_next_char(escape.data), escape.revert_next_char(escape.data));
  u[1] = escape.get_next_char(escape.data);
  if (u[1] < 0) return syx_putc(fd, base) && syx_putc(fd, u[0]);
  if (!ishex(u[1])) return (escape.revert_next_char(escape.data), escape.revert_next_char(escape.data), escape.revert_next_char(escape.data));
  u[2] = escape.get_next_char(escape.data);
  if (u[2] < 0) return syx_putc(fd, base) && syx_putc(fd, u[0]) && syx_putc(fd, u[1]);
  if (!ishex(u[2])) return (escape.revert_next_char(escape.data), escape.revert_next_char(escape.data), escape.revert_next_char(escape.data), escape.revert_next_char(escape.data));
  u[3] = escape.get_next_char(escape.data);
  if (u[3] < 0) return syx_putc(fd, base) && syx_putc(fd, u[0]) && syx_putc(fd, u[1]) && syx_putc(fd, u[2]);
  if (!ishex(u[3])) return (escape.revert_next_char(escape.data), escape.revert_next_char(escape.data), escape.revert_next_char(escape.data), escape.revert_next_char(escape.data), escape.revert_next_char(escape.data));
  if (base == 'U') TODO("Implement wide unicode");
  uint16_t codepoint = (hex_to_int(u[0]) << 12) | (hex_to_int(u[1]) << 8) | (hex_to_int(u[2]) << 4) | hex_to_int(u[3]);
  if (codepoint < 0x80) {
    return syx_putc(fd, codepoint);
  } else if (codepoint < 0x800) {
    return syx_putc(fd, 0xC0 | (codepoint >> 6)) && syx_putc(fd, 0x80 | (codepoint & 0x3F));
  } else {
    return syx_putc(fd, 0xE0 | (codepoint >> 12)) && syx_putc(fd, 0x80 | ((codepoint >> 6) & 0x3F)) && syx_putc(fd, 0x80 | (codepoint & 0x3F));
  }
}

size_t syx_putc_escaped(FILE *fd, struct escape_char_print escape) {
  int c = escape.get_next_char(escape.data);
  if (c < 0) return 0;
  if (c != '\\') return syx_putc(fd, c);
  c = escape.get_next_char(escape.data);
  if (c < 0) return syx_putc(fd, '\\');
  switch (c) {
    case 'a': return syx_putc(fd, '\a');
    case 'b': return syx_putc(fd, '\b');
    case 'e': return syx_putc(fd, '\e');
    case 'f': return syx_putc(fd, '\f');
    case 'n': return syx_putc(fd, '\n');
    case 'r': return syx_putc(fd, '\r');
    case 't': return syx_putc(fd, '\t');
    case 'v': return syx_putc(fd, '\v');
    case '\\': return syx_putc(fd, '\\');
    case '\'': return syx_putc(fd, '\'');
    case '"': return syx_putc(fd, '\"');
    case '?': return syx_putc(fd, '\?');
    case 'x': return syx_put_hex_char(fd, escape);
    case 'u': return syx_put_unicode_char(fd, 'u', escape);
    case 'U': return syx_put_unicode_char(fd, 'U', escape);
  }
  if (isoctal(c)) return syx_put_octal_char(fd, c, escape);
  return syx_putc(fd, c);
}

struct gat_char_data {
  const char *data;
  size_t count;
  size_t index;
};

int gat_char_n(void *_data) {
  struct gat_char_data *data = _data;
  if (data->index >= data->count) return -1;
  return data->data[data->index++];
}

int revert_char_n(void *_data) {
  struct gat_char_data *data = _data;
  data->index -= 1;
  return 1;
}

size_t syx_puts_n(FILE *fd, const char *str, size_t n) {
  struct gat_char_data data = {.data = str, .count = n, .index = 0};
  struct escape_char_print escape = {.data = &data, .get_next_char = gat_char_n, .revert_next_char = revert_char_n};
  while (data.index < n) {
    if (!syx_putc_escaped(fd, escape)) return data.index;
  }
  return n;
}

size_t syx_puts(FILE *fd, String_View sv) {
  struct gat_char_data data = {.data = sv.data, .count = sv.count, .index = 0};
  struct escape_char_print escape = {.data = &data, .get_next_char = gat_char_n, .revert_next_char = revert_char_n};
  while (data.index < sv.count) {
    if (!syx_putc_escaped(fd, escape)) return data.index;
  }
  return data.count;
}

struct gat_char_cstr_data {
  const char *cstr;
  size_t index;
};

int gat_char_cstr(void *_data) {
  struct gat_char_cstr_data *data = _data;
  char c = data->cstr[data->index];
  if (c == 0) return -1;
  return c;
}

int revert_char_cstr(void *_data) {
  struct gat_char_cstr_data *data = _data;
  data->index -= 1;
  return 1;
}

size_t syx_puts_cstr(FILE *fd, const char *str) {
  struct gat_char_cstr_data data = {.cstr = str, .index = 0};
  struct escape_char_print escape = {.data = &data, .get_next_char = gat_char_cstr, .revert_next_char = revert_char_cstr};
  while (str[data.index]) {
    if (!syx_putc_escaped(fd, escape)) return data.index;
  }
  return data.index;
}

ssize_t syx_put_sv_diff(FILE *fd, String_View *base, String_View *offset) {
  ptrdiff_t diff = offset->data - base->data;
  if (diff <= 0) return 0;
  if (!syx_puts_n(fd, base->data, offset->data - base->data)) return -1;
  *base = *offset;
  return diff;
}

#endif // SEXPR_UTILS_IMPL
