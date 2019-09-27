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

// Int are encoded as LEB128 https://en.wikipedia.org/wiki/LEB128
// XXX: handling fread errors?
uint32_t wasm_read_leb_u32(FILE *file) {
  assert(file);

  uint32_t result = 0;
  char c = 128;

  // Here we check if the last bit is 1 (& 128).
  // Per spec it has at most ceil(32 / 7) = 5 bytes.
  for (int i = 0; c & 128 && i < 5; c++) {
    fread(&c, 1, 1, file);
    result = result * 128 + c & 127;
  }

  return result;
}

bool wasm_load_module_sections(FILE *file, wasm_module *module) {
  assert(file);
  assert(module);

  char section_type;
  char last_section = 0;

  while (feof(file) == 0) {
    if (fread(&section_type, 1, 1, file) != 1) {
      // error
      return false;
    }

    uint32_t length = wasm_read_leb_u32(file);

    // There can be an unlimited number number of custom sections (id=0)
    // inbetween other sections. All other sections must be in order.
    if (section_type != 0 && section_type <= last_section) {
      fprintf(stderr, "Invalid order of sections.");
      return false;
    }

    switch (section_type) {
    case 0: // custom
    {
      static_assert(sizeof(long) > sizeof(uint32_t), "xD");
      // SEEK length
    } break;
    case 1:  // type
    case 2:  // import
    case 3:  // func
    case 4:  // table
    case 5:  // mem
    case 6:  // global
    case 7:  // export
    case 8:  // start
    case 9:  // elem
    case 10: // code
    case 11: // data
        ;

    default:;
    }
  }
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