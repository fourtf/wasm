#include "test.h"
#include "wasm/wasm.h"
#include "wasm/wasm_common.h"
#include "wasm/wasm_helper.h"
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
  puts("This test expects you to be in `repo root/subdir`");

  FILE *file = fopen("../tests/reader_test.txt", "rb");
  wasm_reader reader;
  wasm_init_file_reader(&reader, file);

  char buff[4];

  // Reading data.
  MUST_EQUAL(wasm_read(&reader, buff, 4), true);
  MUST_EQUAL_MEM(abcd, buff, 4);

  // Reading over limit. Read 3 to account for a possible CR LF.
  MUST_EQUAL(wasm_read(&reader, &buff, 3), false);
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

// Ad hoc main for tests.
int main(void) {
  puts("Tests started\n");

  setup_tests();

  TEST(test_memory_reader);
  TEST(test_file_reader);
  TEST(test_leb_u32);

  if (all_success) {
    puts("\nAll tests passed PogChamp");
  } else {
    puts("\nSome tests didn't pass :(");
  }

  return all_success ? 0 : 1;
}