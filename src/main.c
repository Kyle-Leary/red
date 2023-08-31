#include "commands.h"
#include "input_thread.h"
#include "insert.h"
#include "mode.h"
#include "normal.h"
#include "render.h"
#include "test_line.h"
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

bool should_quit = false;

// write something into stdin every once in a while.
void *testing_thread(void *data) {
  for (;;) {
    usleep(1000000);
    write(STDIN_FILENO, "a", 1);
  }
  return NULL;
}

pthread_t test_thread = 0;
bool is_using_test_thread = false;

// define signal handlers.
void sigwinch(int num) {
  // the terminal emu/window has been resized.
  handle_resize();
}

int main(int argc, char **argv) {
  do {
    if (argc < 2) {
      break;
    }

    if (strncmp(argv[1], "test", 4) == 0) {
      test_line();

      pthread_create(&test_thread, NULL, testing_thread, NULL);
      is_using_test_thread = true;
    }
  } while (0);

  { // setup general term stuff
    signal(SIGWINCH, sigwinch);
    handle_resize();
  }

  init_render();

  init_commands();

  status_printf("welcome to red.");

  pthread_t input_thread_handle;
  { // setup threads
    w_make_queue(&input.queue, sizeof(InputEvent), MAX_INPUT_EVENTS);
    pthread_create(&input_thread_handle, NULL, input_thread, NULL);
  }

  texts_init();
  text_open("./testfile");

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

    render();

    // poll the input queue, being updated by the other thread.
    usleep(1000);
  }

  {
    pthread_cancel(input_thread_handle);
    input_thread_clean();

    if (is_using_test_thread) {
      pthread_cancel(test_thread);
    }
  }

  clean_render();

  return 0;
}
