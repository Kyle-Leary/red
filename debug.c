#include <stdio.h>

#include "shell.h"

static int port = 1234;
static int cores = 15;
static const char target[] = "./red";
// your gdb wrapper of choice.
static const char gdb_program[] = "/usr/local/bin/gf2";

int main(int argc, char **argv, char **envp) {
  char gdbserver_port[16], gdb_cmd[64], make_cores[8];
  sprintf(gdbserver_port, ":%d", port);
  // we don't need to wrap it in "", since we can just pass it as one arg
  // through the syscall here. in normal sh, passing with "" skips word
  // splitting of the string into the args.
  sprintf(gdb_cmd, "target remote localhost:%d", port);
  sprintf(make_cores, "-j%d", cores);

  RUN("/usr/bin/make", "clean");
  THEN();
  RUN("/usr/bin/make", make_cores);
  THEN();

  // the debugger opens in a new X window in the background.
  // if the debugger doesn't have a window, then run this in some X terminal.
  RUN((char *)gdb_program, "-ex", gdb_cmd);

  // then, pass the current terminal over to gdbserver.
  // gdbserver kills itself after recieving one connection terminated,
  // specifically the one from gdb_program above.
  RUN("/usr/bin/gdbserver", "--once", gdbserver_port, (char *)target);
  return 0;
}
