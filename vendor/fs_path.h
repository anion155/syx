#ifndef FS_PATH_H
#define FS_PATH_H

#include <da.h>
#include <defines.h>
#include <stdlib.h>
#include <str.h>

typedef struct Fs_Path {
  String_Builder cstr;
  String_View *const data;
  size_t count;
  size_t capacity;
} Fs_Path;

#define fs_path_block(str, path_var)                              \
  for (                                                           \
      Fs_Path path_var = fs_path_parse(str), *_forb_ = &path_var; \
      _forb_;                                                     \
      ({                                                          \
        _forb_ = NULL;                                            \
        fs_path_free(&path_var);                                  \
      }))
#define fs_path_block_cstr(str, path_var) fs_path_block(sv_from_cstr(str), path_var)
#define fs_path_block_strlit(str, path_var) fs_path_block(sv_from_strlit(str), path_var)
#define fs_path_with_lvalue(Return_Type, block_header, block) ({ \
  Return_Type result = {0};                                      \
  block_header{result = block};                                  \
  result;                                                        \
})
#define fs_path_with_path(block_header, block) ({   \
  Fs_Path result = {0};                             \
  block_header { fs_path_assign(&result, block); }; \
  result;                                           \
})

#define fs_path_const(strlit, ...)                                            \
  {                                                                           \
      .data = ((String_View[]){__VA_ARGS__}),                                 \
      .capacity = sizeof((String_View[]){__VA_ARGS__}) / sizeof(String_View), \
      .count = sizeof((String_View[]){__VA_ARGS__}) / sizeof(String_View),    \
      .cstr = {.data = strlit, .capacity = sizeof(strlit) - 1, .count = sizeof(strlit) - 1}}
Fs_Path fs_path_copy(Fs_Path path);
#define fs_path_free(path) ({    \
  typeof(*path) *_fpt_ = (path); \
  sb_free(&_fpt_->cstr);         \
  da_free(_fpt_);                \
})
void fs_path_assign(Fs_Path *dst, Fs_Path src);
void fs_path_destructor(void *data);
void fs_path_trim(Fs_Path *path);

#define fs_path_is_absolute(path) ({     \
  typeof(path) _fpia_ = (path);          \
  _fpia_.count && !_fpia_.data[0].count; \
})
#define fs_path_is_relative(path) ({     \
  typeof(path) _fpir_ = (path);          \
  !_fpir_.count || _fpir_.data[0].count; \
})
#define fs_path_is_root(path) ({              \
  typeof(path) _fpir_ = (path);               \
  _fpir_.count == 1 && !_fpir_.data[0].count; \
})

static const Fs_Path fs_path_absolute_root;
static const Fs_Path fs_path_relative_root;

void fs__path_append(Fs_Path *path, String_View file);
#define fs_path_append(path, file) fs__path_append((path), sv_from_like(file))
#define fs_path_append_cstr(path, file) fs__path_append((path), sv_from_cstr(file))
#define fs_path_append_strlit(path, file) fs__path_append((path), sv_from_strlit(file))
void fs_path_pop(Fs_Path *path);
void fs_path_append_root(Fs_Path *path);
void fs_path_append_many_n(Fs_Path *path, String_View *files, size_t n);
#define fs_path_append_many(path, ...) IF_VA_OPT(                                                                            \
    fs_path_append_many_n((path), (String_View[]){__VA_ARGS__}, sizeof((String_View[]){__VA_ARGS__}) / sizeof(String_View)), \
    NOOP(),                                                                                                                  \
    __VA_ARGS__)
void fs_path_append_path(Fs_Path *path, Fs_Path other);

Fs_Path fs__path_parse(String_View str);
#define fs_path_parse(str) fs__path_parse(sv_from_like(str))
#define fs_path_parse_n(str, n) fs__path_parse(sv_from_parts(str, n))
#define fs_path_parse_cstr(str) fs__path_parse(sv_from_cstr(str))
#define fs_path_parse_strlit(str) fs__path_parse(sv_from_strlit(str))

Fs_Path fs_path_join(Fs_Path base, Fs_Path other);

String_View fs_path_file_name(Fs_Path path);
String_View fs_path_file_ext(Fs_Path path);
Fs_Path fs_path_dir_name(Fs_Path path);

#endif // FS_PATH_H

#if defined(FS_PATH_IMPL) && !defined(FS_PATH_IMPL_C)
#define FS_PATH_IMPL_C

static const Fs_Path fs_path_absolute_root = fs_path_const("/", sv_from_parts(NULL, 0));
static const Fs_Path fs_path_relative_root = fs_path_const(".");

constexpr String fs__path_current_dir = string_from_strlit(".");
constexpr String fs__path_parent_dir = string_from_strlit("..");

#define fs__path_with_sync(_path)                                         \
  for (                                                                   \
      Fs_Path *_fpws_ = (_path), **_forp_ = &_fpws_;                      \
      _forp_;                                                             \
      _forp_ = NULL)                                                      \
    for (                                                                 \
        const char *orig_cstr = _fpws_->cstr.data, **_forc_ = &orig_cstr; \
        _forc_;                                                           \
        ({                                                                \
          _forc_ = NULL;                                                  \
          sb_append_null(&_fpws_->cstr);                                  \
          fs__path_resync(_fpws_, orig_cstr);                             \
        }))

void fs__path_resync(Fs_Path *path, const char *orig_cstr) {
  if (path->cstr.data == orig_cstr) return;
  for (size_t index = 0; index < path->count; index += 1) {
    String_View *sv = path->data + index;
    if (!sv->data) continue;
    sv->data = path->cstr.data + (sv->data - orig_cstr);
  }
}

Fs_Path fs_path_copy(Fs_Path path) {
  Fs_Path result = {0};
  fs_path_append_path(&result, path);
  return result;
}

void fs_path_assign(Fs_Path *dst, Fs_Path src) {
  da_const_assign(dst, src);
}

void fs_path_destructor(void *data) {
  Fs_Path *path = data;
  fs_path_free(path);
}

void fs_path_trim(Fs_Path *path) {
  fs__path_with_sync(path) NOOP();
  da_trim_realloc(path);
}

void fs__path_append(Fs_Path *path, String_View file) {
  if (!file.count) return;
  ASSERT(sv_find_right_char(file, '/') == 0, "use fs_path_append_many to append multiple levels of dirictories");
  if (sv_eq(file, fs__path_current_dir)) return;
  if (sv_eq(file, fs__path_parent_dir) && path->count) {
    if (fs_path_is_root(*path)) return;
    String_View last = da_last(*path);
    if (!sv_eq(last, fs__path_parent_dir)) {
      path->cstr.count -= last.count;
      path->count -= 1;
      if (path->count && !fs_path_is_root(*path)) path->cstr.count -= 1;
      path->cstr.data[path->cstr.count] = '\0';
      return;
    }
  }
  size_t start;
  fs__path_with_sync(path) {
    if (path->count && !fs_path_is_root(*path)) sb_append(&path->cstr, '/');
    start = path->cstr.count;
    sb_append_sv(&path->cstr, file);
  }
  da_append(path, sv_from_parts(path->cstr.data + start, file.count));
}

void fs_path_pop(Fs_Path *path) {
  fs__path_append(path, sv_from_like(fs__path_parent_dir));
}

void fs_path_append_root(Fs_Path *path) {
  path->count = 0;
  path->cstr.count = 0;
  sb_append(&path->cstr, '/');
  sb_append_null(&path->cstr);
  da_append(path, sv_from_parts(path->cstr.data, 0));
}

void fs_path_append_many_n(Fs_Path *path, String_View *files, size_t n) {
  for (size_t index = 0; index < n; index += 1) {
    if (!files[index].count) continue;
    fs__path_append(path, files[index]);
  }
}

void fs_path_append_path(Fs_Path *path, Fs_Path other) {
  if (fs_path_is_absolute(other)) fs_path_append_root(path);
  fs_path_append_many_n(path, other.data, other.count);
}

Fs_Path fs__path_parse(String_View str) {
  Fs_Path path = {0};
  if (str.count && *str.data == '/') fs_path_append_root(&path);
  String_View start = sv_from_like(str);
  for (String_View it = sv_from_like(str); it.count; sv_chop_left(&it)) {
    if (*it.data != '/') continue;
    size_t count = it.data - start.data;
    if (count) fs__path_append(&path, sv_from_parts(start.data, count));
    start = sv_from_parts(it.data + 1, it.count - 1);
  }
  if (start.count) fs__path_append(&path, start);
  return path;
}

Fs_Path fs_path_join(Fs_Path base, Fs_Path other) {
  Fs_Path result = {0};
  if (!fs_path_is_absolute(other)) fs_path_append_path(&result, base);
  fs_path_append_path(&result, other);
  return result;
}

String_View fs_path_file_name(Fs_Path path) {
  if (!path.count || fs_path_is_root(path)) return sv_from_parts(NULL, 0);
  return path.data[path.count - 1];
}

String_View fs_path_file_ext(Fs_Path path) {
  String_View file_name = fs_path_file_name(path);
  if (!file_name.count) return sv_from_parts(NULL, 0);
  String_View ext = sv_chop_right_by_char_delim(&file_name, '.');
  if (ext.data == NULL) return sv_from_parts(NULL, 0);
  if (ext.data - file_name.data == 1) return sv_from_parts(NULL, 0);
  return ext;
}

Fs_Path fs_path_dir_name(Fs_Path path) {
  if (!path.count) return fs_path_relative_root;
  if (fs_path_is_root(path)) {
    Fs_Path result = {0};
    fs_path_append_root(&result);
    return result;
  }
  Fs_Path result = fs_path_copy(path);
  fs_path_pop(&result);
  return result;
}

#endif // FS_PATH_IMPL_C

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
