#include "wasm/wasm_reader.h"

#include "wasm/wasm_common.h"
#include <assert.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

bool wasm_file_reader_read(wasm_reader *reader, void *buffer, size_t amount) {
  if (buffer) {
    // Read.
    return 1 == fread(buffer, amount, 1, (FILE *)reader->device);
  } else {
    // Seek.
    if (amount > INT_MAX) {
      fprintf(stderr, "wasm_file_reader_read: amount > INT_MAX\n");
      return false;
    } else {
      return fseek((FILE *)reader->device, amount, SEEK_CUR) == 0;
    }
  }
}

bool wasm_memory_reader_read(wasm_reader *reader, void *buffer, size_t amount) {
  if (amount > reader->size) {
    return false;
  } else {
    if (buffer != NULL) {
      memcpy(buffer, reader->device, amount);
    }
    reader->device += amount;
    reader->size -= amount;
    return true;
  }
}

void wasm_init_file_reader(wasm_reader *reader, FILE *file) {
  reader->device = file;
  reader->size = 0; // Size is ignored in file reader.
  reader->read = &wasm_file_reader_read;
}

void wasm_init_memory_reader(wasm_reader *reader, const void *data,
                             size_t size) {
  reader->device = (void *)data;
  reader->size = size;
  reader->read = &wasm_memory_reader_read;
}

_Bool wasm_read(wasm_reader *reader, void *buffer, size_t amount) {
  return reader->read(reader, buffer, amount);
}

_Bool wasm_seek(wasm_reader *reader, size_t amount) {
  return reader->read(reader, NULL, amount);
}

// Int are encoded as LEB128 https://en.wikipedia.org/wiki/LEB128
// XXX: handling reader errors?
// XXX: handle max
// XXX: check if unsigned char is required in other places
uint32_t wasm_read_leb_u32(wasm_reader *reader) {
  assert(reader);

  uint32_t result = 0;
  unsigned char c = 128;

  // Here we check if the last bit is 1 (& 128).
  // Per spec it has at most ceil(32 / 7) = 5 bytes.
  for (int i = 0; c & 128 && i < 5; i++) {
    if (!wasm_read(reader, &c, 1)) {
      // XXX: Add error handling.
      abort();
    }
    result = result + ((uint32_t)(c & 127) << (i * 7));
  }

  return result;
}