#pragma once

#include <stdlib.h>

#define wasm_alloc(type) ((type*)malloc(sizeof(type))
#define wasm_free(obj) free(obj)