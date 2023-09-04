#include "text.h"
#include "filetype.h"
#include "line.h"
#include "macros.h"
#include "render.h"
#include "string.h"

#include "whisper/gap_buffer.h"

#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

WArray texts = {0};
Text *curr_text = NULL;

void texts_init() { w_make_array(&texts, sizeof(Text), MAX_TEXTS); }

void text_write_char(char c) {
  Text *t = curr_text;

  Line *line = &t->lines[t->y];

  w_gb_insert(line, &c);

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
  w_gb_delete(line);
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

void _open_line_n(int n) {
  Text *t = curr_text;
  char init = '\n';
  t->num_lines++;
  Line new;
  w_gb_create(&new, sizeof(char), LINE_BUF_SZ, &init);

  for (int i = t->num_lines - 2; i > n + 1; i--) {
    // copy into the slot below. push downward all the way up the lines.
    memcpy(&t->lines[i + 1], &t->lines[i], sizeof(Line));
  }

  // copy the new line into the right spot.
  memcpy(&t->lines[n], &new, sizeof(Line));

  t->y = n;
}

// 'O' in vi bindings
void text_open_line_above() {
  Text *t = curr_text;
  // don't let it go negative.
  _open_line_n(MAX(t->y - 1, 0));
}

// 'o' in vi bindings
void text_open_line() {
  Text *t = curr_text;
  // don't let it overflow the file.
  _open_line_n(MIN(t->y + 1, t->num_lines));
}

void text_top() { text_move_y(-100000); }
void text_bottom() { text_move_y(100000); }

void _paragraph_handler(int dir) {
  Text *t = curr_text;

  text_move_y(dir);
  do {
    Line *line = &t->lines[t->y];
    if (((char *)line->buffer)[0] == '\n') {
      // empty line, stop moving unless there's another empty line right in
      // front of us.
      int next_idx = t->y + dir;
      if (IS_INSIDE(next_idx, 0, t->num_lines)) {
        Line *next_line = &t->lines[next_idx];
        if (((char *)next_line->buffer)[0] != '\n') {
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
void text_next_paragraph() { _paragraph_handler(1); }
void text_last_paragraph() { _paragraph_handler(-1); }

void text_move_x(int by) {
  Text *t = curr_text;
  w_gb_shift_by(&t->lines[t->y], by);
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
    w_gb_go_to_beginning(new_line);
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
    fprintf(file, "%s%s", (char *)line->buffer,
            (char *)&line->buffer[line->gap_end + 1]);
  }

  fclose(file);
}

Text *text_open(char *file_path) {
  Text *t = NULL;
  for (int i = 0; i < MAX_TEXTS; i++) {
    // returns NULL if the slot is already used.
    t = w_array_get_slot_ptr(&texts, i);
    if (t != NULL)
      break;
  }
  if (t == NULL) {
    status_printf("No more text slots left, cannot open.");
    return NULL;
  }

  strcpy(t->file_path, file_path);

  if (access(file_path, F_OK) != 0) {
    // file doesn't exist, create it.
    int fd = open(file_path, O_WRONLY | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);
    if (fd == -1) {
      status_printf("Failed to create new file %s.", file_path);
      return NULL;
    }

    write(fd, "\n", 1);
    close(fd);

    // then, init the lines of the text.
    t->num_lines = 1;
    char init = '\n';
    w_gb_create(&t->lines[0], sizeof(char), LINE_BUF_SZ, &init);
  } else {
    // then, read all the lines directly into the buffer.
    FILE *file = fopen(file_path, "r");
    if (file == NULL) {
      status_printf("Could not open %s.", file_path);
      return NULL;
    }

    int i = 0;
    do {
      char buf[LINE_BUF_SZ] = {0};

      Line *line = &t->lines[i];

      if (fgets(buf, LINE_BUF_SZ, file) == NULL) {
        break;
      } else {
        i++;
        // init with the line buffer block of memory.
        w_gb_create_from_block(line, sizeof(char), LINE_BUF_SZ, buf,
                               strlen(buf));
        w_gb_go_to_beginning(line); // put the cursor in the first position.
      }
    } while (1);

    fclose(file);

    t->num_lines = i;

    // new active text, inform the rest of this module.
    status_printf("Opened %s.\n", file_path);
  }

  t->type = get_filetype(file_path);

  curr_text = t;
  return t;
}
