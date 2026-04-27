// SPDX-License-Identifier: MIT

#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <cmocka.h>

#include "bc_core.h"

#include <stdalign.h>
#include <stdint.h>

// cppcheck-suppress-begin unusedStructMember
typedef struct {
    BC_FALSE_SHARING_GUARD(uint64_t, producer_counter);
    BC_FALSE_SHARING_GUARD(uint64_t, consumer_counter);
} bc_test_guarded_pair_t;

_Static_assert(sizeof(bc_test_guarded_pair_t) == 2 * BC_CACHE_LINE_SIZE,
               "BC_FALSE_SHARING_GUARD must occupy a full cache line per guarded value");

_Static_assert(offsetof(bc_test_guarded_pair_t, consumer_counter) == BC_CACHE_LINE_SIZE,
               "BC_FALSE_SHARING_GUARD must place the second value on its own cache line");

_Static_assert(alignof(bc_test_guarded_pair_t) >= BC_CACHE_LINE_SIZE,
               "BC_FALSE_SHARING_GUARD must enforce cache-line alignment of the enclosing struct");

typedef struct {
    BC_FALSE_SHARING_GUARD(uint32_t, lone_value);
} bc_test_guarded_single_t;
// cppcheck-suppress-end unusedStructMember

_Static_assert(sizeof(bc_test_guarded_single_t) == BC_CACHE_LINE_SIZE,
               "BC_FALSE_SHARING_GUARD must pad a single value to a full cache line");

static void test_guarded_pair_struct_size(void** state)
{
    BC_UNUSED(state);

    assert_int_equal(sizeof(bc_test_guarded_pair_t), 2 * BC_CACHE_LINE_SIZE);
}

static void test_guarded_pair_second_field_offset(void** state)
{
    BC_UNUSED(state);

    assert_int_equal(offsetof(bc_test_guarded_pair_t, consumer_counter), BC_CACHE_LINE_SIZE);
}

static void test_guarded_single_struct_size(void** state)
{
    BC_UNUSED(state);

    assert_int_equal(sizeof(bc_test_guarded_single_t), BC_CACHE_LINE_SIZE);
}

static void test_guarded_pair_struct_alignment(void** state)
{
    BC_UNUSED(state);

    assert_true(alignof(bc_test_guarded_pair_t) >= BC_CACHE_LINE_SIZE);
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_guarded_pair_struct_size),
        cmocka_unit_test(test_guarded_pair_second_field_offset),
        cmocka_unit_test(test_guarded_single_struct_size),
        cmocka_unit_test(test_guarded_pair_struct_alignment),
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}
