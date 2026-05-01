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
    assert_true(bc_core_format_unsigned_integer_64_decimal(buffer, sizeof(buffer), 0U, &length));
    assert_int_equal(length, 1);
    assert_memory_equal(buffer, "0", 1);
}

static void test_fmt_uint64_dec_small(void** state)
{
    BC_UNUSED(state);
    char buffer[21];
    size_t length = 0;
    assert_true(bc_core_format_unsigned_integer_64_decimal(buffer, sizeof(buffer), 12345U, &length));
    assert_int_equal(length, 5);
    assert_memory_equal(buffer, "12345", 5);
}

static void test_fmt_uint64_dec_max(void** state)
{
    BC_UNUSED(state);
    char buffer[21];
    size_t length = 0;
    assert_true(bc_core_format_unsigned_integer_64_decimal(buffer, sizeof(buffer), UINT64_MAX, &length));
    assert_int_equal(length, 20);
    assert_memory_equal(buffer, "18446744073709551615", 20);
}

static void test_fmt_uint64_dec_overflow(void** state)
{
    BC_UNUSED(state);
    char buffer[3];
    size_t length = 0;
    assert_false(bc_core_format_unsigned_integer_64_decimal(buffer, sizeof(buffer), 12345U, &length));
}

static void test_fmt_uint64_hex(void** state)
{
    BC_UNUSED(state);
    char buffer[16];
    size_t length = 0;

    assert_true(bc_core_format_unsigned_integer_64_hexadecimal(buffer, sizeof(buffer), 0xDEADBEEFU, &length));
    assert_int_equal(length, 8);
    assert_memory_equal(buffer, "deadbeef", 8);

    assert_true(bc_core_format_unsigned_integer_64_hexadecimal(buffer, sizeof(buffer), 0U, &length));
    assert_int_equal(length, 1);
    assert_memory_equal(buffer, "0", 1);

    assert_true(bc_core_format_unsigned_integer_64_hexadecimal(buffer, sizeof(buffer), UINT64_MAX, &length));
    assert_int_equal(length, 16);
    assert_memory_equal(buffer, "ffffffffffffffff", 16);
}

static void test_fmt_uint64_hex_padded(void** state)
{
    BC_UNUSED(state);
    char buffer[16];
    size_t length = 0;

    assert_true(bc_core_format_unsigned_integer_64_hexadecimal_padded(buffer, sizeof(buffer), 0x2AU, 8U, &length));
    assert_int_equal(length, 8);
    assert_memory_equal(buffer, "0000002a", 8);

    assert_true(bc_core_format_unsigned_integer_64_hexadecimal_padded(buffer, sizeof(buffer), 0xABCDU, 4U, &length));
    assert_int_equal(length, 4);
    assert_memory_equal(buffer, "abcd", 4);

    assert_false(bc_core_format_unsigned_integer_64_hexadecimal_padded(buffer, sizeof(buffer), 0x1U, 0U, &length));
    assert_false(bc_core_format_unsigned_integer_64_hexadecimal_padded(buffer, sizeof(buffer), 0x1U, 17U, &length));
    assert_false(bc_core_format_unsigned_integer_64_hexadecimal_padded(buffer, 3U, 0x1U, 4U, &length));
}

static void test_fmt_int64_positive(void** state)
{
    BC_UNUSED(state);
    char buffer[22];
    size_t length = 0;
    assert_true(bc_core_format_signed_integer_64(buffer, sizeof(buffer), 42, &length));
    assert_int_equal(length, 2);
    assert_memory_equal(buffer, "42", 2);
}

static void test_fmt_int64_negative(void** state)
{
    BC_UNUSED(state);
    char buffer[22];
    size_t length = 0;
    assert_true(bc_core_format_signed_integer_64(buffer, sizeof(buffer), -12345, &length));
    assert_int_equal(length, 6);
    assert_memory_equal(buffer, "-12345", 6);
}

static void test_fmt_int64_int64_min(void** state)
{
    BC_UNUSED(state);
    char buffer[22];
    size_t length = 0;
    assert_true(bc_core_format_signed_integer_64(buffer, sizeof(buffer), INT64_MIN, &length));
    assert_int_equal(length, 20);
    assert_memory_equal(buffer, "-9223372036854775808", 20);
}

static void test_fmt_int64_overflow(void** state)
{
    BC_UNUSED(state);
    char buffer[4];
    size_t length = 0;
    assert_false(bc_core_format_signed_integer_64(buffer, sizeof(buffer), -12345, &length));
}

static void test_fmt_double_basic(void** state)
{
    BC_UNUSED(state);
    char buffer[64];
    size_t length = 0;

    assert_true(bc_core_format_double(buffer, sizeof(buffer), 3.14159, 2, &length));
    assert_int_equal(length, 4);
    assert_memory_equal(buffer, "3.14", 4);

    assert_true(bc_core_format_double(buffer, sizeof(buffer), -0.5, 1, &length));
    assert_int_equal(length, 4);
    assert_memory_equal(buffer, "-0.5", 4);

    assert_true(bc_core_format_double(buffer, sizeof(buffer), 0.0, 0, &length));
    assert_int_equal(length, 1);
    assert_memory_equal(buffer, "0", 1);
}

static void test_fmt_double_rounding(void** state)
{
    BC_UNUSED(state);
    char buffer[64];
    size_t length = 0;

    assert_true(bc_core_format_double(buffer, sizeof(buffer), 0.9999, 3, &length));
    assert_int_equal(length, 5);
    assert_memory_equal(buffer, "1.000", 5);

    assert_true(bc_core_format_double(buffer, sizeof(buffer), 1.005, 2, &length));
    assert_int_equal(length, 4);
}

static void test_fmt_double_special(void** state)
{
    BC_UNUSED(state);
    char buffer[64];
    size_t length = 0;

    double nan_value = NAN;
    double inf_value = INFINITY;

    assert_true(bc_core_format_double(buffer, sizeof(buffer), nan_value, 3, &length));
    assert_int_equal(length, 3);
    assert_memory_equal(buffer, "nan", 3);

    assert_true(bc_core_format_double(buffer, sizeof(buffer), inf_value, 3, &length));
    assert_int_equal(length, 3);
    assert_memory_equal(buffer, "inf", 3);

    assert_true(bc_core_format_double(buffer, sizeof(buffer), -inf_value, 3, &length));
    assert_int_equal(length, 4);
    assert_memory_equal(buffer, "-inf", 4);
}

static void test_fmt_double_bad_frac(void** state)
{
    BC_UNUSED(state);
    char buffer[64];
    size_t length = 0;
    assert_false(bc_core_format_double(buffer, sizeof(buffer), 1.0, -1, &length));
    assert_false(bc_core_format_double(buffer, sizeof(buffer), 1.0, 19, &length));
}

static void test_fmt_bytes_human(void** state)
{
    BC_UNUSED(state);
    char buffer[32];
    size_t length = 0;

    assert_true(bc_core_format_bytes_human_readable(buffer, sizeof(buffer), 512U, &length));
    assert_memory_equal(buffer, "512 B", length);

    assert_true(bc_core_format_bytes_human_readable(buffer, sizeof(buffer), 2048U, &length));
    assert_memory_equal(buffer, "2.00 KB", length);

    assert_true(bc_core_format_bytes_human_readable(buffer, sizeof(buffer), 1536U * 1024U, &length));
    assert_memory_equal(buffer, "1.50 MB", length);

    assert_true(bc_core_format_bytes_human_readable(buffer, sizeof(buffer), (uint64_t)1024U * 1024U * 1024U, &length));
    assert_memory_equal(buffer, "1.00 GB", length);
}

static void test_fmt_duration_ns(void** state)
{
    BC_UNUSED(state);
    char buffer[32];
    size_t length = 0;

    assert_true(bc_core_format_duration_nanoseconds(buffer, sizeof(buffer), 500U, &length));
    assert_memory_equal(buffer, "500ns", length);

    assert_true(bc_core_format_duration_nanoseconds(buffer, sizeof(buffer), 1500U, &length));
    assert_memory_equal(buffer, "1.500us", length);

    assert_true(bc_core_format_duration_nanoseconds(buffer, sizeof(buffer), 2500000U, &length));
    assert_memory_equal(buffer, "2.500ms", length);

    assert_true(bc_core_format_duration_nanoseconds(buffer, sizeof(buffer), 3500000000U, &length));
    assert_memory_equal(buffer, "3.500s", length);
}

static void test_fmt_unicode_codepoint_escape_ascii(void** state)
{
    BC_UNUSED(state);
    char buffer[16];
    size_t length = 0;
    assert_true(bc_core_format_unicode_codepoint_escape(buffer, sizeof(buffer), 0x41U, &length));
    assert_int_equal(length, 6);
    assert_memory_equal(buffer, "\\u0041", 6);
}

static void test_fmt_unicode_codepoint_escape_zero(void** state)
{
    BC_UNUSED(state);
    char buffer[16];
    size_t length = 0;
    assert_true(bc_core_format_unicode_codepoint_escape(buffer, sizeof(buffer), 0x0000U, &length));
    assert_int_equal(length, 6);
    assert_memory_equal(buffer, "\\u0000", 6);
}

static void test_fmt_unicode_codepoint_escape_bmp(void** state)
{
    BC_UNUSED(state);
    char buffer[16];
    size_t length = 0;
    assert_true(bc_core_format_unicode_codepoint_escape(buffer, sizeof(buffer), 0x00E9U, &length));
    assert_int_equal(length, 6);
    assert_memory_equal(buffer, "\\u00E9", 6);
}

static void test_fmt_unicode_codepoint_escape_bmp_max(void** state)
{
    BC_UNUSED(state);
    char buffer[16];
    size_t length = 0;
    assert_true(bc_core_format_unicode_codepoint_escape(buffer, sizeof(buffer), 0xFFFFU, &length));
    assert_int_equal(length, 6);
    assert_memory_equal(buffer, "\\uFFFF", 6);
}

static void test_fmt_unicode_codepoint_escape_supplementary(void** state)
{
    BC_UNUSED(state);
    char buffer[16];
    size_t length = 0;
    assert_true(bc_core_format_unicode_codepoint_escape(buffer, sizeof(buffer), 0x1F600U, &length));
    assert_int_equal(length, 12);
    assert_memory_equal(buffer, "\\uD83D\\uDE00", 12);
}

static void test_fmt_unicode_codepoint_escape_supplementary_min(void** state)
{
    BC_UNUSED(state);
    char buffer[16];
    size_t length = 0;
    assert_true(bc_core_format_unicode_codepoint_escape(buffer, sizeof(buffer), 0x10000U, &length));
    assert_int_equal(length, 12);
    assert_memory_equal(buffer, "\\uD800\\uDC00", 12);
}

static void test_fmt_unicode_codepoint_escape_supplementary_max(void** state)
{
    BC_UNUSED(state);
    char buffer[16];
    size_t length = 0;
    assert_true(bc_core_format_unicode_codepoint_escape(buffer, sizeof(buffer), 0x10FFFFU, &length));
    assert_int_equal(length, 12);
    assert_memory_equal(buffer, "\\uDBFF\\uDFFF", 12);
}

static void test_fmt_unicode_codepoint_escape_rejects_above_max(void** state)
{
    BC_UNUSED(state);
    char buffer[16];
    size_t length = 0;
    assert_false(bc_core_format_unicode_codepoint_escape(buffer, sizeof(buffer), 0x110000U, &length));
}

static void test_fmt_unicode_codepoint_escape_rejects_high_surrogate(void** state)
{
    BC_UNUSED(state);
    char buffer[16];
    size_t length = 0;
    assert_false(bc_core_format_unicode_codepoint_escape(buffer, sizeof(buffer), 0xD800U, &length));
}

static void test_fmt_unicode_codepoint_escape_rejects_low_surrogate(void** state)
{
    BC_UNUSED(state);
    char buffer[16];
    size_t length = 0;
    assert_false(bc_core_format_unicode_codepoint_escape(buffer, sizeof(buffer), 0xDFFFU, &length));
}

static void test_fmt_unicode_codepoint_escape_buffer_too_small_bmp(void** state)
{
    BC_UNUSED(state);
    char buffer[5];
    size_t length = 0;
    assert_false(bc_core_format_unicode_codepoint_escape(buffer, sizeof(buffer), 0x0041U, &length));
}

static void test_fmt_unicode_codepoint_escape_buffer_too_small_supplementary(void** state)
{
    BC_UNUSED(state);
    char buffer[11];
    size_t length = 0;
    assert_false(bc_core_format_unicode_codepoint_escape(buffer, sizeof(buffer), 0x1F600U, &length));
}

static void test_format_double_negative_zero(void** state)
{
    BC_UNUSED(state);
    char buffer[64];
    size_t length = 0;
    double negative_zero = -0.0;
    assert_true(bc_core_format_double(buffer, sizeof(buffer), negative_zero, 2, &length));
    assert_int_equal(length, 5);
    assert_memory_equal(buffer, "-0.00", 5);
}

static void test_format_bytes_human_max_uint64(void** state)
{
    BC_UNUSED(state);
    char buffer[64];
    size_t length = 0;
    assert_true(bc_core_format_bytes_human_readable(buffer, sizeof(buffer), UINT64_MAX, &length));
    assert_in_range(length, 4, 32);
}

static void test_format_uint64_dec_capacity_zero(void** state)
{
    BC_UNUSED(state);
    char buffer[1];
    size_t length = 0;
    assert_false(bc_core_format_unsigned_integer_64_decimal(buffer, 0, 12345U, &length));
}

static void test_format_uint64_hex_capacity_zero(void** state)
{
    BC_UNUSED(state);
    char buffer[1];
    size_t length = 0;
    assert_false(bc_core_format_unsigned_integer_64_hexadecimal(buffer, 0, 0xABCDU, &length));
}

static void test_format_signed_integer_64_negative_capacity_one(void** state)
{
    BC_UNUSED(state);
    char buffer[1];
    size_t length = 0;
    assert_false(bc_core_format_signed_integer_64(buffer, 1, -42, &length));
}

static void test_format_signed_integer_64_negative_capacity_too_small_for_digits(void** state)
{
    BC_UNUSED(state);
    char buffer[2];
    size_t length = 0;
    assert_false(bc_core_format_signed_integer_64(buffer, 2, -12345, &length));
}

static void test_format_signed_integer_64_capacity_zero_negative(void** state)
{
    BC_UNUSED(state);
    char buffer[1];
    size_t length = 0;
    assert_false(bc_core_format_signed_integer_64(buffer, 0, -1, &length));
}

static void test_format_double_value_too_large(void** state)
{
    BC_UNUSED(state);
    char buffer[64];
    size_t length = 0;
    /* magnitude * scale + 0.5 must overflow UINT64_MAX */
    assert_false(bc_core_format_double(buffer, sizeof(buffer), 1e20, 0, &length));
}

static void test_format_double_negative_capacity_zero(void** state)
{
    BC_UNUSED(state);
    char buffer[1];
    size_t length = 0;
    assert_false(bc_core_format_double(buffer, 0, -1.0, 0, &length));
}

static void test_format_bytes_human_capacity_no_room_for_space(void** state)
{
    BC_UNUSED(state);
    char buffer[4];
    size_t length = 0;
    /* "2.00" fits exactly in 4 bytes -> position == capacity -> no room for ' ' */
    assert_false(bc_core_format_bytes_human_readable(buffer, sizeof(buffer), 2048U, &length));
}

static void test_format_double_capacity_zero(void** state)
{
    BC_UNUSED(state);
    char buffer[1];
    size_t length = 0;
    assert_false(bc_core_format_double(buffer, 0, 1.0, 2, &length));
}

static void test_format_double_capacity_too_small_for_integer(void** state)
{
    BC_UNUSED(state);
    char buffer[1];
    size_t length = 0;
    assert_false(bc_core_format_double(buffer, 1, 12345.6, 2, &length));
}

static void test_format_double_capacity_too_small_for_decimal_point(void** state)
{
    BC_UNUSED(state);
    char buffer[2];
    size_t length = 0;
    assert_false(bc_core_format_double(buffer, 2, 12.3, 1, &length));
}

static void test_format_double_capacity_too_small_for_fraction(void** state)
{
    BC_UNUSED(state);
    char buffer[5];
    size_t length = 0;
    assert_false(bc_core_format_double(buffer, 5, 12.345, 5, &length));
}

static void test_format_double_negative_capacity_one(void** state)
{
    BC_UNUSED(state);
    char buffer[1];
    size_t length = 0;
    assert_false(bc_core_format_double(buffer, 1, -1.0, 0, &length));
}

static void test_format_double_negative_infinity_capacity_too_small(void** state)
{
    BC_UNUSED(state);
    char buffer[3];
    size_t length = 0;
    double inf_value = HUGE_VAL;
    assert_false(bc_core_format_double(buffer, sizeof(buffer), -inf_value, 0, &length));
}

static void test_format_double_nan_capacity_too_small(void** state)
{
    BC_UNUSED(state);
    char buffer[2];
    size_t length = 0;
    double nan_value = __builtin_nan("");
    assert_false(bc_core_format_double(buffer, sizeof(buffer), nan_value, 0, &length));
}

static void test_format_bytes_human_b_only(void** state)
{
    BC_UNUSED(state);
    char buffer[16];
    size_t length = 0;
    assert_true(bc_core_format_bytes_human_readable(buffer, sizeof(buffer), 512U, &length));
    assert_memory_equal(buffer, "512 B", length);
}

static void test_format_bytes_human_kb_two_decimals(void** state)
{
    BC_UNUSED(state);
    char buffer[16];
    size_t length = 0;
    assert_true(bc_core_format_bytes_human_readable(buffer, sizeof(buffer), 2048U, &length));
    assert_memory_equal(buffer, "2.00 KB", length);
}

static void test_format_bytes_human_mb_one_decimal(void** state)
{
    BC_UNUSED(state);
    char buffer[16];
    size_t length = 0;
    uint64_t bytes = 50ULL * 1024ULL * 1024ULL;
    assert_true(bc_core_format_bytes_human_readable(buffer, sizeof(buffer), bytes, &length));
    assert_memory_equal(buffer, "50.0 MB", length);
}

static void test_format_bytes_human_gb_zero_decimal(void** state)
{
    BC_UNUSED(state);
    char buffer[16];
    size_t length = 0;
    uint64_t bytes = 500ULL * 1024ULL * 1024ULL * 1024ULL;
    assert_true(bc_core_format_bytes_human_readable(buffer, sizeof(buffer), bytes, &length));
    assert_memory_equal(buffer, "500 GB", length);
}

static void test_format_bytes_human_capacity_too_small(void** state)
{
    BC_UNUSED(state);
    char buffer[3];
    size_t length = 0;
    assert_false(bc_core_format_bytes_human_readable(buffer, sizeof(buffer), 2048U, &length));
}

static void test_format_bytes_human_capacity_no_room_for_unit(void** state)
{
    BC_UNUSED(state);
    char buffer[5];
    size_t length = 0;
    assert_false(bc_core_format_bytes_human_readable(buffer, sizeof(buffer), 2048U, &length));
}

static void test_format_duration_us_three_decimals(void** state)
{
    BC_UNUSED(state);
    char buffer[16];
    size_t length = 0;
    assert_true(bc_core_format_duration_nanoseconds(buffer, sizeof(buffer), 1500U, &length));
    assert_memory_equal(buffer, "1.500us", length);
}

static void test_format_duration_ms_two_decimals(void** state)
{
    BC_UNUSED(state);
    char buffer[16];
    size_t length = 0;
    assert_true(bc_core_format_duration_nanoseconds(buffer, sizeof(buffer), 25ULL * 1000ULL * 1000ULL, &length));
    assert_memory_equal(buffer, "25.00ms", length);
}

static void test_format_duration_s_one_decimal(void** state)
{
    BC_UNUSED(state);
    char buffer[16];
    size_t length = 0;
    assert_true(bc_core_format_duration_nanoseconds(buffer, sizeof(buffer), 250ULL * 1000ULL * 1000ULL * 1000ULL, &length));
    assert_memory_equal(buffer, "250.0s", length);
}

static void test_format_duration_capacity_too_small(void** state)
{
    BC_UNUSED(state);
    char buffer[2];
    size_t length = 0;
    assert_false(bc_core_format_duration_nanoseconds(buffer, sizeof(buffer), 1500U, &length));
}

static void test_format_duration_capacity_no_room_for_unit(void** state)
{
    BC_UNUSED(state);
    char buffer[6];
    size_t length = 0;
    assert_false(bc_core_format_duration_nanoseconds(buffer, sizeof(buffer), 1500U, &length));
}

static void test_writer_write_unicode_codepoint_escape(void** state)
{
    BC_UNUSED(state);
    bc_core_writer_t writer;
    char buffer[64];
    assert_true(bc_core_writer_init_buffer_only(&writer, buffer, sizeof(buffer)));

    assert_true(bc_core_writer_write_unicode_codepoint_escape(&writer, 0x00E9U));
    assert_true(bc_core_writer_write_unicode_codepoint_escape(&writer, 0x1F600U));

    const char* data = NULL;
    size_t length = 0;
    assert_true(bc_core_writer_buffer_data(&writer, &data, &length));
    assert_int_equal(length, 18);
    assert_memory_equal(data, "\\u00E9\\uD83D\\uDE00", 18);
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
        cmocka_unit_test(test_fmt_unicode_codepoint_escape_ascii),
        cmocka_unit_test(test_fmt_unicode_codepoint_escape_zero),
        cmocka_unit_test(test_fmt_unicode_codepoint_escape_bmp),
        cmocka_unit_test(test_fmt_unicode_codepoint_escape_bmp_max),
        cmocka_unit_test(test_fmt_unicode_codepoint_escape_supplementary),
        cmocka_unit_test(test_fmt_unicode_codepoint_escape_supplementary_min),
        cmocka_unit_test(test_fmt_unicode_codepoint_escape_supplementary_max),
        cmocka_unit_test(test_fmt_unicode_codepoint_escape_rejects_above_max),
        cmocka_unit_test(test_fmt_unicode_codepoint_escape_rejects_high_surrogate),
        cmocka_unit_test(test_fmt_unicode_codepoint_escape_rejects_low_surrogate),
        cmocka_unit_test(test_fmt_unicode_codepoint_escape_buffer_too_small_bmp),
        cmocka_unit_test(test_fmt_unicode_codepoint_escape_buffer_too_small_supplementary),
        cmocka_unit_test(test_format_double_negative_zero),
        cmocka_unit_test(test_format_bytes_human_max_uint64),
        cmocka_unit_test(test_format_uint64_dec_capacity_zero),
        cmocka_unit_test(test_format_uint64_hex_capacity_zero),
        cmocka_unit_test(test_format_signed_integer_64_negative_capacity_one),
        cmocka_unit_test(test_format_signed_integer_64_negative_capacity_too_small_for_digits),
        cmocka_unit_test(test_format_signed_integer_64_capacity_zero_negative),
        cmocka_unit_test(test_format_double_value_too_large),
        cmocka_unit_test(test_format_double_negative_capacity_zero),
        cmocka_unit_test(test_format_bytes_human_capacity_no_room_for_space),
        cmocka_unit_test(test_format_double_capacity_zero),
        cmocka_unit_test(test_format_double_capacity_too_small_for_integer),
        cmocka_unit_test(test_format_double_capacity_too_small_for_decimal_point),
        cmocka_unit_test(test_format_double_capacity_too_small_for_fraction),
        cmocka_unit_test(test_format_double_negative_capacity_one),
        cmocka_unit_test(test_format_double_negative_infinity_capacity_too_small),
        cmocka_unit_test(test_format_double_nan_capacity_too_small),
        cmocka_unit_test(test_format_bytes_human_b_only),
        cmocka_unit_test(test_format_bytes_human_kb_two_decimals),
        cmocka_unit_test(test_format_bytes_human_mb_one_decimal),
        cmocka_unit_test(test_format_bytes_human_gb_zero_decimal),
        cmocka_unit_test(test_format_bytes_human_capacity_too_small),
        cmocka_unit_test(test_format_bytes_human_capacity_no_room_for_unit),
        cmocka_unit_test(test_format_duration_us_three_decimals),
        cmocka_unit_test(test_format_duration_ms_two_decimals),
        cmocka_unit_test(test_format_duration_s_one_decimal),
        cmocka_unit_test(test_format_duration_capacity_too_small),
        cmocka_unit_test(test_format_duration_capacity_no_room_for_unit),
        cmocka_unit_test(test_writer_write_unicode_codepoint_escape),
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}
