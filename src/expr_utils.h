#ifndef EXPR_UTILS_H
#define EXPR_UTILS_H
#include <nob.h>
#include <ht.h>
#include "expr_ast.h"

expr_int_t parse_integer(String_View *sv);
expr_real_t parse_fractions(String_View *sv);
expr_real_t parse_real(String_View *sv);

size_t int_length(expr_int_t value);
size_t stringify_int_n(expr_int_t value, char *string);
String_View stringify_int(expr_int_t value);

size_t real_exponent(expr_real_t value);
size_t real_length(expr_real_t value, int precision);
size_t stringify_real_n(expr_real_t value, int precision, char *string);
String_View stringify_real(expr_real_t value, int precision);

Expr *expr_convert_to_bool(Expr *expr);
Expr *expr_convert_to_integer(Expr *expr);
Expr *expr_convert_to_real(Expr *expr);
Expr *expr_convert_to_string(Expr *expr);

#endif // EXPR_UTILS_H

#ifdef EXPR_UTILS_IMPLEMENTATION
#undef EXPR_UTILS_IMPLEMENTATION

expr_int_t parse_integer(String_View *sv) {
  bool is_negative = sv->data[0] == '-';
  if (is_negative) nob_sv_chop_left(sv, 1);
  if (!sv->count) UNREACHABLE("expected number literal here");
  expr_int_t value = 0;
  size_t i = 0;
  while (isdigit(sv->data[i])) {
    value = value * 10 + (sv->data[i] - '0');
    i += 1;
    if (i >= sv->count) break;
  }
  sv->count -= i;
  sv->data  += i;
  if (is_negative) value *= -1;
  return value;
}
expr_real_t parse_fractions(String_View *sv) {
  if (sv->data[0] != '.') UNREACHABLE("expected real number's fractional part start here");
  nob_sv_chop_left(sv, 1);
  expr_int_t fractional_part = 0;
  expr_int_t exponent = 1;
  size_t i = 0;
  while (i < sv->count && isdigit(sv->data[i])) {
    fractional_part = fractional_part * 10 + (sv->data[i] - '0');
    exponent *= 10;
    i += 1;
  }
  if (i == 0) UNREACHABLE("expected real number's fractional part start here");
  sv->count -= i;
  sv->data  += i;
  return (expr_real_t)fractional_part / (expr_real_t)exponent;
}
expr_real_t parse_real(String_View *sv) {
  expr_int_t integer_part = parse_integer(sv);
  if (sv->data[0] != '.') return (expr_real_t)integer_part;
  expr_real_t value = parse_fractions(sv);
  if (integer_part < 0) value = integer_part - value;
  else value = integer_part + value;
  return value;
}

size_t int_length(expr_int_t value) {
  size_t size = 0;
  for (expr_int_t it = value; it != 0; it = it / 10) size += 1;
  if (value == 0) size = 1;
  else if (value < 0) size += 1;
  return size;
}

size_t stringify_int_n(expr_int_t value, char *string) {
  size_t length = int_length(value);
  if (value == 0) {
    string[0] = '0';
    return 1;
  }
  if (value < 0) string[0] = '-';
  for (struct { size_t index; expr_int_t it; } s = {length - 1, value}; s.index >= 0; s.index -= 1, s.it /= 10) {
    string[s.index] = '0' + (value < 0 ? -(s.it % 10) : s.it % 10);
  }
  return length;
}

String_View stringify_int(expr_int_t value) {
  size_t length = int_length(value);
  char *string = malloc(length + 1);
  String_View result = {.data = string, .count = length};
  stringify_int_n(value, string);
  return result;
}

const expr_real_t MINIMAL_REAL_DIFFERENCE = 1e-9;
size_t real_exponent(expr_real_t value) {
  value -= (expr_int_t)value;
  size_t length = 0;
  while (value > MINIMAL_REAL_DIFFERENCE && length < 15) {
    value *= 10;
    value -= (expr_int_t)value;
    length++;
    if (value < MINIMAL_REAL_DIFFERENCE || value > (1.0 - MINIMAL_REAL_DIFFERENCE)) break;
  }
  return length;
}

size_t real_length(expr_real_t value, int precision) {
  size_t integer_length = int_length(value);
  if (precision == 0) return integer_length;
  if ((value - (expr_int_t)value) == 0 && precision == -1) return integer_length;
  if (precision < 0) precision = real_exponent(value);
  return integer_length + 1 + precision;
}

size_t stringify_real_n(expr_real_t value, int precision, char *string) {
  if (precision == 0) return stringify_int_n((expr_int_t)value + 0.5, string);
  if ((value - (expr_int_t)value) == 0 && precision == -1) return stringify_int_n((expr_int_t)value, string);
  size_t integer_length = int_length(value);
  if (precision < 0) precision = real_exponent(value);
  size_t length = integer_length + 1 + precision;
  stringify_int_n(value, string);
  string[integer_length] = '.';
  string[length] = 0;
  expr_int_t exponent = 1;
  for (int index = 0; index < precision; index += 1) exponent *= 10;
  expr_int_t fractions = (expr_int_t)(value * exponent + 0.5);
  stringify_int_n(fractions, string + integer_length + 1);
  return length;
}

String_View stringify_real(expr_real_t value, int precision) {
  if (precision == 0) return stringify_int((expr_int_t)value + 0.5);
  if ((value - (expr_int_t)value) == 0 && precision == -1) return stringify_int((expr_int_t)value);
  if (precision < 0) precision = real_exponent(value);
  String_View res = {.count = real_length(value, precision)};
  char *string = malloc(res.count + 1);
  res.data = string;
  stringify_real_n(value, precision, string);
  return res;
}

Expr *expr_convert_to_bool(Expr *expr) {
  switch (expr->kind) {
    case EXPR_KIND_NIL: return &EXPR_FALSE;
    case EXPR_KIND_SYMBOL: UNREACHABLE("illegal conversion of symbol to bool");
    case EXPR_KIND_PAIR: UNREACHABLE("illegal conversion of pair to bool");
    case EXPR_KIND_QUOTE: UNREACHABLE("illegal conversion of quote to bool");
    case EXPR_KIND_BOOL: return expr;
    case EXPR_KIND_INTEGER: return make_expr_bool((expr_bool_t)expr->integer);
    case EXPR_KIND_REAL: return make_expr_bool((expr_bool_t)expr->real);
    case EXPR_KIND_STRING: return make_expr_bool(expr->string != NULL);
  }
}

Expr *expr_convert_to_integer(Expr *expr) {
  switch (expr->kind) {
    case EXPR_KIND_NIL: return make_expr_integer(0);
    case EXPR_KIND_SYMBOL: UNREACHABLE("illegal conversion of symbol to integer number");
    case EXPR_KIND_PAIR: UNREACHABLE("illegal conversion of pair to integer number");
    case EXPR_KIND_QUOTE: UNREACHABLE("illegal conversion of quote to integer number");
    case EXPR_KIND_BOOL: return make_expr_integer(expr->boolean ? 1 : 0);
    case EXPR_KIND_INTEGER: return expr;
    case EXPR_KIND_REAL: return make_expr_integer((expr_int_t)expr->real);
    case EXPR_KIND_STRING: {
      String_View sv = sv_from_cstr(expr->string);
      return make_expr_integer(parse_integer(&sv));
    }
  }
}
Expr *expr_convert_to_real(Expr *expr) {
  switch (expr->kind) {
    case EXPR_KIND_NIL: return make_expr_real(0);
    case EXPR_KIND_SYMBOL: UNREACHABLE("illegal conversion of symbol to real number");
    case EXPR_KIND_PAIR: UNREACHABLE("illegal conversion of pair to real number");
    case EXPR_KIND_QUOTE: UNREACHABLE("illegal conversion of quote to real number");
    case EXPR_KIND_BOOL: return make_expr_real(expr->boolean ? 1 : 0);
    case EXPR_KIND_INTEGER: return make_expr_real((expr_real_t)expr->integer);
    case EXPR_KIND_REAL: return expr;
    case EXPR_KIND_STRING: {
      String_View sv = sv_from_cstr(expr->string);
      return make_expr_real(parse_real(&sv));
    }
  }
}
Expr *expr_convert_to_string(Expr *expr) {
  switch (expr->kind) {
    case EXPR_KIND_NIL: return make_expr_string("nil");
    case EXPR_KIND_SYMBOL: UNREACHABLE("illegal conversion of symbol to string");
    case EXPR_KIND_PAIR: UNREACHABLE("illegal conversion of pair to string");
    case EXPR_KIND_QUOTE: UNREACHABLE("illegal conversion of quote to string");
    case EXPR_KIND_BOOL: return make_expr_string(expr->boolean ? "true" : "false");
    case EXPR_KIND_INTEGER: return make_expr_string(stringify_int(expr->integer).data);
    case EXPR_KIND_REAL: return make_expr_string(stringify_real(expr->real, -1).data);
    case EXPR_KIND_STRING: return expr;
  }
}

#endif // EXPR_UTILS_IMPLEMENTATION
