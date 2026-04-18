// SPDX-License-Identifier: MIT

#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <cmocka.h>

#include "bc_core.h"
#include "test_bc_core_dispatch_mock.h"

#include <stdlib.h>

extern void bc_core_fill_dispatch_init(void);

static void test_fill_avx2_path_len_0(void** state)
{
    BC_UNUSED(state);

    g_mock_detect_returns = true;
    bc_core_zero(&g_mock_features, sizeof(g_mock_features));

    bc_core_fill_dispatch_init();

    unsigned char* buf = aligned_alloc(64, 64);
    assert_non_null(buf);
    bc_core_fill(buf, 64, (unsigned char)0x00);

    bool result = bc_core_fill(buf, 0, 0x42);

    assert_true(result);

    free(buf);
}

static void test_fill_avx2_path_small(void** state)
{
    BC_UNUSED(state);

    g_mock_detect_returns = true;
    bc_core_zero(&g_mock_features, sizeof(g_mock_features));

    bc_core_fill_dispatch_init();

    size_t len = 63;
    unsigned char* buf = aligned_alloc(64, 64);
    assert_non_null(buf);
    bc_core_fill(buf, 64, (unsigned char)0x00);

    bool result = bc_core_fill(buf, len, 0x42);

    assert_true(result);
    for (size_t i = 0; i < len; i++) {
        assert_int_equal(buf[i], 0x42);
    }

    free(buf);
}

static void test_fill_avx2_path_exact_64(void** state)
{
    BC_UNUSED(state);

    g_mock_detect_returns = true;
    bc_core_zero(&g_mock_features, sizeof(g_mock_features));

    bc_core_fill_dispatch_init();

    size_t len = 64;
    unsigned char* buf = aligned_alloc(64, len);
    assert_non_null(buf);
    bc_core_fill(buf, len, (unsigned char)0x00);

    bool result = bc_core_fill(buf, len, 0x42);

    assert_true(result);
    for (size_t i = 0; i < len; i++) {
        assert_int_equal(buf[i], 0x42);
    }

    free(buf);
}

static void test_fill_avx2_path_65(void** state)
{
    BC_UNUSED(state);

    g_mock_detect_returns = true;
    bc_core_zero(&g_mock_features, sizeof(g_mock_features));

    bc_core_fill_dispatch_init();

    size_t len = 65;
    unsigned char* buf = aligned_alloc(64, 128);
    assert_non_null(buf);
    bc_core_fill(buf, 128, (unsigned char)0x00);

    bool result = bc_core_fill(buf, len, 0x42);

    assert_true(result);
    for (size_t i = 0; i < len; i++) {
        assert_int_equal(buf[i], 0x42);
    }

    free(buf);
}

static void test_fill_avx2_path_256(void** state)
{
    BC_UNUSED(state);

    g_mock_detect_returns = true;
    bc_core_zero(&g_mock_features, sizeof(g_mock_features));

    bc_core_fill_dispatch_init();

    size_t len = 256;
    unsigned char* buf = aligned_alloc(64, len);
    assert_non_null(buf);
    bc_core_fill(buf, len, (unsigned char)0x00);

    bool result = bc_core_fill(buf, len, 0x42);

    assert_true(result);
    for (size_t i = 0; i < len; i++) {
        assert_int_equal(buf[i], 0x42);
    }

    free(buf);
}

static void test_fill_avx2_path_large(void** state)
{
    BC_UNUSED(state);

    g_mock_detect_returns = true;
    bc_core_zero(&g_mock_features, sizeof(g_mock_features));

    bc_core_fill_dispatch_init();

    size_t len = 257;
    unsigned char* buf = aligned_alloc(64, 320);
    assert_non_null(buf);
    bc_core_fill(buf, 320, (unsigned char)0x00);

    bool result = bc_core_fill(buf, len, 0x42);

    assert_true(result);
    for (size_t i = 0; i < len; i++) {
        assert_int_equal(buf[i], 0x42);
    }

    free(buf);
}

static void test_fill_avx2_path_len_1(void** state)
{
    BC_UNUSED(state);

    g_mock_detect_returns = true;
    bc_core_zero(&g_mock_features, sizeof(g_mock_features));

    bc_core_fill_dispatch_init();

    size_t len = 1;
    unsigned char* buf = aligned_alloc(64, 64);
    assert_non_null(buf);
    bc_core_fill(buf, 64, (unsigned char)0x00);

    bool result = bc_core_fill(buf, len, 0x42);

    assert_true(result);
    assert_int_equal(buf[0], 0x42);

    free(buf);
}

static void test_fill_avx2_unaligned_small(void** state)
{
    BC_UNUSED(state);

    g_mock_detect_returns = true;
    bc_core_zero(&g_mock_features, sizeof(g_mock_features));

    bc_core_fill_dispatch_init();

    unsigned char* buf = aligned_alloc(64, 64);
    assert_non_null(buf);
    bc_core_fill(buf, 64, (unsigned char)0x00);

    bool result = bc_core_fill(buf + 1, 5, 0x42);

    assert_true(result);
    for (size_t i = 0; i < 5; i++) {
        assert_int_equal(buf[1 + i], 0x42);
    }

    free(buf);
}

static void test_fill_avx2_unaligned_large(void** state)
{
    BC_UNUSED(state);

    g_mock_detect_returns = true;
    bc_core_zero(&g_mock_features, sizeof(g_mock_features));

    bc_core_fill_dispatch_init();

    unsigned char* buf = aligned_alloc(64, 128);
    assert_non_null(buf);
    bc_core_fill(buf, 128, (unsigned char)0x00);

    bool result = bc_core_fill(buf + 1, 64, 0x42);

    assert_true(result);
    for (size_t i = 0; i < 64; i++) {
        assert_int_equal(buf[1 + i], 0x42);
    }

    free(buf);
}

static void test_fill_avx2_path_len_5(void** state)
{
    BC_UNUSED(state);

    g_mock_detect_returns = true;
    bc_core_zero(&g_mock_features, sizeof(g_mock_features));

    bc_core_fill_dispatch_init();

    size_t len = 5;
    unsigned char* buf = aligned_alloc(64, 64);
    assert_non_null(buf);
    bc_core_fill(buf, 64, (unsigned char)0x00);

    bool result = bc_core_fill(buf, len, 0x42);

    assert_true(result);
    for (size_t i = 0; i < len; i++) {
        assert_int_equal(buf[i], 0x42);
    }

    free(buf);
}

static void test_fill_avx2_path_len_31(void** state)
{
    BC_UNUSED(state);

    g_mock_detect_returns = true;
    bc_core_zero(&g_mock_features, sizeof(g_mock_features));

    bc_core_fill_dispatch_init();

    size_t len = 31;
    unsigned char* buf = aligned_alloc(64, 64);
    assert_non_null(buf);
    bc_core_fill(buf, 64, (unsigned char)0x00);

    bool result = bc_core_fill(buf, len, 0x42);

    assert_true(result);
    for (size_t i = 0; i < len; i++) {
        assert_int_equal(buf[i], 0x42);
    }

    free(buf);
}

static void test_fill_dispatch_detect_failure_avx2_fallback(void** state)
{
    BC_UNUSED(state);

    g_mock_detect_returns = false;
    bc_core_fill_dispatch_init();

    unsigned char buf[16] = {0};
    bool success = bc_core_fill(buf, 16, 0x5A);

    assert_true(success);
    for (size_t i = 0; i < 16; i++) {
        assert_int_equal(buf[i], 0x5A);
    }

    g_mock_detect_returns = true;
    bc_core_zero(&g_mock_features, sizeof(g_mock_features));
    bc_core_fill_dispatch_init();
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_fill_avx2_path_len_0),      cmocka_unit_test(test_fill_avx2_path_len_1),
        cmocka_unit_test(test_fill_avx2_path_len_5),      cmocka_unit_test(test_fill_avx2_path_len_31),
        cmocka_unit_test(test_fill_avx2_unaligned_small), cmocka_unit_test(test_fill_avx2_unaligned_large),
        cmocka_unit_test(test_fill_avx2_path_small),      cmocka_unit_test(test_fill_avx2_path_exact_64),
        cmocka_unit_test(test_fill_avx2_path_65),         cmocka_unit_test(test_fill_avx2_path_256),
        cmocka_unit_test(test_fill_avx2_path_large),      cmocka_unit_test(test_fill_dispatch_detect_failure_avx2_fallback),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
