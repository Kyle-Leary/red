#include "insert.h"
#include "text.h"

void handle_insert_input(InputEvent *e) {
  switch (e->type) {
  case INPUT_CHAR: {
    text_write_char(e->data.as_char);
  } break;
  case INPUT_SPECIAL: {
  } break;
  default: {
  } break;
  }
}
