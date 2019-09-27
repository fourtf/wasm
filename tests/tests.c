#include "test.h"

// Ad hoc main for tests.
int main(void) {
  puts("Tests started\n");

  setup_tests();

  if (all_success) {
    puts("All tests passed PogChamp");
  } else {
    puts("Some tests didn't pass :(");
  }

  return all_success ? 0 : 1;
}