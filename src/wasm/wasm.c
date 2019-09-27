#include "wasm/wasm.h"

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void wasm_free_module(wasm_module *module) { free(module); }

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


bool wasm_load_module_sections(FILE *file, wasm_module *module) {
  assert(file);
  assert(module);
}

wasm_module *wasm_load_module_from_file(const char *file_name) {
  FILE *file = fopen(file_name, "rb");

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

  // Load sections.
  wasm_module *module = (wasm_module *)malloc(sizeof(wasm_module));

  if (wasm_load_module_sections(file, module)) {
    return module;
  } else {
    wasm_free_module(module);
    return NULL;
  }
}