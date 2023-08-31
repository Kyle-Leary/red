#include "text.h"
#include "line.h"
#include "macros.h"
#include "render.h"
#include "string.h"

#include <stdio.h>

WArray texts = {0};
Text *curr_text = NULL;

void texts_init() { w_make_array(&texts, sizeof(Text), MAX_TEXTS); }

void text_write_char(char c) {
  Text *t = curr_text;

  Line *line = &t->lines[t->y];

  line_insert(line, c);

  switch (c) {
  case '\n': {
  } break;
  default: {
  } break;
  }
}

void text_delete_char() {
  Text *t = curr_text;
  Line *line = &t->lines[t->y];
  line_delete(line);
}

void text_delete_line() {
  Text *t = curr_text;
  Line *line = &t->lines[t->y];
  for (int i = t->y; i < t->num_lines - 1; i++) {
    // swap bottom and top lines. this ends up deleting the first one, which is
    // the one our cursor is currently over.
    memcpy(&t->lines[i], &t->lines[i + 1], sizeof(Line));
  }
}

static void internal_open_line_n(int n) {
  Text *t = curr_text;
  for (int i = t->num_lines; i > n; i--) {
    memcpy(&t->lines[i], &t->lines[i - 1], sizeof(Line));
  }
}

// 'O' in vi bindings
void text_open_line_above() {
  Text *t = curr_text;
  // don't let it go negative.
  internal_open_line_n(MAX(t->y - 1, 0));
}

// 'o' in vi bindings
void text_open_line() {
  Text *t = curr_text;
  // don't let it overflow the file.
  internal_open_line_n(MIN(t->y + 1, t->num_lines));
}

static void internal_paragraph_handler(int dir) {
  Text *t = curr_text;
  Line *line = &t->lines[t->y];

  // get the full line, and compare it with \n.
  char buf[LINE_BUF_SZ];
  do {
    sprintf(buf, "%s%s\n", &line->buffer[line->left],
            &line->buffer[line->right]);

    if (buf[0] == '\n') {
      if (text_move_y(dir) == 0) {
        // if we couldn't move anymore.
        break;
      }
    } else {
      // we found a non-empty line.
      break;
    }
  } while (1);
}

// search ahead for lines whose buffers are just a single '\n'.
void text_next_paragraph() { internal_paragraph_handler(1); }
void text_last_paragraph() { internal_paragraph_handler(-1); }

// after moving something around, reorganize the gap buffer. operates on the
// current line and current text.
static void refresh_line() {}

int text_move_x(int by) {
  Text *t = curr_text;

  if (by < 0) {
    // it cannot exceed the left boundary from the cursor.
    by = CLAMP(by, -t->x, 0);
    t->x += by;
  } else {
    by = CLAMP(by, 0, t->lines[t->y].len);
    t->x += by;
  }

  line_shift_by(&t->lines[t->y], by);

  return by;
}

int text_move_y(int by) {
  Text *t = curr_text;

  if (by < 0) {
    by = CLAMP(by, -t->y, 0);
    t->y += by;
  } else {
    by = CLAMP(by, 0, t->num_lines);
    t->y += by;
  }

  return by;
}

void text_save() { status_printf("Saving is not implemented."); }

Text *text_open(char *file_path) {
  Text *t = w_array_get_slot_ptr(&texts, 0);

  strcpy(t->file_path, file_path);

  // then, read all the lines directly into the buffer.
  FILE *file = fopen(file_path, "r");
  int i = 0;
  do {
    Line *line = &t->lines[i];
    line->left = 0;
    line->left_sz = 0;
    line->right = LINE_BUF_SZ / 2;

    memset(line->buffer, 0, LINE_BUF_SZ);

    if (fgets(&line->buffer[line->right], LINE_BUF_SZ / 2, file) == NULL)
      break;

    line->len = strlen(&line->buffer[line->right]);

    i++;
  } while (1);

  t->num_lines = i;

  // new active text, inform the rest of this module.
  curr_text = t;
  return t;
}
