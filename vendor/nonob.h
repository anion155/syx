#ifndef NONOB_H
#define NONOB_H

#include "./jim.h"
#include "./nob.h"

void initialize_context();
void deinitialize_context();

void default_usage(FILE *stream);

void append_cmd_to_cdb();

struct ContextPaths;
struct ContextFlags;

struct Context {
  const char *root;
  struct ContextPaths paths;
  Nob_Cmd cmd;
  Nob_Procs procs;
  Jim cdb_jim;
  void (*usage)(FILE *stream);
  bool flag_help;
  bool flag_ccjson;
  struct ContextFlags flags;
  int argc;
  char **argv;
} ctx = {.cdb_jim = {.pp = 2}, .usage = default_usage};

#endif // NONOB_H

#if defined(NONOB_IMPL) && !defined(NONOB_IMPL_C)
#define NONOB_IMPL_C

#include <libgen.h>
#include <string.h>
#if defined(__APPLE__)
#include <mach-o/dyld.h>
#endif

#define JIM_IMPL
#include "./jim.h"
#define NOB_IMPL
#include "./nob.h"
#define FLAG_IMPL
#include "./flag.h"

char *get_exe_path() {
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
  return temp_strdup(exe_path);
}

void initialize_context() {
  ctx.root = dirname(get_exe_path());

  flag_bool_var(&ctx.flag_ccjson, "ccjson", false, "Creates compile_commands.json");
  flag_bool_var(&ctx.flag_help, "help", false, "Print this help to stdout and exit with 0");
}

void flags_parse() {
  if (!flag_parse(ctx.argc, ctx.argv)) {
    ctx.usage(stderr);
    flag_print_error(stderr);
    exit(1);
  }
  if (ctx.flag_help) {
    ctx.usage(stdout);
    exit(0);
  }
  ctx.argc = flag_rest_argc();
  ctx.argv = flag_rest_argv();

  if (ctx.flag_ccjson) jim_array_begin(&ctx.cdb_jim);
}

void deinitialize_context() {
  if (ctx.flag_ccjson) {
    jim_array_end(&ctx.cdb_jim);
    nob_write_entire_file(
        temp_sprintf("%s/compile_commands.json", ctx.root),
        ctx.cdb_jim.sink,
        ctx.cdb_jim.sink_count);
  }
}

void default_usage(FILE *stream) {
  fprintf(stream, "usage: ./nob [<options>] [--]\n");
  fprintf(stream, "options:\n");
  flag_print_options(stream);
}

void append_cmd_to_cdb() {
  if (!ctx.flag_ccjson) return;
  jim_object_begin(&ctx.cdb_jim);

  /** The working directory of the compilation. All paths specified in the
   * command or file fields must be either absolute or relative to this directory. */
  jim_member_key(&ctx.cdb_jim, "directory");
  jim_string(&ctx.cdb_jim, ctx.root);

  /** The main translation unit source processed by this compilation step.
   * This is used by tools as the key into the compilation database.
   * There can be multiple command objects for the same file, for example
   * if the same source file is compiled with different configurations. */
  jim_member_key(&ctx.cdb_jim, "file");
  if (strstr(ctx.cmd.items[0], "cc")) {
    bool output = false;
    for (const char **arg = ctx.cmd.items + 1; arg < ctx.cmd.items + ctx.cmd.count; ++arg) {
      if (*arg[0] == '-') {
        if (strcmp(*arg, "-o")) output = true;
      } else {
        jim_string(&ctx.cdb_jim, *arg);
        break;
      }
      output = false;
    }
  }

  /** The compile command argv as list of strings. This should run the
   * compilation step for the translation unit file. arguments[0] should be
   * the executable name, such as clang++. Arguments should not be escaped,
   * but ready to pass to execvp(). */
  jim_member_key(&ctx.cdb_jim, "arguments");
  jim_array_begin(&ctx.cdb_jim);
  da_foreach(const char *, arg, &ctx.cmd) {
    jim_string(&ctx.cdb_jim, *arg);
  }
  jim_array_end(&ctx.cdb_jim);

  /** The compile command as a single shell-escaped string. Arguments may be
   * shell quoted and escaped following platform conventions, with ‘"’ and ‘\’
   * being the only special characters. Shell expansion is not supported. */
  // jim_member_key(&ctx.cdb_jim, "command");
  // jim_string(&ctx.cdb_jim, ROOT_PATH);

  /** Either arguments or command is required. arguments is preferred,
   * as shell (un)escaping is a possible source of errors. */

  /** The name of the output created by this compilation step.
   * This field is optional. It can be used to distinguish different
   * processing modes of the same input file. */
  // jim_member_key(&ctx.cdb_jim, "output");
  // jim_string(&ctx.cdb_jim, ROOT_PATH);

  jim_object_end(&ctx.cdb_jim);
}

#endif // NONOB_IMPL
