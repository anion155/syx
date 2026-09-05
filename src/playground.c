#include <stdio.h>
#define SYX_LEXER_IMPL
#include <syx_new/syx_lexer.h>
#define SYX_VALUE_IMPL
#include <syx_new/syx_value.h>
#define SYX_PARSER_IMPL
#include <syx_new/syx_parser.h>
#define SYX_UTILS_IMPL
#include <syx_new/syx_utils.h>
#define SYX_EVAL_BUILTINS_IMPL
#include <syx_new/syx_eval_builtins.h>
#define SYX_EVAL_SPECIALF_IMPL
#include <syx_new/syx_eval_specialf.h>
#define SYX_GLOBAL_ENV_IMPL
#include <syx_new/syx_global_env.h>
#define SYX_EVAL_IMPL
#include <syx_new/syx_eval.h>
#define SYX_OBJECT_IMPL
#include <syx_new/syx_object.h>

int main(void) {
  String_Builder sb = {0};

  // Step 1: Append 256 bytes
  for (int i = 0; i < 256; i++) {
    da_append(&sb, 'A');
  }
  printf("1. Initial:      count=%zu, capacity=%zu\n", sb.count, sb.capacity);

  free(sb.data);
  return 0;
}
