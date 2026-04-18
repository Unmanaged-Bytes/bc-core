// SPDX-License-Identifier: MIT

#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <cmocka.h>

#include "bc_core.h"

#include <stdalign.h>
#include <stdlib.h>

#ifdef __has_include
#if __has_include(<valgrind/valgrind.h>)
#include <valgrind/valgrind.h>
#else
#define RUNNING_ON_VALGRIND 0
#endif
#else
#define RUNNING_ON_VALGRIND 0
#endif

static void test_prefetch_read_range_empty_noop(void** state)
{
    BC_UNUSED(state);
    alignas(64) const unsigned char buffer[128] = {0};
    bc_core_prefetch_read_range(buffer, 0);
}

static void test_prefetch_read_range_small_buffer(void** state)
{
    BC_UNUSED(state);
    alignas(64) unsigned char buffer[128];
    bc_core_fill(buffer, sizeof(buffer), (unsigned char)0x5A);
    bc_core_prefetch_read_range(buffer, sizeof(buffer));
}

static void test_prefetch_read_range_large_buffer(void** state)
{
    BC_UNUSED(state);
    size_t len = 4096;
    unsigned char* buffer = aligned_alloc(64, len);
    assert_non_null(buffer);
    bc_core_fill(buffer, len, (unsigned char)0xA5);
    bc_core_prefetch_read_range(buffer, len);
    free(buffer);
}

static void test_evict_range_empty_noop(void** state)
{
    BC_UNUSED(state);
    if (RUNNING_ON_VALGRIND) {
        skip();
    }
    alignas(64) const unsigned char buffer[128] = {0};
    bc_core_evict_range(buffer, 0);
}

static void test_evict_range_small_buffer(void** state)
{
    BC_UNUSED(state);
    if (RUNNING_ON_VALGRIND) {
        skip();
    }
    alignas(64) unsigned char buffer[128];
    bc_core_fill(buffer, sizeof(buffer), (unsigned char)0xCC);
    bc_core_evict_range(buffer, sizeof(buffer));
}

static void test_evict_range_large_buffer(void** state)
{
    BC_UNUSED(state);
    if (RUNNING_ON_VALGRIND) {
        skip();
    }
    size_t len = 4096;
    unsigned char* buffer = aligned_alloc(64, len);
    assert_non_null(buffer);
    bc_core_fill(buffer, len, (unsigned char)0xAB);
    bc_core_evict_range(buffer, len);
    free(buffer);
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_prefetch_read_range_empty_noop),   cmocka_unit_test(test_prefetch_read_range_small_buffer),
        cmocka_unit_test(test_prefetch_read_range_large_buffer), cmocka_unit_test(test_evict_range_empty_noop),
        cmocka_unit_test(test_evict_range_small_buffer),         cmocka_unit_test(test_evict_range_large_buffer),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
