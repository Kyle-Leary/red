#include "mode.h"
#include "commands.h"
#include "input_thread.h"

#include "insert.h"
#include "keydef.h"
#include "logging.h"
#include "macros.h"
#include "normal.h"
#include "render.h"
#include "text.h"
#include "whisper/gap_buffer.h"
#include <string.h>

Mode curr_mode = NORMAL;

const char *normal_mode = "NORMAL";
const char *insert_mode = "INSERT";
const char *visual_mode = "VISUAL";
const char *command_mode = "COMMAND";

const char *mode_string(Mode mode) {
  switch (mode) {
  case NORMAL: {
    return normal_mode;
  } break;
  case INSERT: {
    return insert_mode;
  } break;
  case VISUAL: {
    return visual_mode;
  } break;
  case COMMAND: {
    return command_mode;
  } break;
  default: {
    ERROR_NO_ARGS("tried to get mode string of invalid mode.\n");
    return NULL;
  } break;
  }
}

void change_mode(Mode mode) {

  switch (mode) {
  case NORMAL: {
    set_cursor_block();
  } break;
  case INSERT: {
    set_cursor_line();
  } break;
  case VISUAL: {
  } break;
  case COMMAND: {
    // clear the status message for command entry.
    memset(render_data.status_message, 0, STATUS_MSG_BUF_SZ);
  } break;
  default: {
  } break;
  }

  if (curr_mode != mode) {
    // handler when leaving a mode.
    switch (curr_mode) {
    case COMMAND: {
      command_input_flush();
    } break;
    default:
      break;
    }

    // then just update the mode state.
    curr_mode = mode;
  }
}

void handle_input(InputEvent *e) {
  // inputs that can occur in any mode and any state, that don't even require an
  // active buffer.
  switch (curr_mode) {
  case COMMAND: {
    handle_command_input(e);
  } break;
  default: {
    switch (e->type) {
    case INPUT_CHAR: {
      switch (e->data.as_char) {
      case ':': {
        change_mode(COMMAND);
      } break;

      case ESC_KEY: {
        change_mode(NORMAL);
      } break;

      case CTRL('p'): {
      } break;

      default:
        break;
      }
    } break;
    default:
      break;
    }
  } break;
  }

  if (!curr_text) { // this is as far as we get if there's no text buffer.
    return;
  }

  // else, we're inputting in a valid text buffer.

  // first, handle general input stuff
  switch (e->type) {
  case INPUT_CHAR: {
    char ch = e->data.as_char;
    switch (ch) {
    case ESC_KEY: {
      w_gb_shift_left(CURR_LINE);
      change_mode(NORMAL);
    } break;
    }
  } break;
  case INPUT_SPECIAL: {
  } break;
  default: {
    ERROR_NO_ARGS("tried to handle input on an input type.");
  } break;
  }

  // then, handle modal input.
  switch (curr_mode) {
  case NORMAL: {
    handle_normal_input(e);
  } break;
  case INSERT: {
    handle_insert_input(e);
  } break;
  case VISUAL: {
  } break;
  case COMMAND: {
  } break;
  default: {
    ERROR_NO_ARGS("tried to handle input on an invalid mode.");
  } break;
  }
}
