// SPDX-License-Identifier: MIT

#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <cmocka.h>

#include "bc_core.h"

#include <stdlib.h>

bool __real_bc_core_l2_cache_size(size_t* out_size);

static bool g_l2_fail = true;

bool __wrap_bc_core_l2_cache_size(size_t* out_size)
{
    if (g_l2_fail) {
        BC_UNUSED(out_size);
        return false;
    }
    return __real_bc_core_l2_cache_size(out_size);
}

static void test_copy_with_zero_l2_uses_aligned_store_path(void** state)
{
    BC_UNUSED(state);

    size_t buffer_size = 1024 * 1024;
    unsigned char* source = malloc(buffer_size);
    unsigned char* destination = malloc(buffer_size);
    assert_non_null(source);
    assert_non_null(destination);

    for (size_t i = 0; i < buffer_size; i++) {
        source[i] = (unsigned char)(i & 0xFF);
    }
    bc_core_zero(destination, buffer_size);

    bool result = bc_core_copy(destination, source, buffer_size);

    assert_true(result);
    assert_memory_equal(destination, source, buffer_size);

    free(source);
    free(destination);
}

static void test_copy_small_buffer_works_with_zero_l2(void** state)
{
    BC_UNUSED(state);

    unsigned char source[64];
    unsigned char destination[64];
    bc_core_fill(source, 64, (unsigned char)0x5A);
    bc_core_zero(destination, 64);

    bool result = bc_core_copy(destination, source, 64);

    assert_true(result);
    assert_memory_equal(destination, source, 64);
}

static void test_copy_real_l2_cache_size_used_when_not_failing(void** state)
{
    BC_UNUSED(state);

    g_l2_fail = false;

    size_t size = 0;
    bool success = bc_core_l2_cache_size(&size);

    assert_true(success);
    assert_true(size > 0);

    g_l2_fail = true;
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_copy_with_zero_l2_uses_aligned_store_path),
        cmocka_unit_test(test_copy_small_buffer_works_with_zero_l2),
        cmocka_unit_test(test_copy_real_l2_cache_size_used_when_not_failing),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
