#ifndef SYX_LEXER_H
#define SYX_LEXER_H

#include <nob.h>
#include <syx_new/syx_value.h>

typedef enum Syx_Token_Kind {
  SYX_TOKEN_KIND_NULL = 0,
  SYX_TOKEN_KIND_LPAREN = '(',
  SYX_TOKEN_KIND_RPAREN = ')',
  SYX_TOKEN_KIND_STRLIT = '"',
  SYX_TOKEN_KIND_NUMLIT = '0',
  SYX_TOKEN_KIND_DISPATCH = '#',
  SYX_TOKEN_KIND_SYMBOL = 'a',
  SYX_TOKEN_KIND_ERROR = 0x100,
  SYX_TOKEN_KIND_EOF,
} Syx_Token_Kind;

const char *syx_token_kind_string(Syx_Token_Kind kind);

typedef struct Syx_Token {
  size_t count;
  const char *data;
  Syx_Token_Kind kind;
} Syx_Token;

typedef struct Syx_Tokens {
  syx_string_view source;
  Syx_Token *items;
  size_t count;
  size_t capacity;
} Syx_Tokens;

Syx_Tokens syx_lexer_tokenize(syx_string_view source);

#endif // SYX_LEXER_H

#define SYX_LEXER_IMPL
#if defined(SYX_LEXER_IMPL) && !defined(SYX_LEXER_IMPL_C)
#define SYX_LEXER_IMPL_C

#include <wchar.h>

#define NOB_IMPL
#include <nob.h>
#define SYX_VALUE_IMPL
#include <syx_new/syx_value.h>

const char *syx_token_kind_string(Syx_Token_Kind kind) {
  switch (kind) {
    case SYX_TOKEN_KIND_LPAREN: return "LPAREN";
    case SYX_TOKEN_KIND_RPAREN: return "RPAREN";
    case SYX_TOKEN_KIND_STRLIT: return "STRLIT";
    case SYX_TOKEN_KIND_NUMLIT: return "NUMLIT";
    case SYX_TOKEN_KIND_DISPATCH: return "DISPATCH";
    case SYX_TOKEN_KIND_SYMBOL: return "SYMBOL";
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
    case '_':
    case '0':
    case '1':
      return true;
    default: return false;
  }
}

int syx_lexer_is_octal_digit(int character) {
  switch (character) {
    case '_':
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

int syx_lexer_is_decimal_digit(int character) {
  switch (character) {
    case '_':
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

int syx_lexer_is_hexadecimal_digit(int character) {
  switch (character) {
    case '_':
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

int syx_lexer_is_delimeter(int character) {
  if (isspace(character)) return true;
  switch (character) {
    case ';':
    case '(':
    case ')':
    case '"':
      return true;
    default: return false;
  }
}

int syx_lexer_is_invalid_delimeter(int character) {
  return character == '"';
}

Syx_Token syx_lexer_get_next_token(syx_string_view *it) {
  Syx_Token token = {.data = it->data, .count = nob_bytes_for_utf8(*it)};
#define it_chop_next() sv_chop_left(it, it->count ? nob_bytes_for_utf8(*it) : 0)
  switch (*it->data) {
    case '(':
    case ')': {
      token.kind = (Syx_Token_Kind)*it->data;
      it_chop_next();
      return token;
    }
    case ';': {
      token.kind = SYX_TOKEN_KIND_NULL;
      while (it->count && *it->data != '\n') it_chop_next();
      token.count = it->data - token.data;
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
      token.count = it->data - token.data + 1;
      if (!it->count || *it->data != '"') goto return_error;
      it_chop_next();
      return token;
    }
    case '|': {
      token.kind = SYX_TOKEN_KIND_SYMBOL;
      it_chop_next();
      while (it->count && *it->data != '|' && *it->data != '\n') it_chop_next();
      token.count = it->data - token.data + 1;
      if (!it->count || *it->data != '|') goto return_error;
      it_chop_next();
      if (it->count) {
        size_t error_width = nob_bytes_for_utf8(*it);
        token.count += error_width;
        if (!syx_lexer_is_delimeter(*it->data)) goto return_error;
        token.count -= error_width;
      }
      return token;
    }
    case '#': {
      token.kind = SYX_TOKEN_KIND_DISPATCH;
      it_chop_next();
      if (it->count < 1) goto return_error;
      token.count += nob_bytes_for_utf8(*it);
      it_chop_next();
      return token;
    }
  }
  if (isdigit(*it->data) || (it->count > 1 && *it->data == '-' && isdigit(*(it->data + 1)))) {
    token.kind = SYX_TOKEN_KIND_NUMLIT;
    if (*it->data == '-') it_chop_next();
    int (*is_digit)(int character) = syx_lexer_is_decimal_digit;
    if (*it->data == '0' && it->count > 2) {
      switch (*(it->data + 1)) {
        case 'x':
        case 'X': is_digit = syx_lexer_is_hexadecimal_digit; break;
        case 'o':
        case 'O': is_digit = syx_lexer_is_octal_digit; break;
        case 'b':
        case 'B': is_digit = syx_lexer_is_binary_digit; break;
      }
      if (is_digit != syx_lexer_is_decimal_digit) {
        it_chop_next();
        it_chop_next();
      }
    }
    size_t digits_read = 0;
    while (it->count) {
      if (*it->data == '.') {
        it_chop_next();
        while (it->count && is_digit(*it->data)) {
          it_chop_next();
          digits_read++;
        }
        break;
      }
      if (!is_digit(*it->data)) break;
      it_chop_next();
      digits_read++;
    }
    size_t error_width = it->count ? nob_bytes_for_utf8(*it) : 0;
    token.count = it->data - token.data + error_width;
    if (it->count && !syx_lexer_is_delimeter(*it->data)) goto return_error;
    token.count -= error_width;
    if (!digits_read) goto return_error;
    return token;
  }
  if (*it->data == '.' && it->count > 1 && isdigit(*(it->data + 1))) {
    it_chop_next();
    token.kind = SYX_TOKEN_KIND_NUMLIT;
    while (it->count && syx_lexer_is_decimal_digit(*it->data)) it_chop_next();
    token.count = it->data - token.data;
    if (it->count && !syx_lexer_is_delimeter(*it->data)) goto return_error;
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
        if (syx_lexer_is_delimeter(*it->data)) break;
        width = mbrtowc(&wc, it->data, it->count, &state);
        if (width == 0 || width == (size_t)-1 || width == (size_t)-2) {
          width = 0;
          break;
        }
        if (!iswprint(wc)) break;
        sv_chop_left(it, width);
      }
      token.count = it->data - token.data + width;
      if (it->count && (!syx_lexer_is_delimeter(*it->data) || syx_lexer_is_invalid_delimeter(*it->data))) goto return_error;
      token.count -= width;
      return token;
    }
  }
return_error:
  token.kind = SYX_TOKEN_KIND_ERROR;
  return token;
#undef it_chop_next
}

Syx_Tokens syx_lexer_tokenize(syx_string_view source) {
  Syx_Tokens tokens = {.source = source};
  for (syx_string_view it = tokens.source; it.count;) {
    while (it.count && syx_lexer_is_whitespace(*it.data)) sv_chop_left(&it, nob_bytes_for_utf8(it));
    Syx_Token token = syx_lexer_get_next_token(&it);
    switch (token.kind) {
      case SYX_TOKEN_KIND_NULL: continue;
      case SYX_TOKEN_KIND_ERROR: {
        da_append(&tokens, token);
        return tokens;
      }
      default: {
        da_append(&tokens, token);
      }
    }
  }
  da_append(&tokens, ((Syx_Token){.data = source.data + source.count, .count = 0, .kind = SYX_TOKEN_KIND_EOF}));
  return tokens;
}

#endif // SYX_PARSER_IMPL_C
