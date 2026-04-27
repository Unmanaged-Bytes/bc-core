// SPDX-License-Identifier: MIT

#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <cmocka.h>

#include "bc_core.h"

#include <stdint.h>
#include <string.h>

static void test_unsigned_zero(void** state)
{
    BC_UNUSED(state);
    uint64_t value = 99;
    size_t consumed = 99;
    assert_true(bc_core_parse_unsigned_integer_64_decimal("0", 1, &value, &consumed));
    assert_int_equal(value, 0);
    assert_int_equal(consumed, 1);
}

static void test_unsigned_single_digit(void** state)
{
    BC_UNUSED(state);
    uint64_t value = 0;
    size_t consumed = 0;
    assert_true(bc_core_parse_unsigned_integer_64_decimal("7", 1, &value, &consumed));
    assert_int_equal(value, 7);
    assert_int_equal(consumed, 1);
}

static void test_unsigned_multiple_digits(void** state)
{
    BC_UNUSED(state);
    uint64_t value = 0;
    size_t consumed = 0;
    assert_true(bc_core_parse_unsigned_integer_64_decimal("12345", 5, &value, &consumed));
    assert_int_equal(value, 12345);
    assert_int_equal(consumed, 5);
}

static void test_unsigned_max_value(void** state)
{
    BC_UNUSED(state);
    const char* text = "18446744073709551615";
    uint64_t value = 0;
    size_t consumed = 0;
    assert_true(bc_core_parse_unsigned_integer_64_decimal(text, strlen(text), &value, &consumed));
    assert_int_equal(value, UINT64_MAX);
    assert_int_equal(consumed, 20);
}

static void test_unsigned_overflow_one_past_max(void** state)
{
    BC_UNUSED(state);
    const char* text = "18446744073709551616";
    uint64_t value = 99;
    size_t consumed = 99;
    assert_false(bc_core_parse_unsigned_integer_64_decimal(text, strlen(text), &value, &consumed));
    assert_int_equal(consumed, 0);
}

static void test_unsigned_overflow_huge(void** state)
{
    BC_UNUSED(state);
    const char* text = "999999999999999999999999";
    uint64_t value = 99;
    size_t consumed = 99;
    assert_false(bc_core_parse_unsigned_integer_64_decimal(text, strlen(text), &value, &consumed));
    assert_int_equal(consumed, 0);
}

static void test_unsigned_stops_at_non_digit(void** state)
{
    BC_UNUSED(state);
    const char* text = "123abc";
    uint64_t value = 0;
    size_t consumed = 0;
    assert_true(bc_core_parse_unsigned_integer_64_decimal(text, 6, &value, &consumed));
    assert_int_equal(value, 123);
    assert_int_equal(consumed, 3);
}

static void test_unsigned_stops_at_length_limit(void** state)
{
    BC_UNUSED(state);
    const char* text = "12345";
    uint64_t value = 0;
    size_t consumed = 0;
    assert_true(bc_core_parse_unsigned_integer_64_decimal(text, 3, &value, &consumed));
    assert_int_equal(value, 123);
    assert_int_equal(consumed, 3);
}

static void test_unsigned_zero_length_fails(void** state)
{
    BC_UNUSED(state);
    uint64_t value = 99;
    size_t consumed = 99;
    assert_false(bc_core_parse_unsigned_integer_64_decimal("anything", 0, &value, &consumed));
    assert_int_equal(consumed, 0);
}

static void test_unsigned_no_digits_fails(void** state)
{
    BC_UNUSED(state);
    const char* text = "abc";
    uint64_t value = 99;
    size_t consumed = 99;
    assert_false(bc_core_parse_unsigned_integer_64_decimal(text, 3, &value, &consumed));
    assert_int_equal(consumed, 0);
}

static void test_unsigned_leading_zero(void** state)
{
    BC_UNUSED(state);
    const char* text = "00007";
    uint64_t value = 0;
    size_t consumed = 0;
    assert_true(bc_core_parse_unsigned_integer_64_decimal(text, 5, &value, &consumed));
    assert_int_equal(value, 7);
    assert_int_equal(consumed, 5);
}

static void test_unsigned_no_sign_accepted(void** state)
{
    BC_UNUSED(state);
    const char* text = "+123";
    uint64_t value = 99;
    size_t consumed = 99;
    assert_false(bc_core_parse_unsigned_integer_64_decimal(text, 4, &value, &consumed));
    assert_int_equal(consumed, 0);
}

static void test_signed_positive(void** state)
{
    BC_UNUSED(state);
    int64_t value = 0;
    size_t consumed = 0;
    assert_true(bc_core_parse_signed_integer_64_decimal("12345", 5, &value, &consumed));
    assert_int_equal(value, 12345);
    assert_int_equal(consumed, 5);
}

static void test_signed_negative(void** state)
{
    BC_UNUSED(state);
    int64_t value = 0;
    size_t consumed = 0;
    assert_true(bc_core_parse_signed_integer_64_decimal("-42", 3, &value, &consumed));
    assert_int_equal(value, -42);
    assert_int_equal(consumed, 3);
}

static void test_signed_explicit_plus(void** state)
{
    BC_UNUSED(state);
    int64_t value = 0;
    size_t consumed = 0;
    assert_true(bc_core_parse_signed_integer_64_decimal("+99", 3, &value, &consumed));
    assert_int_equal(value, 99);
    assert_int_equal(consumed, 3);
}

static void test_signed_max_int64(void** state)
{
    BC_UNUSED(state);
    const char* text = "9223372036854775807";
    int64_t value = 0;
    size_t consumed = 0;
    assert_true(bc_core_parse_signed_integer_64_decimal(text, strlen(text), &value, &consumed));
    assert_int_equal(value, INT64_MAX);
    assert_int_equal(consumed, 19);
}

static void test_signed_min_int64(void** state)
{
    BC_UNUSED(state);
    const char* text = "-9223372036854775808";
    int64_t value = 0;
    size_t consumed = 0;
    assert_true(bc_core_parse_signed_integer_64_decimal(text, strlen(text), &value, &consumed));
    assert_int_equal(value, INT64_MIN);
    assert_int_equal(consumed, 20);
}

static void test_signed_overflow_positive(void** state)
{
    BC_UNUSED(state);
    const char* text = "9223372036854775808";
    int64_t value = 99;
    size_t consumed = 99;
    assert_false(bc_core_parse_signed_integer_64_decimal(text, strlen(text), &value, &consumed));
    assert_int_equal(consumed, 0);
}

static void test_signed_overflow_negative(void** state)
{
    BC_UNUSED(state);
    const char* text = "-9223372036854775809";
    int64_t value = 99;
    size_t consumed = 99;
    assert_false(bc_core_parse_signed_integer_64_decimal(text, strlen(text), &value, &consumed));
    assert_int_equal(consumed, 0);
}

static void test_signed_sign_only_fails(void** state)
{
    BC_UNUSED(state);
    int64_t value = 99;
    size_t consumed = 99;
    assert_false(bc_core_parse_signed_integer_64_decimal("-", 1, &value, &consumed));
    assert_int_equal(consumed, 0);
    consumed = 99;
    assert_false(bc_core_parse_signed_integer_64_decimal("+", 1, &value, &consumed));
    assert_int_equal(consumed, 0);
}

static void test_signed_zero_length_fails(void** state)
{
    BC_UNUSED(state);
    int64_t value = 99;
    size_t consumed = 99;
    assert_false(bc_core_parse_signed_integer_64_decimal("anything", 0, &value, &consumed));
    assert_int_equal(consumed, 0);
}

static void test_signed_stops_at_non_digit_after_sign(void** state)
{
    BC_UNUSED(state);
    int64_t value = 0;
    size_t consumed = 0;
    assert_true(bc_core_parse_signed_integer_64_decimal("-7abc", 5, &value, &consumed));
    assert_int_equal(value, -7);
    assert_int_equal(consumed, 2);
}

static void test_signed_negative_zero(void** state)
{
    BC_UNUSED(state);
    int64_t value = 99;
    size_t consumed = 0;
    assert_true(bc_core_parse_signed_integer_64_decimal("-0", 2, &value, &consumed));
    assert_int_equal(value, 0);
    assert_int_equal(consumed, 2);
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_unsigned_zero),
        cmocka_unit_test(test_unsigned_single_digit),
        cmocka_unit_test(test_unsigned_multiple_digits),
        cmocka_unit_test(test_unsigned_max_value),
        cmocka_unit_test(test_unsigned_overflow_one_past_max),
        cmocka_unit_test(test_unsigned_overflow_huge),
        cmocka_unit_test(test_unsigned_stops_at_non_digit),
        cmocka_unit_test(test_unsigned_stops_at_length_limit),
        cmocka_unit_test(test_unsigned_zero_length_fails),
        cmocka_unit_test(test_unsigned_no_digits_fails),
        cmocka_unit_test(test_unsigned_leading_zero),
        cmocka_unit_test(test_unsigned_no_sign_accepted),
        cmocka_unit_test(test_signed_positive),
        cmocka_unit_test(test_signed_negative),
        cmocka_unit_test(test_signed_explicit_plus),
        cmocka_unit_test(test_signed_max_int64),
        cmocka_unit_test(test_signed_min_int64),
        cmocka_unit_test(test_signed_overflow_positive),
        cmocka_unit_test(test_signed_overflow_negative),
        cmocka_unit_test(test_signed_sign_only_fails),
        cmocka_unit_test(test_signed_zero_length_fails),
        cmocka_unit_test(test_signed_stops_at_non_digit_after_sign),
        cmocka_unit_test(test_signed_negative_zero),
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}
