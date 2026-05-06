#ifndef CLI_H
#define CLI_H

#include <magic.h>
#include <stdlib.h>

#define CLI_ESC "\x1b["

#define CLI_RESET CLI_ESC "0m"
#define CLI_BOLD CLI_ESC "1m"
#define CLI_DIM CLI_ESC "2m"

#define CLI_FG_RED CLI_ESC "31m"
#define CLI_FG_GREEN CLI_ESC "32m"
#define CLI_FG_YELLOW CLI_ESC "33m"
#define CLI_FG_BLACK CLI_ESC "38;2;0;0;0m"
#define CLI_FG_WHITE CLI_ESC "37m"

#define CLI_BG_RED CLI_ESC "41m"
#define CLI_BG_GREEN CLI_ESC "42m"
#define CLI_BG_YELLOW CLI_ESC "43m"
#define CLI_BG_BLUE CLI_ESC "44m"
#define CLI_BG_MAGENTA CLI_ESC "45m"
#define CLI_BG_CYAN CLI_ESC "46m"

#define CLI_MOVE_CURSOR_UP(lines) CLI_ESC STRINGIFY(lines) "A"

typedef struct cli_term_size {
  size_t cols;
  size_t rows;
} cli_term_size;

cli_term_size cli_get_window_size();

#endif // CLI_H
#if defined(CLI_IMPL) && !defined(CLI_IMPL_C)
#define CLI_IMPL_C

#ifdef _WIN32
#  include <windows.h>
#else
#  include <sys/ioctl.h>
#  include <unistd.h>
#endif

cli_term_size cli_get_window_size() {
#ifdef _WIN32
  CONSOLE_SCREEN_BUFFER_INFO csbi;
  GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
  return (cli_term_size){
      csbi.srWindow.Right - csbi.srWindow.Left + 1,
      csbi.srWindow.Bottom - csbi.srWindow.Top + 1};
#else
  struct winsize size;
  if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &size) != 0) return (cli_term_size){80, 40};
  return (cli_term_size){size.ws_col, size.ws_row};
#endif
}

#endif // CLI_IMPL
