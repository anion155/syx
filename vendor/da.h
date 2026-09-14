/**
 * da.h - 0.3.0 - Public Domain - https://github.com/anion155/c-tools
 *
 * Dynamic Array utilities for c23.
 *
 * ## Usage example
 * ```c
   #define DA_IMPL
   #include "da.h"

   typedef Da(int, Int_Array) Int_Array;

   int main(void) {
     Int_Array values = {0};
     da_append(&values, 10);
     da_append_many(&values, 20, 30, 40);
     da_foreach(values, value) {
       printf("%d\n", *value);
     }
     da_free(&values);
   }
 * ```
 */

#ifndef DA_H
#define DA_H

#include <assert.h>
#include <defines.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/**
 * Array size on first initialization.
 */
#ifndef DA_INIT_CAP
#  define DA_INIT_CAP 256
#endif

/**
 * Macro to create dynamic array structure.
 * Own allocated memory, should be freed after use, which means `data` field is not supposed to be changed.
 */
#define Da(Value_Type, ...)         \
  struct FIRST_ARG(__VA_ARGS__) {   \
    typeof(Value_Type) *const data; \
    size_t count;                   \
    size_t capacity;                \
  }

/**
 * Macro to create dynamic array structure.
 * Does not own anything, `data` can be easily maniputed, but safety guaranteed only when moving forward while `count != 0`.
 */
#define Da_Slice(Value_Type, ...) \
  struct __VA_ARGS__ {            \
    typeof(Value_Type) *data;     \
    size_t count;                 \
  }

/**
 * Macro to create constant array structure compatible with dynamic array methods.
 * Own allocated memory, should be freed after use.
 */
#define Da_Const(Value_Type, ...)         \
  struct __VA_ARGS__ {                    \
    const typeof(Value_Type) *const data; \
    const size_t count;                   \
  }

/**
 * Returns the first element of the dynamic array.
 * Can be used with any da compatible structures.
 */
#define da_first(da) ({         \
  typeof((da)) *_da_f_ = &(da); \
  assert(_da_f_->count);        \
  _da_f_->data[0];              \
})

/**
 * Returns the last element of the dynamic array.
 * Can be used with any da compatible structures.
 */
#define da_last(da) ({             \
  typeof((da)) *_da_l_ = &(da);    \
  assert(_da_l_->count);           \
  _da_l_->data[_da_l_->count - 1]; \
})

/**
 * Returns the element at `index`.
 * Can be used with any da compatible structures.
 */
#define da_at(da, index) ({                         \
  typeof((da)) *_da_a_ = &(da);                     \
  size_t _index_ = (index);                         \
  assert(_da_a_->count && _da_a_->count > _index_); \
  _da_a_->data[_index_];                            \
})

/**
 * Iterates over `da`, creating a pointer loop variable named `id` and index `id##_index`.
 * Can be used with any da compatible structures.
 */
#define da_foreach(da, id)                                                             \
  for (typeof((da)) *_da_##id = &(da); _da_##id; _da_##id = NULL)                      \
    for (size_t id##_index = 0, count = _da_##id->count; !id##_index; id##_index += 1) \
      for (typeof(*_da_##id->data) *id = _da_##id->data; id##_index < count; id += 1, id##_index += 1)

/**
 * Returns index of first element satisfying `predicate`, or array's size if not found.
 * `predicate` - is supposed to be an expression returning implicitly convertable to `bool` value.
 * Available variables while in predicate:
 * - `id` - pointer to item,
 * - `id##_index` - current index,
 * - `id##_data` - pointer to start of the array,
 * - `id##_count` - count of the array.
 *
 * Can be used with any da compatible structures.
 */
#define da_find(da, id, predicate) ({                      \
  typeof((da)) *_da_fm_ = &(da);                           \
  const typeof(*_da_fm_->data) *id##_data = _da_fm_->data; \
  UNUSED(id##_data);                                       \
  size_t id##_count = _da_fm_->count;                      \
  UNUSED(id##_count);                                      \
  const typeof(*_da_fm_->data) *id;                        \
  UNUSED(id);                                              \
  size_t id##_index = 0;                                   \
  while (id##_index < id##_count) {                        \
    id = &id##_data[id##_index];                           \
    if (!(predicate)) break;                               \
    id##_index += 1;                                       \
  }                                                        \
  id##_index;                                              \
})

/**
 * Returns index of first element satisfying `predicate`, or array's size if not found.
 * `predicate` - is function or macro that accepts (size_t index, Value_Type item).
 * Can be used with any da compatible structures.
 */
#define da_find_macro(da, predicate) ({                       \
  typeof((da)) *_da_fm_ = &(da);                              \
  size_t _index_ = 0;                                         \
  while (_index_ < _da_fm_->count) {                          \
    if (!(predicate(_da_fm_->data[_index_], _index_))) break; \
    _index_ += 1;                                             \
  }                                                           \
  _index_;                                                    \
})

/**
 * Returns `index + 1` of first element from the end satisfying `predicate`, or `0` if not found.
 * `predicate` - is supposed to be an expression returning implicitly convertable to `bool` value.
 * Available variables while in predicate:
 * - `id` - pointer to item,
 * - `id##_index` - current index,
 * - `id##_data` - pointer to start of the array,
 * - `id##_count` - count of the array.
 *
 * Can be used with any da compatible structures.
 */
#define da_find_right(da, id, predicate) ({                  \
  typeof((da)) *_da_fmr_ = &(da);                            \
  const typeof(*_da_fmr_->data) *id##_data = _da_fmr_->data; \
  UNUSED(id##_data);                                         \
  size_t id##_count = _da_fmr_->count;                       \
  UNUSED(id##_count);                                        \
  const typeof(*_da_fmr_->data) *id;                         \
  UNUSED(id);                                                \
  ssize_t id##_index = id##_count;                           \
  while (id##_index > 0) {                                   \
    id = &id##_data[id##_index - 1];                         \
    if (!(predicate)) break;                                 \
    id##_index -= 1;                                         \
  }                                                          \
  id##_index;                                                \
})

/**
 * Returns `index + 1` of first element from the end satisfying `predicate`, or `0` if not found.
 * `predicate` - is function or macro that accepts (size_t index, Value_Type item).
 * Can be used with any da compatible structures.
 */
#define da_find_right_macro(da, predicate) ({                 \
  typeof((da)) *_da_fmr_ = &(da);                             \
  ssize_t _index_ = id##_count;                               \
  while (_index_ > 0) {                                       \
    if (!(predicate(_da_fm_->data[_index_], _index_))) break; \
    _index_ -= 1;                                             \
  }                                                           \
  _index_;                                                    \
})

/**
 * Internal helper to get modifiable pointer to `data.
 */
#define da__reassign(da) ({                    \
  typeof(*(da)) *_da_dnc_ = (da);              \
  (typeof(*_da_dnc_->data) **)&_da_dnc_->data; \
})

/**
 * Frees `data` buffer, zeroes `capacity` and `count`, and resets pointer to `NULL`.
 */
#define da_free(da) ({            \
  typeof(*(da)) *_da_f_ = (da);   \
  if (_da_f_->data) {             \
    free(_da_f_->data);           \
    *da__reassign(_da_f_) = NULL; \
    _da_f_->capacity = 0;         \
    _da_f_->count = 0;            \
  }                               \
})

/**
 * Ensures capacity is at least `target_capacity` by doubling capacity geometrically.
 */
#define da_reserve(da, target_capacity) ({                                                                         \
  typeof(*(da)) *_da_r_ = (da);                                                                                    \
  if ((target_capacity) > _da_r_->capacity) {                                                                      \
    if (_da_r_->capacity == 0) _da_r_->capacity = DA_INIT_CAP;                                                     \
    while ((target_capacity) > _da_r_->capacity) _da_r_->capacity *= 2;                                            \
    *da__reassign(_da_r_) = (typeof(_da_r_->data))realloc(_da_r_->data, _da_r_->capacity * sizeof(*_da_r_->data)); \
    assert(_da_r_->data != NULL && "Failed to reallocate dynamic array");                                          \
  }                                                                                                                \
  _da_r_->capacity;                                                                                                \
})

/**
 * Reallocates capacity to exactly `target_capacity` if needed without geometric growth.
 */
#define da_reserve_exact(da, target_capacity) ({                                                                        \
  typeof(*(da)) *_da_re_ = (da);                                                                                        \
  if ((target_capacity) > _da_re_->capacity) {                                                                          \
    _da_re_->capacity = (target_capacity);                                                                              \
    *da__reassign(_da_re_) = (typeof(_da_re_->data))realloc(_da_re_->data, (target_capacity) * sizeof(*_da_re_->data)); \
    assert(_da_re_->data != NULL && "Failed to reallocate dynamic array");                                              \
  }                                                                                                                     \
  _da_re_->capacity;                                                                                                    \
})

/**
 * Shrinks memory allocation so `capacity` matches `count`, or frees if `count == 0`.
 */
#define da_trim_realloc(da) ({                                                                                         \
  typeof(*(da)) *_da_tr_ = (da);                                                                                       \
  if (_da_tr_->capacity != _da_tr_->count) {                                                                           \
    if (_da_tr_->count == 0) {                                                                                         \
      da_free(_da_tr_);                                                                                                \
    } else {                                                                                                           \
      _da_tr_->capacity = _da_tr_->count;                                                                              \
      *da__reassign(_da_tr_) = (typeof(_da_tr_->data))realloc(_da_tr_->data, _da_tr_->count * sizeof(*_da_tr_->data)); \
      assert(_da_tr_->data != NULL && "Failed to reallocate dynamic array");                                           \
    }                                                                                                                  \
  }                                                                                                                    \
  _da_tr_->capacity;                                                                                                   \
})

/**
 * Reserves memory for `new_size` and sets `count = new_size`.
 */
#define da_resize(da, new_size) ({ \
  typeof(*(da)) *_da_r_ = (da);    \
  da_reserve(_da_r_, (new_size));  \
  _da_r_->count = (new_size);      \
})

/**
 * Appends a single `item` to the dynamic array, growing capacity if necessary.
 */
#define da_append(da, item) ({           \
  typeof(*(da)) *_da_a_ = (da);          \
  da_reserve(_da_a_, _da_a_->count + 1); \
  _da_a_->data[_da_a_->count] = (item);  \
  _da_a_->count += 1;                    \
  1;                                     \
})

/**
 * Appends `new_count` elements from buffer `new_items`.
 */
#define da_append_many_n(da, new_items, new_count) ({                                           \
  typeof(*(da)) *_da_amn_ = (da);                                                               \
  size_t _new_count_ = (new_count);                                                             \
  da_reserve(_da_amn_, _da_amn_->count + _new_count_);                                          \
  memcpy(_da_amn_->data + _da_amn_->count, (new_items), _new_count_ * sizeof(*_da_amn_->data)); \
  _da_amn_->count += _new_count_;                                                               \
  _new_count_;                                                                                  \
})

/**
 * Appends variadic literal items to `da`.
 */
#define da_append_many(da, ...) ({                              \
  typeof(*(da)) *_da_am_ = (da);                                \
  size_t _added_ = 0;                                           \
  __VA_OPT__(                                                   \
      typeof(*_da_am_->data) items[] = {__VA_ARGS__};           \
      _added_ = sizeof(items) / sizeof(typeof(*_da_am_->data)); \
      da_append_many_n(_da_am_, items, _added_);)               \
  _added_;                                                      \
})

/**
 * Returns the last element and decrements `count`.
 */
#define da_pop(da) ({                            \
  typeof(*(da)) *_da_p_ = (da);                  \
  typeof(*_da_p_->data) last = da_last(*_da_p_); \
  _da_p_->count -= 1;                            \
  last;                                          \
})

/**
 * Removes element at `index` by swapping it with the last element.
 */
#define da_remove_unordered(da, index) ({                     \
  typeof(*(da)) *_da_ru_ = (da);                              \
  size_t _index_ = (index);                                   \
  assert(_index_ < _da_ru_->count);                           \
  _da_ru_->data[_index_] = _da_ru_->data[_da_ru_->count - 1]; \
  _da_ru_->count -= 1;                                        \
})

/**
 * Removes element at `index` by moving all further elements left.
 */
#define da_remove_ordered(da, index) ({                                                                         \
  typeof(*(da)) *_da_ru_ = (da);                                                                                \
  assert(index < _da_ru_->count);                                                                               \
  memmove(_da_ru_->data + index, _da_ru_->data + index + 1, sizeof(*_da_ru_->data) * (_da_ru_->count - index)); \
  _da_ru_->count -= 1;                                                                                          \
})

/**
 * Struct initializer expression for slices, with optional `start` (default `0`) and `count` (defaults to remaining elements).
 */
#define da_slice_init(da, ...) {.data = (da).data + WITH_DEFAULT(0, __VA_ARGS__), .count = WITH_DEFAULT(((da).count - WITH_DEFAULT(0, __VA_ARGS__)), SECOND_ARG(__VA_ARGS__, ))}

/**
 * Returns a slice of `Slice_Type` starting at `start` (default `0`) for `count` elements (default remaining elements).
 */
#define da_slice(da, Slice_Type, ...) ({                                                                       \
  typeof(da) _da_s_ = (da);                                                                                    \
  size_t start = WITH_DEFAULT(0, __VA_ARGS__);                                                                 \
  assert(_da_s_.count >= start);                                                                               \
  size_t slice_count = EXPAND_MACRO(WITH_DEFAULT, _da_s_.count - start __VA_OPT__(, ) REST_ARGS(__VA_ARGS__)); \
  assert(_da_s_.count >= start + slice_count);                                                                 \
  typeof_unqual(*((Slice_Type){0}).data) *_data_ = _da_s_.data;                                                \
  UNUSED(_data_);                                                                                              \
  (Slice_Type){.data = (typeof(((Slice_Type){0}).data))_da_s_.data + start, .count = slice_count};             \
})

/**
 * Creates a `Slice_Type` putting in it full range of `da`.
 */
#define da_slice_whole(da, Slice_Type) ({                     \
  typeof((da)) _da_sw_ = (da);                                \
  (Slice_Type){.data = _da_sw_.data, .count = _da_sw_.count}; \
})

/**
 * Shifts slice head forward by 1, decrements `count`, and returns the dropped first element.
 */
#define da_slice_shift(das) ({                       \
  typeof(*(das)) *_das_s_ = (das);                   \
  typeof(*_das_s_->data) first = da_first(*_das_s_); \
  _das_s_->data += 1;                                \
  _das_s_->count -= 1;                               \
  first;                                             \
})

/**
 * Advances slice head past `n` elements (default `1`) and returns a new slice containing the chopped prefix.
 */
#define da_slice_chop_left(das, ...) ({                                  \
  typeof(*(das)) *_das_cl_ = (das);                                      \
  size_t _count_ = WITH_DEFAULT(1, __VA_ARGS__);                         \
  if (_count_ > _das_cl_->count) _count_ = _das_cl_->count;              \
  typeof(*_das_cl_) result = {.data = _das_cl_->data, .count = _count_}; \
  _das_cl_->data += _count_;                                             \
  _das_cl_->count -= _count_;                                            \
  result;                                                                \
})

/**
 * Shrinks slice tail by `n` elements (default `1`) and returns a new slice containing the chopped suffix.
 */
#define da_slice_chop_right(das, ...) ({                                                               \
  typeof(*(das)) *_das_cl_ = (das);                                                                    \
  size_t _count_ = WITH_DEFAULT(1, __VA_ARGS__);                                                       \
  if (_count_ > _das_cl_->count) _count_ = _das_cl_->count;                                            \
  typeof(*_das_cl_) result = {.data = _das_cl_->data + (_das_cl_->count - _count_), .count = _count_}; \
  _das_cl_->count -= _count_;                                                                          \
  result;                                                                                              \
})

/**
 * Chops and returns prefix of a slice while elements satisfy `predicate`.
 * `predicate` - is supposed to be an expression returning implicitly convertable to `bool` value.
 * Available variables while in predicate:
 * - `id` - pointer to item,
 * - `id##_index` - current index,
 * - `id##_data` - pointer to start of the array,
 * - `id##_count` - count of the array.
 */
#define da_slice_chop_while(das, id, predicate) ({     \
  typeof(*(das)) *_das_cwm_ = (das);                   \
  size_t _index_ = da_find(*_das_cwm_, id, predicate); \
  da_slice_chop_left(_das_cwm_, _index_);              \
})

/**
 * Chops and returns prefix of a slice while elements satisfy `predicate`.
 * `predicate` - is function or macro that accepts (size_t index, Value_Type item).
 */
#define da_slice_chop_while_macro(das, predicate) ({     \
  typeof(*(das)) *_das_cwm_ = (das);                     \
  size_t _index_ = da_find_macro(*_das_cwm_, predicate); \
  da_slice_chop_left(_das_cwm_, _index_);                \
})

/**
 * Chops and returns suffix of a slice while elements satisfy `predicate`.
 * `predicate` - is supposed to be an expression returning implicitly convertable to `bool` value.
 * Available variables while in predicate:
 * - `id` - pointer to item,
 * - `id##_index` - current index,
 * - `id##_data` - pointer to start of the array,
 * - `id##_count` - count of the array.
 */
#define da_slice_chop_right_while(das, id, predicate) ({        \
  typeof(*(das)) *_das_crwm_ = (das);                           \
  size_t _index_ = da_find_right(*_das_crwm_, id, (predicate)); \
  da_slice_chop_right(_das_crwm_, _das_crwm_->count - _index_); \
})

/**
 * Chops and returns suffix of a slice while elements satisfy `predicate`.
 * `predicate` - is function or macro that accepts (size_t index, Value_Type item).
 */
#define da_slice_chop_right_while_macro(das, predicate) ({      \
  typeof(*(das)) *_das_crwm_ = (das);                           \
  size_t _index_ = da_find_right_macro(*_das_crwm_, predicate); \
  da_slice_chop_right(_das_crwm_, _das_crwm_->count - _index_); \
})

/**
 * Chops slice up to (and including) the first element matching `predicate`.
 * `predicate` - is supposed to be an expression returning implicitly convertable to `bool` value.
 * Available variables while in predicate:
 * - `id` - pointer to item,
 * - `id##_index` - current index,
 * - `id##_data` - pointer to start of the array,
 * - `id##_count` - count of the array.
 */
#define da_slice_chop_by(das, id, predicate) ({           \
  typeof(*(das)) *_das_cbm_ = (das);                      \
  size_t _index_ = da_find(*_das_cbm_, id, !(predicate)); \
  typeof(*(das)) _head_;                                  \
  if (_index_ >= _das_cbm_->count) {                      \
    _head_.data = _das_cbm_->data;                        \
    _head_.count = 0;                                     \
    _das_cbm_->data += _das_cbm_->count;                  \
    _das_cbm_->count = 0;                                 \
  } else {                                                \
    _head_ = da_slice_chop_left(_das_cbm_, _index_ + 1);  \
  }                                                       \
  _head_;                                                 \
})

/**
 * Chops slice up to (and including) the first element matching `predicate`.
 * `predicate` - is function or macro that accepts (size_t index, Value_Type item).
 */
#define da_slice_chop_by_macro(das, predicate) ({                           \
  typeof(*(das)) *_das_cbm_ = (das);                                        \
  size_t _index_ = da_find(*_das_cbm_, _id_, !predicate(_id_, _id__index)); \
  typeof(*(das)) _head_;                                                    \
  if (_index_ >= _das_cbm_->count) {                                        \
    _head_.data = _das_cbm_->data;                                          \
    _head_.count = 0;                                                       \
    _das_cbm_->data += _das_cbm_->count;                                    \
    _das_cbm_->count = 0;                                                   \
  } else {                                                                  \
    _head_ = da_slice_chop_left(_das_cbm_, _index_ + 1);                    \
  }                                                                         \
  _head_;                                                                   \
})

/**
 * Chops slice from the end up to (and including) the last element matching `predicate`
 * `predicate` - is supposed to be an expression returning implicitly convertable to `bool` value.
 * Available variables while in predicate:
 * - `id` - pointer to item,
 * - `id##_index` - current index,
 * - `id##_data` - pointer to start of the array,
 * - `id##_count` - count of the array.
 */
#define da_slice_chop_right_by(das, id, predicate) ({                          \
  typeof(*(das)) *_das_crbm_ = (das);                                          \
  size_t _index_ = da_find_right(*_das_crbm_, id, !(predicate));               \
  typeof(*(das)) _tail_;                                                       \
  if (_index_ == 0) {                                                          \
    _tail_.data = _das_crbm_->data;                                            \
    _tail_.count = 0;                                                          \
    _das_crbm_->data += _das_crbm_->count;                                     \
    _das_crbm_->count = 0;                                                     \
  } else {                                                                     \
    _tail_ = da_slice_chop_right(_das_crbm_, _das_crbm_->count - _index_ + 1); \
  }                                                                            \
  _tail_;                                                                      \
})

/**
 * Chops slice from the end up to (and including) the last element matching `predicate`
 * `predicate` - is function or macro that accepts (size_t index, Value_Type item).
 */
#define da_slice_chop_right_by_macro(das, predicate) ({                            \
  typeof(*(das)) *_das_crbm_ = (das);                                              \
  size_t _index_ = da_find_right(*_das_crbm_, _id_, !predicate(_id_, _id__index)); \
  typeof(*(das)) _tail_;                                                           \
  if (_index_ == 0) {                                                              \
    _tail_.data = _das_crbm_->data;                                                \
    _tail_.count = 0;                                                              \
    _das_crbm_->data += _das_crbm_->count;                                         \
    _das_crbm_->count = 0;                                                         \
  } else {                                                                         \
    _tail_ = da_slice_chop_right(_das_crbm_, _das_crbm_->count - _index_ + 1);     \
  }                                                                                \
  _tail_;                                                                          \
})

/**
 * Constructs a read-only dynamic array compatible structure initialized from a compound literal array.
 */
#define da_const_init_from_arraylit(Value_Type, ...) EXPAND2(IF_VA_OPT(                                         \
    ({.data = (Value_Type[]){__VA_ARGS__}, .count = sizeof((Value_Type[]){__VA_ARGS__}) / sizeof(Value_Type)}), \
    ({.data = NULL, .count = 0}), __VA_ARGS__))

#endif // DA_H

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
