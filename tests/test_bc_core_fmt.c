// SPDX-License-Identifier: MIT

#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <cmocka.h>

#include "bc_core.h"

#include <inttypes.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

static void test_fmt_uint64_dec_zero(void** state)
{
    BC_UNUSED(state);
    char buffer[21];
    size_t length = 0;
    assert_true(bc_core_fmt_uint64_dec(buffer, sizeof(buffer), 0U, &length));
    assert_int_equal(length, 1);
    assert_memory_equal(buffer, "0", 1);
}

static void test_fmt_uint64_dec_small(void** state)
{
    BC_UNUSED(state);
    char buffer[21];
    size_t length = 0;
    assert_true(bc_core_fmt_uint64_dec(buffer, sizeof(buffer), 12345U, &length));
    assert_int_equal(length, 5);
    assert_memory_equal(buffer, "12345", 5);
}

static void test_fmt_uint64_dec_max(void** state)
{
    BC_UNUSED(state);
    char buffer[21];
    size_t length = 0;
    assert_true(bc_core_fmt_uint64_dec(buffer, sizeof(buffer), UINT64_MAX, &length));
    assert_int_equal(length, 20);
    assert_memory_equal(buffer, "18446744073709551615", 20);
}

static void test_fmt_uint64_dec_overflow(void** state)
{
    BC_UNUSED(state);
    char buffer[3];
    size_t length = 0;
    assert_false(bc_core_fmt_uint64_dec(buffer, sizeof(buffer), 12345U, &length));
}

static void test_fmt_uint64_hex(void** state)
{
    BC_UNUSED(state);
    char buffer[16];
    size_t length = 0;

    assert_true(bc_core_fmt_uint64_hex(buffer, sizeof(buffer), 0xDEADBEEFU, &length));
    assert_int_equal(length, 8);
    assert_memory_equal(buffer, "deadbeef", 8);

    assert_true(bc_core_fmt_uint64_hex(buffer, sizeof(buffer), 0U, &length));
    assert_int_equal(length, 1);
    assert_memory_equal(buffer, "0", 1);

    assert_true(bc_core_fmt_uint64_hex(buffer, sizeof(buffer), UINT64_MAX, &length));
    assert_int_equal(length, 16);
    assert_memory_equal(buffer, "ffffffffffffffff", 16);
}

static void test_fmt_uint64_hex_padded(void** state)
{
    BC_UNUSED(state);
    char buffer[16];
    size_t length = 0;

    assert_true(bc_core_fmt_uint64_hex_padded(buffer, sizeof(buffer), 0x2AU, 8U, &length));
    assert_int_equal(length, 8);
    assert_memory_equal(buffer, "0000002a", 8);

    assert_true(bc_core_fmt_uint64_hex_padded(buffer, sizeof(buffer), 0xABCDU, 4U, &length));
    assert_int_equal(length, 4);
    assert_memory_equal(buffer, "abcd", 4);

    assert_false(bc_core_fmt_uint64_hex_padded(buffer, sizeof(buffer), 0x1U, 0U, &length));
    assert_false(bc_core_fmt_uint64_hex_padded(buffer, sizeof(buffer), 0x1U, 17U, &length));
    assert_false(bc_core_fmt_uint64_hex_padded(buffer, 3U, 0x1U, 4U, &length));
}

static void test_fmt_int64_positive(void** state)
{
    BC_UNUSED(state);
    char buffer[22];
    size_t length = 0;
    assert_true(bc_core_fmt_int64(buffer, sizeof(buffer), 42, &length));
    assert_int_equal(length, 2);
    assert_memory_equal(buffer, "42", 2);
}

static void test_fmt_int64_negative(void** state)
{
    BC_UNUSED(state);
    char buffer[22];
    size_t length = 0;
    assert_true(bc_core_fmt_int64(buffer, sizeof(buffer), -12345, &length));
    assert_int_equal(length, 6);
    assert_memory_equal(buffer, "-12345", 6);
}

static void test_fmt_int64_int64_min(void** state)
{
    BC_UNUSED(state);
    char buffer[22];
    size_t length = 0;
    assert_true(bc_core_fmt_int64(buffer, sizeof(buffer), INT64_MIN, &length));
    assert_int_equal(length, 20);
    assert_memory_equal(buffer, "-9223372036854775808", 20);
}

static void test_fmt_int64_overflow(void** state)
{
    BC_UNUSED(state);
    char buffer[4];
    size_t length = 0;
    assert_false(bc_core_fmt_int64(buffer, sizeof(buffer), -12345, &length));
}

static void test_fmt_double_basic(void** state)
{
    BC_UNUSED(state);
    char buffer[64];
    size_t length = 0;

    assert_true(bc_core_fmt_double(buffer, sizeof(buffer), 3.14159, 2, &length));
    assert_int_equal(length, 4);
    assert_memory_equal(buffer, "3.14", 4);

    assert_true(bc_core_fmt_double(buffer, sizeof(buffer), -0.5, 1, &length));
    assert_int_equal(length, 4);
    assert_memory_equal(buffer, "-0.5", 4);

    assert_true(bc_core_fmt_double(buffer, sizeof(buffer), 0.0, 0, &length));
    assert_int_equal(length, 1);
    assert_memory_equal(buffer, "0", 1);
}

static void test_fmt_double_rounding(void** state)
{
    BC_UNUSED(state);
    char buffer[64];
    size_t length = 0;

    assert_true(bc_core_fmt_double(buffer, sizeof(buffer), 0.9999, 3, &length));
    assert_int_equal(length, 5);
    assert_memory_equal(buffer, "1.000", 5);

    assert_true(bc_core_fmt_double(buffer, sizeof(buffer), 1.005, 2, &length));
    assert_int_equal(length, 4);
}

static void test_fmt_double_special(void** state)
{
    BC_UNUSED(state);
    char buffer[64];
    size_t length = 0;

    double nan_value = NAN;
    double inf_value = INFINITY;

    assert_true(bc_core_fmt_double(buffer, sizeof(buffer), nan_value, 3, &length));
    assert_int_equal(length, 3);
    assert_memory_equal(buffer, "nan", 3);

    assert_true(bc_core_fmt_double(buffer, sizeof(buffer), inf_value, 3, &length));
    assert_int_equal(length, 3);
    assert_memory_equal(buffer, "inf", 3);

    assert_true(bc_core_fmt_double(buffer, sizeof(buffer), -inf_value, 3, &length));
    assert_int_equal(length, 4);
    assert_memory_equal(buffer, "-inf", 4);
}

static void test_fmt_double_bad_frac(void** state)
{
    BC_UNUSED(state);
    char buffer[64];
    size_t length = 0;
    assert_false(bc_core_fmt_double(buffer, sizeof(buffer), 1.0, -1, &length));
    assert_false(bc_core_fmt_double(buffer, sizeof(buffer), 1.0, 19, &length));
}

static void test_fmt_bytes_human(void** state)
{
    BC_UNUSED(state);
    char buffer[32];
    size_t length = 0;

    assert_true(bc_core_fmt_bytes_human(buffer, sizeof(buffer), 512U, &length));
    assert_memory_equal(buffer, "512 B", length);

    assert_true(bc_core_fmt_bytes_human(buffer, sizeof(buffer), 2048U, &length));
    assert_memory_equal(buffer, "2.00 KB", length);

    assert_true(bc_core_fmt_bytes_human(buffer, sizeof(buffer), 1536U * 1024U, &length));
    assert_memory_equal(buffer, "1.50 MB", length);

    assert_true(bc_core_fmt_bytes_human(buffer, sizeof(buffer), (uint64_t)1024U * 1024U * 1024U, &length));
    assert_memory_equal(buffer, "1.00 GB", length);
}

static void test_fmt_duration_ns(void** state)
{
    BC_UNUSED(state);
    char buffer[32];
    size_t length = 0;

    assert_true(bc_core_fmt_duration_ns(buffer, sizeof(buffer), 500U, &length));
    assert_memory_equal(buffer, "500ns", length);

    assert_true(bc_core_fmt_duration_ns(buffer, sizeof(buffer), 1500U, &length));
    assert_memory_equal(buffer, "1.500us", length);

    assert_true(bc_core_fmt_duration_ns(buffer, sizeof(buffer), 2500000U, &length));
    assert_memory_equal(buffer, "2.500ms", length);

    assert_true(bc_core_fmt_duration_ns(buffer, sizeof(buffer), 3500000000U, &length));
    assert_memory_equal(buffer, "3.500s", length);
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_fmt_uint64_dec_zero),
        cmocka_unit_test(test_fmt_uint64_dec_small),
        cmocka_unit_test(test_fmt_uint64_dec_max),
        cmocka_unit_test(test_fmt_uint64_dec_overflow),
        cmocka_unit_test(test_fmt_uint64_hex),
        cmocka_unit_test(test_fmt_uint64_hex_padded),
        cmocka_unit_test(test_fmt_int64_positive),
        cmocka_unit_test(test_fmt_int64_negative),
        cmocka_unit_test(test_fmt_int64_int64_min),
        cmocka_unit_test(test_fmt_int64_overflow),
        cmocka_unit_test(test_fmt_double_basic),
        cmocka_unit_test(test_fmt_double_rounding),
        cmocka_unit_test(test_fmt_double_special),
        cmocka_unit_test(test_fmt_double_bad_frac),
        cmocka_unit_test(test_fmt_bytes_human),
        cmocka_unit_test(test_fmt_duration_ns),
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}
