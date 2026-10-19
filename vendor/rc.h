#ifndef RC_H
#define RC_H

#include <abort.h>
#include <assert.h>
#include <da.h>
#include <defines.h>
#include <stddef.h>

typedef struct Rc_Circulars Rc_Circulars;

typedef struct Rc_Header {
  size_t strong;
  size_t weak;
} Rc_Header;

typedef void (*Rc_Alloc)(void *__ptr, size_t size);
typedef void (*Rc_Realloc)(void *__ptr, size_t size);
typedef void (*Rc_Free)(void *__ptr);
typedef void (*Rc_Method_Destructor)(void *data);
typedef void (*Rc_Method_Graph_Visitor)(Rc_Circulars *circulars, const void *data, const void *source);

typedef struct Rc_Methods {
  Rc_Method_Destructor destructor;
  Rc_Method_Graph_Visitor graph_visitor;
} Rc_Methods;

typedef struct Rc {
  Rc_Header *header;
  Rc_Methods methods;
} Rc;

Rc *rc_get(const void *data);

void *rc__alloc_opt(void *(*alloc)(size_t size), void (*free)(void *data), size_t size, Rc_Methods opt);
#define rc_malloc_opt(size, methods) rc__alloc_opt(malloc, free, (size), (methods))
#define rc_malloc(size, ...) rc_malloc_opt((size), ((Rc_Methods){__VA_ARGS__}))
void *rc__realloc(void *(*realloc)(void *__ptr, size_t __size), const void *data, size_t size);
#define rc_realloc(data, size) ((__typeof__(data))rc__realloc(realloc, (data), (size)))
void *rc__manage_opt(void *(*alloc)(size_t size), void (*free)(void *data), void *data, size_t size, Rc_Methods opt);
#define rc_manage_opt(data, size, methods) ((__typeof__(data))rc__manage_opt(malloc, free, (data), (size), (methods)))
#define rc_manage(data, size, ...) rc_manage_opt((data), (size), ((Rc_Methods){__VA_ARGS__}))

typedef struct Rc_Token {
  void *data;
  void (*destructor)(void *data);
  void (*graph_visitor)(Rc_Circulars *circulars, void *data, const void *source);
} Rc_Token;

Rc_Token *rc__make_token_opt(void *(*alloc)(size_t size), void (*free)(void *data), Rc_Token opt);
#define rc_make_token_opt(opt) rc__make_token_opt(malloc, free, (opt))
#define rc_make_token(...) rc_make_token_opt(((Rc_Token){__VA_ARGS__}))
void rc_release_token(Rc_Token *token);

void *rc__acquire(void *data);
#define rc_acquire(data) ({           \
  typeof(data) _data_rc_ac_ = (data); \
  rc__acquire(_data_rc_ac_);          \
  _data_rc_ac_;                       \
})
void rc__acquire_all(void *items[], size_t count);
#define rc_acquire_all(...) rc__acquire_all((void *[]){__VA_ARGS__}, sizeof((void *[]){__VA_ARGS__}) / sizeof(void *))
void *rc__move(void *data);
#define rc_move(data) ((__typeof__(data))rc__move((data)))
void rc_release(void *data);
void rc__release_all(void *items[], size_t count);
#define rc_release_all(...) rc__release_all((void *[]){__VA_ARGS__}, sizeof((void *[]){__VA_ARGS__}) / sizeof(void *))
#define rc__guarded(items, size) for (bool _run = (rc__acquire_all(items, size), true); _run; (_run = false, rc__release_all(items, size)))
#define rc_guarded(...) rc__guarded((void *[]){__VA_ARGS__}, sizeof((void *[]){__VA_ARGS__}) / sizeof(void *))

void **rc__downgrade(void *(*alloc)(size_t size), void *data);
#define rc_downgrade(data) ((__typeof__(data) *)rc__downgrade(malloc, (data)))
void *rc__upgrade(void (*free)(void *data), void **weak_data);
#define rc_upgrade(weak_data) ((__typeof__(*weak_data))rc__upgrade(free, (weak_data)))
void rc__weak_free(void (*free)(void *data), void **weak_data);
#define rc_weak_free(weak_data) rc__weak_free(free, (weak_data))

size_t rc_count(const void *data);

struct Rc_Circulars {
  void ***data;
  size_t count;
  size_t capacity;
};

void rc__graph_visitor(Rc_Circulars *circulars, Rc *rc, void **data, const void *source);
void rc_graph_visitor(Rc_Circulars *circulars, void **data, const void *source);

#endif // RC_H

#if defined(RC_IMPL) && !defined(RC_IMPL_C)
#define RC_IMPL_C

#define ABORT_IMPL
#include <abort.h>

inline Rc *rc_get(const void *data) {
  return (Rc *)data - 1;
}

void *rc__alloc_opt(void *(*alloc)(size_t size), void (*free)(void *data), size_t size, Rc_Methods opt) {
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
  Rc *rc = rc_get(data);
  rc = realloc(rc, sizeof(Rc) + size);
  assert(rc);
  return rc + 1;
}

void *rc__manage_opt(void *(*alloc)(size_t size), void (*free)(void *data), void *data, size_t size, Rc_Methods opt) {
  Rc *rc = (Rc *)rc__alloc_opt(alloc, free, size, opt) - 1;
  memcpy(rc + 1, data, size);
  if (free) free(data);
  return rc + 1;
}

void rc__token_destructor(void *data) {
  Rc_Token *token = data;
  if (token->destructor) token->destructor(token->data);
}

void rc__token_graph_visitor(Rc_Circulars *circulars, const void *data, const void *source) {
  const Rc_Token *token = data;
  if (token->graph_visitor) token->graph_visitor(circulars, token->data, source);
}

Rc_Token *rc__make_token_opt(void *(*alloc)(size_t size), void (*free)(void *data), Rc_Token opt) {
  Rc_Token *token = rc__alloc_opt(alloc, free, sizeof(Rc_Token), (Rc_Methods){.destructor = rc__token_destructor, .graph_visitor = rc__token_graph_visitor});
  *token = opt;
  return rc__acquire(token);
}

void rc_release_token(Rc_Token *token) {
  Rc *rc = rc_get(token);
  rc->methods.destructor = NULL;
  rc->methods.graph_visitor = NULL;
  rc_release(token);
}

void *rc__acquire(void *data) {
  if (!data) return data;
  Rc *rc = rc_get(data);
  rc->header->strong += 1;
  return data;
}

void rc__acquire_all(void *items[], size_t count) {
  for (size_t index = 0; index < count; index += 1) {
    if (items[index]) rc_acquire(items[index]);
  }
}

void *rc__move(void *data) {
  if (!data) return data;
  Rc *rc = rc_get(data);
  if (!rc->header->strong) UNREACHABLE("trying to move floating memory");
  rc->header->strong -= 1;
  return data;
}

void rc_release(void *data) {
  if (!data) return;
  Rc *rc = rc_get(data);
  Rc_Header *header = rc->header;
  if (!header->strong) UNREACHABLE("trying to either double free memory or release floating memory");
  header->strong -= 1;
  if (!header->strong) {
    if (rc->methods.destructor) rc->methods.destructor(data);
    free(rc);
    if (!header->weak) free(header);
  } else if (rc->methods.graph_visitor) {
    Rc_Circulars circulars = {0};
    // rc->methods.graph_visitor(&circulars, data, data);
    // if (header->strong == circulars.count) {
    //   da_foreach(circulars, link) **link = NULL;
    //   for (size_t index = 0; index < circulars.count; index++) rc_release(data);
    // }
    da_free(&circulars);
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
  *weak = rc_get(data)->header;
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
  Rc *rc = rc_get(data);
  return rc->header->strong;
}

void rc__graph_visitor(Rc_Circulars *circulars, Rc *rc, void **data, const void *source) {
  if (*data == source) {
    da_append(circulars, data);
    return;
  }
  if (rc && rc->methods.graph_visitor) rc->methods.graph_visitor(circulars, *data, source);
}

void rc_graph_visitor(Rc_Circulars *circulars, void **data, const void *source) {
  rc__graph_visitor(circulars, (Rc *)(*data) - 1, data, source);
}

#endif // RC_IMPL
