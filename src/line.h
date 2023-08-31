#pragma once

#define LINE_BUF_SZ 512

// implementation of a gap buffer.
typedef struct {
  char buffer[LINE_BUF_SZ];

  // both indices, not pointers.
  int left;
  int right;

  // how many chars are in the left buffer.
  int left_sz;

  // how many chars in both?
  int len;
} Line;

int line_shift_left(Line *l);
int line_shift_right(Line *l);

// shift the line left until you can't anymore.
void line_reset(Line *l);

void line_shift_by(Line *l, int by);

// insert ch at gap.
void line_insert(Line *l, char ch);

// delete at gap/cursor implicitly.
void line_delete(Line *l);

void line_debug_print(Line *l, int up_to);
