// SPDX-License-Identifier: MIT

#ifndef BC_CORE_FORMAT_H
#define BC_CORE_FORMAT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

bool bc_core_format_unsigned_integer_64_decimal(char* buffer, size_t capacity, uint64_t value, size_t* out_length);
bool bc_core_format_unsigned_integer_64_hexadecimal(char* buffer, size_t capacity, uint64_t value, size_t* out_length);
bool bc_core_format_unsigned_integer_64_hexadecimal_padded(char* buffer, size_t capacity, uint64_t value, size_t digits,
                                                           size_t* out_length);
bool bc_core_format_signed_integer_64(char* buffer, size_t capacity, int64_t value, size_t* out_length);
bool bc_core_format_double(char* buffer, size_t capacity, double value, int frac_digits, size_t* out_length);
/* Format double using shortest-round-trip semantics: produces the shortest
   decimal representation that, when parsed back via strtod(), yields exactly
   the original value. Equivalent to printf("%.17g", v) but stable.
   Used for JSON/YAML serialization where byte-equivalence matters. */
bool bc_core_format_double_shortest_round_trip(char* buffer, size_t capacity, double value, size_t* out_length);
bool bc_core_format_bytes_human_readable(char* buffer, size_t capacity, uint64_t bytes, size_t* out_length);
bool bc_core_format_duration_nanoseconds(char* buffer, size_t capacity, uint64_t nanoseconds, size_t* out_length);
bool bc_core_format_unicode_codepoint_escape(char* buffer, size_t capacity, uint32_t codepoint, size_t* out_length);

size_t bc_core_format_base64_encoded_length(size_t input_length);
size_t bc_core_format_base64_decoded_length_max(size_t input_length);
bool bc_core_format_base64_encode(const void* input, size_t input_length, char* output, size_t output_capacity, size_t* out_length);
bool bc_core_format_base64_decode(const char* input, size_t input_length, void* output, size_t output_capacity, size_t* out_length);

size_t bc_core_format_base32_encoded_length(size_t input_length);
size_t bc_core_format_base32_decoded_length_max(size_t input_length);
bool bc_core_format_base32_encode(const void* input, size_t input_length, char* output, size_t output_capacity, size_t* out_length);
bool bc_core_format_base32_decode(const char* input, size_t input_length, void* output, size_t output_capacity, size_t* out_length);

typedef struct {
    size_t bytes_per_line;
    bool show_offset;
    bool show_ascii;
} bc_core_format_hex_dump_options_t;

size_t bc_core_format_hex_dump_required_capacity(size_t input_length, const bc_core_format_hex_dump_options_t* options);
bool bc_core_format_hex_dump(const void* input, size_t input_length, uint64_t base_offset, const bc_core_format_hex_dump_options_t* options,
                             char* output, size_t output_capacity, size_t* out_length);

#endif /* BC_CORE_FORMAT_H */
