#include "filetype.h"
#include "highlighting.h"
#include "logging.h"
#include "sig.h"
#include <stdlib.h>
#define _GNU_SOURCE
#include <execinfo.h>

#include "commands.h"
#include "input_thread.h"
#include "insert.h"
#include "mode.h"
#include "normal.h"
#include "render.h"
#include "text.h"

#include <assert.h>
#include <linux/limits.h>
#include <pthread.h>

#include <bits/pthreadtypes.h>

#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include <stdbool.h>

// termios - Terminal IO Settings.
#include <termios.h>

#include "main.h"

bool should_quit = false;

// write something into stdin every once in a while.
void *testing_thread(void *data) {
  for (;;) {
    usleep(1000000);
    write(STDIN_FILENO, "a", 1);
  }
  return NULL;
}

pthread_t main_thread;

pthread_t test_thread = 0;
bool is_using_test_thread = false;

pthread_t input_thread_handle;

static int has_cleaned = 0;

void clean_main() {
  if (has_cleaned) {
    return;
  }

  has_cleaned = 1;

  {
    pthread_cancel(input_thread_handle);
    input_thread_clean();

    if (is_using_test_thread) {
      pthread_cancel(test_thread);
    }
  }

  clean_filetypes();
  clean_highlighting();
  clean_render();
  log_clean();
}

// define signal handlers.
void sigwinch(int num) {
  // the terminal emu/window has been resized.
  handle_resize();
}

int main(int argc, char **argv) {
  log_init();

  printf("not logged\n");
  LOG({ printf("logging is maybe working\n"); });
  printf("still no\n");
  LOG({ printf("logging is probably working\n"); });
  printf("not anymore\n");

  main_thread = pthread_self();

  init_filetypes();
  init_highlighting();
  init_render();
  init_commands();

  status_printf("welcome to red.");

  { // setup threads
    w_make_queue(&input.queue, sizeof(InputEvent), MAX_INPUT_EVENTS);
    pthread_create(&input_thread_handle, NULL, input_thread, NULL);
  }

  texts_init();

#define BUMP()                                                                 \
  {                                                                            \
    argv++;                                                                    \
    argc--;                                                                    \
  }

  BUMP();

  while (argc > 0) {
    if (strncmp(argv[0], "--test", 6) == 0) {
      pthread_create(&test_thread, NULL, testing_thread, NULL);
      is_using_test_thread = true;
      BUMP();
    }

    // open all other args that aren't matched with anything into a text buffer.
    text_open(argv[0]);
    BUMP();
  }

#undef BUMP

  signal(SIGWINCH, sigwinch);

  thread_sig_setup();

  render();

  while (!should_quit) {
    int num_e = 0;
    InputEvent events[MAX_INPUT_EVENTS];

    pthread_mutex_lock(&input.mutex);
    for (;;) { // copy the critical threaded data into a safe local buffer, and
               // minimize the time we spend in the critical section.
      InputEvent *e_ptr = w_dequeue(&input.queue);
      if (e_ptr == NULL) {
        break;
      }

      memcpy(&events[num_e], e_ptr, sizeof(InputEvent));
      num_e++;
    }
    pthread_mutex_unlock(&input.mutex);

    for (int i = 0; i < num_e; i++) {
      InputEvent e = events[i];
      handle_input(&e);
    }

    if (num_e > 0) {
      // only render on input. this is pretty lazy but should work for now.
      render();
    }

    // poll the input queue, being updated by the other thread.
    usleep(1000);
  }

  clean_main();

  return 0;
}
