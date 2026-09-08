#ifndef SB_H
#define SB_H

#include <ctype.h>
#include <da.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <utf.h>

typedef Da(char, String_Builder) String_Builder;
typedef Da_Slice(char, String_View) String_View;
typedef struct String {
  const char *const data;
  const size_t count;
} String;

#define sb_free(sb) da_free((sb))

String_Builder *sb_null_terminate(String_Builder *sb);

#define sb_first(sb) da_first((sb))
#define sb_last(sb) da_last((sb))
#define sb_at(sb, index) da_at((sb), (index))

static inline size_t sb_append(String_Builder *sb, char character) {
  if (sb) da_append(sb, character);
  return 1;
}

static inline size_t sb_append_buf(String_Builder *sb, const char *buffer, size_t count) {
  if (sb) da_append_many_n(sb, buffer, count);
  return count;
}

ssize_t sb_vappendf(String_Builder *sb, const char *fmt, va_list ap);
ssize_t sb_appendf(String_Builder *sb, PRINTF_FMT_PARAM const char *fmt, ...) PRINTF_ATTRIBUTE(2, 3);

static inline size_t sb_append_cstr(String_Builder *sb, const char *cstr) {
  return sb_append_buf(sb, cstr, strlen(cstr));
}

#define sb_append_strlit(sb, str) sb_append_buf((sb), (str), sizeof(str) - 1)

static inline size_t sb__append_sv(String_Builder *sb, String_View sv) {
  return sb_append_buf((sb), sv.data, sv.count);
}
#define sb_append_sv(sb, sv) sb__append_sv((sb), sv_from_like((sv)))

static inline size_t sb_append_repeat(String_Builder *sb, char character, size_t count) {
  if (!sb) return count;
  da_reserve(sb, sb->count + count);
  memset(sb->data + sb->count, character, count);
  return sb->count += count;
}

#define sb_append_null(sb) sb_append((sb), '\0')

// sb_append_int(sb, val) / sb_append_float(sb, val): Lightweight inline append functions for primitives to avoid the overhead of full vsnprintf parsing.

size_t sb__append_pad_align(String_Builder *sb, size_t size, char filler);
#define sb_append_pad_align(sb, size, ...) sb__append_pad_align((sb), (size), WITH_DEFAULT('\0', __VA_ARGS__))

#define sb_substr(sb, ...) da_slice((sb), String_View, __VA_ARGS__)

#define sb_copy(appender, ...) ({           \
  String_Builder sb = {0};                  \
  appender(&sb __VA_OPT__(, ) __VA_ARGS__); \
  sb;                                       \
})

#define string_from_sb(sb) ({                                 \
  String_Builder *_sb_ = (sb);                                \
  sb_null_terminate(_sb_);                                    \
  String string = {.data = _sb_->data, .count = _sb_->count}; \
  *da__reassign(_sb_) = NULL;                                 \
  _sb_->count = 0;                                            \
  _sb_->capacity = 0;                                         \
  string;                                                     \
})

#define string_from(appender, ...) ({                               \
  String_Builder sb = sb_copy(appender __VA_OPT__(, ) __VA_ARGS__); \
  string_from_sb(&sb);                                              \
})
static inline void string_assign(String *dst, String src) {
  assert(dst && !dst->count && !dst->data);
  memcpy(dst, &src, sizeof(String));
}

typedef struct Stringify_State {
  String_Builder *sb;
  size_t count;
  size_t start;
} Stringify_State;

Stringify_State make_stringify_state(String_Builder *sb, size_t capacity) {
  if (sb) da_reserve_exact(sb, sb->count + capacity);
  return (Stringify_State){.sb = sb, .count = 0, .start = sb ? sb->count : 0};
}
#define stringify_append(state, appender, ...) ({                     \
  Stringify_State *_state_ = (state);                                 \
  _state_->count += appender(_state_->sb __VA_OPT__(, ) __VA_ARGS__); \
})
#define stringify_ptr(state, ...) ({                                                      \
  Stringify_State *_state_ = (state);                                                     \
  _state_->sb ? _state_->sb->data + _state_->start + WITH_DEFAULT(0, __VA_ARGS__) : NULL; \
})

#define stringify(appender, ...) ({         \
  String_Builder sb = {0};                  \
  appender(&sb __VA_OPT__(, ) __VA_ARGS__); \
  sb_null_terminate(&sb);                   \
  string_from_sb(&sb);                      \
})
#define fprintf_stringify(f, appender, ...) ({  \
  String_Builder sb = {0};                      \
  appender(&sb __VA_OPT__(, ) __VA_ARGS__);     \
  fprintf((f), "%.*s", (int)sb.count, sb.data); \
  sb_free(&sb);                                 \
})
#define printf_stringify(appender, ...) fprintf_stringify(stdout, appender __VA_OPT__(, ) __VA_ARGS__)

#define sv_from_parts(data_, count_) ((String_View){.data = (data_), .count = (count_)})
#define sv_from_like(value) _Generic((value),                                    \
    String_View: (value),                                                        \
    default: ({                                                                  \
                                       typeof((value)) _val_ = (value);          \
                                       const char *const data = _val_.data;      \
                                       sv_from_parts((char *)data, _val_.count); \
                                     }))
#define sv_from_strlit(str) ((String_View){.count = sizeof(str) - 1, .data = (str)})
#define sv_from_cstr(str) sv_from_parts((str), strlen(str))

static inline bool sv__eq(String_View a, String_View b) {
  if (a.count != b.count) return false;
  return memcmp(a.data, b.data, a.count) == 0;
}
#define sv_eq(a, b) sv__eq(sv_from_like(a), sv_from_like(b))

static inline bool sv__ends_with(String_View sv, String_View suffix) {
  if (sv.count < suffix.count) return false;
  const char *tail = sv.data + sv.count - suffix.count;
  return memcmp(tail, suffix.data, suffix.count) == 0;
}
#define sv_ends_with(sv, suffix) sv__ends_with(sv_from_like(sv), sv_from_like(suffix))

static inline bool sv__starts_with(String_View sv, String_View prefix) {
  if (sv.count < prefix.count) return false;
  return memcmp(sv.data, prefix.data, prefix.count) == 0;
}
#define sv_starts_with(sv, prefix) sv__starts_with(sv_from_like(sv), sv_from_like(prefix))

#define sv_find_macro(sb, item_var, predicate) da_find_macro((sb), item_var, predicate)
#define sv_chop_while_macro(sb, item_var, predicate) da_slice_chop_while_macro((sb), item_var, predicate)

static inline String_View sv__chop_left(String_View *sv, size_t count) {
  if (count > sv->count) count = sv->count;
  String_View result = sv_from_parts(sv->data, count);
  sv->data += count;
  sv->count -= count;
  return result;
}
#define sv_chop_left(sv, ...) sv__chop_left((sv), WITH_DEFAULT(1, __VA_ARGS__))

static inline String_View sv__chop_right(String_View *sv, size_t count) {
  if (count > sv->count) count = sv->count;
  String_View result = sv_from_parts(sv->data + sv->count - count, count);
  sv->count -= count;
  return result;
}
#define sv_chop_right(sv, ...) sv__chop_right((sv), WITH_DEFAULT(1, __VA_ARGS__))

String_View sv_trim_left(String_View *sv);
String_View sv_trim_right(String_View *sv);
size_t sv_trim(String_View *sv);
String_View sv_chop_by_delim(String_View *sv, char delimeter);
String_View sv_chop_by_sv(String_View *sv, String_View delimeter);

String_View sv__chop_while(String_View *sv, bool (*predicate)(char character));
String_View sv__chop_while_i(String_View *sv, bool (*predicate)(char character, size_t index));
#define sv_chop_while(sv, predicate) _Generic((predicate),                                \
    bool (*predicate)(char character): sv__chop_while((sv), (predicate)),                 \
    bool (*predicate)(char character, size_t index): sv__chop_while_i((sv), (predicate)), \
    int (*predicate)(int x): sv__chop_while((sv), (bool (*)(char character))(predicate)))

#define sv_first_utf_length(sv) (utf8_character_lengths[(uint8_t)da_first((sv))])

size_t sv__utf_length(String_View sv, size_t *bytes_overrun);
#define sv_utf_length(sv, ...) sv__utf_length(sv_from_like(sv), WITH_DEFAULT(NULL, __VA_ARGS__))

#define SV_FMT "%.*s"
#define sv_fmt_arg(sv) (int)(sv).count, (sv).data

#endif // SB_H

#if defined(SB_IMPL) && !defined(SB_IMPL_C)
#define SB_IMPL_C

#define UTF_IMPL
#include <utf.h>

String_Builder *sb_null_terminate(String_Builder *sb) {
  if (sb->capacity > sb->count && sb->data[sb->count] == '\0') {
    sb->count += 1;
  } else {
    sb_append_null(sb);
  }
  da_trim_realloc(sb);
  sb->count -= 1;
  return sb;
}

ssize_t sb_vappendf(String_Builder *sb, const char *fmt, va_list ap) {
  va_list args;
  va_copy(args, ap);
  int length = vsnprintf(NULL, 0, fmt, args);
  va_end(args);
  if (sb && length > 0) {
    da_reserve(sb, sb->count + length + 1);
    va_copy(args, ap);
    vsnprintf(sb->data + sb->count, length + 1, fmt, args);
    va_end(args);
    sb->count += length;
  }
  return length;
}

ssize_t sb_appendf(String_Builder *sb, const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  ssize_t length = sb_vappendf(sb, fmt, args);
  va_end(args);
  return length;
}

size_t sb__append_pad_align(String_Builder *sb, size_t size, char filler) {
  size_t rem = sb->count % size;
  if (rem == 0) return 0;
  da_reserve(sb, sb->count + rem);
  memset(sb->data + sb->count, filler, rem);
  sb->count += rem;
  return rem;
}

String_View sv__chop_while(String_View *sv, bool (*predicate)(char character)) {
  return sv_chop_while_macro(sv, character, predicate(*character));
}

String_View sv__chop_while_i(String_View *sv, bool (*predicate)(char character, size_t index)) {
  return sv_chop_while_macro(sv, character, predicate(*character, index));
}

size_t sv__utf_length(String_View sv, size_t *bytes_overrun) {
  size_t count = 0;
  size_t bytes;
  while (sv.count) {
    bytes = sv_first_utf_length(sv);
    if (bytes_overrun && sv.count <= bytes) *bytes_overrun = bytes - sv.count;
    sv_chop_left(&sv, bytes);
    count += 1;
  }
  return count;
}

String_View sv_trim_left(String_View *sv) {
  return sv_chop_while_macro(sv, character, isspace(*character));
}

String_View sv_trim_right(String_View *sv) {
  size_t index = sv_find_macro(*sv, character, isspace(data[count - index - 1]));
  sv->count -= index;
  return (String_View){.data = sv->data + sv->count, .count = index};
}

size_t sv_trim(String_View *sv) {
  String_View start = sv_trim_left(sv);
  String_View end = sv_trim_right(sv);
  return start.count + end.count;
}

String_View sv_chop_by_delim(String_View *sv, char delimeter) {
  size_t index = sv_find_macro(*sv, character, *character != delimeter);
  if (index < sv->count) index += 1;
  String_View result = {.data = sv->data, .count = index};
  sv->data += index;
  sv->count -= index;
  return result;
}

String_View sv_chop_by_sv(String_View *sv, String_View delimeter) {
  String_View it = *sv;
  while (it.count) {
    if (sv__starts_with(it, delimeter)) break;
    sv_chop_left(&it);
  }
  size_t index = it.data - sv->data;
  if (it.count) index += delimeter.count;
  String_View result = {.data = sv->data, .count = index};
  sv->data += index;
  sv->count -= index;
  return result;
}

#endif // SB_IMPL_C
