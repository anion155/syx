#ifndef SYX_UTILS_H
#define SYX_UTILS_H

#define define_constant(type, name)                  \
  typedef type name##_t;                             \
  typedef struct {                                   \
    size_t initialized;                              \
    name##_t data;                                   \
  } name##_w;                                        \
  name##_w w_##name = {0};                           \
  void make_##name(name##_t *name);                  \
  name##_t *name() {                                 \
    if (w_##name.initialized) return &w_##name.data; \
    w_##name.initialized = 1;                        \
    make_##name(&w_##name.data);                     \
    return &w_##name.data;                           \
  }                                                  \
  void make_##name(name##_t *name)

#endif // SYX_UTILS_H

#define SYX_UTILS_IMPL
#if defined(SYX_UTILS_IMPL) && !defined(SYX_UTILS_IMPL_C)
#define SYX_UTILS_IMPL_C

#endif // SYX_UTILS_IMPL_C
