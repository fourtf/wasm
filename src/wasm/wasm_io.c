#include "wasm_io.h"

void wasm_file_reader_read(wasm_reader *reader, void *buffer, size_t amount) {
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

void wasm_memory_reader_read(wasm_reader *reader, void *buffer, size_t amount) {
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

void wasm_init_file_device(wasm_reader *reader, FILE *file) {
  reader->device = file;
  reader->size = 0; // Size is ignored in file reader.
  reader->read = &wasm_file_reader_read;
}

void wasm_init_memory_device(wasm_reader *reader, const void *data,
                             size_t size) {
  reader->device = (void *)data;
  reader->size = size;
  reader->read = &wasm_memory_reader_read;
}