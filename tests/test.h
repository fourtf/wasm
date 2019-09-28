#pragma once

#include <setjmp.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "wasm/wasm_common.h"

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
    printf("%s signal in '%s'.\n", signal_name, name);
    all_success = false;
  } else {
    in_test = true;
    test();
  }

  in_test = false;

  // Check if there is any objects that didn't get destroyed.
  size_t new_alloc_count = wasm_alloc_count();
  if (alloc_count != new_alloc_count) {
    printf("%lld element(s) not deallocated in %s",
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

// Checks if two values are equal by testing with `==`.
#define MUST_EQUAL(a, b)                                                       \
  printf("%s: %s\n", __func__,                                                 \
         (a == b ? "pass" : (all_success = false, "must equal")))

// Checks if two values are equal by testing with `!=`.
#define MUST_NOT_EQUAL(a, b)                                                   \
  printf("%s: %s\n", __func__,                                                 \
         (a != b ? "pass" : (all_success = false, "must not equal")))

// Takes two pointers and compares `length` bytes.
#define MUST_EQUAL_MEM(a, b, length)                                           \
  printf("%s: %s\n", __func__,                                                 \
         (memcmp(a, b, length) == 0                                            \
              ? "pass"                                                         \
              : (all_success = false, "memory not equal")))
