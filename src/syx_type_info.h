#ifndef SYX_TYPE_INFO_H
#define SYX_TYPE_INFO_H

#include <ht.h>
#include <limits.h>
#include <stddef.h>
#include <stdint.h>

#include "syx_value.h"

typedef enum Syx_Type_Info_Kind {
  SYX_TYPE_INFO_KIND_PTR,
  SYX_TYPE_INFO_KIND_FUNCTION_PTR,
  SYX_TYPE_INFO_KIND_VALUE_PTR,
  SYX_TYPE_INFO_KIND_STRUCTURE,
  SYX_TYPE_INFO_KIND_VOID, // void
  SYX_TYPE_INFO_KIND_CHAR, // char
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
#ifdef __SIZEOF_INT128__
#  define SYX_TYPE_I128_SUPPORTED
#endif
#if defined(__STDC_IEC_60559_TYPES__) || defined(__clang__) && defined(__is_identifier) && !__is_identifier(_Float32)
#  define SYX_TYPE_SIZED_FLOAT_SUPPORTED
#endif

#define SYX_TYPE_VOID void
#define SYX_TYPE_CHAR char
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
typedef struct Syx_Type_Info_Structure Syx_Type_Info_Structure;
typedef struct Syx_Type_Info_Function Syx_Type_Info_Function;

typedef SyxV *(*Syx_Type_Info_Structure_Constructor)(Syx_Eval_Ctx *ctx, void *data, SyxV *arguments);
typedef SyxV *(*Syx_Type_Info_Structure_Index_Getter)(Syx_Eval_Ctx *ctx, void *data, syx_integer_t index);
typedef SyxV *(*Syx_Type_Info_Structure_Index_Setter)(Syx_Eval_Ctx *ctx, void *data, syx_integer_t index, SyxV *argument);
typedef SyxV *(*Syx_Type_Info_Structure_Field_Getter)(Syx_Eval_Ctx *ctx, void *data, const char *field_name);
typedef SyxV *(*Syx_Type_Info_Structure_Field_Setter)(Syx_Eval_Ctx *ctx, void *data, const char *field_name, SyxV *argument);
typedef void (*Syx_Type_Info_Structure_Destructor)(void *data);

typedef enum Syx_Type_Info_Structure_Field_Kind {
  SYX_TYPE_INFO_STRUCTURE_FIELD_KIND_DATA,
  SYX_TYPE_INFO_STRUCTURE_FIELD_KIND_ACCESSOR,
  SYX_TYPE_INFO_STRUCTURE_FIELD_KIND_METHOD,
} Syx_Type_Info_Structure_Field_Kind;

typedef struct Syx_Type_Info_Structure_Data_Field {
  Syx_Type_Info *typeinfo;
  size_t offset;
  bool readonly;
} Syx_Type_Info_Structure_Data_Field;

typedef SyxV *(*Syx_Type_Info_Structure_Accessor_Getter)(Syx_Eval_Ctx *ctx, void *data);
typedef SyxV *(*Syx_Type_Info_Structure_Accessor_Setter)(Syx_Eval_Ctx *ctx, void *data, SyxV *argument);

typedef struct Syx_Type_Info_Structure_Accessor_Field {
  Syx_Type_Info_Structure_Accessor_Getter getter;
  Syx_Type_Info_Structure_Accessor_Setter setter;
} Syx_Type_Info_Structure_Accessor_Field;

typedef SyxV *(*Syx_Type_Info_Structure_Method)(Syx_Eval_Ctx *ctx, void *data, SyxV *arguments);

typedef struct Syx_Type_Info_Structure_Field {
  syx_string_t name;
  Syx_Type_Info_Structure_Field_Kind kind;

  union {
    Syx_Type_Info_Structure_Data_Field data;
    Syx_Type_Info_Structure_Accessor_Field accessor;
    Syx_Type_Info_Structure_Method method;
  };
} Syx_Type_Info_Structure_Field;

typedef Ht(const char *, Syx_Type_Info_Structure_Field, Syx_Type_Info_Structure_Fields) Syx_Type_Info_Structure_Fields;

struct Syx_Type_Info_Structure {
  Syx_Type_Info_Structure_Constructor constructor;
  Syx_Type_Info_Structure_Index_Getter index_getter;
  Syx_Type_Info_Structure_Index_Setter index_setter;
  Syx_Type_Info_Structure_Fields fields;
  Syx_Type_Info_Structure_Field_Getter field_getter;
  Syx_Type_Info_Structure_Field_Setter field_setter;
  Syx_Type_Info_Structure_Destructor destructor;
};

struct Syx_Type_Info_Function {
  size_t size;
  Syx_Type_Info *return_type;
  size_t argc;
  Syx_Type_Info **argv;
  bool vaargs;
};

struct Syx_Type_Info {
  size_t size;
  size_t align;
  SyxV_Symbol *symbol;
  Syx_Type_Info_Kind kind;

  union {
    Syx_Type_Info *ptr;
    Syx_Type_Info_Structure structure;
    Syx_Type_Info_Function function;
  };
};

Syx_Type_Info *make_syx_type_info_opt(Syx_Type_Info opt);
#define make_syx_type_info(...) make_syx_type_info_opt((Syx_Type_Info){__VA_ARGS__})

typedef struct Syx_Field_Pair {
  const char *key;
  Syx_Type_Info_Structure_Field field;
} Syx_Field_Pair;

Syx_Type_Info_Structure_Fields make_syx_type_info_structure_fields_opt(Syx_Field_Pair *pairs, size_t count);
#define make_syx_type_info_structure_fields(...) \
  make_syx_type_info_structure_fields_opt(       \
      (Syx_Field_Pair[]){__VA_ARGS__},           \
      sizeof((Syx_Field_Pair[]){__VA_ARGS__}) / sizeof(Syx_Field_Pair))

size_t stringify_syx_type_info_n(char *string, Syx_Type_Info *typeinfo);
syx_string_t stringify_syx_type_info(Syx_Type_Info *typeinfo);
void sb_append_syx_type_info(String_Builder *sb, Syx_Type_Info *typeinfo);

#endif // SYX_TYPE_INFO_H

#if defined(SYX_TYPE_INFO_IMPL) && !defined(SYX_TYPE_INFO_IMPL_C)
#define SYX_TYPE_INFO_IMPL_C

#define SYX_VALUE_IMPL
#include "syx_value.h"
#define SYX_EVAL_IMPL
#include "syx_eval.h"

const char *syx_type_info_kind_name(Syx_Type_Info_Kind kind) {
  switch (kind) {
    case SYX_TYPE_INFO_KIND_PTR: return "*";
    case SYX_TYPE_INFO_KIND_FUNCTION_PTR: return "fn()";
    case SYX_TYPE_INFO_KIND_VALUE_PTR: return "SyxV*";
    case SYX_TYPE_INFO_KIND_STRUCTURE: return "struct {}";
    case SYX_TYPE_INFO_KIND_VOID: return STRINGIFY2(SYX_TYPE_VOID);
    case SYX_TYPE_INFO_KIND_CHAR: return STRINGIFY2(SYX_TYPE_CHAR);
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
  if (typeinfo->symbol) rc_release(get_syxv_from_symbol(typeinfo->symbol));
  switch (typeinfo->kind) {
    case SYX_TYPE_INFO_KIND_PTR: {
      if (typeinfo->ptr) rc_release(typeinfo->ptr);
    } break;
    case SYX_TYPE_INFO_KIND_FUNCTION_PTR: {
      if (typeinfo->function.return_type) rc_release(typeinfo->function.return_type);
      for (size_t index = 0; index < typeinfo->function.argc; index += 1) {
        Syx_Type_Info *arg = typeinfo->function.argv[index];
        if (arg) rc_release(arg);
      }
    } break;
    case SYX_TYPE_INFO_KIND_STRUCTURE: {
      ht_foreach(field, &typeinfo->structure.fields) {
        switch (field->kind) {
          case SYX_TYPE_INFO_STRUCTURE_FIELD_KIND_DATA: {
            if (field->data.typeinfo) rc_release(field->data.typeinfo);
          } break;
          case SYX_TYPE_INFO_STRUCTURE_FIELD_KIND_ACCESSOR: break;
          case SYX_TYPE_INFO_STRUCTURE_FIELD_KIND_METHOD: break;
        }
        free((char *)ht_key(&typeinfo->structure.fields, field));
      }
      ht_free(&typeinfo->structure.fields);
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
    case SYX_TYPE_INFO_KIND_FUNCTION_PTR: {
      if (typeinfo->function.return_type) rc_graph_visitor(circulars, (void **)&typeinfo->function.return_type, source);
      for (size_t index = 0; index < typeinfo->function.argc; index += 1) {
        if (typeinfo->function.argv[index]) {
          rc_graph_visitor(circulars, (void **)&typeinfo->function.argv[index], source);
        }
      }
    }; break;
    case SYX_TYPE_INFO_KIND_STRUCTURE: {
      ht_foreach(field, &typeinfo->structure.fields) {
        switch (field->kind) {
          case SYX_TYPE_INFO_STRUCTURE_FIELD_KIND_DATA: {
            if (field->data.typeinfo) {
              rc_graph_visitor(circulars, (void **)&field->data.typeinfo, source);
            }
          } break;
          case SYX_TYPE_INFO_STRUCTURE_FIELD_KIND_ACCESSOR: break;
          case SYX_TYPE_INFO_STRUCTURE_FIELD_KIND_METHOD: break;
        }
      }
    } break;
    default:
  }
}

Syx_Type_Info *make_syx_type_info_opt(Syx_Type_Info opt) {
  Syx_Type_Info *typeinfo = rc_malloc(sizeof(Syx_Type_Info), .destructor = syx_type_info_destructor, .graph_visitor = syx_type_info_graph_visitor);
  assert(typeinfo);
  memset(typeinfo, 0, sizeof(Syx_Type_Info));
  *typeinfo = opt;
  if (typeinfo->symbol) rc_acquire(get_syxv_from_symbol(typeinfo->symbol));
  switch (typeinfo->kind) {
    case SYX_TYPE_INFO_KIND_PTR: {
      if (typeinfo->size == 0) typeinfo->size = __SIZEOF_POINTER__;
      if (typeinfo->align == 0) typeinfo->align = _Alignof(void *);
      if (typeinfo->ptr) rc_acquire(typeinfo->ptr);
    } break;
    case SYX_TYPE_INFO_KIND_FUNCTION_PTR: {
      if (typeinfo->size == 0) typeinfo->size = sizeof(void (*)());
      if (typeinfo->align == 0) typeinfo->align = _Alignof(void (*)());
      if (typeinfo->function.return_type) rc_acquire(typeinfo->function.return_type);
      for (size_t index = 0; index < typeinfo->function.argc; index += 1) {
        Syx_Type_Info *arg = typeinfo->function.argv[index];
        if (arg) rc_acquire(arg);
      }
    } break;
    case SYX_TYPE_INFO_KIND_VALUE_PTR: typeinfo->size = __SIZEOF_POINTER__; break;
    case SYX_TYPE_INFO_KIND_STRUCTURE: {
      size_t max_align = 0;
      ht_foreach(field, &typeinfo->structure.fields) {
        switch (field->kind) {
          case SYX_TYPE_INFO_STRUCTURE_FIELD_KIND_DATA: {
            if (field->data.typeinfo) rc_acquire(field->data.typeinfo);
            if (field->data.typeinfo->align > max_align) max_align = field->data.typeinfo->align;
          } break;
          case SYX_TYPE_INFO_STRUCTURE_FIELD_KIND_ACCESSOR: break;
          case SYX_TYPE_INFO_STRUCTURE_FIELD_KIND_METHOD: break;
        }
      }
      if (typeinfo->align == 0) typeinfo->align = max_align;
    } break;
    case SYX_TYPE_INFO_KIND_VOID: return typeinfo;
#define __init_typeinfo(type)    \
  typeinfo->size = sizeof(type); \
  typeinfo->align = _Alignof(type)
    case SYX_TYPE_INFO_KIND_CHAR: __init_typeinfo(SYX_TYPE_CHAR); break;
    case SYX_TYPE_INFO_KIND_I8: __init_typeinfo(SYX_TYPE_I8); break;
    case SYX_TYPE_INFO_KIND_I16: __init_typeinfo(SYX_TYPE_I16); break;
    case SYX_TYPE_INFO_KIND_I32: __init_typeinfo(SYX_TYPE_I32); break;
    case SYX_TYPE_INFO_KIND_I64: __init_typeinfo(SYX_TYPE_I64); break;
#ifdef SYX_TYPE_I128_SUPPORTED
    case SYX_TYPE_INFO_KIND_I128: __init_typeinfo(SYX_TYPE_I128); break;
#endif
    case SYX_TYPE_INFO_KIND_U8: __init_typeinfo(SYX_TYPE_U8); break;
    case SYX_TYPE_INFO_KIND_U16: __init_typeinfo(SYX_TYPE_U16); break;
    case SYX_TYPE_INFO_KIND_U32: __init_typeinfo(SYX_TYPE_U32); break;
    case SYX_TYPE_INFO_KIND_U64: __init_typeinfo(SYX_TYPE_U64); break;
#ifdef SYX_TYPE_I128_SUPPORTED
    case SYX_TYPE_INFO_KIND_U128: __init_typeinfo(SYX_TYPE_U128); break;
#endif
    case SYX_TYPE_INFO_KIND_INT: __init_typeinfo(SYX_TYPE_INT); break;
    case SYX_TYPE_INFO_KIND_INT_LONG: __init_typeinfo(SYX_TYPE_INT_LONG); break;
    case SYX_TYPE_INFO_KIND_INT_LONG_LONG: __init_typeinfo(SYX_TYPE_INT_LONG_LONG); break;
    case SYX_TYPE_INFO_KIND_UINT: __init_typeinfo(SYX_TYPE_UINT); break;
    case SYX_TYPE_INFO_KIND_UINT_LONG: __init_typeinfo(SYX_TYPE_UINT_LONG); break;
    case SYX_TYPE_INFO_KIND_UINT_LONG_LONG: __init_typeinfo(SYX_TYPE_UINT_LONG_LONG); break;
#ifdef SYX_TYPE_SIZED_FLOAT_SUPPORTED
    case SYX_TYPE_INFO_KIND_F16: __init_typeinfo(SYX_TYPE_F16); break;
    case SYX_TYPE_INFO_KIND_F32: __init_typeinfo(SYX_TYPE_F32); break;
    case SYX_TYPE_INFO_KIND_F64: __init_typeinfo(SYX_TYPE_F64); break;
    case SYX_TYPE_INFO_KIND_F128: __init_typeinfo(SYX_TYPE_F128); break;
#endif
    case SYX_TYPE_INFO_KIND_FLOAT: __init_typeinfo(SYX_TYPE_FLOAT); break;
    case SYX_TYPE_INFO_KIND_DOUBLE: __init_typeinfo(SYX_TYPE_DOUBLE); break;
    case SYX_TYPE_INFO_KIND_DOUBLE_LONG: __init_typeinfo(SYX_TYPE_DOUBLE_LONG); break;
    case SYX_TYPE_INFO_KIND_SIZE: __init_typeinfo(SYX_TYPE_SIZE); break;
#undef __init_typeinfo
    default: UNREACHABLE(temp_sprintf("kind is not supported: %u '%s'", typeinfo->kind, syx_type_info_kind_name(typeinfo->kind)));
  }
  if (typeinfo->size == 0) UNREACHABLE("typeinfo.size expected");
  return typeinfo;
}

Syx_Type_Info_Structure_Fields make_syx_type_info_structure_fields_opt(Syx_Field_Pair *pairs, size_t count) {
  Syx_Type_Info_Structure_Fields fields = {0};
  fields.hasheq = ht_cstr_hasheq;
  size_t offset = 0;
  for (size_t index = 0; index < count; index += 1) {
    Syx_Type_Info_Structure_Field field = pairs[index].field;
    if (field.kind == SYX_TYPE_INFO_STRUCTURE_FIELD_KIND_DATA) {
      if (!field.data.offset) {
        size_t align = field.data.typeinfo->align - 1;
        offset = (offset + align) & ~(align);
        field.data.offset = offset;
      }
      if (field.data.typeinfo) {
        offset = field.data.offset + field.data.typeinfo->size;
      }
    }
    size_t length = strlen(pairs[index].key);
    char *key = strndup(pairs[index].key, length);
    field.name = (syx_string_t){.items = key, .count = length + 1, .capacity = length + 1};
    *ht_put(&fields, key) = field;
  }
  return fields;
}

size_t stringify_syx_type_info_n(char *string, Syx_Type_Info *typeinfo) {
  __str_it();
  switch (typeinfo->kind) {
    case SYX_TYPE_INFO_KIND_PTR: {
      __str_push_cstr("#.ref(");
      if (typeinfo->symbol) __str_convert(stringify_syxv_symbol_n, typeinfo->symbol);
      else __str_convert(stringify_syx_type_info_n, typeinfo->ptr);
      __str_push(')');
    } break;
    case SYX_TYPE_INFO_KIND_FUNCTION_PTR: {
      __str_push_cstr("#.fn(");
      if (typeinfo->symbol) {
        __str_convert(stringify_syxv_symbol_n, typeinfo->symbol);
      } else {
        if (typeinfo->function.return_type) __str_convert(stringify_syx_type_info_n, typeinfo->function.return_type);
        else __str_push_cstr("<unknown>");
        __str_push('(');
        for (size_t index = 0; index < typeinfo->function.argc; index += 1) {
          if (index != 0) __str_push_cstr(", ");
          Syx_Type_Info *arg = typeinfo->function.argv[index];
          if (arg) __str_convert(stringify_syx_type_info_n, arg);
          else __str_push_cstr("<unknown>");
        }
        __str_push(')');
      };
      __str_push(')');
    } break;
    case SYX_TYPE_INFO_KIND_VALUE_PTR: {
      __str_push_cstr("#.syxv");
    } break;
    case SYX_TYPE_INFO_KIND_STRUCTURE: {
      __str_push_cstr("#.");
      if (typeinfo->symbol) __str_convert(stringify_syxv_symbol_n, typeinfo->symbol);
      else __str_push_cstr("<anonim>");
    } break;
    case SYX_TYPE_INFO_KIND_VOID: __str_push_cstr("#.c_void"); break;
    case SYX_TYPE_INFO_KIND_CHAR: __str_push_cstr("#.c_char"); break;
    case SYX_TYPE_INFO_KIND_I8: __str_push_cstr("#.c_i8"); break;
    case SYX_TYPE_INFO_KIND_I16: __str_push_cstr("#.c_i16"); break;
    case SYX_TYPE_INFO_KIND_I32: __str_push_cstr("#.c_i32"); break;
    case SYX_TYPE_INFO_KIND_I64: __str_push_cstr("#.c_i64"); break;
#ifdef SYX_TYPE_I128_SUPPORTED
    case SYX_TYPE_INFO_KIND_I128: __str_push_cstr("#.c_i128"); break;
#endif
    case SYX_TYPE_INFO_KIND_U8: __str_push_cstr("#.c_u8"); break;
    case SYX_TYPE_INFO_KIND_U16: __str_push_cstr("#.c_u16"); break;
    case SYX_TYPE_INFO_KIND_U32: __str_push_cstr("#.c_u32"); break;
    case SYX_TYPE_INFO_KIND_U64: __str_push_cstr("#.c_u64"); break;
#ifdef SYX_TYPE_I128_SUPPORTED
    case SYX_TYPE_INFO_KIND_U128: __str_push_cstr("#.c_u128"); break;
#endif
    case SYX_TYPE_INFO_KIND_INT: __str_push_cstr("#.c_int"); break;
    case SYX_TYPE_INFO_KIND_INT_LONG: __str_push_cstr("#.c_int_long"); break;
    case SYX_TYPE_INFO_KIND_INT_LONG_LONG: __str_push_cstr("#.c_int_long_long"); break;
    case SYX_TYPE_INFO_KIND_UINT: __str_push_cstr("#.c_uint"); break;
    case SYX_TYPE_INFO_KIND_UINT_LONG: __str_push_cstr("#.c_uint_long"); break;
    case SYX_TYPE_INFO_KIND_UINT_LONG_LONG: __str_push_cstr("#.c_uint_long_long"); break;
#ifdef SYX_TYPE_SIZED_FLOAT_SUPPORTED
    case SYX_TYPE_INFO_KIND_F16: __str_push_cstr("#.c_f16"); break;
    case SYX_TYPE_INFO_KIND_F32: __str_push_cstr("#.c_f32"); break;
    case SYX_TYPE_INFO_KIND_F64: __str_push_cstr("#.c_f64"); break;
    case SYX_TYPE_INFO_KIND_F128: __str_push_cstr("#.c_f128"); break;
#endif
    case SYX_TYPE_INFO_KIND_FLOAT: __str_push_cstr("#.c_float"); break;
    case SYX_TYPE_INFO_KIND_DOUBLE: __str_push_cstr("#.c_double"); break;
    case SYX_TYPE_INFO_KIND_DOUBLE_LONG: __str_push_cstr("#.c_double_long"); break;
    case SYX_TYPE_INFO_KIND_SIZE: __str_push_cstr("#.c_size"); break;
    default: UNREACHABLE(temp_sprintf("kind is not supported: %u '%s'", typeinfo->kind, syx_type_info_kind_name(typeinfo->kind)));
  }
  return __str_width();
}

syx_string_t stringify_syx_type_info(Syx_Type_Info *typeinfo) {
  __stringify_body(stringify_syx_type_info_n, 256, typeinfo);
}

void sb_append_syx_type_info(String_Builder *sb, Syx_Type_Info *typeinfo) {
  __sb_append_body(stringify_syx_type_info_n, 256, typeinfo);
}

#endif // SYX_TYPE_INFO_IMPL
