#ifndef RC_H
#define RC_H

#include <assert.h>
#include <magic.h>
#include <stddef.h>

typedef struct Rc_Circulars Rc_Circulars;

typedef struct Rc_Header {
  size_t strong;
  size_t weak;
} Rc_Header;

typedef struct Rc_Methods {
  void (*destructor)(void *data);
  void (*graph_visitor)(Rc_Circulars *circulars, const void *data, const void *source);
} Rc_Methods;

typedef struct Rc {
  Rc_Header *header;
  Rc_Methods methods;
} Rc;

void *rc__alloc(void *(*alloc)(size_t size), void (*free)(void *data), size_t size, Rc_Methods opt);
#define rc_malloc(size, ...) rc__alloc(malloc, free, (size), (Rc_Methods){__VA_ARGS__})
void *rc__realloc(void *(*realloc)(void *__ptr, size_t __size), const void *data, size_t size);
#define rc_realloc(data, size, ...) (__typeof__(data))rc__realloc(realloc, (data), (size))
void *rc__manage(void *(*alloc)(size_t size), void (*free)(void *data), void *data, size_t size, Rc_Methods opt);
#define rc_manage(data, size, ...) (__typeof__(data))rc__manage(malloc, free, (data), (size), (Rc_Methods){__VA_ARGS__})

void *rc__acquire(void *data);
#define rc_acquire(data) (__typeof__(data))rc__acquire((data))
void *rc__move(void *data);
#define rc_move(data) (__typeof__(data))rc__move((data))
void rc_release(void *data);
void rc__release_all(void *items[], size_t count);
#define rc_release_all(...) rc__release_all((void *[]){__VA_ARGS__}, sizeof((void *[]){__VA_ARGS__}) / sizeof(void *))

void **rc__downgrade(void *(*alloc)(size_t size), void *data);
#define rc_downgrade(data) (__typeof__(data) *)rc__downgrade(malloc, (data))
void *rc__upgrade(void (*free)(void *data), void **weak_data);
#define rc_upgrade(weak_data) (__typeof__(*weak_data))rc__upgrade(free, (weak_data))
void rc__weak_free(void (*free)(void *data), void **weak_data);
#define rc_weak_free(weak_data) rc__weak_free(free, (weak_data))

size_t rc_count(const void *data);

struct Rc_Circulars {
  void ***items;
  size_t count;
  size_t capacity;
};

void rc_graph_visitor(Rc_Circulars *circulars, void **data, const void *source);

#endif // RC_H

#if defined(RC_IMPL) && !defined(RC_IMPL_C)
#define RC_IMPL_C

void *rc__alloc(void *(*alloc)(size_t size), void (*free)(void *data), size_t size, Rc_Methods opt) {
  Rc *rc = alloc(sizeof(Rc) + size);
  if (!rc) return NULL;
  rc->header = alloc(sizeof(Rc_Header));
  if (!rc->header) return (free(rc), NULL);
  rc->header->strong = 0;
  rc->header->weak = 0;
  rc->methods = opt;
  return rc + 1;
}

void *rc__realloc(void *(*realloc)(void *__ptr, size_t __size), const void *data, size_t size) {
  Rc *rc = (Rc *)data - 1;
  rc = realloc(rc, sizeof(Rc) + size);
  assert(rc);
  return rc + 1;
}

void *rc__manage(void *(*alloc)(size_t size), void (*free)(void *data), void *data, size_t size, Rc_Methods opt) {
  Rc *rc = (Rc *)rc__alloc(alloc, free, size, opt) - 1;
  memcpy(rc + 1, data, size);
  free(data);
  return rc + 1;
}

void *rc__acquire(void *data) {
  Rc *rc = (Rc *)data - 1;
  rc->header->strong += 1;
  return data;
}

void *rc__move(void *data) {
  Rc *rc = (Rc *)data - 1;
  if (!rc->header->strong) UNREACHABLE("trying to move floating memory");
  rc->header->strong -= 1;
  return data;
}

void rc_release(void *data) {
  Rc *rc = (Rc *)data - 1;
  Rc_Header *header = rc->header;
  if (!header->strong) UNREACHABLE("trying to either double free memory or release floating memory");
  header->strong -= 1;
  if (!header->strong) {
    if (rc->methods.destructor) rc->methods.destructor(data);
    free(rc);
    if (!header->weak) free(header);
  } else if (rc->methods.graph_visitor) {
    Rc_Circulars circulars = {0};
    rc->methods.graph_visitor(&circulars, data, data);
    if (header->strong == circulars.count) {
      da_foreach(void **, link, &circulars) **link = NULL;
      for (size_t index = 0; index < circulars.count; index++) rc_release(data);
    }
    da_free(circulars);
  }
}

void rc__release_all(void *items[], size_t count) {
  for (size_t index = 0; index < count; index += 1) {
    if (items[index]) rc_release(items[index]);
  }
}

void **rc__downgrade(void *(*alloc)(size_t size), void *data) {
  Rc_Header **weak = alloc(sizeof(Rc_Header *) + sizeof(void *));
  assert(weak);
  *weak = ((Rc *)data - 1)->header;
  (*weak)->weak += 1;
  *((void **)(weak + 1)) = data;
  return (void **)(weak + 1);
}

void *rc__upgrade(void (*free)(void *data), void **weak_data) {
  Rc_Header **weak = ((Rc_Header **)weak_data - 1);
  Rc_Header *header = *weak;
  void *data = *weak_data;
  if (!header->weak) UNREACHABLE("trying to double free weak pointer");
  free(weak);
  header->weak -= 1;
  if (!header->weak && !header->strong) return (free(header), NULL);
  if (!header->strong) return NULL;
  return rc_acquire(data);
}

void rc__weak_free(void (*free)(void *data), void **weak_data) {
  Rc_Header **weak = ((Rc_Header **)weak_data - 1);
  Rc_Header *header = *weak;
  if (!header->weak) UNREACHABLE("trying to double free weak pointer");
  free(weak);
  header->weak -= 1;
  if (!header->weak && !header->strong) free(header);
}

size_t rc_count(const void *data) {
  Rc *rc = (Rc *)data - 1;
  return rc->header->strong;
}

void rc_graph_visitor(Rc_Circulars *circulars, void **data, const void *source) {
  if (*data == source) {
    da_append(circulars, data);
    return;
  }
  Rc *rc = (Rc *)(*data) - 1;
  if (rc->methods.graph_visitor) rc->methods.graph_visitor(circulars, *data, source);
}

#endif // RC_IMPL
