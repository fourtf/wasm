#include "test.h"
#include "wasm/wasm_reader.h"

char abcd[] = {'a', 'b', 'c', 'd'};

void test_memory_reader() {
  wasm_reader reader;
  wasm_init_memory_reader(&reader, abcd, sizeof(abcd));

  char buff[sizeof(abcd)];

  // Reading data.
  MUST_EQUAL(wasm_read(&reader, &buff, sizeof(abcd)), true);
  MUST_EQUAL_MEM(abcd, buff, 4);

  // Reading over limit.
  MUST_EQUAL(wasm_read(&reader, &buff, 1), false);
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

void failed_test() { *((char *)0xdead) = 'a'; }

// Ad hoc main for tests.
int main(void) {
  puts("Tests started\n");

  setup_tests();

  TEST(test_memory_reader);
  TEST(test_file_reader);

  if (all_success) {
    puts("\nAll tests passed PogChamp");
  } else {
    puts("\nSome tests didn't pass :(");
  }

  return all_success ? 0 : 1;
}