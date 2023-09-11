#pragma once

// one for each letter and a global one? not exactly sure how vi usually does
// this.
#include "line.h"
typedef enum Clipboards {
  CLIP_DEFAULT = 0,

  CLIP_A = 1,
  CLIP_B,
  CLIP_C,
  CLIP_D,
  CLIP_E,
  CLIP_F,
  CLIP_G,
  CLIP_H,
  CLIP_I,
  CLIP_J,
  CLIP_K,
  CLIP_L,
  CLIP_M,
  CLIP_N,
  CLIP_O,
  CLIP_P,
  CLIP_Q,
  CLIP_R,
  CLIP_S,
  CLIP_T,
  CLIP_U,
  CLIP_V,
  CLIP_W,
  CLIP_X,
  CLIP_Y,
  CLIP_Z,

  CLIP_GLOBAL,

  CLIP_COUNT,
} Clipboards;

#define MAX_CLIPBOARD_DATA 1024

extern char clipboards[CLIP_COUNT][MAX_CLIPBOARD_DATA];

void clip_copy(Clipboards c, const char *str);

void clip_copy_line(Clipboards c, Line *l);
