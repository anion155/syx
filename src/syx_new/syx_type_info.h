#ifndef SYX_TYPE_INFO_H
#define SYX_TYPE_INFO_H

#include <defines.h>
#include <ffi/ffi.h>
#include <float.h>
#include <ht.h>
#include <limits.h>
#include <stdalign.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <syx_new/syx_value.h>

typedef enum Syx_Type_Kind : unsigned int {
  SYX_TYPE_KIND_PRIMITIVE,
  SYX_TYPE_KIND_PTR,
  SYX_TYPE_KIND_STRUCTURE,
  SYX_TYPE_KIND_FUNCTION_PTR,
  SYX_TYPE_KIND_VALUE_PTR,
} Syx_Type_Kind;

typedef enum Syx_Primitive_Type_Kind : unsigned int {
  SYX_PRIMITIVE_TYPE_KIND_VOID,   // void
  SYX_PRIMITIVE_TYPE_KIND_CHAR,   // char
  SYX_PRIMITIVE_TYPE_KIND_I8,     // int8_t
  SYX_PRIMITIVE_TYPE_KIND_I16,    // int16_t
  SYX_PRIMITIVE_TYPE_KIND_I32,    // int32_t
  SYX_PRIMITIVE_TYPE_KIND_I64,    // int64_t
  SYX_PRIMITIVE_TYPE_KIND_I128,   // __int128_t
  SYX_PRIMITIVE_TYPE_KIND_U8,     // uint8_t
  SYX_PRIMITIVE_TYPE_KIND_U16,    // uint16_t
  SYX_PRIMITIVE_TYPE_KIND_U32,    // uint32_t
  SYX_PRIMITIVE_TYPE_KIND_U64,    // uint64_t
  SYX_PRIMITIVE_TYPE_KIND_U128,   // __uint128_t
  SYX_PRIMITIVE_TYPE_KIND_INT,    // int
  SYX_PRIMITIVE_TYPE_KIND_LONG,   // long
  SYX_PRIMITIVE_TYPE_KIND_LLONG,  // long long
  SYX_PRIMITIVE_TYPE_KIND_UINT,   // unsigned int
  SYX_PRIMITIVE_TYPE_KIND_ULONG,  // unsigned long
  SYX_PRIMITIVE_TYPE_KIND_ULLONG, // unsigned long long
  SYX_PRIMITIVE_TYPE_KIND_FLOAT,  // float
  SYX_PRIMITIVE_TYPE_KIND_DOUBLE, // double
  SYX_PRIMITIVE_TYPE_KIND_SIZE,   // size_t
} Syx_Primitive_Type_Kind;

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
typedef Da_Slice(Syx_Type *, Syx_Types) Syx_Types;

typedef Syx_Value *(*Syx_Type_Structure_Constructor)(Syx_Eval_Ctx *ctx, void *data, Syx_Value *arguments);
typedef Syx_Value *(*Syx_Type_Structure_Index_Getter)(Syx_Eval_Ctx *ctx, void *data, syx_integer_t index);
typedef Syx_Value *(*Syx_Type_Structure_Index_Setter)(Syx_Eval_Ctx *ctx, void *data, syx_integer_t index, Syx_Value *argument);
typedef Syx_Value *(*Syx_Type_Structure_Field_Getter)(Syx_Eval_Ctx *ctx, void *data, const char *field_name);
typedef Syx_Value *(*Syx_Type_Structure_Field_Setter)(Syx_Eval_Ctx *ctx, void *data, const char *field_name, Syx_Value *argument);
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
  Syx_Type *return_type;
  Syx_Types arg_types;
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

ffi_status syx_type_function_ffi_prep(Syx_Type_Function *func);
void *syx_type_function_ffi_call(Syx_Type_Function *func, void (*func_ptr)(void), void **arg_values);

size_t sb_append_syx_type(String_Builder *sb, const Syx_Type *type);

typedef Ht(const char *, Syx_Type *, SYX_KNOWN_TYPES_t) SYX_KNOWN_TYPES_t;
syx_predefine_constant(SYX_KNOWN_TYPES_t, SYX_KNOWN_TYPES);
void syx_env_define_types(Syx_Env *env);

#endif // SYX_TYPE_INFO_H

#if defined(SYX_TYPE_INFO_IMPL) && !defined(SYX_TYPE_INFO_IMPL_C)
#define SYX_TYPE_INFO_IMPL_C

#define SYX_VALUE_IMPL
#include <syx_new/syx_value.h>
#define SYX_EVAL_IMPL
#include <syx_new/syx_eval.h>

void syx_type_destructor(void *data) {
  Syx_Type *type = data;
  if (type->name) rc_release(syx_value_from_symbol(type->name));
}

Syx_Type *make_syx_type(Syx_Type_Kind kind, size_t size, size_t alignment, Syx_Symbol *name, ffi_type *ffi_t, size_t additional_size) {
  Syx_Type *type = rc_malloc(sizeof(Syx_Type) + additional_size);
  assert(type);
  rc_get(type)->methods.destructor = syx_type_destructor;
  type->kind = kind;
  type->size = size;
  type->alignment = alignment;
  if (name) rc_acquire(syx_value_from_symbol(name));
  type->name = name;
  type->ffi_t = ffi_t;
  return type;
}

Syx_Type *make_syx_type_primitive(Syx_Primitive_Type_Kind kind, size_t size, size_t alignment, Syx_Symbol *name, ffi_type *ffi_t) {
  Syx_Type *type = make_syx_type(SYX_TYPE_KIND_PRIMITIVE, size, alignment, name, ffi_t, 0);
  type->primitive = kind;
  return type;
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
  da_foreach(&structure->fields, field) {
    if (field->name) rc_release(syx_value_from_symbol(field->name));
    rc_release(field->type);
  }
}

void syx_type_structure_graph_visitor(Rc_Circulars *circulars, const void *data, const void *source) {
  Syx_Type_Structure *structure = ((const Syx_Type *)data)->structure;
  da_foreach(&structure->fields, field) {
    rc_graph_visitor(circulars, (void **)&(field->type), source);
  }
}

Syx_Type *make_syx_type_structure(Syx_Symbol *name, Syx_Type_Structure structure) {
  Syx_Type *type = make_syx_type(SYX_TYPE_KIND_STRUCTURE, 0, 0, name, NULL, sizeof(Syx_Type_Structure) + sizeof(Syx_Type_Structure_Field) * structure.fields.count + sizeof(ffi_type) + sizeof(ffi_type) * (structure.fields.count + 1));
  rc_get(type)->methods = (Rc_Methods){.destructor = syx_type_structure_destructor, .graph_visitor = syx_type_structure_graph_visitor};
  type->structure = (Syx_Type_Structure *)(type + 1);
  *type->structure = structure;
  Da(Syx_Type_Structure_Field) fields = {.data = (Syx_Type_Structure_Field *)(type->structure + 1), .capacity = structure.fields.count, .count = 0};
  size_t offset = 0, max_alignment = 1;
  da_foreach(&structure.fields, field) {
    if (field->type->alignment > max_alignment) max_alignment = field->type->alignment;
    if (field->type->alignment > 0) offset = (offset + field->type->alignment - 1) & ~(field->type->alignment - 1);
    if (field->offset == 0) field->offset = offset;
    offset += field->type->size;
    if (field->name) rc_acquire(syx_value_from_symbol(field->name));
    rc_acquire(field->type);
    da_append(&fields, *field);
  }
  free((Syx_Type_Structure_Field *)structure.fields.data);
  type->structure->fields = da_slice(fields, Syx_Type_Structure_Fields);
  type->size = (offset + max_alignment - 1) & ~(max_alignment - 1);
  type->alignment = type->alignment;
  type->ffi_t = (ffi_type *)(fields.data + structure.fields.count);
  type->ffi_t->type = FFI_TYPE_STRUCT;
  type->ffi_t->size = 0;
  type->ffi_t->alignment = 0;
  type->ffi_t->elements = (ffi_type **)(type->ffi_t + 1);
  for (size_t index = 0; index < fields.count; index += 1) {
    type->ffi_t->elements[index] = fields.data[index].type->ffi_t;
  }
  type->ffi_t->elements[fields.count] = NULL;
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
  da_foreach(&func->arg_types, arg_type) rc_release((Syx_Type *)*arg_type);
}

void syx_type_function_graph_visitor(Rc_Circulars *circulars, const void *data, const void *source) {
  Syx_Type_Function *func = ((const Syx_Type *)data)->function;
  rc_graph_visitor(circulars, (void **)&(func->return_type), source);
  da_foreach(&func->arg_types, arg_type) {
    rc_graph_visitor(circulars, (void **)arg_type, source);
  }
}

Syx_Type *make_syx_type_function(Syx_Symbol *name, Syx_Type_Function func) {
  Syx_Type *type = make_syx_type(SYX_TYPE_KIND_FUNCTION_PTR, sizeof(void (*)(void)), alignof(void (*)(void)), name, NULL, sizeof(Syx_Type_Function) + sizeof(ffi_cif) + sizeof(Syx_Type *) * func.arg_types.count);
  rc_get(type)->methods = (Rc_Methods){.destructor = syx_type_function_destructor, .graph_visitor = syx_type_function_graph_visitor};
  type->function = (Syx_Type_Function *)(type + 1);
  type->function->return_type = rc_acquire(func.return_type);
  Da(Syx_Type *) arg_types = {.data = (Syx_Type **)((ffi_cif *)(type->function + 1) + 1), .capacity = func.arg_types.count, .count = 0};
  da_append_many_n(&arg_types, func.arg_types.data, func.arg_types.count);
  type->function->arg_types = da_slice(arg_types, Syx_Types);
  type->function->vaargs = func.vaargs;
  return type;
}

ffi_status syx_type_function_ffi_prep(Syx_Type_Function *func) {
  if (func->ffi_f) return FFI_OK;
  ffi_cif *ffi_f = (ffi_cif *)(func + 1);
  ffi_status result = ffi_prep_cif(ffi_f, FFI_DEFAULT_ABI, func->arg_types.count, func->return_type->ffi_t, (ffi_type **)func->arg_types.data);
  if (result == FFI_OK) func->ffi_f = ffi_f;
  return result;
}

void *syx_type_function_ffi_call(Syx_Type_Function *func, void (*func_ptr)(void), void **arg_values) {
  void *return_buffer = malloc(func->return_type->size);
  memset(return_buffer, 0, func->return_type->size);
  ffi_call(func->ffi_f, func_ptr, return_buffer, arg_values);
  return return_buffer;
}

// Syx_Value *syx_eval_native_function(Syx_Eval_Ctx *ctx, Syx_Boxed *boxed, SyxV *arguments) {
//   if (boxed->typeinfo->kind != SYX_TYPE_INFO_KIND_FUNCTION_PTR) RUNTIME_ERROR(ctx, "boxed function pointer expected");
//   Syx_Type_Info_Function *function = &boxed->typeinfo->function;
//   size_t argc = function->argc ? function->argc : 1;
//   if (function->cif.arg_types == NULL) {
//     if (ffi_prep_cif(&function->cif, FFI_DEFAULT_ABI, function->argc, function->cif_return_type, function->cif_argv_types) != FFI_OK) {
//       RUNTIME_ERROR(ctx, "invalid c function descriptor");
//     }
//   }
//   void *arg_values[argc];
//   for (size_t index = 0; index < function->argc; index += 1) {
//     SyxV *argument = syxv_list_next(&arguments);
//     Syx_Type_Info *argument_type = function->argv_types[index];
//     if (argument->kind != SYXV_KIND_BOXED) TODO("implement values conversion to boxed values");
//     if (argument_type->kind != argument->boxed->typeinfo->kind) TODO("implement values conversion to boxed values");
//     arg_values[index] = &argument->boxed->data;
//   }
//   if (arguments && arguments->kind == SYXV_KIND_PAIR) TODO("implement vaargs support");
//   void **return_buffer = malloc(function->return_type->size);
//   memset(return_buffer, 0, function->return_type->size);
//   ffi_call(&function->cif, FFI_FN(*(void **)boxed->data), return_buffer, arg_values);
//   return make_syxv_boxed(make_syx_boxed(.typeinfo = function->return_type, .data = return_buffer, .parent = NULL));
// }

size_t sb_append_syx_type(String_Builder *sb, const Syx_Type *type) {
  Stringify_State state = make_stringify_state(sb, 256);
  switch (type->kind) {
    case SYX_TYPE_KIND_PRIMITIVE: {
      stringify_append(&state, sb_append_strlit, "#.");
      if (!type->name) UNREACHABLE("primitive types must have name");
      stringify_append(&state, sb_append_syx_symbol, type->name);
    } break;
    case SYX_TYPE_KIND_PTR: {
      stringify_append(&state, sb_append_strlit, "#.ref");
      if (type->name) {
        stringify_append(&state, sb_append, '<');
        stringify_append(&state, sb_append_syx_symbol, type->name);
        stringify_append(&state, sb_append, '>');
      }
      stringify_append(&state, sb_append, '(');
      stringify_append(&state, sb_append_syx_type, type->pointer);
      stringify_append(&state, sb_append, ')');
    } break;
    case SYX_TYPE_KIND_STRUCTURE: {
      stringify_append(&state, sb_append_strlit, "#.");
      if (type->name) {
        stringify_append(&state, sb_append, '<');
        stringify_append(&state, sb_append_syx_symbol, type->name);
        stringify_append(&state, sb_append, '>');
      } else {
        stringify_append(&state, sb_append_strlit, "<anonim>");
      }
    } break;
    case SYX_TYPE_KIND_FUNCTION_PTR: {
      stringify_append(&state, sb_append_strlit, "#.fn");
      if (type->name) {
        stringify_append(&state, sb_append, '<');
        stringify_append(&state, sb_append_syx_symbol, type->name);
        stringify_append(&state, sb_append, '>');
      } else {
        stringify_append(&state, sb_append_strlit, "<anonim>");
      }
      Syx_Type_Function *func = type->function;
      stringify_append(&state, sb_append, '(');
      stringify_append(&state, sb_append_syx_type, func->return_type);
      stringify_append(&state, sb_append, '(');
      for (size_t index = 0; index < func->arg_types.count; index += 1) {
        if (index != 0) stringify_append(&state, sb_append_strlit, ", ");
        const Syx_Type *arg = func->arg_types.data[index];
        stringify_append(&state, sb_append_syx_type, arg);
      }
      if (type->function->vaargs) {
        if (func->arg_types.count) stringify_append(&state, sb_append_strlit, ", ");
        stringify_append(&state, sb_append_strlit, "...");
      }
      stringify_append(&state, sb_append, ')');
      stringify_append(&state, sb_append, ')');
    } break;
    case SYX_TYPE_KIND_VALUE_PTR: {
      stringify_append(&state, sb_append_strlit, "#.value");
    } break;
  }
  return state.count;
}

syx_define_constant(, SYX_KNOWN_TYPES) {
  SYX_KNOWN_TYPES->hasheq = ht_cstr_hasheq;
  *ht_put(SYX_KNOWN_TYPES, "c_void") = make_syx_type_primitive(SYX_PRIMITIVE_TYPE_KIND_VOID, sizeof(void), alignof(void), NULL, &ffi_type_void);
  *ht_put(SYX_KNOWN_TYPES, "c_char") = make_syx_type_primitive(SYX_PRIMITIVE_TYPE_KIND_CHAR, sizeof(char), alignof(char), NULL,
#if CHAR_MIN < 0
                                                               &ffi_type_schar
#else
                                                               &ffi_type_uchar
#endif
  );
  *ht_put(SYX_KNOWN_TYPES, "c_i8") = make_syx_type_primitive(SYX_PRIMITIVE_TYPE_KIND_I8, sizeof(int8_t), alignof(int8_t), NULL, &ffi_type_sint8);
  *ht_put(SYX_KNOWN_TYPES, "c_i16") = make_syx_type_primitive(SYX_PRIMITIVE_TYPE_KIND_I16, sizeof(int16_t), alignof(int16_t), NULL, &ffi_type_sint16);
  *ht_put(SYX_KNOWN_TYPES, "c_i32") = make_syx_type_primitive(SYX_PRIMITIVE_TYPE_KIND_I32, sizeof(int32_t), alignof(int32_t), NULL, &ffi_type_sint32);
  *ht_put(SYX_KNOWN_TYPES, "c_i64") = make_syx_type_primitive(SYX_PRIMITIVE_TYPE_KIND_I64, sizeof(int64_t), alignof(int64_t), NULL, &ffi_type_sint64);
  ffi_type *ffi_type_sint128 = malloc(sizeof(ffi_type) + sizeof(ffi_type *) * 3);
  ffi_type_sint128->type = FFI_TYPE_STRUCT;
  ffi_type_sint128->size = 0;
  ffi_type_sint128->alignment = 0;
  ffi_type_sint128->elements = (ffi_type **)(ffi_type_sint128 + 1);
  ffi_type_sint128->elements[0] = &ffi_type_sint64;
  ffi_type_sint128->elements[1] = &ffi_type_sint64;
  ffi_type_sint128->elements[2] = NULL;
  *ht_put(SYX_KNOWN_TYPES, "c_i128") = make_syx_type_primitive(SYX_PRIMITIVE_TYPE_KIND_I128, sizeof(__int128), alignof(__int128), NULL, ffi_type_sint128);
  *ht_put(SYX_KNOWN_TYPES, "c_u8") = make_syx_type_primitive(SYX_PRIMITIVE_TYPE_KIND_U8, sizeof(uint8_t), alignof(uint8_t), NULL, &ffi_type_uint8);
  *ht_put(SYX_KNOWN_TYPES, "c_u16") = make_syx_type_primitive(SYX_PRIMITIVE_TYPE_KIND_U16, sizeof(uint16_t), alignof(uint16_t), NULL, &ffi_type_uint16);
  *ht_put(SYX_KNOWN_TYPES, "c_u32") = make_syx_type_primitive(SYX_PRIMITIVE_TYPE_KIND_U32, sizeof(uint32_t), alignof(uint32_t), NULL, &ffi_type_uint32);
  *ht_put(SYX_KNOWN_TYPES, "c_u64") = make_syx_type_primitive(SYX_PRIMITIVE_TYPE_KIND_U64, sizeof(uint64_t), alignof(uint64_t), NULL, &ffi_type_uint64);
  ffi_type *ffi_type_uint128 = malloc(sizeof(ffi_type) + sizeof(ffi_type *) * 3);
  ffi_type_uint128->type = FFI_TYPE_STRUCT;
  ffi_type_uint128->size = 0;
  ffi_type_uint128->alignment = 0;
  ffi_type_uint128->elements = (ffi_type **)(ffi_type_uint128 + 1);
  ffi_type_uint128->elements[0] = &ffi_type_uint64;
  ffi_type_uint128->elements[1] = &ffi_type_uint64;
  ffi_type_uint128->elements[2] = NULL;
  *ht_put(SYX_KNOWN_TYPES, "c_u128") = make_syx_type_primitive(SYX_PRIMITIVE_TYPE_KIND_U128, sizeof(unsigned __int128), alignof(unsigned __int128), NULL, ffi_type_uint128);
  *ht_put(SYX_KNOWN_TYPES, "c_int") = make_syx_type_primitive(SYX_PRIMITIVE_TYPE_KIND_INT, sizeof(int), alignof(int), NULL, &ffi_type_sint);
  *ht_put(SYX_KNOWN_TYPES, "c_long") = make_syx_type_primitive(SYX_PRIMITIVE_TYPE_KIND_LONG, sizeof(signed long), alignof(signed long), NULL, &ffi_type_slong);
  // *ht_put(SYX_KNOWN_TYPES, "c_llong") = make_syx_type_primitive(SYX_PRIMITIVE_TYPE_KIND_LLONG, sizeof(signed long long), alignof(signed long long), NULL, &ffi_type_sint64);
  *ht_put(SYX_KNOWN_TYPES, "c_uint") = make_syx_type_primitive(SYX_PRIMITIVE_TYPE_KIND_UINT, sizeof(unsigned), alignof(unsigned), NULL, &ffi_type_uint);
  *ht_put(SYX_KNOWN_TYPES, "c_ulong") = make_syx_type_primitive(SYX_PRIMITIVE_TYPE_KIND_ULONG, sizeof(unsigned long), alignof(unsigned long), NULL, &ffi_type_ulong);
  // *ht_put(SYX_KNOWN_TYPES, "c_ullong") = make_syx_type_primitive(SYX_PRIMITIVE_TYPE_KIND_ULLONG, sizeof(unsigned long long), alignof(unsigned long long), NULL, &ffi_type_uint64);
  *ht_put(SYX_KNOWN_TYPES, "c_float") = make_syx_type_primitive(SYX_PRIMITIVE_TYPE_KIND_FLOAT, sizeof(float), alignof(float), NULL, &ffi_type_float);
  *ht_put(SYX_KNOWN_TYPES, "c_double") = make_syx_type_primitive(SYX_PRIMITIVE_TYPE_KIND_DOUBLE, sizeof(double), alignof(double), NULL, &ffi_type_double);
  // *ht_put(SYX_KNOWN_TYPES, "c_size") = make_syx_type_primitive(SYX_PRIMITIVE_TYPE_KIND_SIZE, sizeof(size_t), alignof(size_t), NULL, &ffi_type_double);

  *ht_put(SYX_KNOWN_TYPES, "value") = make_syx_type(SYX_TYPE_KIND_VALUE_PTR, sizeof(Syx_Value *), alignof(Syx_Value *), NULL, &ffi_type_pointer, 0);
  *ht_put(SYX_KNOWN_TYPES, "c_cstr") = make_syx_type_pointer(NULL, *ht_find(SYX_KNOWN_TYPES, "c_char"));
  Syx_Type_Structure_Fields string_fields = make_syx_type_structure_fields(
      (Syx_Type_Structure_Field){.name = make_syx_value_symbol_strlit("data"), .readonly = true, .type = *ht_find(SYX_KNOWN_TYPES, "c_cstr")},
      (Syx_Type_Structure_Field){.name = make_syx_value_symbol_strlit("count"), .readonly = true, .type = *ht_find(SYX_KNOWN_TYPES, "c_size")});
  *ht_put(SYX_KNOWN_TYPES, "string") = make_syx_type_structure(NULL, (Syx_Type_Structure){.fields = string_fields});
}

void syx_env_define_types(Syx_Env *env) {
  UNUSED(env);
  TODO("syx_env_define_types");
  ht_foreach(type, SYX_KNOWN_TYPES()) {
    const char *key = ht_key(SYX_KNOWN_TYPES(), type);
    rc_acquire(type);
    Syx_Value *name = rc_acquire(make_syx_value_symbol_cstr(key));
    (*type)->name = name->symbol;
    // syx_env_define(env, (*type)->name, make_syx_value_native_constructor(*type));
  }
}

#endif // SYX_TYPE_INFO_IMPL
