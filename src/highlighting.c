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
  int num_##name = re_get_matches(line, REGEX(#name), name##_matches);

void init_highlighting() {

  // the buffer is calloced, so all pointers will be NULL if unused.
  w_create_cm(&re_map, sizeof(REComp *), RE_BUF_SZ);

#define DEFINE_REGEX(name_lit, pattern_lit)                                    \
  {                                                                            \
    REComp *re = re_compile(pattern_lit);                                      \
    void *is_not_in_use = w_cm_insert(&re_map, name_lit, &re);                 \
    if (!is_not_in_use) {                                                      \
      status_printf("Name collision with '%s' in highlighting.", name_lit);    \
      LOG({                                                                    \
        printf("WARNING: regex %s already in use, overwriting.\n", name_lit);  \
      });                                                                      \
    }                                                                          \
  }

  DEFINE_REGEX("comments", "//.*");

  // TODO: how to handle escaped quotes?
  DEFINE_REGEX("string_literals", "\"[a-zA-Z0-9 - \\-\\%-%]*\"");
  DEFINE_REGEX("char_literals", "\'[a-zA-Z0-9]\'");
  DEFINE_REGEX("int", "int ");
  DEFINE_REGEX("char", "char ");
  DEFINE_REGEX("star", "\\*");
  DEFINE_REGEX("leftparen", "\\(");
  DEFINE_REGEX("rightparen", "\\)");
  DEFINE_REGEX("semicolon", ";");
  DEFINE_REGEX("escaped", "\\[a-zA-Z0-9]");
  DEFINE_REGEX("function_name", "[a-zA-Z0-9_]+\\(");
  DEFINE_REGEX("preprocessor", "#.*");

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

void print_line(int row, int col, Line *l, Termbuffer *tb) {

  char line_buf[LINE_BUF_SZ];
  sprintf(line_buf, "%s%s", (char *)l->buffer,
          &((char *)l->buffer)[l->gap_end + 1]);

#define COLOR_MATCHES(color, match, start_offset, end_offset)                  \
  {                                                                            \
    tb_change_positional_color(tb, color, row,                                 \
                               col + match->start + start_offset);             \
    tb_change_positional_color(tb, TC_RESET, row,                              \
                               col + match->end + 1 + end_offset);             \
  }

#define SIMPLE_COLOR_ALL(color, name)                                          \
  {                                                                            \
    MATCH(name, line_buf, 8);                                                  \
    for (int i = 0; i < num_##name; i++) {                                     \
      Match *m = &name##_matches[i];                                           \
      COLOR_MATCHES(color, m, 0, 0)                                            \
    }                                                                          \
  }

  MATCH(comments, line_buf, 8);
  if (num_comments >= 1) {
    // only consider the first match, that's the only one that matters.
    Match *m = &comments_matches[0];
    COLOR_MATCHES(TC_CYAN, m, 0, 0);
  }

  MATCH(function_name, line_buf, 8);
  for (int i = 0; i < num_function_name; i++) {
    Match *m = &function_name_matches[0];
    // drop off the first (.
    COLOR_MATCHES(TC_GREEN, m, 0, -1);
  }

  // SIMPLE_COLOR_ALL(TC_RED, escaped);
  SIMPLE_COLOR_ALL(TC_YELLOW, string_literals);

  SIMPLE_COLOR_ALL(TC_BLUE, int);
  SIMPLE_COLOR_ALL(TC_BLUE, char);
  SIMPLE_COLOR_ALL(TC_GREEN, star);

  SIMPLE_COLOR_ALL(TC_BLUE, leftparen);
  SIMPLE_COLOR_ALL(TC_BLUE, rightparen);

  // SIMPLE_COLOR_ALL(TC_RED, semicolon);
  // SIMPLE_COLOR_ALL(TC_RED, char_literals);

  SIMPLE_COLOR_ALL(TC_BG_BLUE, preprocessor);

  tb_pprintf(tb, row, col, "%s", line_buf);

#undef SIMPLE_COLOR_ALL

#undef COLOR_MATCHES
}

#undef REGEX
#undef MATCH
