#include "commands.h"
#include "global.h"
#include "input_thread.h"
#include "keydef.h"
#include "mode.h"
#include "render.h"

#include "text.h"
#include "whisper/colmap.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// write to a single public command buffer.
CommandBuffer command;

#define MAX_WORDS 16
#define MAX_WORD_SIZE 256

typedef struct CommandInput {
  int argc;
  char argv[MAX_WORDS][MAX_WORD_SIZE];
  char joined_argv[(MAX_WORDS - 1) * MAX_WORD_SIZE];
} CommandInput;

typedef void (*command_fn)(CommandInput *);

// map over these in a hashtable to do really fast keyword matching.
typedef struct Command {
  command_fn fn;
} Command;

static WColMap command_map;

// fill in the provided buffer based on the string input. input should be null
// termed.
static void split_string(CommandInput *dest, char *input) {
  memset(dest, 0, sizeof(CommandInput));

  int which_word = 0; // which word buffer are we writing into?
  int word_ch_index =
      0; // which char of the current word buffer are we writing into?

  int i = 0;
  char ch = input[i];
  while (ch != '\0') {
#define NEXT_WORD()                                                            \
  {                                                                            \
    which_word++;                                                              \
    word_ch_index = 0;                                                         \
  }

    ch = input[i];
    switch (ch) {

    case ' ': {
      NEXT_WORD();
      if (which_word > MAX_WORDS) {
        fprintf(stderr, "Could not parse %s, too many words.\n", input);
      }
    } break;

    default: {
      dest->argv[which_word][word_ch_index] = ch;
      word_ch_index++;

      if (word_ch_index > MAX_WORD_SIZE) {
        fprintf(stderr,
                "Could not parse %s, too many characters in one word.\n",
                input);
      }
    } break;
    }

    i++;
#undef NEXT_WORD
  }

  dest->argc = which_word + 1;

  char *joined_ptr = dest->joined_argv;
  for (int i = 1; i < dest->argc - 1; i++) {
    joined_ptr += sprintf(joined_ptr, "%s ", dest->argv[i]);
  }
  sprintf(joined_ptr, "%s", dest->argv[dest->argc - 1]);
}

void command_run(char *command, int len) {
  if (len <= 0) {
    return;
  }

  if (command[0] == '!') {
    status_printf("Running in the shell: '%s'", command + 1);

    // exec the command natively on the system and capture the output,
    // printing it to the status bar.
    FILE *fp;
    char output[1024]; // Change the size according to your needs

    fp = popen(command + 1, "r");
    if (fp == NULL) {
      status_printf("Failed to run command.");
      return;
    }

    /* Read the output a line at a time - output it. */
    while (fgets(output, sizeof(output), fp) != NULL) {
      status_printf("Output: %s", output);
    }

    pclose(fp);

    return;
  }

  // else, just run the command specified in the command hashtable.
  CommandInput cmd_buf;
  split_string(&cmd_buf, command);
  if (cmd_buf.argc == 0)
    return;

  Command *c = (Command *)w_cm_get(&command_map, cmd_buf.argv[0]);
  if (c) {
    c->fn(&cmd_buf);
  } else {
    status_printf("Command not found.");
  }
}

void handle_command_input(InputEvent *e) {
  switch (e->type) {
  case INPUT_CHAR: {
    char ch = e->data.as_char;
    switch (ch) {

    case BACKSPACE: {
      if (command.len <= 0) {
        break;
      }

      command.len--;
      command.buf[command.len] = '\0';
    } break;

    case '\n': {
      // run directly from the status message buffer.
      command_run(command.buf, command.len);
      change_mode(NORMAL);
      // blank out the buffer.
      memset(command.buf, 0, STATUS_MSG_BUF_SZ);
      command.len = 0;
    } break;

    default: {
      // append to the combuf
      command.buf[command.len] = ch;
      command.len++;
    } break;
    }
  } break;

  case INPUT_SPECIAL: {
  } break;

  default: {
  } break;
  }
}

#define TODO()                                                                 \
  { status_printf("[%s] COMMAND NOT IMPLEMENTED ( YET :) )", command); }

void echo(CommandInput *cmd) { status_printf("%s", cmd->joined_argv); }

void write(CommandInput *cmd) { text_save(); }

void quit(CommandInput *cmd) { should_quit = true; }

void writequit(CommandInput *cmd) {
  write(cmd);
  quit(cmd);
}

void edit(CommandInput *cmd) { text_open(cmd->joined_argv); }

#undef TODO

#define INSERT(name, func)                                                     \
  { w_cm_insert(&command_map, name, &(Command){.fn = func}); }

void init_commands() {
  w_create_cm(&command_map, sizeof(Command), 509);

  INSERT("q", quit);
  INSERT("quit", quit);

  INSERT("w", write);
  INSERT("write", write);

  INSERT("wq", writequit);

  INSERT("e", edit);
  INSERT("edit", edit);

  INSERT("echo", echo);
}
