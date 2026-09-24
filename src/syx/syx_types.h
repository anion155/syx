#ifndef SYX_TYPES_H
#define SYX_TYPES_H

#include <defines.h>
#include <ffi/ffi.h>
#include <float.h>
#include <ht.h>
#include <limits.h>
#include <stdalign.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <syx/syx_value.h>

typedef enum Syx_Type_Kind : unsigned int {
  SYX_TYPE_KIND_VOID,
  SYX_TYPE_KIND_PRIMITIVE,
  SYX_TYPE_KIND_STRUCTURE,
  SYX_TYPE_KIND_PTR,
  SYX_TYPE_KIND_FUNCTION_PTR,
} Syx_Type_Kind;

typedef enum Syx_Primitive_Type_Kind : unsigned int {
  SYX_PRIMITIVE_TYPE_KIND_CHAR,    // char
  SYX_PRIMITIVE_TYPE_KIND_I8,      // int8_t
  SYX_PRIMITIVE_TYPE_KIND_I16,     // int16_t
  SYX_PRIMITIVE_TYPE_KIND_I32,     // int32_t
  SYX_PRIMITIVE_TYPE_KIND_I64,     // int64_t
  SYX_PRIMITIVE_TYPE_KIND_I128,    // __int128_t
  SYX_PRIMITIVE_TYPE_KIND_U8,      // uint8_t
  SYX_PRIMITIVE_TYPE_KIND_U16,     // uint16_t
  SYX_PRIMITIVE_TYPE_KIND_U32,     // uint32_t
  SYX_PRIMITIVE_TYPE_KIND_U64,     // uint64_t
  SYX_PRIMITIVE_TYPE_KIND_U128,    // __uint128_t
  SYX_PRIMITIVE_TYPE_KIND_F16,     // f16_t
  SYX_PRIMITIVE_TYPE_KIND_F32,     // f32_t
  SYX_PRIMITIVE_TYPE_KIND_F64,     // f64_t
  SYX_PRIMITIVE_TYPE_KIND_F80,     // f80_t
  SYX_PRIMITIVE_TYPE_KIND_F128,    // f128_t
  SYX_PRIMITIVE_TYPE_KIND_F64PAIR, // f64pair_t
} Syx_Primitive_Type_Kind;

#if SHRT_MAX == 32767
#  define SYX_PRIMITIVE_TYPE_KIND_SHORT SYX_PRIMITIVE_TYPE_KIND_I16
#  define SYX_PRIMITIVE_TYPE_KIND_SSHORT SYX_PRIMITIVE_TYPE_KIND_I16
#  define SYX_PRIMITIVE_TYPE_KIND_USHORT SYX_PRIMITIVE_TYPE_KIND_U16
#elif SHRT_MAX == 2147483647
#  define SYX_PRIMITIVE_TYPE_KIND_SHORT SYX_PRIMITIVE_TYPE_KIND_I32
#  define SYX_PRIMITIVE_TYPE_KIND_SSHORT SYX_PRIMITIVE_TYPE_KIND_I32
#  define SYX_PRIMITIVE_TYPE_KIND_USHORT SYX_PRIMITIVE_TYPE_KIND_U32
#else
#  error "short size not supported"
#endif

#if INT_MAX == 32767
#  define SYX_PRIMITIVE_TYPE_KIND_INT SYX_PRIMITIVE_TYPE_KIND_I16
#  define SYX_PRIMITIVE_TYPE_KIND_SINT SYX_PRIMITIVE_TYPE_KIND_I16
#  define SYX_PRIMITIVE_TYPE_KIND_UINT SYX_PRIMITIVE_TYPE_KIND_U16
#elif INT_MAX == 2147483647
#  define SYX_PRIMITIVE_TYPE_KIND_INT SYX_PRIMITIVE_TYPE_KIND_I32
#  define SYX_PRIMITIVE_TYPE_KIND_SINT SYX_PRIMITIVE_TYPE_KIND_I32
#  define SYX_PRIMITIVE_TYPE_KIND_UINT SYX_PRIMITIVE_TYPE_KIND_U32
#elif INT_MAX == 9223372036854775807
#  define SYX_PRIMITIVE_TYPE_KIND_INT SYX_PRIMITIVE_TYPE_KIND_I64
#  define SYX_PRIMITIVE_TYPE_KIND_SINT SYX_PRIMITIVE_TYPE_KIND_I64
#  define SYX_PRIMITIVE_TYPE_KIND_UINT SYX_PRIMITIVE_TYPE_KIND_U64
#else
#  error "int size not supported"
#endif

#if LONG_MAX == 2147483647
#  define SYX_PRIMITIVE_TYPE_KIND_LONG SYX_PRIMITIVE_TYPE_KIND_I32
#  define SYX_PRIMITIVE_TYPE_KIND_SLONG SYX_PRIMITIVE_TYPE_KIND_I32
#  define SYX_PRIMITIVE_TYPE_KIND_ULONG SYX_PRIMITIVE_TYPE_KIND_U32
#elif LONG_MAX == 9223372036854775807
#  define SYX_PRIMITIVE_TYPE_KIND_LONG SYX_PRIMITIVE_TYPE_KIND_I64
#  define SYX_PRIMITIVE_TYPE_KIND_SLONG SYX_PRIMITIVE_TYPE_KIND_I64
#  define SYX_PRIMITIVE_TYPE_KIND_ULONG SYX_PRIMITIVE_TYPE_KIND_U64
#else
#  error "long size not supported"
#endif

#if LLONG_MAX == 9223372036854775807
#  define SYX_PRIMITIVE_TYPE_KIND_LLONG SYX_PRIMITIVE_TYPE_KIND_I64
#  define SYX_PRIMITIVE_TYPE_KIND_SLLONG SYX_PRIMITIVE_TYPE_KIND_I64
#  define SYX_PRIMITIVE_TYPE_KIND_ULLONG SYX_PRIMITIVE_TYPE_KIND_U64
#else
#  error "long size not supported"
#endif

#if UINTPTR_MAX == 65535
#  define SYX_PRIMITIVE_TYPE_KIND_UINTPTR SYX_PRIMITIVE_TYPE_KIND_U16
#elif UINTPTR_MAX == 4294967295U
#  define SYX_PRIMITIVE_TYPE_KIND_UINTPTR SYX_PRIMITIVE_TYPE_KIND_U32
#elif UINTPTR_MAX == 18446744073709551615ULL
#  define SYX_PRIMITIVE_TYPE_KIND_UINTPTR SYX_PRIMITIVE_TYPE_KIND_U64
#else
#  error "uintptr_t size not supported"
#endif

#if PTRDIFF_MAX == 32767
#  define SYX_PRIMITIVE_TYPE_KIND_PTRDIFF SYX_PRIMITIVE_TYPE_KIND_I16
#elif PTRDIFF_MAX == 2147483647
#  define SYX_PRIMITIVE_TYPE_KIND_PTRDIFF SYX_PRIMITIVE_TYPE_KIND_I32
#elif PTRDIFF_MAX == 9223372036854775807
#  define SYX_PRIMITIVE_TYPE_KIND_PTRDIFF SYX_PRIMITIVE_TYPE_KIND_I64
#else
#  error "ptrdiff_t size not supported"
#endif

#if SIZE_MAX == 65535
#  define SYX_PRIMITIVE_TYPE_KIND_SIZE SYX_PRIMITIVE_TYPE_KIND_U16
#elif SIZE_MAX == 4294967295U
#  define SYX_PRIMITIVE_TYPE_KIND_SIZE SYX_PRIMITIVE_TYPE_KIND_U32
#elif SIZE_MAX == 18446744073709551615ULL
#  define SYX_PRIMITIVE_TYPE_KIND_SIZE SYX_PRIMITIVE_TYPE_KIND_U64
#else
#  error "size_t size not supported"
#endif

#define SYX_PRIMITIVE_TYPE_KIND_FLOAT SYX_PRIMITIVE_TYPE_KIND_F32
#define SYX_PRIMITIVE_TYPE_KIND_DOUBLE SYX_PRIMITIVE_TYPE_KIND_F64
#if FLOATS_LD_KIND == FLOATS_LD_KIND_F64
#  define SYX_PRIMITIVE_TYPE_KIND_LDOUBLE SYX_PRIMITIVE_TYPE_KIND_F64
#elif FLOATS_LD_KIND == FLOATS_LD_KIND_F80
#  define SYX_PRIMITIVE_TYPE_KIND_LDOUBLE SYX_PRIMITIVE_TYPE_KIND_F80
#elif FLOATS_LD_KIND == FLOATS_LD_KIND_F128
#  define SYX_PRIMITIVE_TYPE_KIND_LDOUBLE SYX_PRIMITIVE_TYPE_KIND_F128
#elif FLOATS_LD_KIND == FLOATS_LD_KIND_F64PAIR
#  define SYX_PRIMITIVE_TYPE_KIND_LDOUBLE SYX_PRIMITIVE_TYPE_KIND_F64PAIR
#else
#  error "Unsupported or unknown long double architecture."
#endif

typedef struct Syx_Type_Structure Syx_Type_Structure;
typedef struct Syx_Type_Function Syx_Type_Function;
typedef struct Syx_Type Syx_Type;

typedef struct Syx_Type {
  ffi_type *ffi_t;
  Syx_Type_Kind kind;
  size_t size;
  size_t alignment;
  Syx_Symbol *name;

  union {
    Syx_Primitive_Type_Kind primitive;
    Syx_Type *pointer;
    Syx_Type_Structure *structure;
    Syx_Type_Function *function;
  };
} Syx_Type;
typedef Da(Syx_Type *, Syx_Types_Array) Syx_Types_Array;
typedef Da_Slice(Syx_Type *, Syx_Types) Syx_Types;

void syx_type_rename(Syx_Type *type, Syx_Symbol *name);

typedef Syx_Value *(*Syx_Type_Structure_Constructor)(Syx_Eval_Ctx *ctx, void *data, Syx_Pair *arguments);
// typedef Syx_Value *(*Syx_Type_Structure_Index_Getter)(Syx_Eval_Ctx *ctx, void *data, syx_integer_t index);
// typedef Syx_Value *(*Syx_Type_Structure_Index_Setter)(Syx_Eval_Ctx *ctx, void *data, syx_integer_t index, Syx_Value *argument);
// typedef Syx_Value *(*Syx_Type_Structure_Field_Getter)(Syx_Eval_Ctx *ctx, void *data, const char *field_name);
// typedef Syx_Value *(*Syx_Type_Structure_Field_Setter)(Syx_Eval_Ctx *ctx, void *data, const char *field_name, Syx_Value *argument);
typedef void (*Syx_Type_Structure_Destructor)(void *data);

typedef struct Syx_Type_Structure_Field {
  Syx_Symbol *name;
  Syx_Type *type;
  size_t offset;
  bool readonly;
} Syx_Type_Structure_Field;

typedef Da_Slice(Syx_Type_Structure_Field, Syx_Type_Structure_Fields) Syx_Type_Structure_Fields;

typedef struct Syx_Type_Structure {
  Syx_Type_Structure_Constructor constructor;
  Syx_Type_Structure_Fields fields;
  Syx_Type_Structure_Destructor destructor;
} Syx_Type_Structure;

typedef struct Syx_Type_Function {
  ffi_cif *ffi_f;
  ffi_type **ffi_args;
  Syx_Type *return_type;
  Syx_Types args_types;
  bool vaargs;
} Syx_Type_Function;

Syx_Type *make_syx_type_pointer(Syx_Symbol *name, Syx_Type *target);
Syx_Type *make_syx_type_structure(Syx_Symbol *name, Syx_Type_Structure structure);
Syx_Type_Structure_Fields make__syx_type_structure_fields(const Syx_Type_Structure_Field *items, size_t count);
#define make_syx_type_structure_fields(...) ({                                                \
  Syx_Type_Structure_Field fields[] = {__VA_ARGS__};                                          \
  make__syx_type_structure_fields(fields, sizeof(fields) / sizeof(Syx_Type_Structure_Field)); \
})
Syx_Type *make_syx_type_function(Syx_Symbol *name, Syx_Type_Function func);

Syx_Type_Structure_Field *syx_native_structure_get_field(Syx_Type_Structure *structure, Syx_Symbol *field_symbol);

Syx_Value *syx_eval_native_structure(Syx_Eval_Ctx *ctx, Syx_Native *native, Syx_Type_Structure *structure_type, Syx_Pair *arguments);
Syx_Value *syx_eval_native_pointer(Syx_Eval_Ctx *ctx, Syx_Native *native, Syx_Type *stored_type, Syx_Pair *arguments);
Syx_Value *syx_eval_native_function(Syx_Eval_Ctx *ctx, Syx_Native *native, Syx_Type_Function *function, Syx_Pair *arguments);

size_t sb_append_syx_type(String_Builder *sb, const Syx_Type *type);

typedef struct {
  Syx_Type *c_void;
  Syx_Type *c_char;
  Syx_Type *c_i8;
  Syx_Type *c_i16;
  Syx_Type *c_i32;
  Syx_Type *c_i64;
  Syx_Type *c_i128;
  Syx_Type *c_u8;
  Syx_Type *c_u16;
  Syx_Type *c_u32;
  Syx_Type *c_u64;
  Syx_Type *c_u128;
  Syx_Type *c_f16;
  Syx_Type *c_f32;
  Syx_Type *c_f64;
  Syx_Type *c_f80;
  Syx_Type *c_f128;
  Syx_Type *c_f64pair;
  Syx_Type *c_short;
  Syx_Type *c_sshort;
  Syx_Type *c_ushort;
  Syx_Type *c_int;
  Syx_Type *c_sint;
  Syx_Type *c_uint;
  Syx_Type *c_long;
  Syx_Type *c_slong;
  Syx_Type *c_ulong;
  Syx_Type *c_llong;
  Syx_Type *c_sllong;
  Syx_Type *c_ullong;
  Syx_Type *c_uintptr;
  Syx_Type *c_ptrdiff;
  Syx_Type *c_size;
  Syx_Type *c_float;
  Syx_Type *c_double;
  Syx_Type *c_ldouble;
  Syx_Type *c_value;
  Syx_Type *c_str;
  Syx_Type *c_string;
  Syx_Type *c_file;
} SYX_KNOWN_TYPES_t;
syx_predefine_constant(SYX_KNOWN_TYPES_t, SYX_KNOWN_TYPES);
void syx_env_define_types(Syx_Env *env);

Syx_Value *syx__list_next_type(Syx_Eval_Ctx *ctx, Syx_Pair **list, Syx_Type **type);
#define syx_list_next_type(ctx, list, ...) syx_get_non_value_with_early_exit(Syx_Type, syx__list_next_type, ((ctx), (list)), WITH_DEFAULT((), __VA_ARGS__))
Syx_Value *syx__eval_type_expression(Syx_Eval_Ctx *ctx, Syx_Pair **expression, Syx_Type **type);
#define syx_eval_type_expression(ctx, expression, ...) syx_get_non_value_with_early_exit(Syx_Type, syx__eval_type_expression, ((ctx), (expression)), WITH_DEFAULT((), __VA_ARGS__))

#endif // SYX_TYPES_H

#if defined(SYX_TYPES_IMPL) && !defined(SYX_TYPES_IMPL_C)
#define SYX_TYPES_IMPL_C

#include <str.h>
#define HT_IMPL
#include <ht.h>
#define SYX_VALUE_IMPL
#include <syx/syx_value.h>
#define SYX_EVAL_IMPL
#include <syx/syx_eval.h>

void syx_type_destructor(void *data) {
  Syx_Type *type = data;
  rc_release(syx_value_from_symbol(type->name));
}

Syx_Type *make_syx_type(Syx_Type_Kind kind, size_t size, size_t alignment, Syx_Symbol *name, ffi_type *ffi_t, size_t additional_size) {
  Syx_Type *type = rc_malloc(sizeof(Syx_Type) + additional_size);
  assert(type);
  rc_get(type)->methods.destructor = syx_type_destructor;
  type->kind = kind;
  type->size = size;
  type->alignment = alignment;
  rc_acquire(syx_value_from_symbol(name));
  type->name = name;
  type->ffi_t = ffi_t;
  return type;
}

Syx_Type *make_syx_type_embed_types(Syx_Type_Kind kind, size_t size, size_t alignment, Syx_Symbol *name, ffi_type ffi_t, size_t additional_size) {
  Da(ffi_type) ffi_types = {0};
  da_append(&ffi_types, ffi_t);
  Da_Slice(ffi_type) queue = da_slice_init(ffi_types);
  while (queue.count) {
    ffi_type current = da_slice_shift(&queue);
    if (current.type != FFI_TYPE_STRUCT) continue;
    ffi_type **element = current.elements;
    while (*element != NULL) {
      queue.count += da_append(&ffi_types, **element);
      element += 1;
    }
  }
  Syx_Type *type = make_syx_type(kind, size, alignment, name, NULL, sizeof(ffi_type) * ffi_types.count + additional_size);
  type->ffi_t = (ffi_type *)(type + 1);
  memcpy(type->ffi_t, ffi_types.data, sizeof(ffi_type) * ffi_types.count);
  return type;
}

Syx_Type *make_syx_type_primitive(Syx_Primitive_Type_Kind kind, size_t size, size_t alignment, Syx_Symbol *name, ffi_type *ffi_t) {
  Syx_Type *type = make_syx_type(SYX_TYPE_KIND_PRIMITIVE, size, alignment, name, ffi_t, 0);
  type->primitive = kind;
  return type;
}

Syx_Type *make_syx_type_primitive_embed_types(Syx_Primitive_Type_Kind kind, size_t size, size_t alignment, Syx_Symbol *name, ffi_type ffi_t) {
  Syx_Type *type = make_syx_type_embed_types(SYX_TYPE_KIND_PRIMITIVE, size, alignment, name, ffi_t, 0);
  type->primitive = kind;
  return type;
}

void syx_type_rename(Syx_Type *type, Syx_Symbol *name) {
  rc_acquire(syx_value_from_symbol(name));
  rc_release(syx_value_from_symbol(type->name));
  type->name = name;
}

void syx_type_pointer_destructor(void *data) {
  syx_type_destructor(data);
  Syx_Type *type = data;
  rc_release(type->pointer);
}

void syx_type_pointer_graph_visitor(Rc_Circulars *circulars, const void *data, const void *source) {
  const Syx_Type *type = data;
  if (type->name) rc_graph_visitor(circulars, (void **)&(type->name), source);
}

Syx_Type *make_syx_type_pointer(Syx_Symbol *name, Syx_Type *target) {
  Syx_Type *type = make_syx_type(SYX_TYPE_KIND_PTR, sizeof(void *), alignof(void *), name, &ffi_type_pointer, 0);
  rc_get(type)->methods = (Rc_Methods){.destructor = syx_type_pointer_destructor, .graph_visitor = syx_type_pointer_graph_visitor};
  type->pointer = rc_acquire(target);
  return type;
}

void syx_type_structure_destructor(void *data) {
  syx_type_destructor(data);
  Syx_Type_Structure *structure = ((Syx_Type *)data)->structure;
  da_foreach(structure->fields, field) {
    rc_release(syx_value_from_symbol(field->name));
    rc_release(field->type);
  }
}

void syx_type_structure_graph_visitor(Rc_Circulars *circulars, const void *data, const void *source) {
  Syx_Type_Structure *structure = ((const Syx_Type *)data)->structure;
  da_foreach(structure->fields, field) {
    rc_graph_visitor(circulars, (void **)&(field->type), source);
  }
}

Syx_Type *make_syx_type_structure(Syx_Symbol *name, Syx_Type_Structure structure) {
  ffi_type ffi_t = {.type = FFI_TYPE_STRUCT};
  ffi_type *ffi_elements[structure.fields.count + 1];
  for (size_t index = 0; index < structure.fields.count; index += 1) {
    ffi_elements[index] = structure.fields.data[index].type->ffi_t;
  }
  ffi_elements[structure.fields.count] = NULL;
  ffi_t.elements = ffi_elements;
  Syx_Type *type = make_syx_type_embed_types(SYX_TYPE_KIND_STRUCTURE, 0, 0, name, ffi_t, sizeof(Syx_Type_Structure) + sizeof(Syx_Type_Structure_Field) * structure.fields.count);
  rc_get(type)->methods = (Rc_Methods){.destructor = syx_type_structure_destructor, .graph_visitor = syx_type_structure_graph_visitor};
  type->structure = (Syx_Type_Structure *)(type + 1);
  *type->structure = structure;
  Da(Syx_Type_Structure_Field) fields = {.data = (Syx_Type_Structure_Field *)(type->structure + 1), .capacity = structure.fields.count, .count = 0};
  size_t offset = 0, max_alignment = 1;
  da_foreach(structure.fields, field) {
    if (field->type->alignment > max_alignment) max_alignment = field->type->alignment;
    if (field->type->alignment > 0) offset = (offset + field->type->alignment - 1) & ~(field->type->alignment - 1);
    if (field->offset == 0) field->offset = offset;
    offset += field->type->size;
    rc_acquire(syx_value_from_symbol(field->name));
    rc_acquire(field->type);
    da_append(&fields, *field);
  }
  free((Syx_Type_Structure_Field *)structure.fields.data);
  type->structure->fields = da_slice(fields, Syx_Type_Structure_Fields);
  type->size = (offset + max_alignment - 1) & ~(max_alignment - 1);
  type->alignment = type->alignment;
  return type;
}

Syx_Type_Structure_Fields make__syx_type_structure_fields(const Syx_Type_Structure_Field *items, size_t count) {
  Da(Syx_Type_Structure_Field) fields = {0};
  da_reserve_exact(&fields, count);
  da_append_many_n(&fields, items, count);
  return da_slice(fields, Syx_Type_Structure_Fields);
}

void syx_type_function_destructor(void *data) {
  syx_type_destructor(data);
  Syx_Type_Function *func = ((Syx_Type *)data)->function;
  rc_release(func->return_type);
  da_foreach(func->args_types, arg_type) rc_release(*arg_type);
}

void syx_type_function_graph_visitor(Rc_Circulars *circulars, const void *data, const void *source) {
  Syx_Type_Function *func = ((const Syx_Type *)data)->function;
  rc_graph_visitor(circulars, (void **)&(func->return_type), source);
  da_foreach(func->args_types, arg_type) rc_graph_visitor(circulars, (void **)arg_type, source);
}

Syx_Type *make_syx_type_function(Syx_Symbol *name, Syx_Type_Function func) {
  Syx_Type *type = make_syx_type(SYX_TYPE_KIND_FUNCTION_PTR, sizeof(void (*)(void)), alignof(void (*)(void)), name, NULL, sizeof(Syx_Type_Function) + sizeof(ffi_cif) + sizeof(Syx_Type *) * func.args_types.count + sizeof(ffi_type *) * func.args_types.count);
  rc_get(type)->methods = (Rc_Methods){.destructor = syx_type_function_destructor, .graph_visitor = syx_type_function_graph_visitor};
  type->function = (Syx_Type_Function *)(type + 1);
  type->function->return_type = rc_acquire(func.return_type);
  type->function->ffi_f = (ffi_cif *)(type->function + 1);
  Syx_Types_Array args_types = {.data = (Syx_Type **)(type->function->ffi_f + 1), .capacity = func.args_types.count, .count = 0};
  memcpy(args_types.data, func.args_types.data, sizeof(Syx_Type *) * func.args_types.count);
  args_types.count = func.args_types.count;
  da_foreach(args_types, arg_type) rc_acquire(*arg_type);
  type->function->args_types = da_slice(args_types, Syx_Types);
  type->function->ffi_args = (ffi_type **)(args_types.data + 1);
  da_foreach(args_types, arg_type) type->function->ffi_args[arg_type_index] = (*arg_type)->ffi_t;
  type->function->ffi_f = NULL;
  type->function->vaargs = func.vaargs;
  return type;
}

Syx_Type_Structure_Field *syx_native_structure_get_field(Syx_Type_Structure *structure, Syx_Symbol *field_symbol) {
  size_t field_index = da_find_expr(structure->fields, field, field->name == field_symbol);
  if (field_index >= structure->fields.count) return NULL;
  return &structure->fields.data[field_index];
}

Syx_Value *syx_eval_native_structure(Syx_Eval_Ctx *ctx, Syx_Native *native, Syx_Type_Structure *structure_type, Syx_Pair *arguments) {
  Syx_Value *argument = syx_list_next_nullable(&arguments);
  SYX_EVAL_ASSERT(ctx, argument->kind == SYX_VALUE_KIND_PREFIXED, "native structure evaluation expects prefixed field name");
  SYX_EVAL_ASSERT(ctx, argument->prefixed->kind == SYX_PREFIXED_KIND_COLON, "native structure evaluation expects prefixed field name");
  SYX_EVAL_ASSERT(ctx, argument->prefixed->value->kind == SYX_VALUE_KIND_SYMBOL, "native structure evaluation expects prefixed field name");
  Syx_Symbol *field_name = argument->prefixed->value->symbol;
  Syx_Type_Structure_Field *field = syx_native_structure_get_field(structure_type, field_name);
  SYX_EVAL_ASSERT(ctx, field, "native structure has no such field");
  Syx_Value *value = rc_acquire(make_syx_value_native(native, field->type, (void *)((char *)native->data + field->offset), 0));
  if (!arguments) return rc_move(value);
  Syx_Value *result = rc_acquire(syx_eval_pair(ctx, value, arguments));
  rc_release(value);
  return rc_move(result);
}

Syx_Value *syx_eval_native_pointer(Syx_Eval_Ctx *ctx, Syx_Native *native, Syx_Type *stored_type, Syx_Pair *arguments) {
  Syx_Value *argument = syx_list_next_nullable(&arguments);
  SYX_EVAL_ASSERT(ctx, argument->kind == SYX_VALUE_KIND_SYMBOL, "native pointer evaluation expects symbol argument");
  Syx_Value *asterisk_symbol = rc_acquire(syx_value_symbol_strlit("*"));
  Syx_Value *unref_symbol = rc_acquire(syx_value_symbol_strlit("unref"));
  Syx_Value *value = NULL;
  if (argument == asterisk_symbol || argument == unref_symbol) {
    if (stored_type == SYX_KNOWN_TYPES()->c_value) {
      value = *(Syx_Value **)native->data;
    } else {
      value = rc_acquire(make_syx_value_native(native, stored_type, *(void **)native->data, 0));
    }
  } else {
    SYX_EVAL_THROW(ctx, "unknown method call: '" SV_FMT "'", (sv_fmt_arg(*argument->symbol)), (asterisk_symbol, unref_symbol));
  }
  if (!arguments) return rc_move(value);
  Syx_Value *result = rc_acquire(syx_eval_pair(ctx, value, arguments));
  rc_release_all(value, asterisk_symbol, unref_symbol);
  return result;
}

Syx_Value *syx_eval_native_function(Syx_Eval_Ctx *ctx, Syx_Native *native, Syx_Type_Function *function, Syx_Pair *arguments) {
  if (!function->ffi_f) {
    ffi_cif *ffi_f = (ffi_cif *)(function + 1);
    if (ffi_prep_cif(ffi_f, FFI_DEFAULT_ABI, function->args_types.count, function->return_type->ffi_t, function->ffi_args) != FFI_OK) {
      SYX_EVAL_THROW(ctx, "invalid native function descriptor");
    }
    function->ffi_f = ffi_f;
  }
  void *args_storage[function->args_types.count];
  Syx_Value *evaluated = rc_acquire(syx_eval_map_list(ctx, arguments));
  syx_value_early_exit(evaluated);
  Syx_Pair *evaluated_pair = evaluated->pair;
  da_foreach(function->args_types, arg_type) {
    Syx_Value *argument = syx_list_next(&evaluated_pair);
    if (argument->kind != SYX_VALUE_KIND_NATIVE) SYX_EVAL_TODO(ctx, "TASK(20260913-075748): convert arguments to native values");
    if ((*arg_type) != argument->native->type) SYX_EVAL_TODO(ctx, "TASK(20260913-075748): convert arguments to native values");
    args_storage[arg_type_index] = argument->native->data;
  }
  if (evaluated_pair) SYX_EVAL_TODO(ctx, "TASK(20260913-075819): vaargs support", (), (evaluated));
  Syx_Value *result = rc_acquire(make_syx_value_native_instance(function->return_type));
  memset(result->native->data, 0, function->return_type->size);
  ffi_call(function->ffi_f, *(void (**)(void))native->data, result->native->data, args_storage);
  rc_release(evaluated);
  return rc_move(result);
}

size_t sb_append_syx_type(String_Builder *sb, const Syx_Type *type) {
  Stringify_State state = make_stringify_state(sb, 256);
  switch (type->kind) {
    case SYX_TYPE_KIND_VOID:
    case SYX_TYPE_KIND_PRIMITIVE: {
      if (!type->name) UNREACHABLE("primitive types must have name");
      stringify_append(&state, sb_append_syx_symbol, type->name);
    } break;
    case SYX_TYPE_KIND_STRUCTURE: {
      stringify_append(&state, sb_append_strlit, "c_struct");
      stringify_append(&state, sb_append, '(');
      stringify_append(&state, sb_append, ')');
    } break;
    case SYX_TYPE_KIND_PTR: {
      stringify_append(&state, sb_append_strlit, "c_ref");
      stringify_append(&state, sb_append, '(');
      stringify_append(&state, sb_append_syx_type, type->pointer);
      stringify_append(&state, sb_append, ')');
    } break;
    case SYX_TYPE_KIND_FUNCTION_PTR: {
      stringify_append(&state, sb_append_strlit, "c_fn");
      Syx_Type_Function *func = type->function;
      stringify_append(&state, sb_append, '(');
      stringify_append(&state, sb_append_syx_type, func->return_type);
      stringify_append(&state, sb_append, '(');
      for (size_t index = 0; index < func->args_types.count; index += 1) {
        if (index != 0) stringify_append(&state, sb_append_strlit, ", ");
        const Syx_Type *arg = func->args_types.data[index];
        stringify_append(&state, sb_append_syx_type, arg);
      }
      if (type->function->vaargs) {
        if (func->args_types.count) stringify_append(&state, sb_append_strlit, ", ");
        stringify_append(&state, sb_append_strlit, "...");
      }
      stringify_append(&state, sb_append, ')');
      stringify_append(&state, sb_append, ')');
    } break;
  }
  return state.count;
}

syx_define_constant(SYX_KNOWN_TYPES_t, SYX_KNOWN_TYPES) {
  SYX_KNOWN_TYPES->c_void = rc_acquire(make_syx_type(SYX_TYPE_KIND_VOID, sizeof(void), alignof(void), NULL, &ffi_type_void, 0));

  SYX_KNOWN_TYPES->c_char = make_syx_type_primitive(SYX_PRIMITIVE_TYPE_KIND_CHAR, sizeof(char), alignof(char), NULL, CHAR_MIN < 0 ? &ffi_type_schar : &ffi_type_uchar);
  SYX_KNOWN_TYPES->c_i8 = make_syx_type_primitive(SYX_PRIMITIVE_TYPE_KIND_I8, sizeof(int8_t), alignof(int8_t), NULL, &ffi_type_sint8);
  SYX_KNOWN_TYPES->c_i16 = make_syx_type_primitive(SYX_PRIMITIVE_TYPE_KIND_I16, sizeof(int16_t), alignof(int16_t), NULL, &ffi_type_sint16);
  SYX_KNOWN_TYPES->c_i32 = make_syx_type_primitive(SYX_PRIMITIVE_TYPE_KIND_I32, sizeof(int32_t), alignof(int32_t), NULL, &ffi_type_sint32);
  SYX_KNOWN_TYPES->c_i64 = make_syx_type_primitive(SYX_PRIMITIVE_TYPE_KIND_I64, sizeof(int64_t), alignof(int64_t), NULL, &ffi_type_sint64);
  SYX_KNOWN_TYPES->c_i128 = make_syx_type_primitive_embed_types(SYX_PRIMITIVE_TYPE_KIND_I128, sizeof(__int128_t), alignof(__int128_t), NULL, (ffi_type){.type = FFI_TYPE_STRUCT, .elements = (ffi_type *[]){&ffi_type_sint64, &ffi_type_sint64, NULL}});
  SYX_KNOWN_TYPES->c_u8 = make_syx_type_primitive(SYX_PRIMITIVE_TYPE_KIND_U8, sizeof(uint8_t), alignof(uint8_t), NULL, &ffi_type_uint8);
  SYX_KNOWN_TYPES->c_u16 = make_syx_type_primitive(SYX_PRIMITIVE_TYPE_KIND_U16, sizeof(uint16_t), alignof(uint16_t), NULL, &ffi_type_uint16);
  SYX_KNOWN_TYPES->c_u32 = make_syx_type_primitive(SYX_PRIMITIVE_TYPE_KIND_U32, sizeof(uint32_t), alignof(uint32_t), NULL, &ffi_type_uint32);
  SYX_KNOWN_TYPES->c_u64 = make_syx_type_primitive(SYX_PRIMITIVE_TYPE_KIND_U64, sizeof(uint64_t), alignof(uint64_t), NULL, &ffi_type_uint64);
  SYX_KNOWN_TYPES->c_u128 = make_syx_type_primitive_embed_types(SYX_PRIMITIVE_TYPE_KIND_U128, sizeof(__uint128_t), alignof(__uint128_t), NULL, (ffi_type){.type = FFI_TYPE_STRUCT, .elements = (ffi_type *[]){&ffi_type_uint64, &ffi_type_uint64, NULL}});
  SYX_KNOWN_TYPES->c_f16 = make_syx_type_primitive_embed_types(SYX_PRIMITIVE_TYPE_KIND_F16, sizeof(f16_canonical_t), alignof(f16_canonical_t), NULL, (ffi_type){.type = FFI_TYPE_STRUCT, .elements = (ffi_type *[]){&ffi_type_uint16, NULL}});
  SYX_KNOWN_TYPES->c_f32 = make_syx_type_primitive(SYX_PRIMITIVE_TYPE_KIND_F32, sizeof(float), alignof(float), NULL, &ffi_type_float);
  SYX_KNOWN_TYPES->c_f64 = make_syx_type_primitive(SYX_PRIMITIVE_TYPE_KIND_F64, sizeof(double), alignof(double), NULL, &ffi_type_double);
  SYX_KNOWN_TYPES->c_f80 = make_syx_type_primitive_embed_types(SYX_PRIMITIVE_TYPE_KIND_F80, sizeof(f80_canonical_t), alignof(f80_canonical_t), NULL, (ffi_type){.type = FFI_TYPE_STRUCT, .elements = (ffi_type *[]){&ffi_type_uint64, &ffi_type_uint16, NULL}});
  SYX_KNOWN_TYPES->c_f128 = make_syx_type_primitive_embed_types(SYX_PRIMITIVE_TYPE_KIND_F128, sizeof(f128_canonical_t), alignof(f128_canonical_t), NULL, (ffi_type){.type = FFI_TYPE_STRUCT, .elements = (ffi_type *[]){&ffi_type_uint64, &ffi_type_uint64, NULL}});
  SYX_KNOWN_TYPES->c_f64pair = make_syx_type_primitive_embed_types(SYX_PRIMITIVE_TYPE_KIND_F64PAIR, sizeof(f64pair_canonical_t), alignof(f64pair_canonical_t), NULL, (ffi_type){.type = FFI_TYPE_STRUCT, .elements = (ffi_type *[]){&ffi_type_uint64, &ffi_type_uint64, NULL}});

#if LLONG_MAX == 9223372036854775807
  ffi_type *ffi_type_sllong_ref = &ffi_type_sint64;
  ffi_type *ffi_type_ullong_ref = &ffi_type_uint64;
  ffi_type *ffi_type_llong_ref = &ffi_type_sint64;
#else
#  error "long size not supported"
#endif

#if UINTPTR_MAX == 65535
  ffi_type *ffi_type_uintptr_ref = &ffi_type_uint16;
#elif UINTPTR_MAX == 4294967295U
  ffi_type *ffi_type_uintptr_ref = &ffi_type_uint32;
#elif UINTPTR_MAX == 18446744073709551615ULL
  ffi_type *ffi_type_uintptr_ref = &ffi_type_uint64;
#else
#  error "uintptr_t size not supported"
#endif

#if PTRDIFF_MAX == 32767
  ffi_type *ffi_type_ptrdiff_ref = &ffi_type_sint16;
#elif PTRDIFF_MAX == 2147483647
  ffi_type *ffi_type_ptrdiff_ref = &ffi_type_sint32;
#elif PTRDIFF_MAX == 9223372036854775807
  ffi_type *ffi_type_ptrdiff_ref = &ffi_type_sint64;
#else
#  error "ptrdiff_t size not supported"
#endif

#if SIZE_MAX == 65535
  ffi_type *ffi_type_size_ref = &ffi_type_uint16;
#elif SIZE_MAX == 4294967295U
  ffi_type *ffi_type_size_ref = &ffi_type_uint32;
#elif SIZE_MAX == 18446744073709551615ULL
  ffi_type *ffi_type_size_ref = &ffi_type_uint64;
#else
#  error "size_t size not supported"
#endif

#if FLOATS_LD_KIND == FLOATS_LD_KIND_F64
  ffi_type *ffi_type_ldouble_ref = &ffi_type_double;
#elif FLOATS_LD_KIND == FLOATS_LD_KIND_F80
  ffi_type *ffi_type_ldouble_ref = &ffi_type_f80;
#elif FLOATS_LD_KIND == FLOATS_LD_KIND_F128
  ffi_type *ffi_type_ldouble_ref = &ffi_type_f128;
#elif FLOATS_LD_KIND == FLOATS_LD_KIND_F64PAIR
  ffi_type *ffi_type_ldouble_ref = &ffi_type_f128;
#else
#  error "Unsupported or unknown long double architecture."
#endif

  SYX_KNOWN_TYPES->c_short = make_syx_type_primitive(SYX_PRIMITIVE_TYPE_KIND_SHORT, sizeof(short), alignof(short), NULL, &ffi_type_sshort);
  SYX_KNOWN_TYPES->c_sshort = make_syx_type_primitive(SYX_PRIMITIVE_TYPE_KIND_SSHORT, sizeof(signed short), alignof(signed short), NULL, &ffi_type_sshort);
  SYX_KNOWN_TYPES->c_ushort = make_syx_type_primitive(SYX_PRIMITIVE_TYPE_KIND_USHORT, sizeof(unsigned short), alignof(unsigned short), NULL, &ffi_type_ushort);
  SYX_KNOWN_TYPES->c_int = make_syx_type_primitive(SYX_PRIMITIVE_TYPE_KIND_INT, sizeof(int), alignof(int), NULL, &ffi_type_sint);
  SYX_KNOWN_TYPES->c_sint = make_syx_type_primitive(SYX_PRIMITIVE_TYPE_KIND_SINT, sizeof(signed int), alignof(signed int), NULL, &ffi_type_sint);
  SYX_KNOWN_TYPES->c_uint = make_syx_type_primitive(SYX_PRIMITIVE_TYPE_KIND_UINT, sizeof(unsigned int), alignof(unsigned int), NULL, &ffi_type_uint);
  SYX_KNOWN_TYPES->c_long = make_syx_type_primitive(SYX_PRIMITIVE_TYPE_KIND_LONG, sizeof(long), alignof(long), NULL, &ffi_type_slong);
  SYX_KNOWN_TYPES->c_slong = make_syx_type_primitive(SYX_PRIMITIVE_TYPE_KIND_SLONG, sizeof(signed long), alignof(signed long), NULL, &ffi_type_slong);
  SYX_KNOWN_TYPES->c_ulong = make_syx_type_primitive(SYX_PRIMITIVE_TYPE_KIND_ULONG, sizeof(unsigned long), alignof(unsigned long), NULL, &ffi_type_ulong);
  SYX_KNOWN_TYPES->c_llong = make_syx_type_primitive(SYX_PRIMITIVE_TYPE_KIND_LLONG, sizeof(long long), alignof(long long), NULL, ffi_type_llong_ref);
  SYX_KNOWN_TYPES->c_sllong = make_syx_type_primitive(SYX_PRIMITIVE_TYPE_KIND_SLLONG, sizeof(signed long), alignof(signed long), NULL, ffi_type_sllong_ref);
  SYX_KNOWN_TYPES->c_ullong = make_syx_type_primitive(SYX_PRIMITIVE_TYPE_KIND_ULLONG, sizeof(unsigned long), alignof(unsigned long), NULL, ffi_type_ullong_ref);
  SYX_KNOWN_TYPES->c_uintptr = make_syx_type_primitive(SYX_PRIMITIVE_TYPE_KIND_UINTPTR, sizeof(uintptr_t), alignof(uintptr_t), NULL, ffi_type_uintptr_ref);
  SYX_KNOWN_TYPES->c_ptrdiff = make_syx_type_primitive(SYX_PRIMITIVE_TYPE_KIND_PTRDIFF, sizeof(ptrdiff_t), alignof(ptrdiff_t), NULL, ffi_type_ptrdiff_ref);
  SYX_KNOWN_TYPES->c_size = make_syx_type_primitive(SYX_PRIMITIVE_TYPE_KIND_SIZE, sizeof(size_t), alignof(size_t), NULL, ffi_type_size_ref);
  SYX_KNOWN_TYPES->c_float = make_syx_type_primitive(SYX_PRIMITIVE_TYPE_KIND_FLOAT, sizeof(float), alignof(float), NULL, &ffi_type_float);
  SYX_KNOWN_TYPES->c_double = make_syx_type_primitive(SYX_PRIMITIVE_TYPE_KIND_DOUBLE, sizeof(double), alignof(double), NULL, &ffi_type_double);
  SYX_KNOWN_TYPES->c_ldouble = make_syx_type_primitive(SYX_PRIMITIVE_TYPE_KIND_LDOUBLE, sizeof(long double), alignof(long double), NULL, ffi_type_ldouble_ref);

  SYX_KNOWN_TYPES->c_value = make_syx_type_pointer(NULL, SYX_KNOWN_TYPES->c_void);
  SYX_KNOWN_TYPES->c_str = make_syx_type_pointer(NULL, SYX_KNOWN_TYPES->c_char);
  SYX_KNOWN_TYPES->c_string = make_syx_type_structure(NULL, (Syx_Type_Structure){
                                                                .fields = make_syx_type_structure_fields(
                                                                    (Syx_Type_Structure_Field){.name = syx_value_symbol_strlit("data")->symbol, .readonly = true, .type = SYX_KNOWN_TYPES->c_str},
                                                                    (Syx_Type_Structure_Field){.name = syx_value_symbol_strlit("count")->symbol, .readonly = true, .type = SYX_KNOWN_TYPES->c_size})});
  SYX_KNOWN_TYPES->c_file = make_syx_type_pointer(NULL, SYX_KNOWN_TYPES->c_void);
}

void syx_env_define_types(Syx_Env *env) {
  SYX_KNOWN_TYPES_t *types = SYX_KNOWN_TYPES();
  syx_env_define_type_strlit(env, "c_void", types->c_void);
  syx_env_define_type_strlit(env, "c_char", types->c_char);
  syx_env_define_type_strlit(env, "c_i8", types->c_i8);
  syx_env_define_type_strlit(env, "c_i16", types->c_i16);
  syx_env_define_type_strlit(env, "c_i32", types->c_i32);
  syx_env_define_type_strlit(env, "c_i64", types->c_i64);
  syx_env_define_type_strlit(env, "c_i128", types->c_i128);
  syx_env_define_type_strlit(env, "c_u8", types->c_u8);
  syx_env_define_type_strlit(env, "c_u16", types->c_u16);
  syx_env_define_type_strlit(env, "c_u32", types->c_u32);
  syx_env_define_type_strlit(env, "c_u64", types->c_u64);
  syx_env_define_type_strlit(env, "c_u128", types->c_u128);
  syx_env_define_type_strlit(env, "c_f16", types->c_f16);
  syx_env_define_type_strlit(env, "c_f32", types->c_f32);
  syx_env_define_type_strlit(env, "c_f64", types->c_f64);
  syx_env_define_type_strlit(env, "c_f80", types->c_f80);
  syx_env_define_type_strlit(env, "c_f128", types->c_f128);
  syx_env_define_type_strlit(env, "c_f64pair", types->c_f64pair);
  syx_env_define_type_strlit(env, "c_short", types->c_short);
  syx_env_define_type_strlit(env, "c_sshort", types->c_sshort);
  syx_env_define_type_strlit(env, "c_ushort", types->c_ushort);
  syx_env_define_type_strlit(env, "c_int", types->c_int);
  syx_env_define_type_strlit(env, "c_sint", types->c_sint);
  syx_env_define_type_strlit(env, "c_uint", types->c_uint);
  syx_env_define_type_strlit(env, "c_long", types->c_long);
  syx_env_define_type_strlit(env, "c_slong", types->c_slong);
  syx_env_define_type_strlit(env, "c_ulong", types->c_ulong);
  syx_env_define_type_strlit(env, "c_llong", types->c_llong);
  syx_env_define_type_strlit(env, "c_sllong", types->c_sllong);
  syx_env_define_type_strlit(env, "c_ullong", types->c_ullong);
  syx_env_define_type_strlit(env, "c_uintptr", types->c_uintptr);
  syx_env_define_type_strlit(env, "c_ptrdiff", types->c_ptrdiff);
  syx_env_define_type_strlit(env, "c_size", types->c_size);
  syx_env_define_type_strlit(env, "c_float", types->c_float);
  syx_env_define_type_strlit(env, "c_double", types->c_double);
  syx_env_define_type_strlit(env, "c_ldouble", types->c_ldouble);
  syx_env_define_type_strlit(env, "c_value", types->c_value);
  syx_env_define_type_strlit(env, "c_str", types->c_str);
  syx_env_define_type_strlit(env, "c_string", types->c_string);
  syx_env_define_type_strlit(env, "c_file", types->c_file);
}

Syx_Value *syx__list_next_type(Syx_Eval_Ctx *ctx, Syx_Pair **list, Syx_Type **type) {
  Syx_Value *value = syx_list_next(list);
  if (value->kind == SYX_VALUE_KIND_SYMBOL) {
    *type = syx_env_get_value(ctx, value->symbol);
    SYX_EVAL_ASSERT(ctx, *type, "unknown type '" SV_FMT "'", (sv_fmt_arg(*value->symbol)));
  } else if (value->kind == SYX_VALUE_KIND_PAIR) {
    Syx_Pair *expression = value->pair;
    *type = syx_eval_type_expression(ctx, &expression);
  } else {
    SYX_EVAL_THROW(ctx, "unsupported type expression");
  }
  return NULL;
}
void syx_types_da_descructor(void *data) {
  Syx_Types_Array *types = data;
  da_foreach(*types, type) rc_release(*type);
  da_free(types);
}
Syx_Value *syx__eval_type_expression(Syx_Eval_Ctx *ctx, Syx_Pair **expression, Syx_Type **type) {
  Syx_Value *kind = syx_list_next(expression);
  SYX_EVAL_ASSERT(ctx, kind->kind == SYX_VALUE_KIND_SYMBOL, "expected type kind");
  static Syx_Value *ptr_s = NULL;
  if (!ptr_s) ptr_s = rc_acquire(syx_value_symbol_strlit("ptr"));
  if (kind->symbol == ptr_s->symbol) {
    Syx_Type *target = syx_list_next_type(ctx, expression);
    *type = make_syx_type_pointer(NULL, target);
    return NULL;
  }
  static Syx_Value *struct_s = NULL;
  if (!struct_s) struct_s = rc_acquire(syx_value_symbol_strlit("struct"));
  if (kind->symbol == struct_s->symbol) {
    SYX_EVAL_TODO(ctx);
    // return NULL;
  }
  static Syx_Value *fn_s = NULL;
  if (!fn_s) fn_s = rc_acquire(syx_value_symbol_strlit("fn"));
  if (kind->symbol == fn_s->symbol) {
    Syx_Type *return_type = syx_list_next_type(ctx, expression);
    rc_acquire(return_type);
    Syx_Types_Array *args_types = rc_acquire(rc_malloc(sizeof(Syx_Types_Array)));
    memset(args_types, 0, sizeof(Syx_Types_Array));
    rc_get(args_types)->methods.destructor = syx_types_da_descructor;
    Syx_Value *arg_definition_value = syx_list_next(expression);
    SYX_EVAL_ASSERT(ctx, arg_definition_value->kind == SYX_VALUE_KIND_PAIR, "expected arguments definition", (), (return_type, args_types));
    Syx_Pair *arg_definition = arg_definition_value->pair;
    while (arg_definition) {
      Syx_Type *arg_type = syx_list_next_type(ctx, &arg_definition, (return_type, args_types));
      da_append(args_types, rc_acquire(arg_type));
    }
    *type = make_syx_type_function(NULL, (Syx_Type_Function){.args_types = da_slice(*args_types, Syx_Types), .return_type = rc_move(return_type), .vaargs = false});
    rc_acquire(*type);
    rc_release(args_types);
    rc_move(*type);
    return NULL;
  }
  SYX_EVAL_THROW(ctx, "expected type kind: '" SV_FMT "'", (sv_fmt_arg(*kind->symbol)));
}

#endif // SYX_TYPES_IMPL
