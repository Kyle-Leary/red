#pragma once

#include <execinfo.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "logging.h"
#include "main.h"

#define MAX_STACKFRAMES 25

#define INTERNAL_MESSAGE(sig_name_lit, sig_num)                                \
  "\n\n" sig_name_lit " sig %d: stacktrace - [thread %lu (%s)]\n", sig_num,    \
      pthread_self(), (main_thread == pthread_self()) ? "main" : "not main"

#define FAIL_LOUDLY(sig_name_lit, sig_num)                                     \
  {                                                                            \
    void *array[MAX_STACKFRAMES];                                              \
    size_t size;                                                               \
    size = backtrace(array, MAX_STACKFRAMES);                                  \
    system("clear");                                                           \
    log_printf("fatal error: " INTERNAL_MESSAGE(sig_name_lit, sig_num));       \
    fprintf(stderr, INTERNAL_MESSAGE(sig_name_lit, sig_num));                  \
    backtrace_symbols_fd(array, size, log_fd);                                 \
    backtrace_symbols_fd(array, size, STDERR_FILENO);                          \
    clean_main();                                                              \
    exit(1);                                                                   \
  }

// each thread needs sigs registered seperately. make sure we don't miss a good
// backtrace by registering them all on each thread.
void thread_sig_setup();
