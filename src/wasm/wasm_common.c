#include "wasm_common.h"
#include "assert.h"

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