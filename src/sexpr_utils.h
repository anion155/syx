#ifndef SEXPR_UTILS_H
#define SEXPR_UTILS_H

#include <nob.h>
#include <ht.h>
#include <magic.h>

int islineend(int c);
int issymbol_special(int c);
int issymbol(int c);

void da_destructor(void *data);

typedef bool sexpr_bool_t;
typedef long long int sexpr_int_t;
typedef long double sexpr_real_t;
typedef char * sexpr_string_t;

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
size_t stringify_real_n(sexpr_real_t value, size_t integer_width, size_t precision, char *string);
String_View stringify__real(sexpr_real_t value, ssize_t precision);
#define stringify_real(value, ...) stringify__real((value), WITH_DEFAULT(-MAX_REAL_FRACTIONAL_WIDTH, __VA_ARGS__))

// Expr *expr_convert_to_bool(Expr *expr);
// Expr *expr_convert_to_integer(Expr *expr);
// Expr *expr_convert_to_real(Expr *expr);
// Expr *expr_convert_to_string(Expr *expr);

#endif // SEXPR_UTILS_H

#if defined(SEXPR_UTILS_IMPL) && !defined(SEXPR_UTILS_IMPL_C)
#define SEXPR_UTILS_IMPL_C

int islineend(int c) {
  return (c == '\n' || c == '\r');
}
int issymbol_special(int c) {
  return (
       c == '#' || c == '?' || c == '@' || c == '!' || c == '$' || c == '+' || c == '-' || c == '*' || c == '/'
    || c == '=' || c == '<' || c == '>'
  );
}
int issymbol(int c) {
  return c == '-' || issymbol_special(c) || isalnum(c);
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
  sv->data  += i;
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
  if (i == 0) return false;
  sv->count -= i;
  sv->data  += i;
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
  for (sexpr_int_t it = value; it != 0; it = it / 10) size += 1;
  if (value == 0) size = 1;
  else if (value < 0) size += 1;
  return size;
}

void stringify_int_n(sexpr_int_t value, size_t length, char *string) {
  if (value < 0) string[0] = '-';
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
  value -= (sexpr_int_t)value;
  size_t width = 0;
  while (value > REAL_MINIMAL_DIFFERENCE && width < MAX_REAL_FRACTIONAL_WIDTH) {
    value *= 10;
    value -= (sexpr_int_t)value;
    width++;
    if (value < REAL_MINIMAL_DIFFERENCE || value > (1.0 - REAL_MINIMAL_DIFFERENCE)) break;
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
size_t stringify_real_n(sexpr_real_t value, size_t integer_width, size_t precision, char *string) {
  if (precision == 0) {
    stringify_int_n((sexpr_int_t)value + 0.5, integer_width, string);
    return integer_width;
  }
  size_t width = integer_width + 1 + precision;
  stringify_int_n(value, integer_width, string);
  string[integer_width] = '.';
  string[width] = 0;
  sexpr_int_t exponent = 1;
  for (size_t index = 0; index < precision; index += 1) exponent *= 10;
  sexpr_int_t fractions = (sexpr_int_t)(value * exponent + 0.5);
  stringify_int_n(fractions, precision, string + integer_width + 1);
  return width;
}
String_View stringify__real(sexpr_real_t value, ssize_t precision) {
  if (precision == 0) return stringify_int((sexpr_int_t)value + 0.5);
  // if ((value - (sexpr_int_t)value) == 0 && precision < 0) return stringify_int((sexpr_int_t)value);
  size_t _precision = real__precision(value, precision);
  String_View res = {.count = real_width(value, _precision)};
  char *string = malloc(res.count + 1);
  res.data = string;
  stringify_real_n(value, int_width(value), _precision, string);
  return res;
}

// Expr *expr_convert_to_bool(Expr *expr) {
//   switch (expr->kind) {
//     case EXPR_KIND_NIL: return &EXPR_FALSE;
//     case EXPR_KIND_SYMBOL: UNREACHABLE("illegal conversion of symbol to bool");
//     case EXPR_KIND_PAIR: UNREACHABLE("illegal conversion of pair to bool");
//     case EXPR_KIND_QUOTE: UNREACHABLE("illegal conversion of quote to bool");
//     case EXPR_KIND_BOOL: return expr;
//     case EXPR_KIND_INTEGER: return make_expr_bool((sexpr_bool_t)expr->integer);
//     case EXPR_KIND_REAL: return make_expr_bool((sexpr_bool_t)expr->real);
//     case EXPR_KIND_STRING: return make_expr_bool(expr->string != NULL);
//   }
// }

// Expr *expr_convert_to_integer(Expr *expr) {
//   switch (expr->kind) {
//     case EXPR_KIND_NIL: return make_expr_integer(0);
//     case EXPR_KIND_SYMBOL: UNREACHABLE("illegal conversion of symbol to integer number");
//     case EXPR_KIND_PAIR: UNREACHABLE("illegal conversion of pair to integer number");
//     case EXPR_KIND_QUOTE: UNREACHABLE("illegal conversion of quote to integer number");
//     case EXPR_KIND_BOOL: return make_expr_integer(expr->boolean ? 1 : 0);
//     case EXPR_KIND_INTEGER: return expr;
//     case EXPR_KIND_REAL: return make_expr_integer((sexpr_int_t)expr->real);
//     case EXPR_KIND_STRING: {
//       String_View sv = sv_from_cstr(expr->string);
//       return make_expr_integer(parse_integer(&sv));
//     }
//   }
// }
// Expr *expr_convert_to_real(Expr *expr) {
//   switch (expr->kind) {
//     case EXPR_KIND_NIL: return make_expr_real(0);
//     case EXPR_KIND_SYMBOL: UNREACHABLE("illegal conversion of symbol to real number");
//     case EXPR_KIND_PAIR: UNREACHABLE("illegal conversion of pair to real number");
//     case EXPR_KIND_QUOTE: UNREACHABLE("illegal conversion of quote to real number");
//     case EXPR_KIND_BOOL: return make_expr_real(expr->boolean ? 1 : 0);
//     case EXPR_KIND_INTEGER: return make_expr_real((sexpr_real_t)expr->integer);
//     case EXPR_KIND_REAL: return expr;
//     case EXPR_KIND_STRING: {
//       String_View sv = sv_from_cstr(expr->string);
//       return make_expr_real(parse_real(&sv));
//     }
//   }
// }
// Expr *expr_convert_to_string(Expr *expr) {
//   switch (expr->kind) {
//     case EXPR_KIND_NIL: return make_expr_string("nil");
//     case EXPR_KIND_SYMBOL: UNREACHABLE("illegal conversion of symbol to string");
//     case EXPR_KIND_PAIR: UNREACHABLE("illegal conversion of pair to string");
//     case EXPR_KIND_QUOTE: UNREACHABLE("illegal conversion of quote to string");
//     case EXPR_KIND_BOOL: return make_expr_string(expr->boolean ? "true" : "false");
//     case EXPR_KIND_INTEGER: return make_expr_string(stringify_int(expr->integer).data);
//     case EXPR_KIND_REAL: return make_expr_string(stringify_real(expr->real, -1).data);
//     case EXPR_KIND_STRING: return expr;
//   }
// }

#endif // SEXPR_UTILS_IMPL
