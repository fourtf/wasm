#pragma once

#include <stdint.h>
#include <stdlib.h>

size_t wasm_alloc_count();
void wasm_alloc_inc();
void wasm_alloc_dec();

#define wasm_alloc(type) (wasm_alloc_inc(), (type *)malloc(sizeof(type)))
#define wasm_free(obj) (obj ? (wasm_alloc_dec(), free(obj)) : (void)obj)
