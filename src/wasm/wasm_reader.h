#pragma once

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
} wasm_reader;

// File reader. You will have to close the file yourself.
void wasm_init_file_reader(wasm_reader *reader, FILE *file);
void wasm_init_memory_reader(wasm_reader *reader, void *data, size_t size);

// Returns true on success, false on failure.
bool wasm_read(wasm_reader *, void *buffer, size_t amount);
bool wasm_seek(wasm_reader *, size_t amount);