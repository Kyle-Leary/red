#include "normal.h"
#include "clipboard.h"
#include "keydef.h"
#include "line.h"
#include "macros.h"
#include "mode.h"
#include "text.h"
#include "whisper/gap_buffer.h"

// the previous char pressed. NULL if there's no significant char right before
// us. this is used to implement the "combo" type commands, like dd or yi.
char prev = '\0';

typedef struct FSearchState {
  char c;        // '\0' means we haven't searched yet.
  int direction; // positive or negative for forward or backward.
} FSearchState;

static FSearchState f_search = {0};

static void _do_f_search() { text_f_search(f_search.c, f_search.direction); }

static void char_handler(char c) {
  Line *line = &curr_text->lines[curr_text->y];

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
