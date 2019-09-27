#pragma once

// The header of a wasm module.
typedef struct {
  char magic_number[4];
  char version[4];
} wasm_module_header;

// All data contained in a wasm module.
typedef struct {
  wasm_module_header header;
} wasm_module;

// A wasm program is called a "module". It contains sections similar to how an
// elf/PE executable.
wasm_module *wasm_load_module_from_file(const char *file_name);
void wasm_free_module(wasm_module *module);

void wasm_run_tests();