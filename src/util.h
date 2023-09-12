#pragma once

int fd_redirect(int target_fd, int replacement_fd);
int fd_restore();
