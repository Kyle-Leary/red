#include "normal.h"
#include "clipboard.h"
#include "keydef.h"
#include "line.h"
#include "macros.h"
#include "mode.h"
#include "render.h"
#include "text.h"
#include "whisper/gap_buffer.h"

#include "libregex.h"

// the previous char pressed. NULL if there's no significant char right before
// us. this is used to implement the "combo" type commands, like dd or yi.
char prev = '\0';

typedef struct FSearchState {
  char c;        // '\0' means we haven't searched yet.
  int direction; // positive or negative for forward or backward.
} FSearchState;

SearchState search = {0};

static FSearchState f_search = {0};

static void _do_f_search() { text_f_search(f_search.c, f_search.direction); }

static void _enter_search(int direction) {
  search.is_searching = 1;
  search.direction = direction;

  search.pattern_buf[0] = '\0';
  search.pattern_buf_len = 0;
}

static void _do_search(int direction_modifier) {
  int final_direction = search.direction * direction_modifier;

  int starting_search_row = curr_text->y;
  int starting_search_col = CURR_LINE->gap_start - 1;

  if (final_direction == 1) {
    curr_text->y = starting_search_row;
    CURR_LINE->gap_start = starting_search_col + 1;

    char line_buf[LINE_BUF_SZ];

    // go down the whole text and run regex matches on each line until we either
    // find a match or don't get anything.
    while (curr_text->y < curr_text->num_lines) {
      // match with the whole line.
      sprintf(line_buf, "%c%s",
              ((char *)CURR_LINE->buffer)[CURR_LINE->gap_start - 1],
              &((char *)CURR_LINE->buffer)[CURR_LINE->gap_end + 1]);

      Match match_buf[16];
      int num_matches =
          re_get_matches(line_buf, search.cached_compile, match_buf);

      if (num_matches > 0) {
        w_gb_shift_to(CURR_LINE, match_buf[0].start);
        // TODO: highlight the match?
        return;
      }

      curr_text->y++;
    }

    status_printf("No matches found for pattern '%s'.\n", search.pattern_buf);
  } else if (final_direction == -1) {
    curr_text->y = starting_search_row;
    CURR_LINE->gap_start = starting_search_col + 1;

    char line_buf[LINE_BUF_SZ];

    // go down the whole text and run regex matches on each line until we either
    // find a match or don't get anything.
    while (curr_text->y > 0) {
      // match with the whole line.
      sprintf(line_buf, "%c%s",
              ((char *)CURR_LINE->buffer)[CURR_LINE->gap_start - 1],
              &((char *)CURR_LINE->buffer)[CURR_LINE->gap_end + 1]);

      Match match_buf[16];
      int num_matches =
          re_get_matches(line_buf, search.cached_compile, match_buf);

      if (num_matches > 0) {
        w_gb_shift_to(CURR_LINE, match_buf[0].start);
        // TODO: highlight the match?
        return;
      }

      curr_text->y--;
    }

    status_printf("No matches found for pattern '%s'.\n", search.pattern_buf);
  }
}

static void char_handler(char c) {
  Line *line = &curr_text->lines[curr_text->y];

  if (search.is_searching) {
    if (c == '\n' ||
        c == ESC_KEY) { // quit the search, but keep around the pattern.
      search.is_searching = 0;
      return;
    }

    if (c == '\b') {
      if (search.pattern_buf_len > 0) {
        search.pattern_buf_len--;
        search.pattern_buf[search.pattern_buf_len] = '\0';
      }
    } else {
      search.pattern_buf[search.pattern_buf_len] = c;
      search.pattern_buf_len++;
      search.pattern_buf[search.pattern_buf_len] = '\0';
    }

    // then, update the regex pattern and try to find the next match.
    if (search.cached_compile != NULL) {
      re_free(search.cached_compile);
    }
    search.cached_compile = re_compile(search.pattern_buf);

    _do_search(search.direction);

    return; // ignore all other commands if we're staying in search mode.
  }

  switch (prev) {
  case 'd': {
    switch (c) {
    case 'd': {
      text_delete_line(curr_text->y);
      prev = '\0';
      return;
    } break;

    case 'h': {
    } break;
    case 'j': {
      text_delete_line(curr_text->y);
      text_delete_line(curr_text->y);
      prev = '\0';
      return;
    } break;
    case 'k': {
      text_delete_line(curr_text->y);
      text_move_y(-1);
      text_delete_line(curr_text->y);
      prev = '\0';
      return;
    } break;
    case 'l': {
    } break;

    default: {
      prev = '\0';
    } break;
    }
  } break;

  case 'y': {
    switch (c) {
    case 'y': {
      clip_copy_line(CLIP_DEFAULT, CURR_LINE);
      prev = '\0';
      return;
    } break;

    default: {
      prev = '\0';
    } break;
    }
  } break;

    // this works for either direction.
  case 'F':
  case 'f': {
    f_search.c = c;
    _do_f_search();
    prev = '\0';
    return;
  } break;

  case 'g': {
    switch (c) {
    case 'g': {
      text_top();
      prev = '\0';
      return;
    } break;

    default: {
      prev = '\0';
    } break;
    }
  } break;

  default: {
    // this prev is not registered to be handled, so just let it propagate into
    // the normal loop.
    prev = '\0';
  } break;
  }

  switch (c) {
  case ':': {
    change_mode(COMMAND);
    return;
  } break;

  case 'd': {
    prev = c;
  } break;

  case ';': {
    if (f_search.c != '\0') {
      _do_f_search();
    }
  } break;

  case 'f': {
    f_search.direction = 1;
    prev = c;
  } break;
  case 'F': {
    f_search.direction = -1;
    prev = c;
  } break;

  case '/': {
    _enter_search(1);
  } break;
  case '?': {
    _enter_search(-1);
  } break;

  case 'n': {
    text_move_word(1);
    _do_search(1 * search.direction);
  } break;
  case 'N': {
    // just the opposite of the current search direction.
    text_move_word(-1);
    _do_search(-1 * search.direction);
  } break;

  case '$': {
    w_gb_go_to_end(CURR_LINE);
    w_gb_shift_left(CURR_LINE);
  } break;
  case '0': {
    w_gb_go_to_beginning(line);
  } break;

  case 'x': {
    text_delete_char();
  } break;

  case 'D': {
    text_move_x(-1);
    text_delete_after_cursor();
  } break;

  case 'i': {
    change_mode(INSERT);
  } break;
  case 'I': {
    w_gb_go_to_beginning(line);
    change_mode(INSERT);
  } break;

  case 'a': {
    w_gb_shift_right(line);
    change_mode(INSERT);
  } break;
  case 'A': {
    w_gb_go_to_end(CURR_LINE);
    w_gb_shift_left(CURR_LINE);
    change_mode(INSERT);
  } break;

  case 'G': {
    text_bottom();
  } break;
  case 'g': {
    prev = 'g';
  } break;

  case 'h': {
    text_move_x(-1);
  } break;
  case 'j': {
    text_move_y(1);
  } break;
  case 'k': {
    text_move_y(-1);
  } break;
  case 'l': {
    text_move_x(1);
  } break;

  case 'o': {
    text_open_line();
    change_mode(INSERT);
  } break;
  case 'O': {
    text_open_line_above();
    change_mode(INSERT);
  } break;

  case '{': {
    text_last_paragraph();
  } break;
  case '}': {
    text_next_paragraph();
  } break;

  case CTRL('d'): {
    text_move_y(10);
  } break;
  case CTRL('u'): {
    text_move_y(-10);
  } break;

  case 'p': {
    // paste the default buffer directly into the current text.
    text_paste_buffer(clipboards[CLIP_DEFAULT]);
  } break;

  case 'y': {
    prev = 'y';
  } break;

    // word movements
  case 'w': {
    text_move_word(1);
  } break;
  case 'W': {
    text_move_word(1);
  } break;

  case 'e': {
    text_move_word(1);
  } break;
  case 'E': {
    text_move_word(1);
  } break;

  case 'b': {
    text_move_word(-1);
  } break;
  case 'B': {
    text_move_word(-1);
  } break;
  }
}

void handle_normal_input(InputEvent *e) {
  switch (e->type) {
  case INPUT_CHAR: {
    char_handler(e->data.as_char);
  } break;
  case INPUT_SPECIAL: {
  } break;
  default: {
  } break;
  }
}
