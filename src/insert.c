#include "insert.h"
#include "keydef.h"
#include "text.h"

int num_tab_spaces = 2;

void handle_insert_input(InputEvent *e) {
  switch (e->type) {
  case INPUT_CHAR: {
    switch (e->data.as_char) {
    case BACKSPACE: {
      text_move_x(-1);
      text_delete_char();
      text_move_x(1);
    } break;

    case '\n': {
      text_return();
    } break;

    case '\t': {
      for (int i = 0; i < num_tab_spaces; i++) {
        text_write_char(' ');
      }
    } break;

    default: {
      text_write_char(e->data.as_char);
    } break;
    }

  } break;
  case INPUT_SPECIAL: {
  } break;
  default: {
  } break;
  }
}
