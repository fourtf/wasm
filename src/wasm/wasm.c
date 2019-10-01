#include "wasm/wasm.h"

#include "wasm/wasm_common.h"
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static bool wasm_char_to_valtype(unsigned char c, enum wasm_valtype *out) {
  switch (c) {
  case 0x7F:
    return *out = wasm_valtype_i32, true;
  case 0x7E:
    return *out = wasm_valtype_i64, true;
  case 0x7D:
    return *out = wasm_valtype_f32, true;
  case 0x7C:
    return *out = wasm_valtype_f64, true;
  default:
    return false;
  }
}

bool wasm_read_valtype(wasm_reader *reader, enum wasm_valtype *out) {
  unsigned char c;
  if (!wasm_read_obj(reader, &c)) {
    return false;
  }
  return wasm_char_to_valtype(c, out);
}

const char *wasm_valtype_to_str(enum wasm_valtype type) {
  switch (type) {
  case wasm_valtype_i32:
    return "i32";
  case wasm_valtype_i64:
    return "i64";
  case wasm_valtype_f32:
    return "f32";
  case wasm_valtype_f64:
    return "f64";
  default:
    return "invalid";
  }
}

void wasm_release_function_type(wasm_function_type *type) {
  wasm_vec_deinit(&type->param_types);
}

void wasm_init_global(wasm_global *global) {
  wasm_vec_init(&global->initializer, wasm_global);
}

void wasm_deinit_global(wasm_global *global) {
  wasm_vec_deinit(&global->initializer);
}

void wasm_init_export(wasm_export *export) { export->name = NULL; }

void wasm_deinit_export(wasm_export *export) { wasm_free(export->name); }

void wasm_init_code(wasm_code *code) {
  wasm_vec_init(&code->locals, wasm_locals);
  wasm_vec_init(&code->expr, wasm_expr);
}

void wasm_deinit_code(wasm_code *code) {
  wasm_vec_deinit(&code->locals);
  wasm_vec_deinit(&code->expr);
}

void wasm_free_module(wasm_module *module) {
  if (module) {
    wasm_vec_for_each(&module->function_types,
                      (void (*)(void *))(&wasm_release_function_type));
    wasm_vec_deinit(&module->function_types);
    wasm_vec_deinit(&module->funcs);
    wasm_vec_for_each(&module->globals,
                      (void (*)(void *))(&wasm_deinit_global));
    wasm_vec_deinit(&module->globals);
    wasm_vec_for_each(&module->exports, (void(*))(void *)(&wasm_deinit_export));
    wasm_vec_deinit(&module->exports);
    wasm_vec_for_each(&module->codes, (void (*)(void *))(&wasm_deinit_code));
    wasm_vec_deinit(&module->codes);

    wasm_free(module);
  }
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

bool wasm_read_expr(wasm_reader *reader, wasm_vec *expr) {
  while (1) {
    unsigned char command;
    if (!wasm_read(reader, &command, 1)) {
      fprintf(stderr, "IO error while parsing expr.\n");
    }

    if (command == 0x0B) {
      return true;
    }

    *((unsigned char *)wasm_vec_append(expr)) = command;
  }

  return false;
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
            enum wasm_valtype *type = wasm_vec_append(&func->param_types);

            if (!wasm_read_valtype(reader, type)) {
              fprintf(stderr, "Error while reading param type.");
              return false;
            }
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
          if (!wasm_read_valtype(reader, &func->result_type)) {
            fprintf(stderr, "Error while reading result type.");
            return false;
          }
        }
      }
    } break;

    // func section.
    case 3: {
      uint32_t func_count = wasm_read_leb_u32(reader);

      for (size_t i = 0; i < func_count; i++) {
        wasm_typeidx *func = wasm_vec_append(&module->funcs);
        *func = wasm_read_leb_u32(reader);
      }
    } break;

    // global section.
    case 6: {
      uint32_t global_count = wasm_read_leb_u32(reader);

      for (size_t i = 0; i < global_count; i++) {
        wasm_global *global = wasm_vec_append(&module->globals);
        wasm_init_global(global);

        // type
        global->type = wasm_read_leb_u32(reader);

        // mutablility
        unsigned char c;
        wasm_read(reader, &c, 1);
        if (c > 1) {
          fprintf(stderr, "Level of mutability not supported (%u)", c);
          return false;
        }
        global->is_mutable = c == 1;

        // initializer
        if (!wasm_read_expr(reader, &global->initializer)) {
          return false;
        }
      }
    } break;

    // exports
    case 7: {
      uint32_t export_count = wasm_read_leb_u32(reader);

      for (size_t i = 0; i < export_count; i++) {
        wasm_export *export = wasm_vec_append(&module->exports);

        // name
        if (!wasm_read_string(reader, &export->name)) {
          fprintf(stderr, "Error reading export name.\n");
          return false;
        }

        // export description
        unsigned char c;
        if (!wasm_read(reader, &c, 1)) {
          fprintf(stderr, "Error reading export description.\n");
          return false;
        }

        if (c > 4) {
          switch (c) {
          case 0:
            export->type = wasm_export_func;
            break;
          case 1:
            export->type = wasm_export_table;
            break;
          case 2:
            export->type = wasm_export_mem;
            break;
          case 3:
            export->type = wasm_export_global;
            break;
          default:
            fprintf(stderr, "Invalid export description found.\n");
            return false;
          }
        }

        // export index
        export->idx = wasm_read_leb_u32(reader);
      }
    } break;

    // Code segements. Contains a vector of
    case 10: {
      uint32_t func_count = wasm_read_leb_u32(reader);

      for (size_t i_func = 0; i_func < func_count; i_func++) {
        // This variable may be used for skipping but we don't need it.
        /*uint32_t code_size =*/wasm_read_leb_u32(reader);

        wasm_code *code = wasm_vec_append(&module->codes);
        wasm_init_code(code);

        // vec<locals>.
        uint32_t local_count = wasm_read_leb_u32(reader);
        for (size_t i_local = 0; i_local < local_count; i_local++) {
          wasm_locals *locals = wasm_vec_append(&code->locals);

          if (!wasm_read_leb_u32_2(reader, &locals->n) ||
              !wasm_read_valtype(reader, &locals->type)) {
            fprintf(stderr, "Error while reading local variable definition.");
            return false;
          }
        }

        // expr.
        wasm_read_expr(reader, &code->expr);
      }
    } break;

    case 2:  // import
    case 4:  // table
    case 5:  // mem
    case 8:  // start
    case 9:  // elem
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
  wasm_vec_init(&module->funcs, wasm_typeidx);
  wasm_vec_init(&module->globals, wasm_global);
  wasm_vec_init(&module->exports, wasm_export);
  wasm_vec_init(&module->codes, wasm_code);

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

void wasm_print_module(wasm_module *module) {
  // types
  for (wasm_function_type *type = module->function_types.start;
       type != module->function_types.end; type++) {
    printf("type: params( ");
    for (enum wasm_valtype *param = type->param_types.start;
         param != type->param_types.end; param++) {
      printf("%s ", wasm_valtype_to_str(*param));
    }
    printf(") returns( %s )\n", wasm_valtype_to_str(type->result_type));
  }

  // funcs
  for (uint32_t *idx = module->funcs.start; idx != module->funcs.end; idx++) {
    printf("func with type %u\n", *idx);
  }
}