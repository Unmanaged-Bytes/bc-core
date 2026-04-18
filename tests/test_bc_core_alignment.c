// SPDX-License-Identifier: MIT

#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <cmocka.h>

#include "bc_core.h"

#include <stdalign.h>

static void test_sha256_context_state_is_16byte_aligned(void** state)
{
    BC_UNUSED(state);

    assert_true(alignof(bc_core_sha256_context_t) >= 16);
    assert_true(offsetof(bc_core_sha256_context_t, state) % 16 == 0);
}

static void test_cache_line_macros_defined(void** state)
{
    BC_UNUSED(state);

    assert_true(BC_CACHE_LINE_SIZE == 64);
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_sha256_context_state_is_16byte_aligned),
        cmocka_unit_test(test_cache_line_macros_defined),
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}
