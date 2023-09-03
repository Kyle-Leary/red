#include "shell.h"
#include <stdio.h>

static const char target[] = "./red";
// your gdb wrapper of choice.
static const char gdb_program[] = "/usr/local/bin/gf2";

int main(int argc, char **argv, char **envp) {
  RUN("/usr/bin/make", "clean");
  THEN();
  RUN("/usr/bin/make", "-j15");
  THEN();
  RUN((char *)target, NULL);
}
