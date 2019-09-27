#include "wasm/wasm_helper.h"
#include "wasm/wasm_reader.h"
#include <assert.h>

// Int are encoded as LEB128 https://en.wikipedia.org/wiki/LEB128
// XXX: handling fread errors?
// XXX: write tests
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