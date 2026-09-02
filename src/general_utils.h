#ifndef GENERAL_UTILS_H
#define GENERAL_UTILS_H

#include <nob.h>

#define sv_from_like(sv) nob_sv_from_parts((sv).data, (sv).count)
#define sv_like_eq(a, b) ({                                          \
  typeof((a)) ac = (a);                                              \
  typeof((b)) bc = (b);                                              \
  ac.count == bc.count && (memcmp(ac.data, bc.data, ac.count) == 0); \
})

String_Builder sb_copy_from_sv(String_View sv);

#endif // GENERAL_UTILS_H

#define GENERAL_UTILS_IMPL
#if defined(GENERAL_UTILS_IMPL) && !defined(GENERAL_UTILS_IMPL_C)
#define GENERAL_UTILS_IMPL_C

#define NOB_IMPL
#include <nob.h>

String_Builder sb_copy_from_sv(String_View sv) {
  String_Builder sb = {0};
  da_realloc_capacity(&sb, sv.count + 1);
  sb_append_sv(&sb, sv);
  sb_append(&sb, 0);
  return sb;
}

#endif // GENERAL_UTILS_IMPL_C
