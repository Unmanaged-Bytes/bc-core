// SPDX-License-Identifier: MIT

#include "bc_core_format.h"

#include <math.h>

bool bc_core_format_unsigned_integer_64_decimal(char* buffer, size_t capacity, uint64_t value, size_t* out_length)
{
    char scratch[21];
    size_t digit_count = 0;

    if (value == 0) {
        scratch[digit_count++] = '0';
    } else {
        uint64_t remaining = value;
        while (remaining > 0) {
            scratch[digit_count++] = (char)('0' + (remaining % 10U));
            remaining /= 10U;
        }
    }

    if (digit_count > capacity) {
        return false;
    }

    for (size_t source_index = 0; source_index < digit_count; source_index++) {
        buffer[source_index] = scratch[digit_count - 1U - source_index];
    }

    *out_length = digit_count;
    return true;
}

bool bc_core_format_unsigned_integer_64_hexadecimal(char* buffer, size_t capacity, uint64_t value, size_t* out_length)
{
    static const char digits[] = "0123456789abcdef";
    char scratch[16];
    size_t digit_count = 0;

    if (value == 0) {
        scratch[digit_count++] = '0';
    } else {
        uint64_t remaining = value;
        while (remaining > 0) {
            scratch[digit_count++] = digits[remaining & 0xFU];
            remaining >>= 4U;
        }
    }

    if (digit_count > capacity) {
        return false;
    }

    for (size_t source_index = 0; source_index < digit_count; source_index++) {
        buffer[source_index] = scratch[digit_count - 1U - source_index];
    }

    *out_length = digit_count;
    return true;
}

bool bc_core_format_unsigned_integer_64_hexadecimal_padded(char* buffer, size_t capacity, uint64_t value, size_t digits_requested,
                                                           size_t* out_length)
{
    static const char digits[] = "0123456789abcdef";

    if (digits_requested == 0 || digits_requested > 16U) {
        return false;
    }
    if (digits_requested > capacity) {
        return false;
    }

    for (size_t output_index = 0; output_index < digits_requested; output_index++) {
        size_t shift = (digits_requested - 1U - output_index) * 4U;
        buffer[output_index] = digits[(value >> shift) & 0xFU];
    }

    *out_length = digits_requested;
    return true;
}

bool bc_core_format_signed_integer_64(char* buffer, size_t capacity, int64_t value, size_t* out_length)
{
    if (value >= 0) {
        return bc_core_format_unsigned_integer_64_decimal(buffer, capacity, (uint64_t)value, out_length);
    }

    if (capacity < 1U) {
        return false;
    }

    buffer[0] = '-';

    uint64_t magnitude;
    if (value == INT64_MIN) {
        magnitude = (uint64_t)INT64_MAX + 1U;
    } else {
        magnitude = (uint64_t)(-value);
    }

    size_t digits_length = 0;
    if (!bc_core_format_unsigned_integer_64_decimal(buffer + 1, capacity - 1U, magnitude, &digits_length)) {
        return false;
    }

    *out_length = 1U + digits_length;
    return true;
}

bool bc_core_format_double(char* buffer, size_t capacity, double value, int frac_digits, size_t* out_length)
{
    if (frac_digits < 0 || frac_digits > 18) {
        return false;
    }

    if (isnan(value)) {
        if (capacity < 3U) {
            return false;
        }
        buffer[0] = 'n';
        buffer[1] = 'a';
        buffer[2] = 'n';
        *out_length = 3U;
        return true;
    }

    bool negative = value < 0.0 || (value == 0.0 && signbit(value));
    double magnitude = negative ? -value : value;

    if (isinf(magnitude)) {
        size_t needed = negative ? 4U : 3U;
        if (capacity < needed) {
            return false;
        }
        size_t position = 0;
        if (negative) {
            buffer[position++] = '-';
        }
        buffer[position++] = 'i';
        buffer[position++] = 'n';
        buffer[position++] = 'f';
        *out_length = position;
        return true;
    }

    double scale = 1.0;
    for (int scale_index = 0; scale_index < frac_digits; scale_index++) {
        scale *= 10.0;
    }

    double scaled = magnitude * scale + 0.5;
    if (scaled >= (double)UINT64_MAX) {
        return false;
    }

    uint64_t scaled_integer = (uint64_t)scaled;
    uint64_t integer_part;
    uint64_t fractional_part;
    if (frac_digits == 0) {
        integer_part = scaled_integer;
        fractional_part = 0;
    } else {
        uint64_t divisor = 1;
        for (int divisor_index = 0; divisor_index < frac_digits; divisor_index++) {
            divisor *= 10U;
        }
        integer_part = scaled_integer / divisor;
        fractional_part = scaled_integer % divisor;
    }

    size_t position = 0;
    if (negative) {
        if (capacity < 1U) {
            return false;
        }
        buffer[position++] = '-';
    }

    size_t integer_length = 0;
    if (!bc_core_format_unsigned_integer_64_decimal(buffer + position, capacity - position, integer_part, &integer_length)) {
        return false;
    }
    position += integer_length;

    if (frac_digits > 0) {
        if (position >= capacity) {
            return false;
        }
        buffer[position++] = '.';

        if ((size_t)frac_digits > capacity - position) {
            return false;
        }

        uint64_t remaining = fractional_part;
        for (int fraction_index = 0; fraction_index < frac_digits; fraction_index++) {
            size_t write_index = position + (size_t)(frac_digits - 1 - fraction_index);
            buffer[write_index] = (char)('0' + (remaining % 10U));
            remaining /= 10U;
        }
        position += (size_t)frac_digits;
    }

    *out_length = position;
    return true;
}

bool bc_core_format_bytes_human_readable(char* buffer, size_t capacity, uint64_t bytes, size_t* out_length)
{
    static const char* const unit_labels[] = {"B", "KB", "MB", "GB", "TB", "PB", "EB"};
    const size_t unit_count = sizeof(unit_labels) / sizeof(unit_labels[0]);

    size_t unit_index = 0;
    double display_value = (double)bytes;
    while (display_value >= 1024.0 && unit_index + 1U < unit_count) {
        display_value /= 1024.0;
        unit_index++;
    }

    int frac_digits;
    if (unit_index == 0) {
        frac_digits = 0;
    } else if (display_value >= 100.0) {
        frac_digits = 0;
    } else if (display_value >= 10.0) {
        frac_digits = 1;
    } else {
        frac_digits = 2;
    }

    size_t value_length = 0;
    if (!bc_core_format_double(buffer, capacity, display_value, frac_digits, &value_length)) {
        return false;
    }

    size_t position = value_length;
    if (position >= capacity) {
        return false;
    }
    buffer[position++] = ' ';

    size_t label_index = 0;
    while (unit_labels[unit_index][label_index] != '\0') {
        if (position >= capacity) {
            return false;
        }
        buffer[position++] = unit_labels[unit_index][label_index];
        label_index++;
    }

    *out_length = position;
    return true;
}

bool bc_core_format_duration_nanoseconds(char* buffer, size_t capacity, uint64_t nanoseconds, size_t* out_length)
{
    static const char* const unit_labels[] = {"ns", "us", "ms", "s"};
    static const uint64_t unit_thresholds[] = {1000U, 1000U * 1000U, 1000U * 1000U * 1000U};

    double display_value = (double)nanoseconds;
    size_t unit_index = 0;
    while (unit_index < 3U && nanoseconds >= unit_thresholds[unit_index]) {
        unit_index++;
    }

    double divisor = 1.0;
    for (size_t step_index = 0; step_index < unit_index; step_index++) {
        divisor *= 1000.0;
    }
    display_value /= divisor;

    int frac_digits;
    if (unit_index == 0) {
        frac_digits = 0;
    } else if (display_value >= 100.0) {
        frac_digits = 1;
    } else if (display_value >= 10.0) {
        frac_digits = 2;
    } else {
        frac_digits = 3;
    }

    size_t value_length = 0;
    if (!bc_core_format_double(buffer, capacity, display_value, frac_digits, &value_length)) {
        return false;
    }

    size_t position = value_length;
    size_t label_index = 0;
    while (unit_labels[unit_index][label_index] != '\0') {
        if (position >= capacity) {
            return false;
        }
        buffer[position++] = unit_labels[unit_index][label_index];
        label_index++;
    }

    *out_length = position;
    return true;
}

static void write_four_uppercase_hex(char* destination, uint32_t value)
{
    static const char digits[] = "0123456789ABCDEF";
    destination[0] = digits[(value >> 12) & 0xFU];
    destination[1] = digits[(value >> 8) & 0xFU];
    destination[2] = digits[(value >> 4) & 0xFU];
    destination[3] = digits[value & 0xFU];
}

bool bc_core_format_unicode_codepoint_escape(char* buffer, size_t capacity, uint32_t codepoint, size_t* out_length)
{
    if (buffer == NULL || out_length == NULL) {
        return false;
    }
    if (codepoint > 0x10FFFFU) {
        return false;
    }
    if (codepoint >= 0xD800U && codepoint <= 0xDFFFU) {
        return false;
    }

    if (codepoint <= 0xFFFFU) {
        if (capacity < 6U) {
            return false;
        }
        buffer[0] = '\\';
        buffer[1] = 'u';
        write_four_uppercase_hex(buffer + 2, codepoint);
        *out_length = 6U;
        return true;
    }

    if (capacity < 12U) {
        return false;
    }
    uint32_t adjusted = codepoint - 0x10000U;
    uint32_t high_surrogate = ((adjusted >> 10) & 0x3FFU) + 0xD800U;
    uint32_t low_surrogate = (adjusted & 0x3FFU) + 0xDC00U;
    buffer[0] = '\\';
    buffer[1] = 'u';
    write_four_uppercase_hex(buffer + 2, high_surrogate);
    buffer[6] = '\\';
    buffer[7] = 'u';
    write_four_uppercase_hex(buffer + 8, low_surrogate);
    *out_length = 12U;
    return true;
}
