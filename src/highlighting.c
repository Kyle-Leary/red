#include "highlighting.h"
#include "libregex.h"
#include "line.h"
#include "render.h"
#include "termbuffer.h"

#include <stddef.h>
#include <stdio.h>

static REComp *re_comments = NULL;

void init_highlighting() { re_comments = re_compile("//.*"); }

void clean_highlighting() {}

void print_line(int row, int col, Line *l, Termbuffer *tb) {
  char line_buf[LINE_BUF_SZ];
  sprintf(line_buf, "%s%s", (char *)l->buffer,
          &((char *)l->buffer)[l->gap_end + 1]);

  char *_line_buf = line_buf;

  tb_change_color(tb, TC_RESET);

  Match comments[16];
  int num_comments = re_get_matches(line_buf, re_comments, comments);
  for (int i = 0; i < num_comments; i++) {
    Match *m = &comments[i];

    // write the buffer up to the first part of the comment.
    col += tb_write(tb, row, col, m->start, _line_buf);
    _line_buf += m->start;

    // then, write the rest with the changed color.
    tb_change_color(tb, TC_CYAN);
  }

  tb_pprintf(tb, row, col, "%s%s", l->buffer, &l->buffer[l->gap_end + 1]);
  tb_change_color(tb, TC_RESET);
}
