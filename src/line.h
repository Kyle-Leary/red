#pragma once

#include "whisper/gap_buffer.h"

typedef WGapBuf Line;

#define LINE_BUF_SZ 512

void line_debug_status_print(Line *l, int up_to);
