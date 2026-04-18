// SPDX-License-Identifier: MIT

#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <cmocka.h>

#include "bc_core.h"

#include "bc_core_cache_sizes_internal.h"

#include <stdlib.h>

#define SIMD_PADDING 32

static void test_zero_empty_buffer_returns_true(void** state)
{
    BC_UNUSED(state);

    unsigned char buffer[1] = {0xFF};
    bool success = bc_core_zero(buffer, 0);

    assert_true(success);
    assert_int_equal(buffer[0], 0xFF);
}

static void test_zero_len_1_sets_byte_to_zero(void** state)
{
    BC_UNUSED(state);

    unsigned char buffer[1 + SIMD_PADDING] = {0};
    bc_core_fill(buffer, sizeof(buffer), (unsigned char)0xFF);

    bool success = bc_core_zero(buffer, 1);

    assert_true(success);
    assert_int_equal(buffer[0], 0);
}

static void test_zero_len_15_scalar_path_clears_buffer(void** state)
{
    BC_UNUSED(state);

    unsigned char* buffer = malloc(15 + SIMD_PADDING);
    assert_non_null(buffer);
    bc_core_fill(buffer, 15 + SIMD_PADDING, (unsigned char)0xFF);

    bool success = bc_core_zero(buffer, 15);

    assert_true(success);
    for (size_t index = 0; index < 15; index++) {
        assert_int_equal(buffer[index], 0);
    }

    free(buffer);
}

static void test_zero_len_16_scalar_path_clears_buffer(void** state)
{
    BC_UNUSED(state);

    unsigned char* buffer = malloc(16 + SIMD_PADDING);
    assert_non_null(buffer);
    bc_core_fill(buffer, 16 + SIMD_PADDING, (unsigned char)0xFF);

    bool success = bc_core_zero(buffer, 16);

    assert_true(success);
    for (size_t index = 0; index < 16; index++) {
        assert_int_equal(buffer[index], 0);
    }

    free(buffer);
}

static void test_zero_len_31_scalar_path_clears_buffer(void** state)
{
    BC_UNUSED(state);

    unsigned char* buffer = malloc(31 + SIMD_PADDING);
    assert_non_null(buffer);
    bc_core_fill(buffer, 31 + SIMD_PADDING, (unsigned char)0xFF);

    bool success = bc_core_zero(buffer, 31);

    assert_true(success);
    for (size_t index = 0; index < 31; index++) {
        assert_int_equal(buffer[index], 0);
    }

    free(buffer);
}

static void test_zero_len_32_head_tail_path_clears_buffer(void** state)
{
    BC_UNUSED(state);

    unsigned char* buffer = malloc(32 + SIMD_PADDING);
    assert_non_null(buffer);
    bc_core_fill(buffer, 32 + SIMD_PADDING, (unsigned char)0xFF);

    bool success = bc_core_zero(buffer, 32);

    assert_true(success);
    for (size_t index = 0; index < 32; index++) {
        assert_int_equal(buffer[index], 0);
    }

    free(buffer);
}

static void test_zero_len_33_head_tail_path_clears_buffer(void** state)
{
    BC_UNUSED(state);

    unsigned char* buffer = malloc(33 + SIMD_PADDING);
    assert_non_null(buffer);
    bc_core_fill(buffer, 33 + SIMD_PADDING, (unsigned char)0xFF);

    bool success = bc_core_zero(buffer, 33);

    assert_true(success);
    for (size_t index = 0; index < 33; index++) {
        assert_int_equal(buffer[index], 0);
    }

    free(buffer);
}

static void test_zero_len_63_head_tail_path_clears_buffer(void** state)
{
    BC_UNUSED(state);

    unsigned char* buffer = malloc(63 + SIMD_PADDING);
    assert_non_null(buffer);
    bc_core_fill(buffer, 63 + SIMD_PADDING, (unsigned char)0xFF);

    bool success = bc_core_zero(buffer, 63);

    assert_true(success);
    for (size_t index = 0; index < 63; index++) {
        assert_int_equal(buffer[index], 0);
    }

    free(buffer);
}

static void test_zero_len_64_head_tail_path_clears_buffer(void** state)
{
    BC_UNUSED(state);

    unsigned char* buffer = malloc(64 + SIMD_PADDING);
    assert_non_null(buffer);
    bc_core_fill(buffer, 64 + SIMD_PADDING, (unsigned char)0xFF);

    bool success = bc_core_zero(buffer, 64);

    assert_true(success);
    for (size_t index = 0; index < 64; index++) {
        assert_int_equal(buffer[index], 0);
    }

    free(buffer);
}

static void test_zero_len_65_head_mid_tail_path_clears_buffer(void** state)
{
    BC_UNUSED(state);

    unsigned char* buffer = malloc(65 + SIMD_PADDING);
    assert_non_null(buffer);
    bc_core_fill(buffer, 65 + SIMD_PADDING, (unsigned char)0xFF);

    bool success = bc_core_zero(buffer, 65);

    assert_true(success);
    for (size_t index = 0; index < 65; index++) {
        assert_int_equal(buffer[index], 0);
    }

    free(buffer);
}

static void test_zero_len_127_head_mid_tail_path_clears_buffer(void** state)
{
    BC_UNUSED(state);

    unsigned char* buffer = malloc(127 + SIMD_PADDING);
    assert_non_null(buffer);
    bc_core_fill(buffer, 127 + SIMD_PADDING, (unsigned char)0xFF);

    bool success = bc_core_zero(buffer, 127);

    assert_true(success);
    for (size_t index = 0; index < 127; index++) {
        assert_int_equal(buffer[index], 0);
    }

    free(buffer);
}

static void test_zero_len_128_head_mid_tail_path_clears_buffer(void** state)
{
    BC_UNUSED(state);

    unsigned char* buffer = malloc(128 + SIMD_PADDING);
    assert_non_null(buffer);
    bc_core_fill(buffer, 128 + SIMD_PADDING, (unsigned char)0xFF);

    bool success = bc_core_zero(buffer, 128);

    assert_true(success);
    for (size_t index = 0; index < 128; index++) {
        assert_int_equal(buffer[index], 0);
    }

    free(buffer);
}

static void test_zero_len_129_aligned_stores_loop_clears_buffer(void** state)
{
    BC_UNUSED(state);

    unsigned char* buffer = malloc(129 + SIMD_PADDING);
    assert_non_null(buffer);
    bc_core_fill(buffer, 129 + SIMD_PADDING, (unsigned char)0xFF);

    bool success = bc_core_zero(buffer, 129);

    assert_true(success);
    for (size_t index = 0; index < 129; index++) {
        assert_int_equal(buffer[index], 0);
    }

    free(buffer);
}

static void test_zero_len_256_aligned_stores_loop_clears_buffer(void** state)
{
    BC_UNUSED(state);

    unsigned char* buffer = malloc(256 + SIMD_PADDING);
    assert_non_null(buffer);
    bc_core_fill(buffer, 256 + SIMD_PADDING, (unsigned char)0xFF);

    bool success = bc_core_zero(buffer, 256);

    assert_true(success);
    for (size_t index = 0; index < 256; index++) {
        assert_int_equal(buffer[index], 0);
    }

    free(buffer);
}

static void test_zero_len_1024_aligned_stores_loop_clears_buffer(void** state)
{
    BC_UNUSED(state);

    unsigned char* buffer = malloc(1024 + SIMD_PADDING);
    assert_non_null(buffer);
    bc_core_fill(buffer, 1024 + SIMD_PADDING, (unsigned char)0xFF);

    bool success = bc_core_zero(buffer, 1024);

    assert_true(success);
    for (size_t index = 0; index < 1024; index++) {
        assert_int_equal(buffer[index], 0);
    }

    free(buffer);
}

static void test_zero_len_4096_aligned_stores_loop_clears_buffer(void** state)
{
    BC_UNUSED(state);

    unsigned char* buffer = malloc(4096 + SIMD_PADDING);
    assert_non_null(buffer);
    bc_core_fill(buffer, 4096 + SIMD_PADDING, (unsigned char)0xFF);

    bool success = bc_core_zero(buffer, 4096);

    assert_true(success);
    for (size_t index = 0; index < 4096; index++) {
        assert_int_equal(buffer[index], 0);
    }

    free(buffer);
}

static void test_fill_empty_buffer_returns_true(void** state)
{
    BC_UNUSED(state);

    unsigned char buffer[1] = {0x00};
    bool success = bc_core_fill(buffer, 0, 0xAB);

    assert_true(success);
    assert_int_equal(buffer[0], 0x00);
}

static void test_fill_len_1_sets_single_byte(void** state)
{
    BC_UNUSED(state);

    unsigned char buffer[1 + SIMD_PADDING] = {0};
    bc_core_fill(buffer, sizeof(buffer), (unsigned char)0x00);

    bool success = bc_core_fill(buffer, 1, 0xAB);

    assert_true(success);
    assert_int_equal(buffer[0], 0xAB);
}

static void test_fill_len_15_fills_all_bytes(void** state)
{
    BC_UNUSED(state);

    unsigned char* buffer = malloc(15 + SIMD_PADDING);
    assert_non_null(buffer);
    bc_core_fill(buffer, 15 + SIMD_PADDING, (unsigned char)0x00);

    bool success = bc_core_fill(buffer, 15, 0x5A);

    assert_true(success);
    for (size_t index = 0; index < 15; index++) {
        assert_int_equal(buffer[index], 0x5A);
    }

    free(buffer);
}

static void test_fill_len_32_aligned_fills_all_bytes(void** state)
{
    BC_UNUSED(state);

    unsigned char* buffer = malloc(32 + SIMD_PADDING);
    assert_non_null(buffer);
    bc_core_fill(buffer, 32 + SIMD_PADDING, (unsigned char)0x00);

    bool success = bc_core_fill(buffer, 32, 0xCC);

    assert_true(success);
    for (size_t index = 0; index < 32; index++) {
        assert_int_equal(buffer[index], 0xCC);
    }

    free(buffer);
}

static void test_fill_len_3_unaligned_head_larger_than_len_clips_to_len(void** state)
{
    BC_UNUSED(state);

    unsigned char* buffer = malloc(32 + 3 + SIMD_PADDING);
    assert_non_null(buffer);
    unsigned char* unaligned_buffer = buffer + 1;
    bc_core_fill(unaligned_buffer, 3 + SIMD_PADDING, (unsigned char)0x00);

    bool success = bc_core_fill(unaligned_buffer, 3, 0xAA);

    assert_true(success);
    for (size_t index = 0; index < 3; index++) {
        assert_int_equal(unaligned_buffer[index], 0xAA);
    }

    free(buffer);
}

static void test_fill_len_33_unaligned_head_then_vectorized(void** state)
{
    BC_UNUSED(state);

    unsigned char* buffer = malloc(33 + 1 + SIMD_PADDING);
    assert_non_null(buffer);
    unsigned char* unaligned_buffer = buffer + 1;
    bc_core_fill(unaligned_buffer, 33 + SIMD_PADDING, (unsigned char)0x00);

    bool success = bc_core_fill(unaligned_buffer, 33, 0x77);

    assert_true(success);
    for (size_t index = 0; index < 33; index++) {
        assert_int_equal(unaligned_buffer[index], 0x77);
    }

    free(buffer);
}

static void test_fill_len_63_fills_all_bytes(void** state)
{
    BC_UNUSED(state);

    unsigned char* buffer = malloc(63 + SIMD_PADDING);
    assert_non_null(buffer);
    bc_core_fill(buffer, 63 + SIMD_PADDING, (unsigned char)0x00);

    bool success = bc_core_fill(buffer, 63, 0x42);

    assert_true(success);
    for (size_t index = 0; index < 63; index++) {
        assert_int_equal(buffer[index], 0x42);
    }

    free(buffer);
}

static void test_fill_len_64_fills_all_bytes(void** state)
{
    BC_UNUSED(state);

    unsigned char* buffer = malloc(64 + SIMD_PADDING);
    assert_non_null(buffer);
    bc_core_fill(buffer, 64 + SIMD_PADDING, (unsigned char)0x00);

    bool success = bc_core_fill(buffer, 64, 0x11);

    assert_true(success);
    for (size_t index = 0; index < 64; index++) {
        assert_int_equal(buffer[index], 0x11);
    }

    free(buffer);
}

static void test_fill_len_65_fills_all_bytes(void** state)
{
    BC_UNUSED(state);

    unsigned char* buffer = malloc(65 + SIMD_PADDING);
    assert_non_null(buffer);
    bc_core_fill(buffer, 65 + SIMD_PADDING, (unsigned char)0x00);

    bool success = bc_core_fill(buffer, 65, 0x99);

    assert_true(success);
    for (size_t index = 0; index < 65; index++) {
        assert_int_equal(buffer[index], 0x99);
    }

    free(buffer);
}

static void test_fill_len_128_fills_all_bytes(void** state)
{
    BC_UNUSED(state);

    unsigned char* buffer = malloc(128 + SIMD_PADDING);
    assert_non_null(buffer);
    bc_core_fill(buffer, 128 + SIMD_PADDING, (unsigned char)0x00);

    bool success = bc_core_fill(buffer, 128, 0xBB);

    assert_true(success);
    for (size_t index = 0; index < 128; index++) {
        assert_int_equal(buffer[index], 0xBB);
    }

    free(buffer);
}

static void test_fill_len_256_fills_all_bytes(void** state)
{
    BC_UNUSED(state);

    unsigned char* buffer = malloc(256 + SIMD_PADDING);
    assert_non_null(buffer);
    bc_core_fill(buffer, 256 + SIMD_PADDING, (unsigned char)0x00);

    bool success = bc_core_fill(buffer, 256, 0xDE);

    assert_true(success);
    for (size_t index = 0; index < 256; index++) {
        assert_int_equal(buffer[index], 0xDE);
    }

    free(buffer);
}

static void test_fill_len_4096_fills_all_bytes(void** state)
{
    BC_UNUSED(state);

    unsigned char* buffer = malloc(4096 + SIMD_PADDING);
    assert_non_null(buffer);
    bc_core_fill(buffer, 4096 + SIMD_PADDING, (unsigned char)0x00);

    bool success = bc_core_fill(buffer, 4096, 0xF0);

    assert_true(success);
    for (size_t index = 0; index < 4096; index++) {
        assert_int_equal(buffer[index], 0xF0);
    }

    free(buffer);
}

static void test_fill_value_zero_fills_all_bytes_with_zero(void** state)
{
    BC_UNUSED(state);

    unsigned char* buffer = malloc(64 + SIMD_PADDING);
    assert_non_null(buffer);
    bc_core_fill(buffer, 64 + SIMD_PADDING, (unsigned char)0xFF);

    bool success = bc_core_fill(buffer, 64, 0x00);

    assert_true(success);
    for (size_t index = 0; index < 64; index++) {
        assert_int_equal(buffer[index], 0x00);
    }

    free(buffer);
}

#define CANARY_GUARD_SIZE 128

static void check_fill_len_no_overflow(size_t len, unsigned char fill_value)
{
    const size_t total_size = CANARY_GUARD_SIZE + len + CANARY_GUARD_SIZE;
    unsigned char* backing = malloc(total_size);
    assert_non_null(backing);
    bc_core_fill(backing, total_size, (unsigned char)0xA5);

    unsigned char* target = backing + CANARY_GUARD_SIZE;

    bool success = bc_core_fill(target, len, fill_value);
    assert_true(success);

    for (size_t index = 0; index < CANARY_GUARD_SIZE; index++) {
        assert_int_equal(backing[index], 0xA5);
    }
    for (size_t index = 0; index < len; index++) {
        assert_int_equal(target[index], fill_value);
    }
    for (size_t index = 0; index < CANARY_GUARD_SIZE; index++) {
        assert_int_equal(target[len + index], 0xA5);
    }

    free(backing);
}

static void test_fill_no_overflow_len_sweep_32_to_128(void** state)
{
    BC_UNUSED(state);

    for (size_t len = 32; len <= 128; len++) {
        check_fill_len_no_overflow(len, 0x3C);
    }
}

static void test_fill_no_overflow_len_1_to_31(void** state)
{
    BC_UNUSED(state);

    for (size_t len = 1; len <= 31; len++) {
        check_fill_len_no_overflow(len, 0x71);
    }
}

static void test_fill_no_overflow_len_129_to_256(void** state)
{
    BC_UNUSED(state);

    for (size_t len = 129; len <= 256; len++) {
        check_fill_len_no_overflow(len, 0x5E);
    }
}

static void check_zero_len_no_overflow(size_t len)
{
    const size_t total_size = CANARY_GUARD_SIZE + len + CANARY_GUARD_SIZE;
    unsigned char* backing = malloc(total_size);
    assert_non_null(backing);
    bc_core_fill(backing, total_size, (unsigned char)0xA5);

    unsigned char* target = backing + CANARY_GUARD_SIZE;

    bool success = bc_core_zero(target, len);
    assert_true(success);

    for (size_t index = 0; index < CANARY_GUARD_SIZE; index++) {
        assert_int_equal(backing[index], 0xA5);
    }
    for (size_t index = 0; index < len; index++) {
        assert_int_equal(target[index], 0x00);
    }
    for (size_t index = 0; index < CANARY_GUARD_SIZE; index++) {
        assert_int_equal(target[len + index], 0xA5);
    }

    free(backing);
}

static void test_zero_no_overflow_len_1_to_31(void** state)
{
    BC_UNUSED(state);

    for (size_t len = 1; len <= 31; len++) {
        check_zero_len_no_overflow(len);
    }
}

static void test_zero_no_overflow_len_sweep_32_to_128(void** state)
{
    BC_UNUSED(state);

    for (size_t len = 32; len <= 128; len++) {
        check_zero_len_no_overflow(len);
    }
}

static void test_zero_no_overflow_len_129_to_256(void** state)
{
    BC_UNUSED(state);

    for (size_t len = 129; len <= 256; len++) {
        check_zero_len_no_overflow(len);
    }
}

static void test_zero_secure_empty_buffer_returns_true(void** state)
{
    BC_UNUSED(state);

    unsigned char buffer[1] = {0xFF};
    bool success = bc_core_zero_secure(buffer, 0);

    assert_true(success);
    assert_int_equal(buffer[0], 0xFF);
}

static void test_zero_secure_len_1_clears_byte(void** state)
{
    BC_UNUSED(state);

    unsigned char buffer[1] = {0xFF};
    bool success = bc_core_zero_secure(buffer, 1);

    assert_true(success);
    assert_int_equal(buffer[0], 0);
}

static void test_zero_secure_len_31_clears_all_bytes(void** state)
{
    BC_UNUSED(state);

    unsigned char* buffer = malloc(31);
    assert_non_null(buffer);
    bc_core_fill(buffer, 31, (unsigned char)0xFF);

    bool success = bc_core_zero_secure(buffer, 31);

    assert_true(success);
    for (size_t index = 0; index < 31; index++) {
        assert_int_equal(buffer[index], 0);
    }

    free(buffer);
}

static void test_zero_secure_len_32_clears_all_bytes(void** state)
{
    BC_UNUSED(state);

    unsigned char* buffer = malloc(32);
    assert_non_null(buffer);
    bc_core_fill(buffer, 32, (unsigned char)0xFF);

    bool success = bc_core_zero_secure(buffer, 32);

    assert_true(success);
    for (size_t index = 0; index < 32; index++) {
        assert_int_equal(buffer[index], 0);
    }

    free(buffer);
}

static void test_zero_secure_len_128_clears_all_bytes(void** state)
{
    BC_UNUSED(state);

    unsigned char* buffer = malloc(128);
    assert_non_null(buffer);
    bc_core_fill(buffer, 128, (unsigned char)0xFF);

    bool success = bc_core_zero_secure(buffer, 128);

    assert_true(success);
    for (size_t index = 0; index < 128; index++) {
        assert_int_equal(buffer[index], 0);
    }

    free(buffer);
}

static void test_zero_secure_len_256_clears_all_bytes(void** state)
{
    BC_UNUSED(state);

    unsigned char* buffer = malloc(256);
    assert_non_null(buffer);
    bc_core_fill(buffer, 256, (unsigned char)0xFF);

    bool success = bc_core_zero_secure(buffer, 256);

    assert_true(success);
    for (size_t index = 0; index < 256; index++) {
        assert_int_equal(buffer[index], 0);
    }

    free(buffer);
}

static void test_zero_secure_len_1024_clears_all_bytes(void** state)
{
    BC_UNUSED(state);

    unsigned char* buffer = malloc(1024);
    assert_non_null(buffer);
    bc_core_fill(buffer, 1024, (unsigned char)0xFF);

    bool success = bc_core_zero_secure(buffer, 1024);

    assert_true(success);
    for (size_t index = 0; index < 1024; index++) {
        assert_int_equal(buffer[index], 0);
    }

    free(buffer);
}

static void test_zero_secure_len_4096_clears_all_bytes(void** state)
{
    BC_UNUSED(state);

    unsigned char* buffer = malloc(4096);
    assert_non_null(buffer);
    bc_core_fill(buffer, 4096, (unsigned char)0xFF);

    bool success = bc_core_zero_secure(buffer, 4096);

    assert_true(success);
    for (size_t index = 0; index < 4096; index++) {
        assert_int_equal(buffer[index], 0);
    }

    free(buffer);
}

static void test_get_l2_cache_size_returns_positive_value(void** state)
{
    BC_UNUSED(state);

    size_t result = bc_core_cached_l2_cache_size();

    assert_true(result > 0);
}

static void test_get_l3_cache_size_returns_positive_value(void** state)
{
    BC_UNUSED(state);

    size_t result = bc_core_cached_l3_cache_size();

    assert_true(result > 0);
}

static void test_zero_len_larger_than_l3_uses_nt_stores(void** state)
{
    BC_UNUSED(state);

    size_t l3_cache_size = bc_core_cached_l3_cache_size();
    size_t buffer_size = l3_cache_size + 256 + SIMD_PADDING;
    unsigned char* buffer = malloc(buffer_size);
    assert_non_null(buffer);
    bc_core_fill(buffer, buffer_size, (unsigned char)0xFF);

    bool success = bc_core_zero(buffer, l3_cache_size + 256);

    assert_true(success);
    for (size_t index = 0; index < l3_cache_size + 256; index++) {
        assert_int_equal(buffer[index], 0);
    }

    free(buffer);
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_zero_empty_buffer_returns_true),
        cmocka_unit_test(test_zero_len_1_sets_byte_to_zero),
        cmocka_unit_test(test_zero_len_15_scalar_path_clears_buffer),
        cmocka_unit_test(test_zero_len_16_scalar_path_clears_buffer),
        cmocka_unit_test(test_zero_len_31_scalar_path_clears_buffer),
        cmocka_unit_test(test_zero_len_32_head_tail_path_clears_buffer),
        cmocka_unit_test(test_zero_len_33_head_tail_path_clears_buffer),
        cmocka_unit_test(test_zero_len_63_head_tail_path_clears_buffer),
        cmocka_unit_test(test_zero_len_64_head_tail_path_clears_buffer),
        cmocka_unit_test(test_zero_len_65_head_mid_tail_path_clears_buffer),
        cmocka_unit_test(test_zero_len_127_head_mid_tail_path_clears_buffer),
        cmocka_unit_test(test_zero_len_128_head_mid_tail_path_clears_buffer),
        cmocka_unit_test(test_zero_len_129_aligned_stores_loop_clears_buffer),
        cmocka_unit_test(test_zero_len_256_aligned_stores_loop_clears_buffer),
        cmocka_unit_test(test_zero_len_1024_aligned_stores_loop_clears_buffer),
        cmocka_unit_test(test_zero_len_4096_aligned_stores_loop_clears_buffer),
        cmocka_unit_test(test_fill_empty_buffer_returns_true),
        cmocka_unit_test(test_fill_len_1_sets_single_byte),
        cmocka_unit_test(test_fill_len_15_fills_all_bytes),
        cmocka_unit_test(test_fill_len_32_aligned_fills_all_bytes),
        cmocka_unit_test(test_fill_len_3_unaligned_head_larger_than_len_clips_to_len),
        cmocka_unit_test(test_fill_len_33_unaligned_head_then_vectorized),
        cmocka_unit_test(test_fill_len_63_fills_all_bytes),
        cmocka_unit_test(test_fill_len_64_fills_all_bytes),
        cmocka_unit_test(test_fill_len_65_fills_all_bytes),
        cmocka_unit_test(test_fill_len_128_fills_all_bytes),
        cmocka_unit_test(test_fill_len_256_fills_all_bytes),
        cmocka_unit_test(test_fill_len_4096_fills_all_bytes),
        cmocka_unit_test(test_fill_value_zero_fills_all_bytes_with_zero),
        cmocka_unit_test(test_fill_no_overflow_len_1_to_31),
        cmocka_unit_test(test_fill_no_overflow_len_sweep_32_to_128),
        cmocka_unit_test(test_fill_no_overflow_len_129_to_256),
        cmocka_unit_test(test_zero_no_overflow_len_1_to_31),
        cmocka_unit_test(test_zero_no_overflow_len_sweep_32_to_128),
        cmocka_unit_test(test_zero_no_overflow_len_129_to_256),
        cmocka_unit_test(test_zero_secure_empty_buffer_returns_true),
        cmocka_unit_test(test_zero_secure_len_1_clears_byte),
        cmocka_unit_test(test_zero_secure_len_31_clears_all_bytes),
        cmocka_unit_test(test_zero_secure_len_32_clears_all_bytes),
        cmocka_unit_test(test_zero_secure_len_128_clears_all_bytes),
        cmocka_unit_test(test_zero_secure_len_256_clears_all_bytes),
        cmocka_unit_test(test_zero_secure_len_1024_clears_all_bytes),
        cmocka_unit_test(test_zero_secure_len_4096_clears_all_bytes),
        cmocka_unit_test(test_get_l2_cache_size_returns_positive_value),
        cmocka_unit_test(test_get_l3_cache_size_returns_positive_value),
        cmocka_unit_test(test_zero_len_larger_than_l3_uses_nt_stores),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
