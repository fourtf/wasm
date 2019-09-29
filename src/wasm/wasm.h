#pragma once

#include "wasm/wasm_reader.h"
#include "wasm/wasm_vec.h"
#include "wasm_common.h"
#include <stdint.h>

// Function type.
// XXX: since there is only 4 values it might be worth considering using a char
// and #defines instead.
enum wasm_valtype {
  wasm_valtype_i32,
  wasm_valtype_i64,
  wasm_valtype_f32,
  wasm_valtype_f64,
};

typedef struct {
  // Storing `wasm_valtype`
  wasm_vec param_types;
  enum wasm_valtype result_type;
} wasm_function_type;

// void wasm_init_function_type_params(wasm_function_type *type, size_t
// param_count);
void wasm_release_function_type(wasm_function_type *type);

// The header of a wasm module.
typedef struct {
  char magic_number[4];
  char version[4];
} wasm_module_header;

// All data contained in a wasm module.
typedef struct {
  wasm_module_header header;
  // Storing wasm_function_type.
  wasm_vec function_types;
} wasm_module;

// A wasm program is called a "module". It contains sections similar to how an
// elf/PE executable.
wasm_module *wasm_load_module(wasm_reader *reader);
wasm_module *wasm_load_module_from_file(const char *file_name);
void wasm_free_module(wasm_module *module);
