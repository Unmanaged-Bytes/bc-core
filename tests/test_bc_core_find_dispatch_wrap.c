// SPDX-License-Identifier: MIT

#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <cmocka.h>

#include "bc_core.h"
#include "test_bc_core_dispatch_mock.h"

#include <stdlib.h>

extern void bc_core_find_dispatch_init(void);

/* ===== Helpers ===== */

static void setup_avx2(void)
{
    g_mock_detect_returns = true;
    bc_core_zero(&g_mock_features, sizeof(g_mock_features));
    g_mock_features.has_avx2 = true;
    bc_core_find_dispatch_init();
}

/* ===== find_byte — avx2 ===== */

static void test_find_byte_avx2_not_found(void** state)
{
    BC_UNUSED(state);
    setup_avx2();

    unsigned char* buf = aligned_alloc(64, 256);
    assert_non_null(buf);
    bc_core_fill(buf, 256, (unsigned char)0xAA);
    size_t offset = 0;
    bool found = bc_core_find_byte(buf, 256, 0xBB, &offset);
    assert_false(found);
    free(buf);
}

static void test_find_byte_avx2_at_0(void** state)
{
    BC_UNUSED(state);
    setup_avx2();

    unsigned char* buf = aligned_alloc(64, 256);
    assert_non_null(buf);
    bc_core_fill(buf, 256, (unsigned char)0xAA);
    buf[0] = 0xBB;
    size_t offset = 0;
    bool found = bc_core_find_byte(buf, 256, 0xBB, &offset);
    assert_true(found);
    assert_int_equal(offset, 0);
    free(buf);
}

static void test_find_byte_avx2_at_63(void** state)
{
    BC_UNUSED(state);
    setup_avx2();

    unsigned char* buf = aligned_alloc(64, 256);
    assert_non_null(buf);
    bc_core_fill(buf, 256, (unsigned char)0xAA);
    buf[63] = 0xBB;
    size_t offset = 0;
    bool found = bc_core_find_byte(buf, 256, 0xBB, &offset);
    assert_true(found);
    assert_int_equal(offset, 63);
    free(buf);
}

static void test_find_byte_avx2_at_64(void** state)
{
    BC_UNUSED(state);
    setup_avx2();

    unsigned char* buf = aligned_alloc(64, 256);
    assert_non_null(buf);
    bc_core_fill(buf, 256, (unsigned char)0xAA);
    buf[64] = 0xBB;
    size_t offset = 0;
    bool found = bc_core_find_byte(buf, 256, 0xBB, &offset);
    assert_true(found);
    assert_int_equal(offset, 64);
    free(buf);
}

static void test_find_byte_avx2_at_255(void** state)
{
    BC_UNUSED(state);
    setup_avx2();

    unsigned char* buf = aligned_alloc(64, 256);
    assert_non_null(buf);
    bc_core_fill(buf, 256, (unsigned char)0xAA);
    buf[255] = 0xBB;
    size_t offset = 0;
    bool found = bc_core_find_byte(buf, 256, 0xBB, &offset);
    assert_true(found);
    assert_int_equal(offset, 255);
    free(buf);
}

static void test_find_byte_avx2_len_0(void** state)
{
    BC_UNUSED(state);
    setup_avx2();

    unsigned char* buf = aligned_alloc(64, 64);
    assert_non_null(buf);
    bc_core_fill(buf, 64, (unsigned char)0xBB);
    size_t offset = 0;
    bool found = bc_core_find_byte(buf, 0, 0xBB, &offset);
    assert_false(found);
    free(buf);
}

static void test_find_byte_avx2_len_1_found(void** state)
{
    BC_UNUSED(state);
    setup_avx2();

    unsigned char* buf = aligned_alloc(64, 64);
    assert_non_null(buf);
    bc_core_fill(buf, 64, (unsigned char)0xAA);
    buf[0] = 0xBB;
    size_t offset = 0;
    bool found = bc_core_find_byte(buf, 1, 0xBB, &offset);
    assert_true(found);
    assert_int_equal(offset, 0);
    free(buf);
}

/* ===== find_pattern — avx2 ===== */

static void test_find_pattern_avx2_not_found(void** state)
{
    BC_UNUSED(state);
    setup_avx2();

    unsigned char* buf = aligned_alloc(64, 256);
    assert_non_null(buf);
    bc_core_fill(buf, 256, (unsigned char)'X');
    const char* needle = "ABCDEF";
    size_t offset = 0;
    bool found = bc_core_find_pattern(buf, 256, needle, 6, &offset);
    assert_false(found);
    free(buf);
}

static void test_find_pattern_avx2_at_start(void** state)
{
    BC_UNUSED(state);
    setup_avx2();

    unsigned char* buf = aligned_alloc(64, 256);
    assert_non_null(buf);
    bc_core_fill(buf, 256, (unsigned char)'X');
    bc_core_copy(buf, "ABCDEF", 6);
    size_t offset = 0;
    bool found = bc_core_find_pattern(buf, 256, "ABCDEF", 6, &offset);
    assert_true(found);
    assert_int_equal(offset, 0);
    free(buf);
}

static void test_find_pattern_avx2_at_63(void** state)
{
    BC_UNUSED(state);
    setup_avx2();

    unsigned char* buf = aligned_alloc(64, 256);
    assert_non_null(buf);
    bc_core_fill(buf, 256, (unsigned char)'X');
    bc_core_copy(buf + 63, "ABCDEF", 6);
    size_t offset = 0;
    bool found = bc_core_find_pattern(buf, 256, "ABCDEF", 6, &offset);
    assert_true(found);
    assert_int_equal(offset, 63);
    free(buf);
}

static void test_find_pattern_avx2_at_64(void** state)
{
    BC_UNUSED(state);
    setup_avx2();

    unsigned char* buf = aligned_alloc(64, 256);
    assert_non_null(buf);
    bc_core_fill(buf, 256, (unsigned char)'X');
    bc_core_copy(buf + 64, "ABCDEF", 6);
    size_t offset = 0;
    bool found = bc_core_find_pattern(buf, 256, "ABCDEF", 6, &offset);
    assert_true(found);
    assert_int_equal(offset, 64);
    free(buf);
}

static void test_find_pattern_avx2_false_first_last(void** state)
{
    BC_UNUSED(state);
    setup_avx2();

    unsigned char* buf = aligned_alloc(64, 256);
    assert_non_null(buf);
    bc_core_fill(buf, 256, (unsigned char)'X');
    buf[10] = 'A';
    buf[15] = 'F';
    size_t offset = 0;
    bool found = bc_core_find_pattern(buf, 256, "ABCDEF", 6, &offset);
    assert_false(found);
    free(buf);
}

static void test_find_pattern_avx2_len_0(void** state)
{
    BC_UNUSED(state);
    setup_avx2();

    unsigned char* buf = aligned_alloc(64, 256);
    assert_non_null(buf);
    bc_core_fill(buf, 256, (unsigned char)'X');
    bc_core_copy(buf, "ABCDEF", 6);
    size_t offset = 0;
    bool found = bc_core_find_pattern(buf, 0, "ABCDEF", 6, &offset);
    assert_false(found);
    free(buf);
}

/* ===== find_last_byte — avx2 ===== */

static void test_find_last_byte_avx2_not_found(void** state)
{
    BC_UNUSED(state);
    setup_avx2();

    unsigned char* buf = aligned_alloc(64, 256);
    assert_non_null(buf);
    bc_core_fill(buf, 256, (unsigned char)0xAA);
    size_t offset = 0;
    bool found = bc_core_find_last_byte(buf, 256, 0xBB, &offset);
    assert_false(found);
    free(buf);
}

static void test_find_last_byte_avx2_single_at_0(void** state)
{
    BC_UNUSED(state);
    setup_avx2();

    unsigned char* buf = aligned_alloc(64, 256);
    assert_non_null(buf);
    bc_core_fill(buf, 256, (unsigned char)0xAA);
    buf[0] = 0xBB;
    size_t offset = 0;
    bool found = bc_core_find_last_byte(buf, 256, 0xBB, &offset);
    assert_true(found);
    assert_int_equal(offset, 0);
    free(buf);
}

static void test_find_last_byte_avx2_multiple(void** state)
{
    BC_UNUSED(state);
    setup_avx2();

    unsigned char* buf = aligned_alloc(64, 256);
    assert_non_null(buf);
    bc_core_fill(buf, 256, (unsigned char)0xAA);
    buf[10] = 0xBB;
    buf[63] = 0xBB;
    buf[64] = 0xBB;
    buf[200] = 0xBB;
    size_t offset = 0;
    bool found = bc_core_find_last_byte(buf, 256, 0xBB, &offset);
    assert_true(found);
    assert_int_equal(offset, 200);
    free(buf);
}

static void test_find_last_byte_avx2_at_end(void** state)
{
    BC_UNUSED(state);
    setup_avx2();

    unsigned char* buf = aligned_alloc(64, 256);
    assert_non_null(buf);
    bc_core_fill(buf, 256, (unsigned char)0xAA);
    buf[255] = 0xBB;
    size_t offset = 0;
    bool found = bc_core_find_last_byte(buf, 256, 0xBB, &offset);
    assert_true(found);
    assert_int_equal(offset, 255);
    free(buf);
}

/* ===== find_any_byte — avx2 ===== */

static void test_find_any_byte_avx2_not_found(void** state)
{
    BC_UNUSED(state);
    setup_avx2();

    unsigned char* buf = aligned_alloc(64, 256);
    assert_non_null(buf);
    bc_core_fill(buf, 256, (unsigned char)0x00);
    const unsigned char targets[] = {0x41, 0x42, 0x43};
    size_t offset = 0;
    bool found = bc_core_find_any_byte(buf, 256, targets, 3, &offset);
    assert_false(found);
    free(buf);
}

static void test_find_any_byte_avx2_first_target(void** state)
{
    BC_UNUSED(state);
    setup_avx2();

    unsigned char* buf = aligned_alloc(64, 256);
    assert_non_null(buf);
    bc_core_fill(buf, 256, (unsigned char)0x00);
    buf[50] = 0x41;
    const unsigned char targets[] = {0x41, 0x42, 0x43};
    size_t offset = 0;
    bool found = bc_core_find_any_byte(buf, 256, targets, 3, &offset);
    assert_true(found);
    assert_int_equal(offset, 50);
    free(buf);
}

static void test_find_any_byte_avx2_second_target(void** state)
{
    BC_UNUSED(state);
    setup_avx2();

    unsigned char* buf = aligned_alloc(64, 256);
    assert_non_null(buf);
    bc_core_fill(buf, 256, (unsigned char)0x00);
    buf[20] = 0x42;
    buf[50] = 0x41;
    const unsigned char targets[] = {0x41, 0x42, 0x43};
    size_t offset = 0;
    bool found = bc_core_find_any_byte(buf, 256, targets, 3, &offset);
    assert_true(found);
    assert_int_equal(offset, 20);
    free(buf);
}

static void test_find_any_byte_avx2_at_boundary(void** state)
{
    BC_UNUSED(state);
    setup_avx2();

    unsigned char* buf = aligned_alloc(64, 256);
    assert_non_null(buf);
    bc_core_fill(buf, 256, (unsigned char)0x00);
    buf[64] = 0x43;
    const unsigned char targets[] = {0x41, 0x42, 0x43};
    size_t offset = 0;
    bool found = bc_core_find_any_byte(buf, 256, targets, 3, &offset);
    assert_true(found);
    assert_int_equal(offset, 64);
    free(buf);
}

static void test_find_byte_avx2_at_320(void** state)
{
    BC_UNUSED(state);
    setup_avx2();

    unsigned char* buf = aligned_alloc(64, 512);
    assert_non_null(buf);
    bc_core_fill(buf, 512, (unsigned char)0xAA);
    buf[320] = 0xBB;
    size_t offset = 0;
    bool found = bc_core_find_byte(buf, 512, 0xBB, &offset);
    assert_true(found);
    assert_int_equal(offset, 320);
    free(buf);
}

static void test_find_byte_avx2_at_352(void** state)
{
    BC_UNUSED(state);
    setup_avx2();

    unsigned char* buf = aligned_alloc(64, 512);
    assert_non_null(buf);
    bc_core_fill(buf, 512, (unsigned char)0xAA);
    buf[352] = 0xBB;
    size_t offset = 0;
    bool found = bc_core_find_byte(buf, 512, 0xBB, &offset);
    assert_true(found);
    assert_int_equal(offset, 352);
    free(buf);
}

static void test_find_byte_avx2_tail_byte(void** state)
{
    BC_UNUSED(state);
    setup_avx2();

    unsigned char* buf = aligned_alloc(64, 192);
    assert_non_null(buf);
    bc_core_fill(buf, 192, (unsigned char)0xAA);
    buf[129] = 0xBB;
    size_t offset = 0;
    bool found = bc_core_find_byte(buf, 131, 0xBB, &offset);
    assert_true(found);
    assert_int_equal(offset, 129);
    free(buf);
}

static void test_find_byte_avx2_512_not_found(void** state)
{
    BC_UNUSED(state);
    setup_avx2();

    unsigned char* buf = aligned_alloc(64, 512);
    assert_non_null(buf);
    bc_core_fill(buf, 512, (unsigned char)0xAA);
    size_t offset = 0;
    bool found = bc_core_find_byte(buf, 512, 0xBB, &offset);
    assert_false(found);
    free(buf);
}

/* ===== find_pattern — large buffers ===== */

static void test_find_pattern_avx2_at_400(void** state)
{
    BC_UNUSED(state);
    setup_avx2();

    unsigned char* buf = aligned_alloc(64, 512);
    assert_non_null(buf);
    bc_core_fill(buf, 512, (unsigned char)'X');
    bc_core_copy(buf + 400, "ABCDEF", 6);
    size_t offset = 0;
    bool found = bc_core_find_pattern(buf, 512, "ABCDEF", 6, &offset);
    assert_true(found);
    assert_int_equal(offset, 400);
    free(buf);
}

/* ===== find_last_byte — large buffers ===== */

static void test_find_last_byte_avx2_at_10_in_512(void** state)
{
    BC_UNUSED(state);
    setup_avx2();

    unsigned char* buf = aligned_alloc(64, 512);
    assert_non_null(buf);
    bc_core_fill(buf, 512, (unsigned char)0xAA);
    buf[10] = 0xBB;
    size_t offset = 0;
    bool found = bc_core_find_last_byte(buf, 512, 0xBB, &offset);
    assert_true(found);
    assert_int_equal(offset, 10);
    free(buf);
}

/* ===== find_any_byte — large buffers ===== */

static void test_find_any_byte_avx2_at_300(void** state)
{
    BC_UNUSED(state);
    setup_avx2();

    unsigned char* buf = aligned_alloc(64, 512);
    assert_non_null(buf);
    bc_core_fill(buf, 512, (unsigned char)0x00);
    buf[300] = 0x41;
    const unsigned char targets[] = {0x41, 0x42, 0x43};
    size_t offset = 0;
    bool found = bc_core_find_any_byte(buf, 512, targets, 3, &offset);
    assert_true(found);
    assert_int_equal(offset, 300);
    free(buf);
}

/* ===== find_byte — avx2 32B loop ===== */

static void test_find_byte_avx2_32b_loop_match_at_140(void** state)
{
    BC_UNUSED(state);
    setup_avx2();

    unsigned char* buf = aligned_alloc(64, 192);
    assert_non_null(buf);
    bc_core_fill(buf, 192, (unsigned char)0xAA);
    buf[140] = 0xBB;
    size_t offset = 0;
    bool found = bc_core_find_byte(buf, 160, 0xBB, &offset);
    assert_true(found);
    assert_int_equal(offset, 140);
    free(buf);
}

static void test_find_byte_avx2_32b_loop_no_match(void** state)
{
    BC_UNUSED(state);
    setup_avx2();

    unsigned char* buf = aligned_alloc(64, 192);
    assert_non_null(buf);
    bc_core_fill(buf, 192, (unsigned char)0xAA);
    size_t offset = 0;
    bool found = bc_core_find_byte(buf, 160, 0xBB, &offset);
    assert_false(found);
    free(buf);
}

/* ===== find_pattern — avx2 len_1 and len > data ===== */

static void test_find_pattern_avx2_len_1(void** state)
{
    BC_UNUSED(state);
    setup_avx2();

    unsigned char* buf = aligned_alloc(64, 64);
    assert_non_null(buf);
    bc_core_fill(buf, 64, (unsigned char)0x00);
    buf[20] = 0x5A;
    const unsigned char pattern[1] = {0x5A};
    size_t offset = 0;
    bool found = bc_core_find_pattern(buf, 64, pattern, 1, &offset);
    assert_true(found);
    assert_int_equal(offset, 20);
    free(buf);
}

static void test_find_pattern_avx2_longer_than_data(void** state)
{
    BC_UNUSED(state);
    setup_avx2();

    unsigned char* buf = aligned_alloc(64, 64);
    assert_non_null(buf);
    bc_core_fill(buf, 64, (unsigned char)0xAA);
    unsigned char pattern[10];
    bc_core_fill(pattern, 10, (unsigned char)0xAA);
    size_t offset = 0;
    bool found = bc_core_find_pattern(buf, 5, pattern, 10, &offset);
    assert_false(found);
    free(buf);
}

/* ===== find_pattern — avx2 false positives in m1/m2/m3 ===== */

static void test_find_pattern_avx2_false_in_m1_match_in_m2(void** state)
{
    BC_UNUSED(state);
    setup_avx2();

    unsigned char* buf = aligned_alloc(64, 256);
    assert_non_null(buf);
    bc_core_fill(buf, 256, (unsigned char)0x01);
    buf[40] = 0xAA;
    buf[41] = 0xFF;
    buf[42] = 0xCC;
    buf[70] = 0xAA;
    buf[71] = 0xBB;
    buf[72] = 0xCC;
    size_t offset = 0;
    bool found = bc_core_find_pattern(buf, 256, "\xAA\xBB\xCC", 3, &offset);
    assert_true(found);
    assert_int_equal(offset, 70);
    free(buf);
}

static void test_find_pattern_avx2_false_in_m2_match_in_m3(void** state)
{
    BC_UNUSED(state);
    setup_avx2();

    unsigned char* buf = aligned_alloc(64, 256);
    assert_non_null(buf);
    bc_core_fill(buf, 256, (unsigned char)0x01);
    buf[70] = 0xAA;
    buf[71] = 0xFF;
    buf[72] = 0xCC;
    buf[100] = 0xAA;
    buf[101] = 0xBB;
    buf[102] = 0xCC;
    size_t offset = 0;
    bool found = bc_core_find_pattern(buf, 256, "\xAA\xBB\xCC", 3, &offset);
    assert_true(found);
    assert_int_equal(offset, 100);
    free(buf);
}

static void test_find_pattern_avx2_false_in_m3_match_next_block(void** state)
{
    BC_UNUSED(state);
    setup_avx2();

    unsigned char* buf = aligned_alloc(64, 512);
    assert_non_null(buf);
    bc_core_fill(buf, 512, (unsigned char)0x01);
    buf[100] = 0xAA;
    buf[101] = 0xFF;
    buf[102] = 0xCC;
    buf[150] = 0xAA;
    buf[151] = 0xBB;
    buf[152] = 0xCC;
    size_t offset = 0;
    bool found = bc_core_find_pattern(buf, 512, "\xAA\xBB\xCC", 3, &offset);
    assert_true(found);
    assert_int_equal(offset, 150);
    free(buf);
}

static void test_find_pattern_avx2_32b_false_positive(void** state)
{
    BC_UNUSED(state);
    setup_avx2();

    unsigned char* buf = aligned_alloc(64, 64);
    assert_non_null(buf);
    bc_core_fill(buf, 64, (unsigned char)0x01);
    buf[5] = 0xAA;
    buf[6] = 0xFF;
    buf[7] = 0xCC;
    buf[42] = 0xAA;
    buf[43] = 0xBB;
    buf[44] = 0xCC;
    size_t offset = 0;
    bool found = bc_core_find_pattern(buf, 45, "\xAA\xBB\xCC", 3, &offset);
    assert_true(found);
    assert_int_equal(offset, 42);
    free(buf);
}

/* ===== find_last_byte — avx2 no tail (exact multiple of 32) not found ===== */

static void test_find_last_byte_avx2_exact_multiple_not_found(void** state)
{
    BC_UNUSED(state);
    setup_avx2();

    unsigned char* buf = aligned_alloc(64, 64);
    assert_non_null(buf);
    bc_core_fill(buf, 64, (unsigned char)0xAA);
    size_t offset = 0;
    bool found = bc_core_find_last_byte(buf, 64, 0xBB, &offset);
    assert_false(found);
    free(buf);
}

static void test_find_last_byte_avx2_tail_not_found(void** state)
{
    BC_UNUSED(state);
    setup_avx2();

    unsigned char* buf = aligned_alloc(64, 128);
    assert_non_null(buf);
    bc_core_fill(buf, 128, (unsigned char)0xAA);
    buf[10] = 0xBB;
    size_t offset = 0;
    bool found = bc_core_find_last_byte(buf, 35, 0xBB, &offset);
    assert_true(found);
    assert_int_equal(offset, 10);
    free(buf);
}

/* ===== find_any_byte — avx2 scalar tail ===== */

static void test_find_any_byte_avx2_scalar_tail_found(void** state)
{
    BC_UNUSED(state);
    setup_avx2();

    unsigned char* buf = aligned_alloc(64, 64);
    assert_non_null(buf);
    bc_core_fill(buf, 64, (unsigned char)0x00);
    buf[33] = 0x42;
    const unsigned char targets[] = {0x42};
    size_t offset = 0;
    bool found = bc_core_find_any_byte(buf, 35, targets, 1, &offset);
    assert_true(found);
    assert_int_equal(offset, 33);
    free(buf);
}

static void test_find_any_byte_avx2_scalar_not_found(void** state)
{
    BC_UNUSED(state);
    setup_avx2();

    unsigned char* buf = aligned_alloc(64, 64);
    assert_non_null(buf);
    bc_core_fill(buf, 64, (unsigned char)0x00);
    const unsigned char targets[] = {0x42};
    size_t offset = 0;
    bool found = bc_core_find_any_byte(buf, 35, targets, 1, &offset);
    assert_false(found);
    free(buf);
}

/* ===== find_last_byte — v1 branch ===== */

static void test_find_last_byte_avx2_only_in_v1_block(void** state)
{
    BC_UNUSED(state);

    g_mock_detect_returns = true;
    bc_core_zero(&g_mock_features, sizeof(g_mock_features));
    bc_core_find_dispatch_init();

    unsigned char data[256];
    bc_core_fill(data, sizeof(data), (unsigned char)0x00);
    data[160] = 0xAA;
    size_t offset = 99;

    bool found = bc_core_find_last_byte(data, 256, 0xAA, &offset);

    assert_true(found);
    assert_int_equal(offset, 160);
}

/* ===== dispatch fallback (detect returns false) ===== */

static void test_find_dispatch_detect_failure_avx2_fallback(void** state)
{
    BC_UNUSED(state);

    g_mock_detect_returns = false;
    bc_core_find_dispatch_init();

    unsigned char* buf = aligned_alloc(64, 64);
    assert_non_null(buf);
    bc_core_fill(buf, 64, (unsigned char)0x00);
    buf[5] = 0xFF;
    size_t offset = 0;
    bool found = bc_core_find_byte(buf, 64, 0xFF, &offset);
    assert_true(found);
    assert_int_equal(offset, 5);
    free(buf);
}

int main(void)
{
    const struct CMUnitTest tests[] = {cmocka_unit_test(test_find_byte_avx2_not_found),
                                       cmocka_unit_test(test_find_byte_avx2_at_0),
                                       cmocka_unit_test(test_find_byte_avx2_at_63),
                                       cmocka_unit_test(test_find_byte_avx2_at_64),
                                       cmocka_unit_test(test_find_byte_avx2_at_255),
                                       cmocka_unit_test(test_find_byte_avx2_len_0),
                                       cmocka_unit_test(test_find_byte_avx2_len_1_found),
                                       cmocka_unit_test(test_find_byte_avx2_at_320),
                                       cmocka_unit_test(test_find_byte_avx2_at_352),
                                       cmocka_unit_test(test_find_byte_avx2_tail_byte),
                                       cmocka_unit_test(test_find_byte_avx2_512_not_found),
                                       cmocka_unit_test(test_find_pattern_avx2_not_found),
                                       cmocka_unit_test(test_find_pattern_avx2_at_start),
                                       cmocka_unit_test(test_find_pattern_avx2_at_63),
                                       cmocka_unit_test(test_find_pattern_avx2_at_64),
                                       cmocka_unit_test(test_find_pattern_avx2_false_first_last),
                                       cmocka_unit_test(test_find_pattern_avx2_len_0),
                                       cmocka_unit_test(test_find_pattern_avx2_at_400),
                                       cmocka_unit_test(test_find_last_byte_avx2_not_found),
                                       cmocka_unit_test(test_find_last_byte_avx2_single_at_0),
                                       cmocka_unit_test(test_find_last_byte_avx2_multiple),
                                       cmocka_unit_test(test_find_last_byte_avx2_at_end),
                                       cmocka_unit_test(test_find_last_byte_avx2_at_10_in_512),
                                       cmocka_unit_test(test_find_any_byte_avx2_not_found),
                                       cmocka_unit_test(test_find_any_byte_avx2_first_target),
                                       cmocka_unit_test(test_find_any_byte_avx2_second_target),
                                       cmocka_unit_test(test_find_any_byte_avx2_at_boundary),
                                       cmocka_unit_test(test_find_any_byte_avx2_at_300),
                                       cmocka_unit_test(test_find_byte_avx2_32b_loop_match_at_140),
                                       cmocka_unit_test(test_find_byte_avx2_32b_loop_no_match),
                                       cmocka_unit_test(test_find_pattern_avx2_len_1),
                                       cmocka_unit_test(test_find_pattern_avx2_longer_than_data),
                                       cmocka_unit_test(test_find_pattern_avx2_false_in_m1_match_in_m2),
                                       cmocka_unit_test(test_find_pattern_avx2_false_in_m2_match_in_m3),
                                       cmocka_unit_test(test_find_pattern_avx2_false_in_m3_match_next_block),
                                       cmocka_unit_test(test_find_pattern_avx2_32b_false_positive),
                                       cmocka_unit_test(test_find_last_byte_avx2_exact_multiple_not_found),
                                       cmocka_unit_test(test_find_last_byte_avx2_tail_not_found),
                                       cmocka_unit_test(test_find_any_byte_avx2_scalar_tail_found),
                                       cmocka_unit_test(test_find_any_byte_avx2_scalar_not_found),
                                       cmocka_unit_test(test_find_last_byte_avx2_only_in_v1_block),
                                       cmocka_unit_test(test_find_dispatch_detect_failure_avx2_fallback)};

    return cmocka_run_group_tests(tests, NULL, NULL);
}
