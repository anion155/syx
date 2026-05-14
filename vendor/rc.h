#ifndef RC_H
#define RC_H

#include <assert.h>
#include <magic.h>
#include <stddef.h>

typedef struct {
  ptrdiff_t count;
  void (*destroy)(void *data);
} Rc;

#define RcBox(T, ...)  \
  struct __VA_ARGS__ { \
    Rc rc;             \
    T data;            \
  }

void *rc__alloc(size_t size, void (*destroy)(void *data));
#define rc_alloc(size, ...) rc__alloc((size), WITH_DEFAULT(NULL, __VA_ARGS__))

void *rc__realloc(const void *data, size_t size);
#define rc_realloc(data, size) (__typeof__(data))rc__realloc((data), (size))

void *rc__manage_copy(const void *data, size_t size, void (*destroy)(void *data));
#define rc_manage_copy(data, size, ...) rc__manage_copy((data), (size), WITH_DEFAULT(NULL, __VA_ARGS__))

char *rc__manage_strndup(const char *data, size_t size, void (*destroy)(void *data));
#define rc_manage_strndup(data, size, ...) rc__manage_strndup((data), (size), WITH_DEFAULT(NULL, __VA_ARGS__))

char *rc__manage_strdup(const char *data, void (*destroy)(void *data));
#define rc_manage_strdup(data, ...) rc__manage_strdup((data), WITH_DEFAULT(NULL, __VA_ARGS__))

void *rc__manage(void *data, size_t size, void (*destroy)(void *data));
#define rc_manage(data, size, ...) (__typeof__(data))rc__manage((data), (size), WITH_DEFAULT(NULL, __VA_ARGS__))

void *rc__acquire(void *data);
#define rc_acquire(data) (__typeof__(data))rc__acquire((data))

void *rc__move(void *data);
#define rc_move(data) (__typeof__(data))rc__move((data))

void rc_release(const void *data);

void rc__release_all(const void *items[], size_t count);
#define rc_release_all(...) rc__release_all((const void *[]){__VA_ARGS__}, sizeof((const void *[]){__VA_ARGS__}) / sizeof(const void *))

ptrdiff_t rc_count(void *data);

typedef struct RC_Circulars {
  void ***items;
  size_t count;
  size_t capacity;
} RC_Circulars;

#define rc_count_circular(count_circular, circulars, data, parent_type) \
  do {                                                                  \
    if (*(data) == (parent_type)) da_append(circulars, (void **)data);  \
    else count_circular(circulars, *(data), (parent_type));             \
  } while (0)

#define rc_release_circular(count_circular, data)         \
  do {                                                    \
    rc_release((data));                                   \
    RC_Circulars circulars = {0};                         \
    count_circular(&circulars, (data), (data));           \
    if (rc_count((data)) == (ptrdiff_t)circulars.count) { \
      da_foreach(void **, link, &circulars) {             \
        **link = NULL;                                    \
        rc_release((data));                               \
      }                                                   \
    }                                                     \
    da_free(circulars);                                   \
  } while (0)

#endif // RC_H

#if defined(RC_IMPL) && !defined(RC_IMPL_C)
#define RC_IMPL_C

void *rc__alloc(size_t size, void (*destroy)(void *data)) {
  Rc *rc = malloc(sizeof(Rc) + size);
  assert(rc);
  rc->count = 0;
  rc->destroy = destroy;
  return rc + 1;
}

void *rc__realloc(const void *data, size_t size) {
  Rc *rc = (Rc *)data - 1;
  assert(rc);
  rc = realloc(rc, sizeof(Rc) + size);
  return rc + 1;
}

void *rc__manage_copy(const void *data, size_t size, void (*destroy)(void *data)) {
  Rc *rc = malloc(sizeof(Rc) + size);
  rc->count = 0;
  rc->destroy = destroy;
  memcpy(rc + 1, data, size);
  return rc + 1;
}

char *rc__manage_strndup(const char *data, size_t size, void (*destroy)(void *data)) {
  char *name = rc__manage_copy(data, (size + 1) * sizeof(char), destroy);
  name[size] = 0;
  return name;
}

char *rc__manage_strdup(const char *data, void (*destroy)(void *data)) {
  return rc__manage_strndup(data, strlen(data), destroy);
}

void *rc__manage(void *data, size_t size, void (*destroy)(void *data)) {
  Rc *rc = rc__manage_copy((void *)data, size, destroy);
  free(data);
  return rc + 1;
}

void *rc__acquire(void *data) {
  Rc *rc = (Rc *)data - 1;
  rc->count += 1;
  return data;
}

void *rc__move(void *data) {
  Rc *rc = (Rc *)data - 1;
  rc->count -= 1;
  return data;
}

void rc_release(const void *data) {
  Rc *rc = (Rc *)data - 1;
  rc->count -= 1;
  if (rc->count <= 0) {
    if (rc->destroy != NULL) rc->destroy(rc + 1);
    free(rc);
  }
}

void rc__release_all(const void *items[], size_t count) {
  for (size_t index = 0; index < count; index += 1) {
    if (items[index]) rc_release(items[index]);
  }
}

ptrdiff_t rc_count(void *data) {
  Rc *rc = (Rc *)data - 1;
  return rc->count;
}

#endif // RC_IMPL
