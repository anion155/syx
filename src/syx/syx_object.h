#ifndef SYX_OBJECT_H
#define SYX_OBJECT_H

#include <syx/syx_value.h>

Syx_Value *syx_object_get(Syx_Eval_Ctx *ctx, Syx_Object *object, Syx_Symbol *field_name);
Syx_Value *syx_object_define_field(Syx_Eval_Ctx *ctx, Syx_Object *object, Syx_Symbol *field_name, Syx_Field descriptor);
Syx_Value *syx_object_set_field(Syx_Eval_Ctx *ctx, Syx_Object *object, Syx_Symbol *field_name, Syx_Field descriptor);
Syx_Value *syx_object_delete_field(Syx_Eval_Ctx *ctx, Syx_Object *object, Syx_Symbol *field_name);

Syx_Value *syx_object_update(Syx_Eval_Ctx *ctx, Syx_Object *object, Syx_Pair **arguments, Syx_Value *first_field);
Syx_Value *syx_object_define(Syx_Eval_Ctx *ctx, Syx_Pair **arguments, Syx_Value *first_field);

#endif // SYX_OBJECT_H

#if defined(SYX_OBJECT_IMPL) && !defined(SYX_OBJECT_IMPL_C)
#define SYX_OBJECT_IMPL_C

#define SYX_VALUE_IMPL
#include <syx/syx_value.h>
#define SYX_EVAL_IMPL
#include <syx/syx_eval.h>
#define SYX_FIELDS_IMPL
#include <syx/syx_fields.h>

Syx_Object *syx__object_lookup(Syx_Object *object, Syx_Symbol *field_name) {
  while (object) {
    if (syx_fields_has(object->fields, field_name)) break;
    object = object->proto;
  }
  return object;
}

Syx_Value *syx_object_get(Syx_Eval_Ctx *ctx, Syx_Object *object, Syx_Symbol *field_name) {
  bool extensible = object->access & SYX_OBJECT_ACCESS_FLAG_EXTEND;
  object = syx__object_lookup(object, field_name);
  if (!object) {
    SYX_EVAL_ASSERT(ctx, extensible, "object is not extensible");
    return syx_value_nil();
  }
  return syx_fields_get(ctx, object->fields, field_name);
}

Syx_Value *syx_object_define_field(Syx_Eval_Ctx *ctx, Syx_Object *object, Syx_Symbol *field_name, Syx_Field descriptor) {
  SYX_EVAL_ASSERT(ctx, object->access & SYX_OBJECT_ACCESS_FLAG_EXTEND, "object is not extensible");
  syx_fields_define_field(object->fields, field_name, descriptor);
  return syx_value_nil();
}

Syx_Value *syx_object_set_field(Syx_Eval_Ctx *ctx, Syx_Object *object, Syx_Symbol *field_name, Syx_Field descriptor) {
  object = ({
    Syx_Object *storage = syx__object_lookup(object, field_name);
    if (!storage) SYX_EVAL_ASSERT(ctx, object->access & SYX_OBJECT_ACCESS_FLAG_EXTEND, "object is not extensible");
    storage ? storage : object;
  });
  SYX_EVAL_ASSERT(ctx, object->access & SYX_OBJECT_ACCESS_FLAG_WRITE, "object is not writable");
  syx_fields_define_field(object->fields, field_name, descriptor);
  return syx_value_nil();
}

Syx_Value *syx_object_delete_field(Syx_Eval_Ctx *ctx, Syx_Object *object, Syx_Symbol *field_name) {
  object = syx__object_lookup(object, field_name);
  if (!object) return syx_value_bool_false();
  SYX_EVAL_ASSERT(ctx, object->access & SYX_OBJECT_ACCESS_FLAG_DELETE, "can not delete field from object");
  syx_fields_delete_field(object->fields, field_name);
  return syx_value_bool_true();
}

// (:a <a-value> :b (:c <c-value>))
Syx_Value *syx_object_update(Syx_Eval_Ctx *ctx, Syx_Object *object, Syx_Pair **arguments, Syx_Value *first_field) {
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

    Syx_Value *value = rc_acquire(syx_eval_unquote(ctx, syx_list_next(arguments)));
    if (value->kind == SYX_VALUE_KIND_PAIR && value->pair && value->pair->left->kind == SYX_VALUE_KIND_PREFIXED && value->pair->left->prefixed->kind == SYX_PREFIXED_KIND_COLON) {
      Syx_Value *stored = syx_object_get(ctx, object, field->prefixed->value->symbol);
      Syx_Pair *arguments = value->pair;
      if (stored && stored->kind == SYX_VALUE_KIND_OBJECT) {
        syx_safe_call_with_release(syx_object_update(ctx, stored->object, &arguments, NULL), (field, value));
      } else {
        Syx_Value *result_object = syx_safe_call(syx_object_define(ctx, &arguments, NULL), (field, value));
        syx_safe_call_with_release(syx_object_set_field(ctx, object, field->prefixed->value->symbol, syx_value_field(rc_move(result_object))), (field, value));
      }
      rc_release_all(field, value);
      continue;
    }
    syx_safe_call_with_release(syx_object_set_field(ctx, object, field->prefixed->value->symbol, syx_form_field(rc_move(value))), (field));
    rc_release(field);
  }
  return syx_value_nil();
}

// ([:proto <proto-object>] :a <a-value> :b (:c <c-value>))
Syx_Value *syx_object_define(Syx_Eval_Ctx *ctx, Syx_Pair **arguments, Syx_Value *first_field) {
  Syx_Value *value = rc_acquire(make_syx_value_object());

  Syx_Value *proto_symbol = rc_acquire(syx_value_symbol_strlit("proto"));
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

  Syx_Value *result = rc_acquire(syx_object_update(ctx, value->object, arguments, field));
  syx_value_early_exit(result, (value));
  rc_release(result);
  return rc_move(value);
}

#endif // SYX_OBJECT_IMPL_C
