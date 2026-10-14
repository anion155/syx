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

#include <cli.h>

#define FLAG_IMPL
#include <flag.h>
#define HT_IMPL
#include <ht.h>
#define RC_IMPL
#include <rc.h>
#define FS_IMPL
#include <fs.h>

#define SYX_VALUE_IMPL
#include <syx/syx_value.h>
#define SYX_EVAL_IMPL
#include <syx/syx_eval.h>
#define SYX_GLOBAL_ENV_IMPL
#include <syx/syx_global_env.h>
#define SYX_PARSER_IMPL
#include <syx/syx_parser.h>

#define STR_IMPL
#include <str.h>

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

Syx_Value *syx_parse_and_eval_main(Syx_Eval_Ctx *eval_ctx, String_View source) {
  // do {
  //   token = syx_parser_next_token(&ctx);
  //   printf("kind = %d; line = %zu; column = %zu; count = %zu; text = '%.*s'\n", token.kind, token.line, token.column, token.count, (int)token.count, token.data);
  // } while (token.kind != SYX_PARSER_TOKEN_KIND_EOF);
  Syx_Value *expressions = rc_acquire(parse_syx(eval_ctx->env, source, true));
  syx_list_for_each(expressions->pair, expression) {
    if (expression->kind == SYX_VALUE_KIND_EXIT) {
      printf(CLI_BG_RED CLI_FG_WHITE "Parser exception:" CLI_RESET CLI_DIM " ");
      switch (expression->exit->kind) {
        case SYX_EXIT_KIND_RETURNED: printf("unexpected return value"); break;
        case SYX_EXIT_KIND_THROWN: printf(SV_FMT, sv_fmt_arg(*expression->exit->thrown->reason->string)); break;
      }
      printf("\n" CLI_RESET);
      return syx_value_nil();
    }
  }
  Syx_Value *result = NULL;
  syx_list_for_each(expressions->pair, expression) {
    if (script_ctx.opt_xtrace) {
      printf(CLI_DIM ">");
      printf_stringify(sb_append_syx_value, expression);
      printf("\n" CLI_RESET);
    }
    if (result) rc_release(result);
    result = rc_acquire(syx_eval(eval_ctx, expression));
    if (result->kind == SYX_VALUE_KIND_EXIT) {
      switch (result->exit->kind) {
        case SYX_EXIT_KIND_RETURNED: {
          Syx_Value *value = rc_acquire(result->exit->returned);
          rc_release(result);
          return rc_move(value);
        }
        case SYX_EXIT_KIND_THROWN: {
          printf(CLI_BG_RED CLI_FG_WHITE "Unhandled exception:" CLI_RESET CLI_DIM " ");
          printf(SV_FMT, sv_fmt_arg(*result->exit->thrown->reason->string));
          printf("\n" CLI_RESET);
          if (script_ctx.opt_error) return rc_move(result);
          continue;
        }
      }
    }
    if (script_ctx.opt_xtrace) {
      printf_stringify(sb_append_syx_value, result);
      printf("\n");
    }
  }
  if (!script_ctx.opt_xtrace && script_ctx.opt_print && result && !(result->kind == SYX_VALUE_KIND_EXIT && result->exit->kind == SYX_EXIT_KIND_THROWN)) {
    printf_stringify(sb_append_syx_value, result);
    printf("\n");
  }
  if (!result) return syx_value_nil();
  return rc_move(result);
}

int run_syx(String_View source_sv) {
  Syx_Value *result = rc_acquire(syx_parse_and_eval_main(script_ctx.eval_ctx, source_sv));
  if (!result || result->kind != SYX_VALUE_KIND_EXIT || result->exit->kind != SYX_EXIT_KIND_RETURNED) {
    rc_release(result);
    return -1;
  }
  syx_integer_t code = 0;
  Syx_Value *converted = rc_acquire(syx_convert_to_number(script_ctx.eval_ctx, result));
  if (converted->kind == SYX_VALUE_KIND_NUMBER) code = syx_number_get(converted->number);
  else if (converted->kind == SYX_VALUE_KIND_EXIT && converted->exit->kind == SYX_EXIT_KIND_THROWN) code = 1;
  else code = 0;
  rc_release(result);
  rc_release(converted);
  return code;
}

Syx_Value *eval_quit(Syx_Eval_Ctx *ctx, Syx_Pair *arguments) {
  UNUSED(ctx);
  UNUSED(arguments);
  TODO("TASK(20260913-072941): eval_quit");
  // SyxV *result = syxv_list_next(&arguments);
  // if (result->kind == SYXV_KIND_NIL) result = make_syxv_integer(0);
}

Syx_Value *eval_setopt(Syx_Eval_Ctx *ctx, Syx_Pair *arguments) {
  Syx_Value *name = syx_list_next(&arguments);
  SYX_EVAL_ASSERT(ctx, name->kind == SYX_VALUE_KIND_SYMBOL, "option name expected");
  bool **option = ht_find(ctx_options(), name->symbol->data);
  SYX_EVAL_ASSERT(ctx, option, "option not found");
  Syx_Value *evaluated = rc_acquire(syx_eval(ctx, syx_list_next(&arguments)));
  syx_convert_to_storage(ctx, evaluated, *option);
  return NULL;
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

  Fs_Path working_dir = fs_get_cwd();
  script_ctx.eval_ctx = rc_acquire(make_global_syx_eval_ctx(working_dir));

  syx_env_define_value_cstr(script_ctx.eval_ctx->env->parent, "quit", make_syx_value_closure_builtin(NULL, eval_quit));
  syx_env_define_value_cstr(script_ctx.eval_ctx->env->parent, "setopt", make_syx_value_closure_builtin(NULL, eval_setopt));

  int result = 0;
  if (commands->count) {
    // Run some commands, get output from them
    // `syx -c '(do-something)' -c '(do-other)'`
    String_Builder sb = {0};
    for (size_t index = 0; index < commands->count; index += 1) {
      sb_append_cstr(&sb, commands->items[index]);
    }
    sb_append(&sb, 0);
    int run_result = run_syx(sv_from_like(sb));
    if (run_result >= 0) return run_result;
  } else if (*opt_stdin) {
    // Run some commands from stdin, get output from them
    // `cat script.syx | syx -s`
    String_Builder sb = {0};
    if (!fs_read_from_stdin(&sb)) UNREACHABLE("Failed to read stdin");
    sb_append(&sb, 0);
    int run_result = run_syx(sv_from_like(sb));
    sb_free(&sb);
    if (run_result >= 0) return run_result;
  } else if (argc == 1) {
    // Run script
    // `syx script.syx`
    String_Builder sb = {0};
    Fs_Path script_path = fs_path_parse_cstr(argv[0]);
    script_ctx.eval_ctx->cwd.count = 0;
    fs_path_append_path(&script_ctx.eval_ctx->cwd, fs_path_dir_name(script_path));
    if (!fs_read_from_file(&sb, script_path)) UNREACHABLE("Failed to read file");
    sb_append_null(&sb);
    int run_result = run_syx(sv_from_like(sb));
    sb_free(&sb);
    fs_path_free(&script_path);
    if (run_result >= 0) return run_result;
  } else {
    // REPL
    // `syx`
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
      if (run_result >= 0) return run_result;
    }
  }

  return result;
  UNUSED(ctx_options);
}
