#ifndef SYX_OBJECT_H
#define SYX_OBJECT_H

#include <syx_new/syx_value.h>

Syx_Value **syx_object_lookup(Syx_Object *object, Syx_Symbol *field_name);
Syx_Value *syx_object_get(Syx_Eval_Ctx *ctx, Syx_Object *object, Syx_Symbol *field_name);
void syx_object_define(Syx_Object *object, Syx_Symbol *field_name, Syx_Value *form);
void syx_object_set(Syx_Object *object, Syx_Symbol *field_name, Syx_Value *form);
void syx_object_delete(Syx_Object *object, Syx_Symbol *field_name);

#endif // SYX_OBJECT_H

#define SYX_OBJECT_IMPL
#if defined(SYX_OBJECT_IMPL) && !defined(SYX_OBJECT_IMPL_C)
#define SYX_OBJECT_IMPL_C

#define SYX_VALUE_IMPL
#include <syx_new/syx_value.h>
#define SYX_EVAL_IMPL
#include <syx_new/syx_eval.h>

Syx_Value **syx_object_lookup(Syx_Object *object, Syx_Symbol *field_name) {
  Syx_Value **field;
  while (!field && object) {
    field = ht_find(&object->fields, field_name);
    if (field) break;
    object = object->proto;
  }
  return field;
}

Syx_Value *syx_object_get(Syx_Eval_Ctx *ctx, Syx_Object *object, Syx_Symbol *field_name) {
  Syx_Value **field = syx_object_lookup(object, field_name);
  if (!field) {
    return syx_value_nil();
  } else if ((*field)->kind == SYX_VALUE_KIND_PREFIXED && (*field)->prefixed->kind == SYX_PREFIXED_KIND_QUOTE) {
    return (*field)->prefixed->value;
  } else {
    Syx_Value *result = rc_acquire(syx_eval(ctx, *field));
    rc_release(*field);
    *field = rc_acquire(make_syx_value_prefixed(SYX_PREFIXED_KIND_QUOTE, result));
    return rc_move(result);
  }
}

inline void syx_object_field_set(Syx_Object *object, Syx_Symbol *field_name, Syx_Value *form, Syx_Value **field) {
  if (!field) {
    rc_acquire(syx_value_from_symbol(field_name));
    field = ht_put(&object->fields, field_name);
    *field = rc_acquire(form);
  } else {
    rc_release(*field);
    *field = rc_acquire(form);
  }
}

void syx_object_define(Syx_Object *object, Syx_Symbol *field_name, Syx_Value *form) {
  Syx_Value **field = ht_find(&object->fields, field_name);
  syx_object_field_set(object, field_name, form, field);
}

void syx_object_set(Syx_Object *object, Syx_Symbol *field_name, Syx_Value *form) {
  Syx_Value **field = syx_object_lookup(object, field_name);
  syx_object_field_set(object, field_name, form, field);
}

void syx_object_delete(Syx_Object *object, Syx_Symbol *field_name) {
  Syx_Value **field = ht_find(&object->fields, field_name);
  if (field) {
    rc_release(syx_value_from_symbol(field_name));
    rc_release(*field);
  }
}

#endif // SYX_OBJECT_IMPL_C
