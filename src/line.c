#include "line.h"
#include "macros.h"
#include "render.h"
#include <stdio.h>

// define the cursor to be the FIRST character in the right buffer. i think this
// is nonstandard, but it makes sense to me.
#define CURSOR (l->buffer[l->right])
#define LEFT (l->buffer[l->left])
#define LEFT_MAX (l->buffer[l->left + l->left_sz])
#define RIGHT (l->buffer[l->right + 1])

// move a character from the left buffer into the right buffer.
int line_shift_left(Line *l) {
  if (l->left_sz == 0) {
    return 1;
  }

  // bump to the left.
  l->right--;
  // add the character on the left as the new cursor, now that we've allocated
  // space in the right buffer.
  CURSOR = LEFT_MAX;
  LEFT_MAX = '\0';
  // we've taken away a character from the left buffer.
  l->left_sz--;

  return 0;
}

// move a character from the right buffer into the left buffer.
int line_shift_right(Line *l) {
  if (l->right >= LINE_BUF_SZ) {
    return 1;
  }

  LEFT_MAX = CURSOR;
  l->left_sz++;
  CURSOR = '\0';
  // bump the next gap, move the cursor into the new right buffer character.
  l->right++;

  return 0;
}

void line_reset(Line *l) {
  while (line_shift_left(l) == 0) {
    // wait until it returns a nonzero status to indicate error shifting left.
  }
}

void line_shift_by(Line *l, int by) {
  status_printf("shifting by %d.", by);

  if (by < 0) {
    for (int i = 0; i < -1 * by; i++) {
      line_shift_left(l);
    }
  } else {
    for (int i = 0; i < by; i++) {
      line_shift_right(l);
    }
  }
}

void line_insert(Line *l, char ch) { l->len++; }

void line_delete(Line *l) { l->len++; }

void line_debug_print(Line *l, int up_to) {
  printf("\n");
  int max = MIN(up_to, LINE_BUF_SZ);

  printf("|");
  for (int i = 0; i < max; i++) {
    char ch = l->buffer[i];
    printf("%c|", (ch == '\0') ? ' ' : ch);
  }
  printf("\n");

  printf("|");
  for (int i = 0; i < max; i++) {
    printf("%d|", i);
  }
  printf("\n\n");
}
