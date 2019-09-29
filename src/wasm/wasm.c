#include "wasm/wasm.h"

#include "wasm/wasm_common.h"
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static bool wasm_is_valid_valtype(unsigned char c) {
  return c <= 0x7F || c >= 0x7C;
}

static bool wasm_char_to_valtype(unsigned char c) {
  switch (c) {
  case 0x7F:
    return wasm_valtype_i32;
  case 0x7E:
    return wasm_valtype_i64;
  case 0x7D:
    return wasm_valtype_f32;
  case 0x7C:
    return wasm_valtype_f64;
  default:
    abort();
  }
}

void wasm_release_function_type(wasm_function_type *type) {
  wasm_vec_deinit(&type->param_types);
}

void wasm_free_module(wasm_module *module) {
  wasm_vec_for_each(&module->function_types,
                    (void (*)(void *))(&wasm_release_function_type));
  wasm_vec_deinit(&module->function_types);

  wasm_free(module);
}

bool wasm_load_header(wasm_reader *reader, wasm_module_header *header) {
  size_t header_size = sizeof(wasm_module_header);

  // Read header.
  if (!wasm_read(reader, header, header_size)) {
    fprintf(stderr, "Error reading header.\n");
    return false;
  }

  // Check magic number.
  if (memcmp(&header->magic_number, "\0asm", 4) != 0) {
    fprintf(stderr, "Magic number is invalid.\n");
    return false;
  }

  // Check version.
  if (memcmp(&header->version, "\x01\0\0\0", 4) != 0) {
    fprintf(stderr, "Magic number is invalid.\n");
    return false;
  }

  return true;
}

bool wasm_load_module_sections(wasm_reader *reader, wasm_module *module) {
  assert(reader);
  assert(module);

  char section_type;
  char last_section_type =
      0; // Last section that we parsed (except custom section)

  // Keep reading sections until the reader ends.
  while (wasm_read(reader, &section_type, 1)) {
    uint32_t section_length = wasm_read_leb_u32(reader);

    // All sections except for the custom section (id=0) must be in order.
    if (section_type != 0 && section_type < last_section_type) {
      fprintf(stderr, "Invalid order of sections.\n");
      return false;
    }

    switch (section_type) {
    // Custom section. There can be an unlimited number of custom sections
    // (id=0) inbetween other sections. They can contain e.g. debugging
    // information. We just skip them.
    case 0: {
      if (!wasm_seek(reader, section_length)) {
        fprintf(stderr, "Error skipping custom section.\n");
        return false;
      }
    } break;
    // Function type section. A vector of function types.
    case 1: {
      uint32_t type_count = wasm_read_leb_u32(reader);

      for (size_t i = 0; i < type_count; i++) {
        // First byte needs to be 0x60.
        {
          char c;
          if (!wasm_read(reader, &c, 1)) {
            fprintf(stderr, "IO error loading function 0x60 byte.\n");
            return false;
          }

          if (c != 0x60) {
            fprintf(stderr, "Function needs to start with 0x60.\n");
            return false;
          }
        }

        // We create a function object.
        wasm_function_type *func =
            (wasm_function_type *)wasm_vec_append(&module->function_types);

        wasm_vec_init(&func->param_types, enum wasm_valtype);

        // Parameters.
        {
          uint32_t param_count = wasm_read_leb_u32(reader);

          for (size_t i = 0; i < param_count; i++) {
            char c;

            if (!wasm_read(reader, &c, 1)) {
              fprintf(stderr, "IO Error while reading parameters.");
              return false;
            }

            if (!wasm_is_valid_valtype(c)) {
              fprintf(stderr, "Invalid valtype in function definition.");
              return false;
            }

            *((enum wasm_valtype *)wasm_vec_append(&func->param_types)) =
                wasm_char_to_valtype(c);
          }
        }

        // Result type(s).
        {
          uint32_t result_count = wasm_read_leb_u32(reader);

          // There can only be one return type in the wasm spec right now.
          if (result_count != 1) {
            fprintf(stderr, "Only one result type supported.");
          }

          // No loop here since there can currently only be one return type.
          char c;
          wasm_read(reader, &c, 1);
        }
      }
    } break;
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
      fprintf(stderr, "Parsing section %u is not implemented yet\n",
              section_type);
      return false;

    default:
      fprintf(stderr, "Unknown section found: %u\n", section_type);
      return false;
    }

    last_section_type = section_type;
  }
  return true;
}

wasm_module *wasm_load_module(wasm_reader *reader) {
  // Read header.
  wasm_module_header header;
  if (!wasm_load_header(reader, &header)) {
    return NULL;
  }

  // Load sections. We free `module` ourselves on failure. `wasm_free_module`
  // handle deleting partially laoded modules.
  wasm_module *module = wasm_alloc(wasm_module);
  wasm_vec_init(&module->function_types, wasm_function_type);

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

  wasm_module *result = wasm_load_module(&reader);
  fclose(file);
  return result;
}