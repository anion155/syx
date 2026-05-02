#ifndef CLI_H
#define CLI_H

#include <magic.h>
#include <stdlib.h>

#define ESC "\x1b["

#define RESET ESC "0m"
#define BOLD ESC "1m"
#define DIM ESC "2m"
#define F_RED ESC "31m"
#define F_GREEN ESC "32m"
#define F_YELLOW ESC "33m"
#define F_BLACK ESC "38;2;0;0;0m"
#define F_WHITE ESC "37m"
#define BG_RED ESC "41m"
#define BG_GREEN ESC "42m"
#define BG_YELLOW ESC "43m"
#define BG_BLUE ESC "44m"
#define BG_MAGENTA ESC "45m"
#define BG_CYAN ESC "46m"

#define MOVE_CURSOR_UP(lines) ESC STRINGIFY(lines) "A"

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
