#ifndef SYX_TYPE_INFO_H
#define SYX_TYPE_INFO_H

#include <limits.h>
#include <stddef.h>
#include <stdint.h>

typedef enum Syx_Type_Info_Kind {
  SYX_TYPE_INFO_KIND_PTR,
  SYX_TYPE_INFO_KIND_STRUCTURE,
  SYX_TYPE_INFO_KIND_FUNCTION,
  SYX_TYPE_INFO_KIND_VOID, // void
  // stdint
  SYX_TYPE_INFO_KIND_I8,   // int8_t
  SYX_TYPE_INFO_KIND_I16,  // int16_t
  SYX_TYPE_INFO_KIND_I32,  // int32_t
  SYX_TYPE_INFO_KIND_I64,  // int64_t
  SYX_TYPE_INFO_KIND_I128, // int128_t
  SYX_TYPE_INFO_KIND_U8,   // uint8_t
  SYX_TYPE_INFO_KIND_U16,  // uint16_t
  SYX_TYPE_INFO_KIND_U32,  // uint32_t
  SYX_TYPE_INFO_KIND_U64,  // uint64_t
  SYX_TYPE_INFO_KIND_U128, // uint128_t
  // int
  SYX_TYPE_INFO_KIND_INT,
  SYX_TYPE_INFO_KIND_INT_LONG,
  SYX_TYPE_INFO_KIND_INT_LONG_LONG,
  SYX_TYPE_INFO_KIND_UINT,
  SYX_TYPE_INFO_KIND_UINT_LONG,
  SYX_TYPE_INFO_KIND_UINT_LONG_LONG,
  // stdfloat
  SYX_TYPE_INFO_KIND_F16,  // _Float16
  SYX_TYPE_INFO_KIND_F32,  // _Float32
  SYX_TYPE_INFO_KIND_F64,  // _Float64
  SYX_TYPE_INFO_KIND_F128, // _Float128
  // float
  SYX_TYPE_INFO_KIND_FLOAT,
  SYX_TYPE_INFO_KIND_DOUBLE,
  SYX_TYPE_INFO_KIND_DOUBLE_LONG,
  // stddef
  SYX_TYPE_INFO_KIND_SIZE, // size_t
} Syx_Type_Info_Kind;

#define SYX_TYPE_VOID void
#define SYX_TYPE_I8 int8_t
#define SYX_TYPE_I16 int16_t
#define SYX_TYPE_I32 int32_t
#define SYX_TYPE_I64 int64_t
#define SYX_TYPE_I128 __int128
#define SYX_TYPE_U8 uint8_t
#define SYX_TYPE_U16 uint16_t
#define SYX_TYPE_U32 uint32_t
#define SYX_TYPE_U64 uint64_t
#define SYX_TYPE_U128 unsigned __int128
#define SYX_TYPE_INT int
#define SYX_TYPE_INT_LONG signed long
#define SYX_TYPE_INT_LONG_LONG signed long long
#define SYX_TYPE_UINT unsigned
#define SYX_TYPE_UINT_LONG unsigned long
#define SYX_TYPE_UINT_LONG_LONG unsigned long long
#define SYX_TYPE_F16 _Float16
#define SYX_TYPE_F32 _Float32
#define SYX_TYPE_F64 _Float64
#define SYX_TYPE_F128 _Float128
#define SYX_TYPE_FLOAT float
#define SYX_TYPE_DOUBLE double
#define SYX_TYPE_DOUBLE_LONG long double
#define SYX_TYPE_SIZE size_t
const char *syx_type_info_kind_name(Syx_Type_Info_Kind kind);

typedef struct Syx_Type_Info Syx_Type_Info;
typedef struct Syx_Structure_Type_Info Syx_Structure_Type_Info;
typedef struct Syx_Function_Type_Info Syx_Function_Type_Info;

struct Syx_Type_Info {
  size_t size;
  Syx_Type_Info_Kind kind;

  union {
    Syx_Type_Info *ptr;
    Syx_Structure_Type_Info *structure;
    Syx_Function_Type_Info *function;
  };
};

Syx_Type_Info *make_syx_type_info_opt(Syx_Type_Info opt);
#define make_syx_type_info(...) make_syx_type_info_opt((Syx_Type_Info){__VA_ARGS__})

Syx_Structure_Type_Info *make_syx_structure_type_info_opt(Syx_Structure_Type_Info opt);
#define make_syx_structure_type_info(...) make_syx_structure_type_info_opt((Syx_Structure_Type_Info){__VA_ARGS__})

Syx_Function_Type_Info *make_syx_function_type_info_opt(Syx_Function_Type_Info opt);
#define make_syx_function_type_info(...) make_syx_function_type_info_opt((Syx_Function_Type_Info){__VA_ARGS__})

#endif // SYX_TYPE_INFO_H

#if defined(SYX_TYPE_INFO_IMPL) && !defined(SYX_TYPE_INFO_IMPL_C)
#define SYX_TYPE_INFO_IMPL_C

const char *syx_type_info_kind_name(Syx_Type_Info_Kind kind) {
  switch (kind) {
    case SYX_TYPE_INFO_KIND_PTR: return "*";
    case SYX_TYPE_INFO_KIND_STRUCTURE: return "struct {}";
    case SYX_TYPE_INFO_KIND_FUNCTION: return "fn()";
    case SYX_TYPE_INFO_KIND_VOID: return STRINGIFY2(SYX_TYPE_VOID);
    case SYX_TYPE_INFO_KIND_I8: return STRINGIFY2(SYX_TYPE_I8);
    case SYX_TYPE_INFO_KIND_I16: return STRINGIFY2(SYX_TYPE_I16);
    case SYX_TYPE_INFO_KIND_I32: return STRINGIFY2(SYX_TYPE_I32);
    case SYX_TYPE_INFO_KIND_I64: return STRINGIFY2(SYX_TYPE_I64);
    case SYX_TYPE_INFO_KIND_I128: return STRINGIFY2(SYX_TYPE_I128);
    case SYX_TYPE_INFO_KIND_U8: return STRINGIFY2(SYX_TYPE_U8);
    case SYX_TYPE_INFO_KIND_U16: return STRINGIFY2(SYX_TYPE_U16);
    case SYX_TYPE_INFO_KIND_U32: return STRINGIFY2(SYX_TYPE_U32);
    case SYX_TYPE_INFO_KIND_U64: return STRINGIFY2(SYX_TYPE_U64);
    case SYX_TYPE_INFO_KIND_U128: return STRINGIFY2(SYX_TYPE_U128);
    case SYX_TYPE_INFO_KIND_INT: return STRINGIFY2(SYX_TYPE_INT);
    case SYX_TYPE_INFO_KIND_INT_LONG: return STRINGIFY2(SYX_TYPE_INT_LONG);
    case SYX_TYPE_INFO_KIND_INT_LONG_LONG: return STRINGIFY2(SYX_TYPE_INT_LONG_LONG);
    case SYX_TYPE_INFO_KIND_UINT: return STRINGIFY2(SYX_TYPE_UINT);
    case SYX_TYPE_INFO_KIND_UINT_LONG: return STRINGIFY2(SYX_TYPE_UINT_LONG);
    case SYX_TYPE_INFO_KIND_UINT_LONG_LONG: return STRINGIFY2(SYX_TYPE_UINT_LONG_LONG);
    case SYX_TYPE_INFO_KIND_F16: return STRINGIFY2(SYX_TYPE_F16);
    case SYX_TYPE_INFO_KIND_F32: return STRINGIFY2(SYX_TYPE_F32);
    case SYX_TYPE_INFO_KIND_F64: return STRINGIFY2(SYX_TYPE_F64);
    case SYX_TYPE_INFO_KIND_F128: return STRINGIFY2(SYX_TYPE_F128);
    case SYX_TYPE_INFO_KIND_FLOAT: return STRINGIFY2(SYX_TYPE_FLOAT);
    case SYX_TYPE_INFO_KIND_DOUBLE: return STRINGIFY2(SYX_TYPE_DOUBLE);
    case SYX_TYPE_INFO_KIND_DOUBLE_LONG: return STRINGIFY2(SYX_TYPE_DOUBLE_LONG);
    case SYX_TYPE_INFO_KIND_SIZE: return STRINGIFY2(SYX_TYPE_SIZE);
  }
}

void syx_type_info_destructor(void *data) {
  Syx_Type_Info *typeinfo = data;
  switch (typeinfo->kind) {
    case SYX_TYPE_INFO_KIND_PTR: {
      if (typeinfo->ptr) rc_release(typeinfo->ptr);
    } break;
    case SYX_TYPE_INFO_KIND_STRUCTURE: {
      if (typeinfo->structure) rc_release(typeinfo->structure);
    } break;
    case SYX_TYPE_INFO_KIND_FUNCTION: {
      if (typeinfo->function) rc_release(typeinfo->function);
    } break;
    default:
  }
}

void syx_type_info_graph_visitor(Rc_Circulars *circulars, const void *data, const void *source) {
  const Syx_Type_Info *typeinfo = data;
  switch (typeinfo->kind) {
    case SYX_TYPE_INFO_KIND_PTR: {
      rc_graph_visitor(circulars, (void **)&typeinfo->ptr, source);
    } break;
    case SYX_TYPE_INFO_KIND_STRUCTURE: {
      rc_graph_visitor(circulars, (void **)&typeinfo->structure, source);
    } break;
    case SYX_TYPE_INFO_KIND_FUNCTION: {
      rc_graph_visitor(circulars, (void **)&typeinfo->function, source);
    }; break;
    default:
  }
}

Syx_Type_Info *make_syx_type_info_opt(Syx_Type_Info opt) {
  Syx_Type_Info *info = rc_malloc(sizeof(Syx_Type_Info), .destructor = syx_type_info_destructor, .graph_visitor = syx_type_info_graph_visitor);
  memset(info, 0, sizeof(Syx_Type_Info));
  *info = opt;
  switch (info->kind) {
    case SYX_TYPE_INFO_KIND_PTR: {
      if (info->size == 0) {
        switch (info->ptr->kind) {
          case SYX_TYPE_INFO_KIND_FUNCTION: info->size = info->ptr->size; break;
          default: info->size = __SIZEOF_POINTER__;
        }
      }
      if (info->ptr) rc_acquire(info->ptr);
    } break;
    case SYX_TYPE_INFO_KIND_STRUCTURE: {
      if (info->size == 0) info->size = info->structure->size;
      if (info->structure) rc_acquire(info->structure);
    } break;
    case SYX_TYPE_INFO_KIND_FUNCTION: {
      if (info->size == 0) info->size = sizeof(void (*)());
      if (info->function) rc_acquire(info->function);
    } break;
    case SYX_TYPE_INFO_KIND_VOID: break;
    case SYX_TYPE_INFO_KIND_I8: info->size = 8; break;
    case SYX_TYPE_INFO_KIND_I16: info->size = 16; break;
    case SYX_TYPE_INFO_KIND_I32: info->size = 32; break;
    case SYX_TYPE_INFO_KIND_I64: info->size = 64; break;
    case SYX_TYPE_INFO_KIND_I128: info->size = 128; break;
    case SYX_TYPE_INFO_KIND_U8: info->size = 8; break;
    case SYX_TYPE_INFO_KIND_U16: info->size = 16; break;
    case SYX_TYPE_INFO_KIND_U32: info->size = 32; break;
    case SYX_TYPE_INFO_KIND_U64: info->size = 64; break;
    case SYX_TYPE_INFO_KIND_U128: info->size = 128; break;
    case SYX_TYPE_INFO_KIND_INT: info->size = __SIZEOF_INT__; break;
    case SYX_TYPE_INFO_KIND_INT_LONG: info->size = __SIZEOF_LONG__; break;
    case SYX_TYPE_INFO_KIND_INT_LONG_LONG: info->size = __SIZEOF_LONG_LONG__; break;
    case SYX_TYPE_INFO_KIND_UINT: info->size = __SIZEOF_INT__; break;
    case SYX_TYPE_INFO_KIND_UINT_LONG: info->size = __SIZEOF_LONG__; break;
    case SYX_TYPE_INFO_KIND_UINT_LONG_LONG: info->size = __SIZEOF_LONG_LONG__; break;
    case SYX_TYPE_INFO_KIND_F16: info->size = 16; break;
    case SYX_TYPE_INFO_KIND_F32: info->size = 32; break;
    case SYX_TYPE_INFO_KIND_F64: info->size = 64; break;
    case SYX_TYPE_INFO_KIND_F128: info->size = 128; break;
    case SYX_TYPE_INFO_KIND_FLOAT: info->size = __SIZEOF_FLOAT__; break;
    case SYX_TYPE_INFO_KIND_DOUBLE: info->size = __SIZEOF_DOUBLE__; break;
    case SYX_TYPE_INFO_KIND_DOUBLE_LONG: info->size = __SIZEOF_LONG_DOUBLE__; break;
    case SYX_TYPE_INFO_KIND_SIZE: info->size = __SIZEOF_SIZE_T__; break;
  }
  return info;
}

#endif // SYX_TYPE_INFO_IMPL
