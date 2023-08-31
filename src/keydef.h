#pragma once

#define ESC_KEY (27)

// get the lower 5 bits of the character, that's the ctrl variant in ASCII.
#define CTRL(ch) (ch & 0x1F)

#define BACKSPACE (127)
