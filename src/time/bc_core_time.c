// SPDX-License-Identifier: MIT

#include "bc_core_time.h"

#include <errno.h>
#include <time.h>

static const int64_t BC_CORE_TIME_NS_PER_SEC = BC_CORE_TIME_NANOSECONDS_PER_SECOND;

static bool bc_core_time_capture(clockid_t clock_id, bc_core_time_t* out_time)
{
    struct timespec captured = {0, 0};
    if (clock_gettime(clock_id, &captured) != 0) {
        return false;
    }
    out_time->seconds = (int64_t)captured.tv_sec;
    out_time->nanoseconds = (int32_t)captured.tv_nsec;
    return true;
}

bool bc_core_time_monotonic_now(bc_core_time_t* out_time)
{
    return bc_core_time_capture(CLOCK_MONOTONIC, out_time);
}

bool bc_core_time_realtime_now(bc_core_time_t* out_time)
{
    return bc_core_time_capture(CLOCK_REALTIME, out_time);
}

static bool bc_core_time_sleep_timespec(struct timespec requested)
{
    struct timespec remaining = {0, 0};
    while (nanosleep(&requested, &remaining) != 0) {
        if (errno != EINTR) {
            return false;
        }
        requested = remaining;
    }
    return true;
}

bool bc_core_time_sleep_milliseconds(uint64_t milliseconds)
{
    struct timespec requested;
    requested.tv_sec = (time_t)(milliseconds / (uint64_t)BC_CORE_TIME_MILLISECONDS_PER_SECOND);
    const uint64_t remainder_ms = milliseconds % (uint64_t)BC_CORE_TIME_MILLISECONDS_PER_SECOND;
    requested.tv_nsec = (long)(remainder_ms * (uint64_t)BC_CORE_TIME_NANOSECONDS_PER_MILLISECOND);
    return bc_core_time_sleep_timespec(requested);
}

bool bc_core_time_sleep_nanoseconds(uint64_t nanoseconds)
{
    struct timespec requested;
    requested.tv_sec = (time_t)(nanoseconds / (uint64_t)BC_CORE_TIME_NANOSECONDS_PER_SECOND);
    requested.tv_nsec = (long)(nanoseconds % (uint64_t)BC_CORE_TIME_NANOSECONDS_PER_SECOND);
    return bc_core_time_sleep_timespec(requested);
}

bc_core_time_t bc_core_time_normalize(bc_core_time_t time)
{
    bc_core_time_t normalized = time;
    if (normalized.nanoseconds >= (int32_t)BC_CORE_TIME_NS_PER_SEC) {
        const int64_t carry_seconds = normalized.nanoseconds / (int32_t)BC_CORE_TIME_NS_PER_SEC;
        normalized.seconds += carry_seconds;
        normalized.nanoseconds -= (int32_t)(carry_seconds * BC_CORE_TIME_NS_PER_SEC);
    } else if (normalized.nanoseconds < 0) {
        const int64_t borrow_seconds = ((-(int64_t)normalized.nanoseconds) + BC_CORE_TIME_NS_PER_SEC - 1) / BC_CORE_TIME_NS_PER_SEC;
        normalized.seconds -= borrow_seconds;
        normalized.nanoseconds += (int32_t)(borrow_seconds * BC_CORE_TIME_NS_PER_SEC);
    }
    return normalized;
}

bc_core_time_t bc_core_time_add(bc_core_time_t left, bc_core_time_t right)
{
    bc_core_time_t result;
    result.seconds = left.seconds + right.seconds;
    result.nanoseconds = left.nanoseconds + right.nanoseconds;
    return bc_core_time_normalize(result);
}

bc_core_time_t bc_core_time_subtract(bc_core_time_t end, bc_core_time_t start)
{
    bc_core_time_t result;
    result.seconds = end.seconds - start.seconds;
    result.nanoseconds = end.nanoseconds - start.nanoseconds;
    return bc_core_time_normalize(result);
}

int bc_core_time_compare(bc_core_time_t left, bc_core_time_t right)
{
    const bc_core_time_t left_normalized = bc_core_time_normalize(left);
    const bc_core_time_t right_normalized = bc_core_time_normalize(right);
    if (left_normalized.seconds < right_normalized.seconds) {
        return -1;
    }
    if (left_normalized.seconds > right_normalized.seconds) {
        return 1;
    }
    if (left_normalized.nanoseconds < right_normalized.nanoseconds) {
        return -1;
    }
    if (left_normalized.nanoseconds > right_normalized.nanoseconds) {
        return 1;
    }
    return 0;
}

bool bc_core_time_to_nanoseconds(bc_core_time_t time, uint64_t* out_nanoseconds)
{
    const bc_core_time_t normalized = bc_core_time_normalize(time);
    if (normalized.seconds < 0) {
        return false;
    }
    const uint64_t maximum_seconds = UINT64_MAX / (uint64_t)BC_CORE_TIME_NS_PER_SEC;
    const uint64_t seconds_unsigned = (uint64_t)normalized.seconds;
    if (seconds_unsigned > maximum_seconds) {
        return false;
    }
    const uint64_t scaled = seconds_unsigned * (uint64_t)BC_CORE_TIME_NS_PER_SEC;
    const uint64_t nanoseconds_unsigned = (uint64_t)normalized.nanoseconds;
    if (UINT64_MAX - scaled < nanoseconds_unsigned) {
        return false;
    }
    *out_nanoseconds = scaled + nanoseconds_unsigned;
    return true;
}

bool bc_core_time_to_milliseconds(bc_core_time_t time, uint64_t* out_milliseconds)
{
    const bc_core_time_t normalized = bc_core_time_normalize(time);
    if (normalized.seconds < 0) {
        return false;
    }
    const uint64_t maximum_seconds = UINT64_MAX / (uint64_t)BC_CORE_TIME_MILLISECONDS_PER_SECOND;
    const uint64_t seconds_unsigned = (uint64_t)normalized.seconds;
    if (seconds_unsigned > maximum_seconds) {
        return false;
    }
    const uint64_t scaled = seconds_unsigned * (uint64_t)BC_CORE_TIME_MILLISECONDS_PER_SECOND;
    const uint64_t milliseconds_part = (uint64_t)normalized.nanoseconds / (uint64_t)BC_CORE_TIME_NANOSECONDS_PER_MILLISECOND;
    if (UINT64_MAX - scaled < milliseconds_part) {
        return false;
    }
    *out_milliseconds = scaled + milliseconds_part;
    return true;
}

bc_core_time_t bc_core_time_from_nanoseconds(uint64_t nanoseconds)
{
    bc_core_time_t result;
    result.seconds = (int64_t)(nanoseconds / (uint64_t)BC_CORE_TIME_NS_PER_SEC);
    result.nanoseconds = (int32_t)(nanoseconds % (uint64_t)BC_CORE_TIME_NS_PER_SEC);
    return result;
}

bc_core_time_t bc_core_time_from_milliseconds(uint64_t milliseconds)
{
    bc_core_time_t result;
    result.seconds = (int64_t)(milliseconds / (uint64_t)BC_CORE_TIME_MILLISECONDS_PER_SECOND);
    const uint64_t remainder_ms = milliseconds % (uint64_t)BC_CORE_TIME_MILLISECONDS_PER_SECOND;
    result.nanoseconds = (int32_t)(remainder_ms * (uint64_t)BC_CORE_TIME_NANOSECONDS_PER_MILLISECOND);
    return result;
}

static bool bc_core_time_to_struct_tm_utc(bc_core_time_t time, struct tm* out_struct)
{
    const bc_core_time_t normalized = bc_core_time_normalize(time);
    const time_t epoch_seconds = (time_t)normalized.seconds;
    if ((int64_t)epoch_seconds != normalized.seconds) {
        return false;
    }
    return gmtime_r(&epoch_seconds, out_struct) != NULL;
}

bool bc_core_time_format_iso_8601_utc(char* buffer, size_t capacity, bc_core_time_t time, size_t* out_length)
{
    struct tm broken_down;
    if (!bc_core_time_to_struct_tm_utc(time, &broken_down)) {
        return false;
    }
    const size_t written = strftime(buffer, capacity, "%Y-%m-%dT%H:%M:%SZ", &broken_down);
    if (written == 0) {
        return false;
    }
    *out_length = written;
    return true;
}

static bool bc_core_time_struct_tm_to_seconds_utc(const struct tm* broken_down, int64_t* out_seconds)
{
    struct tm copy = *broken_down;
    copy.tm_isdst = 0;
    const time_t encoded = timegm(&copy);
    if (encoded == (time_t)-1) {
        return false;
    }
    *out_seconds = (int64_t)encoded;
    return true;
}

bool bc_core_time_parse_iso_8601_utc(const char* text, size_t length, bc_core_time_t* out_time, size_t* out_consumed)
{
    if (length < 20U) {
        return false;
    }
    if (text[19] != 'Z') {
        return false;
    }
    char zero_terminated[21];
    for (size_t index = 0; index < 20U; index++) {
        zero_terminated[index] = text[index];
    }
    zero_terminated[20] = '\0';
    struct tm broken_down = {0};
    const char* tail = strptime(zero_terminated, "%Y-%m-%dT%H:%M:%SZ", &broken_down);
    if (tail == NULL || *tail != '\0') {
        return false;
    }
    int64_t encoded_seconds = 0;
    if (!bc_core_time_struct_tm_to_seconds_utc(&broken_down, &encoded_seconds)) {
        return false;
    }
    out_time->seconds = encoded_seconds;
    out_time->nanoseconds = 0;
    *out_consumed = 20U;
    return true;
}

bool bc_core_time_format_utc(char* buffer, size_t capacity, const char* format_pattern, bc_core_time_t time, size_t* out_length)
{
    struct tm broken_down;
    if (!bc_core_time_to_struct_tm_utc(time, &broken_down)) {
        return false;
    }
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-nonliteral"
    const size_t written = strftime(buffer, capacity, format_pattern, &broken_down);
#pragma GCC diagnostic pop
    if (written == 0) {
        return false;
    }
    *out_length = written;
    return true;
}

bool bc_core_time_parse_utc(const char* text, size_t length, const char* format_pattern, bc_core_time_t* out_time, size_t* out_consumed)
{
    if (length == 0) {
        return false;
    }
    char stack_buffer[128];
    if (length >= sizeof(stack_buffer)) {
        return false;
    }
    for (size_t index = 0; index < length; index++) {
        stack_buffer[index] = text[index];
    }
    stack_buffer[length] = '\0';
    struct tm broken_down = {0};
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-nonliteral"
    const char* tail = strptime(stack_buffer, format_pattern, &broken_down);
#pragma GCC diagnostic pop
    if (tail == NULL) {
        return false;
    }
    int64_t encoded_seconds = 0;
    if (!bc_core_time_struct_tm_to_seconds_utc(&broken_down, &encoded_seconds)) {
        return false;
    }
    out_time->seconds = encoded_seconds;
    out_time->nanoseconds = 0;
    *out_consumed = (size_t)(tail - stack_buffer);
    return true;
}

static bool bc_core_time_to_struct_tm_local(bc_core_time_t time, struct tm* out_struct)
{
    const bc_core_time_t normalized = bc_core_time_normalize(time);
    const time_t epoch_seconds = (time_t)normalized.seconds;
    if ((int64_t)epoch_seconds != normalized.seconds) {
        return false;
    }
    return localtime_r(&epoch_seconds, out_struct) != NULL;
}

bool bc_core_time_format_local(char* buffer, size_t capacity, const char* format_pattern, bc_core_time_t time, size_t* out_length)
{
    struct tm broken_down;
    if (!bc_core_time_to_struct_tm_local(time, &broken_down)) {
        return false;
    }
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-nonliteral"
    const size_t written = strftime(buffer, capacity, format_pattern, &broken_down);
#pragma GCC diagnostic pop
    if (written == 0) {
        return false;
    }
    *out_length = written;
    return true;
}
