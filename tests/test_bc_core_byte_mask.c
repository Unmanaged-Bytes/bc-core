// SPDX-License-Identifier: MIT

#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <cmocka.h>

#include "bc_core.h"

#include <string.h>

static void test_byte_mask_prepare_empty_target(void** state)
{
    BC_UNUSED(state);
    bc_core_byte_mask_t mask;
    bool ok = bc_core_byte_mask_prepare(NULL, 0, &mask);
    assert_true(ok);
    size_t offset = 99;
    assert_false(bc_core_find_byte_in_mask("abcdef", 6, &mask, &offset));
}

static void test_byte_mask_prepare_single_target(void** state)
{
    BC_UNUSED(state);
    const unsigned char targets[] = {(unsigned char)'c'};
    bc_core_byte_mask_t mask;
    assert_true(bc_core_byte_mask_prepare(targets, 1, &mask));
    size_t offset = 0;
    assert_true(bc_core_find_byte_in_mask("abcdef", 6, &mask, &offset));
    assert_int_equal(offset, 2);
}

static void test_byte_mask_prepare_multiple_targets(void** state)
{
    BC_UNUSED(state);
    const unsigned char targets[] = {(unsigned char)'x', (unsigned char)'d', (unsigned char)'q'};
    bc_core_byte_mask_t mask;
    assert_true(bc_core_byte_mask_prepare(targets, 3, &mask));
    size_t offset = 0;
    assert_true(bc_core_find_byte_in_mask("abcdef", 6, &mask, &offset));
    assert_int_equal(offset, 3);
}

static void test_byte_mask_high_nibble_above_seven(void** state)
{
    BC_UNUSED(state);
    const unsigned char targets[] = {0xA5U, 0xFFU, 0x80U};
    bc_core_byte_mask_t mask;
    assert_true(bc_core_byte_mask_prepare(targets, 3, &mask));
    unsigned char data[] = {0x10, 0x20, 0xA5, 0x30};
    size_t offset = 0;
    assert_true(bc_core_find_byte_in_mask(data, sizeof(data), &mask, &offset));
    assert_int_equal(offset, 2);
}

static void test_byte_mask_high_nibble_mixed(void** state)
{
    BC_UNUSED(state);
    const unsigned char targets[] = {(unsigned char)'A', 0xFFU};
    bc_core_byte_mask_t mask;
    assert_true(bc_core_byte_mask_prepare(targets, 2, &mask));
    size_t offset = 0;
    assert_true(bc_core_find_byte_in_mask("abcAxxx", 7, &mask, &offset));
    assert_int_equal(offset, 3);
    unsigned char data2[] = {0x00, 0xFF, 0x10};
    offset = 0;
    assert_true(bc_core_find_byte_in_mask(data2, sizeof(data2), &mask, &offset));
    assert_int_equal(offset, 1);
}

static void test_byte_mask_no_match(void** state)
{
    BC_UNUSED(state);
    const unsigned char targets[] = {(unsigned char)'z'};
    bc_core_byte_mask_t mask;
    assert_true(bc_core_byte_mask_prepare(targets, 1, &mask));
    size_t offset = 99;
    assert_false(bc_core_find_byte_in_mask("abcdef", 6, &mask, &offset));
}

static void test_byte_mask_empty_input(void** state)
{
    BC_UNUSED(state);
    const unsigned char targets[] = {(unsigned char)'a'};
    bc_core_byte_mask_t mask;
    assert_true(bc_core_byte_mask_prepare(targets, 1, &mask));
    size_t offset = 0;
    assert_false(bc_core_find_byte_in_mask("", 0, &mask, &offset));
}

static void test_byte_mask_long_input_match_in_avx(void** state)
{
    BC_UNUSED(state);
    unsigned char buffer[256];
    memset(buffer, (unsigned char)'a', sizeof(buffer));
    buffer[100] = (unsigned char)'X';
    const unsigned char targets[] = {(unsigned char)'X'};
    bc_core_byte_mask_t mask;
    assert_true(bc_core_byte_mask_prepare(targets, 1, &mask));
    size_t offset = 0;
    assert_true(bc_core_find_byte_in_mask(buffer, sizeof(buffer), &mask, &offset));
    assert_int_equal(offset, 100);
}

static void test_byte_mask_long_input_no_match(void** state)
{
    BC_UNUSED(state);
    unsigned char buffer[256];
    memset(buffer, (unsigned char)'a', sizeof(buffer));
    const unsigned char targets[] = {(unsigned char)'Z'};
    bc_core_byte_mask_t mask;
    assert_true(bc_core_byte_mask_prepare(targets, 1, &mask));
    size_t offset = 99;
    assert_false(bc_core_find_byte_in_mask(buffer, sizeof(buffer), &mask, &offset));
}

static void test_byte_mask_long_input_match_in_tail(void** state)
{
    BC_UNUSED(state);
    unsigned char buffer[40];
    memset(buffer, (unsigned char)'a', sizeof(buffer));
    buffer[39] = (unsigned char)'Z';
    const unsigned char targets[] = {(unsigned char)'Z'};
    bc_core_byte_mask_t mask;
    assert_true(bc_core_byte_mask_prepare(targets, 1, &mask));
    size_t offset = 0;
    assert_true(bc_core_find_byte_in_mask(buffer, sizeof(buffer), &mask, &offset));
    assert_int_equal(offset, 39);
}

static void test_byte_mask_find_not_in_mask_first_outside(void** state)
{
    BC_UNUSED(state);
    const unsigned char targets[] = {(unsigned char)'a'};
    bc_core_byte_mask_t mask;
    assert_true(bc_core_byte_mask_prepare(targets, 1, &mask));
    size_t offset = 0;
    assert_true(bc_core_find_byte_not_in_mask("aaab", 4, &mask, &offset));
    assert_int_equal(offset, 3);
}

static void test_byte_mask_find_not_in_mask_all_in(void** state)
{
    BC_UNUSED(state);
    const unsigned char targets[] = {(unsigned char)'a', (unsigned char)'b', (unsigned char)'c'};
    bc_core_byte_mask_t mask;
    assert_true(bc_core_byte_mask_prepare(targets, 3, &mask));
    size_t offset = 99;
    assert_false(bc_core_find_byte_not_in_mask("aabbccab", 8, &mask, &offset));
}

static void test_byte_mask_find_not_in_mask_long_input(void** state)
{
    BC_UNUSED(state);
    unsigned char buffer[256];
    memset(buffer, (unsigned char)'a', sizeof(buffer));
    buffer[150] = (unsigned char)'Z';
    const unsigned char targets[] = {(unsigned char)'a'};
    bc_core_byte_mask_t mask;
    assert_true(bc_core_byte_mask_prepare(targets, 1, &mask));
    size_t offset = 0;
    assert_true(bc_core_find_byte_not_in_mask(buffer, sizeof(buffer), &mask, &offset));
    assert_int_equal(offset, 150);
}

static void test_byte_mask_find_not_in_mask_empty_input(void** state)
{
    BC_UNUSED(state);
    const unsigned char targets[] = {(unsigned char)'a'};
    bc_core_byte_mask_t mask;
    assert_true(bc_core_byte_mask_prepare(targets, 1, &mask));
    size_t offset = 99;
    assert_false(bc_core_find_byte_not_in_mask("", 0, &mask, &offset));
}

static bool predicate_uppercase(unsigned char byte_value, void* user_data)
{
    BC_UNUSED(user_data);
    return byte_value >= (unsigned char)'A' && byte_value <= (unsigned char)'Z';
}

static bool predicate_none(unsigned char byte_value, void* user_data)
{
    BC_UNUSED(byte_value);
    BC_UNUSED(user_data);
    return false;
}

static bool predicate_high_bit(unsigned char byte_value, void* user_data)
{
    BC_UNUSED(user_data);
    return (byte_value & 0x80U) != 0U;
}

static void test_byte_mask_prepare_predicate_uppercase(void** state)
{
    BC_UNUSED(state);
    bc_core_byte_mask_t mask;
    assert_true(bc_core_byte_mask_prepare_predicate(predicate_uppercase, NULL, &mask));
    size_t offset = 0;
    assert_true(bc_core_find_byte_in_mask("hello World", 11, &mask, &offset));
    assert_int_equal(offset, 6);
}

static void test_byte_mask_prepare_predicate_no_match(void** state)
{
    BC_UNUSED(state);
    bc_core_byte_mask_t mask;
    assert_true(bc_core_byte_mask_prepare_predicate(predicate_none, NULL, &mask));
    size_t offset = 99;
    assert_false(bc_core_find_byte_in_mask("abcde", 5, &mask, &offset));
}

static void test_byte_mask_prepare_predicate_high_bit(void** state)
{
    BC_UNUSED(state);
    bc_core_byte_mask_t mask;
    assert_true(bc_core_byte_mask_prepare_predicate(predicate_high_bit, NULL, &mask));
    unsigned char data[] = {0x10, 0x20, 0xA5, 0x30};
    size_t offset = 0;
    assert_true(bc_core_find_byte_in_mask(data, sizeof(data), &mask, &offset));
    assert_int_equal(offset, 2);
}

static void test_byte_mask_find_not_in_mask_high_nibble_above_seven(void** state)
{
    BC_UNUSED(state);
    const unsigned char targets[] = {(unsigned char)'a'};
    bc_core_byte_mask_t mask;
    assert_true(bc_core_byte_mask_prepare(targets, 1, &mask));
    unsigned char data[] = {(unsigned char)'a', 0xFFU};
    size_t offset = 0;
    assert_true(bc_core_find_byte_not_in_mask(data, sizeof(data), &mask, &offset));
    assert_int_equal(offset, 1);
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_byte_mask_find_not_in_mask_high_nibble_above_seven),
        cmocka_unit_test(test_byte_mask_prepare_empty_target),
        cmocka_unit_test(test_byte_mask_prepare_single_target),
        cmocka_unit_test(test_byte_mask_prepare_multiple_targets),
        cmocka_unit_test(test_byte_mask_high_nibble_above_seven),
        cmocka_unit_test(test_byte_mask_high_nibble_mixed),
        cmocka_unit_test(test_byte_mask_no_match),
        cmocka_unit_test(test_byte_mask_empty_input),
        cmocka_unit_test(test_byte_mask_long_input_match_in_avx),
        cmocka_unit_test(test_byte_mask_long_input_no_match),
        cmocka_unit_test(test_byte_mask_long_input_match_in_tail),
        cmocka_unit_test(test_byte_mask_find_not_in_mask_first_outside),
        cmocka_unit_test(test_byte_mask_find_not_in_mask_all_in),
        cmocka_unit_test(test_byte_mask_find_not_in_mask_long_input),
        cmocka_unit_test(test_byte_mask_find_not_in_mask_empty_input),
        cmocka_unit_test(test_byte_mask_prepare_predicate_uppercase),
        cmocka_unit_test(test_byte_mask_prepare_predicate_no_match),
        cmocka_unit_test(test_byte_mask_prepare_predicate_high_bit),
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}
