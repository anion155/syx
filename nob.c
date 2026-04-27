#include <string.h>
#include <libgen.h>
#if defined(__APPLE__)
#include <mach-o/dyld.h>
#endif

#define FLAG_IMPL
#include "./vendor/flag.h"
#define NOB_IMPL
#include "./vendor/nob.h"
#define JIM_IMPL
#include "./vendor/jim.h"

typedef struct Context {
  struct {
    const char *root;
    const char *src;
    const char *vendor;
    const char *build;
  } paths;
  Nob_Cmd cmd;
  Nob_Procs procs;
  Jim cdb_jim;
  struct {
    bool cdb;
    bool run;
    bool debug;
  } flags;
  int argc;
  char **argv;
} Context;
bool init_context(Context *ctx) {
  char exe_path[PATH_MAX + 1];
#if defined(__APPLE__)
  {
    char link_path[PATH_MAX];
    uint32_t size = sizeof(link_path);
    if (_NSGetExecutablePath(link_path, &size) != 0) return false;
    if (realpath(link_path, exe_path) == NULL) return false;
  }
#elif defined(__linux__)
  {
    ssize_t count = readlink("/proc/self/exe", exe_path, PATH_MAX);
    printf("%d\n", count);
    if (count == 0) return false;
    exe_path[count] = 0;
  }
#endif
  ctx->paths.root = dirname(exe_path);
  ctx->paths.src = temp_sprintf("%s/src", ctx->paths.root);
  ctx->paths.vendor = temp_sprintf("%s/vendor", ctx->paths.root);
  ctx->paths.build = temp_sprintf("%s/build", ctx->paths.root);

  flag_bool_var(&ctx->flags.cdb,   "cdb", false, "Creates compile_commands.json");
  flag_bool_var(&ctx->flags.run,   "run", false, "Run result, when provided arguments after '--'");
  flag_bool_var(&ctx->flags.debug, "g",   false, "Build with debugger support");

  return true;
}

void usage(FILE *stream);
void flag_parse_with_help(Context *ctx);
bool cc_run_opt(Context *ctx, Nob_Cmd_Opt opt);
#define cc_run(ctx, ...) cc_run_opt((ctx), (Nob_Cmd_Opt){ __VA_ARGS__ })

bool build(Context *ctx) {
  if (!nob_mkdir_if_not_exists(ctx->paths.build)) return false;

  nob_cc(&ctx->cmd);
  nob_cc_flags(&ctx->cmd);
  nob_cmd_append(&ctx->cmd, temp_sprintf("-I%s", ctx->paths.src));
  nob_cmd_append(&ctx->cmd, temp_sprintf("-I%s", ctx->paths.vendor));
  if (ctx->flags.debug) nob_cmd_append(&ctx->cmd, "-ggdb");
  // if (!ctx->flags.debug) nob_cmd_append(&ctx->cmd, "-fsanitize=address");
  nob_cc_inputs(&ctx->cmd, "-std=c23");
  #ifdef __APPLE__
    nob_cmd_append(&ctx->cmd, "-ledit");
  #else
    nob_cmd_append(&cmd, "-lreadline");
  #endif
  nob_cc_inputs(&ctx->cmd, temp_sprintf("%s/main.c", ctx->paths.src));
  nob_cc_output(&ctx->cmd, temp_sprintf("%s/sexpr", ctx->paths.build));
  if (!cc_run(ctx)) return false;

  if (ctx->flags.run) {
    if (ctx->flags.debug) nob_cmd_append(&ctx->cmd, "lldb");
    nob_cmd_append(&ctx->cmd, temp_sprintf("%s/sexpr", ctx->paths.build));
    if (ctx->argc) nob_da_append_many(&ctx->cmd, ctx->argv, ctx->argc);
    if (!nob_cmd_run(&ctx->cmd)) return false;
  }

  return true;
}

int main(int argc, char **argv) {
  NOB_GO_REBUILD_URSELF(argc, argv);
  Context ctx = {.argc = argc, .argv = argv, .cdb_jim = {.pp = 2}};
  init_context(&ctx);
  flag_parse_with_help(&ctx);
  if (ctx.flags.cdb) jim_array_begin(&ctx.cdb_jim);
  bool result = build(&ctx);
  if (ctx.flags.cdb) {
    jim_array_end(&ctx.cdb_jim);
    nob_write_entire_file(
      temp_sprintf("%s/compile_commands.json", ctx.paths.root),
      ctx.cdb_jim.sink,
      ctx.cdb_jim.sink_count
    );
  }
  if (!result) return 1;
  return 0;
}

void usage(FILE *stream) {
  fprintf(stream, "usage: ./nob [<options>] [--]\n");
  fprintf(stream, "options:\n");
  flag_print_options(stream);
}

void flag_parse_with_help(Context *ctx) {
  bool *flag_help = flag_bool("help", false, "Print this help to stdout and exit with 0");
  if (!flag_parse(ctx->argc, ctx->argv)) {
    usage(stderr);
    flag_print_error(stderr);
    exit(1);
  }
  if (*flag_help) {
    usage(stdout);
    exit(0);
  }
  ctx->argc = flag_rest_argc();
  ctx->argv = flag_rest_argv();
}

bool cc_run_opt(Context *ctx, Nob_Cmd_Opt opt) {
  if (ctx->flags.cdb) {
    jim_object_begin(&ctx->cdb_jim);

    /** The working directory of the compilation. All paths specified in the
     * command or file fields must be either absolute or relative to this directory. */
    jim_member_key(&ctx->cdb_jim, "directory");
    jim_string(&ctx->cdb_jim, ctx->paths.root);

    /** The main translation unit source processed by this compilation step.
     * This is used by tools as the key into the compilation database.
     * There can be multiple command objects for the same file, for example
     * if the same source file is compiled with different configurations. */
    jim_member_key(&ctx->cdb_jim, "file");
    if (strstr(ctx->cmd.items[0], "cc")) {
      bool output = false;
      for (const char **arg = ctx->cmd.items + 1; arg < ctx->cmd.items + ctx->cmd.count; ++arg) {
        if (*arg[0] == '-') {
          if (strcmp(*arg, "-o")) output = true;
        } else {
          jim_string(&ctx->cdb_jim, *arg);
          break;
        }
        output = false;
      }
    }

    /** The compile command argv as list of strings. This should run the
     * compilation step for the translation unit file. arguments[0] should be
     * the executable name, such as clang++. Arguments should not be escaped,
     * but ready to pass to execvp(). */
    jim_member_key(&ctx->cdb_jim, "arguments");
    jim_array_begin(&ctx->cdb_jim);
    da_foreach(const char *, arg, &ctx->cmd) {
      jim_string(&ctx->cdb_jim, *arg);
    }
    jim_array_end(&ctx->cdb_jim);

    /** The compile command as a single shell-escaped string. Arguments may be
     * shell quoted and escaped following platform conventions, with ‘"’ and ‘\’
     * being the only special characters. Shell expansion is not supported. */
    // jim_member_key(&ctx->cdb_jim, "command");
    // jim_string(&ctx->cdb_jim, ROOT_PATH);

    /** Either arguments or command is required. arguments is preferred,
     * as shell (un)escaping is a possible source of errors. */

    /** The name of the output created by this compilation step.
     * This field is optional. It can be used to distinguish different
     * processing modes of the same input file. */
    // jim_member_key(&ctx->cdb_jim, "output");
    // jim_string(&ctx->cdb_jim, ROOT_PATH);

    jim_object_end(&ctx->cdb_jim);
  }
  return nob_cmd_run_opt(&ctx->cmd, opt);
}
