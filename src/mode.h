#pragma once

#include "input_thread.h"

typedef enum Mode {
  NORMAL,
  INSERT,
  VISUAL,
  COMMAND,
  MODE_COUNT,
} Mode;

extern Mode curr_mode;

const char *mode_string(Mode mode);

void change_mode(Mode mode);

void handle_input(InputEvent *e);
