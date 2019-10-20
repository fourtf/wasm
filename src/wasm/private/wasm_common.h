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

// Endianess
bool wasm_is_little_endian();

// unused
#define wasm_bswap_32(x)                                                       \
  ((x & 0x000000ff) << 24 | (x & 0x0000ff00) << 8 | (x & 0x00ff0000) >> 8 |    \
   (x & 0xff000000) >> 24)

#define wasm_bswap_64(x)                                                       \
  ((x & 0x00000000000000ff) << 56 | (x & 0x000000000000ff00) << 40 |           \
   (x & 0x0000000000ff0000) << 24 | (x & 0x00000000ff000000) << 8 |            \
   (x & 0x000000ff00000000) >> 8 | (x & 0x0000ff0000000000) >> 24 |            \
   (x & 0x00ff000000000000) >> 40 | (x & 0xff00000000000000) >> 56)
