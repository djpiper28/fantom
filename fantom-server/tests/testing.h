#pragma once
#include <stdio.h>
#include <stdlib.h>
#include "../src/logger.h"
#include "../src/ansi_colour.h"

#define TEST_FAIL ANSI_RED    "test-fail" ANSI_RESET
#define TEST_PASS ANSI_GREEN  "test-pass" ANSI_RESET
#define TEST_INFO ANSI_YELLOW "test-info" ANSI_RESET

#define ASSERT(x) if (!(x)) { lprintf(LOG_ERROR, "Test Error: Assertion failure, line %d in %s"\n", __LINE__, __FILENAME__); return 0; }

typedef struct unit_test {
    int (*test_method)();
    const char *test_name;
} unit_test;

// Standalone test for a function
int test_func(int (*test_method)(), const char *test_name);
// Test a set if named functions
int run_tests(unit_test *tests, size_t number, const char *module_name);
