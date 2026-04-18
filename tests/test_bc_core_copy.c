// SPDX-License-Identifier: MIT

#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <cmocka.h>
#include "bc_core.h"
#include <stdlib.h>

static void test_copy_len_zero_returns_true(void** state)
{
    BC_UNUSED(state);

    unsigned char destination[4] = {0xAA, 0xAA, 0xAA, 0xAA};
    const unsigned char source[4] = {0x11, 0x22, 0x33, 0x44};

    bool result = bc_core_copy(destination, source, 0);

    assert_true(result);
    assert_int_equal(destination[0], 0xAA);
}

static void test_copy_len_1_scalar_path(void** state)
{
    BC_UNUSED(state);

    unsigned char destination[1] = {0};
    const unsigned char source[1] = {0x42};

    bool result = bc_core_copy(destination, source, 1);

    assert_true(result);
    assert_int_equal(destination[0], 0x42);
}

static void test_copy_len_15_scalar_path(void** state)
{
    BC_UNUSED(state);

    unsigned char destination[15];
    unsigned char source[15];
    bc_core_fill(source, 15, (unsigned char)0xAB);
    bc_core_zero(destination, 15);

    bool result = bc_core_copy(destination, source, 15);

    assert_true(result);
    assert_memory_equal(destination, source, 15);
}

static void test_copy_len_31_scalar_path(void** state)
{
    BC_UNUSED(state);

    unsigned char destination[31];
    unsigned char source[31];
    bc_core_fill(source, 31, (unsigned char)0xCD);
    bc_core_zero(destination, 31);

    bool result = bc_core_copy(destination, source, 31);

    assert_true(result);
    assert_memory_equal(destination, source, 31);
}

static void test_copy_len_32_head_plus_tail_path(void** state)
{
    BC_UNUSED(state);

    unsigned char destination[32];
    unsigned char source[32];
    bc_core_fill(source, 32, (unsigned char)0x11);
    bc_core_zero(destination, 32);

    bool result = bc_core_copy(destination, source, 32);

    assert_true(result);
    assert_memory_equal(destination, source, 32);
}

static void test_copy_len_33_head_plus_tail_path(void** state)
{
    BC_UNUSED(state);

    unsigned char destination[128];
    unsigned char source[128];
    for (size_t i = 0; i < 33; i++) {
        source[i] = (unsigned char)i;
    }
    bc_core_zero(destination, 128);

    bool result = bc_core_copy(destination, source, 33);

    assert_true(result);
    assert_memory_equal(destination, source, 33);
}

static void test_copy_len_63_head_plus_mid_plus_tail_no_second_mid(void** state)
{
    BC_UNUSED(state);

    unsigned char destination[128];
    unsigned char source[128];
    for (size_t i = 0; i < 63; i++) {
        source[i] = (unsigned char)(i * 3);
    }
    bc_core_zero(destination, 128);

    bool result = bc_core_copy(destination, source, 63);

    assert_true(result);
    assert_memory_equal(destination, source, 63);
}

static void test_copy_len_64_head_plus_mid_plus_tail_no_second_mid(void** state)
{
    BC_UNUSED(state);

    unsigned char destination[128];
    unsigned char source[128];
    bc_core_fill(source, 64, (unsigned char)0x55);
    bc_core_zero(destination, 128);

    bool result = bc_core_copy(destination, source, 64);

    assert_true(result);
    assert_memory_equal(destination, source, 64);
}

static void test_copy_len_65_head_plus_mid1_mid2_plus_tail(void** state)
{
    BC_UNUSED(state);

    unsigned char destination[128];
    unsigned char source[128];
    for (size_t i = 0; i < 65; i++) {
        source[i] = (unsigned char)(i + 1);
    }
    bc_core_zero(destination, 128);

    bool result = bc_core_copy(destination, source, 65);

    assert_true(result);
    assert_memory_equal(destination, source, 65);
}

static void test_copy_len_127_head_plus_mid1_mid2_plus_tail(void** state)
{
    BC_UNUSED(state);

    unsigned char destination[128];
    unsigned char source[128];
    for (size_t i = 0; i < 127; i++) {
        source[i] = (unsigned char)(255 - i);
    }
    bc_core_zero(destination, 128);

    bool result = bc_core_copy(destination, source, 127);

    assert_true(result);
    assert_memory_equal(destination, source, 127);
}

static void test_copy_len_128_head_plus_mid1_mid2_plus_tail(void** state)
{
    BC_UNUSED(state);

    unsigned char destination[128];
    unsigned char source[128];
    bc_core_fill(source, 128, (unsigned char)0x77);
    bc_core_zero(destination, 128);

    bool result = bc_core_copy(destination, source, 128);

    assert_true(result);
    assert_memory_equal(destination, source, 128);
}

static void test_copy_len_129_aligned_store_path(void** state)
{
    BC_UNUSED(state);

    unsigned char destination[129];
    unsigned char source[129];
    for (size_t i = 0; i < 129; i++) {
        source[i] = (unsigned char)(i ^ 0x5A);
    }
    bc_core_zero(destination, 129);

    bool result = bc_core_copy(destination, source, 129);

    assert_true(result);
    assert_memory_equal(destination, source, 129);
}

static void test_copy_len_256_aligned_store_path(void** state)
{
    BC_UNUSED(state);

    unsigned char destination[256];
    unsigned char source[256];
    for (size_t i = 0; i < 256; i++) {
        source[i] = (unsigned char)i;
    }
    bc_core_zero(destination, 256);

    bool result = bc_core_copy(destination, source, 256);

    assert_true(result);
    assert_memory_equal(destination, source, 256);
}

static void test_copy_len_1024_aligned_store_path(void** state)
{
    BC_UNUSED(state);

    unsigned char destination[1024];
    unsigned char source[1024];
    for (size_t i = 0; i < 1024; i++) {
        source[i] = (unsigned char)(i & 0xFF);
    }
    bc_core_zero(destination, 1024);

    bool result = bc_core_copy(destination, source, 1024);

    assert_true(result);
    assert_memory_equal(destination, source, 1024);
}

static void test_move_len_zero_returns_true(void** state)
{
    BC_UNUSED(state);

    unsigned char buffer[4] = {0xAA, 0xBB, 0xCC, 0xDD};

    bool result = bc_core_move(buffer, buffer, 0);

    assert_true(result);
    assert_int_equal(buffer[0], 0xAA);
}

static void test_move_forward_no_overlap(void** state)
{
    BC_UNUSED(state);

    unsigned char source[100];
    unsigned char destination[100];
    for (size_t i = 0; i < 100; i++) {
        source[i] = (unsigned char)(i + 1);
    }
    bc_core_zero(destination, 100);

    bool result = bc_core_move(destination, source, 100);

    assert_true(result);
    assert_memory_equal(destination, source, 100);
}

static void test_move_forward_dst_before_src_no_overlap_large(void** state)
{
    BC_UNUSED(state);

    unsigned char buffer[512];
    unsigned char expected[256];
    for (size_t i = 0; i < 256; i++) {
        buffer[i + 256] = (unsigned char)(i ^ 0xA5);
        expected[i] = (unsigned char)(i ^ 0xA5);
    }

    bool result = bc_core_move(buffer, buffer + 256, 256);

    assert_true(result);
    assert_memory_equal(buffer, expected, 256);
}

static void test_move_backward_overlap_dst_after_src(void** state)
{
    BC_UNUSED(state);

    unsigned char buffer[200];
    for (size_t i = 0; i < 200; i++) {
        buffer[i] = (unsigned char)(i & 0xFF);
    }
    unsigned char expected[100];
    for (size_t i = 0; i < 100; i++) {
        expected[i] = (unsigned char)(i & 0xFF);
    }

    bool result = bc_core_move(buffer + 10, buffer, 100);

    assert_true(result);
    assert_memory_equal(buffer + 10, expected, 100);
}

static void test_move_backward_overlap_large_vector_path(void** state)
{
    BC_UNUSED(state);

    unsigned char buffer[300];
    for (size_t i = 0; i < 300; i++) {
        buffer[i] = (unsigned char)(i & 0xFF);
    }
    unsigned char expected[256];
    for (size_t i = 0; i < 256; i++) {
        expected[i] = (unsigned char)(i & 0xFF);
    }

    bool result = bc_core_move(buffer + 10, buffer, 256);

    assert_true(result);
    assert_memory_equal(buffer + 10, expected, 256);
}

static void test_move_len_1_forward(void** state)
{
    BC_UNUSED(state);

    unsigned char source = 0x99;
    unsigned char destination = 0x00;

    bool result = bc_core_move(&destination, &source, 1);

    assert_true(result);
    assert_int_equal(destination, 0x99);
}

static void test_swap_len_zero_returns_true(void** state)
{
    BC_UNUSED(state);

    unsigned char buffer_a[4] = {0x11, 0x22, 0x33, 0x44};
    unsigned char buffer_b[4] = {0xAA, 0xBB, 0xCC, 0xDD};

    bool result = bc_core_swap(buffer_a, buffer_b, 0);

    assert_true(result);
    assert_int_equal(buffer_a[0], 0x11);
    assert_int_equal(buffer_b[0], 0xAA);
}

static void test_swap_scalar_tail_only(void** state)
{
    BC_UNUSED(state);

    unsigned char buffer_a[15];
    unsigned char buffer_b[15];
    bc_core_fill(buffer_a, 15, (unsigned char)0x11);
    bc_core_fill(buffer_b, 15, (unsigned char)0xFF);

    bool result = bc_core_swap(buffer_a, buffer_b, 15);

    assert_true(result);
    for (size_t i = 0; i < 15; i++) {
        assert_int_equal(buffer_a[i], 0xFF);
        assert_int_equal(buffer_b[i], 0x11);
    }
}

static void test_swap_vector_chunks_only(void** state)
{
    BC_UNUSED(state);

    unsigned char buffer_a[32];
    unsigned char buffer_b[32];
    bc_core_fill(buffer_a, 32, (unsigned char)0x22);
    bc_core_fill(buffer_b, 32, (unsigned char)0xEE);

    bool result = bc_core_swap(buffer_a, buffer_b, 32);

    assert_true(result);
    for (size_t i = 0; i < 32; i++) {
        assert_int_equal(buffer_a[i], 0xEE);
        assert_int_equal(buffer_b[i], 0x22);
    }
}

static void test_swap_vector_chunks_plus_scalar_tail(void** state)
{
    BC_UNUSED(state);

    unsigned char buffer_a[50];
    unsigned char buffer_b[50];
    bc_core_fill(buffer_a, 50, (unsigned char)0x33);
    bc_core_fill(buffer_b, 50, (unsigned char)0xCC);

    bool result = bc_core_swap(buffer_a, buffer_b, 50);

    assert_true(result);
    for (size_t i = 0; i < 50; i++) {
        assert_int_equal(buffer_a[i], 0xCC);
        assert_int_equal(buffer_b[i], 0x33);
    }
}

static void test_swap_len_1_scalar_tail(void** state)
{
    BC_UNUSED(state);

    unsigned char buffer_a[1] = {0x01};
    unsigned char buffer_b[1] = {0x02};

    bool result = bc_core_swap(buffer_a, buffer_b, 1);

    assert_true(result);
    assert_int_equal(buffer_a[0], 0x02);
    assert_int_equal(buffer_b[0], 0x01);
}

static void test_swap_large_buffer(void** state)
{
    BC_UNUSED(state);

    unsigned char buffer_a[1024];
    unsigned char buffer_b[1024];
    for (size_t i = 0; i < 1024; i++) {
        buffer_a[i] = (unsigned char)(i & 0xFF);
        buffer_b[i] = (unsigned char)(255 - (i & 0xFF));
    }

    bool result = bc_core_swap(buffer_a, buffer_b, 1024);

    assert_true(result);
    for (size_t i = 0; i < 1024; i++) {
        assert_int_equal(buffer_a[i], (unsigned char)(255 - (i & 0xFF)));
        assert_int_equal(buffer_b[i], (unsigned char)(i & 0xFF));
    }
}

static void test_move_forward_dst_after_src_plus_len_no_overlap(void** state)
{
    BC_UNUSED(state);

    /*
     * destination >= source + len : dst is placed strictly after src+len.
     * This covers the true branch of: destination < source || destination >= source + len
     * via the second condition (destination >= source + len).
     * src = buffer, len = 64, dst = buffer + 64. No overlap. Forward copy path.
     */
    unsigned char buffer[128];
    unsigned char expected[64];
    for (size_t i = 0; i < 64; i++) {
        buffer[i] = (unsigned char)(i ^ 0x55);
        expected[i] = (unsigned char)(i ^ 0x55);
    }
    bc_core_zero(buffer + 64, 64);

    bool result = bc_core_move(buffer + 64, buffer, 64);

    assert_true(result);
    assert_memory_equal(buffer + 64, expected, 64);
}

static void test_copy_larger_than_l2_activates_nt_store_path(void** state)
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

    bool result = bc_core_copy(destination, source, buffer_size);

    assert_true(result);
    assert_memory_equal(destination, source, buffer_size);

    free(source);
    free(destination);
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_copy_len_zero_returns_true),
        cmocka_unit_test(test_copy_len_1_scalar_path),
        cmocka_unit_test(test_copy_len_15_scalar_path),
        cmocka_unit_test(test_copy_len_31_scalar_path),
        cmocka_unit_test(test_copy_len_32_head_plus_tail_path),
        cmocka_unit_test(test_copy_len_33_head_plus_tail_path),
        cmocka_unit_test(test_copy_len_63_head_plus_mid_plus_tail_no_second_mid),
        cmocka_unit_test(test_copy_len_64_head_plus_mid_plus_tail_no_second_mid),
        cmocka_unit_test(test_copy_len_65_head_plus_mid1_mid2_plus_tail),
        cmocka_unit_test(test_copy_len_127_head_plus_mid1_mid2_plus_tail),
        cmocka_unit_test(test_copy_len_128_head_plus_mid1_mid2_plus_tail),
        cmocka_unit_test(test_copy_len_129_aligned_store_path),
        cmocka_unit_test(test_copy_len_256_aligned_store_path),
        cmocka_unit_test(test_copy_len_1024_aligned_store_path),
        cmocka_unit_test(test_move_len_zero_returns_true),
        cmocka_unit_test(test_move_forward_no_overlap),
        cmocka_unit_test(test_move_forward_dst_before_src_no_overlap_large),
        cmocka_unit_test(test_move_backward_overlap_dst_after_src),
        cmocka_unit_test(test_move_backward_overlap_large_vector_path),
        cmocka_unit_test(test_move_len_1_forward),
        cmocka_unit_test(test_swap_len_zero_returns_true),
        cmocka_unit_test(test_swap_scalar_tail_only),
        cmocka_unit_test(test_swap_vector_chunks_only),
        cmocka_unit_test(test_swap_vector_chunks_plus_scalar_tail),
        cmocka_unit_test(test_swap_len_1_scalar_tail),
        cmocka_unit_test(test_swap_large_buffer),
        cmocka_unit_test(test_move_forward_dst_after_src_plus_len_no_overlap),
        cmocka_unit_test(test_copy_larger_than_l2_activates_nt_store_path),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
