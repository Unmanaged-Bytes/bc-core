// SPDX-License-Identifier: MIT

#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <cmocka.h>

#include "bc_core.h"
#include "test_bc_core_dispatch_mock.h"

#include <stdlib.h>

extern void bc_core_compare_dispatch_init(void);

static void test_compare_avx2_len_0(void** state)
{
    BC_UNUSED(state);

    g_mock_detect_returns = true;
    bc_core_zero(&g_mock_features, sizeof(g_mock_features));

    bc_core_compare_dispatch_init();

    unsigned char* a = aligned_alloc(64, 64);
    assert_non_null(a);
    unsigned char* b = aligned_alloc(64, 64);
    assert_non_null(b);

    int out_result = 99;
    bool success = bc_core_compare(a, b, 0, &out_result);

    assert_true(success);
    assert_int_equal(out_result, 0);

    free(a);
    free(b);
}

static void test_compare_avx2_identical(void** state)
{
    BC_UNUSED(state);

    g_mock_detect_returns = true;
    bc_core_zero(&g_mock_features, sizeof(g_mock_features));

    bc_core_compare_dispatch_init();

    unsigned char* a = aligned_alloc(64, 320);
    assert_non_null(a);
    unsigned char* b = aligned_alloc(64, 320);
    assert_non_null(b);
    bc_core_fill(a, 257, (unsigned char)0x55);
    bc_core_fill(b, 257, (unsigned char)0x55);

    int out_result = 99;
    bool success = bc_core_compare(a, b, 257, &out_result);

    assert_true(success);
    assert_int_equal(out_result, 0);

    free(a);
    free(b);
}

static void test_compare_avx2_less_first_byte(void** state)
{
    BC_UNUSED(state);

    g_mock_detect_returns = true;
    bc_core_zero(&g_mock_features, sizeof(g_mock_features));

    bc_core_compare_dispatch_init();

    unsigned char* a = aligned_alloc(64, 64);
    assert_non_null(a);
    unsigned char* b = aligned_alloc(64, 64);
    assert_non_null(b);
    bc_core_fill(a, 64, (unsigned char)0x55);
    bc_core_fill(b, 64, (unsigned char)0x55);
    a[0] = 0x10;
    b[0] = 0x20;

    int out_result = 0;
    bool success = bc_core_compare(a, b, 64, &out_result);

    assert_true(success);
    assert_true(out_result < 0);

    free(a);
    free(b);
}

static void test_compare_avx2_greater_first_byte(void** state)
{
    BC_UNUSED(state);

    g_mock_detect_returns = true;
    bc_core_zero(&g_mock_features, sizeof(g_mock_features));

    bc_core_compare_dispatch_init();

    unsigned char* a = aligned_alloc(64, 64);
    assert_non_null(a);
    unsigned char* b = aligned_alloc(64, 64);
    assert_non_null(b);
    bc_core_fill(a, 64, (unsigned char)0x55);
    bc_core_fill(b, 64, (unsigned char)0x55);
    a[0] = 0x20;
    b[0] = 0x10;

    int out_result = 0;
    bool success = bc_core_compare(a, b, 64, &out_result);

    assert_true(success);
    assert_true(out_result > 0);

    free(a);
    free(b);
}

static void test_compare_avx2_less_at_63(void** state)
{
    BC_UNUSED(state);

    g_mock_detect_returns = true;
    bc_core_zero(&g_mock_features, sizeof(g_mock_features));

    bc_core_compare_dispatch_init();

    unsigned char* a = aligned_alloc(64, 128);
    assert_non_null(a);
    unsigned char* b = aligned_alloc(64, 128);
    assert_non_null(b);
    bc_core_fill(a, 128, (unsigned char)0x55);
    bc_core_fill(b, 128, (unsigned char)0x55);
    a[63] = 0x10;
    b[63] = 0x20;

    int out_result = 0;
    bool success = bc_core_compare(a, b, 128, &out_result);

    assert_true(success);
    assert_true(out_result < 0);

    free(a);
    free(b);
}

static void test_compare_avx2_less_at_64(void** state)
{
    BC_UNUSED(state);

    g_mock_detect_returns = true;
    bc_core_zero(&g_mock_features, sizeof(g_mock_features));

    bc_core_compare_dispatch_init();

    unsigned char* a = aligned_alloc(64, 128);
    assert_non_null(a);
    unsigned char* b = aligned_alloc(64, 128);
    assert_non_null(b);
    bc_core_fill(a, 128, (unsigned char)0x55);
    bc_core_fill(b, 128, (unsigned char)0x55);
    a[64] = 0x10;
    b[64] = 0x20;

    int out_result = 0;
    bool success = bc_core_compare(a, b, 128, &out_result);

    assert_true(success);
    assert_true(out_result < 0);

    free(a);
    free(b);
}

static void test_compare_avx2_less_at_last(void** state)
{
    BC_UNUSED(state);

    g_mock_detect_returns = true;
    bc_core_zero(&g_mock_features, sizeof(g_mock_features));

    bc_core_compare_dispatch_init();

    unsigned char* a = aligned_alloc(64, 320);
    assert_non_null(a);
    unsigned char* b = aligned_alloc(64, 320);
    assert_non_null(b);
    bc_core_fill(a, 257, (unsigned char)0x55);
    bc_core_fill(b, 257, (unsigned char)0x55);
    a[256] = 0x10;
    b[256] = 0x20;

    int out_result = 0;
    bool success = bc_core_compare(a, b, 257, &out_result);

    assert_true(success);
    assert_true(out_result < 0);

    free(a);
    free(b);
}

static void test_compare_avx2_less_in_tail_35(void** state)
{
    BC_UNUSED(state);

    g_mock_detect_returns = true;
    bc_core_zero(&g_mock_features, sizeof(g_mock_features));

    bc_core_compare_dispatch_init();

    unsigned char* a = aligned_alloc(64, 64);
    assert_non_null(a);
    unsigned char* b = aligned_alloc(64, 64);
    assert_non_null(b);
    bc_core_fill(a, 64, (unsigned char)0x55);
    bc_core_fill(b, 64, (unsigned char)0x55);
    a[33] = 0x10;
    b[33] = 0x20;

    int out_result = 0;
    bool success = bc_core_compare(a, b, 35, &out_result);

    assert_true(success);
    assert_true(out_result < 0);

    free(a);
    free(b);
}

static void test_compare_avx2_greater_in_tail_35(void** state)
{
    BC_UNUSED(state);

    g_mock_detect_returns = true;
    bc_core_zero(&g_mock_features, sizeof(g_mock_features));

    bc_core_compare_dispatch_init();

    unsigned char* a = aligned_alloc(64, 64);
    assert_non_null(a);
    unsigned char* b = aligned_alloc(64, 64);
    assert_non_null(b);
    bc_core_fill(a, 64, (unsigned char)0x55);
    bc_core_fill(b, 64, (unsigned char)0x55);
    a[33] = 0x20;
    b[33] = 0x10;

    int out_result = 0;
    bool success = bc_core_compare(a, b, 35, &out_result);

    assert_true(success);
    assert_true(out_result > 0);

    free(a);
    free(b);
}

static void test_compare_dispatch_detect_failure_avx2_fallback(void** state)
{
    BC_UNUSED(state);

    g_mock_detect_returns = false;
    bc_core_compare_dispatch_init();

    const unsigned char a[4] = {0x01, 0x02, 0x03, 0x04};
    const unsigned char b[4] = {0x01, 0x02, 0x03, 0x04};
    int result = 99;
    bool success = bc_core_compare(a, b, 4, &result);

    assert_true(success);
    assert_int_equal(result, 0);

    g_mock_detect_returns = true;
    bc_core_zero(&g_mock_features, sizeof(g_mock_features));
    bc_core_compare_dispatch_init();
}

int main(void)
{
    const struct CMUnitTest tests[] = {cmocka_unit_test(test_compare_avx2_len_0),
                                       cmocka_unit_test(test_compare_avx2_identical),
                                       cmocka_unit_test(test_compare_avx2_less_first_byte),
                                       cmocka_unit_test(test_compare_avx2_greater_first_byte),
                                       cmocka_unit_test(test_compare_avx2_less_at_63),
                                       cmocka_unit_test(test_compare_avx2_less_at_64),
                                       cmocka_unit_test(test_compare_avx2_less_at_last),
                                       cmocka_unit_test(test_compare_avx2_less_in_tail_35),
                                       cmocka_unit_test(test_compare_avx2_greater_in_tail_35),
                                       cmocka_unit_test(test_compare_dispatch_detect_failure_avx2_fallback)};

    return cmocka_run_group_tests(tests, NULL, NULL);
}
