#include "wasm/wasm_reader.h"

#include "wasm/wasm_common.h"
#include <assert.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

void wasm_read(wasm_reader *reader, void *buffer, size_t amount) {
  if (!reader->read(reader, buffer, amount)) {
    wasm_reader_fail(reader);
  }
}

void wasm_seek(wasm_reader *reader, size_t amount) {
  if (!reader->read(reader, NULL, amount)) {
    wasm_reader_fail(reader);
  }
}

void wasm_reader_fail(struct wasm_reader *reader) { longjmp(reader->buf, 1); }

// Int are encoded as LEB128 https://en.wikipedia.org/wiki/LEB128
// XXX: handling reader errors?
// XXX: handle max
// XXX: check if unsigned char is required in other places
uint32_t wasm_read_leb_u32(wasm_reader *reader) {
  uint32_t result = 0;

  // We set it to 128 initially so the loop doesn't end immediately.
  unsigned char c = 128;

  // Here we check if the last bit is 1 (& 128).
  // Per spec it has at most ceil(32 / 7) = 5 bytes.
  for (int i = 0; c & 128 && i < 5; i++) {
    wasm_read_obj(reader, &c);

    // Input:
    // Byte 0: 1aaaaaaa
    // Byte 1: 1bbbbbbb
    // Byte 2: 1ccccccc
    // Byte 3: 1ddddddd
    // Byte 4: xxxxeeee
    //
    // Output:
    // <            32 bits           >
    // eeeedddddddcccccccbbbbbbbaaaaaaa
    //
    // => xxxx needs to be 0
    if (i == 4 && c & (1 << 7 | 1 << 6 | 1 << 5 | 1 << 4)) {
      wasm_reader_fail(reader);
    }

    result = result + ((uint32_t)(c & 127) << (i * 7));
  }

  return result;
}

float wasm_read_f32(wasm_reader *reader) {
  char data[4];
  wasm_read(reader, data, 4);

  if (!wasm_is_little_endian()) {
    char swapped[4];
    swapped[0] = data[3];
    swapped[1] = data[2];
    swapped[2] = data[1];
    swapped[3] = data[0];
    return *((float *)swapped);
  } else {
    return *((float *)data);
  }
}

double wasm_read_f64(wasm_reader *reader) {
  char data[8];
  wasm_read(reader, data, 4);

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
    return *((double *)swapped);
  } else {
    return *((double *)data);
  }
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

char *wasm_read_string(wasm_reader *reader, char *out) {
  uint32_t length = wasm_read_leb_u32(reader);
  char *str = wasm_alloc_array(char, length + 1);

  if (!wasm_read(reader, str, length)) {
    wasm_free(str);
    wasm_reader_fail(reader);
  }

  str[length] = '\0';

  if (!wasm_validate_utf8(str)) {
    fprintf(stderr,
            "Looks like invalid utf8 but maybe the function is wrong.\n");
    wasm_free(str);
    wasm_reader_fail(reader);
  }
  return str;
}
