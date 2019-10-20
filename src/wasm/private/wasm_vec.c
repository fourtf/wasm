#include "wasm/wasm_vec.h"
#include "wasm/wasm_common.h"
#include <assert.h>
#include <stdio.h>

#define wasm_vec_first_capacity 8

void _wasm_vec_init(wasm_vec *vec, size_t item_size) {
  // We set start and end to NULL so you can still iterate.
  vec->start = NULL;
  vec->end = NULL;
  vec->capacity = NULL;
  vec->item_size = item_size;
}

void wasm_vec_deinit(wasm_vec *vec) {
  if (vec->start) {
    wasm_free(vec->start);
  }

  vec->start = NULL;
  vec->end = NULL;
  vec->capacity = NULL;
}

void *wasm_vec_append(wasm_vec *vec) {
  if (vec->start == NULL) {
    // Empty vec.
    size_t n = vec->item_size * wasm_vec_first_capacity;
    // XXX: Not handling alloc failure here.
    vec->start = wasm_alloc_n(n);
    vec->end = vec->start + vec->item_size;
    vec->capacity = vec->start + n;
    return vec->start;
  } else if (vec->end == vec->capacity) {
    // No space in buffer.
    size_t n = vec->end - vec->start;
    vec->start = wasm_realloc_n(vec->start, n * 2);
    vec->end = vec->start + n;
    vec->capacity = vec->start + n * 2;
    return vec->end - 1;
  } else {
    // There is space in buffer.
    vec->end += vec->item_size;
    return vec->end - vec->item_size;
  }
}

void *wasm_vec_get(wasm_vec *vec, size_t index) {
  assert(vec->start + (index * vec->item_size) < vec->end);
  return vec->start + (index * vec->item_size);
}

void wasm_vec_for_each(wasm_vec *vec, void (*func)(void *)) {
  for (void *it = vec->start; it != vec->end; it += vec->item_size) {
    func(it);
  }
}

size_t wasm_vec_size(wasm_vec *vec) {
  return (vec->end - vec->start) / vec->item_size;
}
