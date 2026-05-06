#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef __APPLE__
#  include <editline/readline.h>
#else
#  include <readline/history.h>
#  include <readline/readline.h>
#endif

#define NOB_IMPL
#include <nob.h>
#define FLAG_IMPL
#include <flag.h>
#define HT_IMPL
#include <ht.h>
#define RC_IMPL
#include <cli.h>
#include <rc.h>

#define SYX_PARSER_IMPL
#include "syx_parser.h"
#define SYX_VALUE_IMPL
#include "syx_value.h"
#define SYX_EVAL_IMPL
#include "syx_eval.h"

typedef struct Syx_Script_Context {
  Syx_Eval_Ctx *eval_ctx;
  bool opt_xtrace;
  bool opt_print;
} Syx_Script_Context;

Syx_Script_Context script_ctx = {0};

define_constant(Ht(const char *, bool *), ctx_options) {
  ctx_options->hasheq = ht_cstr_hasheq;
  *ht_put(ctx_options, "x") = &script_ctx.opt_xtrace;
  *ht_put(ctx_options, "p") = &script_ctx.opt_print;
}

SyxV *syx_parse_and_eval(Syx_Eval_Ctx *ctx, const char *source_cstr) {
  SyxV *result = NULL;
  parser_syxvs_for_each(value, source_cstr) {
    if (script_ctx.opt_xtrace) {
      printf(CLI_DIM ">");
      print_syxv(value);
      printf("\n" CLI_RESET);
    }
    if (result) rc_release(result);
    result = rc_acquire(syx_eval(ctx, value));
    if (result->kind == SYXV_KIND_RETURN_VALUE) {
      SyxV *return_value = rc_acquire(result->return_value);
      rc_release(result);
      return rc_move(return_value);
    }
    if (!syx_eval_report_error(ctx, result)) {
      rc_release(parser_syxvs_for_each_iterator());
      return rc_move(result);
    }
    if (script_ctx.opt_xtrace) {
      print_syxv(result);
      printf("\n");
    }
  }
  if (!script_ctx.opt_xtrace && script_ctx.opt_print && result) {
    print_syxv(result);
    printf("\n");
  }
  if (!result) return make_syxv_nil();
  return rc_move(result);
}

int run_syx(const char *source_cstr) {
  SyxV *result = rc_acquire(syx_parse_and_eval(script_ctx.eval_ctx, source_cstr));
  if (result->kind == SYXV_KIND_RETURN_VALUE) {
    int code = syx_convert_to_integer_v(script_ctx.eval_ctx, result);
    rc_release(result);
    return code;
  }
  rc_release(result);
  return -1;
}

SyxV *eval_quit(Syx_Eval_Ctx *ctx, SyxV *arguments) {
  UNUSED(ctx);
  UNUSED(arguments);
  TODO("eval_quit");
  // SyxV *result = syxv_list_next(&arguments);
  // if (result->kind == SYXV_KIND_NIL) result = make_syxv_integer(0);
}

SyxV *eval_setopt(Syx_Eval_Ctx *ctx, Syx_SpecialF *callable, SyxV *arguments) {
  UNUSED(callable);
  SyxV *name = syxv_list_next(&arguments);
  if (name->kind != SYXV_KIND_SYMBOL) RUNTIME_ERROR("option name expected", ctx);
  bool **option = ht_find(ctx_options(), name->symbol.name);
  if (option == NULL) RUNTIME_ERROR("option not found", ctx);
  SyxV *value = syx_eval(ctx, syxv_list_next(&arguments));
  (**option) = syx_convert_to_bool_v(ctx, value);
  return NULL;
}

SyxV *eval_import(Syx_Eval_Ctx *ctx, Syx_SpecialF *callable, SyxV *arguments) {
  SyxV *name = syx_eval(ctx, syxv_list_next(&arguments));
  if (name->kind != SYXV_KIND_STRING) RUNTIME_ERROR("module name expected", ctx);
  String_Builder module_sb = {0};
  if (!nob_read_entire_file(name->string.data, &module_sb)) UNREACHABLE("Failed to read file");
  sb_append(&module_sb, 0);
  char *module_content = module_sb.items;
  module_sb.items = rc_acquire(rc_manage_strndup(module_content, module_sb.count));
  free(module_content);
  syx_ctx_push_frame(ctx, (SyxV *)((char *)callable - offsetof(SyxV, specialf)));
  Syx_Eval_Ctx *import_ctx = rc_acquire(inherit_syx_eval_ctx(ctx, .env = syx_env_global(ctx->env)));
  SyxV *result = syx_parse_and_eval(import_ctx, module_sb.items);
  // TODO: implement exports from module
  rc_release(result);
  rc_release(import_ctx);
  syx_ctx_pop_frame(ctx);
  syx_eval_early_exit(result, module_sb.items);
  return make_syxv_nil();
}

void usage(FILE *stream) {
  fprintf(stream, "usage: %s [options] [file]\n", flag_program_name());
  fprintf(stream, "modes:\n");
  fprintf(stream, "  %-16s  %s\n", "syx", "Starts the interactive REPL.");
  fprintf(stream, "  %-16s  %s\n", "syx <file.syx>", "Executes the specified script file.");
  fprintf(stream, "  %-16s  %s\n", "syx -c \"expr\"", "Executes the provided syx string.");
  fprintf(stream, "options:\n");
  flag_print_options(stream);
}

#define HIST_FILE "history.txt"

int main(int argc, char **argv) {
  srand(time(NULL));

  bool *opt_xtrace = flag_bool("x", false, "Print every expression before evaluation");
  bool *opt_print = flag_bool("p", false, "Print result of last evaluation");
  Flag_List *commands = flag_list("c", "Commands to run");
  bool *help = flag_bool("h", false, "Show this help message");
  if (!flag_parse(argc, argv)) {
    flag_print_error(stderr);
    usage(stderr);
    exit(1);
  }
  if (*help) {
    usage(stdout);
    exit(0);
  }
  argc = flag_rest_argc();
  argv = flag_rest_argv();

  if (commands->count) script_ctx.opt_print = true;
  else if (argc == 1) script_ctx.opt_print = false;
  else script_ctx.opt_print = true;

  if (*opt_xtrace) script_ctx.opt_xtrace = *opt_xtrace;
  else if (*opt_print) script_ctx.opt_print = true;

  script_ctx.eval_ctx = make_global_syx_eval_ctx();

  syx_env_define_cstr(script_ctx.eval_ctx->env, "quit", make_syxv_builtin(NULL, eval_quit));
  syx_env_define_cstr(script_ctx.eval_ctx->env, "setopt", make_syxv_specialf(NULL, eval_setopt));
  syx_env_define_cstr(script_ctx.eval_ctx->env, "import", make_syxv_specialf(NULL, eval_import));

  int result = 0;
  if (commands->count) {
    String_Builder sb = {0};
    da_foreach(const char *, command, commands) sb_append_cstr(&sb, *command);
    sb_append(&sb, 0);
    int run_result = run_syx(sb.items);
    if (run_result >= 0) nob_return_defer(run_result);
  } else if (argc == 1) {
    String_Builder sb = {0};
    if (!nob_read_entire_file(argv[0], &sb)) UNREACHABLE("Failed to read file");
    sb_append(&sb, 0);
    int run_result = run_syx(sb.items);
    sb_free(sb);
    if (run_result >= 0) nob_return_defer(run_result);
  } else {
    printf("Syx Language REPL\n");
    read_history(HIST_FILE);
    char *line_ptr;
    while ((line_ptr = readline("> ")) != NULL) {
      if (line_ptr && *line_ptr) {
        add_history(line_ptr);
        write_history(HIST_FILE);
      }
      int run_result = run_syx(line_ptr);
      free(line_ptr);
      if (run_result >= 0) nob_return_defer(run_result);
    }
  }

defer:
  rc_release(script_ctx.eval_ctx);
  // ht_free(ctx_options());
  // ht_free(FD_CONSTANTS());
  // ht_free(SYXV_CONSTANTS());
  // ht_free(SYXV_SYMBOLS());
  return result;
}
