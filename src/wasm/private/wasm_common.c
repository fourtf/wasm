#include "wasm_common.h"
#include "assert.h"

// #define wasm_is_power_of_2(x) ((x & (x - 1)) == 0)

static _Atomic size_t wasm_alloc_count_ = 0;

size_t wasm_alloc_count() { return wasm_alloc_count_; }
void wasm_alloc_inc() { wasm_alloc_count_++; }
void wasm_alloc_dec() { wasm_alloc_count_--; }

void wasm_free(void *obj) {
  if (obj) {
    wasm_alloc_dec();
    free(obj);
  }
}

bool wasm_is_little_endian() {
  unsigned int is_little_endian = 1;
  return ((int8_t *)&is_little_endian)[0];
}