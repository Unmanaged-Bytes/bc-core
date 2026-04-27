// SPDX-License-Identifier: MIT

#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdlib.h>
#include <cmocka.h>
#include "bc_core.h"

static void test_equal_len_zero_sets_out_equal_true(void** state)
{
    BC_UNUSED(state);

    bool out_equal = false;

    bool result = bc_core_equal("x", "y", 0, &out_equal);

    assert_true(result);
    assert_true(out_equal);
}

static void test_equal_len_1_identical_bytes(void** state)
{
    BC_UNUSED(state);

    const unsigned char a[1] = {0x42};
    const unsigned char b[1] = {0x42};
    bool out_equal = false;

    bool result = bc_core_equal(a, b, 1, &out_equal);

    assert_true(result);
    assert_true(out_equal);
}

static void test_equal_len_1_different_bytes(void** state)
{
    BC_UNUSED(state);

    const unsigned char a[1] = {0x11};
    const unsigned char b[1] = {0x22};
    bool out_equal = true;

    bool result = bc_core_equal(a, b, 1, &out_equal);

    assert_true(result);
    assert_false(out_equal);
}

static void test_equal_len_8_identical_uint64_accumulation(void** state)
{
    BC_UNUSED(state);

    const unsigned char a[8] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
    const unsigned char b[8] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
    bool out_equal = false;

    bool result = bc_core_equal(a, b, 8, &out_equal);

    assert_true(result);
    assert_true(out_equal);
}

static void test_equal_len_8_different_last_byte_uint64_accumulation(void** state)
{
    BC_UNUSED(state);

    const unsigned char a[8] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
    const unsigned char b[8] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0xFF};
    bool out_equal = true;

    bool result = bc_core_equal(a, b, 8, &out_equal);

    assert_true(result);
    assert_false(out_equal);
}

static void test_equal_len_9_single_vector_path_identical(void** state)
{
    BC_UNUSED(state);

    unsigned char a[9];
    unsigned char b[9];
    bc_core_fill(a, 9, (unsigned char)0xAA);
    bc_core_fill(b, 9, (unsigned char)0xAA);
    bool out_equal = false;

    bool result = bc_core_equal(a, b, 9, &out_equal);

    assert_true(result);
    assert_true(out_equal);
}

static void test_equal_len_9_single_vector_path_different(void** state)
{
    BC_UNUSED(state);

    unsigned char a[9];
    unsigned char b[9];
    bc_core_fill(a, 9, (unsigned char)0xAA);
    bc_core_fill(b, 9, (unsigned char)0xAA);
    b[8] = 0xBB;
    bool out_equal = true;

    bool result = bc_core_equal(a, b, 9, &out_equal);

    assert_true(result);
    assert_false(out_equal);
}

static void test_equal_len_32_single_vector_full_mask_identical(void** state)
{
    BC_UNUSED(state);

    unsigned char a[32];
    unsigned char b[32];
    bc_core_fill(a, 32, (unsigned char)0x55);
    bc_core_fill(b, 32, (unsigned char)0x55);
    bool out_equal = false;

    bool result = bc_core_equal(a, b, 32, &out_equal);

    assert_true(result);
    assert_true(out_equal);
}

static void test_equal_len_32_single_vector_full_mask_different(void** state)
{
    BC_UNUSED(state);

    unsigned char a[32];
    unsigned char b[32];
    bc_core_fill(a, 32, (unsigned char)0x55);
    bc_core_fill(b, 32, (unsigned char)0x55);
    b[0] = 0x56;
    bool out_equal = true;

    bool result = bc_core_equal(a, b, 32, &out_equal);

    assert_true(result);
    assert_false(out_equal);
}

static void test_equal_len_33_multi_vector_identical(void** state)
{
    BC_UNUSED(state);

    unsigned char a[33];
    unsigned char b[33];
    bc_core_fill(a, 33, (unsigned char)0x77);
    bc_core_fill(b, 33, (unsigned char)0x77);
    bool out_equal = false;

    bool result = bc_core_equal(a, b, 33, &out_equal);

    assert_true(result);
    assert_true(out_equal);
}

static void test_equal_len_33_multi_vector_difference_in_vector_block(void** state)
{
    BC_UNUSED(state);

    unsigned char a[33];
    unsigned char b[33];
    bc_core_fill(a, 33, (unsigned char)0x77);
    bc_core_fill(b, 33, (unsigned char)0x77);
    b[15] = 0x00;
    bool out_equal = true;

    bool result = bc_core_equal(a, b, 33, &out_equal);

    assert_true(result);
    assert_false(out_equal);
}

static void test_equal_len_33_multi_vector_difference_in_tail(void** state)
{
    BC_UNUSED(state);

    unsigned char a[33];
    unsigned char b[33];
    bc_core_fill(a, 33, (unsigned char)0x77);
    bc_core_fill(b, 33, (unsigned char)0x77);
    b[32] = 0x00;
    bool out_equal = true;

    bool result = bc_core_equal(a, b, 33, &out_equal);

    assert_true(result);
    assert_false(out_equal);
}

static void test_equal_len_64_multi_vector_identical(void** state)
{
    BC_UNUSED(state);

    unsigned char a[64];
    unsigned char b[64];
    for (size_t i = 0; i < 64; i++) {
        a[i] = (unsigned char)i;
        b[i] = (unsigned char)i;
    }
    bool out_equal = false;

    bool result = bc_core_equal(a, b, 64, &out_equal);

    assert_true(result);
    assert_true(out_equal);
}

static void test_equal_len_1024_multi_vector_identical(void** state)
{
    BC_UNUSED(state);

    unsigned char a[1024];
    unsigned char b[1024];
    bc_core_fill(a, 1024, (unsigned char)0x3C);
    bc_core_fill(b, 1024, (unsigned char)0x3C);
    bool out_equal = false;

    bool result = bc_core_equal(a, b, 1024, &out_equal);

    assert_true(result);
    assert_true(out_equal);
}

static void test_equal_len_1024_multi_vector_difference_at_start(void** state)
{
    BC_UNUSED(state);

    unsigned char a[1024];
    unsigned char b[1024];
    bc_core_fill(a, 1024, (unsigned char)0x3C);
    bc_core_fill(b, 1024, (unsigned char)0x3C);
    b[0] = 0xFF;
    bool out_equal = true;

    bool result = bc_core_equal(a, b, 1024, &out_equal);

    assert_true(result);
    assert_false(out_equal);
}

static void test_compare_len_zero_sets_out_result_zero(void** state)
{
    BC_UNUSED(state);

    int out_result = 99;

    bool result = bc_core_compare("x", "y", 0, &out_result);

    assert_true(result);
    assert_int_equal(out_result, 0);
}

static void test_compare_identical_buffers_returns_zero(void** state)
{
    BC_UNUSED(state);

    unsigned char a[64];
    unsigned char b[64];
    bc_core_fill(a, 64, (unsigned char)0xAA);
    bc_core_fill(b, 64, (unsigned char)0xAA);
    int out_result = 99;

    bool result = bc_core_compare(a, b, 64, &out_result);

    assert_true(result);
    assert_int_equal(out_result, 0);
}

static void test_compare_a_less_than_b_returns_minus_one(void** state)
{
    BC_UNUSED(state);

    unsigned char a[32];
    unsigned char b[32];
    bc_core_fill(a, 32, (unsigned char)0x10);
    bc_core_fill(b, 32, (unsigned char)0x20);
    int out_result = 99;

    bool result = bc_core_compare(a, b, 32, &out_result);

    assert_true(result);
    assert_int_equal(out_result, -1);
}

static void test_compare_a_greater_than_b_returns_one(void** state)
{
    BC_UNUSED(state);

    unsigned char a[32];
    unsigned char b[32];
    bc_core_fill(a, 32, (unsigned char)0x20);
    bc_core_fill(b, 32, (unsigned char)0x10);
    int out_result = 99;

    bool result = bc_core_compare(a, b, 32, &out_result);

    assert_true(result);
    assert_int_equal(out_result, 1);
}

static void test_compare_difference_in_vector_block_a_less(void** state)
{
    BC_UNUSED(state);

    unsigned char a[64];
    unsigned char b[64];
    bc_core_fill(a, 64, (unsigned char)0x50);
    bc_core_fill(b, 64, (unsigned char)0x50);
    b[10] = 0x60;
    int out_result = 99;

    bool result = bc_core_compare(a, b, 64, &out_result);

    assert_true(result);
    assert_int_equal(out_result, -1);
}

static void test_compare_difference_in_vector_block_a_greater(void** state)
{
    BC_UNUSED(state);

    unsigned char a[64];
    unsigned char b[64];
    bc_core_fill(a, 64, (unsigned char)0x50);
    bc_core_fill(b, 64, (unsigned char)0x50);
    a[10] = 0x60;
    int out_result = 99;

    bool result = bc_core_compare(a, b, 64, &out_result);

    assert_true(result);
    assert_int_equal(out_result, 1);
}

static void test_compare_difference_in_tail_bytes_a_less(void** state)
{
    BC_UNUSED(state);

    unsigned char a[35];
    unsigned char b[35];
    bc_core_fill(a, 35, (unsigned char)0x50);
    bc_core_fill(b, 35, (unsigned char)0x50);
    b[33] = 0x60;
    int out_result = 99;

    bool result = bc_core_compare(a, b, 35, &out_result);

    assert_true(result);
    assert_int_equal(out_result, -1);
}

static void test_compare_difference_in_tail_bytes_a_greater(void** state)
{
    BC_UNUSED(state);

    unsigned char a[35];
    unsigned char b[35];
    bc_core_fill(a, 35, (unsigned char)0x50);
    bc_core_fill(b, 35, (unsigned char)0x50);
    a[33] = 0x60;
    int out_result = 99;

    bool result = bc_core_compare(a, b, 35, &out_result);

    assert_true(result);
    assert_int_equal(out_result, 1);
}

static void test_compare_len_1_a_less_than_b(void** state)
{
    BC_UNUSED(state);

    const unsigned char a[1] = {0x01};
    const unsigned char b[1] = {0x02};
    int out_result = 99;

    bool result = bc_core_compare(a, b, 1, &out_result);

    assert_true(result);
    assert_int_equal(out_result, -1);
}

static void test_compare_len_1_a_greater_than_b(void** state)
{
    BC_UNUSED(state);

    const unsigned char a[1] = {0x02};
    const unsigned char b[1] = {0x01};
    int out_result = 99;

    bool result = bc_core_compare(a, b, 1, &out_result);

    assert_true(result);
    assert_int_equal(out_result, 1);
}

static void test_compare_len_1024_identical(void** state)
{
    BC_UNUSED(state);

    unsigned char a[1024];
    unsigned char b[1024];
    bc_core_fill(a, 1024, (unsigned char)0x7F);
    bc_core_fill(b, 1024, (unsigned char)0x7F);
    int out_result = 99;

    bool result = bc_core_compare(a, b, 1024, &out_result);

    assert_true(result);
    assert_int_equal(out_result, 0);
}

static void run_equal_at_length(size_t length, bool equal_expected, size_t diff_at)
{
    unsigned char* a = malloc(length + 64);
    unsigned char* b = malloc(length + 64);
    assert_non_null(a);
    assert_non_null(b);
    bc_core_fill(a, length + 64, (unsigned char)0x55);
    bc_core_fill(b, length + 64, (unsigned char)0x55);
    if (!equal_expected) {
        b[diff_at] = (unsigned char)0xAA;
    }
    bool out_equal = !equal_expected;
    bool result = bc_core_equal(a, b, length, &out_equal);
    assert_true(result);
    assert_int_equal(out_equal, equal_expected ? 1 : 0);
    free(a);
    free(b);
}

static void test_equal_unroll_boundary_127_identical(void** state)
{
    BC_UNUSED(state);
    run_equal_at_length(127, true, 0);
}

static void test_equal_unroll_boundary_128_identical(void** state)
{
    BC_UNUSED(state);
    run_equal_at_length(128, true, 0);
}

static void test_equal_unroll_boundary_128_diff_first(void** state)
{
    BC_UNUSED(state);
    run_equal_at_length(128, false, 0);
}

static void test_equal_unroll_boundary_128_diff_byte_31(void** state)
{
    BC_UNUSED(state);
    run_equal_at_length(128, false, 31);
}

static void test_equal_unroll_boundary_128_diff_byte_32(void** state)
{
    BC_UNUSED(state);
    run_equal_at_length(128, false, 32);
}

static void test_equal_unroll_boundary_128_diff_byte_63(void** state)
{
    BC_UNUSED(state);
    run_equal_at_length(128, false, 63);
}

static void test_equal_unroll_boundary_128_diff_byte_95(void** state)
{
    BC_UNUSED(state);
    run_equal_at_length(128, false, 95);
}

static void test_equal_unroll_boundary_128_diff_last(void** state)
{
    BC_UNUSED(state);
    run_equal_at_length(128, false, 127);
}

static void test_equal_unroll_boundary_129_diff_in_unroll(void** state)
{
    BC_UNUSED(state);
    run_equal_at_length(129, false, 64);
}

static void test_equal_unroll_boundary_129_diff_in_tail(void** state)
{
    BC_UNUSED(state);
    run_equal_at_length(129, false, 128);
}

static void test_equal_unroll_boundary_159_diff_in_tail(void** state)
{
    BC_UNUSED(state);
    run_equal_at_length(159, false, 158);
}

static void test_equal_unroll_boundary_160_diff_at_chunk_seam(void** state)
{
    BC_UNUSED(state);
    run_equal_at_length(160, false, 128);
}

static void test_equal_unroll_boundary_256_diff_at_second_unroll(void** state)
{
    BC_UNUSED(state);
    run_equal_at_length(256, false, 200);
}

static void test_equal_unroll_unaligned_offset_1_len_128(void** state)
{
    BC_UNUSED(state);
    unsigned char* base_a = malloc(256);
    unsigned char* base_b = malloc(256);
    assert_non_null(base_a);
    assert_non_null(base_b);
    bc_core_fill(base_a, 256, (unsigned char)0x33);
    bc_core_fill(base_b, 256, (unsigned char)0x33);
    bool out_equal = false;
    assert_true(bc_core_equal(base_a + 1, base_b + 1, 128, &out_equal));
    assert_true(out_equal);
    free(base_a);
    free(base_b);
}

static void test_equal_unroll_unaligned_offset_7_len_129_diff_at_64(void** state)
{
    BC_UNUSED(state);
    unsigned char* base_a = malloc(256);
    unsigned char* base_b = malloc(256);
    assert_non_null(base_a);
    assert_non_null(base_b);
    bc_core_fill(base_a, 256, (unsigned char)0x33);
    bc_core_fill(base_b, 256, (unsigned char)0x33);
    base_b[7 + 64] = (unsigned char)0xCC;
    bool out_equal = true;
    assert_true(bc_core_equal(base_a + 7, base_b + 7, 129, &out_equal));
    assert_false(out_equal);
    free(base_a);
    free(base_b);
}

static void test_equal_unroll_unaligned_offset_31_len_192(void** state)
{
    BC_UNUSED(state);
    unsigned char* base_a = malloc(384);
    unsigned char* base_b = malloc(384);
    assert_non_null(base_a);
    assert_non_null(base_b);
    bc_core_fill(base_a, 384, (unsigned char)0x77);
    bc_core_fill(base_b, 384, (unsigned char)0x77);
    bool out_equal = false;
    assert_true(bc_core_equal(base_a + 31, base_b + 31, 192, &out_equal));
    assert_true(out_equal);
    free(base_a);
    free(base_b);
}

static void run_compare_at_length_diff(size_t length, size_t diff_at, unsigned char a_value, unsigned char b_value, int expected)
{
    unsigned char* a = malloc(length + 64);
    unsigned char* b = malloc(length + 64);
    assert_non_null(a);
    assert_non_null(b);
    bc_core_fill(a, length + 64, (unsigned char)0x42);
    bc_core_fill(b, length + 64, (unsigned char)0x42);
    a[diff_at] = a_value;
    b[diff_at] = b_value;
    int out_result = 99;
    assert_true(bc_core_compare(a, b, length, &out_result));
    assert_int_equal(out_result, expected);
    free(a);
    free(b);
}

static void test_compare_unroll_boundary_128_diff_first_a_less(void** state)
{
    BC_UNUSED(state);
    run_compare_at_length_diff(128, 0, 0x10, 0x20, -1);
}

static void test_compare_unroll_boundary_128_diff_byte_64_a_greater(void** state)
{
    BC_UNUSED(state);
    run_compare_at_length_diff(128, 64, 0x80, 0x10, 1);
}

static void test_compare_unroll_boundary_128_diff_last_a_less(void** state)
{
    BC_UNUSED(state);
    run_compare_at_length_diff(128, 127, 0x05, 0x06, -1);
}

static void test_compare_unroll_boundary_129_diff_in_tail(void** state)
{
    BC_UNUSED(state);
    run_compare_at_length_diff(129, 128, 0x77, 0x10, 1);
}

static void test_compare_unroll_boundary_256_diff_in_second_unroll(void** state)
{
    BC_UNUSED(state);
    run_compare_at_length_diff(256, 200, 0x10, 0x20, -1);
}

static void test_compare_unroll_unaligned_offset_3_len_128_diff_at_64(void** state)
{
    BC_UNUSED(state);
    unsigned char* base_a = malloc(256);
    unsigned char* base_b = malloc(256);
    assert_non_null(base_a);
    assert_non_null(base_b);
    bc_core_fill(base_a, 256, (unsigned char)0x42);
    bc_core_fill(base_b, 256, (unsigned char)0x42);
    base_a[3 + 64] = (unsigned char)0x10;
    base_b[3 + 64] = (unsigned char)0x20;
    int out_result = 99;
    assert_true(bc_core_compare(base_a + 3, base_b + 3, 128, &out_result));
    assert_int_equal(out_result, -1);
    free(base_a);
    free(base_b);
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_equal_len_zero_sets_out_equal_true),
        cmocka_unit_test(test_equal_len_1_identical_bytes),
        cmocka_unit_test(test_equal_len_1_different_bytes),
        cmocka_unit_test(test_equal_len_8_identical_uint64_accumulation),
        cmocka_unit_test(test_equal_len_8_different_last_byte_uint64_accumulation),
        cmocka_unit_test(test_equal_len_9_single_vector_path_identical),
        cmocka_unit_test(test_equal_len_9_single_vector_path_different),
        cmocka_unit_test(test_equal_len_32_single_vector_full_mask_identical),
        cmocka_unit_test(test_equal_len_32_single_vector_full_mask_different),
        cmocka_unit_test(test_equal_len_33_multi_vector_identical),
        cmocka_unit_test(test_equal_len_33_multi_vector_difference_in_vector_block),
        cmocka_unit_test(test_equal_len_33_multi_vector_difference_in_tail),
        cmocka_unit_test(test_equal_len_64_multi_vector_identical),
        cmocka_unit_test(test_equal_len_1024_multi_vector_identical),
        cmocka_unit_test(test_equal_len_1024_multi_vector_difference_at_start),
        cmocka_unit_test(test_compare_len_zero_sets_out_result_zero),
        cmocka_unit_test(test_compare_identical_buffers_returns_zero),
        cmocka_unit_test(test_compare_a_less_than_b_returns_minus_one),
        cmocka_unit_test(test_compare_a_greater_than_b_returns_one),
        cmocka_unit_test(test_compare_difference_in_vector_block_a_less),
        cmocka_unit_test(test_compare_difference_in_vector_block_a_greater),
        cmocka_unit_test(test_compare_difference_in_tail_bytes_a_less),
        cmocka_unit_test(test_compare_difference_in_tail_bytes_a_greater),
        cmocka_unit_test(test_compare_len_1_a_less_than_b),
        cmocka_unit_test(test_compare_len_1_a_greater_than_b),
        cmocka_unit_test(test_compare_len_1024_identical),
        cmocka_unit_test(test_equal_unroll_boundary_127_identical),
        cmocka_unit_test(test_equal_unroll_boundary_128_identical),
        cmocka_unit_test(test_equal_unroll_boundary_128_diff_first),
        cmocka_unit_test(test_equal_unroll_boundary_128_diff_byte_31),
        cmocka_unit_test(test_equal_unroll_boundary_128_diff_byte_32),
        cmocka_unit_test(test_equal_unroll_boundary_128_diff_byte_63),
        cmocka_unit_test(test_equal_unroll_boundary_128_diff_byte_95),
        cmocka_unit_test(test_equal_unroll_boundary_128_diff_last),
        cmocka_unit_test(test_equal_unroll_boundary_129_diff_in_unroll),
        cmocka_unit_test(test_equal_unroll_boundary_129_diff_in_tail),
        cmocka_unit_test(test_equal_unroll_boundary_159_diff_in_tail),
        cmocka_unit_test(test_equal_unroll_boundary_160_diff_at_chunk_seam),
        cmocka_unit_test(test_equal_unroll_boundary_256_diff_at_second_unroll),
        cmocka_unit_test(test_equal_unroll_unaligned_offset_1_len_128),
        cmocka_unit_test(test_equal_unroll_unaligned_offset_7_len_129_diff_at_64),
        cmocka_unit_test(test_equal_unroll_unaligned_offset_31_len_192),
        cmocka_unit_test(test_compare_unroll_boundary_128_diff_first_a_less),
        cmocka_unit_test(test_compare_unroll_boundary_128_diff_byte_64_a_greater),
        cmocka_unit_test(test_compare_unroll_boundary_128_diff_last_a_less),
        cmocka_unit_test(test_compare_unroll_boundary_129_diff_in_tail),
        cmocka_unit_test(test_compare_unroll_boundary_256_diff_in_second_unroll),
        cmocka_unit_test(test_compare_unroll_unaligned_offset_3_len_128_diff_at_64),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
