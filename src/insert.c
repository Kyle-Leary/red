#include "insert.h"
#include "keydef.h"
#include "text.h"

int num_tab_spaces = 2;

void handle_insert_input(InputEvent *e) {
  switch (e->type) {
  case INPUT_CHAR: {
    switch (e->data.as_char) {
    case BACKSPACE: {
      text_delete_char();
    } break;

    case '\n': {
      // open new line below.
      text_open_line();
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
