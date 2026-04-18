// SPDX-License-Identifier: MIT

#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <cmocka.h>

#include "bc_core.h"
#include "test_bc_core_dispatch_mock.h"

#include <stdlib.h>

extern void bc_core_equal_dispatch_init(void);

static void test_equal_avx2_len_0(void** state)
{
    BC_UNUSED(state);

    g_mock_detect_returns = true;
    bc_core_zero(&g_mock_features, sizeof(g_mock_features));
    bc_core_equal_dispatch_init();

    unsigned char* a = aligned_alloc(64, 64);
    assert_non_null(a);
    unsigned char* b = aligned_alloc(64, 64);
    assert_non_null(b);

    bool out_equal = false;
    bool success = bc_core_equal(a, b, 0, &out_equal);

    assert_true(success);
    assert_true(out_equal);

    free(a);
    free(b);
}

static void test_equal_avx2_identical_small(void** state)
{
    BC_UNUSED(state);

    g_mock_detect_returns = true;
    bc_core_zero(&g_mock_features, sizeof(g_mock_features));
    bc_core_equal_dispatch_init();

    unsigned char* a = aligned_alloc(64, 64);
    assert_non_null(a);
    unsigned char* b = aligned_alloc(64, 64);
    assert_non_null(b);
    bc_core_fill(a, 63, (unsigned char)0xAB);
    bc_core_fill(b, 63, (unsigned char)0xAB);

    bool out_equal = false;
    bool success = bc_core_equal(a, b, 63, &out_equal);

    assert_true(success);
    assert_true(out_equal);

    free(a);
    free(b);
}

static void test_equal_avx2_identical_64(void** state)
{
    BC_UNUSED(state);

    g_mock_detect_returns = true;
    bc_core_zero(&g_mock_features, sizeof(g_mock_features));
    bc_core_equal_dispatch_init();

    unsigned char* a = aligned_alloc(64, 64);
    assert_non_null(a);
    unsigned char* b = aligned_alloc(64, 64);
    assert_non_null(b);
    bc_core_fill(a, 64, (unsigned char)0x7F);
    bc_core_fill(b, 64, (unsigned char)0x7F);

    bool out_equal = false;
    bool success = bc_core_equal(a, b, 64, &out_equal);

    assert_true(success);
    assert_true(out_equal);

    free(a);
    free(b);
}

static void test_equal_avx2_identical_257(void** state)
{
    BC_UNUSED(state);

    g_mock_detect_returns = true;
    bc_core_zero(&g_mock_features, sizeof(g_mock_features));
    bc_core_equal_dispatch_init();

    unsigned char* a = aligned_alloc(64, 320);
    assert_non_null(a);
    unsigned char* b = aligned_alloc(64, 320);
    assert_non_null(b);
    bc_core_fill(a, 257, (unsigned char)0x55);
    bc_core_fill(b, 257, (unsigned char)0x55);

    bool out_equal = false;
    bool success = bc_core_equal(a, b, 257, &out_equal);

    assert_true(success);
    assert_true(out_equal);

    free(a);
    free(b);
}

static void test_equal_avx2_differ_first_byte(void** state)
{
    BC_UNUSED(state);

    g_mock_detect_returns = true;
    bc_core_zero(&g_mock_features, sizeof(g_mock_features));
    bc_core_equal_dispatch_init();

    unsigned char* a = aligned_alloc(64, 64);
    assert_non_null(a);
    unsigned char* b = aligned_alloc(64, 64);
    assert_non_null(b);
    bc_core_fill(a, 64, (unsigned char)0xAA);
    bc_core_fill(b, 64, (unsigned char)0xAA);
    b[0] = 0xBB;

    bool out_equal = true;
    bool success = bc_core_equal(a, b, 64, &out_equal);

    assert_true(success);
    assert_false(out_equal);

    free(a);
    free(b);
}

static void test_equal_avx2_differ_last_byte(void** state)
{
    BC_UNUSED(state);

    g_mock_detect_returns = true;
    bc_core_zero(&g_mock_features, sizeof(g_mock_features));
    bc_core_equal_dispatch_init();

    unsigned char* a = aligned_alloc(64, 64);
    assert_non_null(a);
    unsigned char* b = aligned_alloc(64, 64);
    assert_non_null(b);
    bc_core_fill(a, 64, (unsigned char)0xAA);
    bc_core_fill(b, 64, (unsigned char)0xAA);
    b[63] = 0xBB;

    bool out_equal = true;
    bool success = bc_core_equal(a, b, 64, &out_equal);

    assert_true(success);
    assert_false(out_equal);

    free(a);
    free(b);
}

static void test_equal_avx2_differ_at_boundary_63(void** state)
{
    BC_UNUSED(state);

    g_mock_detect_returns = true;
    bc_core_zero(&g_mock_features, sizeof(g_mock_features));
    bc_core_equal_dispatch_init();

    unsigned char* a = aligned_alloc(64, 128);
    assert_non_null(a);
    unsigned char* b = aligned_alloc(64, 128);
    assert_non_null(b);
    bc_core_fill(a, 128, (unsigned char)0xCC);
    bc_core_fill(b, 128, (unsigned char)0xCC);
    b[63] = 0xDD;

    bool out_equal = true;
    bool success = bc_core_equal(a, b, 128, &out_equal);

    assert_true(success);
    assert_false(out_equal);

    free(a);
    free(b);
}

static void test_equal_avx2_differ_at_boundary_64(void** state)
{
    BC_UNUSED(state);

    g_mock_detect_returns = true;
    bc_core_zero(&g_mock_features, sizeof(g_mock_features));
    bc_core_equal_dispatch_init();

    unsigned char* a = aligned_alloc(64, 128);
    assert_non_null(a);
    unsigned char* b = aligned_alloc(64, 128);
    assert_non_null(b);
    bc_core_fill(a, 128, (unsigned char)0xCC);
    bc_core_fill(b, 128, (unsigned char)0xCC);
    b[64] = 0xDD;

    bool out_equal = true;
    bool success = bc_core_equal(a, b, 128, &out_equal);

    assert_true(success);
    assert_false(out_equal);

    free(a);
    free(b);
}

static void test_equal_avx2_differ_in_tail_35(void** state)
{
    BC_UNUSED(state);

    g_mock_detect_returns = true;
    bc_core_zero(&g_mock_features, sizeof(g_mock_features));
    bc_core_equal_dispatch_init();

    unsigned char* a = aligned_alloc(64, 64);
    assert_non_null(a);
    unsigned char* b = aligned_alloc(64, 64);
    assert_non_null(b);
    bc_core_fill(a, 64, (unsigned char)0xAA);
    bc_core_fill(b, 64, (unsigned char)0xAA);
    b[33] = 0xBB;

    bool out_equal = true;
    bool success = bc_core_equal(a, b, 35, &out_equal);

    assert_true(success);
    assert_false(out_equal);

    free(a);
    free(b);
}

static void test_equal_avx2_identical_48(void** state)
{
    BC_UNUSED(state);

    g_mock_detect_returns = true;
    bc_core_zero(&g_mock_features, sizeof(g_mock_features));
    bc_core_equal_dispatch_init();

    unsigned char* a = aligned_alloc(64, 64);
    assert_non_null(a);
    unsigned char* b = aligned_alloc(64, 64);
    assert_non_null(b);
    bc_core_fill(a, 48, (unsigned char)0x77);
    bc_core_fill(b, 48, (unsigned char)0x77);

    bool out_equal = false;
    bool success = bc_core_equal(a, b, 48, &out_equal);

    assert_true(success);
    assert_true(out_equal);

    free(a);
    free(b);
}

static void test_equal_dispatch_detect_failure_avx2_fallback(void** state)
{
    BC_UNUSED(state);

    g_mock_detect_returns = false;
    bc_core_equal_dispatch_init();

    const unsigned char a[4] = {0xAA, 0xBB, 0xCC, 0xDD};
    const unsigned char b[4] = {0xAA, 0xBB, 0xCC, 0xDD};
    bool eq = false;
    bool success = bc_core_equal(a, b, 4, &eq);

    assert_true(success);
    assert_true(eq);

    g_mock_detect_returns = true;
    bc_core_zero(&g_mock_features, sizeof(g_mock_features));
    bc_core_equal_dispatch_init();
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_equal_avx2_len_0),
        cmocka_unit_test(test_equal_avx2_identical_small),
        cmocka_unit_test(test_equal_avx2_identical_64),
        cmocka_unit_test(test_equal_avx2_identical_257),
        cmocka_unit_test(test_equal_avx2_differ_first_byte),
        cmocka_unit_test(test_equal_avx2_differ_last_byte),
        cmocka_unit_test(test_equal_avx2_differ_at_boundary_63),
        cmocka_unit_test(test_equal_avx2_differ_at_boundary_64),
        cmocka_unit_test(test_equal_avx2_differ_in_tail_35),
        cmocka_unit_test(test_equal_avx2_identical_48),
        cmocka_unit_test(test_equal_dispatch_detect_failure_avx2_fallback),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
