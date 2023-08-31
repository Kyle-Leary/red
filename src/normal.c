#include "normal.h"
#include "keydef.h"
#include "line.h"
#include "macros.h"
#include "mode.h"
#include "text.h"

// the previous char pressed. NULL if there's no significant char right before
// us. this is used to implement the "combo" type commands, like dd or yi.
static char prev = '\0';

static void char_handler(char c) {
  Line *line = &curr_text->lines[curr_text->y];

  switch (prev) {
  case 'd': {
    text_delete_line();
    prev = '\0';
  } break;
  case ';': {
    // keep it in f-search mode.
    return;
  } break;
  default: {
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

  case 'f': {
    prev = c;
  } break;
  case 'F': {
    prev = c;
  } break;

  case '$': {
    line_go_to_end(&curr_text->lines[curr_text->y]);
  } break;
  case '0': {
    line_go_to_beginning(line);
  } break;

  case 'x': {
    text_delete_char();
  } break;

  case 'i': {
    change_mode(INSERT);
  } break;
  case 'I': {
    line_go_to_beginning(line);
    change_mode(INSERT);
  } break;
  case 'a': {
    line_shift_right(line);
    change_mode(INSERT);
  } break;
  case 'A': {
    line_go_to_end(line);
    change_mode(INSERT);
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
  } break;
  case 'O': {
    text_open_line_above();
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
