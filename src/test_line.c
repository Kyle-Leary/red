#include "line.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

void test_line() {
  Line l = {.gap_start = 0, .gap_end = 3, .buffer = {0}};
  strncpy(l.buffer, "    abcd", LINE_BUF_SZ);

  // Test for shifting left from an initially empty left buffer
  line_shift_left(&l);
  assert(l.gap_start == 0);
  assert(l.gap_end == 3);

  // Test for a normal shift left operation
  l.gap_start = 2;
  l.gap_end = 3;
  memcpy(l.buffer, "ab\0\0cd", 6);

  line_debug_print(&l, 6);
  line_shift_left(&l);
  line_debug_print(&l, 6);

  assert(l.gap_start == 1);
  assert(l.gap_end == 2);
  assert(memcmp(l.buffer, "a\0\0bcd", 6) == 0);

  line_debug_print(&l, 6);
  line_insert(&l, 'i');
  line_debug_print(&l, 6);
  line_delete(&l);
  line_debug_print(&l, 6);

  line_debug_print(&l, 6);
  line_shift_right(&l);
  line_debug_print(&l, 6);

  line_debug_print(&l, 6);
  line_shift_right(&l);
  line_debug_print(&l, 6);

  char buf[LINE_BUF_SZ] = {0};
  char *ptr = buf;
  // if we're on the current cursor line, then give the cursor a nice red
  // background.

  ptr += sprintf(ptr, "%s", l.buffer);
  ptr += sprintf(ptr, "%s", &l.buffer[l.gap_end + 1]);

  printf("%s\n", buf);

  assert(0);

  printf("All tests passed!\n");
}
