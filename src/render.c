#include "render.h"
#include "ansi.h"
#include "commands.h"
#include "filetype.h"
#include "highlighting.h"
#include "line.h"
#include "macros.h"
#include "mode.h"
#include "normal.h"
#include "termbuffer.h"
#include "text.h"

#include <stdbool.h>
#include <string.h>
#include <sys/ioctl.h>
#include <termios.h>

#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <unistd.h>

void set_cursor_block() {
  printf("\033[2 q");
  fflush(stdout);
}

void set_cursor_underline() {
  printf("\033[4 q");
  fflush(stdout);
}

void set_cursor_line() {
  printf("\033[6 q");
  fflush(stdout);
}

void reset_cursor_shape() {
  printf("\033[q");
  fflush(stdout);
}

static void move_cursor(int row, int col) {
  printf("\033[%d;%dH", row, col);
  fflush(stdout); // Flush the output buffer
}

static void hide_cursor() {
  printf("\033[?25l");
  fflush(stdout);
}

static void show_cursor() {
  printf("\033[?25h");
  fflush(stdout);
}

static void clear_screen() {
  // Clear the screen
  printf("\033[2J");
  // Move the cursor to the top-left corner
  printf("\033[H");
  fflush(stdout);
}

RenderData render_data = {0};
#define TB (&render_data.tb)

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
  // clear all the garbage out of the buffer.
  tb_clear(TB);

  int text_display_rows = render_data.tb.row - 4;

  // make sure any previous colors aren't affecting us still.
  tb_change_color(TB, TC_RESET);
  tb_change_color(TB, TC_BG_RED);
  tb_change_color(TB, TC_WHITE);

  if (curr_text == NULL) {
    // near the bottom, on the last columns of the terminal:
    tb_pprintf(TB, 0, 5, "-=== STATUS: [ EDITING - %s | MODE - %s ] %c ===-",
               "NONE", mode_string(curr_mode), prev ? prev : ' ');
  } else {
    tb_pprintf(
        TB, 0, 5,
        "-=== STATUS: [ EDITING - %s | FILETYPE - %s | MODE - %s ] %c ===-",
        curr_text->file_path, get_filetype_string(curr_text->type),
        mode_string(curr_mode), prev ? prev : ' ');
  }

  tb_change_color(TB, TC_RESET);

  int com_row = 2;

  if (curr_mode == COMMAND)
    tb_pprintf(TB, com_row, 1, " : %50s", command.buf);
  else
    tb_pprintf(TB, com_row, 1, " > %50s", render_data.status_message);

  if (curr_text == NULL) {
    // if we're not inside any text buffer, simply don't render the text view.
    tb_pprintf(TB, 5, 3, ":edit <filepath> to edit a file.");
    tb_draw(TB);
    return;
  }

  int center_offset = 10;

  int line_offset = 4;
  int j = line_offset;
  while (j < TB->row - 1) { // until we've run out of room:
    int idx = curr_text->y + (j - line_offset) - center_offset;

    if (idx >= curr_text->num_lines)
      break;

    if (idx >= 0) {
      Line *line = &curr_text->lines[idx];

      int base_col = 1;
      tb_change_color(TB, TC_RESET);
      tb_pprintf(TB, j, base_col, " ", idx + 1);
      tb_change_color(TB, TC_BG_GREEN);
      tb_change_color(TB, TC_BLACK);
      tb_pprintf(TB, j, base_col + 1, "%3d", idx + 1);
      tb_change_color(TB, TC_RESET);
      tb_change_color(TB, TC_RED);
      tb_pprintf(TB, j, base_col + 4, ": ", idx + 1);
      tb_change_color(TB, TC_RESET);

      print_line(j, base_col + 7, line, TB);
    }

    j++;
  }

  tb_draw(TB);

  move_cursor(5 + center_offset,
              8 + ((CURR_LINE->gap_start) ? CURR_LINE->gap_start : 1));
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

void handle_resize() { tb_handle_resize(TB); }

void init_render() { tb_init(TB); }

void clean_render() {
  clear_screen();
  show_cursor();
}

void render() { render_text(); }
