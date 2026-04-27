// SPDX-License-Identifier: MIT

#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <cmocka.h>

#include "bc_core.h"

#include <float.h>
#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static double parse_back(const char* buffer, size_t length)
{
    char nul_terminated[64];
    assert_true(length < sizeof(nul_terminated));
    memcpy(nul_terminated, buffer, length);
    nul_terminated[length] = '\0';
    return strtod(nul_terminated, NULL);
}

static void assert_round_trip(double value)
{
    char buffer[64];
    size_t length = 0;
    assert_true(bc_core_format_double_shortest_round_trip(buffer, sizeof(buffer), value, &length));
    assert_true(length > 0U);
    assert_true(length < sizeof(buffer));

    double parsed = parse_back(buffer, length);
    if (isnan(value)) {
        assert_true(isnan(parsed));
    } else {
        uint64_t value_bits;
        uint64_t parsed_bits;
        memcpy(&value_bits, &value, sizeof(value_bits));
        memcpy(&parsed_bits, &parsed, sizeof(parsed_bits));
        assert_int_equal(value_bits, parsed_bits);
    }
}

static void test_zero_positive(void** state)
{
    BC_UNUSED(state);
    assert_round_trip(0.0);
}

static void test_zero_negative(void** state)
{
    BC_UNUSED(state);
    char buffer[64];
    size_t length = 0;
    double minus_zero = -0.0;
    assert_true(bc_core_format_double_shortest_round_trip(buffer, sizeof(buffer), minus_zero, &length));
    double parsed = parse_back(buffer, length);
    /* sign bit must be preserved by round-trip. */
    assert_true(signbit(parsed) != 0);
}

static void test_nan(void** state)
{
    BC_UNUSED(state);
    char buffer[64];
    size_t length = 0;
    assert_true(bc_core_format_double_shortest_round_trip(buffer, sizeof(buffer), (double)NAN, &length));
    double parsed = parse_back(buffer, length);
    assert_true(isnan(parsed));
}

static void test_positive_infinity(void** state)
{
    BC_UNUSED(state);
    char buffer[64];
    size_t length = 0;
    double inf_value = (double)INFINITY;
    assert_true(bc_core_format_double_shortest_round_trip(buffer, sizeof(buffer), inf_value, &length));
    double parsed = parse_back(buffer, length);
    assert_true(isinf(parsed));
    assert_true(parsed > 0.0);
}

static void test_negative_infinity(void** state)
{
    BC_UNUSED(state);
    char buffer[64];
    size_t length = 0;
    double inf_value = -(double)INFINITY;
    assert_true(bc_core_format_double_shortest_round_trip(buffer, sizeof(buffer), inf_value, &length));
    double parsed = parse_back(buffer, length);
    assert_true(isinf(parsed));
    assert_true(parsed < 0.0);
}

static void test_dbl_min_subnormal_boundary(void** state)
{
    BC_UNUSED(state);
    assert_round_trip(DBL_MIN);
    assert_round_trip(DBL_MIN / 2.0);
    assert_round_trip(DBL_TRUE_MIN);
}

static void test_dbl_max(void** state)
{
    BC_UNUSED(state);
    assert_round_trip(DBL_MAX);
    assert_round_trip(-DBL_MAX);
}

static void test_integers_one_to_seventeen_digits(void** state)
{
    BC_UNUSED(state);
    double value = 1.0;
    for (int digit_count = 1; digit_count <= 17; digit_count++) {
        assert_round_trip(value);
        value = value * 10.0 + 1.0;
    }
}

static void test_simple_values(void** state)
{
    BC_UNUSED(state);
    assert_round_trip(1.0);
    assert_round_trip(-1.0);
    assert_round_trip(2.0);
    assert_round_trip(0.5);
    assert_round_trip(0.1);
    assert_round_trip(3.141592653589793);
    assert_round_trip(2.718281828459045);
    assert_round_trip(1e100);
    assert_round_trip(1e-100);
    assert_round_trip(-0.0001);
}

static void test_random_round_trip_property(void** state)
{
    BC_UNUSED(state);
    /* Deterministic seed for reproducibility. */
    srand(0xC0FFEEU);
    for (int sample_index = 0; sample_index < 100; sample_index++) {
        uint64_t random_bits = 0;
        for (int byte_index = 0; byte_index < 8; byte_index++) {
            random_bits = (random_bits << 8) | (uint64_t)(rand() & 0xFF);
        }
        double value;
        memcpy(&value, &random_bits, sizeof(value));
        if (isnan(value) || isinf(value)) {
            continue;
        }
        assert_round_trip(value);
    }
}

static void test_buffer_too_small(void** state)
{
    BC_UNUSED(state);
    char buffer[4];
    size_t length = 0;
    /* "%.17g" needs more than 4 chars for a value like 1.234e-300. */
    assert_false(bc_core_format_double_shortest_round_trip(buffer, sizeof(buffer), 1.234e-300, &length));
}

static void test_capacity_zero(void** state)
{
    BC_UNUSED(state);
    char buffer[8];
    size_t length = 0;
    assert_false(bc_core_format_double_shortest_round_trip(buffer, 0U, 1.0, &length));
}

static void test_buffer_exact_size_succeeds(void** state)
{
    BC_UNUSED(state);
    char first_buffer[64];
    size_t first_length = 0;
    assert_true(bc_core_format_double_shortest_round_trip(first_buffer, sizeof(first_buffer), 0.1, &first_length));

    /* Need first_length + 1 for the null byte snprintf writes. */
    char tight_buffer[64];
    size_t tight_length = 0;
    assert_true(bc_core_format_double_shortest_round_trip(tight_buffer, first_length + 1U, 0.1, &tight_length));
    assert_int_equal(tight_length, first_length);
    assert_memory_equal(tight_buffer, first_buffer, first_length);
}

static void test_buffer_one_short_fails(void** state)
{
    BC_UNUSED(state);
    char first_buffer[64];
    size_t first_length = 0;
    assert_true(bc_core_format_double_shortest_round_trip(first_buffer, sizeof(first_buffer), 0.1, &first_length));

    char tight_buffer[64];
    size_t tight_length = 0;
    /* first_length bytes is one short (no room for null terminator). */
    assert_false(bc_core_format_double_shortest_round_trip(tight_buffer, first_length, 0.1, &tight_length));
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_zero_positive),
        cmocka_unit_test(test_zero_negative),
        cmocka_unit_test(test_nan),
        cmocka_unit_test(test_positive_infinity),
        cmocka_unit_test(test_negative_infinity),
        cmocka_unit_test(test_dbl_min_subnormal_boundary),
        cmocka_unit_test(test_dbl_max),
        cmocka_unit_test(test_integers_one_to_seventeen_digits),
        cmocka_unit_test(test_simple_values),
        cmocka_unit_test(test_random_round_trip_property),
        cmocka_unit_test(test_buffer_too_small),
        cmocka_unit_test(test_capacity_zero),
        cmocka_unit_test(test_buffer_exact_size_succeeds),
        cmocka_unit_test(test_buffer_one_short_fails),
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}
