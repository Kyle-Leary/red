#pragma once

#define STATUS_MSG_BUF_SZ 256

typedef struct RenderData {
  // the size of our term.
  int row, col;
  char status_message[STATUS_MSG_BUF_SZ];
  int status_message_len;
} RenderData;

extern RenderData render_data;

void handle_resize();

// write a message to the status bar to inform the user about something.
void status_printf(const char *format, ...);

void init_render();
void clean_render();

// decide whether or not we need a re-render, handle everything related to
// drawing on the screen/getting required terminal variables.
void render();

void set_cursor_block();
void set_cursor_underline();
void set_cursor_line();
void reset_cursor_shape();
