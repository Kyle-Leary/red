#pragma once

#include "input_thread.h"
#include "render.h"

typedef struct CommandBuffer {
  // keep this null terminated, it'll be printf'd directly in the renderer.
  char buf[STATUS_MSG_BUF_SZ + 1];
  int len;
} CommandBuffer;

extern CommandBuffer command;

void command_run(char *command, int len);
void command_input_flush();

void handle_command_input(InputEvent *e);

// setup hashtable for keywords.
void init_commands();
