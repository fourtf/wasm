#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

// We count the number of allocations and deallocations to find leaks. The tests
// check the number of allocated objects after each test.
size_t wasm_alloc_count();
void wasm_alloc_inc();
void wasm_alloc_dec();

// Alloc/free functions that alloc an object or an array of object of a certain
// type.
#define wasm_alloc(type) (wasm_alloc_inc(), (type *)malloc(sizeof(type)))
#define wasm_alloc_array(type, size)                                           \
  (wasm_alloc_inc(), (type *)malloc(sizeof(type) * size))
void wasm_free(void *obj);

// Alloc functions that alloc `n` bytes.
#define wasm_alloc_n(n) (wasm_alloc_inc(), malloc(n))
#define wasm_realloc_n(ptr, size) (realloc(ptr, size))
