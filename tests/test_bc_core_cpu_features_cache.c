// SPDX-License-Identifier: MIT

#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <cmocka.h>

#include "bc_core.h"
#include "bc_core_cpu_features_internal.h"

static void test_cpu_features_detect_second_call_returns_cached(void** state)
{
    BC_UNUSED(state);

    bc_core_cpu_features_t first = {0};
    bool success_first = bc_core_cpu_features_detect(&first);

    bc_core_cpu_features_t second = {0};
    bool success_second = bc_core_cpu_features_detect(&second);

    assert_true(success_first);
    assert_true(success_second);
    assert_memory_equal(&first, &second, sizeof(bc_core_cpu_features_t));
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_cpu_features_detect_second_call_returns_cached),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
