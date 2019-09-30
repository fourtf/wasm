#pragma once

#include <stdbool.h>
#include <stdint.h>
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
void wasm_init_memory_reader(wasm_reader *reader, const void *data,
                             size_t size);

// Returns true on success, false on failure.
bool wasm_read(wasm_reader *, void *buffer, size_t amount);
bool wasm_seek(wasm_reader *, size_t amount);

// leb128 encoded number.
// XXX: Not sure how errors should be handled. Right now there is two different
// ways.
uint32_t wasm_read_leb_u32(wasm_reader *reader);

// Little endian raw floats.
bool wasm_read_f32(wasm_reader *reader, float *out);
bool wasm_read_f64(wasm_reader *reader, double *out);

// Reads a utf8 encoded string. Nothing is assigned to out if the operation
// fails.
bool wasm_read_string(wasm_reader *reader, char **out);