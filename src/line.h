#pragma once

#define LINE_BUF_SZ 512

// implementation of a gap buffer.
typedef struct {
  char buffer[LINE_BUF_SZ + 1];

  int gap_start;
  int gap_end;
} Line;

int line_get_length(Line *l);

int line_shift_left(Line *l);
int line_shift_right(Line *l);

void line_go_to_beginning(Line *l);
void line_go_to_end(Line *l);

void line_shift_by(Line *l, int by);

// insert ch at gap.
void line_insert(Line *l, char ch);

// delete at gap/cursor implicitly.
void line_delete(Line *l);

void line_debug_print(Line *l, int up_to);

void line_debug_status_print(Line *l, int up_to);
