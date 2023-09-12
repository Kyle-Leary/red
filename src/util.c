#include "util.h"
#include "logging.h"

#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

static int fd_buffer = -1;
static int target_fd_storage = -1;

int fd_redirect(int target_fd, int replacement_fd) {
  int tmp_fd = dup(target_fd);
  if (tmp_fd == -1) {
    log_printf("Failed to dup target_fd\n");
    return 1;
  }

  if (dup2(replacement_fd, target_fd) == -1) {
    log_printf("Failed to dup2 replacement_fd\n");
    close(tmp_fd); // Close the temporarily duplicated fd
    return 1;
  }

  // If we get here, it means the operation succeeded, so we can update the
  // global state
  fd_buffer = tmp_fd;
  target_fd_storage = target_fd;

  return 0;
}

// restore the fd_buffer into target_fd_storage.
int fd_restore() {
  if (fd_buffer == -1 || target_fd_storage == -1) {
    log_printf("fd_buffer or target_fd_storage is -1, nothing to restore.\n");
    return 1;
  }

  if (dup2(fd_buffer, target_fd_storage) == -1) {
    log_printf("Failed to dup2 fd_buffer\n");
    return 1;
  }

  close(fd_buffer);

  fd_buffer = -1;
  target_fd_storage = -1;

  return 0;
}
