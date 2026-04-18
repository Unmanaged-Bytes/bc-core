// SPDX-License-Identifier: MIT

#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <cmocka.h>
#include "bc_core.h"
#include <stdlib.h>

#define BUFFER_SIZE 8192

static void test_length_empty(void** state)
{
    BC_UNUSED(state);

    unsigned char* data = aligned_alloc(64, BUFFER_SIZE);
    assert_non_null(data);
    bc_core_fill(data, BUFFER_SIZE, (unsigned char)0xAA);
    data[0] = 0x00;

    size_t length = 99;
    bool success = bc_core_length(data, 0x00, &length);

    assert_true(success);
    assert_int_equal(length, 0);

    free(data);
}

static void test_length_single_byte(void** state)
{
    BC_UNUSED(state);

    unsigned char* data = aligned_alloc(64, BUFFER_SIZE);
    assert_non_null(data);
    bc_core_fill(data, BUFFER_SIZE, (unsigned char)0xAA);
    data[1] = 0x00;

    size_t length = 99;
    bool success = bc_core_length(data, 0x00, &length);

    assert_true(success);
    assert_int_equal(length, 1);

    free(data);
}

static void test_length_small(void** state)
{
    BC_UNUSED(state);

    unsigned char* data = aligned_alloc(64, BUFFER_SIZE);
    assert_non_null(data);
    bc_core_fill(data, BUFFER_SIZE, (unsigned char)0xAA);
    data[10] = 0x00;

    size_t length = 99;
    bool success = bc_core_length(data, 0x00, &length);

    assert_true(success);
    assert_int_equal(length, 10);

    free(data);
}

static void test_length_exact_32(void** state)
{
    BC_UNUSED(state);

    unsigned char* data = aligned_alloc(64, BUFFER_SIZE);
    assert_non_null(data);
    bc_core_fill(data, BUFFER_SIZE, (unsigned char)0xAA);
    data[32] = 0x00;

    size_t length = 99;
    bool success = bc_core_length(data, 0x00, &length);

    assert_true(success);
    assert_int_equal(length, 32);

    free(data);
}

static void test_length_exact_64(void** state)
{
    BC_UNUSED(state);

    unsigned char* data = aligned_alloc(64, BUFFER_SIZE);
    assert_non_null(data);
    bc_core_fill(data, BUFFER_SIZE, (unsigned char)0xAA);
    data[64] = 0x00;

    size_t length = 99;
    bool success = bc_core_length(data, 0x00, &length);

    assert_true(success);
    assert_int_equal(length, 64);

    free(data);
}

static void test_length_33(void** state)
{
    BC_UNUSED(state);

    unsigned char* data = aligned_alloc(64, BUFFER_SIZE);
    assert_non_null(data);
    bc_core_fill(data, BUFFER_SIZE, (unsigned char)0xAA);
    data[33] = 0x00;

    size_t length = 99;
    bool success = bc_core_length(data, 0x00, &length);

    assert_true(success);
    assert_int_equal(length, 33);

    free(data);
}

static void test_length_65(void** state)
{
    BC_UNUSED(state);

    unsigned char* data = aligned_alloc(64, BUFFER_SIZE);
    assert_non_null(data);
    bc_core_fill(data, BUFFER_SIZE, (unsigned char)0xAA);
    data[65] = 0x00;

    size_t length = 99;
    bool success = bc_core_length(data, 0x00, &length);

    assert_true(success);
    assert_int_equal(length, 65);

    free(data);
}

static void test_length_255(void** state)
{
    BC_UNUSED(state);

    unsigned char* data = aligned_alloc(64, BUFFER_SIZE);
    assert_non_null(data);
    bc_core_fill(data, BUFFER_SIZE, (unsigned char)0xAA);
    data[255] = 0x00;

    size_t length = 99;
    bool success = bc_core_length(data, 0x00, &length);

    assert_true(success);
    assert_int_equal(length, 255);

    free(data);
}

static void test_length_4095(void** state)
{
    BC_UNUSED(state);

    unsigned char* data = aligned_alloc(64, BUFFER_SIZE);
    assert_non_null(data);
    bc_core_fill(data, BUFFER_SIZE, (unsigned char)0xAA);
    data[4095] = 0x00;

    size_t length = 99;
    bool success = bc_core_length(data, 0x00, &length);

    assert_true(success);
    assert_int_equal(length, 4095);

    free(data);
}

static void test_length_large_4096(void** state)
{
    BC_UNUSED(state);

    unsigned char* data = aligned_alloc(64, BUFFER_SIZE);
    assert_non_null(data);
    bc_core_fill(data, BUFFER_SIZE, (unsigned char)0xAA);
    data[4096] = 0x00;

    size_t length = 99;
    bool success = bc_core_length(data, 0x00, &length);

    assert_true(success);
    assert_int_equal(length, 4096);

    free(data);
}

static void test_length_custom_terminator(void** state)
{
    BC_UNUSED(state);

    unsigned char* data = aligned_alloc(64, BUFFER_SIZE);
    assert_non_null(data);
    bc_core_fill(data, BUFFER_SIZE, (unsigned char)0xAA);
    data[42] = 0xFF;

    size_t length = 99;
    bool success = bc_core_length(data, 0xFF, &length);

    assert_true(success);
    assert_int_equal(length, 42);

    free(data);
}

static void test_length_newline_terminator(void** state)
{
    BC_UNUSED(state);

    unsigned char* data = aligned_alloc(64, BUFFER_SIZE);
    assert_non_null(data);
    bc_core_fill(data, BUFFER_SIZE, (unsigned char)0xAA);
    data[20] = '\n';

    size_t length = 99;
    bool success = bc_core_length(data, '\n', &length);

    assert_true(success);
    assert_int_equal(length, 20);

    free(data);
}

static void test_length_unaligned_1(void** state)
{
    BC_UNUSED(state);

    unsigned char* data = aligned_alloc(64, BUFFER_SIZE);
    assert_non_null(data);
    bc_core_fill(data, BUFFER_SIZE, (unsigned char)0xAA);
    data[1 + 50] = 0x00;

    size_t length = 99;
    bool success = bc_core_length(data + 1, 0x00, &length);

    assert_true(success);
    assert_int_equal(length, 50);

    free(data);
}

static void test_length_unaligned_7(void** state)
{
    BC_UNUSED(state);

    unsigned char* data = aligned_alloc(64, BUFFER_SIZE);
    assert_non_null(data);
    bc_core_fill(data, BUFFER_SIZE, (unsigned char)0xAA);
    data[7 + 100] = 0x00;

    size_t length = 99;
    bool success = bc_core_length(data + 7, 0x00, &length);

    assert_true(success);
    assert_int_equal(length, 100);

    free(data);
}

static void test_length_unaligned_31(void** state)
{
    BC_UNUSED(state);

    unsigned char* data = aligned_alloc(64, BUFFER_SIZE);
    assert_non_null(data);
    bc_core_fill(data, BUFFER_SIZE, (unsigned char)0xAA);
    data[31 + 200] = 0x00;

    size_t length = 99;
    bool success = bc_core_length(data + 31, 0x00, &length);

    assert_true(success);
    assert_int_equal(length, 200);

    free(data);
}

static void test_length_160(void** state)
{
    BC_UNUSED(state);

    unsigned char* data = aligned_alloc(64, BUFFER_SIZE);
    assert_non_null(data);
    bc_core_fill(data, BUFFER_SIZE, (unsigned char)0xAA);
    data[160] = 0x00;

    size_t length = 99;
    bool success = bc_core_length(data, 0x00, &length);

    assert_true(success);
    assert_int_equal(length, 160);

    free(data);
}

static void test_length_224(void** state)
{
    BC_UNUSED(state);

    unsigned char* data = aligned_alloc(64, BUFFER_SIZE);
    assert_non_null(data);
    bc_core_fill(data, BUFFER_SIZE, (unsigned char)0xAA);
    data[224] = 0x00;

    size_t length = 99;
    bool success = bc_core_length(data, 0x00, &length);

    assert_true(success);
    assert_int_equal(length, 224);

    free(data);
}

static void test_length_all_same_except_last(void** state)
{
    BC_UNUSED(state);

    unsigned char* data = aligned_alloc(64, BUFFER_SIZE);
    assert_non_null(data);
    bc_core_fill(data, BUFFER_SIZE, (unsigned char)0xAA);
    data[512] = 0xBB;

    size_t length = 99;
    bool success = bc_core_length(data, 0xBB, &length);

    assert_true(success);
    assert_int_equal(length, 512);

    free(data);
}

static void test_length_aligned_128_terminator_in_while_loop(void** state)
{
    BC_UNUSED(state);

    unsigned char* data = aligned_alloc(128, BUFFER_SIZE);
    assert_non_null(data);
    bc_core_fill(data, BUFFER_SIZE, (unsigned char)0xAA);
    data[50] = 0x00;

    size_t length = 99;
    bool success = bc_core_length(data, 0x00, &length);

    assert_true(success);
    assert_int_equal(length, 50);

    free(data);
}

static void test_length_aligned_128_terminator_in_unroll(void** state)
{
    BC_UNUSED(state);

    unsigned char* data = aligned_alloc(128, BUFFER_SIZE);
    assert_non_null(data);
    bc_core_fill(data, BUFFER_SIZE, (unsigned char)0xAA);
    data[300] = 0x00;

    size_t length = 99;
    bool success = bc_core_length(data, 0x00, &length);

    assert_true(success);
    assert_int_equal(length, 300);

    free(data);
}

static void test_length_unaligned_3_short(void** state)
{
    BC_UNUSED(state);

    unsigned char* data = aligned_alloc(128, BUFFER_SIZE);
    assert_non_null(data);
    bc_core_fill(data, BUFFER_SIZE, (unsigned char)0xAA);
    data[3 + 15] = 0x00;

    size_t length = 99;
    bool success = bc_core_length(data + 3, 0x00, &length);

    assert_true(success);
    assert_int_equal(length, 15);

    free(data);
}

static void test_length_unaligned_3_long(void** state)
{
    BC_UNUSED(state);

    unsigned char* data = aligned_alloc(128, BUFFER_SIZE);
    assert_non_null(data);
    bc_core_fill(data, BUFFER_SIZE, (unsigned char)0xAA);
    data[3 + 500] = 0x00;

    size_t length = 99;
    bool success = bc_core_length(data + 3, 0x00, &length);

    assert_true(success);
    assert_int_equal(length, 500);

    free(data);
}

static void test_length_unaligned_17_in_while_loop(void** state)
{
    BC_UNUSED(state);

    unsigned char* data = aligned_alloc(128, BUFFER_SIZE);
    assert_non_null(data);
    bc_core_fill(data, BUFFER_SIZE, (unsigned char)0xAA);
    data[17 + 60] = 0x00;

    size_t length = 99;
    bool success = bc_core_length(data + 17, 0x00, &length);

    assert_true(success);
    assert_int_equal(length, 60);

    free(data);
}

static void test_length_unaligned_17_in_unroll(void** state)
{
    BC_UNUSED(state);

    unsigned char* data = aligned_alloc(128, BUFFER_SIZE);
    assert_non_null(data);
    bc_core_fill(data, BUFFER_SIZE, (unsigned char)0xAA);
    data[17 + 400] = 0x00;

    size_t length = 99;
    bool success = bc_core_length(data + 17, 0x00, &length);

    assert_true(success);
    assert_int_equal(length, 400);

    free(data);
}

static void test_length_unaligned_63(void** state)
{
    BC_UNUSED(state);

    unsigned char* data = aligned_alloc(128, BUFFER_SIZE);
    assert_non_null(data);
    bc_core_fill(data, BUFFER_SIZE, (unsigned char)0xAA);
    data[63 + 250] = 0x00;

    size_t length = 99;
    bool success = bc_core_length(data + 63, 0x00, &length);

    assert_true(success);
    assert_int_equal(length, 250);

    free(data);
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_length_empty),
        cmocka_unit_test(test_length_single_byte),
        cmocka_unit_test(test_length_small),
        cmocka_unit_test(test_length_exact_32),
        cmocka_unit_test(test_length_exact_64),
        cmocka_unit_test(test_length_33),
        cmocka_unit_test(test_length_65),
        cmocka_unit_test(test_length_255),
        cmocka_unit_test(test_length_4095),
        cmocka_unit_test(test_length_large_4096),
        cmocka_unit_test(test_length_custom_terminator),
        cmocka_unit_test(test_length_newline_terminator),
        cmocka_unit_test(test_length_unaligned_1),
        cmocka_unit_test(test_length_unaligned_7),
        cmocka_unit_test(test_length_unaligned_31),
        cmocka_unit_test(test_length_160),
        cmocka_unit_test(test_length_224),
        cmocka_unit_test(test_length_all_same_except_last),
        cmocka_unit_test(test_length_aligned_128_terminator_in_while_loop),
        cmocka_unit_test(test_length_aligned_128_terminator_in_unroll),
        cmocka_unit_test(test_length_unaligned_3_short),
        cmocka_unit_test(test_length_unaligned_3_long),
        cmocka_unit_test(test_length_unaligned_17_in_while_loop),
        cmocka_unit_test(test_length_unaligned_17_in_unroll),
        cmocka_unit_test(test_length_unaligned_63),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
