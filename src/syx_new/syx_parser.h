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

Syx_Value *parse_syx_list_values(Syx_Tokens *tokens, Syx_Token_Kind opening_token, Syx_Token_Kind closing_token) {
  SYX_ASSERT(da_first(tokens).kind == opening_token, "expected start token");
  tokens_chop_left(tokens, 1);
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

Syx_Value *parse_syx_string_value(Syx_Tokens *tokens) {
  Syx_Token token = da_first(tokens);
  tokens_chop_left(tokens, 1);
  SYX_ASSERT(token.kind == SYX_TOKEN_KIND_STRLIT, "expected string literal token");
  SYX_ASSERT(token.data[0] == '"' && token.data[token.count - 1] == '"', "invalid string literal");
  syx_string literal = {0};
  size_t count = token.count - 1;
  da_reserve(&literal, count);
  for (size_t tindex = 1; tindex < count; tindex += 1) {
    if (token.data[tindex] == '\\') {
      tindex += 1;
      if (count <= tindex) {
        sb_append(&literal, '\\');
        continue;
      }
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
          tindex += 1;
          if (count <= tindex) {
            sb_append(&literal, 'x');
            continue;
          }
          tindex += 1;
          if (count <= tindex) {
            sb_append(&literal, 'x');
            sb_append(&literal, token.data[tindex - 1]);
            continue;
          }
          if (!syx_utils_is_hex_digit(token.data[tindex - 1]) || !syx_utils_is_hex_digit(token.data[tindex])) {
            sb_append(&literal, 'x');
            sb_append(&literal, token.data[tindex - 1]);
            sb_append(&literal, token.data[tindex]);
            continue;
          }
          char byte = syx_utils_hex_to_decimal(token.data[tindex - 1]) * 16 + syx_utils_hex_to_decimal(token.data[tindex]);
          sb_append(&literal, byte);
          continue;
        } break;
        case 'u': {
          tindex += 1;
          if (count <= tindex) {
            sb_append(&literal, 'u');
            continue;
          }
          if (token.data[tindex] == '{') {
            TODO("string literal: Unicode Long \\u{H...H}");
          }
          TODO("string literal: Unicode Short \\uHHHH");
        } break;
        case 'U': TODO("string literal: Unicode Long"); break;
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

Syx_Value *parse_syx_number_value(Syx_Tokens *tokens) {
  UNUSED(tokens);
  TODO("parse_syx_number_value");
}

Syx_Value *parse_syx_symbol_value(Syx_Tokens *tokens) {
  UNUSED(tokens);
  TODO("parse_syx_symbol_value");
}

Syx_Value *parse_syx_dispatch(Syx_Tokens *tokens) {
  UNUSED(tokens);
  TODO("parse_syx_dispatch");
}

Syx_Value *parse_syx_value(Syx_Tokens *tokens) {
  if (!tokens->count) return NULL;
  switch (da_first(tokens).kind) {
    case SYX_TOKEN_KIND_EOF: {
      tokens_chop_left(tokens, 1);
      return NULL;
    }
    case SYX_TOKEN_KIND_NULL: {
      tokens_chop_left(tokens, 1);
      return parse_syx_value(tokens);
    }
    case SYX_TOKEN_KIND_LPAREN:
      return parse_syx_list_values(tokens, SYX_TOKEN_KIND_LPAREN, SYX_TOKEN_KIND_RPAREN);
    case SYX_TOKEN_KIND_STRLIT:
      return parse_syx_string_value(tokens);
    case SYX_TOKEN_KIND_NUMLIT:
      return parse_syx_number_value(tokens);
    case SYX_TOKEN_KIND_SYMBOL:
      return parse_syx_symbol_value(tokens);
    case SYX_TOKEN_KIND_DISPATCH:
      return parse_syx_dispatch(tokens);
    default:
      SYX_THROW("unexpected token");
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
