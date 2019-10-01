#pragma once

#include "wasm/wasm_reader.h"
#include "wasm/wasm_vec.h"
#include "wasm_common.h"
#include <stdint.h>

// Function type.
// XXX: since there is only 4 values it might be worth considering using a char
// and #defines instead.
enum wasm_valtype {
  wasm_valtype_error,
  wasm_valtype_i32,
  wasm_valtype_i64,
  wasm_valtype_f32,
  wasm_valtype_f64,
};

typedef struct {
  // Storing `wasm_valtype`.
  wasm_vec param_types;
  enum wasm_valtype result_type;
} wasm_function_type;

typedef uint32_t wasm_typeidx;
// Storing `unsigned char`.
typedef wasm_vec wasm_expr;

typedef struct {
  enum wasm_valtype type;
  bool is_mutable;
  wasm_expr initializer;
} wasm_global;

enum wasm_export_type {
  wasm_export_func,
  wasm_export_table,
  wasm_export_mem,
  wasm_export_global
};

typedef struct {
  char *name;
  enum wasm_export_type type;
  uint32_t idx;
} wasm_export;

typedef struct {
  uint32_t n;
  enum wasm_valtype type;
} wasm_locals;

typedef struct {
  // Storing `wasm_locals`.
  wasm_vec locals;
  wasm_expr expr;
} wasm_code;

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
  // Storing `wasm_function_type`.
  wasm_vec function_types;
  // Storing `wasm_typeidx`.
  wasm_vec funcs;
  // Storing `wasm_global`.
  wasm_vec globals;
  // Storing `wasm_export`.
  wasm_vec exports;
  // Storing `wasm_code`.
  wasm_vec codes;
} wasm_module;

// A wasm program is called a "module". It contains sections similar to how an
// elf/PE executable.
wasm_module *wasm_load_module(wasm_reader *reader);
wasm_module *wasm_load_module_from_file(const char *file_name);
void wasm_free_module(wasm_module *module);

void wasm_print_module(wasm_module *module);