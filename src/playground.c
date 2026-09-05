#include <stdio.h>
#define SV_IMPL
#include <sv.h>
#include <syx_new/syx_eval.h>

int main(void) {
  String_Builder sb = {0};

  // Step 1: Append 256 bytes
  for (int i = 0; i < 256; i++) {
    da_append(&sb, 'A');
  }
  printf("1. Initial:      count=%zu, capacity=%zu\n", sb.count, sb.capacity);

  // Step 2: Trim and null-terminate
  sb_null_terminate(&sb);
  printf("2. Terminated:   count=%zu, capacity=%zu (Allocated memory: %zu bytes)\n",
         sb.count, sb.capacity, sb.capacity);

  // Step 3: Append 1 character ('B')
  da_append(&sb, 'B');
  printf("3. Append 'B':   count=%zu, capacity=%zu\n", sb.count, sb.capacity);

  // Step 4: Append 1 character ('C')
  da_append(&sb, 'C');
  printf("4. Append 'C':   count=%zu, capacity=%zu (Allocated memory: 258 bytes!)\n",
         sb.count, sb.capacity);

  // Step 5: Null-terminate again
  printf("5. Attempting sb_null_terminate...\n");
  sb_null_terminate(&sb);

  printf("Success! count=%zu, capacity=%zu\n", sb.count, sb.capacity);

  free(sb.data);
  return 0;
}
