#pragma once

#include "input_thread.h"
#include "whisper/array.h"
#define MAX_MACRO_LEN 256

// a macro is really just a list of characters.
// each macro is stored in a lowercase letter.
extern char macro_buf[26][MAX_MACRO_LEN];
extern char curr_macro;

void macro_start_recording(char ch);
void macro_stop_recording();

void macro_handle_input(InputEvent *e);
// feed this character buffer into stdin.
void macro_play(char ch);
