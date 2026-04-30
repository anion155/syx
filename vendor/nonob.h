#ifndef NONOB_H
#define NONOB_H

#include "./flag.h"
#include "./ht.h"
#include "./jim.h"
#include "./magic.h"
#include "./nob.h"

typedef struct NoNob_Command NoNob_Command;

struct NoNob_Command {
  const char *description;
  void *flags;
  void (*init)(NoNob_Command *command);
  bool (*run)();
};

struct NoNob_Context_Storage;

struct NoNob_Context {
  const char *exe_path;
  Nob_Cmd cmd;
  Nob_Procs procs;
  Jim ccjson;
  void (*usage)(FILE *stream, NoNob_Command *command);
  bool flag_help;
  bool flag_no_ccjson;
  Ht(const char *, NoNob_Command) commands;
  int argc;
  char **argv;
  struct NoNob_Context_Storage *s;
} ctx = {.ccjson = {.pp = 2}, .commands = {.hasheq = ht_cstr_hasheq}};

typedef struct NoNob_Initialize_Opt {
  bool disable_ccjson;
} NoNob_Initialize_Opt;

void nonob_initialize_opt(int argc, char **argv, NoNob_Initialize_Opt opt);
#define nonob_initialize(argc, argv, ...) nonob_initialize_opt((argc), (argv), (NoNob_Initialize_Opt){__VA_ARGS__})
void nonob_deinitialize();

void nonob_parse_options();

void nonob__define_command_opt(const char *name, NoNob_Command command);
#define nonob_define_command(name, description_cstr) nonob__define_command_opt( \
    STRINGIFY(name),                                                            \
    (NoNob_Command){                                                            \
        .description = (description_cstr),                                      \
        .flags = flag_c_new(NULL),                                              \
        .init = command_##name##_init,                                          \
        .run = command_##name##_run,                                            \
    })
bool nonob__run_command(const char *default_command);
#define nonob_run_command(...) nonob__run_command(WITH_DEFAULT(NULL, __VA_ARGS__))
void nonob_append_cmd_to_ccjson();

#endif // NONOB_H

#if defined(NONOB_IMPL) && !defined(NONOB_IMPL_C)
#define NONOB_IMPL_C

#include <libgen.h>
#include <string.h>
#if defined(__APPLE__)
#  include <mach-o/dyld.h>
#endif

#define JIM_IMPL
#include "./jim.h"
#define NOB_IMPL
#include "./nob.h"
#define FLAG_IMPL
#include "./flag.h"
#define HT_IMPL
#include "./ht.h"

char *get_exe_path() {
  char exe_path[PATH_MAX + 1];
#if defined(__APPLE__)
  {
    char link_path[PATH_MAX];
    uint32_t size = sizeof(link_path);
    if (_NSGetExecutablePath(link_path, &size) != 0) return "";
    if (realpath(link_path, exe_path) == NULL) return "";
  }
#elif defined(__linux__)
  {
    ssize_t count = readlink("/proc/self/exe", exe_path, PATH_MAX);
    printf("%d\n", count);
    if (count == 0) return "";
    exe_path[count] = 0;
  }
#endif
  return temp_strdup(exe_path);
}

void nonob_default_usage(FILE *stream, NoNob_Command *command) {
  if (command) {
    Flag_Context *flags = command->flags;
    fprintf(stream, "usage: %s %s", flag_program_name(), ht_key(&ctx.commands, command));
    if (flags->flags_count) fprintf(stream, " [options]");
    fprintf(stream, "\n");
    if (flags->flags_count) {
      fprintf(stream, "options:\n");
      flag_c_print_options(command->flags, stream);
    }
  } else {
    fprintf(stream, "usage: %s", flag_program_name());
    if (flag_global_context.flags_count) fprintf(stream, " [options]");
    bool first = true;
    ht_foreach(command, &ctx.commands) {
      const char *name = ht_key(&ctx.commands, command);
      if (first) fprintf(stream, " %s", name);
      else fprintf(stream, "|%s", name);
      first = false;
    }
    fprintf(stream, " \n");
    if (ctx.commands.count) {
      fprintf(stream, "command:\n");
      ht_foreach(command, &ctx.commands) {
        const char *name = ht_key(&ctx.commands, command);
        fprintf(stream, "  %-20s  %s\n", name, command->description);
      }
    }
    if (flag_global_context.flags_count) {
      fprintf(stream, "options:\n");
      flag_print_options(stream);
    }
  }
}

void nonob_initialize_opt(int argc, char **argv, NoNob_Initialize_Opt opt) {
  ctx.argc = argc;
  ctx.argv = argv;
  ctx.exe_path = dirname(get_exe_path());
  ctx.s = temp_alloc(sizeof(struct NoNob_Context_Storage));

  if (!opt.disable_ccjson) flag_bool_var(&ctx.flag_no_ccjson, "no-ccjson", false, "Disable creation of compile_commands.json");
  else ctx.flag_no_ccjson = true;
  flag_bool_var(&ctx.flag_help, "help", false, "Print this help to stdout and exit with 0");
}

void nonob_deinitialize() {
  if (!ctx.flag_no_ccjson) {
    jim_array_end(&ctx.ccjson);
    nob_write_entire_file(
        temp_sprintf("%s/compile_commands.json", ctx.exe_path),
        ctx.ccjson.sink,
        ctx.ccjson.sink_count);
  }
}

void nonob_parse_options() {
  if (!flag_parse(ctx.argc, ctx.argv)) {
    flag_print_error(stderr);
    if (ctx.usage) ctx.usage(stderr, NULL);
    else nonob_default_usage(stderr, NULL);
    exit(1);
  }
  if (ctx.flag_help) {
    if (ctx.usage) ctx.usage(stdout, NULL);
    else nonob_default_usage(stdout, NULL);
    exit(0);
  }
  ctx.argc = flag_rest_argc();
  ctx.argv = flag_rest_argv();
  ht_foreach(command, &ctx.commands) {
    flag_c_set_program_name(command->flags, flag_program_name());
  }

  if (!ctx.flag_no_ccjson) jim_array_begin(&ctx.ccjson);
}

void nonob__define_command_opt(const char *name, NoNob_Command command) {
  *ht_put(&ctx.commands, name) = command;
}

bool nonob__run_command(const char *default_command) {
  const char *name = default_command;
  if (ctx.argc > 0) {
    name = ctx.argv[0];
    flag_shift_args(&ctx.argc, &ctx.argv);
  }
  NoNob_Command *command = !name ? NULL : ht_find(&ctx.commands, name);
  if (!command) {
    if (ctx.usage) ctx.usage(stderr, NULL);
    else nonob_default_usage(stderr, NULL);
    fprintf(stderr, "ERROR: %s: unknown command\n", name);
    exit(1);
  }
  Flag_Context *flags = command->flags;
  if (command->init) command->init(command);
  if (ctx.argc && flags->flags_count) {
    bool *help = flag_c_bool(flags, "help", false, "Print this help to stdout and exit with 0");
    if (!flag_c_parse(flags, ctx.argc, ctx.argv)) {
      flag_c_print_error(flags, stderr);
      ctx.usage(stderr, command);
      exit(1);
    }
    if (*help) {
      if (ctx.usage) ctx.usage(stdout, command);
      else nonob_default_usage(stdout, command);
      exit(0);
    }
    ctx.argc = flag_c_rest_argc(flags);
    ctx.argv = flag_c_rest_argv(flags);
  }

  return command->run();
}

void nonob_append_cmd_to_ccjson() {
  if (ctx.flag_no_ccjson) return;
  jim_object_begin(&ctx.ccjson);

  /** The working directory of the compilation. All paths specified in the
   * command or file fields must be either absolute or relative to this directory. */
  jim_member_key(&ctx.ccjson, "directory");
  jim_string(&ctx.ccjson, ctx.exe_path);

  /** The main translation unit source processed by this compilation step.
   * This is used by tools as the key into the compilation database.
   * There can be multiple command objects for the same file, for example
   * if the same source file is compiled with different configurations. */
  if (strstr(ctx.cmd.items[0], "cc")) {
    bool output = false;
    for (const char **arg = ctx.cmd.items + 1; arg < ctx.cmd.items + ctx.cmd.count; ++arg) {
      if (strcmp(*arg, "-o") == 0) {
        output = true;
      } else if (output) {
        jim_member_key(&ctx.ccjson, "file");
        jim_string(&ctx.ccjson, *arg);
        break;
      }
      output = false;
    }
  }

  /** The compile command argv as list of strings. This should run the
   * compilation step for the translation unit file. arguments[0] should be
   * the executable name, such as clang++. Arguments should not be escaped,
   * but ready to pass to execvp(). */
  jim_member_key(&ctx.ccjson, "arguments");
  jim_array_begin(&ctx.ccjson);
  da_foreach(const char *, arg, &ctx.cmd) {
    jim_string(&ctx.ccjson, *arg);
  }
  jim_array_end(&ctx.ccjson);

  /** The compile command as a single shell-escaped string. Arguments may be
   * shell quoted and escaped following platform conventions, with ‘"’ and ‘\’
   * being the only special characters. Shell expansion is not supported. */
  // jim_member_key(&ctx.ccjson, "command");
  // jim_string(&ctx.ccjson, ROOT_PATH);

  /** Either arguments or command is required. arguments is preferred,
   * as shell (un)escaping is a possible source of errors. */

  /** The name of the output created by this compilation step.
   * This field is optional. It can be used to distinguish different
   * processing modes of the same input file. */
  // jim_member_key(&ctx.ccjson, "output");
  // jim_string(&ctx.ccjson, ROOT_PATH);

  jim_object_end(&ctx.ccjson);
}

#endif // NONOB_IMPL
