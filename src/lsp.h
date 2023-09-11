#pragma once

// most lsps can read from stdin and post to stdout. so, we can just start it as
// a subprocess and read from its input/output streams to communicate back and
// forth, skipping the entire TCP stuff.

#include "whisper/queue.h"

#include <pthread.h>

#include <bits/pthreadtypes.h>

#define MAX_LSP_EVENTS 256

typedef struct LSPEvent {
} LSPEvent;

typedef struct LSPQueue {
  pthread_mutex_t request_mutex;
  pthread_mutex_t response_mutex;
  WQueue requests;
  WQueue responses;
} LSPQueue;

#define LSP_COMMAND_BUF_SZ 256

typedef struct LSP {
  LSPQueue queues;
  int pipe_in; // input and output pipe streams directly into the lsp process.
  int pipe_out;
  char command_buf[LSP_COMMAND_BUF_SZ];
} LSP;

extern LSPQueue lsp;

void *lsp_thread(void *data);
void lsp_thread_clean();

// the command to start up the lsp. this runs a thread and returns a new
// LSP to read and write from using the proper mutex locking.
void new_lsp(LSP *lsp, const char *command);
void close_lsp(LSP *lsp);
