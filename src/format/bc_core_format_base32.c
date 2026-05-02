// SPDX-License-Identifier: MIT

#include "bc_core_format.h"

#include <stdint.h>

static const char BC_CORE_BASE32_ALPHABET[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ234567";

static int bc_core_base32_decode_char(char input)
{
    if (input >= 'A' && input <= 'Z') {
        return (int)(input - 'A');
    }
    if (input >= 'a' && input <= 'z') {
        return (int)(input - 'a');
    }
    if (input >= '2' && input <= '7') {
        return (int)(input - '2') + 26;
    }
    return -1;
}

size_t bc_core_format_base32_encoded_length(size_t input_length)
{
    return ((input_length + 4u) / 5u) * 8u;
}

size_t bc_core_format_base32_decoded_length_max(size_t input_length)
{
    return (input_length / 8u) * 5u;
}

static const size_t BC_CORE_BASE32_PADDING_FOR_REMAINDER[6] = {0, 6, 4, 3, 1, 0};
static const size_t BC_CORE_BASE32_BYTES_FOR_QUINTET[9] = {0, 0, 1, 0, 2, 3, 0, 4, 5};

bool bc_core_format_base32_encode(const void* input, size_t input_length, char* output, size_t output_capacity, size_t* out_length)
{
    const size_t required = bc_core_format_base32_encoded_length(input_length);
    if (required > output_capacity) {
        return false;
    }
    const uint8_t* source = (const uint8_t*)input;
    size_t read_index = 0;
    size_t write_index = 0;
    while (read_index + 5u <= input_length) {
        const uint64_t value = ((uint64_t)source[read_index] << 32) | ((uint64_t)source[read_index + 1] << 24) |
                               ((uint64_t)source[read_index + 2] << 16) | ((uint64_t)source[read_index + 3] << 8) |
                               (uint64_t)source[read_index + 4];
        for (int shift_step = 0; shift_step < 8; shift_step++) {
            const unsigned int shift_amount = (unsigned int)(35 - shift_step * 5);
            output[write_index + (size_t)shift_step] = BC_CORE_BASE32_ALPHABET[(value >> shift_amount) & 0x1Fu];
        }
        read_index += 5;
        write_index += 8;
    }
    const size_t remaining = input_length - read_index;
    if (remaining > 0u) {
        uint64_t accumulator = 0;
        for (size_t byte_index = 0; byte_index < remaining; byte_index++) {
            accumulator |= (uint64_t)source[read_index + byte_index] << ((4u - byte_index) * 8u);
        }
        const size_t total_chars = 8u - BC_CORE_BASE32_PADDING_FOR_REMAINDER[remaining];
        for (size_t char_index = 0; char_index < total_chars; char_index++) {
            const unsigned int shift_amount = (unsigned int)(35u - char_index * 5u);
            output[write_index + char_index] = BC_CORE_BASE32_ALPHABET[(accumulator >> shift_amount) & 0x1Fu];
        }
        for (size_t pad_index = total_chars; pad_index < 8u; pad_index++) {
            output[write_index + pad_index] = '=';
        }
        write_index += 8;
    }
    *out_length = write_index;
    return true;
}

bool bc_core_format_base32_decode(const char* input, size_t input_length, void* output, size_t output_capacity, size_t* out_length)
{
    if ((input_length % 8u) != 0u) {
        return false;
    }
    if (input_length == 0u) {
        *out_length = 0;
        return true;
    }
    size_t padding = 0;
    while (padding < 8u && input[input_length - 1u - padding] == '=') {
        padding += 1;
    }
    if (padding == 8u || padding == 1u || padding == 2u || padding == 5u) {
        return false;
    }
    const size_t valid_chars_per_block = 8u - padding;
    const size_t bytes_in_last_block = BC_CORE_BASE32_BYTES_FOR_QUINTET[valid_chars_per_block];
    const size_t expected_output_length = (input_length / 8u - 1u) * 5u + bytes_in_last_block;
    if (expected_output_length > output_capacity) {
        return false;
    }
    uint8_t* destination = (uint8_t*)output;
    size_t read_index = 0;
    size_t write_index = 0;
    while (read_index + 8u <= input_length) {
        const bool last_block = (read_index + 8u) == input_length;
        const size_t valid = last_block ? valid_chars_per_block : 8u;
        const size_t emit = last_block ? bytes_in_last_block : 5u;
        uint64_t accumulator = 0;
        for (size_t char_index = 0; char_index < valid; char_index++) {
            const int decoded = bc_core_base32_decode_char(input[read_index + char_index]);
            if (decoded < 0) {
                return false;
            }
            accumulator = (accumulator << 5) | (uint64_t)decoded;
        }
        accumulator <<= (8u - valid) * 5u;
        for (size_t byte_index = 0; byte_index < emit; byte_index++) {
            const unsigned int shift_amount = (unsigned int)(32u - byte_index * 8u);
            destination[write_index + byte_index] = (uint8_t)((accumulator >> shift_amount) & 0xFFu);
        }
        write_index += emit;
        read_index += 8;
    }
    *out_length = write_index;
    return true;
}
