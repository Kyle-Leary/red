#pragma once

#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

#define FORK_FAILURE()                                                         \
  {                                                                            \
    fprintf(stderr, "Failed to fork child process.\n");                        \
    exit(1);                                                                   \
  }

// like && in shell. the parent blocks until it knows the exit status of the
// child, then status is in the status variable. we exit if the status is
// nonzero.
#define THEN(...)                                                              \
  {                                                                            \
    int status;                                                                \
    wait(&status);                                                             \
    if (status != 0) {                                                         \
      exit(127);                                                               \
    } else {                                                                   \
      __VA_ARGS__                                                              \
    }                                                                          \
  }

// just wait, don't care about the status code.
#define WAIT()                                                                 \
  {                                                                            \
    int status;                                                                \
    wait(&status);                                                             \
  }

// like ||
#define IF_FAILED(...)                                                         \
  {                                                                            \
    int status;                                                                \
    wait(&status);                                                             \
    if (status != 0) {                                                         \
      __VA_ARGS__                                                              \
    }                                                                          \
  }

#define EXEC(abs_path_literal, ...)                                            \
  {                                                                            \
    char *arg[] = {abs_path_literal, __VA_ARGS__, NULL};                       \
    execve(abs_path_literal, arg, envp);                                       \
    exit(127);                                                                 \
  }

#define RUN_INTERNAL(block, abs_path_literal, ...)                             \
  {                                                                            \
    int clean_pid = fork();                                                    \
    if (clean_pid == -1) {                                                     \
      FORK_FAILURE();                                                          \
    } else if (clean_pid == 0) {                                               \
      block;                                                                   \
      EXEC(abs_path_literal, __VA_ARGS__);                                     \
    }                                                                          \
  }

#define RUN(abs_path_literal, ...)                                             \
  { RUN_INTERNAL({}, abs_path_literal, __VA_ARGS__); }

#define RUN_BG(abs_path_literal, ...)                                          \
  {                                                                            \
    RUN_INTERNAL({ setsid(); }, abs_path_literal, __VA_ARGS__);                \
  }
