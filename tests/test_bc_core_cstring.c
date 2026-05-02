// SPDX-License-Identifier: MIT

#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <cmocka.h>

#include "bc_core.h"

#include <stdbool.h>

static void test_cstring_equal_identical(void** state)
{
    BC_UNUSED(state);
    bool equal = false;
    assert_true(bc_core_cstring_equal("hello", "hello", &equal));
    assert_true(equal);
}

static void test_cstring_equal_different(void** state)
{
    BC_UNUSED(state);
    bool equal = true;
    assert_true(bc_core_cstring_equal("hello", "world", &equal));
    assert_false(equal);
}

static void test_cstring_equal_different_lengths(void** state)
{
    BC_UNUSED(state);
    bool equal = true;
    assert_true(bc_core_cstring_equal("hello", "hello!", &equal));
    assert_false(equal);
}

static void test_cstring_equal_empty_both(void** state)
{
    BC_UNUSED(state);
    bool equal = false;
    assert_true(bc_core_cstring_equal("", "", &equal));
    assert_true(equal);
}

static void test_cstring_equal_empty_one_side(void** state)
{
    BC_UNUSED(state);
    bool equal = true;
    assert_true(bc_core_cstring_equal("", "x", &equal));
    assert_false(equal);

    bool equal_reverse = true;
    assert_true(bc_core_cstring_equal("x", "", &equal_reverse));
    assert_false(equal_reverse);
}

static void test_cstring_compare_lex_order(void** state)
{
    BC_UNUSED(state);

    int order = 0;
    assert_true(bc_core_cstring_compare("abc", "abd", &order));
    assert_true(order < 0);

    order = 0;
    assert_true(bc_core_cstring_compare("abd", "abc", &order));
    assert_true(order > 0);

    order = 99;
    assert_true(bc_core_cstring_compare("abc", "abc", &order));
    assert_int_equal(order, 0);

    order = 99;
    assert_true(bc_core_cstring_compare("abc", "abcd", &order));
    assert_true(order < 0);

    order = 99;
    assert_true(bc_core_cstring_compare("abcd", "abc", &order));
    assert_true(order > 0);
}

static void test_cstring_compare_empty(void** state)
{
    BC_UNUSED(state);

    int order = 99;
    assert_true(bc_core_cstring_compare("", "", &order));
    assert_int_equal(order, 0);

    order = 99;
    assert_true(bc_core_cstring_compare("", "x", &order));
    assert_true(order < 0);

    order = 99;
    assert_true(bc_core_cstring_compare("x", "", &order));
    assert_true(order > 0);
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_cstring_equal_identical),         cmocka_unit_test(test_cstring_equal_different),
        cmocka_unit_test(test_cstring_equal_different_lengths), cmocka_unit_test(test_cstring_equal_empty_both),
        cmocka_unit_test(test_cstring_equal_empty_one_side),    cmocka_unit_test(test_cstring_compare_lex_order),
        cmocka_unit_test(test_cstring_compare_empty),
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}
