// SPDX-License-Identifier: MIT

#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <cmocka.h>

#include "bc_core.h"

#include <stdalign.h>
#include <stdlib.h>
#include <string.h>

static const bc_core_cache_policy_t all_policies[3] = {
    BC_CORE_CACHE_POLICY_DEFAULT,
    BC_CORE_CACHE_POLICY_CACHED,
    BC_CORE_CACHE_POLICY_STREAMING,
};

/* ---- copy_with_policy ---- */

static void test_copy_with_policy_zero_length(void** state)
{
    BC_UNUSED(state);
    for (size_t p = 0; p < 3; p++) {
        assert_true(bc_core_copy_with_policy(NULL, NULL, 0, all_policies[p]));
    }
}

static void test_copy_with_policy_small_under_32(void** state)
{
    BC_UNUSED(state);
    unsigned char src[20];
    unsigned char dst[20];
    for (size_t i = 0; i < sizeof(src); i++) {
        src[i] = (unsigned char)(i + 1);
    }
    for (size_t p = 0; p < 3; p++) {
        memset(dst, 0, sizeof(dst));
        assert_true(bc_core_copy_with_policy(dst, src, sizeof(src), all_policies[p]));
        assert_memory_equal(dst, src, sizeof(src));
    }
}

static void test_copy_with_policy_32_to_128(void** state)
{
    BC_UNUSED(state);
    alignas(64) unsigned char src[128];
    alignas(64) unsigned char dst[128];
    for (size_t i = 0; i < sizeof(src); i++) {
        src[i] = (unsigned char)((i * 7) & 0xFF);
    }
    const size_t sizes[5] = {32, 48, 64, 96, 128};
    for (size_t s = 0; s < 5; s++) {
        for (size_t p = 0; p < 3; p++) {
            memset(dst, 0, sizeof(dst));
            assert_true(bc_core_copy_with_policy(dst, src, sizes[s], all_policies[p]));
            assert_memory_equal(dst, src, sizes[s]);
        }
    }
}

static void test_copy_with_policy_over_128(void** state)
{
    BC_UNUSED(state);
    size_t len = 4096;
    unsigned char* src = aligned_alloc(64, len);
    unsigned char* dst = aligned_alloc(64, len);
    assert_non_null(src);
    assert_non_null(dst);
    for (size_t i = 0; i < len; i++) {
        src[i] = (unsigned char)((i * 13) & 0xFF);
    }
    for (size_t p = 0; p < 3; p++) {
        memset(dst, 0, len);
        assert_true(bc_core_copy_with_policy(dst, src, len, all_policies[p]));
        assert_memory_equal(dst, src, len);
    }
    free(src);
    free(dst);
}

/* ---- fill_with_policy ---- */

static void test_fill_with_policy_zero_length(void** state)
{
    BC_UNUSED(state);
    for (size_t p = 0; p < 3; p++) {
        assert_true(bc_core_fill_with_policy(NULL, 0, 0, all_policies[p]));
    }
}

static void test_fill_with_policy_small_under_32(void** state)
{
    BC_UNUSED(state);
    unsigned char dst[20];
    for (size_t p = 0; p < 3; p++) {
        memset(dst, 0, sizeof(dst));
        assert_true(bc_core_fill_with_policy(dst, sizeof(dst), (unsigned char)0x5A, all_policies[p]));
        for (size_t i = 0; i < sizeof(dst); i++) {
            assert_int_equal(dst[i], 0x5A);
        }
    }
}

static void test_fill_with_policy_32_to_128(void** state)
{
    BC_UNUSED(state);
    alignas(64) unsigned char dst[128];
    const size_t sizes[5] = {32, 48, 64, 96, 128};
    for (size_t s = 0; s < 5; s++) {
        for (size_t p = 0; p < 3; p++) {
            memset(dst, 0, sizeof(dst));
            assert_true(bc_core_fill_with_policy(dst, sizes[s], (unsigned char)0xA3, all_policies[p]));
            for (size_t i = 0; i < sizes[s]; i++) {
                assert_int_equal(dst[i], 0xA3);
            }
        }
    }
}

static void test_fill_with_policy_over_128(void** state)
{
    BC_UNUSED(state);
    size_t len = 4096;
    unsigned char* dst = aligned_alloc(64, len);
    assert_non_null(dst);
    for (size_t p = 0; p < 3; p++) {
        memset(dst, 0, len);
        assert_true(bc_core_fill_with_policy(dst, len, (unsigned char)0xCC, all_policies[p]));
        for (size_t i = 0; i < len; i++) {
            assert_int_equal(dst[i], 0xCC);
        }
    }
    free(dst);
}

/* ---- zero_with_policy ---- */

static void test_zero_with_policy_zero_length(void** state)
{
    BC_UNUSED(state);
    for (size_t p = 0; p < 3; p++) {
        assert_true(bc_core_zero_with_policy(NULL, 0, all_policies[p]));
    }
}

static void test_zero_with_policy_small_under_32(void** state)
{
    BC_UNUSED(state);
    unsigned char dst[20];
    for (size_t p = 0; p < 3; p++) {
        memset(dst, 0xFF, sizeof(dst));
        assert_true(bc_core_zero_with_policy(dst, sizeof(dst), all_policies[p]));
        for (size_t i = 0; i < sizeof(dst); i++) {
            assert_int_equal(dst[i], 0);
        }
    }
}

static void test_zero_with_policy_32_to_128(void** state)
{
    BC_UNUSED(state);
    alignas(64) unsigned char dst[128];
    const size_t sizes[5] = {32, 48, 64, 96, 128};
    for (size_t s = 0; s < 5; s++) {
        for (size_t p = 0; p < 3; p++) {
            memset(dst, 0xFF, sizeof(dst));
            assert_true(bc_core_zero_with_policy(dst, sizes[s], all_policies[p]));
            for (size_t i = 0; i < sizes[s]; i++) {
                assert_int_equal(dst[i], 0);
            }
        }
    }
}

static void test_zero_with_policy_over_128(void** state)
{
    BC_UNUSED(state);
    size_t len = 4096;
    unsigned char* dst = aligned_alloc(64, len);
    assert_non_null(dst);
    for (size_t p = 0; p < 3; p++) {
        memset(dst, 0xFF, len);
        assert_true(bc_core_zero_with_policy(dst, len, all_policies[p]));
        for (size_t i = 0; i < len; i++) {
            assert_int_equal(dst[i], 0);
        }
    }
    free(dst);
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_copy_with_policy_zero_length), cmocka_unit_test(test_copy_with_policy_small_under_32),
        cmocka_unit_test(test_copy_with_policy_32_to_128),   cmocka_unit_test(test_copy_with_policy_over_128),
        cmocka_unit_test(test_fill_with_policy_zero_length), cmocka_unit_test(test_fill_with_policy_small_under_32),
        cmocka_unit_test(test_fill_with_policy_32_to_128),   cmocka_unit_test(test_fill_with_policy_over_128),
        cmocka_unit_test(test_zero_with_policy_zero_length), cmocka_unit_test(test_zero_with_policy_small_under_32),
        cmocka_unit_test(test_zero_with_policy_32_to_128),   cmocka_unit_test(test_zero_with_policy_over_128),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
