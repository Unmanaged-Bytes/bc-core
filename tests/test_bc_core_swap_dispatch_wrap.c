// SPDX-License-Identifier: MIT

#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <cmocka.h>

#include "bc_core.h"
#include "test_bc_core_dispatch_mock.h"

#include <stdlib.h>

extern void bc_core_swap_dispatch_init(void);

static void test_swap_avx2_path_len_0(void** state)
{
    BC_UNUSED(state);

    g_mock_detect_returns = true;
    bc_core_zero(&g_mock_features, sizeof(g_mock_features));

    bc_core_swap_dispatch_init();

    unsigned char* buf_a = aligned_alloc(64, 64);
    unsigned char* buf_b = aligned_alloc(64, 64);
    assert_non_null(buf_a);
    assert_non_null(buf_b);
    bc_core_fill(buf_a, 64, (unsigned char)0xAA);
    bc_core_fill(buf_b, 64, (unsigned char)0xBB);

    bool result = bc_core_swap(buf_a, buf_b, 0);

    assert_true(result);

    free(buf_a);
    free(buf_b);
}

static void test_swap_avx2_path_small(void** state)
{
    BC_UNUSED(state);

    g_mock_detect_returns = true;
    bc_core_zero(&g_mock_features, sizeof(g_mock_features));

    bc_core_swap_dispatch_init();

    size_t len = 63;
    unsigned char* buf_a = aligned_alloc(64, 64);
    unsigned char* buf_b = aligned_alloc(64, 64);
    assert_non_null(buf_a);
    assert_non_null(buf_b);

    unsigned char original_a[63];
    unsigned char original_b[63];
    for (size_t i = 0; i < len; i++) {
        buf_a[i] = (unsigned char)(i & 0xFF);
        buf_b[i] = (unsigned char)((i + 128) & 0xFF);
        original_a[i] = buf_a[i];
        original_b[i] = buf_b[i];
    }

    bool result = bc_core_swap(buf_a, buf_b, len);

    assert_true(result);
    assert_memory_equal(buf_a, original_b, len);
    assert_memory_equal(buf_b, original_a, len);

    free(buf_a);
    free(buf_b);
}

static void test_swap_avx2_path_exact_64(void** state)
{
    BC_UNUSED(state);

    g_mock_detect_returns = true;
    bc_core_zero(&g_mock_features, sizeof(g_mock_features));

    bc_core_swap_dispatch_init();

    size_t len = 64;
    unsigned char* buf_a = aligned_alloc(64, len);
    unsigned char* buf_b = aligned_alloc(64, len);
    assert_non_null(buf_a);
    assert_non_null(buf_b);

    unsigned char original_a[64];
    unsigned char original_b[64];
    for (size_t i = 0; i < len; i++) {
        buf_a[i] = (unsigned char)(i & 0xFF);
        buf_b[i] = (unsigned char)((i + 128) & 0xFF);
        original_a[i] = buf_a[i];
        original_b[i] = buf_b[i];
    }

    bool result = bc_core_swap(buf_a, buf_b, len);

    assert_true(result);
    assert_memory_equal(buf_a, original_b, len);
    assert_memory_equal(buf_b, original_a, len);

    free(buf_a);
    free(buf_b);
}

static void test_swap_avx2_path_65(void** state)
{
    BC_UNUSED(state);

    g_mock_detect_returns = true;
    bc_core_zero(&g_mock_features, sizeof(g_mock_features));

    bc_core_swap_dispatch_init();

    size_t len = 65;
    unsigned char* buf_a = aligned_alloc(64, 128);
    unsigned char* buf_b = aligned_alloc(64, 128);
    assert_non_null(buf_a);
    assert_non_null(buf_b);

    unsigned char original_a[65];
    unsigned char original_b[65];
    for (size_t i = 0; i < len; i++) {
        buf_a[i] = (unsigned char)(i & 0xFF);
        buf_b[i] = (unsigned char)((i + 128) & 0xFF);
        original_a[i] = buf_a[i];
        original_b[i] = buf_b[i];
    }

    bool result = bc_core_swap(buf_a, buf_b, len);

    assert_true(result);
    assert_memory_equal(buf_a, original_b, len);
    assert_memory_equal(buf_b, original_a, len);

    free(buf_a);
    free(buf_b);
}

static void test_swap_avx2_path_256(void** state)
{
    BC_UNUSED(state);

    g_mock_detect_returns = true;
    bc_core_zero(&g_mock_features, sizeof(g_mock_features));

    bc_core_swap_dispatch_init();

    size_t len = 256;
    unsigned char* buf_a = aligned_alloc(64, len);
    unsigned char* buf_b = aligned_alloc(64, len);
    assert_non_null(buf_a);
    assert_non_null(buf_b);

    unsigned char original_a[256];
    unsigned char original_b[256];
    for (size_t i = 0; i < len; i++) {
        buf_a[i] = (unsigned char)(i & 0xFF);
        buf_b[i] = (unsigned char)((i + 128) & 0xFF);
        original_a[i] = buf_a[i];
        original_b[i] = buf_b[i];
    }

    bool result = bc_core_swap(buf_a, buf_b, len);

    assert_true(result);
    assert_memory_equal(buf_a, original_b, len);
    assert_memory_equal(buf_b, original_a, len);

    free(buf_a);
    free(buf_b);
}

static void test_swap_avx2_path_large(void** state)
{
    BC_UNUSED(state);

    g_mock_detect_returns = true;
    bc_core_zero(&g_mock_features, sizeof(g_mock_features));

    bc_core_swap_dispatch_init();

    size_t len = 257;
    unsigned char* buf_a = aligned_alloc(64, 320);
    unsigned char* buf_b = aligned_alloc(64, 320);
    assert_non_null(buf_a);
    assert_non_null(buf_b);

    unsigned char original_a[257];
    unsigned char original_b[257];
    for (size_t i = 0; i < len; i++) {
        buf_a[i] = (unsigned char)(i & 0xFF);
        buf_b[i] = (unsigned char)((i + 128) & 0xFF);
        original_a[i] = buf_a[i];
        original_b[i] = buf_b[i];
    }

    bool result = bc_core_swap(buf_a, buf_b, len);

    assert_true(result);
    assert_memory_equal(buf_a, original_b, len);
    assert_memory_equal(buf_b, original_a, len);

    free(buf_a);
    free(buf_b);
}

static void test_swap_dispatch_detect_failure_avx2_fallback(void** state)
{
    BC_UNUSED(state);

    g_mock_detect_returns = false;
    bc_core_swap_dispatch_init();

    unsigned char a[4] = {0x01, 0x02, 0x03, 0x04};
    unsigned char b[4] = {0x0A, 0x0B, 0x0C, 0x0D};
    bool success = bc_core_swap(a, b, 4);

    assert_true(success);
    assert_int_equal(a[0], 0x0A);
    assert_int_equal(b[0], 0x01);

    g_mock_detect_returns = true;
    bc_core_zero(&g_mock_features, sizeof(g_mock_features));
    bc_core_swap_dispatch_init();
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_swap_avx2_path_len_0),
        cmocka_unit_test(test_swap_avx2_path_small),
        cmocka_unit_test(test_swap_avx2_path_exact_64),
        cmocka_unit_test(test_swap_avx2_path_65),
        cmocka_unit_test(test_swap_avx2_path_256),
        cmocka_unit_test(test_swap_avx2_path_large),
        cmocka_unit_test(test_swap_dispatch_detect_failure_avx2_fallback),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
