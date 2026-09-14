# Memory optimisation: pack values

- STATUS: OPEN
- PRIORITY: 95
- TAGS: value

Pack values into predefined structs:
- Syx_Value_Kind as 3 bits stored in opaque pointer (typedef void* Syx_value):
  + SYX_VALUE_KIND_FALSE: NULL
  + SYX_VALUE_KIND_TRUE: NULL | 0b001
  + SYX_VALUE_KIND_PAIR: (NULL | 0b010) || ((Syx_Pair* & ~0b111) | 0b010)
  + SYX_VALUE_KIND_LITERAL: (Syx_Literal* & ~0b111) | 0b011
  + SYX_VALUE_KIND_CLOSURE: (Syx_Closure* & ~0b111) | 0b100
  + SYX_VALUE_KIND_NATIVE: (Syx_Native* & ~0b111) | 0b101
  + SYX_VALUE_KIND_EXIT: (Syx_Exit* & ~0b111) | 0b110
  + SYX_VALUE_KIND_PREFIXED: (Syx_Prefixed* & ~0b111) | 0b111
- Syx_Pair: { Syx_Value left, Syx_Value right }
- Syx_Literal_Kind: SYX_LITERAL_KIND_SYMBOL, SYX_LITERAL_KIND_NUMBER, SYX_LITERAL_KIND_STRING, SYX_LITERAL_KIND_OBJECT
- Syx_Symbol_Literal: { Syx_Literal_Kind lkind = SYX_LITERAL_KIND_SYMBOL; bool guarded; size_t count } + const data[count] // maybe hide guarded in lkind's high bits
- Syx_Number_Literal: { Syx_Literal_Kind lkind = SYX_LITERAL_KIND_NUMBER; Syx_Number_Kind nkind } + union { float f32; int8_t i8 } // maybe hide nkind in lkind's high bits
- Syx_String_Literal: { Syx_Literal_Kind lkind = SYX_LITERAL_KIND_STRING; size_t count } + const data[count]
- Syx_Object_Literal: { Syx_Literal_Kind lkind = SYX_LITERAL_KIND_OBJECT; Syx_Object_Literal *parent; Ht(Syx_Symbol_Literal *, Syx_value) fields } // parent is untagged pointer
- Syx_Literal: Syx_Symbol_Literal* | Syx_Number_Literal* | Syx_String_Literal* | Syx_Object_Literal*
- Syx_Closure_Kind: SYX_CLOSURE_KIND_SPECIALF, SYX_CLOSURE_KIND_BUILTIN, SYX_CLOSURE_KIND_LAMBDA, SYX_CLOSURE_KIND_NATIVE_CONSTRUCTOR
- Syx_Closure_Special_Form: { Syx_Closure_Kind kind = SYX_CLOSURE_KIND_SPECIALF; Syx_Special_Form callback }
- Syx_Closure_Builtin: { Syx_Closure_Kind kind = SYX_CLOSURE_KIND_BUILTIN; Syx_Builtins callback }
- Syx_Closure_Lambda: { Syx_Closure_Kind kind = SYX_CLOSURE_KIND_LAMBDA; ... }
- Syx_Closure_Native_Constructor: { Syx_Closure_Kind kind = SYX_CLOSURE_KIND_NATIVE_CONSTRUCTOR; Syx_Type* type }
- Syx_Closure: Syx_Closure_Special_Form*, Syx_Closure_Builtin*, Syx_Closure_Lambda*, Syx_Closure_Native_Constructor*
- ...
