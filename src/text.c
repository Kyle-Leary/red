#include "text.h"
#include "filetype.h"
#include "line.h"
#include "logging.h"
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

void text_f_search(char c, int direction) {
  Text *t = curr_text;
  Line *line = &t->lines[t->y];

  if (direction > 0) {
    // forward search.
    int start = line->gap_end + 1;
    int end = line->num_elms; // search to the right of the cursor

    // don't search on the current character.
    for (int i = start + 1; i < end; i++) {
      if (((char *)line->buffer)[i] == c) {
        w_gb_shift_to(CURR_LINE, line->gap_start + (i - start));
        return;
      }
    }
  } else {
    // backward search.
    int start = 0;
    int end = line->gap_start; // search to the left of the cursor

    for (int i = end - 2; i >= start; i--) {
      if (((char *)line->buffer)[i] == c) {
        w_gb_shift_to(CURR_LINE, i);
        return;
      }
    }
  }
}

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
  if (((char *)CURR_LINE->buffer)[0] == '\n') {
    // TODO: delete the line here, move the rest up to the above line.
    return;
  }

  Text *t = curr_text;
  Line *line = &t->lines[t->y];
  w_gb_delete(line);
}

void text_delete_line(int line_idx) {
  if (curr_text->num_lines == 1) {
    // don't delete the last line.
    return;
  }

  Text *t = curr_text;
  Line *line = &t->lines[line_idx];
  for (int i = line_idx; i < t->num_lines - 1; i++) {
    // swap bottom and top lines. this ends up deleting the first one, which is
    // the one our cursor is currently over.
    memcpy(&t->lines[i], &t->lines[i + 1], sizeof(Line));
  }
  memset(&t->lines[t->num_lines], 0, sizeof(Line));
  t->num_lines--;

  // decide how our current position changes based on the new deletion.
  if (line_idx < t->y) {
    t->y--;
  } else if (line_idx > t->y) {
    // it's above us, nothing changes.
  } else {
    t->y = CLAMP(t->y, t->y, t->num_lines - 1);
  }
}

void _open_line_n(int line_idx) {
  log_printf("opening line at %d\n", line_idx);

  Text *t = curr_text;
  t->num_lines++;
  Line new;
  w_gb_create(&new, sizeof(char), LINE_BUF_SZ);

  for (int i = t->num_lines - 2; i > line_idx - 1; i--) {
    // copy into the slot below. push downward all the way up the lines.
    memcpy(&t->lines[i + 1], &t->lines[i], sizeof(Line));
  }

  // copy the new line into the right spot.
  memcpy(&t->lines[line_idx], &new, sizeof(Line));

  t->y = line_idx;
}

void text_paste_buffer(const char *buffer) {
  int len = strlen(buffer);
  for (int i = 0; i < len; i++) {
    char ch = buffer[i];
    switch (ch) {
    case '\n': {
      // we don't need to write the newline, since each line already ends with a
      // newline.
      // open a new line and start pasting there.
      text_open_line();
    } break;

    default: {
      text_write_char(ch);
    } break;
    }
  }
}

// 'O' in vi bindings
void text_open_line_above() {
  Text *t = curr_text;
  // don't let it go negative.
  _open_line_n(MAX(t->y, 0));
}

// 'o' in vi bindings
void text_open_line() {
  Text *t = curr_text;
  // don't let it overflow the file.
  _open_line_n(MIN(t->y + 1, t->num_lines));
}

// the return key, open a line then move the text after the cursor onto the next
// line.
void text_return() {
  char buf[LINE_BUF_SZ];
  Line *prev_line = CURR_LINE;
  int sz = w_gb_get_length_after_gap(prev_line);
  if (sz == 0) {
    // if there's nothing after the cursor, just open a new line.
    text_open_line();
    return;
  } else {
    buf[0] = ((char *)prev_line->buffer)[prev_line->gap_start - 1];
    memcpy(buf + 1, &prev_line->buffer[prev_line->gap_end + 1],
           sz);             // get the text after the buffer.
    buf[sz - 1 + 1] = '\0'; // null term over the newline, but add in the extra
                            // cursor character.

    text_delete_after_cursor();
    text_delete_char(); // then, delete the cursor itself.
    status_printf("end '%s'\n", &CURR_LINE->buffer[CURR_LINE->gap_end + 1]);
    text_open_line();

    text_paste_buffer(buf);
  }
}

void text_delete_after_cursor() { w_gb_delete_after_cursor(CURR_LINE); }

void text_top() { text_move_y(-100000); }
void text_bottom() { text_move_y(100000); }

void _paragraph_handler(int dir) {
  Text *t = curr_text;

  text_move_y(dir);
  do {
    if (w_gb_get_length(CURR_LINE) == 0) {
      // empty line, stop moving unless there's another empty line right in
      // front of us.
      int next_idx = t->y + dir;
      if (IS_INSIDE(next_idx, 0, t->num_lines)) {
        Line *next_line = &t->lines[next_idx];
        if (w_gb_get_length(next_line) == 0) {
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

  switch (t->type) {
  case FTYPE_BUFFER: {
    status_printf("Can't save nameless buffer. Open a file with :edit.");
    return;
  } break;

  case FTYPE_FILE_BROWSER: {
    status_printf("Can't save file browser.");
    return;
  } break;

  default: {
    if (!IS_INSIDE(t->type, FTYPE_UNKNOWN, FTYPE_COUNT)) {
      status_printf("ERROR: Unknown filetype: '%d'.", t->type);
      return;
    }
    // this is some normal filetype, we can save it.
  } break;
  }

  FILE *file = fopen(t->file_path, "w");

  char buf[LINE_BUF_SZ];
  char *ptr = buf;

  for (int i = 0; i < t->num_lines; i++) {
    Line *line = &t->lines[i];
    // add in the newline externally, remember that we aren't keeping the
    // newline in the buffer.
    fprintf(file, "%s%s\n", (char *)line->buffer,
            (char *)&line->buffer[line->gap_end + 1]);
  }

  status_printf("Wrote '%d' lines to '%s'.", t->num_lines, t->file_path);

  fclose(file);
}

Text *_find_slot() {
  Text *t = NULL;
  for (int i = 0; i < MAX_TEXTS; i++) {
    // returns NULL if the slot is already used.
    t = w_array_get_slot_ptr(&texts, i);
    if (t != NULL)
      break;
  }

  // TODO: dynalloc a number of buffers so that this never happens.
  if (t == NULL) {
    status_printf("No more text slots left, cannot open.");
    ERROR_NO_ARGS("Ran out of text slots, cannot open. Increase MAX_TEXTS.");
  }

  return t;
}

Text *text_open_file_browser(const char *dir_path) {
  Text *t = _find_slot();

  t->type = FTYPE_FILE_BROWSER;
  t->num_lines = 1;
  t->y = 0;

  w_gb_create(&t->lines[0], sizeof(char), LINE_BUF_SZ);

  curr_text = t;

  return t;
}

static const char buffer_msg[] =
    "This is an unnamed buffer and will not be saved.";

Text *text_open_buffer() {
  Text *t = _find_slot();

  t->type = FTYPE_BUFFER;
  t->num_lines = 1;
  t->y = 0;

  // init with a nice message on the first line so that the user doesn't
  // accidentally start working on a garbage buffer.
  t->num_lines = 1;
  w_gb_create_from_block(&t->lines[0], sizeof(char), LINE_BUF_SZ, buffer_msg,
                         sizeof(buffer_msg));

  curr_text = t;

  return t;
}

Text *text_open_file(const char *file_path) {
  Text *t = _find_slot();

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
    w_gb_create(&t->lines[0], sizeof(char), LINE_BUF_SZ);
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

      if (fgets(buf, LINE_BUF_SZ, file) ==
          NULL) { // this includes the newline into the buffer.
        break;
      } else {
        Line *line = &t->lines[i];
        // init with the line buffer block of memory.
        w_gb_create_from_block(line, sizeof(char), LINE_BUF_SZ, buf,
                               strlen(buf) -
                                   1); // trim the newline from the block.
        w_gb_go_to_beginning(line);    // put the cursor in the first position.
        i++;
      }
    } while (1);

    fclose(file);

    t->num_lines = i;
  }

  // new active text, inform the rest of this module.
  status_printf("Opened %s.\n", file_path);

  t->type = get_filetype(file_path);

  curr_text = t;
  return t;
}
