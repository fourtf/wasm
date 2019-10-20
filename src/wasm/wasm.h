#pragma once

#include "wasm/wasm_io.h"
#include <stdint.h>

// A wasm program is called a "module". It contains sections similar to how an
// elf/PE executable.
wasm_module *wasm_load_module(wasm_read_device *reader);
wasm_module *wasm_load_module_from_file(const char *file_name);
void wasm_free_module(wasm_module *module);
void wasm_print_module(wasm_module *module);