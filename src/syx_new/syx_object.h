#ifndef SYX_OBJECT_H
#define SYX_OBJECT_H

#include <syx_new/syx_value.h>

Syx_Value **syx_object_lookup(Syx_Object *object, Syx_Symbol *field_name);
Syx_Value *syx_object_get(Syx_Eval_Ctx *ctx, Syx_Object *object, Syx_Symbol *field_name);
void syx_object_define(Syx_Object *object, Syx_Symbol *field_name, Syx_Value *form);
void syx_object_set(Syx_Object *object, Syx_Symbol *field_name, Syx_Value *form);
void syx_object_delete(Syx_Object *object, Syx_Symbol *field_name);

Syx_Value *syx_update_object(Syx_Eval_Ctx *ctx, Syx_Object *object, Syx_Pair **arguments, Syx_Value *first_field);
Syx_Value *syx_define_object(Syx_Eval_Ctx *ctx, Syx_Pair **arguments, Syx_Value *first_field);

#endif // SYX_OBJECT_H

#if defined(SYX_OBJECT_IMPL) && !defined(SYX_OBJECT_IMPL_C)
#define SYX_OBJECT_IMPL_C

#define SYX_VALUE_IMPL
#include <syx_new/syx_value.h>
#define SYX_EVAL_IMPL
#include <syx_new/syx_eval.h>

Syx_Value **syx_object_lookup(Syx_Object *object, Syx_Symbol *field_name) {
  Syx_Value **field = NULL;
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
    return NULL;
  } else if ((*field)->kind == SYX_VALUE_KIND_PREFIXED && (*field)->prefixed->kind == SYX_PREFIXED_KIND_QUOTE) {
    return (*field)->prefixed->value;
  } else {
    Syx_Value *result = rc_acquire(syx_eval(ctx, *field));
    rc_release(*field);
    *field = rc_acquire(make_syx_value_prefixed(SYX_PREFIXED_KIND_QUOTE, result));
    return rc_move(result);
  }
}

void syx__object_field_set(Syx_Object *object, Syx_Symbol *field_name, Syx_Value *form, Syx_Value **field) {
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
  syx__object_field_set(object, field_name, form, field);
}

void syx_object_set(Syx_Object *object, Syx_Symbol *field_name, Syx_Value *form) {
  Syx_Value **field = syx_object_lookup(object, field_name);
  syx__object_field_set(object, field_name, form, field);
}

void syx_object_delete(Syx_Object *object, Syx_Symbol *field_name) {
  Syx_Value **field = ht_find(&object->fields, field_name);
  if (!field) return;
  rc_release(syx_value_from_symbol(field_name));
  rc_release(*field);
}

// (:a (:b (:c <c-value> :d <d-value> :e (:f <f-value>) :g <g-value>)))
Syx_Value *syx_update_object(Syx_Eval_Ctx *ctx, Syx_Object *object, Syx_Pair **arguments, Syx_Value *first_field) {
  while (*arguments) {
    Syx_Value *field;
    if (first_field) {
      field = first_field;
      first_field = NULL;
    } else {
      field = rc_acquire(syx_eval_unquote(ctx, syx_list_next(arguments)));
      syx_value_early_exit(field);
    }
    SYX_EVAL_ASSERT(ctx, field->kind == SYX_VALUE_KIND_PREFIXED, "colon prefixed symbol expected", (), (field));
    SYX_EVAL_ASSERT(ctx, field->prefixed->kind == SYX_PREFIXED_KIND_COLON, "colon prefixed symbol expected", (), (field));
    SYX_EVAL_ASSERT(ctx, field->prefixed->value->kind == SYX_VALUE_KIND_SYMBOL, "colon prefixed symbol expected", (), (field));

    Syx_Value *form = rc_acquire(syx_eval_unquote(ctx, syx_list_next(arguments)));
    if (form->kind == SYX_VALUE_KIND_PAIR && form->pair && form->pair->left->kind == SYX_VALUE_KIND_PREFIXED && form->pair->left->prefixed->kind == SYX_PREFIXED_KIND_COLON) {
      Syx_Pair *arguments = form->pair;
      Syx_Value *result = rc_acquire(syx_define_object(ctx, &arguments, NULL));
      syx_value_early_exit(result, (field, form));
      syx_object_set(object, field->prefixed->value->symbol, result);
      rc_release_all(field, form);
      continue;
    }
    syx_object_set(object, field->prefixed->value->symbol, rc_move(form));
    rc_release(field);
  }
  return syx_value_nil();
}

// ([:proto <proto-object>] :a (:b (:c <c-value> :d <d-value> :e (:f <f-value>) :g <g-value>)))
Syx_Value *syx_define_object(Syx_Eval_Ctx *ctx, Syx_Pair **arguments, Syx_Value *first_field) {
  Syx_Value *value = rc_acquire(make_syx_value_object(NULL));

  Syx_Value *proto_symbol = rc_acquire(make_syx_value_symbol_strlit("proto"));
  Syx_Value *field;
  if (first_field) {
    field = first_field;
    first_field = NULL;
  } else {
    field = rc_acquire(syx_eval_unquote(ctx, syx_list_next(arguments)));
    syx_value_early_exit(field, (proto_symbol));
  }
  if (field->kind == SYX_VALUE_KIND_PREFIXED && field->prefixed->kind == SYX_PREFIXED_KIND_COLON && field->prefixed->value == proto_symbol) {
    Syx_Value *proto = rc_acquire(syx_eval_unquote(ctx, syx_list_next(arguments)));
    syx_value_early_exit(proto, (proto_symbol));
    value->object->proto = rc_acquire(proto)->object;
    rc_release(proto);
    field = NULL;
  }
  rc_release(proto_symbol);

  Syx_Value *result = rc_acquire(syx_update_object(ctx, value->object, arguments, field));
  syx_value_early_exit(result, (value));
  return rc_move(value);
}

#endif // SYX_OBJECT_IMPL_C
