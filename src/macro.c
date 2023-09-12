#include "macro.h"
#include "input_thread.h"
#include "keydef.h"
#include "render.h"
#include "whisper/queue.h"
#include <pthread.h>
#include <string.h>
#include <unistd.h>

// all of these are NULLed out by default.
char macro_buf[26][MAX_MACRO_LEN] = {0};

#define GET_IDX(ch) (ch - 'a')
#define IS_VALID(ch) (ch >= 'a' && ch <= 'z')

char curr_macro = '\0';

void macro_start_recording(char ch) {
  if (curr_macro != '\0') {
    status_printf("Already recording macro '%c'.\n", curr_macro);
  }

  if (IS_VALID(ch)) {
    curr_macro = ch;
    char *buf = macro_buf[GET_IDX(ch)];
    memset(buf, 0, MAX_MACRO_LEN);
    status_printf("Recording macro '%c'.\n", ch);
  } else {
    status_printf("Invalid macro '%c', cannot record.\n", ch);
  }
}

void macro_stop_recording() {
  if (curr_macro == '\0') {
    status_printf("Not recording a macro.\n");
    return;
  }

  // then, trim the last 'q' character off of the macro.
  char *buf = macro_buf[GET_IDX(curr_macro)];
  buf[strlen(buf) - 1] = '\0';

  status_printf("Stopped recording macro '%c'.\n", curr_macro);
  curr_macro = '\0';
}

void macro_handle_input(InputEvent *e) {
  if (curr_macro == '\0') {
    return;
  }

  char *buf = macro_buf[GET_IDX(curr_macro)];
  switch (e->type) {
  case INPUT_CHAR: {
    char ch = e->data.as_char;
    switch (ch) {

    case '\n': {
      macro_stop_recording();
    } break;

    default: {
      // append to the combuf
      buf[strlen(buf)] = ch;
    } break;
    }
  } break;
  default: {
  } break;
  }
}

void macro_play(char ch) {
  if (!IS_VALID(ch)) {
    status_printf("Invalid macro '%c', cannot playback.\n", ch);
    return;
  }

  // find the buffer, then write all the contents to stdin directly.
  char *buf = macro_buf[GET_IDX(ch)];
  status_printf("Playing macro '%c': '%s'.\n", ch, buf);

  for (int i = 0; i < strlen(buf); i++) {
    char ch = buf[i];
    InputEvent e = {0};
    e.type = INPUT_CHAR;
    e.data.as_char = ch;
    // ugly. probably move the input thread to an epoll instance on the main
    // thread at some point.
    pthread_mutex_lock(&input.mutex);
    w_enqueue(&input.queue, &e);
    pthread_mutex_unlock(&input.mutex);
  }
}
