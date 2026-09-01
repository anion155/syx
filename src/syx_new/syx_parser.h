#ifndef SYX_PARSER_H
#define SYX_PARSER_H

#include <nob.h>
#include <syx_new/syx_lexer.h>

#endif // SYX_PARSER_H

#define SYX_PARSER_IMPL
#if defined(SYX_PARSER_IMPL) && !defined(SYX_PARSER_IMPL_C)
#define SYX_PARSER_IMPL_C

#include <wchar.h>

#define NOB_IMPL
#include <nob.h>
#define SYX_LEXER_IMPL
#include <syx_new/syx_lexer.h>

Syx_Value *parse_syx_value(Syx_Tokens *tokens) {
  UNUSED(tokens);
  TODO("parse_syx_value");
  // char current = syx_parser_ctx_current(ctx);
  // syx_parser_chop_while(ctx, syx_parser_is_whitespace);
  // syx_parser_is_whitespace
  // if ()
}

Syx_Value *parse_syx(syx_string_view source) {
  Syx_Tokens tokens = syx_lexer_tokenize(source);
  if (!tokens.count) SYX_THROW("Failed to parse syx script");
  if (tokens.items[tokens.count - 1].kind != SYX_TOKEN_KIND_EOF) SYX_THROW("Failed to parse syx script");
  Syx_Value *list = rc_acquire(make_syx_value_pair(NULL, NULL));
  Syx_Pair *pair = list->pair;
  for (Syx_Tokens it = tokens; it.count;) {
    Syx_Value *value = rc_acquire(parse_syx_value(&it));
    syx_value_early_exit(value, list);
    pair->left = value;
    pair->right = rc_acquire(make_syx_value_pair(NULL, syx_value_nil()));
    pair = pair->right->pair;
  }
  return list;
}

#endif // SYX_PARSER_IMPL_C
