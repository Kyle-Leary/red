#include "input_thread.h"
#include "string.h"
#include <termios.h>
#include <unistd.h>

#include "global.h"

struct termios original_term_settings;

InputQueue input = {
    .queue = {0},
    .mutex = PTHREAD_MUTEX_INITIALIZER,
};

// read directly from stdin on another thread so it doesn't block.
void *input_thread(void *data) {
  {
    // usually, unix manages a newline-terminated buffer for stdin where our
    // input only updates when the newline is met. disable that, and take in
    // stdin interactively.
    struct termios term;
    // attributes of a terminal are set directly on a file descriptor.
    tcgetattr(STDIN_FILENO, &term);
    // keep around the old settings so that we can reset them.
    memcpy(&original_term_settings, &term, sizeof(struct termios));
    // set the local modes of the input, don't echo.
    // ~ICANON - disable canonical mode, so we don't newline buffer.
    term.c_lflag &= ~(ICANON | ECHO);
    // TCSAFLUSH - flush the input buffers before updating the settings.
    // basically a pipeline sync.
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &term);
  }

  // use a buffer over stdin, since we need to read codes that are longer than
  // one char at times.
  char buf[256];

  for (;;) {
    // this blocks until we read a character.
    int n_read = read(STDIN_FILENO, buf, sizeof(buf));

    int num_events_parsed = 0;
    InputEvent e_buffer[MAX_INPUT_EVENTS];

    { // PARSING LOOP
      char *ptr = buf;

      // the trickiest part of this is that we might recieve a partial code.
      for (int i = 0; i < n_read; i++) {
        InputEvent *e = &e_buffer[i];

        e->type = INPUT_CHAR;
        e->data.as_char = ptr[0];

        ptr++; // move a character through the input buffer.
        num_events_parsed++;
      }
    }

    { // critical section, write an event into the queue.
      pthread_mutex_lock(&input.mutex);
      for (int i = 0; i < num_events_parsed; i++) {
        // write all the events that we parsed into here. don't parse directly
        // in the critical section because we don't need to.
        w_enqueue(&input.queue, &e_buffer[i]);
      }
      pthread_mutex_unlock(&input.mutex);
    }
  }

  return NULL;
}

void input_thread_clean() {
  // restore the old terminal settings
  // TCSANOW - set it NOW, don't do anything else.
  tcsetattr(STDIN_FILENO, TCSANOW, &original_term_settings);
}
