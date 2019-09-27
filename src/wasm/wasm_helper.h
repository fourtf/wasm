#pragma once

#include "wasm/wasm_reader.h"
#include <stdint.h>

// leb128 encoded number.
uint32_t wasm_read_leb_u32(wasm_reader *reader);