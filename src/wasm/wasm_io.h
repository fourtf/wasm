#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

// An abstract device that can read and seek.
typedef struct _wasm_reader_t {
  void *device;
  size_t size;

  // Returns true on success, false on failure.
  // Buffer can NULL, then it is considered a seek.
  _Bool (*read)(struct _wasm_reader_t *reader, void *buffer, size_t amount);
} wasm_read_device;

// File reader. You will have to close the file yourself.
void wasm_init_file_device(wasm_read_device *reader, FILE *file);
void wasm_init_memory_device(wasm_read_device *reader, const void *data,
                             size_t size);