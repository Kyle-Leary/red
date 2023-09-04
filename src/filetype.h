#pragma once

typedef enum Filetype {
  FTYPE_UNKNOWN = 0,

  FTYPE_BUFFER, // generic buffer, cannot save.
  FTYPE_C,      // '*.c' files.

  FTYPE_COUNT,
} Filetype;

#define MAX_FILETYPE_STRLEN 16

// an array of filetype strings, indexed by their enum variant.
extern char filetype_strings[FTYPE_COUNT][MAX_FILETYPE_STRLEN];

char *get_filetype_string(Filetype ft);

void init_filetypes();
void clean_filetypes();

Filetype get_filetype(const char *filepath);
