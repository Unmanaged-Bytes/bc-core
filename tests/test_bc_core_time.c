// SPDX-License-Identifier: MIT

#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <cmocka.h>

#include "bc_core.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static void test_monotonic_now_returns_increasing_values(void** state)
{
    BC_UNUSED(state);
    bc_core_time_t before = {0, 0};
    bc_core_time_t after = {0, 0};
    assert_true(bc_core_time_monotonic_now(&before));
    assert_true(bc_core_time_sleep_milliseconds(2));
    assert_true(bc_core_time_monotonic_now(&after));
    const bc_core_time_t delta = bc_core_time_subtract(after, before);
    assert_true(delta.seconds >= 0);
    if (delta.seconds == 0) {
        assert_true(delta.nanoseconds > 0);
    }
}

static void test_realtime_now_succeeds(void** state)
{
    BC_UNUSED(state);
    bc_core_time_t now = {0, 0};
    assert_true(bc_core_time_realtime_now(&now));
    assert_true(now.seconds > 0);
    assert_true(now.nanoseconds >= 0);
    assert_true(now.nanoseconds < BC_CORE_TIME_NANOSECONDS_PER_SECOND);
}

static void test_sleep_milliseconds_zero_returns_immediately(void** state)
{
    BC_UNUSED(state);
    assert_true(bc_core_time_sleep_milliseconds(0));
}

static void test_sleep_nanoseconds_small(void** state)
{
    BC_UNUSED(state);
    assert_true(bc_core_time_sleep_nanoseconds(1));
    assert_true(bc_core_time_sleep_nanoseconds(0));
}

static void test_sleep_milliseconds_carry_to_seconds(void** state)
{
    BC_UNUSED(state);
    bc_core_time_t before = {0, 0};
    bc_core_time_t after = {0, 0};
    assert_true(bc_core_time_monotonic_now(&before));
    assert_true(bc_core_time_sleep_milliseconds(1500));
    assert_true(bc_core_time_monotonic_now(&after));
    const bc_core_time_t delta = bc_core_time_subtract(after, before);
    uint64_t delta_milliseconds = 0;
    assert_true(bc_core_time_to_milliseconds(delta, &delta_milliseconds));
    assert_true(delta_milliseconds >= 1500U);
    assert_true(delta_milliseconds < 3000U);
}

static void test_sleep_nanoseconds_carry_to_seconds(void** state)
{
    BC_UNUSED(state);
    assert_true(bc_core_time_sleep_nanoseconds((uint64_t)BC_CORE_TIME_NANOSECONDS_PER_SECOND + 1U));
}

static void test_normalize_carry_positive(void** state)
{
    BC_UNUSED(state);
    bc_core_time_t input = {5, BC_CORE_TIME_NANOSECONDS_PER_SECOND + 250};
    const bc_core_time_t result = bc_core_time_normalize(input);
    assert_int_equal(result.seconds, 6);
    assert_int_equal(result.nanoseconds, 250);
}

static void test_normalize_carry_multiple_seconds(void** state)
{
    BC_UNUSED(state);
    bc_core_time_t input = {0, 2 * BC_CORE_TIME_NANOSECONDS_PER_SECOND + 7};
    const bc_core_time_t result = bc_core_time_normalize(input);
    assert_int_equal(result.seconds, 2);
    assert_int_equal(result.nanoseconds, 7);
}

static void test_normalize_borrow_negative(void** state)
{
    BC_UNUSED(state);
    bc_core_time_t input = {5, -250};
    const bc_core_time_t result = bc_core_time_normalize(input);
    assert_int_equal(result.seconds, 4);
    assert_int_equal(result.nanoseconds, BC_CORE_TIME_NANOSECONDS_PER_SECOND - 250);
}

static void test_normalize_borrow_multiple_seconds(void** state)
{
    BC_UNUSED(state);
    bc_core_time_t input = {5, -BC_CORE_TIME_NANOSECONDS_PER_SECOND - 250};
    const bc_core_time_t result = bc_core_time_normalize(input);
    assert_int_equal(result.seconds, 3);
    assert_int_equal(result.nanoseconds, BC_CORE_TIME_NANOSECONDS_PER_SECOND - 250);
}

static void test_normalize_already_normal(void** state)
{
    BC_UNUSED(state);
    bc_core_time_t input = {100, 500};
    const bc_core_time_t result = bc_core_time_normalize(input);
    assert_int_equal(result.seconds, 100);
    assert_int_equal(result.nanoseconds, 500);
}

static void test_add_no_carry(void** state)
{
    BC_UNUSED(state);
    bc_core_time_t left = {5, 100};
    bc_core_time_t right = {3, 200};
    const bc_core_time_t result = bc_core_time_add(left, right);
    assert_int_equal(result.seconds, 8);
    assert_int_equal(result.nanoseconds, 300);
}

static void test_add_with_carry(void** state)
{
    BC_UNUSED(state);
    bc_core_time_t left = {5, BC_CORE_TIME_NANOSECONDS_PER_SECOND - 100};
    bc_core_time_t right = {3, 250};
    const bc_core_time_t result = bc_core_time_add(left, right);
    assert_int_equal(result.seconds, 9);
    assert_int_equal(result.nanoseconds, 150);
}

static void test_subtract_no_borrow(void** state)
{
    BC_UNUSED(state);
    bc_core_time_t end = {10, 500};
    bc_core_time_t start = {3, 200};
    const bc_core_time_t result = bc_core_time_subtract(end, start);
    assert_int_equal(result.seconds, 7);
    assert_int_equal(result.nanoseconds, 300);
}

static void test_subtract_with_borrow(void** state)
{
    BC_UNUSED(state);
    bc_core_time_t end = {10, 100};
    bc_core_time_t start = {3, 500};
    const bc_core_time_t result = bc_core_time_subtract(end, start);
    assert_int_equal(result.seconds, 6);
    assert_int_equal(result.nanoseconds, BC_CORE_TIME_NANOSECONDS_PER_SECOND - 400);
}

static void test_subtract_negative_result(void** state)
{
    BC_UNUSED(state);
    bc_core_time_t end = {1, 0};
    bc_core_time_t start = {5, 0};
    const bc_core_time_t result = bc_core_time_subtract(end, start);
    assert_int_equal(result.seconds, -4);
    assert_int_equal(result.nanoseconds, 0);
}

static void test_compare_seconds_dominant(void** state)
{
    BC_UNUSED(state);
    bc_core_time_t earlier = {5, 999};
    bc_core_time_t later = {6, 0};
    assert_int_equal(bc_core_time_compare(earlier, later), -1);
    assert_int_equal(bc_core_time_compare(later, earlier), 1);
}

static void test_compare_nanoseconds_tiebreak(void** state)
{
    BC_UNUSED(state);
    bc_core_time_t earlier = {5, 100};
    bc_core_time_t later = {5, 200};
    assert_int_equal(bc_core_time_compare(earlier, later), -1);
    assert_int_equal(bc_core_time_compare(later, earlier), 1);
}

static void test_compare_equal(void** state)
{
    BC_UNUSED(state);
    bc_core_time_t left = {5, 100};
    bc_core_time_t right = {5, 100};
    assert_int_equal(bc_core_time_compare(left, right), 0);
}

static void test_compare_normalizes_inputs(void** state)
{
    BC_UNUSED(state);
    bc_core_time_t left = {5, 0};
    bc_core_time_t right = {4, BC_CORE_TIME_NANOSECONDS_PER_SECOND};
    assert_int_equal(bc_core_time_compare(left, right), 0);
}

static void test_to_nanoseconds_simple(void** state)
{
    BC_UNUSED(state);
    bc_core_time_t input = {2, 500};
    uint64_t output = 0;
    assert_true(bc_core_time_to_nanoseconds(input, &output));
    assert_int_equal(output, 2U * (uint64_t)BC_CORE_TIME_NANOSECONDS_PER_SECOND + 500U);
}

static void test_to_nanoseconds_negative_returns_false(void** state)
{
    BC_UNUSED(state);
    bc_core_time_t input = {-1, 0};
    uint64_t output = 0;
    assert_false(bc_core_time_to_nanoseconds(input, &output));
}

static void test_to_nanoseconds_overflow(void** state)
{
    BC_UNUSED(state);
    bc_core_time_t input = {INT64_MAX, 0};
    uint64_t output = 0;
    assert_false(bc_core_time_to_nanoseconds(input, &output));
}

static void test_to_milliseconds_simple(void** state)
{
    BC_UNUSED(state);
    bc_core_time_t input = {3, 250U * BC_CORE_TIME_NANOSECONDS_PER_MILLISECOND};
    uint64_t output = 0;
    assert_true(bc_core_time_to_milliseconds(input, &output));
    assert_int_equal(output, 3250U);
}

static void test_to_milliseconds_negative_returns_false(void** state)
{
    BC_UNUSED(state);
    bc_core_time_t input = {-5, 0};
    uint64_t output = 0;
    assert_false(bc_core_time_to_milliseconds(input, &output));
}

static void test_to_milliseconds_overflow(void** state)
{
    BC_UNUSED(state);
    bc_core_time_t input = {INT64_MAX, 0};
    uint64_t output = 0;
    assert_false(bc_core_time_to_milliseconds(input, &output));
}

static void test_from_nanoseconds_round_trip(void** state)
{
    BC_UNUSED(state);
    const uint64_t source = 7U * (uint64_t)BC_CORE_TIME_NANOSECONDS_PER_SECOND + 1234U;
    const bc_core_time_t decoded = bc_core_time_from_nanoseconds(source);
    assert_int_equal(decoded.seconds, 7);
    assert_int_equal(decoded.nanoseconds, 1234);
    uint64_t encoded = 0;
    assert_true(bc_core_time_to_nanoseconds(decoded, &encoded));
    assert_int_equal(encoded, source);
}

static void test_from_milliseconds_round_trip(void** state)
{
    BC_UNUSED(state);
    const uint64_t source = 12345U;
    const bc_core_time_t decoded = bc_core_time_from_milliseconds(source);
    assert_int_equal(decoded.seconds, 12);
    assert_int_equal(decoded.nanoseconds, 345 * BC_CORE_TIME_NANOSECONDS_PER_MILLISECOND);
    uint64_t encoded = 0;
    assert_true(bc_core_time_to_milliseconds(decoded, &encoded));
    assert_int_equal(encoded, source);
}

static void test_format_iso_8601_utc_epoch(void** state)
{
    BC_UNUSED(state);
    char buffer[32];
    size_t length = 0;
    bc_core_time_t input = {0, 0};
    assert_true(bc_core_time_format_iso_8601_utc(buffer, sizeof(buffer), input, &length));
    assert_int_equal(length, 20);
    assert_memory_equal(buffer, "1970-01-01T00:00:00Z", 20);
}

static void test_format_iso_8601_utc_known_date(void** state)
{
    BC_UNUSED(state);
    char buffer[32];
    size_t length = 0;
    bc_core_time_t input = {1700000000, 0};
    assert_true(bc_core_time_format_iso_8601_utc(buffer, sizeof(buffer), input, &length));
    assert_int_equal(length, 20);
    assert_memory_equal(buffer, "2023-11-14T22:13:20Z", 20);
}

static void test_format_iso_8601_utc_buffer_too_small(void** state)
{
    BC_UNUSED(state);
    char buffer[10];
    size_t length = 0;
    bc_core_time_t input = {0, 0};
    assert_false(bc_core_time_format_iso_8601_utc(buffer, sizeof(buffer), input, &length));
}

static void test_parse_iso_8601_utc_round_trip(void** state)
{
    BC_UNUSED(state);
    const char* sample = "2023-11-14T22:13:20Z";
    bc_core_time_t parsed = {0, 0};
    size_t consumed = 0;
    assert_true(bc_core_time_parse_iso_8601_utc(sample, 20U, &parsed, &consumed));
    assert_int_equal(consumed, 20);
    assert_int_equal(parsed.seconds, 1700000000);
    assert_int_equal(parsed.nanoseconds, 0);
}

static void test_parse_iso_8601_utc_too_short(void** state)
{
    BC_UNUSED(state);
    const char* sample = "2023-11-14";
    bc_core_time_t parsed = {0, 0};
    size_t consumed = 0;
    assert_false(bc_core_time_parse_iso_8601_utc(sample, 10U, &parsed, &consumed));
}

static void test_parse_iso_8601_utc_missing_z_suffix(void** state)
{
    BC_UNUSED(state);
    const char* sample = "2023-11-14T22:13:20+";
    bc_core_time_t parsed = {0, 0};
    size_t consumed = 0;
    assert_false(bc_core_time_parse_iso_8601_utc(sample, 20U, &parsed, &consumed));
}

static void test_parse_iso_8601_utc_invalid_content(void** state)
{
    BC_UNUSED(state);
    const char* sample = "not-a-date-stringZ!!";
    bc_core_time_t parsed = {0, 0};
    size_t consumed = 0;
    assert_false(bc_core_time_parse_iso_8601_utc(sample, 20U, &parsed, &consumed));
}

static void test_format_utc_custom_pattern(void** state)
{
    BC_UNUSED(state);
    char buffer[64];
    size_t length = 0;
    bc_core_time_t input = {1700000000, 0};
    assert_true(bc_core_time_format_utc(buffer, sizeof(buffer), "%Y/%m/%d %H:%M", input, &length));
    assert_int_equal(length, 16);
    assert_memory_equal(buffer, "2023/11/14 22:13", 16);
}

static void test_format_utc_buffer_too_small(void** state)
{
    BC_UNUSED(state);
    char buffer[5];
    size_t length = 0;
    bc_core_time_t input = {1700000000, 0};
    assert_false(bc_core_time_format_utc(buffer, sizeof(buffer), "%Y-%m-%d", input, &length));
}

static void test_parse_utc_custom_pattern(void** state)
{
    BC_UNUSED(state);
    const char* sample = "2023/11/14 22:13:20";
    bc_core_time_t parsed = {0, 0};
    size_t consumed = 0;
    assert_true(bc_core_time_parse_utc(sample, 19U, "%Y/%m/%d %H:%M:%S", &parsed, &consumed));
    assert_int_equal(consumed, 19);
    assert_int_equal(parsed.seconds, 1700000000);
}

static void test_parse_utc_partial_consumption(void** state)
{
    BC_UNUSED(state);
    const char* sample = "2023-11-14 trailing junk";
    bc_core_time_t parsed = {0, 0};
    size_t consumed = 0;
    assert_true(bc_core_time_parse_utc(sample, 24U, "%Y-%m-%d", &parsed, &consumed));
    assert_int_equal(consumed, 10);
}

static void test_parse_utc_invalid_input(void** state)
{
    BC_UNUSED(state);
    const char* sample = "not-a-date";
    bc_core_time_t parsed = {0, 0};
    size_t consumed = 0;
    assert_false(bc_core_time_parse_utc(sample, 10U, "%Y-%m-%d", &parsed, &consumed));
}

static void test_parse_utc_zero_length(void** state)
{
    BC_UNUSED(state);
    bc_core_time_t parsed = {0, 0};
    size_t consumed = 0;
    assert_false(bc_core_time_parse_utc("", 0U, "%Y-%m-%d", &parsed, &consumed));
}

static void test_parse_utc_too_long(void** state)
{
    BC_UNUSED(state);
    char large_input[200];
    for (size_t index = 0; index < sizeof(large_input); index++) {
        large_input[index] = 'x';
    }
    bc_core_time_t parsed = {0, 0};
    size_t consumed = 0;
    assert_false(bc_core_time_parse_utc(large_input, sizeof(large_input), "%Y-%m-%d", &parsed, &consumed));
}

static void force_tz_utc(void)
{
    setenv("TZ", "UTC", 1);
    tzset();
}

static void test_format_local_known_date(void** state)
{
    BC_UNUSED(state);
    force_tz_utc();
    char buffer[64];
    size_t length = 0;
    bc_core_time_t input = {1700000000, 0};
    assert_true(bc_core_time_format_local(buffer, sizeof(buffer), "%Y-%m-%d %H:%M", input, &length));
    assert_int_equal(length, 16);
    assert_memory_equal(buffer, "2023-11-14 22:13", 16);
}

static void test_format_local_buffer_too_small(void** state)
{
    BC_UNUSED(state);
    force_tz_utc();
    char buffer[5];
    size_t length = 0;
    bc_core_time_t input = {1700000000, 0};
    assert_false(bc_core_time_format_local(buffer, sizeof(buffer), "%Y-%m-%d", input, &length));
}

static void test_format_local_round_trip_with_parse_utc(void** state)
{
    BC_UNUSED(state);
    force_tz_utc();
    char buffer[64];
    size_t length = 0;
    bc_core_time_t input = {1700000000, 0};
    assert_true(bc_core_time_format_local(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", input, &length));
    assert_int_equal(length, 19);

    bc_core_time_t parsed = {0, 0};
    size_t consumed = 0;
    assert_true(bc_core_time_parse_utc(buffer, length, "%Y-%m-%d %H:%M:%S", &parsed, &consumed));
    assert_int_equal(consumed, 19);
    assert_int_equal(parsed.seconds, input.seconds);
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_monotonic_now_returns_increasing_values),
        cmocka_unit_test(test_realtime_now_succeeds),
        cmocka_unit_test(test_sleep_milliseconds_zero_returns_immediately),
        cmocka_unit_test(test_sleep_nanoseconds_small),
        cmocka_unit_test(test_sleep_milliseconds_carry_to_seconds),
        cmocka_unit_test(test_sleep_nanoseconds_carry_to_seconds),
        cmocka_unit_test(test_normalize_carry_positive),
        cmocka_unit_test(test_normalize_carry_multiple_seconds),
        cmocka_unit_test(test_normalize_borrow_negative),
        cmocka_unit_test(test_normalize_borrow_multiple_seconds),
        cmocka_unit_test(test_normalize_already_normal),
        cmocka_unit_test(test_add_no_carry),
        cmocka_unit_test(test_add_with_carry),
        cmocka_unit_test(test_subtract_no_borrow),
        cmocka_unit_test(test_subtract_with_borrow),
        cmocka_unit_test(test_subtract_negative_result),
        cmocka_unit_test(test_compare_seconds_dominant),
        cmocka_unit_test(test_compare_nanoseconds_tiebreak),
        cmocka_unit_test(test_compare_equal),
        cmocka_unit_test(test_compare_normalizes_inputs),
        cmocka_unit_test(test_to_nanoseconds_simple),
        cmocka_unit_test(test_to_nanoseconds_negative_returns_false),
        cmocka_unit_test(test_to_nanoseconds_overflow),
        cmocka_unit_test(test_to_milliseconds_simple),
        cmocka_unit_test(test_to_milliseconds_negative_returns_false),
        cmocka_unit_test(test_to_milliseconds_overflow),
        cmocka_unit_test(test_from_nanoseconds_round_trip),
        cmocka_unit_test(test_from_milliseconds_round_trip),
        cmocka_unit_test(test_format_iso_8601_utc_epoch),
        cmocka_unit_test(test_format_iso_8601_utc_known_date),
        cmocka_unit_test(test_format_iso_8601_utc_buffer_too_small),
        cmocka_unit_test(test_parse_iso_8601_utc_round_trip),
        cmocka_unit_test(test_parse_iso_8601_utc_too_short),
        cmocka_unit_test(test_parse_iso_8601_utc_missing_z_suffix),
        cmocka_unit_test(test_parse_iso_8601_utc_invalid_content),
        cmocka_unit_test(test_format_utc_custom_pattern),
        cmocka_unit_test(test_format_utc_buffer_too_small),
        cmocka_unit_test(test_parse_utc_custom_pattern),
        cmocka_unit_test(test_parse_utc_partial_consumption),
        cmocka_unit_test(test_parse_utc_invalid_input),
        cmocka_unit_test(test_parse_utc_zero_length),
        cmocka_unit_test(test_parse_utc_too_long),
        cmocka_unit_test(test_format_local_known_date),
        cmocka_unit_test(test_format_local_buffer_too_small),
        cmocka_unit_test(test_format_local_round_trip_with_parse_utc),
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}
