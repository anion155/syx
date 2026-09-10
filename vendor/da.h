#ifndef DA_H
#define DA_H

#include <assert.h>
#include <defines.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifndef DA_INIT_CAP
#  define DA_INIT_CAP 256
#endif

#define Da(Value, ...)         \
  struct __VA_ARGS__ {         \
    typeof(Value) *const data; \
    size_t count;              \
    size_t capacity;           \
  }

#define Da_Slice(Value, ...) \
  struct __VA_ARGS__ {       \
    typeof(Value) *data;     \
    size_t count;            \
  }

#define da__reassign(da) ({                           \
  typeof(da) _da_dnc_ = (da);                         \
  (typeof_unqual(*_da_dnc_->data) **)&_da_dnc_->data; \
})
#define da_free(da) ({            \
  typeof(da) _da_f_ = (da);       \
  if (_da_f_->data) {             \
    free(_da_f_->data);           \
    *da__reassign(_da_f_) = NULL; \
    _da_f_->capacity = 0;         \
    _da_f_->count = 0;            \
  }                               \
})

#define da_reserve(da, target_capacity) ({                                                                         \
  typeof(da) _da_r_ = (da);                                                                                        \
  if ((target_capacity) > _da_r_->capacity) {                                                                      \
    if (_da_r_->capacity == 0) _da_r_->capacity = DA_INIT_CAP;                                                     \
    while ((target_capacity) > _da_r_->capacity) _da_r_->capacity *= 2;                                            \
    *da__reassign(_da_r_) = (typeof(_da_r_->data))realloc(_da_r_->data, _da_r_->capacity * sizeof(*_da_r_->data)); \
    assert(_da_r_->data != NULL && "Failed to reallocate dynamic array");                                          \
  }                                                                                                                \
  _da_r_->capacity;                                                                                                \
})

#define da_reserve_exact(da, target_capacity) ({                                                                        \
  typeof(da) _da_re_ = (da);                                                                                            \
  if ((target_capacity) > _da_re_->capacity) {                                                                          \
    _da_re_->capacity = (target_capacity);                                                                              \
    *da__reassign(_da_re_) = (typeof(_da_re_->data))realloc(_da_re_->data, (target_capacity) * sizeof(*_da_re_->data)); \
    assert(_da_re_->data != NULL && "Failed to reallocate dynamic array");                                              \
  }                                                                                                                     \
  _da_re_->capacity;                                                                                                    \
})

#define da_trim_realloc(da) ({                                                                                         \
  typeof(da) _da_tr_ = (da);                                                                                           \
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

#define da_first(da) ({     \
  typeof(da) _da_f_ = (da); \
  assert(_da_f_.count);     \
  _da_f_.data[0];           \
})

#define da_last(da) ({           \
  typeof(da) _da_l_ = (da);      \
  assert(_da_l_.count);          \
  _da_l_.data[_da_l_.count - 1]; \
})

#define da_at(da, index) ({                       \
  typeof(da) _da_a_ = (da);                       \
  size_t _index_ = (index);                       \
  assert(_da_a_.count && _da_a_.count > _index_); \
  _da_a_.data[_index_];                           \
})

#define da_append(da, item) ({           \
  typeof(da) _da_a_ = (da);              \
  da_reserve(_da_a_, _da_a_->count + 1); \
  _da_a_->data[_da_a_->count] = (item);  \
  _da_a_->count += 1;                    \
  1;                                     \
})

#define da_append_many_n(da, new_items, new_count) ({                                           \
  typeof(da) _da_amn_ = (da);                                                                   \
  size_t _new_count_ = (new_count);                                                             \
  da_reserve(_da_amn_, _da_amn_->count + _new_count_);                                          \
  memcpy(_da_amn_->data + _da_amn_->count, (new_items), _new_count_ * sizeof(*_da_amn_->data)); \
  _da_amn_->count += _new_count_;                                                               \
  _new_count_;                                                                                  \
})
#define da_append_many(da, ...) ({                              \
  typeof(da) _da_am_ = (da);                                    \
  size_t _added_ = 0;                                           \
  __VA_OPT__(                                                   \
      typeof(*_da_am_->data) items[] = {__VA_ARGS__};           \
      _added_ = sizeof(items) / sizeof(typeof(*_da_am_->data)); \
      da_append_many_n(_da_am_, items, _added_);)               \
  _added_;                                                      \
})

#define da_pop(da) ({                            \
  typeof(da) _da_p_ = (da);                      \
  typeof(*_da_p_->data) last = da_last(*_da_p_); \
  _da_p_->count -= 1;                            \
  last;                                          \
})

#define da_resize(da, new_size) ({ \
  typeof(da) _da_r_ = (da);        \
  da_reserve(_da_r_, (new_size));  \
  _da_r_->count = (new_size);      \
})

#define da_remove_unordered(da, index) ({                 \
  typeof(da) _da_ru_ = (da);                              \
  assert(index < _da_ru_->count);                         \
  _da_ru_->data[index] = _da_ru_->data[--_da_ru_->count]; \
  _da_ru_->count;                                         \
})

#define da_foreach(da, it)                                        \
  for (typeof(*(da)) *_da_##it = (da); _da_##it; _da_##it = NULL) \
    for (typeof(*_da_##it->data) *it = _da_##it->data, *_last_##it = it + _da_##it->count; it < _last_##it; ++it)

#define da_find_macro(da, item_var, predicate) ({   \
  typeof(da) _da_fm_ = (da);                        \
  size_t index = 0;                                 \
  const typeof(*_da_fm_.data) *data = _da_fm_.data; \
  UNUSED(data);                                     \
  size_t count = _da_fm_.count;                     \
  UNUSED(count);                                    \
  const typeof(*_da_fm_.data) *item_var;            \
  UNUSED(item_var);                                 \
  while (index < _da_fm_.count) {                   \
    item_var = &(_da_fm_.data)[index];              \
    if (!(predicate)) break;                        \
    index += 1;                                     \
  }                                                 \
  index;                                            \
})

#define da_slice_init(da, ...) {.data = (da).data + WITH_DEFAULT(0, __VA_ARGS__), .count = WITH_DEFAULT(((da).count - WITH_DEFAULT(0, __VA_ARGS__)), SECOND_ARG(__VA_ARGS__, ))}

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

#define da_slice_whole(da, Slice_Type) ({                     \
  typeof(da) _da_sw_ = (da);                                  \
  (Slice_Type){.data = _da_sw_.data, .count = _da_sw_.count}; \
})

#define da_slice_shift(da) ({                      \
  typeof(da) _ds_s_ = (da);                        \
  typeof(*_ds_s_->data) first = da_first(*_ds_s_); \
  _ds_s_->data += 1;                               \
  _ds_s_->count -= 1;                              \
  first;                                           \
})

#define da_slice_chop_left(da, ...) ({                           \
  typeof(da) _ds_cl_ = (da);                                     \
  size_t n = WITH_DEFAULT(1, __VA_ARGS__);                       \
  if (n > _ds_cl_->count) n = _ds_cl_->count;                    \
  typeof(*_ds_cl_) result = {.data = _ds_cl_->data, .count = n}; \
  _ds_cl_->data += n;                                            \
  _ds_cl_->count -= n;                                           \
  result;                                                        \
})

#define da_slice_chop_right(da, ...) ({                                                 \
  typeof(da) _ds_cr_ = (da);                                                            \
  size_t n = WITH_DEFAULT(1, __VA_ARGS__);                                              \
  if (n > _ds_cr_->count) n = _ds_cr_->count;                                           \
  typeof(*_ds_cr_) result = {.data = _ds_cr_->data + (_ds_cr_->count - n), .count = n}; \
  _ds_cr_->count -= n;                                                                  \
  result;                                                                               \
})

#define da_slice_chop_while_macro(da, item_var, predicate) ({          \
  typeof(da) _ds_cwm_ = (da);                                          \
  size_t index = da_find_macro(*_ds_cwm_, item_var, predicate);        \
  typeof(*_ds_cwm_) result = {.data = _ds_cwm_->data, .count = index}; \
  _ds_cwm_->count -= index;                                            \
  _ds_cwm_->data += index;                                             \
  result;                                                              \
})

#endif // DA_H
