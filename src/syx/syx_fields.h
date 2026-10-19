#ifndef SYX_FIELDS_H
#define SYX_FIELDS_H

#include <defines.h>
#include <ht.h>

typedef struct Syx_Symbol Syx_Symbol;
typedef struct Syx_Value Syx_Value;
typedef struct Syx_Closure Syx_Closure;
typedef struct Syx_Eval_Ctx Syx_Eval_Ctx;

typedef enum Syx_Field_Kind {
  SYX_FIELD_KIND_FORM,
  SYX_FIELD_KIND_VALUE,
  SYX_FIELD_KIND_ACCESSOR,
} Syx_Field_Kind;
typedef struct Syx_Value_Field {
  bool writable;
  Syx_Value *value;
} Syx_Value_Field;
typedef struct Syx_Accessor_Field {
  Syx_Closure *getter;
  Syx_Closure *setter;
} Syx_Accessor_Field;
typedef struct Syx_Field {
  Syx_Field_Kind kind;
  union {
    Syx_Value *form;
    Syx_Value_Field value;
    Syx_Accessor_Field accessor;
  } as;
} Syx_Field;
typedef Ht(Syx_Symbol *, Syx_Field, Syx_Fields_Map) Syx_Fields_Map;

void syx_fields_init(Syx_Fields_Map *fields);
#define syx_form_field(_form) ((Syx_Field){.kind = SYX_FIELD_KIND_FORM, .as = {.form = (_form)}})
#define syx_value_field(_value, ...) ((Syx_Field){.kind = SYX_FIELD_KIND_VALUE, .as = {.value = {.writable = WITH_DEFAULT(true, __VA_ARGS__), .value = (_value)}}})
#define syx_accessor_field(_getter, ...) ((Syx_Field){.kind = SYX_FIELD_KIND_ACCESSOR, .as = {.accessor = {.getter = (_getter), .setter = WITH_DEFAULT(NULL, __VA_ARGS__)}}})
void syx_field_acquire(Syx_Field *field);
void syx_field_release(Syx_Field *field);
Syx_Fields_Map *make_syx_fields();
Syx_Field *syx_fields_get_field(Syx_Fields_Map *fields, Syx_Symbol *name);
bool syx_fields_has(Syx_Fields_Map *fields, Syx_Symbol *name);
Syx_Value *syx_fields_get(Syx_Eval_Ctx *ctx, Syx_Fields_Map *fields, Syx_Symbol *name);
Syx_Value *syx_field_get(Syx_Eval_Ctx *ctx, Syx_Field *field);
void syx_fields_define_field(Syx_Fields_Map *fields, Syx_Symbol *name, Syx_Field descriptor);
void syx_fields_delete_field(Syx_Fields_Map *fields, Syx_Symbol *name);
size_t sb_append_syx_fields(String_Builder *sb, const Syx_Fields_Map *fields);

#endif // SYX_FIELDS_H

#if defined(SYX_FIELDS_IMPL) && !defined(SYX_FIELDS_IMPL_C)
#define SYX_FIELDS_IMPL_C

#define SYX_VALUE_IMPL
#include <syx/syx_value.h>
#define SYX_EVAL_IMPL
#include <syx/syx_eval.h>

void syx_fields_init(Syx_Fields_Map *fields) {
  fields->hasheq = ht_syx_symbol_hasheq;
}

void syx_field_acquire(Syx_Field *field) {
  if (!field) return;
  switch (field->kind) {
    case SYX_FIELD_KIND_FORM: {
      rc_acquire(field->as.form);
    } break;
    case SYX_FIELD_KIND_VALUE: {
      rc_acquire(field->as.value.value);
    } break;
    case SYX_FIELD_KIND_ACCESSOR: {
      rc_acquire(syx_value_from_closure(field->as.accessor.getter));
      rc_acquire(syx_value_from_closure(field->as.accessor.setter));
    } break;
  }
}

void syx_field_release(Syx_Field *field) {
  if (!field) return;
  switch (field->kind) {
    case SYX_FIELD_KIND_FORM: {
      rc_release(field->as.form);
    } break;
    case SYX_FIELD_KIND_VALUE: {
      rc_release(field->as.value.value);
    } break;
    case SYX_FIELD_KIND_ACCESSOR: {
      rc_release(syx_value_from_closure(field->as.accessor.getter));
      rc_release(syx_value_from_closure(field->as.accessor.setter));
    } break;
  }
}

void syx_fields_destructor(void *data) {
  Syx_Fields_Map *fields = data;
  ht_foreach(field, fields) {
    Syx_Symbol *field_name = ht_key(fields, field);
    syx_field_release(field);
    rc_release(syx_value_from_symbol(field_name));
  }
  ht_free(fields);
}

void syx_fields_graph_visitor(Rc_Circulars *circulars, const void *data, const void *source) {
  const Syx_Fields_Map *fields = data;
  ht_foreach(field, fields) {
    switch (field->kind) {
      case SYX_FIELD_KIND_FORM: {
        if (field->as.form) {
          rc_graph_visitor(circulars, (void **)&field->as.form, source);
        }
      } break;
      case SYX_FIELD_KIND_VALUE: {
        if (field->as.value.value) {
          rc_graph_visitor(circulars, (void **)&field->as.value.value, source);
        }
      } break;
      case SYX_FIELD_KIND_ACCESSOR: {
        if (field->as.accessor.getter) {
          rc__graph_visitor(circulars, rc_get(syx_value_from_closure(field->as.accessor.getter)), (void **)&field->as.accessor.getter, source);
        }
        if (field->as.accessor.setter) {
          rc__graph_visitor(circulars, rc_get(syx_value_from_closure(field->as.accessor.setter)), (void **)&field->as.accessor.setter, source);
        }
      } break;
    }
  }
}

Syx_Fields_Map *make_syx_fields() {
  Syx_Fields_Map *fields = rc_malloc(sizeof(Syx_Fields_Map), .destructor = syx_fields_destructor, .graph_visitor = syx_fields_graph_visitor);
  assert(fields);
  memset(fields, 0, sizeof(Syx_Fields_Map));
  syx_fields_init(fields);
  return fields;
}

Syx_Field *syx_fields_get_field(Syx_Fields_Map *fields, Syx_Symbol *name) {
  return ht_find(fields, name);
}

bool syx_fields_has(Syx_Fields_Map *fields, Syx_Symbol *name) {
  return ht_find(fields, name) != NULL;
}

Syx_Value *syx_fields_get(Syx_Eval_Ctx *ctx, Syx_Fields_Map *fields, Syx_Symbol *name) {
  Syx_Field *field = ht_find(fields, name);
  if (!field) return NULL;
  return syx_field_get(ctx, field);
}

Syx_Value *syx_field_get(Syx_Eval_Ctx *ctx, Syx_Field *field) {
  switch (field->kind) {
    case SYX_FIELD_KIND_FORM: {
      Syx_Value *result = rc_acquire(syx_eval(ctx, field->as.form));
      rc_release(field->as.form);
      *field = (Syx_Field){.kind = SYX_FIELD_KIND_VALUE, .as.value = {.writable = true, .value = rc_acquire(result)}};
      return rc_move(result);
    } break;
    case SYX_FIELD_KIND_VALUE: {
      return field->as.value.value;
    } break;
    case SYX_FIELD_KIND_ACCESSOR: {
      return rc_acquire(syx_eval_closure(ctx, field->as.accessor.getter, NULL));
    } break;
  }
}

void syx_fields_define_field(Syx_Fields_Map *fields, Syx_Symbol *name, Syx_Field descriptor) {
  Syx_Field *field = ht_find(fields, name);
  if (!field) {
    rc_acquire(syx_value_from_symbol(name));
    field = ht_put(fields, name);
  } else {
    syx_field_release(field);
  }
  *field = descriptor;
  syx_field_acquire(field);
}

void syx_fields_delete_field(Syx_Fields_Map *fields, Syx_Symbol *name) {
  Syx_Field *field = ht_find(fields, name);
  if (!field) return;
  rc_release(syx_value_from_symbol(name));
  syx_field_release(field);
  ht_find_and_delete(fields, name);
}

size_t sb_append_syx_fields(String_Builder *sb, const Syx_Fields_Map *fields) {
  Stringify_State state = make_stringify_state(sb, 256);
  bool add_space = false;
  ht_foreach(field, (Syx_Fields_Map *)fields) {
    if (add_space) {
      stringify_append(&state, sb_append, ' ');
    } else {
      add_space = true;
    }
    stringify_append(&state, sb_append, ':');
    Syx_Symbol *name = ht_key(fields, field);
    stringify_append(&state, sb_append_syx_value, syx_value_from_symbol(name));
    stringify_append(&state, sb_append, ' ');
    switch (field->kind) {
      case SYX_FIELD_KIND_FORM: {
        stringify_append(&state, sb_append, '\'');
        stringify_append(&state, sb_append_syx_value, field->as.form);
      } break;
      case SYX_FIELD_KIND_VALUE: {
        stringify_append(&state, sb_append_syx_value, field->as.value.value);
      } break;
      case SYX_FIELD_KIND_ACCESSOR: {
        TODO();
        // stringify_append(&state, sb_append, '\'');
        // stringify_append(&state, sb_append_syx_value, field->as.form);
      } break;
    }
  }
  return state.count;
}

#endif // SYX_FIELDS_IMPL_C
