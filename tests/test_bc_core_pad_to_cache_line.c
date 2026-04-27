// SPDX-License-Identifier: MIT

#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <cmocka.h>

#include "bc_core.h"

// cppcheck-suppress-begin unusedStructMember
typedef struct {
    int hot_counter;
    BC_PAD_TO_CACHE_LINE(sizeof(int));
    int next_field;
} bc_test_padded_struct_t;

typedef struct {
    char data[40];
    BC_PAD_TO_CACHE_LINE(40);
    char tail;
} bc_test_padded_40_t;

typedef struct {
    char head;
    BC_PAD_TO_CACHE_LINE(1);
    char tail;
} bc_test_padded_1_t;
// cppcheck-suppress-end unusedStructMember

_Static_assert(offsetof(bc_test_padded_struct_t, next_field) == BC_CACHE_LINE_SIZE,
               "BC_PAD_TO_CACHE_LINE must push next_field onto the next cache line");

_Static_assert(offsetof(bc_test_padded_40_t, tail) == BC_CACHE_LINE_SIZE, "BC_PAD_TO_CACHE_LINE must align tail at 64 when used_bytes=40");

_Static_assert(offsetof(bc_test_padded_1_t, tail) == BC_CACHE_LINE_SIZE, "BC_PAD_TO_CACHE_LINE must align tail at 64 when used_bytes=1");

static void test_padded_struct_next_field_on_fresh_cache_line(void** state)
{
    BC_UNUSED(state);

    assert_int_equal(offsetof(bc_test_padded_struct_t, next_field), BC_CACHE_LINE_SIZE);
}

static void test_padded_struct_with_40_used_bytes(void** state)
{
    BC_UNUSED(state);

    assert_int_equal(offsetof(bc_test_padded_40_t, tail), BC_CACHE_LINE_SIZE);
}

static void test_padded_struct_with_1_used_byte(void** state)
{
    BC_UNUSED(state);

    assert_int_equal(offsetof(bc_test_padded_1_t, tail), BC_CACHE_LINE_SIZE);
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_padded_struct_next_field_on_fresh_cache_line),
        cmocka_unit_test(test_padded_struct_with_40_used_bytes),
        cmocka_unit_test(test_padded_struct_with_1_used_byte),
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}
