// SPDX-License-Identifier: MIT

#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <cmocka.h>
#include "bc_core.h"

static void test_contains_byte_found(void** state)
{
    BC_UNUSED(state);

    const unsigned char data[16] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x42};
    bool out_found = false;

    bc_core_contains_byte(data, 16, 0x42, &out_found);

    assert_true(out_found);
}

static void test_contains_byte_not_found(void** state)
{
    BC_UNUSED(state);

    unsigned char data[16];
    bc_core_fill(data, 16, (unsigned char)0xAA);
    bool out_found = true;

    bc_core_contains_byte(data, 16, 0x42, &out_found);

    assert_false(out_found);
}

static void test_contains_byte_found_at_first_position(void** state)
{
    BC_UNUSED(state);

    const unsigned char data[8] = {0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    bool out_found = false;

    bc_core_contains_byte(data, 8, 0xFF, &out_found);

    assert_true(out_found);
}

static void test_contains_byte_len_zero_not_found(void** state)
{
    BC_UNUSED(state);

    const unsigned char data[4] = {0x42, 0x42, 0x42, 0x42};
    bool out_found = true;

    bc_core_contains_byte(data, 0, 0x42, &out_found);

    assert_false(out_found);
}

static void test_contains_pattern_found(void** state)
{
    BC_UNUSED(state);

    const unsigned char data[20] = {0x00, 0x01, 0x02, 0xDE, 0xAD, 0xBE, 0xEF, 0x07, 0x08, 0x09,
                                    0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x12, 0x13};
    const unsigned char pattern[4] = {0xDE, 0xAD, 0xBE, 0xEF};
    bool out_found = false;

    bc_core_contains_pattern(data, 20, pattern, 4, &out_found);

    assert_true(out_found);
}

static void test_contains_pattern_not_found(void** state)
{
    BC_UNUSED(state);

    unsigned char data[20];
    bc_core_fill(data, 20, (unsigned char)0x55);
    const unsigned char pattern[4] = {0xDE, 0xAD, 0xBE, 0xEF};
    bool out_found = true;

    bc_core_contains_pattern(data, 20, pattern, 4, &out_found);

    assert_false(out_found);
}

static void test_contains_pattern_empty_pattern_not_found(void** state)
{
    BC_UNUSED(state);

    const unsigned char data[8] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
    bool out_found = true;

    bc_core_contains_pattern(data, 8, "", 0, &out_found);

    assert_false(out_found);
}

static void test_contains_pattern_found_at_end(void** state)
{
    BC_UNUSED(state);

    const unsigned char data[10] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0xFE, 0xFF};
    const unsigned char pattern[2] = {0xFE, 0xFF};
    bool out_found = false;

    bc_core_contains_pattern(data, 10, pattern, 2, &out_found);

    assert_true(out_found);
}

static void test_starts_with_matches(void** state)
{
    BC_UNUSED(state);

    const unsigned char data[10] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22, 0x33, 0x44};
    const unsigned char prefix[3] = {0xAA, 0xBB, 0xCC};
    bool out_result = false;

    bc_core_starts_with(data, 10, prefix, 3, &out_result);

    assert_true(out_result);
}

static void test_starts_with_does_not_match(void** state)
{
    BC_UNUSED(state);

    const unsigned char data[10] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22, 0x33, 0x44};
    const unsigned char prefix[3] = {0xAA, 0xBB, 0xFF};
    bool out_result = true;

    bc_core_starts_with(data, 10, prefix, 3, &out_result);

    assert_false(out_result);
}

static void test_starts_with_prefix_longer_than_data(void** state)
{
    BC_UNUSED(state);

    const unsigned char data[3] = {0xAA, 0xBB, 0xCC};
    const unsigned char prefix[5] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE};
    bool out_result = true;

    bc_core_starts_with(data, 3, prefix, 5, &out_result);

    assert_false(out_result);
}

static void test_starts_with_full_match(void** state)
{
    BC_UNUSED(state);

    const unsigned char data[4] = {0x11, 0x22, 0x33, 0x44};
    const unsigned char prefix[4] = {0x11, 0x22, 0x33, 0x44};
    bool out_result = false;

    bc_core_starts_with(data, 4, prefix, 4, &out_result);

    assert_true(out_result);
}

static void test_ends_with_matches(void** state)
{
    BC_UNUSED(state);

    const unsigned char data[10] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22, 0x33, 0x44};
    const unsigned char suffix[3] = {0x22, 0x33, 0x44};
    bool out_result = false;

    bc_core_ends_with(data, 10, suffix, 3, &out_result);

    assert_true(out_result);
}

static void test_ends_with_does_not_match(void** state)
{
    BC_UNUSED(state);

    const unsigned char data[10] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22, 0x33, 0x44};
    const unsigned char suffix[3] = {0x22, 0x33, 0x55};
    bool out_result = true;

    bc_core_ends_with(data, 10, suffix, 3, &out_result);

    assert_false(out_result);
}

static void test_ends_with_suffix_longer_than_data(void** state)
{
    BC_UNUSED(state);

    const unsigned char data[3] = {0xAA, 0xBB, 0xCC};
    const unsigned char suffix[5] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE};
    bool out_result = true;

    bc_core_ends_with(data, 3, suffix, 5, &out_result);

    assert_false(out_result);
}

static void test_ends_with_full_match(void** state)
{
    BC_UNUSED(state);

    const unsigned char data[4] = {0x11, 0x22, 0x33, 0x44};
    const unsigned char suffix[4] = {0x11, 0x22, 0x33, 0x44};
    bool out_result = false;

    bc_core_ends_with(data, 4, suffix, 4, &out_result);

    assert_true(out_result);
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_contains_byte_found),
        cmocka_unit_test(test_contains_byte_not_found),
        cmocka_unit_test(test_contains_byte_found_at_first_position),
        cmocka_unit_test(test_contains_byte_len_zero_not_found),
        cmocka_unit_test(test_contains_pattern_found),
        cmocka_unit_test(test_contains_pattern_not_found),
        cmocka_unit_test(test_contains_pattern_empty_pattern_not_found),
        cmocka_unit_test(test_contains_pattern_found_at_end),
        cmocka_unit_test(test_starts_with_matches),
        cmocka_unit_test(test_starts_with_does_not_match),
        cmocka_unit_test(test_starts_with_prefix_longer_than_data),
        cmocka_unit_test(test_starts_with_full_match),
        cmocka_unit_test(test_ends_with_matches),
        cmocka_unit_test(test_ends_with_does_not_match),
        cmocka_unit_test(test_ends_with_suffix_longer_than_data),
        cmocka_unit_test(test_ends_with_full_match),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
