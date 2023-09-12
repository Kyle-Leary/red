#pragma once

#include "input_thread.h"
#include "libregex.h"
#include "line.h"

// handling of normal mode stuff.

extern char prev;

// state for the / ? search command.
typedef struct SearchState {
  int starting_search_row; // where should we anchor the search from?
  int starting_search_col;

  char pattern_buf[LINE_BUF_SZ];
  int pattern_buf_len;

  REComp *cached_compile;
  int direction; // 1 for forward and -1 for backward.

  int is_searching;
} SearchState;

extern SearchState search;

void handle_normal_input(InputEvent *e);
