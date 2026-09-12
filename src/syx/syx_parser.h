#ifndef SYX_PARSER_H
#define SYX_PARSER_H

#include <sb.h>
#include <syx/syx_lexer.h>

Syx_Value *parse_syx(String_View source, bool ignore_errors);

#endif // SYX_PARSER_H

#if defined(SYX_PARSER_IMPL) && !defined(SYX_PARSER_IMPL_C)
#define SYX_PARSER_IMPL_C

#include <wchar.h>

#define SB_IMPL
#include <sb.h>
#define SYX_LEXER_IMPL
#include <syx/syx_lexer.h>

Syx_Value *parse_syx_value(Syx_Tokens *tokens);

Syx_Value *parse_syx_list_values(Syx_Tokens *tokens, Syx_Token_Kind closing_token) {
  if (da_first(*tokens).kind == closing_token) {
    da_slice_chop_left(tokens);
    return syx_value_nil();
  }
  Syx_Value *list = NULL;
  Syx_Value **pair_value = &list;
  while (tokens->count) {
    Syx_Token first = da_first(*tokens);
    if (first.kind == SYX_TOKEN_KIND_SYMBOL && sv_eq(first, sv_from_strlit("."))) {
      Syx_Value *value = rc_acquire(parse_syx_value(tokens));
      syx_value_early_exit(value, (list));
      *pair_value = value;
      SYX_ASSERT(da_first(*tokens).kind == closing_token, "expected end token", (), (list));
      goto without_nil;
    }
    Syx_Value *value = rc_acquire(parse_syx_value(tokens));
    syx_value_early_exit(value, (list));
    *pair_value = rc_acquire(make_syx_value_pair(value, NULL));
    pair_value = &(*pair_value)->pair->right;
    if (da_first(*tokens).kind == closing_token) break;
  }
  *pair_value = rc_acquire(syx_value_nil());
without_nil:
  SYX_ASSERT(da_first(*tokens).kind == closing_token, "expected end token", (), (list));
  da_slice_chop_left(tokens);
  return list;
}

uint16_t syx_parser_utf_4_chars_to_codepoint(char chars[4]) {
  return (syx_utils_hex_to_decimal(chars[0]) << 12) |
         (syx_utils_hex_to_decimal(chars[1]) << 8) |
         (syx_utils_hex_to_decimal(chars[2]) << 4) |
         syx_utils_hex_to_decimal(chars[3]);
}

bool syx_parser_utf_codepoint_to_string(uint32_t codepoint, String_Builder *string) {
  if (codepoint <= 0x007F) {
    sb_append(string, (uint8_t)codepoint);
  } else if (codepoint <= 0x07FF) {
    sb_append(string, (uint8_t)(0xC0 | (codepoint >> 6)));
    sb_append(string, (uint8_t)(0x80 | (codepoint & 0x3F)));
  } else if (codepoint <= 0xFFFF) {
    sb_append(string, (uint8_t)(0xE0 | (codepoint >> 12)));
    sb_append(string, (uint8_t)(0x80 | ((codepoint >> 6) & 0x3F)));
    sb_append(string, (uint8_t)(0x80 | (codepoint & 0x3F)));
  } else if (codepoint <= 0x10FFFF) {
    sb_append(string, (uint8_t)(0xF0 | (codepoint >> 18)));
    sb_append(string, (uint8_t)(0x80 | ((codepoint >> 12) & 0x3F)));
    sb_append(string, (uint8_t)(0x80 | ((codepoint >> 6) & 0x3F)));
    sb_append(string, (uint8_t)(0x80 | (codepoint & 0x3F)));
  } else {
    return false;
  }
  return true;
}

uint32_t syx_parser_utf_string_to_codepoint(String_View string) {
  if (string.count < sv_first_utf_length(string)) return 0;
#define get(index, mask, ...) ((__VA_OPT__((uint32_t)(uint8_t)) string.data[index] & mask) __VA_OPT__(<< __VA_ARGS__))
  if (get(0, 0b10000000) == 0) {
    return string.data[0];
  } else if (get(0, 0b11100000) == 0b11000000) {
    if (get(1, 0b11000000) != 0b10000000) return 0;
    return get(0, 0b00011111, 6) | get(1, 0b00111111);
  } else if (get(0, 0b11110000) == 0b11100000) {
    if (get(1, 0b11000000) != 0b10000000) return 0;
    if (get(2, 0b11000000) != 0b10000000) return 0;
    return get(0, 0b00001111, 12) | get(1, 0b00111111, 6) | get(2, 0b00111111);
  } else if (get(0, 0b11111000) == 0b11110000) {
    if (get(1, 0b11000000) != 0b10000000) return 0;
    if (get(2, 0b11000000) != 0b10000000) return 0;
    if (get(3, 0b11000000) != 0b10000000) return 0;
    return get(0, 0b00000111, 18) | get(1, 0b00111111, 12) | get(2, 0b00111111, 6) | get(3, 0b00111111);
  } else {
    return 0;
  }
#undef get
}

bool syx_parser_validate_utf_bytes(char *chars, size_t bytes_count) {
  for (size_t index = 0; index < (bytes_count); index += 1) {
    if (!syx_utils_is_hex_digit(chars[index])) return false;
  }
  return true;
}

Syx_Value *parse_syx_string_value(Syx_Token token) {
  SYX_ASSERT(token.kind == SYX_TOKEN_KIND_STRLIT, "expected string literal token");
  SYX_ASSERT(token.data[0] == '"' && token.data[token.count - 1] == '"', "invalid string literal");
  token.data += 1;
  token.count -= 2;
  String_Builder literal = {0};
  da_reserve(&literal, token.count);
  for (size_t tindex = 0; tindex < token.count; tindex += 1) {
    if (token.data[tindex] == '\\') {
      tindex += 1;
      if (token.count <= tindex) {
        sb_append(&literal, '\\');
        continue;
      }

#define handle_error()                       \
  {                                          \
    sb_append(&literal, token.data[tindex]); \
    continue;                                \
  }
#define utf_bytes_from_string(bytes_count)                      \
  if (token.count - tindex - 1 < (bytes_count)) handle_error(); \
  char chars[(bytes_count)];                                    \
  memcpy(chars, token.data + tindex + 1, (bytes_count));        \
  if (!syx_parser_validate_utf_bytes(chars, (bytes_count))) handle_error();

      switch (token.data[tindex]) {
        case 'b': sb_append(&literal, '\b'); break;
        case 'f': sb_append(&literal, '\f'); break;
        case 'v': sb_append(&literal, '\v'); break;
        case 'a': sb_append(&literal, '\a'); break;
        case 'e': sb_append(&literal, '\e'); break;
        case 'n': sb_append(&literal, '\n'); break;
        case 'r': sb_append(&literal, '\r'); break;
        case 't': sb_append(&literal, '\t'); break;
        case '0': sb_append(&literal, '\0'); break;
        case 'x': {
          utf_bytes_from_string(2);
          uint8_t byte = (syx_utils_hex_to_decimal(chars[0]) << 4) + syx_utils_hex_to_decimal(chars[1]);
          sb_append(&literal, byte);
          tindex += 2;
        } break;
        case 'u': {
          if (token.count > tindex + 3 && token.data[tindex + 1] == '{') {
            size_t count = 0;
            while (true) {
              size_t index = tindex + count + 2;
              if (token.count <= index) {
                count = 0;
                break;
              }
              if (token.data[index] == '}') break;
              if (!syx_utils_is_hex_digit(token.data[index])) {
                count = 0;
                break;
              }
              count += 1;
            }
            if (!count || count > 8) handle_error();
            char chars[8] = {};
            memset(chars, '0', 8 - count);
            memcpy(chars + (8 - count), token.data + tindex + 2, count);
            uint16_t high = syx_parser_utf_4_chars_to_codepoint(chars);
            uint16_t low = syx_parser_utf_4_chars_to_codepoint(chars + 4);
            if (!syx_parser_utf_codepoint_to_string(((uint32_t)high << 16) | low, &literal)) handle_error();
            tindex += count + 2;
            continue;
          }
          utf_bytes_from_string(4);
          if (!syx_parser_utf_codepoint_to_string(syx_parser_utf_4_chars_to_codepoint(chars), &literal)) handle_error();
          tindex += 4;
        } break;
        case 'U': {
          utf_bytes_from_string(8);
          uint16_t high = syx_parser_utf_4_chars_to_codepoint(chars);
          uint16_t low = syx_parser_utf_4_chars_to_codepoint(chars + 4);
          if (!syx_parser_utf_codepoint_to_string(((uint32_t)high << 16) | low, &literal)) handle_error();
          tindex += 8;
        } break;
        default: sb_append(&literal, token.data[tindex]); break;
      }
      continue;
    }
    size_t width = sv_first_utf_length(token);
    for (size_t lindex = 0; lindex < width; lindex += 1) {
      sb_append(&literal, token.data[tindex + lindex]);
    }
  }
#undef handle_error
#undef utf_bytes_from_string
  Syx_Value *value = make_syx_value_string_n_dup(literal.data, literal.count);
  sb_free(&literal);
  return value;
}

Syx_Value *parse_syx_number_binary_value(Syx_Token token) {
  String_View sv = sv_from_like(token);
  bool negative = false;
  if (sv.data[0] == '-') negative = (sv.data += 1, sv.count -= 1, true);
  SYX_ASSERT(sv.data[0] == '0' && (sv.data[1] == 'b' || sv.data[1] == 'B'), "expected binary number");
  sv_chop_left(&sv, 2);
  syx_integer_t number = 0;
  while (sv.count) {
    switch (sv.data[0]) {
      case '0': number = number << 1 | 0; break;
      case '1': number = number << 1 | 1; break;
      case '_': break;
      case '.': SYX_TODO("binary fractionals number literals are not supported");
      default: SYX_THROW("expected binary number");
    }
    sv_chop_left(&sv, 1);
  }
  if (negative) number *= -1;
  return make_syx_value_number_integer(number);
}

Syx_Value *parse_syx_number_octal_value(Syx_Token token) {
  String_View sv = sv_from_like(token);
  bool negative = false;
  if (sv.data[0] == '-') negative = (sv.data += 1, sv.count -= 1, true);
  SYX_ASSERT(sv.data[0] == '0' && (sv.data[1] == 'o' || sv.data[1] == 'O'), "expected octal number");
  sv_chop_left(&sv, 2);
  syx_integer_t number = 0;
  while (sv.count) {
    switch (sv.data[0]) {
      case '0': number = number << 3 | 0; break;
      case '1': number = number << 3 | 1; break;
      case '2': number = number << 3 | 2; break;
      case '3': number = number << 3 | 3; break;
      case '4': number = number << 3 | 4; break;
      case '5': number = number << 3 | 5; break;
      case '6': number = number << 3 | 6; break;
      case '7': number = number << 3 | 7; break;
      case '_': break;
      case '.': SYX_TODO("octal fractionals number literals are not supported");
      default: SYX_THROW("expected octal number");
    }
    sv_chop_left(&sv, 1);
  }
  if (negative) number *= -1;
  return make_syx_value_number_integer(number);
}

Syx_Value *parse_syx_number_decimal_fractional_value(String_View sv, bool negative, syx_integer_t integer) {
  SYX_ASSERT(sv.data[0] == '.', "expected fractional number");
  sv_chop_left(&sv, 1);
  syx_integer_t fractions = 0;
  syx_fractional_t exponent = 1;
  while (sv.count) {
    switch (sv.data[0]) {
      case '0': fractions = fractions * 10 + 0; break;
      case '1': fractions = fractions * 10 + 1; break;
      case '2': fractions = fractions * 10 + 2; break;
      case '3': fractions = fractions * 10 + 3; break;
      case '4': fractions = fractions * 10 + 4; break;
      case '5': fractions = fractions * 10 + 5; break;
      case '6': fractions = fractions * 10 + 6; break;
      case '7': fractions = fractions * 10 + 7; break;
      case '8': fractions = fractions * 10 + 8; break;
      case '9': fractions = fractions * 10 + 9; break;
      case '_': break;
      default: SYX_THROW("expected fractions number part");
    }
    sv_chop_left(&sv, 1);
    exponent *= 10;
  }
  syx_fractional_t number = (syx_fractional_t)integer + (syx_fractional_t)fractions / exponent;
  if (negative) number *= -1;
  return make_syx_value_number_fractional(number);
}

Syx_Value *parse_syx_number_decimal_value(Syx_Token token) {
  String_View sv = sv_from_like(token);
  bool negative = false;
  if (sv.data[0] == '-') negative = (sv.data += 1, sv.count -= 1, true);
  syx_integer_t number = 0;
  while (sv.count) {
    switch (sv.data[0]) {
      case '0': number = number * 10 + 0; break;
      case '1': number = number * 10 + 1; break;
      case '2': number = number * 10 + 2; break;
      case '3': number = number * 10 + 3; break;
      case '4': number = number * 10 + 4; break;
      case '5': number = number * 10 + 5; break;
      case '6': number = number * 10 + 6; break;
      case '7': number = number * 10 + 7; break;
      case '8': number = number * 10 + 8; break;
      case '9': number = number * 10 + 9; break;
      case '_': break;
      case '.': return parse_syx_number_decimal_fractional_value(sv, negative, number);
      default: SYX_THROW("expected number");
    }
    sv_chop_left(&sv, 1);
  }
  if (negative) number *= -1;
  return make_syx_value_number_integer(number);
}

Syx_Value *parse_syx_number_hex_value(Syx_Token token) {
  String_View sv = sv_from_like(token);
  bool negative = false;
  if (sv.data[0] == '-') negative = (sv.data += 1, sv.count -= 1, true);
  SYX_ASSERT(sv.data[0] == '0' && (sv.data[1] == 'x' || sv.data[1] == 'X'), "expected hex number");
  sv_chop_left(&sv, 2);
  syx_integer_t number = 0;
  while (sv.count) {
    switch (sv.data[0]) {
      case '0': number = number << 4 | 0; break;
      case '1': number = number << 4 | 1; break;
      case '2': number = number << 4 | 2; break;
      case '3': number = number << 4 | 3; break;
      case '4': number = number << 4 | 4; break;
      case '5': number = number << 4 | 5; break;
      case '6': number = number << 4 | 6; break;
      case '7': number = number << 4 | 7; break;
      case '8': number = number << 4 | 8; break;
      case '9': number = number << 4 | 9; break;
      case 'a':
      case 'A': number = number << 4 | 10; break;
      case 'b':
      case 'B': number = number << 4 | 11; break;
      case 'c':
      case 'C': number = number << 4 | 12; break;
      case 'd':
      case 'D': number = number << 4 | 13; break;
      case 'e':
      case 'E': number = number << 4 | 14; break;
      case 'f':
      case 'F': number = number << 4 | 15; break;
      case '_': break;
      case '.': SYX_TODO("hex fractionals number literals are not supported");
      default: SYX_THROW("expected hex number");
    }
    sv_chop_left(&sv, 1);
  }
  if (negative) number *= -1;
  return make_syx_value_number_integer(number);
}

Syx_Value *parse_syx_symbol_value(Syx_Token token) {
  if (token.data[0] == '|' && token.data[token.count - 1] == '|') {
    return make_syx_value_symbol_n(token.data + 1, token.count - 2);
  } else {
    return make_syx_value_symbol_n(token.data, token.count);
  }
}

Syx_Value *parse_syx_prefix(Syx_Token token, Syx_Tokens *tokens) {
  SYX_ASSERT(token.kind == SYX_TOKEN_KIND_PREFIX, "prefix expected");
  uint32_t type = syx_parser_utf_string_to_codepoint(sv_from_like(token));
  switch (type) {
    case '\'':
    case ',': {
      return make_syx_value_prefixed((Syx_Prefixed_Kind)type, parse_syx_value(tokens));
    }
    case ':':
    case '$': {
      Syx_Token symbol = da_first(da_slice_chop_left(tokens));
      SYX_ASSERT(symbol.kind == SYX_TOKEN_KIND_SYMBOL, "symbol expected");
      return make_syx_value_prefixed((Syx_Prefixed_Kind)type, parse_syx_symbol_value(symbol));
    }
    default: SYX_THROW("unexpected prefix type");
  }
}

Syx_Value *parse_syx_dispatch(Syx_Token token, Syx_Tokens *tokens) {
  SYX_ASSERT(token.kind == SYX_TOKEN_KIND_DISPATCH && token.count > 1, "dispatch expected");
  uint32_t type = syx_parser_utf_string_to_codepoint(sv_from_parts((char *)token.data + 1, token.count - 1));
  switch (type) {
    case 'n': return syx_value_nil();
    case 't': return syx_value_bool_true();
    case 'f': return syx_value_bool_false();
    case 'R': {
      SYX_ASSERT(tokens->count >= 1, "expected string literal");
      token = da_slice_shift(tokens);
      SYX_ASSERT(token.kind == SYX_TOKEN_KIND_STRLIT, "expected string literal");
      return make_syx_value_string_n_dup(token.data, token.count);
    }
    case '{': {
      Syx_Value *fields = rc_acquire(parse_syx_list_values(tokens, SYX_TOKEN_KIND_RCURLY));
      syx_value_early_exit(fields);
      return make_syx_value_pair(make_syx_value_symbol_strlit("object"), rc_move(fields));
    }
    default: SYX_THROW("unexpected dispatch type");
  }
}

Syx_Value *parse_syx_value(Syx_Tokens *tokens) {
  SYX_ASSERT(tokens->count, "expected value");
  Syx_Token first = da_first(da_slice_chop_left(tokens));
  switch (first.kind) {
    case SYX_TOKEN_KIND_NULL: return parse_syx_value(tokens);
    case SYX_TOKEN_KIND_LPAREN: return parse_syx_list_values(tokens, SYX_TOKEN_KIND_RPAREN);
    case SYX_TOKEN_KIND_STRLIT: return parse_syx_string_value(first);
    case SYX_TOKEN_KIND_NUMBINLIT: return parse_syx_number_binary_value(first);
    case SYX_TOKEN_KIND_NUMOCTLIT: return parse_syx_number_octal_value(first);
    case SYX_TOKEN_KIND_NUMDECLIT: return parse_syx_number_decimal_value(first);
    case SYX_TOKEN_KIND_NUMHEXLIT: return parse_syx_number_hex_value(first);
    case SYX_TOKEN_KIND_SYMBOL: return parse_syx_symbol_value(first);
    case SYX_TOKEN_KIND_PREFIX: return parse_syx_prefix(first, tokens);
    case SYX_TOKEN_KIND_DISPATCH: return parse_syx_dispatch(first, tokens);
    default: SYX_THROW("unexpected token");
  }
}

Syx_Value *parse_syx(String_View source, bool ignore_errors) {
  Syx_Tokens tokens = syx_lexer_tokenize(source);
  SYX_ASSERT(tokens.count, "Failed to parse syx script");
  SYX_ASSERT(tokens.data[tokens.count - 1].kind == SYX_TOKEN_KIND_EOF, "Failed to parse syx script");
  tokens.count -= 1;
  Syx_Value *list = NULL;
  Syx_Value **pair_value = &list;
  for (Syx_Tokens it = tokens; it.count;) {
    Syx_Value *value = rc_acquire(parse_syx_value(&it));
    if (!ignore_errors) syx_value_early_exit(value, (list));
    *pair_value = rc_acquire(make_syx_value_pair(value, NULL));
    pair_value = &(*pair_value)->pair->right;
  }
  *pair_value = rc_acquire(syx_value_nil());
  return rc_move(list);
}

#undef tokens_chop_left

#endif // SYX_PARSER_IMPL_C
