#include "render.h"
#include "ansi.h"
#include "commands.h"
#include "macros.h"
#include "mode.h"
#include "text.h"

#include <stdbool.h>
#include <string.h>
#include <sys/ioctl.h>
#include <termios.h>

#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <unistd.h>

RenderData render_data = {0};

static void set_cursor_block() {
  printf("\033[2 q");
  fflush(stdout);
}

static void set_cursor_underline() {
  printf("\033[4 q");
  fflush(stdout);
}

static void set_cursor_line() {
  printf("\033[6 q");
  fflush(stdout);
}

static void reset_cursor_shape() {
  printf("\033[q");
  fflush(stdout);
}

static void hide_cursor() { printf("\033[?25l\n"); }

static void show_cursor() { printf("\033[?25h\n"); }

static void get_term_sz() {
  struct winsize w;
  ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
  render_data.row = w.ws_row;
  render_data.col = w.ws_col;
}

static void clear_screen() {
  // Clear the screen
  printf("\033[2J");
  // Move the cursor to the top-left corner
  printf("\033[H\n");
}

static void pprintf(int x, int y, const char *format, ...) {
  // Move the cursor to the desired position
  printf("\033[%d;%dH", y, x);

  // Handle variable arguments
  va_list args;
  va_start(args, format);
  vprintf(format, args);
  va_end(args);
}

static void render_text() {
  clear_screen();

  if (curr_text == NULL) {
    printf("NO ACTIVE TEXT\n");
    return;
  }

  int text_display_rows = render_data.row - 4;

  Text t = *curr_text;
  int end = MIN(t.num_lines, text_display_rows + t.y);

  // near the bottom, on the last columns of the terminal:
  pprintf(0, render_data.col, "STATUS: [ EDITING - %s | MODE - %s ]\n",
          t.file_path, mode_string(curr_mode));
  pprintf(0, render_data.col, " > %s\n", render_data.status_message);
  pprintf(0, render_data.col, " : %s\n", command.buf);

  for (int i = t.y; i < end; i++) {
    bool is_curr_line = (i == t.y);

#define IF_IS_CURR(str) ((is_curr_line) ? str : "")

    Line *line = &curr_text->lines[i];

    // print the cursor character with a cool background.
    pprintf(0, 1 + i - t.y, "%3d: %s%s%c%s%s\n", i + 1,
            &line->buffer[line->left], IF_IS_CURR(ANSI_BG_RED),
            line->buffer[line->right], IF_IS_CURR(ANSI_RESET),
            &line->buffer[line->right + 1]);

#undef IF_IS_CURR
  }
}

void status_printf(const char *format, ...) {
  // Start with a buffer of some reasonable size
  char *buffer = render_data.status_message;

  // Handle variable arguments
  va_list args;
  va_start(args, format);
  int required_size = vsnprintf(buffer, STATUS_MSG_BUF_SZ, format, args);
  va_end(args);
}

void handle_resize() { get_term_sz(); }

void init_render() {}

void clean_render() {
  clear_screen();
  show_cursor();
}

void render() { render_text(); }
