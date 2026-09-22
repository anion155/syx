#ifndef SYX_LEXER_H
#define SYX_LEXER_H

#include <str_utf.h>
#include <syx/syx_utils.h>
#include <syx/syx_value.h>

typedef enum Syx_Token_Kind {
  SYX_TOKEN_KIND_NULL = 0,
  SYX_TOKEN_KIND_LPAREN,
  SYX_TOKEN_KIND_RPAREN,
  SYX_TOKEN_KIND_LCURLY,
  SYX_TOKEN_KIND_RCURLY,
  SYX_TOKEN_KIND_STRLIT,
  SYX_TOKEN_KIND_NAN,
  SYX_TOKEN_KIND_INFINITY_POSITIVE,
  SYX_TOKEN_KIND_INFINITY_NEGATIVE,
  SYX_TOKEN_KIND_BIN_INT_LIT,
  SYX_TOKEN_KIND_BIN_FRC_LIT,
  SYX_TOKEN_KIND_OCT_INT_LIT,
  SYX_TOKEN_KIND_OCT_FRC_LIT,
  SYX_TOKEN_KIND_DEC_INT_LIT,
  SYX_TOKEN_KIND_DEC_FRC_LIT,
  SYX_TOKEN_KIND_HEX_INT_LIT,
  SYX_TOKEN_KIND_HEX_FRC_LIT,
  SYX_TOKEN_KIND_SYMBOL,
  SYX_TOKEN_KIND_PREFIX,
  SYX_TOKEN_KIND_DISPATCH,
  SYX_TOKEN_KIND_ERROR = 0x100,
  SYX_TOKEN_KIND_EOF,
} Syx_Token_Kind;

const char *syx_token_kind_string(Syx_Token_Kind kind);

typedef struct Syx_Token {
  String_View source;
  Syx_Token_Kind kind;
} Syx_Token;

typedef Da(Syx_Token, Syx_Tokens_Da) Syx_Tokens_Da;

typedef struct Syx_Tokens {
  const Syx_Token *data;
  size_t count;
  size_t capacity;
  String_View source;
} Syx_Tokens;

Syx_Tokens syx_lexer_tokenize(String_View source);

#endif // SYX_LEXER_H

#if defined(SYX_LEXER_IMPL) && !defined(SYX_LEXER_IMPL_C)
#define SYX_LEXER_IMPL_C

#include <wchar.h>

#define SYX_UTILS_IMPL
#include <syx/syx_utils.h>
#define SYX_VALUE_IMPL
#include <syx/syx_value.h>
#define STR_UTF_IMPL
#include <str_utf.h>

const char *syx_token_kind_string(Syx_Token_Kind kind) {
  switch (kind) {
    case SYX_TOKEN_KIND_LPAREN: return "LEFT_PARENTHESIS";
    case SYX_TOKEN_KIND_RPAREN: return "RIGHT_PARENTHESIS";
    case SYX_TOKEN_KIND_STRLIT: return "STRING_LITERAL";
    case SYX_TOKEN_KIND_BIN_INT_LIT: return "BINARY_INTEGER_LITERAL";
    case SYX_TOKEN_KIND_BIN_FRC_LIT: return "BINARY_FRACTIONAL_LITERAL";
    case SYX_TOKEN_KIND_OCT_INT_LIT: return "OCTAL_INTEGER_LITERAL";
    case SYX_TOKEN_KIND_OCT_FRC_LIT: return "OCTAL_FRACTIONAL_LITERAL";
    case SYX_TOKEN_KIND_DEC_INT_LIT: return "DECIMAL_INTEGER_LITERAL";
    case SYX_TOKEN_KIND_DEC_FRC_LIT: return "DECIMAL_FRACTIONAL_LITERAL";
    case SYX_TOKEN_KIND_HEX_INT_LIT: return "HEX_INTEGER_LITERAL";
    case SYX_TOKEN_KIND_HEX_FRC_LIT: return "HEX_FRACTIONAL_LITERAL";
    case SYX_TOKEN_KIND_SYMBOL: return "SYMBOL";
    case SYX_TOKEN_KIND_DISPATCH: return "DISPATCH";
    case SYX_TOKEN_KIND_ERROR: return "ERROR";
    case SYX_TOKEN_KIND_EOF: return "EOF";
    default: return "UNKNOWN";
  }
}

int syx_lexer_is_whitespace(int character) {
  return isspace(character);
}

int syx_lexer_is_binary_digit(int character) {
  switch (character) {
    case '0':
    case '1':
      return true;
    default: return false;
  }
}

int syx_lexer_is_octal_digit(int character) {
  switch (character) {
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
      return true;
    default: return false;
  }
}

int syx_lexer_is_delimeter(int character) {
  if (isspace(character)) return true;
  switch (character) {
    case ';':
    case '(':
    case ')':
    case '{':
    case '}':
    case '"':
      return true;
    default: return false;
  }
}

int syx_lexer_is_symbol_delimeter(int character) {
  if (syx_lexer_is_delimeter(character)) return true;
  switch (character) {
    case ':': return true;
    default: return false;
  }
}

int syx_lexer_is_invalid_delimeter(int character) {
  return character == '"';
}

Syx_Token syx_lexer_get_next_token(String_View *it) {
  Syx_Token token = {.source = *it};
#define it_chop_next() sv_chop_left(it, it->count ? sv_first_utf_length(*it) : 0)
#define set_kind_return(kind_value) ({ \
  token.kind = kind_value;             \
  it_chop_next();                      \
  return token;                        \
})
  switch (*it->data) {
    case '(': set_kind_return(SYX_TOKEN_KIND_LPAREN); break;
    case ')': set_kind_return(SYX_TOKEN_KIND_RPAREN); break;
    case '{': set_kind_return(SYX_TOKEN_KIND_LCURLY); break;
    case '}': set_kind_return(SYX_TOKEN_KIND_RCURLY); break;
    case '\'': set_kind_return(SYX_TOKEN_KIND_PREFIX); break;
    case ',': set_kind_return(SYX_TOKEN_KIND_PREFIX); break;
    case ':': set_kind_return(SYX_TOKEN_KIND_PREFIX); break;
    case '$': set_kind_return(SYX_TOKEN_KIND_PREFIX); break;
    case ';': {
      token.kind = SYX_TOKEN_KIND_NULL;
      while (it->count && *it->data != '\n') it_chop_next();
      token.source.count = it->data - token.source.data;
      it_chop_next();
      return token;
    }
    case '"': {
      token.kind = SYX_TOKEN_KIND_STRLIT;
      it_chop_next();
      while (it->count && *it->data != '"' && *it->data != '\n') {
        if (*it->data == '\\') it_chop_next();
        it_chop_next();
      }
      token.source.count = it->data - token.source.data + 1;
      if (!it->count || *it->data != '"') goto return_error;
      it_chop_next();
      return token;
    }
    case '|': {
      token.kind = SYX_TOKEN_KIND_SYMBOL;
      it_chop_next();
      while (it->count && *it->data != '|' && *it->data != '\n') it_chop_next();
      token.source.count = it->data - token.source.data + 1;
      if (!it->count || *it->data != '|') goto return_error;
      it_chop_next();
      if (it->count) {
        size_t error_width = sv_first_utf_length(*it);
        token.source.count += error_width;
        if (!syx_lexer_is_delimeter(*it->data)) goto return_error;
        token.source.count -= error_width;
      }
      return token;
    }
    case '#': {
      token.kind = SYX_TOKEN_KIND_DISPATCH;
      it_chop_next();
      if (it->count < 1) goto return_error;
      token.source.count += sv_first_utf_length(*it);
      it_chop_next();
      return token;
    }
  }
#undef set_kind_return
  if (isdigit(*it->data) || (it->count > 1 && (*it->data == '-' || *it->data == '+') && isdigit(*(it->data + 1)))) {
    token.kind = SYX_TOKEN_KIND_DEC_INT_LIT;
    if (*it->data == '-' || *it->data == '+') {
      char sign = *it->data;
      it_chop_next();
      if (sv_eq(*it, sv_from_strlit("Infinity"))) {
        if (sign == '-') token.kind = SYX_TOKEN_KIND_INFINITY_NEGATIVE;
        else if (sign == '+') token.kind = SYX_TOKEN_KIND_INFINITY_POSITIVE;
        else UNREACHABLE("should be already checked to be this two exact characters");
        return token;
      }
    }
    int (*is_digit)(int character) = syx_utils_is_decimal_digit;
    if (*it->data == '0' && it->count > 2) {
      switch (*(it->data + 1)) {
        case 'x':
        case 'X': {
          is_digit = syx_utils_is_hex_digit;
          token.kind = SYX_TOKEN_KIND_HEX_INT_LIT;
        } break;
        case 'o':
        case 'O': {
          is_digit = syx_lexer_is_octal_digit;
          token.kind = SYX_TOKEN_KIND_OCT_INT_LIT;
        }; break;
        case 'b':
        case 'B': {
          is_digit = syx_lexer_is_binary_digit;
          token.kind = SYX_TOKEN_KIND_BIN_INT_LIT;
        }; break;
      }
      if (is_digit != syx_utils_is_decimal_digit) {
        it_chop_next();
        it_chop_next();
      }
    }
    size_t digits_read = 0;
    bool underscore = false;
    while (it->count) {
      if (*it->data == '.') {
        if (underscore) break;
        it_chop_next();
        token.kind += 1;
        size_t digits_read_ = digits_read;
        digits_read = 0;
        while (it->count) {
          if (*it->data == '_') {
            it_chop_next();
            if (underscore || !digits_read) break;
            underscore = true;
            continue;
          }
          underscore = false;
          if (!is_digit(*it->data)) break;
          it_chop_next();
          digits_read++;
        }
        digits_read += digits_read_;
        break;
      }
      if (*it->data == '_') {
        it_chop_next();
        if (underscore || !digits_read) break;
        underscore = true;
        continue;
      }
      underscore = false;
      if (!is_digit(*it->data)) break;
      it_chop_next();
      digits_read++;
    }
    if (*it->data == '_') goto return_error;
    token.source.count = it->data - token.source.data;
    if (token.source.count && *(it->data - 1) == '_') goto return_error;
    size_t error_width = it->count ? sv_first_utf_length(*it) : 0;
    token.source.count += error_width;
    if (it->count && !syx_lexer_is_delimeter(*it->data)) goto return_error;
    token.source.count -= error_width;
    if (!digits_read) goto return_error;
    return token;
  }
  if (*it->data == '.' && it->count > 1 && isdigit(*(it->data + 1))) {
    it_chop_next();
    token.kind = SYX_TOKEN_KIND_DEC_FRC_LIT;
    bool underscore = false;
    while (it->count) {
      if (*it->data == '_') {
        it_chop_next();
        if (underscore) break;
        underscore = true;
        continue;
      }
      underscore = false;
      if (!syx_utils_is_decimal_digit(*it->data)) break;
      it_chop_next();
    }
    token.source.count = it->data - token.source.data;
    if (token.source.count && *(it->data - 1) == '_') goto return_error;
    if (it->count && !syx_lexer_is_delimeter(*it->data)) goto return_error;
    return token;
  }
  if (sv_eq(token.source, sv_from_strlit("NaN"))) {
    token.kind = SYX_TOKEN_KIND_NAN;
    return token;
  }
  if (sv_eq(token.source, sv_from_strlit("Infinity"))) {
    token.kind = SYX_TOKEN_KIND_INFINITY_POSITIVE;
    return token;
  }
  {
    wchar_t wc;
    mbstate_t state = {0};
    size_t width = mbrtowc(&wc, it->data, it->count, &state);
    if (width == 0 || width == (size_t)-1 || width == (size_t)-2) goto return_error;
    if (iswprint(wc)) {
      token.kind = SYX_TOKEN_KIND_SYMBOL;
      sv_chop_left(it, width);
      while (it->count) {
        width = 1;
        if (syx_lexer_is_symbol_delimeter(*it->data)) break;
        width = mbrtowc(&wc, it->data, it->count, &state);
        if (width == 0 || width == (size_t)-1 || width == (size_t)-2) {
          width = 0;
          break;
        }
        if (!iswprint(wc)) break;
        sv_chop_left(it, width);
      }
      token.source.count = it->data - token.source.data + width;
      if (it->count && (!syx_lexer_is_symbol_delimeter(*it->data) || syx_lexer_is_invalid_delimeter(*it->data))) goto return_error;
      token.source.count -= width;
      return token;
    }
  }
return_error:
  token.kind = SYX_TOKEN_KIND_ERROR;
  return token;
#undef it_chop_next
}

Syx_Tokens syx_lexer_tokenize(String_View source) {
  Syx_Tokens_Da tokens = {};
  for (String_View it = source; it.count;) {
    while (it.count && syx_lexer_is_whitespace(*it.data)) sv_chop_left(&it, sv_first_utf_length(it));
    Syx_Token token = syx_lexer_get_next_token(&it);
    switch (token.kind) {
      case SYX_TOKEN_KIND_NULL: continue;
      case SYX_TOKEN_KIND_ERROR: da_append(&tokens, token); goto result;
      default: da_append(&tokens, token);
    }
  }
  da_append(&tokens, ((Syx_Token){.source = (String_View){.data = source.data + source.count, .count = 0}, .kind = SYX_TOKEN_KIND_EOF}));
  da_trim_realloc(&tokens);
result:
  return (Syx_Tokens){.source = source, .data = tokens.data, .capacity = tokens.capacity, .count = tokens.count};
}

#endif // SYX_LEXER_IMPL_C
