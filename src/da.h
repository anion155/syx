#ifndef DA_H
#define DA_H

#include <assert.h>
#include <magic.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifndef DA_INIT_CAP
#  define DA_INIT_CAP 256
#endif

#define Da(Value, ...) \
  struct __VA_ARGS__ { \
    size_t count;      \
    size_t capacity;   \
    Value *data;       \
  }

#define Da_Slice(Value, ...) \
  struct __VA_ARGS__ {       \
    size_t count;            \
    const Value *data;       \
  }

#define da_free(da) ({    \
  typeof(da) _da_ = (da); \
  free(_da_->data);       \
  _da_->data = NULL;      \
  _da_->capacity = 0;     \
  _da_->count = 0;        \
})

#define da_reserve(da, target_capacity) ({                                                      \
  typeof(da) _da_ = (da);                                                                       \
  if ((target_capacity) > _da_->capacity) {                                                     \
    if (_da_->capacity == 0) _da_->capacity = DA_INIT_CAP;                                      \
    while ((target_capacity) > _da_->capacity) _da_->capacity *= 2;                             \
    _da_->data = (typeof(_da_->data))realloc(_da_->data, _da_->capacity * sizeof(*_da_->data)); \
    assert(_da_->data != NULL && "Failed to reallocate dynamic array");                         \
  }                                                                                             \
  _da_->capacity;                                                                               \
})

#define da_reserve_exact(da, target_capacity) ({                                                   \
  typeof(da) _da_ = (da);                                                                          \
  if ((target_capacity) > _da_->capacity) {                                                        \
    _da_->capacity = (target_capacity);                                                            \
    _da_->data = (typeof(_da_->data))realloc(_da_->data, (target_capacity) * sizeof(*_da_->data)); \
    assert(_da_->data != NULL && "Failed to reallocate dynamic array");                            \
  }                                                                                                \
  _da_->capacity;                                                                                  \
})

#define da_trim_realloc(da) ({                                                                     \
  typeof(da) _da__ = (da);                                                                         \
  if (_da__->capacity != _da__->count) {                                                           \
    if (_da__->count == 0) {                                                                       \
      da_free(_da__);                                                                              \
    } else {                                                                                       \
      _da__->capacity = _da__->count;                                                              \
      _da__->data = (typeof(_da_->data))realloc(_da__->data, _da__->count * sizeof(*_da__->data)); \
      assert(_da__->data != NULL && "Failed to reallocate dynamic array");                         \
    }                                                                                              \
  }                                                                                                \
  _da__->capacity;                                                                                 \
})

#define da_first(da) ({   \
  typeof(da) _da_ = (da); \
  assert(_da_->count);    \
  _da_->data[0];          \
})

#define da_last(da) ({         \
  typeof(da) _da_ = (da);      \
  assert(_da_->count);         \
  _da_->data[_da_->count - 1]; \
})

#define da_append(da, item) ({         \
  typeof(da) _da__ = (da);             \
  da_reserve(_da__, _da__->count + 1); \
  _da__->data[_da__->count] = (item);  \
  _da__->count += 1;                   \
})

#define da_append_many_n(da, new_items, new_count) ({                                  \
  typeof(da) _da__ = (da);                                                             \
  da_reserve(_da__, _da__->count + (new_count));                                       \
  memcpy(_da__->data + _da__->count, (new_items), (new_count) * sizeof(*_da__->data)); \
  _da__->count += (new_count);                                                         \
})
#define da_append_many(da, ...) ({                                  \
  typeof(da) _da___ = (da);                                         \
  __VA_OPT__(                                                       \
      typeof(*_da___->data) items[] = {__VA_ARGS__};                \
      size_t count = sizeof(items) / sizeof(typeof(*_da___->data)); \
      da_append_many_n(_da___, items, count);)                      \
  _da___->count;                                                    \
})

#define da_pop(da) ({                         \
  typeof(da) _da__ = (da);                    \
  typeof(*_da__->data) last = da_last(_da__); \
  _da__->count -= 1;                          \
  last;                                       \
})

#define da_resize(da, new_size) ({ \
  typeof(da) _da__ = (da);         \
  da_reserve(_da__, (new_size));   \
  _da__->count = (new_size);       \
})

#define da_remove_unordered(da, index) ({        \
  typeof(da) _da_ = (da);                        \
  assert(index < _da_->count);                   \
  _da_->data[index] = _da_->data[--_da_->count]; \
  (void)0;                                       \
})

#define da_foreach(da, it)                                        \
  for (typeof(*(da)) *_da_##it = (da); _da_##it; _da_##it = NULL) \
    for (typeof(*_da_##it->data) *it = _da_##it->data, *last = it + _da_##it->count; it < last; ++it)

#define da_slice(da, Slice_Type, ...) ({                                                                \
  typeof(da) _da_ = (da);                                                                               \
  size_t start = WITH_DEFAULT(0, __VA_ARGS__);                                                          \
  assert(_da_->count >= start);                                                                         \
  size_t slice_count = EXPAND(WITH_DEFAULT, _da_->count - start __VA_OPT__(, ) REST_ARGS(__VA_ARGS__)); \
  assert(_da_->count >= start + slice_count);                                                           \
  (Slice_Type){.data = _da_->data + start, .count = slice_count};                                       \
})

#define da_slice_whole(da, Slice_Type) ({                 \
  typeof(da) _da_ = (da);                                 \
  (Slice_Type){.data = _da_->data, .count = _da_->count}; \
})

#define da_assert_is_slice(da) ({                                      \
  typeof(da) _da_ = (da);                                              \
  _Static_assert(                                                      \
      _Generic(&_da_->data,                                            \
          const typeof(*_da_->data) **: true,                          \
          default: false),                                             \
      "expected pointer to a Da_Slice (not a full Da dynamic array)"); \
  (void)0;                                                             \
})

#define da_slice_shift(da) ({                   \
  typeof(da) _da__ = (da);                      \
  da_assert_is_slice(_da__);                    \
  typeof(*_da__->data) first = da_first(_da__); \
  _da__->data += 1;                             \
  _da__->count -= 1;                            \
  first;                                        \
})

#define da_slice_chop_left(da, ...) ({                       \
  typeof(da) _da__ = (da);                                   \
  da_assert_is_slice(_da__);                                 \
  size_t n = WITH_DEFAULT(1, __VA_ARGS__);                   \
  if (n > _da__->count) n = _da__->count;                    \
  typeof(*_da__) result = {.data = _da__->data, .count = n}; \
  _da__->data += n;                                          \
  _da__->count -= n;                                         \
  result;                                                    \
})

#define da_slice_chop_right(da, ...) ({                                           \
  typeof(da) _da__ = (da);                                                        \
  da_assert_is_slice(_da__);                                                      \
  size_t n = WITH_DEFAULT(1, __VA_ARGS__);                                        \
  if (n > _da__->count) n = _da__->count;                                         \
  typeof(*_da__) result = {.data = _da__->data + (_da__->count - n), .count = n}; \
  _da__->count -= n;                                                              \
  result;                                                                         \
})

#define da_slice_chop_while(da, item_var, predicate) ({           \
  typeof(da) _da__ = (da);                                        \
  da_assert_is_slice(_da__);                                      \
  size_t index = 0;                                               \
  while (index < _da__->count) {                                  \
    const typeof(*_da__->data) *item_var = &(_da__->data)[index]; \
    UNUSED(item_var);                                             \
    if (!(predicate)) break;                                      \
    index += 1;                                                   \
  }                                                               \
  typeof(*_da__) result = {.data = _da__->data, .count = index};  \
  _da__->count -= index;                                          \
  _da__->data += index;                                           \
  result;                                                         \
})

#endif // DA_H
