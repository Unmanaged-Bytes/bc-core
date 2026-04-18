// SPDX-License-Identifier: MIT

#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <cmocka.h>

#include "bc_core.h"
#include "test_bc_core_dispatch_mock.h"

#include <stdlib.h>

extern void bc_core_move_dispatch_init(void);

static void test_move_avx2_path_len_0(void** state)
{
    BC_UNUSED(state);

    g_mock_detect_returns = true;
    bc_core_zero(&g_mock_features, sizeof(g_mock_features));

    bc_core_move_dispatch_init();

    unsigned char* src = aligned_alloc(64, 64);
    unsigned char* dst = aligned_alloc(64, 64);
    assert_non_null(src);
    assert_non_null(dst);
    bc_core_fill(src, 64, (unsigned char)0x5A);
    bc_core_fill(dst, 64, (unsigned char)0x00);

    bool result = bc_core_move(dst, src, 0);

    assert_true(result);

    free(src);
    free(dst);
}

static void test_move_avx2_path_small(void** state)
{
    BC_UNUSED(state);

    g_mock_detect_returns = true;
    bc_core_zero(&g_mock_features, sizeof(g_mock_features));

    bc_core_move_dispatch_init();

    size_t len = 63;
    unsigned char* src = aligned_alloc(64, 64);
    unsigned char* dst = aligned_alloc(64, 64);
    assert_non_null(src);
    assert_non_null(dst);
    for (size_t i = 0; i < len; i++) {
        src[i] = (unsigned char)(i & 0xFF);
    }
    bc_core_fill(dst, 64, (unsigned char)0x00);

    bool result = bc_core_move(dst, src, len);

    assert_true(result);
    assert_memory_equal(dst, src, len);

    free(src);
    free(dst);
}

static void test_move_avx2_path_exact_64(void** state)
{
    BC_UNUSED(state);

    g_mock_detect_returns = true;
    bc_core_zero(&g_mock_features, sizeof(g_mock_features));

    bc_core_move_dispatch_init();

    size_t len = 64;
    unsigned char* src = aligned_alloc(64, len);
    unsigned char* dst = aligned_alloc(64, len);
    assert_non_null(src);
    assert_non_null(dst);
    for (size_t i = 0; i < len; i++) {
        src[i] = (unsigned char)(i & 0xFF);
    }
    bc_core_fill(dst, len, (unsigned char)0x00);

    bool result = bc_core_move(dst, src, len);

    assert_true(result);
    assert_memory_equal(dst, src, len);

    free(src);
    free(dst);
}

static void test_move_avx2_path_65(void** state)
{
    BC_UNUSED(state);

    g_mock_detect_returns = true;
    bc_core_zero(&g_mock_features, sizeof(g_mock_features));

    bc_core_move_dispatch_init();

    size_t len = 65;
    unsigned char* src = aligned_alloc(64, 128);
    unsigned char* dst = aligned_alloc(64, 128);
    assert_non_null(src);
    assert_non_null(dst);
    for (size_t i = 0; i < len; i++) {
        src[i] = (unsigned char)(i & 0xFF);
    }
    bc_core_fill(dst, 128, (unsigned char)0x00);

    bool result = bc_core_move(dst, src, len);

    assert_true(result);
    assert_memory_equal(dst, src, len);

    free(src);
    free(dst);
}

static void test_move_avx2_path_256(void** state)
{
    BC_UNUSED(state);

    g_mock_detect_returns = true;
    bc_core_zero(&g_mock_features, sizeof(g_mock_features));

    bc_core_move_dispatch_init();

    size_t len = 256;
    unsigned char* src = aligned_alloc(64, len);
    unsigned char* dst = aligned_alloc(64, len);
    assert_non_null(src);
    assert_non_null(dst);
    for (size_t i = 0; i < len; i++) {
        src[i] = (unsigned char)(i & 0xFF);
    }
    bc_core_fill(dst, len, (unsigned char)0x00);

    bool result = bc_core_move(dst, src, len);

    assert_true(result);
    assert_memory_equal(dst, src, len);

    free(src);
    free(dst);
}

static void test_move_avx2_path_large(void** state)
{
    BC_UNUSED(state);

    g_mock_detect_returns = true;
    bc_core_zero(&g_mock_features, sizeof(g_mock_features));

    bc_core_move_dispatch_init();

    size_t len = 257;
    unsigned char* src = aligned_alloc(64, 320);
    unsigned char* dst = aligned_alloc(64, 320);
    assert_non_null(src);
    assert_non_null(dst);
    for (size_t i = 0; i < len; i++) {
        src[i] = (unsigned char)(i & 0xFF);
    }
    bc_core_fill(dst, 320, (unsigned char)0x00);

    bool result = bc_core_move(dst, src, len);

    assert_true(result);
    assert_memory_equal(dst, src, len);

    free(src);
    free(dst);
}

static void test_move_avx2_forward_overlap(void** state)
{
    BC_UNUSED(state);

    g_mock_detect_returns = true;
    bc_core_zero(&g_mock_features, sizeof(g_mock_features));

    bc_core_move_dispatch_init();

    size_t total = 128;
    size_t len = 96;
    unsigned char* buf = aligned_alloc(64, total);
    assert_non_null(buf);
    for (size_t i = 0; i < total; i++) {
        buf[i] = (unsigned char)(i & 0xFF);
    }

    unsigned char expected[96];
    for (size_t i = 0; i < len; i++) {
        expected[i] = (unsigned char)(i & 0xFF);
    }

    bool result = bc_core_move(buf + 32, buf, len);

    assert_true(result);
    assert_memory_equal(buf + 32, expected, len);

    free(buf);
}

static void test_move_avx2_backward_overlap(void** state)
{
    BC_UNUSED(state);

    g_mock_detect_returns = true;
    bc_core_zero(&g_mock_features, sizeof(g_mock_features));

    bc_core_move_dispatch_init();

    size_t total = 128;
    size_t len = 96;
    unsigned char* buf = aligned_alloc(64, total);
    assert_non_null(buf);
    for (size_t i = 0; i < total; i++) {
        buf[i] = (unsigned char)(i & 0xFF);
    }

    unsigned char expected[96];
    for (size_t i = 0; i < len; i++) {
        expected[i] = (unsigned char)((i + 32) & 0xFF);
    }

    bool result = bc_core_move(buf, buf + 32, len);

    assert_true(result);
    assert_memory_equal(buf, expected, len);

    free(buf);
}

static void test_move_avx2_path_len_1(void** state)
{
    BC_UNUSED(state);

    g_mock_detect_returns = true;
    bc_core_zero(&g_mock_features, sizeof(g_mock_features));

    bc_core_move_dispatch_init();

    size_t len = 1;
    unsigned char* src = aligned_alloc(64, 64);
    unsigned char* dst = aligned_alloc(64, 64);
    assert_non_null(src);
    assert_non_null(dst);
    src[0] = 0xAB;
    bc_core_fill(dst, 64, (unsigned char)0x00);

    bool result = bc_core_move(dst, src, len);

    assert_true(result);
    assert_memory_equal(dst, src, len);

    free(src);
    free(dst);
}

static void test_move_avx2_path_len_16(void** state)
{
    BC_UNUSED(state);

    g_mock_detect_returns = true;
    bc_core_zero(&g_mock_features, sizeof(g_mock_features));

    bc_core_move_dispatch_init();

    size_t len = 16;
    unsigned char* src = aligned_alloc(64, 64);
    unsigned char* dst = aligned_alloc(64, 64);
    assert_non_null(src);
    assert_non_null(dst);
    for (size_t i = 0; i < len; i++) {
        src[i] = (unsigned char)(i & 0xFF);
    }
    bc_core_fill(dst, 64, (unsigned char)0x00);

    bool result = bc_core_move(dst, src, len);

    assert_true(result);
    assert_memory_equal(dst, src, len);

    free(src);
    free(dst);
}

static void test_move_avx2_path_len_31(void** state)
{
    BC_UNUSED(state);

    g_mock_detect_returns = true;
    bc_core_zero(&g_mock_features, sizeof(g_mock_features));

    bc_core_move_dispatch_init();

    size_t len = 31;
    unsigned char* src = aligned_alloc(64, 64);
    unsigned char* dst = aligned_alloc(64, 64);
    assert_non_null(src);
    assert_non_null(dst);
    for (size_t i = 0; i < len; i++) {
        src[i] = (unsigned char)(i & 0xFF);
    }
    bc_core_fill(dst, 64, (unsigned char)0x00);

    bool result = bc_core_move(dst, src, len);

    assert_true(result);
    assert_memory_equal(dst, src, len);

    free(src);
    free(dst);
}

static void test_move_avx2_backward_overlap_small(void** state)
{
    BC_UNUSED(state);

    g_mock_detect_returns = true;
    bc_core_zero(&g_mock_features, sizeof(g_mock_features));

    bc_core_move_dispatch_init();

    size_t total = 64;
    size_t len = 16;
    unsigned char* buf = aligned_alloc(64, total);
    assert_non_null(buf);
    for (size_t i = 0; i < total; i++) {
        buf[i] = (unsigned char)(i & 0xFF);
    }

    unsigned char expected[16];
    for (size_t i = 0; i < len; i++) {
        expected[i] = (unsigned char)((i + 8) & 0xFF);
    }

    bool result = bc_core_move(buf, buf + 8, len);

    assert_true(result);
    assert_memory_equal(buf, expected, len);

    free(buf);
}

static void test_move_avx2_unaligned_forward(void** state)
{
    BC_UNUSED(state);

    g_mock_detect_returns = true;
    bc_core_zero(&g_mock_features, sizeof(g_mock_features));

    bc_core_move_dispatch_init();

    size_t len = 5;
    unsigned char* buf = aligned_alloc(64, 128);
    assert_non_null(buf);
    for (size_t i = 0; i < 128; i++) {
        buf[i] = (unsigned char)(i & 0xFF);
    }

    unsigned char expected[5];
    for (size_t i = 0; i < len; i++) {
        expected[i] = buf[64 + i];
    }

    bool result = bc_core_move(buf + 1, buf + 64, len);

    assert_true(result);
    assert_memory_equal(buf + 1, expected, len);

    free(buf);
}

static void test_move_avx2_backward_overlap_tail(void** state)
{
    BC_UNUSED(state);

    g_mock_detect_returns = true;
    bc_core_zero(&g_mock_features, sizeof(g_mock_features));

    bc_core_move_dispatch_init();

    size_t total = 192;
    size_t len = 97;
    unsigned char* buf = aligned_alloc(64, total);
    assert_non_null(buf);
    for (size_t i = 0; i < total; i++) {
        buf[i] = (unsigned char)(i & 0xFF);
    }

    unsigned char expected[97];
    for (size_t i = 0; i < len; i++) {
        expected[i] = (unsigned char)((i + 32) & 0xFF);
    }

    bool result = bc_core_move(buf, buf + 32, len);

    assert_true(result);
    assert_memory_equal(buf, expected, len);

    free(buf);
}

static void test_move_avx2_backward_with_tail_bytes(void** state)
{
    BC_UNUSED(state);

    g_mock_detect_returns = true;
    bc_core_zero(&g_mock_features, sizeof(g_mock_features));

    bc_core_move_dispatch_init();

    size_t total = 128;
    size_t len = 35;
    unsigned char* buf = aligned_alloc(64, total);
    assert_non_null(buf);
    for (size_t i = 0; i < total; i++) {
        buf[i] = (unsigned char)(i & 0xFF);
    }

    unsigned char expected[35];
    for (size_t i = 0; i < len; i++) {
        expected[i] = (unsigned char)((i + 8) & 0xFF);
    }

    bool result = bc_core_move(buf, buf + 8, len);

    assert_true(result);
    assert_memory_equal(buf, expected, len);

    free(buf);
}

static void test_move_avx2_forward_overlap_small_len_caps_head(void** state)
{
    BC_UNUSED(state);

    g_mock_detect_returns = true;
    bc_core_zero(&g_mock_features, sizeof(g_mock_features));
    bc_core_move_dispatch_init();

    unsigned char* buf = aligned_alloc(64, 128);
    assert_non_null(buf);
    for (size_t i = 0; i < 128; i++) {
        buf[i] = (unsigned char)(i & 0xFF);
    }
    unsigned char* dst = buf + 1;
    const unsigned char* src = buf + 2;
    unsigned char expected[5];
    for (size_t i = 0; i < 5; i++) {
        expected[i] = (unsigned char)((i + 2) & 0xFF);
    }

    bool result = bc_core_move(dst, src, 5);

    assert_true(result);
    assert_memory_equal(dst, expected, 5);

    free(buf);
}

static void test_move_avx2_forward_overlap_unaligned_dst_scalar_head(void** state)
{
    BC_UNUSED(state);

    g_mock_detect_returns = true;
    bc_core_zero(&g_mock_features, sizeof(g_mock_features));
    bc_core_move_dispatch_init();

    unsigned char* buf = aligned_alloc(64, 256);
    assert_non_null(buf);
    for (size_t i = 0; i < 256; i++) {
        buf[i] = (unsigned char)(i & 0xFF);
    }
    unsigned char* dst = buf + 1;
    const unsigned char* src = buf + 2;
    size_t len = 64;
    unsigned char expected[64];
    for (size_t i = 0; i < len; i++) {
        expected[i] = (unsigned char)((i + 2) & 0xFF);
    }

    bool result = bc_core_move(dst, src, len);

    assert_true(result);
    assert_memory_equal(dst, expected, len);

    free(buf);
}

static void test_move_dispatch_detect_failure_avx2_fallback(void** state)
{
    BC_UNUSED(state);

    g_mock_detect_returns = false;
    bc_core_move_dispatch_init();

    unsigned char* src = aligned_alloc(64, 64);
    unsigned char* dst = aligned_alloc(64, 64);
    assert_non_null(src);
    assert_non_null(dst);
    for (size_t i = 0; i < 64; i++) {
        src[i] = (unsigned char)(i & 0xFF);
    }
    bc_core_fill(dst, 64, (unsigned char)0x00);

    bool result = bc_core_move(dst, src, 64);

    assert_true(result);
    assert_memory_equal(dst, src, 64);

    free(src);
    free(dst);

    g_mock_detect_returns = true;
    bc_core_zero(&g_mock_features, sizeof(g_mock_features));
    bc_core_move_dispatch_init();
}

int main(void)
{
    const struct CMUnitTest tests[] = {cmocka_unit_test(test_move_avx2_path_len_0),
                                       cmocka_unit_test(test_move_avx2_path_len_1),
                                       cmocka_unit_test(test_move_avx2_path_len_16),
                                       cmocka_unit_test(test_move_avx2_path_len_31),
                                       cmocka_unit_test(test_move_avx2_path_small),
                                       cmocka_unit_test(test_move_avx2_path_exact_64),
                                       cmocka_unit_test(test_move_avx2_path_65),
                                       cmocka_unit_test(test_move_avx2_path_256),
                                       cmocka_unit_test(test_move_avx2_path_large),
                                       cmocka_unit_test(test_move_avx2_forward_overlap),
                                       cmocka_unit_test(test_move_avx2_backward_overlap),
                                       cmocka_unit_test(test_move_avx2_backward_overlap_small),
                                       cmocka_unit_test(test_move_avx2_unaligned_forward),
                                       cmocka_unit_test(test_move_avx2_backward_overlap_tail),
                                       cmocka_unit_test(test_move_avx2_backward_with_tail_bytes),
                                       cmocka_unit_test(test_move_avx2_forward_overlap_small_len_caps_head),
                                       cmocka_unit_test(test_move_avx2_forward_overlap_unaligned_dst_scalar_head),
                                       cmocka_unit_test(test_move_dispatch_detect_failure_avx2_fallback)};

    return cmocka_run_group_tests(tests, NULL, NULL);
}
