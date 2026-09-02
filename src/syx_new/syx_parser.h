#ifndef SYX_PARSER_H
#define SYX_PARSER_H

#include <nob.h>
#include <syx_new/syx_lexer.h>

Syx_Value *parse_syx(syx_string_view source);

#endif // SYX_PARSER_H

#define SYX_PARSER_IMPL
#if defined(SYX_PARSER_IMPL) && !defined(SYX_PARSER_IMPL_C)
#define SYX_PARSER_IMPL_C

#include <wchar.h>

#define NOB_IMPL
#include <nob.h>
#define SYX_LEXER_IMPL
#include <syx_new/syx_lexer.h>

#define tokens_chop_left(tokens, n)               \
  do {                                            \
    (tokens)->items += MIN((n), (tokens)->count); \
    (tokens)->count -= MIN((n), (tokens)->count); \
  } while (0)
#define sv_like_eq(a, b) ({                                       \
  typeof((a)) ac = (a);                                           \
  typeof((b)) bc = (b);                                           \
  ac.count == bc.count &&memcmp(ac.data, bc.data, ac.count) == 0; \
})

Syx_Value *parse_syx_value(Syx_Tokens *tokens);

Syx_Value *parse_syx_list_values(Syx_Tokens *tokens, Syx_Token_Kind closing_token) {
  if (da_first(tokens).kind == closing_token) {
    tokens_chop_left(tokens, 1);
    return syx_value_nil();
  }
  Syx_Value *list = rc_acquire(make_syx_value_pair(NULL, NULL));
  Syx_Pair *pair = list->pair;
  while (tokens->count) {
    Syx_Token first = da_first(tokens);
    if (first.kind == SYX_TOKEN_KIND_SYMBOL && sv_like_eq(first, SVLIT("."))) {
      Syx_Value *value = rc_acquire(parse_syx_value(tokens));
      syx_value_early_exit(value, list);
      pair->right = value;
      SYX_ASSERT(da_first(tokens).kind == closing_token, "expected end token", NULL, list);
      break;
    }
    Syx_Value *value = rc_acquire(parse_syx_value(tokens));
    syx_value_early_exit(value, list);
    pair->left = value;
    pair->right = rc_acquire(make_syx_value_pair(NULL, syx_value_nil()));
    pair = pair->right->pair;
    if (da_first(tokens).kind == closing_token) break;
  }
  SYX_ASSERT(da_first(tokens).kind == closing_token, "expected end token", NULL, list);
  tokens_chop_left(tokens, 1);
  return list;
}

uint16_t syx_parser_utf_chars_to_codepoint(char chars[4]) {
  return (syx_utils_hex_to_decimal(chars[0]) << 12) | (syx_utils_hex_to_decimal(chars[1]) << 8) | (syx_utils_hex_to_decimal(chars[2]) << 4) | syx_utils_hex_to_decimal(chars[3]);
}

void syx_parser_utf_codepoint_to_string(uint32_t codepoint, syx_string *string) {
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
    UNREACHABLE("invalid utf codepoint");
  }
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
  syx_string literal = {0};
  da_reserve(&literal, token.count);
  for (size_t tindex = 0; tindex < token.count; tindex += 1) {
    if (token.data[tindex] == '\\') {
      tindex += 1;
      if (token.count <= tindex) {
        sb_append(&literal, '\\');
        continue;
      }

#define syx_parser_utf_bytes_from_string(bytes_count)                 \
  if (token.count - tindex - 1 < (bytes_count)) continue;             \
  char chars[(bytes_count)];                                          \
  memcpy(chars, token.data + tindex + 1, (bytes_count));              \
  if (!syx_parser_validate_utf_bytes(chars, (bytes_count))) continue; \
  tindex += (bytes_count);

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
          syx_parser_utf_bytes_from_string(2);
          uint8_t byte = (syx_utils_hex_to_decimal(chars[0]) << 4) + syx_utils_hex_to_decimal(chars[1]);
          sb_append(&literal, byte);
          continue;
        } break;
        case 'u': {
          if (token.count > tindex + 2 && token.data[tindex + 2] == '{') {
            TODO("string literal: Unicode Long \\u{H...H}");
          }
          syx_parser_utf_bytes_from_string(4);
          syx_parser_utf_codepoint_to_string(syx_parser_utf_chars_to_codepoint(chars), &literal);
        } break;
        case 'U': {
          syx_parser_utf_bytes_from_string(8);
          uint16_t high = syx_parser_utf_chars_to_codepoint(chars);
          uint16_t low = syx_parser_utf_chars_to_codepoint(chars + 4);
          syx_parser_utf_codepoint_to_string(((uint32_t)high << 16) | low, &literal);
        } break;
        default: sb_append(&literal, token.data[tindex]); break;
      }
      continue;
    }
    size_t width = nob__bytes_for_utf8[(uint8_t)token.data[tindex]];
    for (size_t lindex = 0; lindex < width; lindex += 1) {
      sb_append(&literal, token.data[tindex + lindex]);
    }
  }
  Syx_Value *value = make_syx_value_string_dup(literal.items, literal.count);
  sb_free(literal);
  return value;
}

Syx_Value *parse_syx_number_binary_value(Syx_Token token) {
  UNUSED(token);
  TODO("parse_syx_number_binary_value");
}

Syx_Value *parse_syx_number_decimal_value(Syx_Token token) {
  UNUSED(token);
  TODO("parse_syx_number_decimal_value");
}

Syx_Value *parse_syx_number_octal_value(Syx_Token token) {
  UNUSED(token);
  TODO("parse_syx_number_octal_value");
}

Syx_Value *parse_syx_number_hex_value(Syx_Token token) {
  UNUSED(token);
  TODO("parse_syx_number_hex_value");
}

Syx_Value *parse_syx_symbol_value(Syx_Token token) {
  if (token.data[0] == '|' && token.data[token.count - 1] == '|') {
    return make_syx_value_symbol_n(token.data + 1, token.count - 2);
  } else {
    return make_syx_value_symbol_n(token.data, token.count);
  }
}

Syx_Value *parse_syx_dispatch(Syx_Token token, Syx_Tokens *tokens) {
  UNUSED(token);
  UNUSED(tokens);
  TODO("parse_syx_dispatch");
}

Syx_Value *parse_syx_value(Syx_Tokens *tokens) {
  if (!tokens->count) return NULL;
  Syx_Token first = da_first(tokens);
  tokens_chop_left(tokens, 1);
  switch (first.kind) {
    case SYX_TOKEN_KIND_EOF: return NULL;
    case SYX_TOKEN_KIND_NULL: return parse_syx_value(tokens);
    case SYX_TOKEN_KIND_LPAREN: return parse_syx_list_values(tokens, SYX_TOKEN_KIND_RPAREN);
    case SYX_TOKEN_KIND_STRLIT: return parse_syx_string_value(first);
    case SYX_TOKEN_KIND_NUMBINLIT: return parse_syx_number_binary_value(first);
    case SYX_TOKEN_KIND_NUMDECLIT: return parse_syx_number_decimal_value(first);
    case SYX_TOKEN_KIND_NUMOCTLIT: return parse_syx_number_octal_value(first);
    case SYX_TOKEN_KIND_NUMHEXLIT: return parse_syx_number_hex_value(first);
    case SYX_TOKEN_KIND_SYMBOL: return parse_syx_symbol_value(first);
    case SYX_TOKEN_KIND_DISPATCH: return parse_syx_dispatch(first, tokens);
    default: SYX_THROW("unexpected token");
  }
}

Syx_Value *parse_syx(syx_string_view source) {
  Syx_Tokens tokens = syx_lexer_tokenize(source);
  if (!tokens.count) SYX_THROW("Failed to parse syx script");
  if (tokens.items[tokens.count - 1].kind != SYX_TOKEN_KIND_EOF) SYX_THROW("Failed to parse syx script");
  Syx_Value *list = rc_acquire(make_syx_value_pair(NULL, NULL));
  Syx_Pair *pair = list->pair;
  for (Syx_Tokens it = tokens; it.count;) {
    Syx_Value *value = rc_acquire(parse_syx_value(&it));
    if (!value) continue;
    syx_value_early_exit(value, list);
    pair->left = value;
    pair->right = rc_acquire(make_syx_value_pair(NULL, syx_value_nil()));
    pair = pair->right->pair;
  }
  return list;
}

#endif // SYX_PARSER_IMPL_C
