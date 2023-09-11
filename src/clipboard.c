#include "clipboard.h"
#include <stdio.h>
#include <string.h>

char clipboards[CLIP_COUNT][MAX_CLIPBOARD_DATA] = {0};

void clip_copy(Clipboards c, const char *str) { strcpy(clipboards[c], str); }

void clip_copy_line(Clipboards c, Line *l) {
  char line_buf[LINE_BUF_SZ];
  sprintf(line_buf, "%s%s", (char *)l->buffer,
          &((char *)l->buffer)[l->gap_end + 1]);

  clip_copy(c, line_buf);
}
