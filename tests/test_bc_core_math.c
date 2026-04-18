// SPDX-License-Identifier: MIT

#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <cmocka.h>

#include "bc_core.h"

#include <stdint.h>

static void test_safe_multiply_zero_times_zero_returns_zero(void** state)
{
    BC_UNUSED(state);

    size_t result = 1;
    bool success = bc_core_safe_multiply(0, 0, &result);

    assert_true(success);
    assert_int_equal(result, 0);
}

static void test_safe_multiply_one_times_one_returns_one(void** state)
{
    BC_UNUSED(state);

    size_t result = 0;
    bool success = bc_core_safe_multiply(1, 1, &result);

    assert_true(success);
    assert_int_equal(result, 1);
}

static void test_safe_multiply_normal_values_returns_product(void** state)
{
    BC_UNUSED(state);

    size_t result = 0;
    bool success = bc_core_safe_multiply(42, 100, &result);

    assert_true(success);
    assert_int_equal(result, 4200);
}

static void test_safe_multiply_size_max_overflow_returns_false(void** state)
{
    BC_UNUSED(state);

    size_t result = 0;
    bool success = bc_core_safe_multiply(SIZE_MAX, 2, &result);

    assert_false(success);
}

static void test_safe_add_zero_plus_zero_returns_zero(void** state)
{
    BC_UNUSED(state);

    size_t result = 1;
    bool success = bc_core_safe_add(0, 0, &result);

    assert_true(success);
    assert_int_equal(result, 0);
}

static void test_safe_add_normal_values_returns_sum(void** state)
{
    BC_UNUSED(state);

    size_t result = 0;
    bool success = bc_core_safe_add(100, 200, &result);

    assert_true(success);
    assert_int_equal(result, 300);
}

static void test_safe_add_size_max_overflow_returns_false(void** state)
{
    BC_UNUSED(state);

    size_t result = 0;
    bool success = bc_core_safe_add(SIZE_MAX, 1, &result);

    assert_false(success);
}

static void test_align_up_already_aligned_value_returns_same(void** state)
{
    BC_UNUSED(state);

    size_t result = 0;
    bool success = bc_core_align_up(64, 32, &result);

    assert_true(success);
    assert_int_equal(result, 64);
}

static void test_align_up_unaligned_value_rounds_up(void** state)
{
    BC_UNUSED(state);

    size_t result = 0;
    bool success = bc_core_align_up(33, 32, &result);

    assert_true(success);
    assert_int_equal(result, 64);
}

static void test_align_up_zero_value_returns_zero(void** state)
{
    BC_UNUSED(state);

    size_t result = 1;
    bool success = bc_core_align_up(0, 32, &result);

    assert_true(success);
    assert_int_equal(result, 0);
}

static void test_align_up_alignment_one_returns_value_unchanged(void** state)
{
    BC_UNUSED(state);

    size_t result = 0;
    bool success = bc_core_align_up(17, 1, &result);

    assert_true(success);
    assert_int_equal(result, 17);
}

static void test_align_up_size_max_overflow_returns_false(void** state)
{
    BC_UNUSED(state);

    size_t result = 0;
    bool success = bc_core_align_up(SIZE_MAX, 32, &result);

    assert_false(success);
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_safe_multiply_zero_times_zero_returns_zero),
        cmocka_unit_test(test_safe_multiply_one_times_one_returns_one),
        cmocka_unit_test(test_safe_multiply_normal_values_returns_product),
        cmocka_unit_test(test_safe_multiply_size_max_overflow_returns_false),
        cmocka_unit_test(test_safe_add_zero_plus_zero_returns_zero),
        cmocka_unit_test(test_safe_add_normal_values_returns_sum),
        cmocka_unit_test(test_safe_add_size_max_overflow_returns_false),
        cmocka_unit_test(test_align_up_already_aligned_value_returns_same),
        cmocka_unit_test(test_align_up_unaligned_value_rounds_up),
        cmocka_unit_test(test_align_up_zero_value_returns_zero),
        cmocka_unit_test(test_align_up_alignment_one_returns_value_unchanged),
        cmocka_unit_test(test_align_up_size_max_overflow_returns_false),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
