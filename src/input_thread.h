#pragma once

#include "whisper/queue.h"

#include <pthread.h>

#include <bits/pthreadtypes.h>

#define MAX_INPUT_EVENTS 256

// mostly ripped from GLFW's input mappings, so converting from that to this is
// cheap.
typedef enum InputType {
  INPUT_CHAR,
  INPUT_SPECIAL,

  INPUT_COUNT,
} InputType;

// all characters that must be parsed (and usually start with the \033) are
// handled here.
typedef enum SpecialInput {
  SPECIAL_INVALID = 0,

  F1,
  F2,
  F3,
  F4,
  F5,
  F6,
  F7,
  F8,
  F9,
  F10,
  F11,
  F12,

  UP,
  LEFT,
  RIGHT,
  DOWN,

  CTRL_UP,
  CTRL_LEFT,
  CTRL_RIGHT,
  CTRL_DOWN,

  SPECIAL_COUNT,
} SpecialInput;

typedef struct InputEvent {
  InputType type;
  union {
    char as_char;
  } data;
} InputEvent;

// represent inputs as a list of events in a queue, wrapped around by a mutex.
typedef struct InputQueue {
  pthread_mutex_t mutex;
  // a queue of InputEvents, the input_thread internally acts as a parser over
  // all the escape codes and generates safe-to-use Input events.
  WQueue queue;
} InputQueue;

extern InputQueue input;

// just start this thread and share the "input" global between the two with the
// mutex. the input thread should handle all terminal-stdin-configuration bs all
// by itself.
void *input_thread(void *data);

// i don't know how to make the read syscall stop blocking when we close the
// thread, so here. just hard cancel the thread and do de-init outside of the
// thread, called from main's cleaning routine.
void input_thread_clean();
