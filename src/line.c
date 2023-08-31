#include "line.h"
#include "macros.h"
#include "render.h"
#include <stdio.h>

// be sure to keep the gap NULLed for easy printf'ing.

int line_get_length(Line *l) {
  return (l->gap_start + (LINE_BUF_SZ - l->gap_end));
}

// move a character from the left buffer into the right buffer.
int line_shift_left(Line *l) {
  if (l->gap_start <= 1) {
    return 1;
  }

  l->gap_start--;
  l->buffer[l->gap_end] = l->buffer[l->gap_start];
  l->buffer[l->gap_start] = '\0';
  l->gap_end--;

  return 0;
}

// move a character from the right buffer into the left buffer.
int line_shift_right(Line *l) {
  if (l->gap_end >= LINE_BUF_SZ - 1) {
    return 1;
  }

  l->gap_end++;
  l->buffer[l->gap_start] = l->buffer[l->gap_end];
  l->buffer[l->gap_end] = '\0';
  l->gap_start++;

  return 0;
}

void line_go_to_beginning(Line *l) {
  while (line_shift_left(l) == 0) {
    // wait until it returns a nonzero status to indicate error shifting left.
  }
}

void line_go_to_end(Line *l) {
  while (line_shift_right(l) == 0) {
  }
}

void line_shift_by(Line *l, int by) {
  if (by < 0) {
    by = CLAMP(by, -l->gap_start, 0);
    for (int i = 0; i < -1 * by; i++) {
      line_shift_left(l);
    }
  } else {
    by = CLAMP(by, 0, line_get_length(l) - l->gap_start);
    for (int i = 0; i < by; i++) {
      line_shift_right(l);
    }
  }
}

void line_insert(Line *l, char ch) {
  if (l->gap_start == l->gap_end)
    return;

  l->buffer[l->gap_start] = ch;
  l->gap_start++;
}

void line_delete(Line *l) {
  if (l->gap_start == 0)
    return;

  l->gap_start--;
  l->buffer[l->gap_start] = '\0';

  // now we don't have any character at the cursor. get one.
  line_shift_right(l);
}

void line_debug_print(Line *l, int up_to) {
  printf("\n");
  int max = MIN(up_to, LINE_BUF_SZ);

  printf("|");
  for (int i = 0; i < max; i++) {
    char ch = l->buffer[i];
    printf("%c|", (ch == '\0') ? 'N' : ch);
  }
  printf("\n");

  printf("|");
  for (int i = 0; i < max; i++) {
    printf("%d|", i);
  }
  printf("\n\n");
}

void line_debug_status_print(Line *l, int up_to) {
  int max = MIN(up_to, LINE_BUF_SZ);

  char buf[LINE_BUF_SZ];
  char *ptr = buf;

  ptr += sprintf(ptr, "|");
  for (int i = 0; i < max; i++) {
    char ch = l->buffer[i];
    ptr += sprintf(ptr, "%c|", (ch == '\0') ? 'N' : ch);
  }

  status_printf("%s", buf);
}
