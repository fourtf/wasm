#pragma once

#include "wasm/wasm_io.h"
#include <setjmp.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

struct wasm_reader {
  wasm_read_device device;
  jmp_buf buf;
};

// Returns true on success, false on failure.
void wasm_read(wasm_reader *reader, void *buffer, size_t amount);
void wasm_seek(wasm_reader *reader, size_t amount);
#define wasm_read_obj(reader, ptr) wasm_read(reader, ptr, sizeof(*ptr))

void wasm_reader_fail(struct wasm_reader *reader);
#define wasm_reader_assert(reader, expr)                                       \
  if (!(expr)) {                                                               \
    wasm_reader_fail(reader);                                                  \
  }

uint32_t wasm_read_leb_u32(wasm_reader *reader);

// Little endian raw floats.
float wasm_read_f32(wasm_reader *reader);
double wasm_read_f64(wasm_reader *reader);

// Reads a utf8 encoded string. Nothing is assigned to out if the operation
// fails.
char *wasm_read_string(wasm_reader *reader, char *out);