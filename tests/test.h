#pragma once

#include <setjmp.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "wasm/wasm_common.h"

// Copied from here: https://stackoverflow.com/a/3219471
#define ANSI_COLOR_RED "\x1b[31m"
#define ANSI_COLOR_GREEN "\x1b[32m"
#define ANSI_COLOR_YELLOW "\x1b[33m"
#define ANSI_COLOR_BLUE "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN "\x1b[36m"
#define ANSI_COLOR_RESET "\x1b[0m"

// Recovery from segfault and other errors.
jmp_buf failure_buffer;
bool in_test = false;
const char *signal_name = "";

// Check if all tests passed.
bool all_success = true;

// Custom handler for segfault and other errors.
void handle_signal(int signal) {
  if (in_test) {
    switch (signal) {
    case SIGSEGV:
      signal_name = "SIGSEGV";
      break;
    case SIGABRT:
      signal_name = "SIGABRT";
      break;
    case SIGFPE:
      signal_name = "SIGFPE";
      break;
    default:
      signal_name = "UNKNOWN";
      break;
    }

    longjmp(failure_buffer, 0);
  }
}

// Runs a test and recovers from segfaults.
void test(void (*test)(), const char *name) {
  size_t alloc_count = wasm_alloc_count();
  setjmp(failure_buffer);

  if (in_test) {
    printf(ANSI_COLOR_BLUE "%s signal in '%s'.\n" ANSI_COLOR_RESET, signal_name,
           name);
    all_success = false;
  } else {
    in_test = true;
    test();
  }

  in_test = false;

  // Check if there is any objects that didn't get destroyed.
  size_t new_alloc_count = wasm_alloc_count();
  if (alloc_count != new_alloc_count) {
    printf(ANSI_COLOR_BLUE
           "%lld element(s) not deallocated in %s" ANSI_COLOR_RESET,
           (signed long long)new_alloc_count - (signed long long)alloc_count,
           name);
  }
}

void setup_tests() {
  signal(SIGSEGV, handle_signal);
  signal(SIGABRT, handle_signal);
  signal(SIGFPE, handle_signal);
}

#define TEST_IMPL(a, b) test(&a, b)
#define TEST(func) TEST_IMPL(func, #func)

#define SUCCESS ANSI_COLOR_GREEN "pass" ANSI_COLOR_RESET
#define FAILURE(x) ANSI_COLOR_GREEN x ANSI_COLOR_RESET

#define MUST(cond, fail)                                                       \
  printf("%s: %s\n", __func__,                                                 \
         (cond ? SUCCESS : (all_success = false, FAILURE(fail))))

// Checks if two values are equal by testing with `==`.
#define MUST_EQUAL(a, b) MUST(a == b, "must equal")

// Checks if two values are equal by testing with `!=`.
#define MUST_NOT_EQUAL(a, b) MUST(a != b, "must not equal")

// Takes two pointers and compares `length` bytes.
#define MUST_EQUAL_MEM(a, b, length)                                           \
  MUST(memcmp(a, b, length) == 0, "memory not equal")
