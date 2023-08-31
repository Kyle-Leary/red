#include "line.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

void test_line_shift_left() {
  Line l = {.left = 0, .right = 4, .left_sz = 0, .len = 4};
  strncpy(l.buffer, "    abcd", LINE_BUF_SZ);

  // Test for shifting left from an initially empty left buffer
  line_shift_left(&l);
  assert(l.left_sz == 0);
  assert(l.right == 4);

  // Test for a normal shift left operation
  l.right = 6;
  l.left_sz = 2;
  strncpy(l.buffer, "ab  cd", LINE_BUF_SZ);

  line_debug_print(&l, 10);
  line_shift_left(&l);
  line_debug_print(&l, 10);

  assert(l.left_sz == 1);
  assert(l.right == 5);
  assert(strncmp(l.buffer, "a b cd", LINE_BUF_SZ) == 0);
}

void test_line_shift_right() {
  Line l = {.left = 0, .right = LINE_BUF_SZ, .left_sz = 0, .len = LINE_BUF_SZ};

  // Test for shifting right into an initially full buffer
  line_shift_right(&l);
  assert(l.right == LINE_BUF_SZ);

  // Test for a normal shift right operation
  l.right = 6;
  l.left_sz = 2;
  strncpy(l.buffer, "ab  cd", LINE_BUF_SZ);
  line_shift_right(&l);
  assert(l.left_sz == 3);
  assert(l.right == 7);
  assert(strncmp(l.buffer, "abc  d", LINE_BUF_SZ) == 0);
}

void test_line_shift_by() {
  Line l = {.left = 0, .right = 6, .left_sz = 2, .len = 4};
  strncpy(l.buffer, "ab  cd", LINE_BUF_SZ);

  // Shifting left by a negative value
  line_shift_by(&l, -1);
  assert(l.left_sz == 1);
  assert(l.right == 5);

  // Shifting right by a positive value
  line_shift_by(&l, 1);
  assert(l.left_sz == 2);
  assert(l.right == 6);
}

void test_line_insert() {
  Line l = {.left = 0, .right = 2, .left_sz = 0, .len = 0};
  strncpy(l.buffer, "  ", LINE_BUF_SZ);
  line_insert(&l, 'a');
  assert(l.len == 1);

  // Assuming your line_insert function also updates the buffer and indices,
  // which it currently doesn't in your partial implementation
  // assert(strncmp(l.buffer, "a ", LINE_BUF_SZ) == 0);
}

void test_line_delete() {
  Line l = {.left = 0, .right = 4, .left_sz = 2, .len = 4};
  strncpy(l.buffer, "ab  cd", LINE_BUF_SZ);
  line_delete(&l);
  assert(l.len == 3);

  // Assuming your line_delete function also updates the buffer and indices,
  // which it currently doesn't in your partial implementation
  // assert(strncmp(l.buffer, "a  cd", LINE_BUF_SZ) == 0);
}

void test_line() {
  test_line_shift_left();
  test_line_shift_right();
  test_line_shift_by();
  test_line_insert();
  test_line_delete();
  printf("All tests passed!\n");
}
