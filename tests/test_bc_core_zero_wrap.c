// SPDX-License-Identifier: MIT

#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <cmocka.h>

#include "bc_core.h"

#include "bc_core_cache_sizes_internal.h"

#include <stdlib.h>
#include <unistd.h>

long __real_sysconf(int name);

static bool g_sysconf_fail = true;

long __wrap_sysconf(int name)
{
    if (g_sysconf_fail) {
        BC_UNUSED(name);
        return -1;
    }
    return __real_sysconf(name);
}

extern void bc_core_cache_sizes_init(void);

static void test_cache_sizes_init_sysconf_failure_leaves_defaults(void** state)
{
    BC_UNUSED(state);

    g_sysconf_fail = true;
    bc_core_cache_sizes_init();

    size_t l1d = bc_core_cached_l1d_cache_size();
    size_t l2 = bc_core_cached_l2_cache_size();
    size_t l3 = bc_core_cached_l3_cache_size();

    assert_int_equal(l1d, 0);
    assert_int_equal(l2, 0);
    assert_int_equal(l3, BC_CORE_L3_CACHE_SIZE_DEFAULT);

    g_sysconf_fail = false;
}

static void test_zero_sysconf_failure_l2_returns_zero_default(void** state)
{
    BC_UNUSED(state);

    size_t l2_size = bc_core_cached_l2_cache_size();

    assert_int_equal(l2_size, 0);
}

static void test_zero_sysconf_failure_l3_returns_16mb_default(void** state)
{
    BC_UNUSED(state);

    size_t l3_size = bc_core_cached_l3_cache_size();

    assert_int_equal(l3_size, BC_CORE_L3_CACHE_SIZE_DEFAULT);
}

static void test_zero_still_works_with_default_cache_sizes(void** state)
{
    BC_UNUSED(state);

    size_t buffer_size = 256;
    unsigned char buffer[256];
    bc_core_fill(buffer, buffer_size, (unsigned char)0xAA);

    bool result = bc_core_zero(buffer, buffer_size);

    assert_true(result);
    for (size_t i = 0; i < buffer_size; i++) {
        assert_int_equal(buffer[i], 0);
    }
}

static void test_zero_large_buffer_uses_nt_path_with_default_l3(void** state)
{
    BC_UNUSED(state);

    size_t buffer_size = 32 * 1024 * 1024;
    unsigned char* buffer = malloc(buffer_size);
    assert_non_null(buffer);

    bc_core_fill(buffer, buffer_size, (unsigned char)0xBB);

    bool result = bc_core_zero(buffer, buffer_size);

    assert_true(result);
    for (size_t i = 0; i < buffer_size; i++) {
        assert_int_equal(buffer[i], 0);
    }

    free(buffer);
}

static void test_cache_sizes_init_real_sysconf_updates_sizes(void** state)
{
    BC_UNUSED(state);

    g_sysconf_fail = false;
    bc_core_cache_sizes_init();

    size_t l2 = bc_core_cached_l2_cache_size();
    assert_true(l2 > 0);

    g_sysconf_fail = true;
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_cache_sizes_init_sysconf_failure_leaves_defaults),
        cmocka_unit_test(test_zero_sysconf_failure_l2_returns_zero_default),
        cmocka_unit_test(test_zero_sysconf_failure_l3_returns_16mb_default),
        cmocka_unit_test(test_zero_still_works_with_default_cache_sizes),
        cmocka_unit_test(test_zero_large_buffer_uses_nt_path_with_default_l3),
        cmocka_unit_test(test_cache_sizes_init_real_sysconf_updates_sizes),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
