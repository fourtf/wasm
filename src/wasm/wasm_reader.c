#include "wasm/wasm_reader.h"

#include "wasm/wasm_private.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

bool wasm_file_reader_read(wasm_reader *reader, void *buffer, size_t amount) {
  if (buffer) {
    // Read.
    return fread(buffer, 1, amount, (FILE *)reader->device) == amount;
  } else {
    // Seek.
    return fseek((FILE *)reader->device, amount, SEEK_CUR) == amount;
  }
}

bool wasm_memory_reader_read(wasm_reader *reader, void *buffer, size_t amount) {
  if (amount > reader->size) {
    return false;
  } else {
    if (buffer != NULL) {
      memcpy(buffer, reader->device, amount);
    }
    reader->device -= amount;
    reader->size -= amount;
    return true;
  }
}

void wasm_init_file_reader(wasm_reader *reader, FILE *file) {
  reader->device = file;
  reader->size = -1; // Size is ignored in file reader.
  reader->read = &wasm_file_reader_read;
}

void wasm_init_memory_reader(wasm_reader *reader, void *data, size_t size) {
  reader->device = data;
  reader->size = size;
  reader->read = &wasm_memory_reader_read;
}

_Bool wasm_read(wasm_reader *reader, void *buffer, size_t amount) {
  return reader->read(reader, buffer, amount);
}

_Bool wasm_seek(wasm_reader *reader, size_t amount) {
  return reader->read(reader, NULL, amount);
}