/*

Module
    Section

*/

#include "wasm/wasm.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

bool wasm_load_header(FILE *file, wasm_module_header *header) {
  size_t header_size = sizeof(wasm_module_header);
  if (fread(header, 1, header_size, file) != header_size) {
    fprintf(stderr, "File too short.");
  }

  // Check magic number.
  if (memcmp(&header->magic_number, "\0asm", 4) != 0) {
    fprintf(stderr, "Magic number is invalid.");
    return false;
  }

  // Check version.
  if (memcmp(&header->version, "\x01\0\0\0", 4) != 0) {
    fprintf(stderr, "Magic number is invalid.");
    return false;
  }
}

// WASM allows a variable number of "custom sections" between the other
// sections, e.g. for debugging info.
void wasm_skip_customsecs() {}

wasm_module *wasm_load_module_from_file(const char *file_name) {
  FILE *file = fopen(file_name, "r");

  // Failed to open file.
  if (file == NULL) {
    fprintf(stderr, "Failed to open file.");
    return NULL;
  }

  // Read header.
  wasm_module_header header;
  if (!wasm_load_header(file, &header)) {
    return NULL;
  }

  // Return result.
  wasm_module *module = (wasm_module *)malloc(sizeof(wasm_module));
  module->header = header;
  return module;
}