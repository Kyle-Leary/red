#pragma once

#include "filetype.h"
#include "input_thread.h"
#include "line.h"
#include "whisper/array.h"

#define CURR_LINE (&curr_text->lines[curr_text->y])

#define NUM_LINES 1000
#define MAX_FILE_PATH_SZ 256

#define MAX_TEXTS 5

// the internal text buffer is a dynamically-sized array of Lines, which are
// themselves gap buffers.

typedef struct Text {
  Filetype type;

  char file_path[MAX_FILE_PATH_SZ];
  Line lines[NUM_LINES];
  int num_lines;

  // the line number, thought of here as the "y position" into the text.
  int y;
} Text;

extern WArray texts;

// NULL if we don't have any file open currently.
extern Text *curr_text;

void text_move_x(int by);

int text_move_y(int by);

void texts_init();

// save the currently opened file. this re-writes the original file from the
// internal buffer, so be careful.
void text_save();

// append to the currently active buffer.
void text_write_char(char c);

// both operate at the cursor.
void text_delete_char();
void text_delete_line(int line_idx);

void text_f_search(char c, int direction);

void text_move_word(int n);

void text_open_line_above();
void text_open_line();

void text_next_paragraph();
void text_last_paragraph();

void text_go_to_buffer(uint index);

// allocate a slot in the texts buffer, and return that.
// all of these text_open methods also implicitly set the curr_text pointer, and
// "open" that buffer in the main editing context.
Text *text_open_file(const char *file_path);
Text *text_open_file_browser(const char *dir_path);
Text *text_open_buffer();
Text *text_open_buffer_viewer();

void text_handle_input(InputEvent *e);

void text_top();
void text_bottom();

void text_paste_buffer(const char *buffer);

void text_delete_after_cursor();

void text_return();

void text_clean();
