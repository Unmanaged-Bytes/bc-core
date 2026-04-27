// SPDX-License-Identifier: MIT

#include "bc_core_parse.h"

#include <stdint.h>

bool bc_core_parse_unsigned_integer_64_decimal(const char* text, size_t length, uint64_t* out_value, size_t* out_consumed)
{
    if (length == 0) {
        *out_value = 0;
        *out_consumed = 0;
        return false;
    }

    uint64_t value = 0;
    size_t index = 0;

    while (index < length) {
        unsigned char byte = (unsigned char)text[index];
        if (byte < '0' || byte > '9') {
            break;
        }
        unsigned int digit = (unsigned int)(byte - '0');
        if (value > (UINT64_MAX - digit) / 10ULL) {
            *out_value = 0;
            *out_consumed = 0;
            return false;
        }
        value = value * 10ULL + (uint64_t)digit;
        index += 1;
    }

    if (index == 0) {
        *out_value = 0;
        *out_consumed = 0;
        return false;
    }

    *out_value = value;
    *out_consumed = index;
    return true;
}

bool bc_core_parse_signed_integer_64_decimal(const char* text, size_t length, int64_t* out_value, size_t* out_consumed)
{
    if (length == 0) {
        *out_value = 0;
        *out_consumed = 0;
        return false;
    }

    bool negative = false;
    size_t prefix_length = 0;
    if (text[0] == '-') {
        negative = true;
        prefix_length = 1;
    } else if (text[0] == '+') {
        prefix_length = 1;
    }

    if (prefix_length >= length) {
        *out_value = 0;
        *out_consumed = 0;
        return false;
    }

    uint64_t magnitude = 0;
    size_t magnitude_consumed = 0;
    if (!bc_core_parse_unsigned_integer_64_decimal(text + prefix_length, length - prefix_length, &magnitude, &magnitude_consumed)) {
        *out_value = 0;
        *out_consumed = 0;
        return false;
    }

    if (negative) {
        if (magnitude > (uint64_t)INT64_MAX + 1ULL) {
            *out_value = 0;
            *out_consumed = 0;
            return false;
        }
        if (magnitude == (uint64_t)INT64_MAX + 1ULL) {
            *out_value = INT64_MIN;
        } else {
            *out_value = -(int64_t)magnitude;
        }
    } else {
        if (magnitude > (uint64_t)INT64_MAX) {
            *out_value = 0;
            *out_consumed = 0;
            return false;
        }
        *out_value = (int64_t)magnitude;
    }

    *out_consumed = prefix_length + magnitude_consumed;
    return true;
}
