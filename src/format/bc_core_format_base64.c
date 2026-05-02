// SPDX-License-Identifier: MIT

#include "bc_core_format.h"

#include <stdint.h>

static const char BC_CORE_BASE64_ALPHABET[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static int bc_core_base64_decode_char(char input)
{
    if (input >= 'A' && input <= 'Z') {
        return (int)(input - 'A');
    }
    if (input >= 'a' && input <= 'z') {
        return (int)(input - 'a') + 26;
    }
    if (input >= '0' && input <= '9') {
        return (int)(input - '0') + 52;
    }
    if (input == '+') {
        return 62;
    }
    if (input == '/') {
        return 63;
    }
    return -1;
}

size_t bc_core_format_base64_encoded_length(size_t input_length)
{
    return ((input_length + 2u) / 3u) * 4u;
}

size_t bc_core_format_base64_decoded_length_max(size_t input_length)
{
    return (input_length / 4u) * 3u;
}

bool bc_core_format_base64_encode(const void* input, size_t input_length, char* output, size_t output_capacity, size_t* out_length)
{
    const size_t required = bc_core_format_base64_encoded_length(input_length);
    if (required > output_capacity) {
        return false;
    }
    const uint8_t* source = (const uint8_t*)input;
    size_t write_index = 0;
    size_t read_index = 0;
    while (read_index + 3u <= input_length) {
        const uint32_t triple =
            ((uint32_t)source[read_index] << 16) | ((uint32_t)source[read_index + 1] << 8) | (uint32_t)source[read_index + 2];
        output[write_index] = BC_CORE_BASE64_ALPHABET[(triple >> 18) & 0x3Fu];
        output[write_index + 1] = BC_CORE_BASE64_ALPHABET[(triple >> 12) & 0x3Fu];
        output[write_index + 2] = BC_CORE_BASE64_ALPHABET[(triple >> 6) & 0x3Fu];
        output[write_index + 3] = BC_CORE_BASE64_ALPHABET[triple & 0x3Fu];
        write_index += 4;
        read_index += 3;
    }
    const size_t remaining = input_length - read_index;
    if (remaining == 1u) {
        const uint32_t single = (uint32_t)source[read_index] << 16;
        output[write_index] = BC_CORE_BASE64_ALPHABET[(single >> 18) & 0x3Fu];
        output[write_index + 1] = BC_CORE_BASE64_ALPHABET[(single >> 12) & 0x3Fu];
        output[write_index + 2] = '=';
        output[write_index + 3] = '=';
        write_index += 4;
    } else if (remaining == 2u) {
        const uint32_t pair = ((uint32_t)source[read_index] << 16) | ((uint32_t)source[read_index + 1] << 8);
        output[write_index] = BC_CORE_BASE64_ALPHABET[(pair >> 18) & 0x3Fu];
        output[write_index + 1] = BC_CORE_BASE64_ALPHABET[(pair >> 12) & 0x3Fu];
        output[write_index + 2] = BC_CORE_BASE64_ALPHABET[(pair >> 6) & 0x3Fu];
        output[write_index + 3] = '=';
        write_index += 4;
    }
    *out_length = write_index;
    return true;
}

bool bc_core_format_base64_decode(const char* input, size_t input_length, void* output, size_t output_capacity, size_t* out_length)
{
    if ((input_length % 4u) != 0u) {
        return false;
    }
    if (input_length == 0u) {
        *out_length = 0;
        return true;
    }
    size_t padding = 0;
    if (input[input_length - 1] == '=') {
        padding += 1;
        if (input_length >= 2u && input[input_length - 2] == '=') {
            padding += 1;
        }
    }
    const size_t expected_output_length = (input_length / 4u) * 3u - padding;
    if (expected_output_length > output_capacity) {
        return false;
    }
    uint8_t* destination = (uint8_t*)output;
    size_t read_index = 0;
    size_t write_index = 0;
    while (read_index + 4u <= input_length) {
        const int v0 = bc_core_base64_decode_char(input[read_index]);
        const int v1 = bc_core_base64_decode_char(input[read_index + 1]);
        const int v2 = input[read_index + 2] == '=' ? 0 : bc_core_base64_decode_char(input[read_index + 2]);
        const int v3 = input[read_index + 3] == '=' ? 0 : bc_core_base64_decode_char(input[read_index + 3]);
        if (v0 < 0 || v1 < 0 || v2 < 0 || v3 < 0) {
            return false;
        }
        const uint32_t combined = ((uint32_t)v0 << 18) | ((uint32_t)v1 << 12) | ((uint32_t)v2 << 6) | (uint32_t)v3;
        const bool last_quartet = (read_index + 4u) == input_length;
        const size_t bytes_to_emit = last_quartet ? 3u - padding : 3u;
        if (bytes_to_emit >= 1u) {
            destination[write_index] = (uint8_t)((combined >> 16) & 0xFFu);
        }
        if (bytes_to_emit >= 2u) {
            destination[write_index + 1] = (uint8_t)((combined >> 8) & 0xFFu);
        }
        if (bytes_to_emit >= 3u) {
            destination[write_index + 2] = (uint8_t)(combined & 0xFFu);
        }
        write_index += bytes_to_emit;
        read_index += 4;
    }
    *out_length = write_index;
    return true;
}
