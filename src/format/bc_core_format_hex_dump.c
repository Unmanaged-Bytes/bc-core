// SPDX-License-Identifier: MIT

#include "bc_core_format.h"

#include <stdint.h>

static const char BC_CORE_HEX_DIGITS[] = "0123456789abcdef";

static const bc_core_format_hex_dump_options_t BC_CORE_HEX_DUMP_DEFAULT = {16u, true, true};

static const bc_core_format_hex_dump_options_t* bc_core_hex_dump_resolve(const bc_core_format_hex_dump_options_t* options)
{
    return options != NULL ? options : &BC_CORE_HEX_DUMP_DEFAULT;
}

static size_t bc_core_hex_dump_line_length(size_t bytes_on_line, const bc_core_format_hex_dump_options_t* options)
{
    size_t length = 0;
    if (options->show_offset) {
        length += 8u + 2u;
    }
    length += options->bytes_per_line * 3u;
    length += 1u;
    if (options->show_ascii) {
        length += 1u + options->bytes_per_line + 1u;
    }
    length += 1u;
    (void)bytes_on_line;
    return length;
}

size_t bc_core_format_hex_dump_required_capacity(size_t input_length, const bc_core_format_hex_dump_options_t* options)
{
    const bc_core_format_hex_dump_options_t* resolved = bc_core_hex_dump_resolve(options);
    if (resolved->bytes_per_line == 0u) {
        return 0u;
    }
    if (input_length == 0u) {
        return 0u;
    }
    const size_t line_count = (input_length + resolved->bytes_per_line - 1u) / resolved->bytes_per_line;
    return line_count * bc_core_hex_dump_line_length(resolved->bytes_per_line, resolved);
}

static bool bc_core_hex_dump_emit_offset(uint64_t offset, char* output, size_t capacity, size_t* position)
{
    if (*position + 8u + 2u > capacity) {
        return false;
    }
    for (int digit_index = 7; digit_index >= 0; digit_index--) {
        const unsigned int shift_amount = (unsigned int)(digit_index * 4);
        output[*position] = BC_CORE_HEX_DIGITS[(offset >> shift_amount) & 0xFu];
        *position += 1;
    }
    output[*position] = ' ';
    *position += 1;
    output[*position] = ' ';
    *position += 1;
    return true;
}

static bool bc_core_hex_dump_emit_byte_hex(uint8_t value, char* output, size_t capacity, size_t* position)
{
    if (*position + 3u > capacity) {
        return false;
    }
    output[*position] = BC_CORE_HEX_DIGITS[value >> 4];
    output[*position + 1] = BC_CORE_HEX_DIGITS[value & 0xFu];
    output[*position + 2] = ' ';
    *position += 3;
    return true;
}

static bool bc_core_hex_dump_emit_padding_hex(char* output, size_t capacity, size_t* position)
{
    if (*position + 3u > capacity) {
        return false;
    }
    output[*position] = ' ';
    output[*position + 1] = ' ';
    output[*position + 2] = ' ';
    *position += 3;
    return true;
}

bool bc_core_format_hex_dump(const void* input, size_t input_length, uint64_t base_offset, const bc_core_format_hex_dump_options_t* options,
                             char* output, size_t output_capacity, size_t* out_length)
{
    const bc_core_format_hex_dump_options_t* resolved = bc_core_hex_dump_resolve(options);
    if (resolved->bytes_per_line == 0u) {
        return false;
    }
    if (input_length == 0u) {
        *out_length = 0;
        return true;
    }
    const uint8_t* source = (const uint8_t*)input;
    size_t position = 0;
    size_t read_index = 0;
    while (read_index < input_length) {
        const uint64_t line_offset = base_offset + (uint64_t)read_index;
        if (resolved->show_offset && !bc_core_hex_dump_emit_offset(line_offset, output, output_capacity, &position)) {
            return false;
        }
        const size_t bytes_remaining = input_length - read_index;
        const size_t bytes_on_line = bytes_remaining < resolved->bytes_per_line ? bytes_remaining : resolved->bytes_per_line;
        for (size_t byte_index = 0; byte_index < resolved->bytes_per_line; byte_index++) {
            if (byte_index < bytes_on_line) {
                if (!bc_core_hex_dump_emit_byte_hex(source[read_index + byte_index], output, output_capacity, &position)) {
                    return false;
                }
            } else {
                if (!bc_core_hex_dump_emit_padding_hex(output, output_capacity, &position)) {
                    return false;
                }
            }
        }
        if (position >= output_capacity) {
            return false;
        }
        output[position++] = ' ';
        if (resolved->show_ascii) {
            if (position + 1u + bytes_on_line + 1u > output_capacity) {
                return false;
            }
            output[position++] = '|';
            for (size_t byte_index = 0; byte_index < bytes_on_line; byte_index++) {
                const uint8_t value = source[read_index + byte_index];
                output[position++] = (value >= 0x20u && value < 0x7Fu) ? (char)value : '.';
            }
            output[position++] = '|';
        }
        if (position >= output_capacity) {
            return false;
        }
        output[position++] = '\n';
        read_index += bytes_on_line;
    }
    *out_length = position;
    return true;
}
