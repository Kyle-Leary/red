#include "sig.h"

void sigsegv(int sig) { FAIL_LOUDLY("SIGSEGV", sig); }
void sigabrt(int sig) { FAIL_LOUDLY("SIGABRT", sig); }
void sigfpe(int sig) { FAIL_LOUDLY("SIGFPE", sig); }
void sigill(int sig) { FAIL_LOUDLY("SIGILL", sig); }

void thread_sig_setup() {
  signal(SIGSEGV, sigsegv);
  signal(SIGABRT, sigsegv);
  signal(SIGFPE, sigfpe);
  signal(SIGILL, sigill);
}
