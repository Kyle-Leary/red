#include "filetype.h"
#include "whisper/colmap.h"
#include <string.h>

// a hashmap from strings to Filetypes.
static WColMap filetype_map;

char filetype_strings[FTYPE_COUNT][MAX_FILETYPE_STRLEN] = {0};

void init_filetypes() {
  w_create_cm(&filetype_map, sizeof(Filetype), 509);

#define INSERT(strlit, variant)                                                \
  {                                                                            \
    Filetype ft = variant;                                                     \
    strncpy(filetype_strings[variant], strlit, MAX_FILETYPE_STRLEN);           \
    w_cm_insert(&filetype_map, strlit, &ft);                                   \
  }

  INSERT("c", FTYPE_C);
}

char *get_filetype_string(Filetype ft) { return filetype_strings[ft]; }

Filetype get_filetype(const char *filepath) {
  char buf[256];
  // any strcpy'd buffer is also null terminated.
  strcpy(buf, filepath);

  char *first_dot = strchr(filepath, '.');
  if (first_dot == NULL) {
    // there was no dot in the filepath.
    return FTYPE_UNKNOWN;
  }

  // the rest of the stuff after the dot is retained in the first_dot pointer.
  // use that as the pointer to the filepath and retrieve it from the hashtable.
  Filetype *ft = w_cm_get(&filetype_map, first_dot + 1);
  if (ft == NULL) {
    return FTYPE_UNKNOWN;
  } else {
    return *ft;
  }
}

void clean_filetypes() { w_free_cm(&filetype_map); }
