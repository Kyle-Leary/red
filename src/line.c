#include "line.h"
#include "macros.h"
#include "render.h"
#include <stdio.h>

void line_debug_status_print(Line *l, int up_to) {
  int max = MIN(up_to, LINE_BUF_SZ);

  char buf[LINE_BUF_SZ];
  char *ptr = buf;

  ptr += sprintf(ptr, "|");
  for (int i = 0; i < max; i++) {
    char ch = ((char *)l->buffer)[i];
    ptr += sprintf(ptr, "%c|", (ch == '\0') ? 'N' : ch);
  }

  status_printf("%s", buf);
}
