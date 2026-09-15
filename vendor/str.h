/**
 * str.h - 0.5.0 - Public Domain - https://github.com/anion155/c-tools
 *
 * String utilities for c23.
 *
 * ## Usage example
 * ```c
  #define STR_IMPL
  #include "str.h"

  int main(void) {
    String_Builder sb = {0};
    sb_append_strlit(&values, "test");
    sb_append(&values, ':');
    sb_appendf(&values, " %d", 10);
    printf("sb: capacity: %zu, count: %zu, data: '"SV_FMT"'", values.capacity, values.count, sv_fmt_arg(values));

    String str = string_from_sb(&sb);
    printf("sb: capacity: %zu, count: %zu, data: '"SV_FMT"'", values.capacity, values.count, sv_fmt_arg(values));
    printf("str: capacity: %zu, count: %zu, data: '"SV_FMT"'", str.capacity, str.count, sv_fmt_arg(str));

    string_free(&str);
    printf("str: capacity: %zu, count: %zu, data: '"SV_FMT"'", str.capacity, str.count, sv_fmt_arg(str));
  }
 * ```
 *
 * ## Requirements
 *
 * - C23
 * - GNU statement expressions
 * - [da.h](./da.h)
 * - [defines.h](./defines.h)
 */

#ifndef STR_H
#define STR_H

#include <ctype.h>
#include <da.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>

/** Dynamic array of characters. */
typedef Da(char, String_Builder) String_Builder;

/** Slice view to array of characters. */
typedef Da_Slice(char, String_View) String_View;

/** Immutable string container. */
typedef Da_Const(char, String) String;

/** Frees the buffer of `sb` and resets fields to 0. */
#define sb_free(sb) da_free((sb))

/** Reallocate underlying string as cstr with null terminal and `capacity == count + 1`. */
String_Builder *sb_null_terminate(String_Builder *sb);

/** Returns the first character of `sb`. */
#define sb_first(sb) da_first((sb))
/** Returns the last character of `sb`. */
#define sb_last(sb) da_last((sb))
/** Returns the character at `index`. */
#define sb_at(sb, index) da_at((sb), (index))

/**
 * Appends a single character.
 * Returns number of characters added.
 */
static inline size_t sb_append(String_Builder *sb, char character) {
  if (sb) da_append(sb, character);
  return 1;
}
/**
 * Appends `count` characters from `buffer`.
 * Returns number of characters added.
 */
static inline size_t sb_append_buf(String_Builder *sb, const char *buffer, size_t count) {
  if (sb) da_append_many_n(sb, buffer, count);
  return count;
}
/**
 * Appends formatted string, va_list argument version.
 * Returns number of characters added.
 */
ssize_t sb_vappendf(String_Builder *sb, const char *fmt, va_list ap);
/**
 * Appends formatted string.
 * Returns number of characters added.
 */
ssize_t sb_appendf(String_Builder *sb, PRINTF_FMT_PARAM const char *fmt, ...) PRINTF_ATTRIBUTE(2, 3);
/**
 * Appends a null-terminated C string.
 * Returns number of characters added.
 */
static inline size_t sb_append_cstr(String_Builder *sb, const char *cstr) {
  return sb_append_buf(sb, cstr, strlen(cstr));
}
/**
 * Appends a string literal.
 * Returns number of characters added.
 */
#define sb_append_strlit(sb, str) sb_append_buf((sb), (str), sizeof(str) - 1)
static inline size_t sb__append_sv(String_Builder *sb, String_View sv) {
  return sb_append_buf((sb), sv.data, sv.count);
}
/**
 * Appends a `String_View` compatible value.
 * Returns number of characters added.
 */
#define sb_append_sv(sb, sv) sb__append_sv((sb), sv_from_like((sv)))
/**
 * Appends `character` repeated `count` times.
 * Returns number of characters added.
 */
static inline size_t sb_append_repeat(String_Builder *sb, char character, size_t count) {
  if (!sb) return count;
  da_reserve(sb, sb->count + count);
  memset(sb->data + sb->count, character, count);
  sb->count += count;
  return count;
}
/**
 * Appends `'\0'` directly to `sb`.
 * Returns number of characters added.
 */
#define sb_append_null(sb) sb_append((sb), '\0')
size_t sb__append_pad_align(String_Builder *sb, size_t size, char filler);
/**
 * Pads `sb` up to a multiple boundary of `size` using `filler` (defaults to `'\0'`).
 * Returns number of characters added.
 */
#define sb_append_pad_align(sb, size, ...) sb__append_pad_align((sb), (size), WITH_DEFAULT('\0', __VA_ARGS__))

/** Extracts a `String_View` slice from `sb`. */
#define sb_substr(sb, ...) da_slice((sb), String_View, __VA_ARGS__)

/**
 * Executes `some_sb_append` callback on a temporary `String_Builder` and returns it.
 *
 * Example of usage:
 * ```c
  String_Builder sb = sb_copy(sb_append, 't');
 * ```
 */
#define sb_copy(some_sb_append, ...) ({           \
  String_Builder sb = {0};                        \
  some_sb_append(&sb __VA_OPT__(, ) __VA_ARGS__); \
  sb;                                             \
})

/** Helper structure to create custom `sb_append_` compatible functions. */
typedef struct Stringify_State {
  String_Builder *sb;
  size_t count;
  size_t start;
} Stringify_State;

/**
 * Example of `sb_append_` like function:
 * ```c
  size_t sb_append_rand(String_Builder *sb, size_t size) {
    Stringify_State state = make_stringify_state(sb, size - 1); // won't be enough eventually, but it's ok, cause we are using String_Builder
    stringify_append(&state, sb_append, ':');
    for (size_t index = 0; index < size; index += 1) {
      stringify_append(&state, sb_append, 'a' + rand() % 10);
    }
    if (state.sb) {
      for (size_t index = 0; index < size; index += 1) {
        state.sb.data[index] += 1;
      }
      da_reserve(&state.sb, state.sb.count + 2);
      *stringify_ptr(&state) = '(';
      state.count += 1;
      *stringify_ptr(&state) = ')';
      state.count += 1;
    } else {
      state.count += 2;
    }
    return state.count;
  }
 * ```
 */

/** Initializes a `Stringify_State` structure, ensuring `sb` has target capacity. */
Stringify_State make_stringify_state(String_Builder *sb, size_t capacity) {
  if (sb) da_reserve_exact(sb, sb->count + capacity);
  return (Stringify_State){.sb = sb, .count = 0, .start = sb ? sb->count : 0};
}
/** Runs `some_sb_append` callback into the state's `sb` and updates total appended count. */
#define stringify_append(state, some_sb_append, ...) ({                     \
  Stringify_State *_state_ = (state);                                       \
  _state_->count += some_sb_append(_state_->sb __VA_OPT__(, ) __VA_ARGS__); \
})
/** Computes pointer relative to the start position of this stringify session. */
#define stringify_ptr(state, ...) ({                                                      \
  Stringify_State *_state_ = (state);                                                     \
  _state_->sb ? _state_->sb->data + _state_->start + WITH_DEFAULT(0, __VA_ARGS__) : NULL; \
})

/**
 * Formats output directly to `FILE *f` using `some_sb_append` without allocating a long-lived string.
 *
 * Example of usage:
 * ```c
  fprintf_stringify(stderr, sb_append, 't');
 * ```
 */
#define fprintf_stringify(f, some_sb_append, ...) ({ \
  String_Builder sb = {0};                           \
  some_sb_append(&sb __VA_OPT__(, ) __VA_ARGS__);    \
  fprintf((f), "%.*s", (int)sb.count, sb.data);      \
  sb_free(&sb);                                      \
})
/**
 * Formats directly to `stdout` using `some_sb_append`.
 *
 * Example of usage:
 * ```c
  printf_stringify(sb_append, 't');
 * ```
 */
#define printf_stringify(some_sb_append, ...) fprintf_stringify(stdout, some_sb_append __VA_OPT__(, ) __VA_ARGS__)

/** Consumes `sb`, null-terminates it, transfers dynamic buffer ownership to a `String`. */
#define string_from_sb(sb) ({                                 \
  String_Builder *_sb_ = (sb);                                \
  sb_null_terminate(_sb_);                                    \
  String string = {.data = _sb_->data, .count = _sb_->count}; \
  *da__reassign(_sb_) = NULL;                                 \
  _sb_->count = 0;                                            \
  _sb_->capacity = 0;                                         \
  string;                                                     \
})
/** Runs `some_sb_append` to populate a builder and returns an owning `String`. */
#define string_from(some_sb_append, ...) ({                               \
  String_Builder sb = sb_copy(some_sb_append __VA_OPT__(, ) __VA_ARGS__); \
  string_from_sb(&sb);                                                    \
})
/** Initializes an immutable `String` wrapper pointing directly to `str`. */
#define string_from_strlit(str) (String){.data = str, .count = sizeof(str) - 1}
/** Asserts `dst` is empty, then copies fields from `src`. */
static inline void string_assign(String *dst, String src) {
  assert(dst && !dst->count && !dst->data);
  memcpy(dst, &src, sizeof(String));
}
/** Frees the heap allocation backing `str` and zeroes out the struct. */
#define string_free(str) ({          \
  typeof(*(str)) *_str_ = (str);     \
  char *data = *da__reassign(_str_); \
  if (data) {                        \
    free(data);                      \
    memset(str, 0, sizeof(String));  \
  }                                  \
})

/** Constructs a `String_View` from pointer and length. */
#define sv_from_parts(data_, count_) ((String_View){.data = (data_), .count = (count_)})
/** Polymorphic conversion macro mapping any String_View-like into `String_View`. */
#define sv_from_like(value) _Generic((value), String_View: (value), default: ({  \
                                       typeof((value)) _val_ = (value);          \
                                       const char *const data = _val_.data;      \
                                       sv_from_parts((char *)data, _val_.count); \
                                     }))
/** Creates a `String_View` from a compile-time string literal. */
#define sv_from_strlit(str) ((String_View){.count = sizeof(str) - 1, .data = (str)})
/** Creates a `String_View` from a standard C-string (`strlen`). */
#define sv_from_cstr(str) sv_from_parts((str), strlen(str))

static inline bool sv__eq(String_View a, String_View b) {
  if (a.count != b.count) return false;
  return memcmp(a.data, b.data, a.count) == 0;
}
/** Compares two string views. */
#define sv_eq(a, b) sv__eq(sv_from_like(a), sv_from_like(b))

static inline bool sv__ends_with(String_View sv, String_View suffix) {
  if (sv.count < suffix.count) return false;
  const char *tail = sv.data + sv.count - suffix.count;
  return memcmp(tail, suffix.data, suffix.count) == 0;
}
/** Returns `true` if `sv` starts with `prefix`. */
#define sv_ends_with(sv, suffix) sv__ends_with(sv_from_like(sv), sv_from_like(suffix))

static inline bool sv__starts_with(String_View sv, String_View prefix) {
  if (sv.count < prefix.count) return false;
  return memcmp(sv.data, prefix.data, prefix.count) == 0;
}
/** Returns `true` if `sv` ends with `suffix`. */
#define sv_starts_with(sv, prefix) sv__starts_with(sv_from_like(sv), sv_from_like(prefix))

static inline String_View sv__chop_left(String_View *sv, size_t count) {
  if (count > sv->count) count = sv->count;
  String_View result = sv_from_parts(sv->data, count);
  sv->data += count;
  sv->count -= count;
  return result;
}
/**
 * Mutates `sv` by removing `n` elements (default `1`) from the left
 * and returns the chopped prefix.
 */
#define sv_chop_left(sv, ...) sv__chop_left((sv), WITH_DEFAULT(1, __VA_ARGS__))

static inline String_View sv__chop_right(String_View *sv, size_t count) {
  if (count > sv->count) count = sv->count;
  String_View result = sv_from_parts(sv->data + sv->count - count, count);
  sv->count -= count;
  return result;
}
/**
 * Mutates `sv` by removing `n` elements (default `1`) from the right
 * and returns the chopped suffix.
 */
#define sv_chop_right(sv, ...) sv__chop_right((sv), WITH_DEFAULT(1, __VA_ARGS__))

/** Searches forward for element matching `expr`. Same as `da_find_expr`. */
#define sv_find_expr(sv, id, expr) da_find_expr((sv), id, expr)
/** Searches forward for element satisfying `predicate`. Same as `da_find_pred`. */
#define sv_find_pred(sv, predicate) da_find_pred((sv), predicate)
/** Searches backward for element matching `expr`. Same as `da_find_right_expr`. */
#define sv_find_right_expr(sv, id, expr) da_find_right_expr((sv), id, expr)
/** Searches backward for element satisfying `predicate`. Same as `da_find_right_pred`. */
#define sv_find_right_pred(sv, predicate) da_find_right_pred((sv), predicate)

/** Chops prefix while `expr` evaluates to true. Same as `da_slice_chop_while_expr`. */
#define sv_chop_while_expr(sv, id, expr) da_slice_chop_while_expr((sv), id, expr)
/** Chops prefix while `predicate` evaluates to true. Same as `da_slice_chop_while_pred`. */
#define sv_chop_while_pred(sv, predicate) da_slice_chop_while_pred((sv), predicate)
String_View sv__chop_while_int(String_View *sv, int (*predicate)(int x));
String_View sv__chop_while(String_View *sv, bool (*predicate)(char character));
String_View sv__chop_while_i(String_View *sv, bool (*predicate)(char character, size_t index));
/** Polymorphic chop-left macro accepting. */
#define sv_chop_while(sv, predicate) _Generic((predicate),                                \
    int (*predicate)(int x): sv__chop_while_int((sv), (predicate)),                       \
    bool (*predicate)(char character, size_t index): sv__chop_while_i((sv), (predicate)), \
    bool (*predicate)(char character): sv__chop_while((sv), (predicate)))

/** Chops suffix from right while `expr` is true. Same as `da_slice_chop_right_while_expr`. */
#define sv_chop_right_while_expr(sv, id, expr) da_slice_chop_right_while_expr((sv), id, expr)
/** Chops suffix from right while `predicate` is true. Same as `da_slice_chop_right_while_pred`. */
#define sv_chop_right_while_pred(sv, predicate) da_slice_chop_right_while_pred((sv), predicate)
String_View sv__chop_right_while_int(String_View *sv, int (*predicate)(int x));
String_View sv__chop_right_while(String_View *sv, bool (*predicate)(char character));
String_View sv__chop_right_while_i(String_View *sv, bool (*predicate)(char character, size_t index));
/** Polymorphic chop-right macro accepting. */
#define sv_chop_right_while(sv, predicate) _Generic((predicate),                                \
    int (*predicate)(int x): sv__chop_right_while_int((sv), (predicate)),                       \
    bool (*predicate)(char character, size_t index): sv__chop_right_while_i((sv), (predicate)), \
    bool (*predicate)(char character): sv__chop_right_while((sv), (predicate)))

/** Chops prefix up to first match of `expr`. Same as `da_slice_chop_by_delim_expr`. */
#define sv_chop_by_delim_expr(sv, id, expr) da_slice_chop_by_delim_expr((sv), id, expr)
/** Chops prefix up to first match of `predicate`. Same as `da_slice_chop_by_delim_pred`. */
#define sv_chop_by_delim_pred(sv, predicate) da_slice_chop_by_delim_pred((sv), predicate)
String_View sv__chop_by_delim_int(String_View *sv, int (*predicate)(int x));
String_View sv__chop_by_delim(String_View *sv, bool (*predicate)(char character));
String_View sv__chop_by_delim_i(String_View *sv, bool (*predicate)(char character, size_t index));
/** Polymorphic delimiter chop accepting single-char predicate variants. */
#define sv_chop_by_delim(sv, predicate) _Generic((predicate),                                \
    int (*predicate)(int x): sv__chop_by_delim_int((sv), (predicate)),                       \
    bool (*predicate)(char character, size_t index): sv__chop_by_delim_i((sv), (predicate)), \
    bool (*predicate)(char character): sv__chop_by_delim((sv), (predicate)))

/** Chops suffix back to last match of `expr`. Same as `da_slice_chop_right_by_delim_expr`. */
#define sv_chop_right_by_delim_expr(sv, id, expr) da_slice_chop_right_by_delim_expr((sv), id, expr)
/** Chops suffix back to last match of `predicate`. Same as `da_slice_chop_right_by_delim_pred`. */
#define sv_chop_right_by_delim_pred(sv, predicate) da_slice_chop_right_by_delim_pred((sv), predicate)
String_View sv__chop_right_by_delim_int(String_View *sv, int (*predicate)(int x));
String_View sv__chop_right_by_delim(String_View *sv, bool (*predicate)(char character));
String_View sv__chop_right_by_delim_i(String_View *sv, bool (*predicate)(char character, size_t index));
/** Polymorphic delimiter chop-right accepting single-char predicate variants. */
#define sv_chop_right_by_delim(sv, predicate) _Generic((predicate),                                \
    int (*predicate)(int x): sv__chop_right_by_delim_int((sv), (predicate)),                       \
    bool (*predicate)(char character, size_t index): sv__chop_right_by_delim_i((sv), (predicate)), \
    bool (*predicate)(char character): sv__chop_right_by_delim((sv), (predicate)))

/** Strips leading whitespace (`isspace`) from `sv`. */
static inline String_View sv_trim_left(String_View *sv) {
  return sv__chop_by_delim_int(sv, isspace);
}
/** Strips trailing whitespace (`isspace`) from `sv`. */
static inline String_View sv_trim_right(String_View *sv) {
  return sv__chop_right_by_delim_int(sv, isspace);
}
/** Trims both leading and trailing whitespace from `sv`. */
static inline size_t sv_trim(String_View *sv) {
  String_View start = sv_trim_left(sv);
  String_View end = sv_trim_right(sv);
  return start.count + end.count;
}

/** Splits `sv` at the first occurrence of character `delimeter`. */
String_View sv_chop_by_char_delim(String_View *sv, char delimeter);
/** Splits `sv` from the right at the last occurrence of character `delimeter`. */
String_View sv_chop_right_by_char_delim(String_View *sv, char delimeter);

/** Splits `sv` at first occurrence of `String_View` `delimeter`. */
String_View sv_chop_by_sv(String_View *sv, String_View delimeter);
/** Splits `sv` from right at last occurrence of `String_View` `delimeter`. */
String_View sv_chop_right_by_sv(String_View *sv, String_View delimeter);

/** Format string constant `"%.*s"` for `printf` family functions. */
#define SV_FMT "%.*s"
/** Expands to `(int)(sv).count, (sv).data` for use with `SV_FMT`. */
#define sv_fmt_arg(sv) (int)(sv).count, (sv).data

#endif // STR_H

#if defined(STR_IMPL) && !defined(STR_IMPL_C)
#define STR_IMPL_C

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
  size_t pad = size - rem;
  da_reserve(sb, sb->count + pad);
  memset(sb->data + sb->count, filler, pad);
  sb->count += pad;
  return pad;
}

String_View sv__chop_while_int(String_View *sv, int (*predicate)(int x)) {
  return sv_chop_while_expr(sv, character, (bool)predicate((int)*character));
}

String_View sv__chop_while(String_View *sv, bool (*predicate)(char character)) {
  return sv_chop_while_expr(sv, character, predicate(*character));
}

String_View sv__chop_while_i(String_View *sv, bool (*predicate)(char character, size_t index)) {
  return sv_chop_while_expr(sv, character, predicate(*character, character_index));
}

String_View sv__chop_right_while_int(String_View *sv, int (*predicate)(int x)) {
  return sv_chop_right_while_expr(sv, character, (bool)predicate((int)*character));
}

String_View sv__chop_right_while(String_View *sv, bool (*predicate)(char character)) {
  return sv_chop_right_while_expr(sv, character, predicate(*character));
}

String_View sv__chop_right_while_i(String_View *sv, bool (*predicate)(char character, size_t index)) {
  return sv_chop_right_while_expr(sv, character, predicate(*character, character_index));
}

String_View sv__chop_by_delim_int(String_View *sv, int (*predicate)(int x)) {
  return sv_chop_by_delim_expr(sv, character, (bool)predicate((int)*character));
}

String_View sv__chop_by_delim(String_View *sv, bool (*predicate)(char character)) {
  return sv_chop_by_delim_expr(sv, character, predicate(*character));
}

String_View sv__chop_by_delim_i(String_View *sv, bool (*predicate)(char character, size_t index)) {
  return sv_chop_by_delim_expr(sv, character, predicate(*character, character_index));
}

String_View sv__chop_right_by_delim_int(String_View *sv, int (*predicate)(int x)) {
  return sv_chop_right_by_delim_expr(sv, character, (bool)predicate((int)*character));
}

String_View sv__chop_right_by_delim(String_View *sv, bool (*predicate)(char character)) {
  return sv_chop_right_by_delim_expr(sv, character, predicate(*character));
}

String_View sv__chop_right_by_delim_i(String_View *sv, bool (*predicate)(char character, size_t index)) {
  return sv_chop_right_by_delim_expr(sv, character, predicate(*character, character_index));
}

String_View sv_chop_by_char_delim(String_View *sv, char delimeter) {
  return sv_chop_by_delim_expr(sv, character, *character == delimeter);
}

String_View sv_chop_right_by_char_delim(String_View *sv, char delimeter) {
  return sv_chop_right_by_delim_expr(sv, character, *character == delimeter);
}

String_View sv_chop_by_sv(String_View *sv, String_View delimeter) {
  if (!sv->count || !delimeter.count) return (String_View){.data = sv->data, .count = 0};
  if (sv->count == delimeter.count) {
    if (memcmp(sv->data, delimeter.data, delimeter.count) != 0) goto not_found;
    char *data = sv->data;
    sv->data += delimeter.count;
    sv->count = 0;
    return (String_View){.data = data, .count = 0};
  }
  if (sv->count < delimeter.count) goto not_found;
  size_t index = 0, count_no_del = sv->count - delimeter.count;
  while (index <= count_no_del) {
    if (memcmp(sv->data + index, delimeter.data, delimeter.count) == 0) break;
    index += 1;
  }
  if (index >= count_no_del) goto not_found;
  String_View result = {.data = sv->data, .count = index};
  index += delimeter.count;
  sv->data += index;
  sv->count -= index;
  return result;

not_found:
  String_View error = {.data = NULL, .count = sv->count};
  sv->data += sv->count;
  sv->count = 0;
  return error;
}

String_View sv_chop_right_by_sv(String_View *sv, String_View delimeter) {
  if (!sv->count || !delimeter.count) return (String_View){.data = sv->data, .count = 0};
  if (sv->count == delimeter.count) {
    if (memcmp(sv->data, delimeter.data, delimeter.count) != 0) goto not_found;
    char *data = sv->data;
    sv->data += delimeter.count;
    sv->count = 0;
    return (String_View){.data = data, .count = 0};
  }
  if (sv->count < delimeter.count) goto not_found;
  size_t index = sv->count - delimeter.count;
  while (index > 0) {
    if (memcmp(sv->data + index - 1, delimeter.data, delimeter.count) == 0) break;
    index -= 1;
  }
  if (index == 0) goto not_found;
  index -= 1;
  size_t del_index = index + delimeter.count;
  String_View result = {.data = sv->data + del_index, .count = sv->count - del_index};
  sv->count = index;
  return result;

not_found:
  String_View error = {.data = NULL, .count = sv->count};
  sv->count = 0;
  return error;
}

#endif // STR_IMPL_C

/**
 * This is free and unencumbered software released into the public domain.
 *
 * Anyone is free to copy, modify, publish, use, compile, sell, or
 * distribute this software, either in source code form or as a compiled
 * binary, for any purpose, commercial or non-commercial, and by any
 * means.
 *
 * In jurisdictions that recognize copyright laws, the author or authors
 * of this software dedicate any and all copyright interest in the
 * software to the public domain. We make this dedication for the benefit
 * of the public at large and to the detriment of our heirs and
 * successors. We intend this dedication to be an overt act of
 * relinquishment in perpetuity of all present and future rights to this
 * software under copyright law.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * For more information, please refer to <https://unlicense.org/>
 */
