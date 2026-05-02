// SPDX-License-Identifier: MIT

#ifndef BC_CORE_TIME_H
#define BC_CORE_TIME_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define BC_CORE_TIME_NANOSECONDS_PER_SECOND 1000000000
#define BC_CORE_TIME_NANOSECONDS_PER_MILLISECOND 1000000
#define BC_CORE_TIME_NANOSECONDS_PER_MICROSECOND 1000
#define BC_CORE_TIME_MILLISECONDS_PER_SECOND 1000
#define BC_CORE_TIME_MICROSECONDS_PER_SECOND 1000000

typedef struct {
    int64_t seconds;
    int32_t nanoseconds;
} bc_core_time_t;

bool bc_core_time_monotonic_now(bc_core_time_t* out_time);
bool bc_core_time_realtime_now(bc_core_time_t* out_time);

bool bc_core_time_sleep_milliseconds(uint64_t milliseconds);
bool bc_core_time_sleep_nanoseconds(uint64_t nanoseconds);

bc_core_time_t bc_core_time_normalize(bc_core_time_t time);
bc_core_time_t bc_core_time_add(bc_core_time_t left, bc_core_time_t right);
bc_core_time_t bc_core_time_subtract(bc_core_time_t end, bc_core_time_t start);
int bc_core_time_compare(bc_core_time_t left, bc_core_time_t right);

bool bc_core_time_to_nanoseconds(bc_core_time_t time, uint64_t* out_nanoseconds);
bool bc_core_time_to_milliseconds(bc_core_time_t time, uint64_t* out_milliseconds);
bc_core_time_t bc_core_time_from_nanoseconds(uint64_t nanoseconds);
bc_core_time_t bc_core_time_from_milliseconds(uint64_t milliseconds);

bool bc_core_time_format_iso_8601_utc(char* buffer, size_t capacity, bc_core_time_t time, size_t* out_length);
bool bc_core_time_parse_iso_8601_utc(const char* text, size_t length, bc_core_time_t* out_time, size_t* out_consumed);

bool bc_core_time_format_utc(char* buffer, size_t capacity, const char* format_pattern, bc_core_time_t time, size_t* out_length);
bool bc_core_time_parse_utc(const char* text, size_t length, const char* format_pattern, bc_core_time_t* out_time, size_t* out_consumed);

#endif /* BC_CORE_TIME_H */
