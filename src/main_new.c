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

#define SYX_VALUE_IMPL
#include <syx_new/syx_value.h>
#define SYX_EVAL_IMPL
#include <syx_new/syx_eval.h>
#define SYX_GLOBAL_ENV_IMPL
#include <syx_new/syx_global_env.h>
#define SYX_PARSER_IMPL
#include <syx_new/syx_parser.h>

typedef struct Syx_Script_Context {
  Syx_Eval_Ctx *eval_ctx;
  bool opt_xtrace;
  bool opt_print;
  bool opt_error;
} Syx_Script_Context;

Syx_Script_Context script_ctx = {0, .opt_error = true};

syx_define_constant(Ht(const char *, bool *), ctx_options) {
  ctx_options->hasheq = ht_cstr_hasheq;
  *ht_put(ctx_options, "x") = &script_ctx.opt_xtrace;
  *ht_put(ctx_options, "p") = &script_ctx.opt_print;
  *ht_put(ctx_options, "e") = &script_ctx.opt_error;
}

Syx_Value *syx_parse_and_eval(Syx_Eval_Ctx *eval_ctx, String_View source) {
  UNUSED(eval_ctx);
  // Syx_Tokens tokens = syx_lexer_tokenize(source);
  // da_foreach(Syx_Token, token, &tokens) {
  //   printf("%s: '%.*s'\n", syx_token_kind_string(token->kind), (int)token->count, token->data);
  // }

  Syx_Value *values = parse_syx(source);
  syx_list_for_each(values->pair, value) {
    printf("%.*s\n", (int)value->string->count, value->string->data);
  }

  // SyxV_Parser_Context ctx = {.source = source_sv};
  // Syx_Parser_Token token;
  return NULL;
  // do {
  //   token = syx_parser_next_token(&ctx);
  //   printf("kind = %d; line = %zu; column = %zu; count = %zu; text = '%.*s'\n", token.kind, token.line, token.column, token.count, (int)token.count, token.data);
  // } while (token.kind != SYX_PARSER_TOKEN_KIND_EOF);
  // TODO("syx_parse_and_eval");
  // SyxV *expressions = parse_multiple_syxv(source_sv);
  // if (!syx_parser_report_error(expressions)) return make_syxv_nil();
  // SyxV *result = NULL;
  // syxv_list_for_each(expression, expressions) {
  //   if (script_ctx.opt_xtrace) {
  //     printf(CLI_DIM ">");
  //     printf_with(str_append_syxv, expression);
  //     printf("\n" CLI_RESET);
  //   }
  //   if (result) rc_release(result);
  //   result = rc_acquire(syx_eval(ctx, expression));
  //   if (result->kind == SYXV_KIND_RETURN_VALUE) {
  //     SyxV *value = rc_acquire(result->return_value);
  //     rc_release(result);
  //     return rc_move(value);
  //   }
  //   if (!syx_eval_report_error(ctx, result)) {
  //     if (script_ctx.opt_error) return rc_move(result);
  //   } else if (script_ctx.opt_xtrace) {
  //     printf_with(str_append_syxv, result);
  //     printf("\n");
  //   }
  // }
  // if (!script_ctx.opt_xtrace && script_ctx.opt_print && result && result->kind != SYXV_KIND_THROWN) {
  //   printf_with(str_append_syxv, result);
  //   printf("\n");
  // }
  // if (!result) return make_syxv_nil();
  // return rc_move(result);
}

int run_syx(String_View source_sv) {
  Syx_Value *result = rc_acquire(syx_parse_and_eval(script_ctx.eval_ctx, source_sv));
  if (!result || result->kind != SYX_VALUE_KIND_EXIT || result->exit->kind != SYX_EXIT_KIND_RETURNED) {
    rc_release(result);
    return -1;
  }
  syx_integer_t code = 0;
  if (result->kind == SYX_VALUE_KIND_NUMBER) {
    switch (result->number->kind) {
      case SYX_NUMBER_KIND_INTEGER: code = result->number->integer; break;
      case SYX_NUMBER_KIND_FRACTIONAL: code = result->number->fractional; break;
    }
  }
  // SyxV *converted = rc_acquire(syx_convert_to_number(script_ctx.eval_ctx, result));
  // if (converted->kind == SYXV_KIND_NUMBER) code = syx_number_integer_value(converted->number);
  // else if (converted->kind == SYXV_KIND_THROWN) code = 1;
  // else code = 0;
  rc_release(result);
  // rc_release(converted);
  return code;
}

// SyxV *eval_quit(Syx_Eval_Ctx *ctx, SyxV *arguments) {
//   UNUSED(ctx);
//   UNUSED(arguments);
//   TODO("eval_quit");
//   // SyxV *result = syxv_list_next(&arguments);
//   // if (result->kind == SYXV_KIND_NIL) result = make_syxv_integer(0);
// }

// SyxV *eval_setopt(Syx_Eval_Ctx *ctx, Syx_SpecialF *callable, SyxV *arguments) {
//   UNUSED(callable);
//   SyxV *name = syxv_list_next(&arguments);
//   if (name->kind != SYXV_KIND_SYMBOL) RUNTIME_ERROR(ctx, "option name expected");
//   bool **option = ht_find(ctx_options(), name->symbol.name);
//   if (option == NULL) RUNTIME_ERROR(ctx, "option not found");
//   SyxV *evaluated = syx_eval(ctx, syxv_list_next(&arguments));
//   bool value = {0};
//   syx_convert_to(ctx, evaluated, &value);
//   (**option) = value;
//   return NULL;
// }

// SyxV *eval_import(Syx_Eval_Ctx *ctx, Syx_SpecialF *callable, SyxV *arguments) {
//   UNUSED(callable);
//   SyxV *name = syx_eval(ctx, syxv_list_next(&arguments));
//   if (name->kind != SYXV_KIND_STRING) RUNTIME_ERROR(ctx, "module name expected");
//   String_Builder module_sb = {0};
//   if (!nob_read_entire_file(name->string.data, &module_sb)) UNREACHABLE("Failed to read file");
//   sb_append(&module_sb, 0);
//   module_sb.items = rc_acquire(rc_manage(module_sb.items, module_sb.count));
//   syx_ctx_push_frame(ctx, "import");
//   Syx_Eval_Ctx *import_ctx = rc_acquire(inherit_syx_eval_ctx(ctx, .env = syx_env_global(ctx->env)));
//   SyxV *result = rc_acquire(syx_parse_and_eval(import_ctx, sb_to_sv(module_sb)));
//   syx_ctx_pop_frame(ctx, result);
//   syx_eval_early_exit(result, module_sb.items, import_ctx);
//   // TODO: implement exports from module
//   rc_release(result);
//   rc_release(import_ctx);
//   return make_syxv_nil();
// }

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
  bool *opt_error = flag_bool("e", true, "Should stop on unhandled error");
  bool *opt_stdin = flag_bool("s", false, "Execute script from stdin");
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
  if (*opt_xtrace) script_ctx.opt_xtrace = true;
  if (*opt_print) script_ctx.opt_print = true;
  if (!(*opt_error)) script_ctx.opt_error = false;

  script_ctx.eval_ctx = rc_acquire(make_global_syx_eval_ctx());

  // syx_env_define_cstr(script_ctx.eval_ctx->global_env, "quit", make_syxv_builtin(NULL, eval_quit));
  // syx_env_define_cstr(script_ctx.eval_ctx->global_env, "setopt", make_syxv_specialf(NULL, eval_setopt));
  // syx_env_define_cstr(script_ctx.eval_ctx->global_env, "import", make_syxv_specialf(NULL, eval_import));

  int result = 0;
  if (commands->count) {
    String_Builder sb = {0};
    da_foreach(const char *, command, commands) sb_append_cstr(&sb, *command);
    sb_append(&sb, 0);
    int run_result = run_syx(sb_to_sv(sb));
    if (run_result >= 0) nob_return_defer(run_result);
  } else if (*opt_stdin) {
    String_Builder sb = {0};
    if (!nob_read_entire_stdin(&sb)) UNREACHABLE("Failed to read stdin");
    sb_append(&sb, 0);
    int run_result = run_syx(sb_to_sv(sb));
    sb_free(sb);
    if (run_result >= 0) nob_return_defer(run_result);
  } else if (argc == 1) {
    String_Builder sb = {0};
    if (!nob_read_entire_file(argv[0], &sb)) UNREACHABLE("Failed to read file");
    sb_append(&sb, 0);
    int run_result = run_syx(sb_to_sv(sb));
    sb_free(sb);
    if (run_result >= 0) nob_return_defer(run_result);
  } else {
    printf("Syx Language REPL\n");
    read_history(HIST_FILE);
    char *line;
    while ((line = readline("> ")) != NULL) {
      if (!line) continue;
      if (*line) {
        add_history(line);
        write_history(HIST_FILE);
      }
      int run_result = run_syx(sv_from_cstr(line));
      free(line);
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
