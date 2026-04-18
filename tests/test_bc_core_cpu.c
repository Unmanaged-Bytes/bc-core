// SPDX-License-Identifier: MIT

#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <cmocka.h>

#include "bc_core.h"

static void test_chunk_size_returns_positive_power_of_two(void** state)
{
    BC_UNUSED(state);

    size_t result = 0;
    bool success = bc_core_chunk_size(&result);

    assert_true(success);
    assert_true(result == 32 || result == 64);
}

static void test_preferred_alignment_returns_64(void** state)
{
    BC_UNUSED(state);

    size_t result = 0;
    bool success = bc_core_preferred_alignment(&result);

    assert_true(success);
    assert_int_equal(result, 64);
}

static void test_cache_line_size_returns_positive_value(void** state)
{
    BC_UNUSED(state);

    size_t result = 0;
    bool success = bc_core_cache_line_size(&result);

    assert_true(success);
    assert_true(result > 0);
}

static void test_l1d_cache_size_returns_positive_value(void** state)
{
    BC_UNUSED(state);

    size_t result = 0;
    bool success = bc_core_l1d_cache_size(&result);

    assert_true(success);
    assert_true(result > 0);
}

static void test_l2_cache_size_returns_positive_value(void** state)
{
    BC_UNUSED(state);

    size_t result = 0;
    bool success = bc_core_l2_cache_size(&result);

    assert_true(success);
    assert_true(result > 0);
}

static void test_l3_cache_size_returns_positive_value(void** state)
{
    BC_UNUSED(state);

    size_t result = 0;
    bool success = bc_core_l3_cache_size(&result);

    assert_true(success);
    assert_true(result > 0);
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_chunk_size_returns_positive_power_of_two), cmocka_unit_test(test_preferred_alignment_returns_64),
        cmocka_unit_test(test_cache_line_size_returns_positive_value),   cmocka_unit_test(test_l1d_cache_size_returns_positive_value),
        cmocka_unit_test(test_l2_cache_size_returns_positive_value),     cmocka_unit_test(test_l3_cache_size_returns_positive_value),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
