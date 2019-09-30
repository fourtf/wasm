#include "test.h"
#include "wasm/wasm.h"
#include "wasm/wasm_common.h"
#include "wasm/wasm_reader.h"

char abcd[] = {'a', 'b', 'c', 'd'};

void test_memory_reader() {
  wasm_reader reader;
  wasm_init_memory_reader(&reader, abcd, sizeof(abcd));

  {
    char buff[sizeof(abcd)];
    // Reading data.
    MUST_EQUAL(wasm_read(&reader, &buff, sizeof(abcd)), true);
    MUST_EQUAL_MEM(abcd, buff, 4);

    // Reading over limit.
    MUST_EQUAL(wasm_read(&reader, &buff, 1), false);
  }
  {
    char c;
    // Reinit reader.
    wasm_init_memory_reader(&reader, abcd, sizeof(abcd));
    MUST_EQUAL(wasm_read(&reader, &c, 1), true);
    MUST_EQUAL(c, abcd[0]);
    MUST_EQUAL(wasm_read(&reader, &c, 1), true);
    MUST_EQUAL(c, abcd[1]);
    MUST_EQUAL(wasm_read(&reader, &c, 1), true);
    MUST_EQUAL(c, abcd[2]);
    MUST_EQUAL(wasm_read(&reader, &c, 1), true);
    MUST_EQUAL(c, abcd[3]);
  }
}

void test_file_reader() {
  FILE *file = fopen("../tests/reader_test.txt", "rb");
  wasm_reader reader;
  wasm_init_file_reader(&reader, file);

  char buff[4];

  // Reading data.
  MUST_EQUAL(wasm_read(&reader, buff, 4), true);
  MUST_EQUAL_MEM(abcd, buff, 4);

  // Reading over limit. Read 3 to account for a possible CR LF.
  MUST_EQUAL(wasm_read(&reader, &buff, 3), false);

  fclose(file);
}

void test_leb_u32() {
  wasm_reader reader;

  {
    const char data = {12};
    wasm_init_memory_reader(&reader, &data, sizeof(data));
    MUST_EQUAL(wasm_read_leb_u32(&reader), 12);
  }
  {
    const char data[] = {255, 1};
    wasm_init_memory_reader(&reader, data, sizeof(data));
    MUST_EQUAL(wasm_read_leb_u32(&reader), 255);
  }
  {
    const char data[] = {0xE5, 0x8E, 0x26};
    wasm_init_memory_reader(&reader, data, sizeof(data));
    MUST_EQUAL(wasm_read_leb_u32(&reader), 624485);
  }
}

void test_skip_custom_section() {
  wasm_module *module =
      wasm_load_module_from_file("../tests/files/custom_section.wasm");

  MUST_NOT_EQUAL(module, NULL);
  wasm_free_module(module);
}

void test_vec() {
  // Add tests for wasm_vec here :(
}

void test_emscripten_file_1() {
  wasm_module *module =
      wasm_load_module_from_file("../tests/files/emscripten_1/a.out.wasm");

  MUST_NOT_EQUAL(module, NULL);
  if (module) {
    MUST_EQUAL(wasm_vec_size(&module->function_types), 2);
    MUST_EQUAL(wasm_vec_size(&module->funcs), 2);
    MUST_EQUAL(wasm_vec_size(&module->globals), 1);

    // Already put those here for the future:
    // MUST_EQUAL(wasm_vec_size(&module->exports), 2);
  }

  wasm_free_module(module);
}

// Ad hoc main for tests.
int main(void) {
  puts("Tests started");
  puts("It is expected that you are in a subdirectory of the repos root (e.g. "
       "the build directory). Tests may fail if that is not the case.\n");

  setup_tests();

  TEST(test_memory_reader);
  TEST(test_file_reader);
  TEST(test_leb_u32);
  TEST(test_skip_custom_section);
  TEST(test_vec);
  TEST(test_emscripten_file_1);

  if (all_success) {
    puts("\nAll tests passed PogChamp");
  } else {
    puts("\nSome tests didn't pass :(");
  }

  return all_success ? 0 : 1;
}