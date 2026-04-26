#ifndef RC_H
#define RC_H

#include <stddef.h>
#include <magic.h>
#include <assert.h>

typedef struct {
    ptrdiff_t count;
    void (*destroy)(void *data);
} Rc;

#define RcBox(T, ...) \
struct __VA_ARGS__ {  \
    Rc rc;            \
    T data;           \
}

void *rc__alloc(size_t size, void (*destroy)(void *data));
#define rc_alloc(size, ...) rc__alloc((size), WITH_DEFAULT(NULL, __VA_ARGS__))

void *rc__realloc(const void *data, size_t size);
#define rc_realloc(data, size) (__typeof__(data))rc__realloc((data), (size))

void *rc__manage_copy(const void *data, size_t size, void (*destroy)(void *data));
#define rc_manage_copy(data, size, ...) rc__manage_copy((data), (size), WITH_DEFAULT(NULL, __VA_ARGS__))

void *rc__manage(void *data, size_t size, void (*destroy)(void *data));
#define rc_manage(data, size, ...) (__typeof__(data))rc__manage((data), (size), WITH_DEFAULT(NULL, __VA_ARGS__))

void *rc__acquire(void *data);
#define rc_acquire(data) (__typeof__(data))rc__acquire((data))

void rc_release(void *data);

ptrdiff_t rc_count(void *data);

#endif // RC_H

#if defined(RC_IMPL) && !defined(RC_IMPL_C)
#define RC_IMPL_C

void *rc__alloc(size_t size, void (*destroy)(void *data))
{
    Rc *rc = malloc(sizeof(Rc) + size);
    assert(rc);
    rc->count = 0;
    rc->destroy = destroy;
    return rc + 1;
}

void *rc__realloc(const void *data, size_t size)
{
    Rc *rc = (Rc*)data - 1;
    assert(rc);
    rc = realloc(rc, sizeof(Rc) + size);
    return rc + 1;
}

void *rc__manage_copy(const void *data, size_t size, void (*destroy)(void *data))
{
    Rc *rc = malloc(sizeof(Rc) + size);
    rc->count = 0;
    rc->destroy = destroy;
    memcpy(rc + 1, data, size);
    return rc + 1;
}

void *rc__manage(void *data, size_t size, void (*destroy)(void *data))
{
    Rc *rc = rc__manage_copy((void *)data, size, destroy);
    free(data);
    return rc + 1;
}

void *rc__acquire(void *data)
{
    Rc *rc = (Rc*)data - 1;
    rc->count += 1;
    return data;
}

void rc_release(void *data)
{
    Rc *rc = (Rc*)data - 1;
    rc->count -= 1;
    if (rc->count <= 0) {
        if (rc->destroy != NULL) rc->destroy(rc + 1);
        free(rc);
    }
}

ptrdiff_t rc_count(void *data)
{
    Rc *rc = (Rc*)data - 1;
    return rc->count;
}

#endif // RC_IMPL
