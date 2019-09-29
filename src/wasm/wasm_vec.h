#include <stdlib.h>

// XXX: in the future we have the ability to store sizeof(void*)/item_size
// elements in the capacity if that is required for performance. This could e.g.
// be 8 valtypes if they are size 1 on 64-Bit.
typedef struct {
  // The start points to the first element or NULL if the vec is empty.
  void *start;
  // The end points to one after the last element or NULL if the vec is empty.
  void *end;
  void *capacity;
  size_t item_size;
} wasm_vec;

void _wasm_vec_init(wasm_vec *vec, size_t item_size);
#define wasm_vec_init(vec_ptr, type) _wasm_vec_init(vec_ptr, sizeof(type))
void *wasm_vec_append(wasm_vec *vec);
void *wasm_vec_get(wasm_vec *vec, size_t index);
void wasm_vec_deinit(wasm_vec *vec);

void wasm_vec_for_each(wasm_vec *vec, void (*func)(void *));
size_t wasm_vec_size(wasm_vec *vec);