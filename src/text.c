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
  memset(&t->lines[t->num_lines], 0, sizeof(Line));
  t->num_lines--;
}

static void internal_open_line_n(int n) {
  Text *t = curr_text;
  for (int i = t->num_lines; i > n; i--) {
    memcpy(&t->lines[i], &t->lines[i - 1], sizeof(Line));
  }
  t->num_lines++;

  memset(&t->lines[t->num_lines], 0, sizeof(Line));
  t->lines[t->num_lines].gap_start = 1;
  t->lines[t->num_lines].gap_end = LINE_BUF_SZ - 1;
  t->lines[t->num_lines].buffer[0] = '\n';
  t->y = n;
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

  text_move_y(dir);
  do {
    Line *line = &t->lines[t->y];
    if (line->buffer[0] == '\n') {
      // empty line, stop moving unless there's another empty line right in
      // front of us.
      int next_idx = t->y + dir;
      if (IS_INSIDE(next_idx, 0, t->num_lines)) {
        Line *next_line = &t->lines[next_idx];
        if (next_line->buffer[0] != '\n') {
          break;
        }
        // if it's not an empty, move again.
      } else {
        // end of file. (or start of file)
        break;
      }
    }

    // we found a non-empty line.
    // move past it.
    if (text_move_y(dir) == 0) {
      // if we couldn't move anymore.
      break;
    }
  } while (1);
}

// search ahead for lines whose buffers are just a single '\n'.
void text_next_paragraph() { internal_paragraph_handler(1); }
void text_last_paragraph() { internal_paragraph_handler(-1); }

void text_move_x(int by) {
  Text *t = curr_text;
  line_shift_by(&t->lines[t->y], by);
}

int text_move_y(int by) {
  Text *t = curr_text;

  int old_x = t->lines[t->y].gap_start - 1;

  if (by < 0) {
    by = CLAMP(by, -t->y, 0);
    t->y += by;
  } else {
    by = CLAMP(by, 0, (t->num_lines - t->y - 1));
    t->y += by;
  }

  if (by != 0) {
    Line *new_line = &t->lines[t->y];
    line_go_to_beginning(new_line);
    text_move_x(old_x);
  }
  return by;
}

void text_save() {
  Text *t = curr_text;

  FILE *file = fopen(t->file_path, "w");

  char buf[LINE_BUF_SZ];
  char *ptr = buf;

  for (int i = 0; i < t->num_lines; i++) {
    Line *line = &t->lines[i];
    fprintf(file, "%s%s", line->buffer, &line->buffer[line->gap_end + 1]);
  }

  fclose(file);
}

Text *text_open(char *file_path) {
  Text *t = w_array_get_slot_ptr(&texts, 0);

  strcpy(t->file_path, file_path);

  // then, read all the lines directly into the buffer.
  FILE *file = fopen(file_path, "r");
  int i = 0;
  do {
    Line *line = &t->lines[i];
    line->gap_start = 0;
    line->gap_end = LINE_BUF_SZ;

    memset(line->buffer, 0, LINE_BUF_SZ);

    if (fgets(&line->buffer[line->gap_start], LINE_BUF_SZ, file) == NULL) {
      break;
    } else {
      i++;
      // put everything to the left of the gap.
      line->gap_start += strlen(&line->buffer[line->gap_start]);
      line->gap_start++;
      line_go_to_beginning(line);
    }
  } while (1);

  t->num_lines = i;

  // new active text, inform the rest of this module.
  curr_text = t;
  return t;
}
