#pragma once

#include "util.h"

extern int log_fd;

void log_init();
void log_clean();
void log_printf(const char *format, ...);

// everything printed to stdout in this macro will instead be printed to the log
// file.
#define LOG(...)                                                               \
  {                                                                            \
    if (fd_redirect(STDOUT_FILENO, log_fd) == 0) {                             \
      do {                                                                     \
        __VA_ARGS__                                                            \
      } while (0);                                                             \
      if (fd_restore() != 0) {                                                 \
        log_printf(stderr, "Failed to restore stdout to logfile.\n");          \
      }                                                                        \
    } else {                                                                   \
      log_printf(stderr, "Failed to redirect stdout to logfile.\n");           \
    }                                                                          \
  }
