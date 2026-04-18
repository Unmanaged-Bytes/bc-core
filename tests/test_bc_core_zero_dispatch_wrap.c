// SPDX-License-Identifier: MIT

#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <cmocka.h>

#include "bc_core.h"
#include "test_bc_core_dispatch_mock.h"

#include <stdlib.h>
#include <unistd.h>

static long g_mock_sysconf_value = 256;

long __wrap_sysconf(int name)
{
    BC_UNUSED(name);
    return g_mock_sysconf_value;
}

extern void bc_core_zero_dispatch_init(void);

static void test_zero_avx2_path_len_0(void** state)
{
    BC_UNUSED(state);

    g_mock_detect_returns = true;
    bc_core_zero(&g_mock_features, sizeof(g_mock_features));

    bc_core_zero_dispatch_init();

    unsigned char* buf = aligned_alloc(64, 64);
    assert_non_null(buf);
    bc_core_fill(buf, 64, (unsigned char)0xAA);

    bool result = bc_core_zero(buf, 0);

    assert_true(result);

    free(buf);
}

static void test_zero_avx2_path_small(void** state)
{
    BC_UNUSED(state);

    g_mock_detect_returns = true;
    bc_core_zero(&g_mock_features, sizeof(g_mock_features));

    bc_core_zero_dispatch_init();

    size_t len = 63;
    unsigned char* buf = aligned_alloc(64, 64);
    assert_non_null(buf);
    bc_core_fill(buf, 64, (unsigned char)0xAA);

    bool result = bc_core_zero(buf, len);

    assert_true(result);
    for (size_t i = 0; i < len; i++) {
        assert_int_equal(buf[i], 0);
    }

    free(buf);
}

static void test_zero_avx2_path_exact_64(void** state)
{
    BC_UNUSED(state);

    g_mock_detect_returns = true;
    bc_core_zero(&g_mock_features, sizeof(g_mock_features));

    bc_core_zero_dispatch_init();

    size_t len = 64;
    unsigned char* buf = aligned_alloc(64, len);
    assert_non_null(buf);
    bc_core_fill(buf, len, (unsigned char)0xAA);

    bool result = bc_core_zero(buf, len);

    assert_true(result);
    for (size_t i = 0; i < len; i++) {
        assert_int_equal(buf[i], 0);
    }

    free(buf);
}

static void test_zero_avx2_path_large(void** state)
{
    BC_UNUSED(state);

    g_mock_detect_returns = true;
    bc_core_zero(&g_mock_features, sizeof(g_mock_features));

    bc_core_zero_dispatch_init();

    size_t len = 257;
    unsigned char* buf = aligned_alloc(64, 320);
    assert_non_null(buf);
    bc_core_fill(buf, 320, (unsigned char)0xAA);

    bool result = bc_core_zero(buf, len);

    assert_true(result);
    for (size_t i = 0; i < len; i++) {
        assert_int_equal(buf[i], 0);
    }

    free(buf);
}

static void test_zero_avx2_path_non_nt_large(void** state)
{
    BC_UNUSED(state);

    g_mock_detect_returns = true;
    bc_core_zero(&g_mock_features, sizeof(g_mock_features));

    bc_core_zero_dispatch_init();

    size_t len = 200;
    unsigned char* buf = aligned_alloc(64, 256);
    assert_non_null(buf);
    bc_core_fill(buf, 256, (unsigned char)0xAA);

    bool result = bc_core_zero(buf, len);

    assert_true(result);
    for (size_t i = 0; i < len; i++) {
        assert_int_equal(buf[i], 0);
    }

    free(buf);
}

static void test_zero_avx2_path_65(void** state)
{
    BC_UNUSED(state);

    g_mock_detect_returns = true;
    bc_core_zero(&g_mock_features, sizeof(g_mock_features));

    bc_core_zero_dispatch_init();

    size_t len = 65;
    unsigned char* buf = aligned_alloc(64, 128);
    assert_non_null(buf);
    bc_core_fill(buf, 128, (unsigned char)0xAA);

    bool result = bc_core_zero(buf, len);

    assert_true(result);
    for (size_t i = 0; i < len; i++) {
        assert_int_equal(buf[i], 0);
    }

    free(buf);
}

static void test_zero_avx2_path_len_1(void** state)
{
    BC_UNUSED(state);

    g_mock_detect_returns = true;
    bc_core_zero(&g_mock_features, sizeof(g_mock_features));

    bc_core_zero_dispatch_init();

    size_t len = 1;
    unsigned char* buf = aligned_alloc(64, 64);
    assert_non_null(buf);
    bc_core_fill(buf, 64, (unsigned char)0xAA);

    bool result = bc_core_zero(buf, len);

    assert_true(result);
    assert_int_equal(buf[0], 0);

    free(buf);
}

static void test_zero_avx2_path_len_16(void** state)
{
    BC_UNUSED(state);

    g_mock_detect_returns = true;
    bc_core_zero(&g_mock_features, sizeof(g_mock_features));

    bc_core_zero_dispatch_init();

    size_t len = 16;
    unsigned char* buf = aligned_alloc(64, 64);
    assert_non_null(buf);
    bc_core_fill(buf, 64, (unsigned char)0xAA);

    bool result = bc_core_zero(buf, len);

    assert_true(result);
    for (size_t i = 0; i < len; i++) {
        assert_int_equal(buf[i], 0);
    }

    free(buf);
}

static void test_zero_avx2_path_len_31(void** state)
{
    BC_UNUSED(state);

    g_mock_detect_returns = true;
    bc_core_zero(&g_mock_features, sizeof(g_mock_features));

    bc_core_zero_dispatch_init();

    size_t len = 31;
    unsigned char* buf = aligned_alloc(64, 64);
    assert_non_null(buf);
    bc_core_fill(buf, 64, (unsigned char)0xAA);

    bool result = bc_core_zero(buf, len);

    assert_true(result);
    for (size_t i = 0; i < len; i++) {
        assert_int_equal(buf[i], 0);
    }

    free(buf);
}

static void test_zero_avx2_nt_store_path(void** state)
{
    BC_UNUSED(state);

    g_mock_detect_returns = true;
    bc_core_zero(&g_mock_features, sizeof(g_mock_features));

    bc_core_zero_dispatch_init();

    size_t len = 512;
    unsigned char* buf = aligned_alloc(64, len);
    assert_non_null(buf);
    bc_core_fill(buf, len, (unsigned char)0xAA);

    bool result = bc_core_zero(buf, len);

    assert_true(result);
    for (size_t i = 0; i < len; i++) {
        assert_int_equal(buf[i], 0);
    }

    free(buf);
}

static void test_zero_dispatch_detect_failure_avx2_fallback(void** state)
{
    BC_UNUSED(state);

    g_mock_detect_returns = false;
    bc_core_zero_dispatch_init();

    unsigned char buf[16];
    bc_core_fill(buf, 16, (unsigned char)0xFF);
    bool success = bc_core_zero(buf, 16);

    assert_true(success);
    for (size_t i = 0; i < 16; i++) {
        assert_int_equal(buf[i], 0);
    }

    g_mock_detect_returns = true;
    bc_core_zero(&g_mock_features, sizeof(g_mock_features));
    bc_core_zero_dispatch_init();
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_zero_avx2_path_len_0),
        cmocka_unit_test(test_zero_avx2_path_65),
        cmocka_unit_test(test_zero_avx2_path_len_1),
        cmocka_unit_test(test_zero_avx2_path_len_16),
        cmocka_unit_test(test_zero_avx2_path_len_31),
        cmocka_unit_test(test_zero_avx2_path_small),
        cmocka_unit_test(test_zero_avx2_path_exact_64),
        cmocka_unit_test(test_zero_avx2_path_large),
        cmocka_unit_test(test_zero_avx2_path_non_nt_large),
        cmocka_unit_test(test_zero_avx2_nt_store_path),
        cmocka_unit_test(test_zero_dispatch_detect_failure_avx2_fallback),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
