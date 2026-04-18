// SPDX-License-Identifier: MIT

#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <cmocka.h>

#include "bc_core.h"

#include <unistd.h>

long __real_sysconf(int name);

static long g_mock_sysconf_return = -1;
static bool g_mock_sysconf_active = false;

long __wrap_sysconf(int name)
{
    if (g_mock_sysconf_active) {
        BC_UNUSED(name);
        return g_mock_sysconf_return;
    }
    return __real_sysconf(name);
}

static void test_cache_line_size_sysconf_failure_returns_false(void** state)
{
    BC_UNUSED(state);

    g_mock_sysconf_active = true;
    g_mock_sysconf_return = -1;

    size_t size = 0;
    bool success = bc_core_cache_line_size(&size);

    assert_false(success);

    g_mock_sysconf_active = false;
}

static void test_l1d_cache_size_sysconf_failure_returns_false(void** state)
{
    BC_UNUSED(state);

    g_mock_sysconf_active = true;
    g_mock_sysconf_return = -1;

    size_t size = 0;
    bool success = bc_core_l1d_cache_size(&size);

    assert_false(success);

    g_mock_sysconf_active = false;
}

static void test_l2_cache_size_sysconf_failure_returns_false(void** state)
{
    BC_UNUSED(state);

    g_mock_sysconf_active = true;
    g_mock_sysconf_return = -1;

    size_t size = 0;
    bool success = bc_core_l2_cache_size(&size);

    assert_false(success);

    g_mock_sysconf_active = false;
}

static void test_l3_cache_size_sysconf_failure_returns_false(void** state)
{
    BC_UNUSED(state);

    g_mock_sysconf_active = true;
    g_mock_sysconf_return = -1;

    size_t size = 0;
    bool success = bc_core_l3_cache_size(&size);

    assert_false(success);

    g_mock_sysconf_active = false;
}

static void test_cache_line_size_sysconf_returns_zero_returns_false(void** state)
{
    BC_UNUSED(state);

    g_mock_sysconf_active = true;
    g_mock_sysconf_return = 0;

    size_t size = 0;
    bool success = bc_core_cache_line_size(&size);

    assert_false(success);

    g_mock_sysconf_active = false;
}

static void test_l2_cache_size_real_sysconf_returns_positive(void** state)
{
    BC_UNUSED(state);

    g_mock_sysconf_active = false;

    size_t size = 0;
    bool success = bc_core_l2_cache_size(&size);

    assert_true(success);
    assert_true(size > 0);
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_cache_line_size_sysconf_failure_returns_false),
        cmocka_unit_test(test_l1d_cache_size_sysconf_failure_returns_false),
        cmocka_unit_test(test_l2_cache_size_sysconf_failure_returns_false),
        cmocka_unit_test(test_l3_cache_size_sysconf_failure_returns_false),
        cmocka_unit_test(test_cache_line_size_sysconf_returns_zero_returns_false),
        cmocka_unit_test(test_l2_cache_size_real_sysconf_returns_positive),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
