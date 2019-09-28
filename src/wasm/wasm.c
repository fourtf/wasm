#include "wasm/wasm.h"

#include "wasm/wasm_common.h"
#include "wasm/wasm_helper.h"
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void wasm_free_module(wasm_module *module) { free(module); }

bool wasm_load_header(wasm_reader *reader, wasm_module_header *header) {
  size_t header_size = sizeof(wasm_module_header);

  // Read header.
  if (!wasm_read(reader, header, header_size)) {
    fprintf(stderr, "File too short.");
    return false;
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

bool wasm_load_module_sections(wasm_reader *reader, wasm_module *module) {
  assert(reader);
  assert(module);

  char section_type;
  char last_section = 0;

  while (wasm_read(reader, &section_type, 1)) {
    uint32_t length = wasm_read_leb_u32(reader);

    // All sections except for the custom section (id=0) must be in order.
    if (section_type != 0 && section_type <= last_section) {
      fprintf(stderr, "Invalid order of sections.");
      return false;
    }

    switch (section_type) {
    // There can be an unlimited number of custom sections (id=0) inbetween
    // other sections so we just skip them.
    case 0: // custom
    {
      if (!wasm_seek(reader, length)) {
        return false;
      }
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

    default:
      fprintf(stderr, "Invalid section found: %d", section_type);
      break;
    }
  }
}

wasm_module *wasm_load_module(wasm_reader *reader) {
  // Read header.
  wasm_module_header header;
  if (!wasm_load_header(reader, &header)) {
    return NULL;
  }

  // Load sections.
  wasm_module *module = wasm_alloc(wasm_module);

  if (wasm_load_module_sections(reader, module)) {
    return module;
  } else {
    wasm_free_module(module);
    return NULL;
  }
}

wasm_module *wasm_load_module_from_file(const char *file_name) {
  FILE *file = fopen(file_name, "rb");

  // Failed to open file.
  if (file == NULL) {
    perror("File error");
    return NULL;
  }

  // Wrap file in reader.
  wasm_reader reader;
  wasm_init_file_reader(&reader, file);
  fclose(file);
  return wasm_load_module(&reader);
}