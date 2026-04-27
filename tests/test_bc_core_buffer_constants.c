// SPDX-License-Identifier: MIT

#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <cmocka.h>

#include "bc_core.h"

static void test_l1_hot_strictly_less_than_l1_full(void** state)
{
    BC_UNUSED(state);

    assert_true(BC_BUFFER_L1_HOT_BYTES < BC_BUFFER_L1_FULL_BYTES);
}

static void test_l1_full_strictly_less_than_l2_hot(void** state)
{
    BC_UNUSED(state);

    assert_true(BC_BUFFER_L1_FULL_BYTES < BC_BUFFER_L2_HOT_BYTES);
}

static void test_l2_hot_strictly_less_than_l2_full(void** state)
{
    BC_UNUSED(state);

    assert_true(BC_BUFFER_L2_HOT_BYTES < BC_BUFFER_L2_FULL_BYTES);
}

static void test_l2_full_strictly_less_than_l3_per_thread(void** state)
{
    BC_UNUSED(state);

    assert_true(BC_BUFFER_L2_FULL_BYTES < BC_BUFFER_L3_PER_THREAD_BYTES);
}

static void test_all_tiers_are_cache_line_aligned(void** state)
{
    BC_UNUSED(state);

    assert_int_equal(BC_BUFFER_L1_HOT_BYTES % BC_CACHE_LINE_SIZE, 0);
    assert_int_equal(BC_BUFFER_L1_FULL_BYTES % BC_CACHE_LINE_SIZE, 0);
    assert_int_equal(BC_BUFFER_L2_HOT_BYTES % BC_CACHE_LINE_SIZE, 0);
    assert_int_equal(BC_BUFFER_L2_FULL_BYTES % BC_CACHE_LINE_SIZE, 0);
    assert_int_equal(BC_BUFFER_L3_PER_THREAD_BYTES % BC_CACHE_LINE_SIZE, 0);
}

static void test_streaming_thresholds_ordered(void** state)
{
    BC_UNUSED(state);

    assert_true(BC_BUFFER_COPY_STREAMING_THRESHOLD <= BC_BUFFER_ZERO_STREAMING_THRESHOLD);
    assert_true(BC_BUFFER_COPY_STREAMING_THRESHOLD <= BC_BUFFER_FILL_STREAMING_THRESHOLD);
    assert_int_equal(BC_BUFFER_ZERO_STREAMING_THRESHOLD, BC_BUFFER_FILL_STREAMING_THRESHOLD);
}

static void test_streaming_thresholds_above_l2_full(void** state)
{
    BC_UNUSED(state);

    assert_true(BC_BUFFER_COPY_STREAMING_THRESHOLD > BC_BUFFER_L2_FULL_BYTES);
    assert_true(BC_BUFFER_ZERO_STREAMING_THRESHOLD > BC_BUFFER_L2_FULL_BYTES);
    assert_true(BC_BUFFER_FILL_STREAMING_THRESHOLD > BC_BUFFER_L2_FULL_BYTES);
}

static void test_huge_page_threshold_is_2_mib(void** state)
{
    BC_UNUSED(state);

    assert_int_equal(BC_BUFFER_HUGE_PAGE_THRESHOLD, (size_t)2 * 1024 * 1024);
}

static void test_constant_values_match_brief(void** state)
{
    BC_UNUSED(state);

    assert_int_equal(BC_BUFFER_L1_HOT_BYTES, (size_t)16 * 1024);
    assert_int_equal(BC_BUFFER_L1_FULL_BYTES, (size_t)24 * 1024);
    assert_int_equal(BC_BUFFER_L2_HOT_BYTES, (size_t)256 * 1024);
    assert_int_equal(BC_BUFFER_L2_FULL_BYTES, (size_t)384 * 1024);
    assert_int_equal(BC_BUFFER_L3_PER_THREAD_BYTES, (size_t)1024 * 1024);
    assert_int_equal(BC_BUFFER_COPY_STREAMING_THRESHOLD, (size_t)4 * 1024 * 1024);
    assert_int_equal(BC_BUFFER_ZERO_STREAMING_THRESHOLD, (size_t)4 * 1024 * 1024);
    assert_int_equal(BC_BUFFER_FILL_STREAMING_THRESHOLD, (size_t)4 * 1024 * 1024);
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_l1_hot_strictly_less_than_l1_full),  cmocka_unit_test(test_l1_full_strictly_less_than_l2_hot),
        cmocka_unit_test(test_l2_hot_strictly_less_than_l2_full),  cmocka_unit_test(test_l2_full_strictly_less_than_l3_per_thread),
        cmocka_unit_test(test_all_tiers_are_cache_line_aligned),   cmocka_unit_test(test_streaming_thresholds_ordered),
        cmocka_unit_test(test_streaming_thresholds_above_l2_full), cmocka_unit_test(test_huge_page_threshold_is_2_mib),
        cmocka_unit_test(test_constant_values_match_brief),
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}
