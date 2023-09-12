#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

int log_fd = -1;

void log_init() {
  log_fd = open("log.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
  if (log_fd == -1) {
    perror("Error opening log file");
    exit(1);
  }
  char *msg = "starting red\n";
  write(log_fd, msg, strlen(msg));
}

void log_clean() {
  char *msg = "leaving red\n";
  write(log_fd, msg, strlen(msg));

  if (log_fd != -1) {
    close(log_fd);
    log_fd = -1;
  }
}

void log_printf(const char *format, ...) {
  if (log_fd == -1) {
    return;
  }

  char buffer[512];
  time_t current_time = time(NULL);
  int length = snprintf(buffer, sizeof(buffer), "%ld: ", current_time);

  va_list args;
  va_start(args, format);
  length += vsnprintf(buffer + length, sizeof(buffer) - length, format, args);
  va_end(args);

  write(log_fd, buffer, length);
}
