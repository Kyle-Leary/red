#include "insert.h"
#include "keydef.h"
#include "text.h"

void handle_insert_input(InputEvent *e) {
  switch (e->type) {
  case INPUT_CHAR: {
    switch (e->data.as_char) {
    case BACKSPACE: {
      text_delete_char();
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
