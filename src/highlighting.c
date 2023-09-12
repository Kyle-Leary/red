#include "highlighting.h"
#include "libregex.h"
#include "line.h"
#include "logging.h"
#include "render.h"
#include "termbuffer.h"
#include "util.h"
#include "whisper/array.h"
#include "whisper/colmap.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

WColMap re_map;

#define RE_BUF_SZ 509

// will crash if not found.
#define REGEX(name_lit) *((REComp **)w_cm_get(&re_map, name_lit))
#define MATCH(name, line, max_matches)                                         \
  Match name##_matches[max_matches];                                           \
  int num_##name = re_get_matches(line_buf, REGEX(#name), name##_matches);

void init_highlighting() {

  // the buffer is calloced, so all pointers will be NULL if unused.
  w_create_cm(&re_map, sizeof(REComp *), RE_BUF_SZ);

#define DEFINE_REGEX(name_lit, pattern_lit)                                    \
  {                                                                            \
    REComp *re = re_compile(pattern_lit);                                      \
    w_cm_insert(&re_map, name_lit, &re);                                       \
  }

  DEFINE_REGEX("comments", "//.*");
  DEFINE_REGEX("int", "int ");
  DEFINE_REGEX("char", "char ");

#undef DEFINE_REGEX
}

void clean_highlighting() {
  // try to free every bucket.
  for (int i = 0; i < RE_BUF_SZ; i++) {
    REComp **r = w_array_get(&re_map, i);
    if (r) {
      // LOG({ re_debug_print(r); });

      re_free(*r);
    }
  }

  w_free_cm(&re_map);
}

#define COLOR_MATCHES(color, match)                                            \
  {                                                                            \
    tb_change_positional_color(tb, color, row, col + match->start - 1);        \
    tb_change_positional_color(tb, TC_RESET, row, col + match->end);           \
  }

void print_line(int row, int col, Line *l, Termbuffer *tb) {
  char line_buf[LINE_BUF_SZ];
  sprintf(line_buf, "%s%s", (char *)l->buffer,
          &((char *)l->buffer)[l->gap_end + 1]);

  int line_idx = 0;

  MATCH(comments, line_buf, 8);
  if (num_comments >= 1) {
    // only consider the first match, that's the only one that matters.
    Match *m = &comments_matches[0];
    COLOR_MATCHES(TC_CYAN, m)
  }

  MATCH(int, line_buf, 8);
  for (int i = 0; i < num_int; i++) {
    Match *m = &int_matches[i];
    COLOR_MATCHES(TC_BLUE, m)
  }
  tb_change_color(tb, TC_RESET);

  tb_pprintf(tb, row, col, "%s", line_buf);
}

#undef REGEX
#undef MATCH
