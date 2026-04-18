// SPDX-License-Identifier: MIT

#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <cmocka.h>

#include "bc_core.h"
#include "test_bc_core_dispatch_mock.h"

#include <stdlib.h>

static bool g_l2_fail = false;
static size_t g_mock_l2_cache_size = 256;

bool __wrap_bc_core_l2_cache_size(size_t* out_size)
{
    if (g_l2_fail) {
        BC_UNUSED(out_size);
        return false;
    }
    *out_size = g_mock_l2_cache_size;
    return true;
}

extern void bc_core_copy_dispatch_init(void);

static void test_copy_avx2_path_len_0(void** state)
{
    BC_UNUSED(state);

    g_mock_detect_returns = true;
    bc_core_zero(&g_mock_features, sizeof(g_mock_features));

    bc_core_copy_dispatch_init();

    unsigned char* src = aligned_alloc(64, 64);
    unsigned char* dst = aligned_alloc(64, 64);
    assert_non_null(src);
    assert_non_null(dst);
    bc_core_fill(src, 64, (unsigned char)0x5A);
    bc_core_fill(dst, 64, (unsigned char)0x00);

    bool result = bc_core_copy(dst, src, 0);

    assert_true(result);

    free(src);
    free(dst);
}

static void test_copy_avx2_path_small(void** state)
{
    BC_UNUSED(state);

    g_mock_detect_returns = true;
    bc_core_zero(&g_mock_features, sizeof(g_mock_features));

    bc_core_copy_dispatch_init();

    size_t len = 63;
    unsigned char* src = aligned_alloc(64, 64);
    unsigned char* dst = aligned_alloc(64, 64);
    assert_non_null(src);
    assert_non_null(dst);
    for (size_t i = 0; i < len; i++) {
        src[i] = (unsigned char)(i & 0xFF);
    }
    bc_core_fill(dst, 64, (unsigned char)0x00);

    bool result = bc_core_copy(dst, src, len);

    assert_true(result);
    assert_memory_equal(dst, src, len);

    free(src);
    free(dst);
}

static void test_copy_avx2_path_exact_64(void** state)
{
    BC_UNUSED(state);

    g_mock_detect_returns = true;
    bc_core_zero(&g_mock_features, sizeof(g_mock_features));

    bc_core_copy_dispatch_init();

    size_t len = 64;
    unsigned char* src = aligned_alloc(64, len);
    unsigned char* dst = aligned_alloc(64, len);
    assert_non_null(src);
    assert_non_null(dst);
    for (size_t i = 0; i < len; i++) {
        src[i] = (unsigned char)(i & 0xFF);
    }
    bc_core_fill(dst, len, (unsigned char)0x00);

    bool result = bc_core_copy(dst, src, len);

    assert_true(result);
    assert_memory_equal(dst, src, len);

    free(src);
    free(dst);
}

static void test_copy_avx2_path_65(void** state)
{
    BC_UNUSED(state);

    g_mock_detect_returns = true;
    bc_core_zero(&g_mock_features, sizeof(g_mock_features));

    bc_core_copy_dispatch_init();

    size_t len = 65;
    unsigned char* src = aligned_alloc(64, 128);
    unsigned char* dst = aligned_alloc(64, 128);
    assert_non_null(src);
    assert_non_null(dst);
    for (size_t i = 0; i < len; i++) {
        src[i] = (unsigned char)(i & 0xFF);
    }
    bc_core_fill(dst, 128, (unsigned char)0x00);

    bool result = bc_core_copy(dst, src, len);

    assert_true(result);
    assert_memory_equal(dst, src, len);

    free(src);
    free(dst);
}

static void test_copy_avx2_path_256(void** state)
{
    BC_UNUSED(state);

    g_mock_detect_returns = true;
    bc_core_zero(&g_mock_features, sizeof(g_mock_features));

    bc_core_copy_dispatch_init();

    size_t len = 256;
    unsigned char* src = aligned_alloc(64, len);
    unsigned char* dst = aligned_alloc(64, len);
    assert_non_null(src);
    assert_non_null(dst);
    for (size_t i = 0; i < len; i++) {
        src[i] = (unsigned char)(i & 0xFF);
    }
    bc_core_fill(dst, len, (unsigned char)0x00);

    bool result = bc_core_copy(dst, src, len);

    assert_true(result);
    assert_memory_equal(dst, src, len);

    free(src);
    free(dst);
}

static void test_copy_avx2_path_large(void** state)
{
    BC_UNUSED(state);

    g_mock_detect_returns = true;
    bc_core_zero(&g_mock_features, sizeof(g_mock_features));

    bc_core_copy_dispatch_init();

    size_t len = 257;
    unsigned char* src = aligned_alloc(64, 320);
    unsigned char* dst = aligned_alloc(64, 320);
    assert_non_null(src);
    assert_non_null(dst);
    for (size_t i = 0; i < len; i++) {
        src[i] = (unsigned char)(i & 0xFF);
    }
    bc_core_fill(dst, 320, (unsigned char)0x00);

    bool result = bc_core_copy(dst, src, len);

    assert_true(result);
    assert_memory_equal(dst, src, len);

    free(src);
    free(dst);
}

static void test_copy_no_overlap(void** state)
{
    BC_UNUSED(state);

    g_mock_detect_returns = true;
    bc_core_zero(&g_mock_features, sizeof(g_mock_features));

    bc_core_copy_dispatch_init();

    size_t len = 128;
    unsigned char* src = aligned_alloc(64, len);
    unsigned char* dst = aligned_alloc(64, len);
    assert_non_null(src);
    assert_non_null(dst);
    for (size_t i = 0; i < len; i++) {
        src[i] = (unsigned char)(i & 0xFF);
    }
    bc_core_fill(dst, len, (unsigned char)0x00);

    bool result = bc_core_copy(dst, src, len);

    assert_true(result);
    assert_memory_equal(dst, src, len);

    free(src);
    free(dst);
}

static void test_copy_avx2_path_len_1(void** state)
{
    BC_UNUSED(state);

    g_mock_detect_returns = true;
    bc_core_zero(&g_mock_features, sizeof(g_mock_features));

    bc_core_copy_dispatch_init();

    size_t len = 1;
    unsigned char* src = aligned_alloc(64, 64);
    unsigned char* dst = aligned_alloc(64, 64);
    assert_non_null(src);
    assert_non_null(dst);
    src[0] = 0xAB;
    bc_core_fill(dst, 64, (unsigned char)0x00);

    bool result = bc_core_copy(dst, src, len);

    assert_true(result);
    assert_memory_equal(dst, src, len);

    free(src);
    free(dst);
}

static void test_copy_avx2_path_len_16(void** state)
{
    BC_UNUSED(state);

    g_mock_detect_returns = true;
    bc_core_zero(&g_mock_features, sizeof(g_mock_features));

    bc_core_copy_dispatch_init();

    size_t len = 16;
    unsigned char* src = aligned_alloc(64, 64);
    unsigned char* dst = aligned_alloc(64, 64);
    assert_non_null(src);
    assert_non_null(dst);
    for (size_t i = 0; i < len; i++) {
        src[i] = (unsigned char)(i & 0xFF);
    }
    bc_core_fill(dst, 64, (unsigned char)0x00);

    bool result = bc_core_copy(dst, src, len);

    assert_true(result);
    assert_memory_equal(dst, src, len);

    free(src);
    free(dst);
}

static void test_copy_avx2_path_len_31(void** state)
{
    BC_UNUSED(state);

    g_mock_detect_returns = true;
    bc_core_zero(&g_mock_features, sizeof(g_mock_features));

    bc_core_copy_dispatch_init();

    size_t len = 31;
    unsigned char* src = aligned_alloc(64, 64);
    unsigned char* dst = aligned_alloc(64, 64);
    assert_non_null(src);
    assert_non_null(dst);
    for (size_t i = 0; i < len; i++) {
        src[i] = (unsigned char)(i & 0xFF);
    }
    bc_core_fill(dst, 64, (unsigned char)0x00);

    bool result = bc_core_copy(dst, src, len);

    assert_true(result);
    assert_memory_equal(dst, src, len);

    free(src);
    free(dst);
}

static void test_copy_avx2_nt_store_path(void** state)
{
    BC_UNUSED(state);

    g_mock_detect_returns = true;
    bc_core_zero(&g_mock_features, sizeof(g_mock_features));

    bc_core_copy_dispatch_init();

    size_t len = 512;
    unsigned char* src = aligned_alloc(64, len);
    unsigned char* dst = aligned_alloc(64, len);
    assert_non_null(src);
    assert_non_null(dst);
    for (size_t i = 0; i < len; i++) {
        src[i] = (unsigned char)(i & 0xFF);
    }
    bc_core_fill(dst, len, (unsigned char)0x00);

    bool result = bc_core_copy(dst, src, len);

    assert_true(result);
    assert_memory_equal(dst, src, len);

    free(src);
    free(dst);
}

static void test_copy_avx2_path_len_32(void** state)
{
    BC_UNUSED(state);

    g_mock_detect_returns = true;
    bc_core_zero(&g_mock_features, sizeof(g_mock_features));
    g_l2_fail = false;
    g_mock_l2_cache_size = 256;

    bc_core_copy_dispatch_init();

    size_t len = 32;
    unsigned char* src = aligned_alloc(64, 64);
    unsigned char* dst = aligned_alloc(64, 64);
    assert_non_null(src);
    assert_non_null(dst);
    for (size_t i = 0; i < len; i++) {
        src[i] = (unsigned char)(i & 0xFF);
    }
    bc_core_fill(dst, 64, (unsigned char)0x00);

    bool result = bc_core_copy(dst, src, len);

    assert_true(result);
    assert_memory_equal(dst, src, len);

    free(src);
    free(dst);
}

static void test_copy_avx2_l2_zero_large_buffer(void** state)
{
    BC_UNUSED(state);

    g_mock_detect_returns = true;
    bc_core_zero(&g_mock_features, sizeof(g_mock_features));
    g_l2_fail = true;

    bc_core_copy_dispatch_init();

    size_t len = 512;
    unsigned char* src = aligned_alloc(64, len);
    unsigned char* dst = aligned_alloc(64, len);
    assert_non_null(src);
    assert_non_null(dst);
    for (size_t i = 0; i < len; i++) {
        src[i] = (unsigned char)(i & 0xFF);
    }
    bc_core_fill(dst, len, (unsigned char)0x00);

    bool result = bc_core_copy(dst, src, len);

    assert_true(result);
    assert_memory_equal(dst, src, len);

    free(src);
    free(dst);

    g_l2_fail = false;
    g_mock_l2_cache_size = 256;
}

static void test_copy_dispatch_detect_failure_avx2_fallback(void** state)
{
    BC_UNUSED(state);

    g_mock_detect_returns = false;
    g_l2_fail = false;
    g_mock_l2_cache_size = 256;

    bc_core_copy_dispatch_init();

    size_t len = 64;
    unsigned char* src = aligned_alloc(64, len);
    unsigned char* dst = aligned_alloc(64, len);
    assert_non_null(src);
    assert_non_null(dst);
    for (size_t i = 0; i < len; i++) {
        src[i] = (unsigned char)(i & 0xFF);
    }
    bc_core_fill(dst, len, (unsigned char)0x00);

    bool result = bc_core_copy(dst, src, len);

    assert_true(result);
    assert_memory_equal(dst, src, len);

    free(src);
    free(dst);

    g_mock_detect_returns = true;
}

static void test_copy_l2_cache_size_wrap_returns_false_when_fail(void** state)
{
    BC_UNUSED(state);

    g_l2_fail = true;

    size_t size = 99;
    bool result = bc_core_l2_cache_size(&size);

    assert_false(result);

    g_l2_fail = false;
    g_mock_l2_cache_size = 256;
}

int main(void)
{
    const struct CMUnitTest tests[] = {cmocka_unit_test(test_copy_avx2_path_len_0),
                                       cmocka_unit_test(test_copy_avx2_path_len_1),
                                       cmocka_unit_test(test_copy_avx2_path_len_16),
                                       cmocka_unit_test(test_copy_avx2_path_len_31),
                                       cmocka_unit_test(test_copy_avx2_path_small),
                                       cmocka_unit_test(test_copy_avx2_path_exact_64),
                                       cmocka_unit_test(test_copy_avx2_path_65),
                                       cmocka_unit_test(test_copy_avx2_path_256),
                                       cmocka_unit_test(test_copy_avx2_path_large),
                                       cmocka_unit_test(test_copy_no_overlap),
                                       cmocka_unit_test(test_copy_avx2_nt_store_path),
                                       cmocka_unit_test(test_copy_avx2_path_len_32),
                                       cmocka_unit_test(test_copy_avx2_l2_zero_large_buffer),
                                       cmocka_unit_test(test_copy_dispatch_detect_failure_avx2_fallback),
                                       cmocka_unit_test(test_copy_l2_cache_size_wrap_returns_false_when_fail)};

    return cmocka_run_group_tests(tests, NULL, NULL);
}
