#include "lsp.h"
#include "sig.h"
#include "string.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <unistd.h>

#include "global.h"

LSPQueue lsp = {
    .request_mutex = PTHREAD_MUTEX_INITIALIZER,
    .requests = {0},
    .response_mutex = PTHREAD_MUTEX_INITIALIZER,
    .responses = {0},
};

// pass in an LSP* directly to the thread.
void *lsp_thread(void *data) {
  thread_sig_setup();

  LSP *lsp = (LSP *)data;

#define IN (lsp->pipe_in)
#define OUT (lsp->pipe_out)

  int pipes[2];

  pipe(pipes);

  int f = fork();

  switch (f) {
  case -1: {
    // kill the thread.
    perror("fork");
    return NULL;
  } break;

  case 0: { // the lsp process.
    // take over stdin and stdout with the pipe file descriptors.
    dup2(IN, 0);
    dup2(OUT, 1);
    system(lsp->command_buf);
  } break;
  }

  // we can now write and read to the process with IN and OUT.

  // poll the OUT fd for reading.
  int epoll_fd = epoll_create1(0);

  { // setup the epoll fd
    struct epoll_event ev;
    ev.events = EPOLLIN; // Notify when ready to read
    ev.data.fd = OUT;

    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, OUT, &ev) == -1) {
      perror("epoll_ctl");
      exit(EXIT_FAILURE);
    }
  }

  char buf[256];

  for (;;) {
    { // handle requests.
      pthread_mutex_lock(&lsp->queues.request_mutex);
      pthread_mutex_unlock(&lsp->queues.request_mutex);
    }

    // then, handle responses.
    struct epoll_event events[10];
    int nfds = epoll_wait(epoll_fd, events, 10, -1);
    if (nfds == -1) {
      perror("epoll_wait");
      exit(EXIT_FAILURE);
    }

    int n_read = read(OUT, buf, sizeof(buf));

    int num_events_parsed = 0;
    LSPEvent e_buffer[MAX_LSP_EVENTS];

    { // PARSING LOOP
      char *ptr = buf;

      for (int i = 0; i < n_read; i++) {
        LSPEvent *e = &e_buffer[i];
      }
    }

    { // critical section, write an event into the queue.
      pthread_mutex_lock(&lsp->queues.response_mutex);
      pthread_mutex_unlock(&lsp->queues.response_mutex);
    }
  }

  return NULL;

#undef IN
#undef OUT
}

void new_lsp(LSP *lsp, const char *command) {
  strncpy(lsp->command_buf, command, LSP_COMMAND_BUF_SZ);
}

void close_lsp(LSP *lsp) {}
