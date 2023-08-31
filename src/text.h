#pragma once

#include "line.h"
#include "whisper/array.h"

#define NUM_LINES 1000
#define MAX_FILE_PATH_SZ 256

#define MAX_TEXTS 5

typedef struct Text {
  char file_path[MAX_FILE_PATH_SZ];
  Line lines[NUM_LINES];
  int num_lines;

  // the user's x and y position into the text.
  int x, y;
} Text;

extern WArray texts;

// NULL if we don't have any file open currently.
extern Text *curr_text;

int text_move_x(int by);

int text_move_y(int by);

void texts_init();

// save the currently opened file.
void text_save();

// append to the currently active buffer.
void text_write_char(char c);

// both operate at the cursor.
void text_delete_char();
void text_delete_line();

void text_open_line_above();
void text_open_line();

void text_next_paragraph();
void text_last_paragraph();

// allocate a slot in the texts buffer, and return that.
Text *text_open(char *file_path);
