// SPDX-License-Identifier: MIT

#include "bc_core.h"

#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void fuzz_uint64_dec(uint64_t value)
{
    char actual[21];
    size_t length = 0;
    if (!bc_core_format_unsigned_integer_64_decimal(actual, sizeof(actual), value, &length)) {
        abort();
    }
    char expected[32];
    int expected_length = snprintf(expected, sizeof(expected), "%" PRIu64, value);
    if (expected_length < 0) {
        abort();
    }
    if (length != (size_t)expected_length) {
        abort();
    }
    if (memcmp(actual, expected, length) != 0) {
        abort();
    }
}

static void fuzz_uint64_hex(uint64_t value)
{
    char actual[16];
    size_t length = 0;
    if (!bc_core_format_unsigned_integer_64_hexadecimal(actual, sizeof(actual), value, &length)) {
        abort();
    }
    char expected[32];
    int expected_length = snprintf(expected, sizeof(expected), "%" PRIx64, value);
    if (expected_length < 0) {
        abort();
    }
    if (length != (size_t)expected_length) {
        abort();
    }
    if (memcmp(actual, expected, length) != 0) {
        abort();
    }
}

static void fuzz_int64(int64_t value)
{
    char actual[22];
    size_t length = 0;
    if (!bc_core_format_signed_integer_64(actual, sizeof(actual), value, &length)) {
        abort();
    }
    char expected[32];
    int expected_length = snprintf(expected, sizeof(expected), "%" PRId64, value);
    if (expected_length < 0) {
        abort();
    }
    if (length != (size_t)expected_length) {
        abort();
    }
    if (memcmp(actual, expected, length) != 0) {
        abort();
    }
}

static void fuzz_double(double value, int frac_digits)
{
    if (frac_digits < 0 || frac_digits > 18) {
        return;
    }
    char buffer[64];
    size_t length = 0;
    (void)bc_core_format_double(buffer, sizeof(buffer), value, frac_digits, &length);
}

static void fuzz_bytes_human(uint64_t bytes)
{
    char buffer[32];
    size_t length = 0;
    if (!bc_core_format_bytes_human_readable(buffer, sizeof(buffer), bytes, &length)) {
        abort();
    }
}

static void fuzz_duration_ns(uint64_t nanoseconds)
{
    char buffer[32];
    size_t length = 0;
    if (!bc_core_format_duration_nanoseconds(buffer, sizeof(buffer), nanoseconds, &length)) {
        abort();
    }
}

int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if (size < 8) {
        return 0;
    }

    uint64_t value = 0;
    memcpy(&value, data, 8);

    fuzz_uint64_dec(value);
    fuzz_uint64_hex(value);
    fuzz_int64((int64_t)value);
    fuzz_bytes_human(value);
    fuzz_duration_ns(value);

    if (size >= 17) {
        double d = 0;
        memcpy(&d, data + 8, 8);
        int frac = (int)(data[16] % 19);
        fuzz_double(d, frac);
    }

    return 0;
}

#ifndef BC_FUZZ_LIBFUZZER
int main(int argc, char** argv)
{
    if (argc < 2) {
        fprintf(stderr, "usage: %s <iterations> [seed]\n", argv[0]);
        return 2;
    }
    unsigned long iterations = strtoul(argv[1], NULL, 10);
    unsigned long seed = (argc >= 3) ? strtoul(argv[2], NULL, 10) : 0;
    srand((unsigned int)seed);

    uint8_t buffer[32];
    for (unsigned long loop_index = 0; loop_index < iterations; loop_index++) {
        for (size_t byte_index = 0; byte_index < sizeof(buffer); byte_index++) {
            buffer[byte_index] = (uint8_t)(rand() & 0xFF);
        }
        LLVMFuzzerTestOneInput(buffer, sizeof(buffer));
    }
    return 0;
}
#endif
