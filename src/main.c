#include <stdio.h>

#include "wasm/wasm.h"

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr,
            "Program requires one argument which is the wasm file to exec.");

    return 1;
  }

  const char *file_name = argv[1];

  wasm_module *module = wasm_load_module_from_file(file_name);

  if (module == NULL) {
    fprintf(stderr, "Failed to load wasm module.");
  }
}
