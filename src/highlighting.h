#pragma once

#include "line.h"
#include "termbuffer.h"

// compile and free regexes and etc
void init_highlighting();
void clean_highlighting();

void print_line(int row, int col, Line *l, Termbuffer *tb);
