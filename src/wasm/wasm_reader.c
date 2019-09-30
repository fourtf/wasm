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

bool wasm_read_f32(wasm_reader *reader, float *out) {
  char data[4];

  if (!wasm_read(reader, data, 4)) {
    return false;
  }

  if (!wasm_is_little_endian()) {
    char swapped[4];
    swapped[0] = data[3];
    swapped[1] = data[2];
    swapped[2] = data[1];
    swapped[3] = data[0];
    *out = *((float *)swapped);
  } else {
    *out = *((float *)data);
  }

  return true;
}

bool wasm_read_f64(wasm_reader *reader, double *out) {
  char data[8];

  if (!wasm_read(reader, data, 4)) {
    return false;
  }

  if (!wasm_is_little_endian()) {
    char swapped[8];
    swapped[0] = data[7];
    swapped[1] = data[6];
    swapped[2] = data[5];
    swapped[3] = data[4];
    swapped[4] = data[3];
    swapped[5] = data[2];
    swapped[6] = data[1];
    swapped[7] = data[0];
    *out = *((double *)swapped);
  } else {
    *out = *((double *)data);
  }

  return true;
}

// XXX: This code has never been tested and is just assumed to be working.
bool wasm_validate_utf8(char *str) {
  while (*str) {
    if ((*str & (1 << 7)) == 0) {
      // 1 byte
      str++;
    } else if ((*str & (1 << 7 | 1 << 6 | 1 << 5)) == (1 << 7 | 1 << 6)) {
      // 2 bytes
      if (str[1] == '\0') {
        return false;
      }

      if ((str[1] & (1 << 7 | 1 << 6)) != (1 << 7)) {
        return false;
      }
      str += 2;
    } else if ((*str & (1 << 7 | 1 << 6 | 1 << 5 | 1 << 4)) ==
               (1 << 7 | 1 << 6 | 1 << 5)) {
      // 3 bytes
      if (str[1] == '\0' || str[2] == '\0') {
        return false;
      }

      if ((str[1] & (1 << 7 | 1 << 6)) != (1 << 7)) {
        return false;
      }
      if ((str[2] & (1 << 7 | 1 << 6)) != (1 << 7)) {
        return false;
      }
      str += 3;
    } else if ((*str & (1 << 7 | 1 << 6 | 1 << 5 | 1 << 4 | 1 << 3)) ==
               (1 << 7 | 1 << 6 | 1 << 5 | 1 << 4)) {
      // 4 bytes
      if (str[1] == '\0' || str[2] == '\0' || str[3] == '\0') {
        return false;
      }
      if ((str[1] & (1 << 7 | 1 << 6)) != (1 << 7)) {
        return false;
      }
      if ((str[2] & (1 << 7 | 1 << 6)) != (1 << 7)) {
        return false;
      }
      if ((str[3] & (1 << 7 | 1 << 6)) != (1 << 7)) {
        return false;
      }
      str += 4;
    } else {
      puts("uffka");
      return false;
    }
  }

  return true;
}

bool wasm_read_string(wasm_reader *reader, char **out) {
  uint32_t length = wasm_read_leb_u32(reader);
  char *str = wasm_alloc_array(char, length + 1);

  if (!wasm_read(reader, str, length)) {
    wasm_free(str);
    return false;
  }

  str[length] = '\0';

  if (!wasm_validate_utf8(str)) {
    fprintf(stderr,
            "Looks like invalid utf8 but maybe the function is wrong.\n");
    wasm_free(str);
    return false;
  }

  *out = str;
  return true;
}
