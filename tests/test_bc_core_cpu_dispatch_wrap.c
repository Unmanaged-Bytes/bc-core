// SPDX-License-Identifier: MIT

#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <cmocka.h>

#include "bc_core.h"
#include "test_bc_core_dispatch_mock.h"

void bc_core_cpu_dispatch_init(void);

static void test_chunk_size_returns_32(void** state)
{
    BC_UNUSED(state);

    g_mock_detect_returns = true;
    bc_core_zero(&g_mock_features, sizeof(g_mock_features));

    bc_core_cpu_dispatch_init();

    size_t size = 0;
    bool success = bc_core_chunk_size(&size);

    assert_true(success);
    assert_int_equal(size, 32);
}

static void test_chunk_size_defaults_to_32_on_detect_failure(void** state)
{
    BC_UNUSED(state);

    g_mock_detect_returns = false;

    bc_core_cpu_dispatch_init();

    size_t size = 0;
    bool success = bc_core_chunk_size(&size);

    assert_true(success);
    assert_int_equal(size, 32);
}

static void test_preferred_alignment_always_64(void** state)
{
    BC_UNUSED(state);

    g_mock_detect_returns = true;
    bc_core_zero(&g_mock_features, sizeof(g_mock_features));

    bc_core_cpu_dispatch_init();

    size_t alignment = 0;
    bool success = bc_core_preferred_alignment(&alignment);

    assert_true(success);
    assert_int_equal(alignment, 64);
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_chunk_size_returns_32),
        cmocka_unit_test(test_chunk_size_defaults_to_32_on_detect_failure),
        cmocka_unit_test(test_preferred_alignment_always_64),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
