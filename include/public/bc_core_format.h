// SPDX-License-Identifier: MIT

#ifndef BC_CORE_FORMAT_H
#define BC_CORE_FORMAT_H

#include "bc_core_io.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

bool bc_core_format_unsigned_integer_64_decimal(char* buffer, size_t capacity, uint64_t value, size_t* out_length);
bool bc_core_format_unsigned_integer_64_hexadecimal(char* buffer, size_t capacity, uint64_t value, size_t* out_length);
bool bc_core_format_unsigned_integer_64_hexadecimal_padded(char* buffer, size_t capacity, uint64_t value, size_t digits,
                                                           size_t* out_length);
bool bc_core_format_signed_integer_64(char* buffer, size_t capacity, int64_t value, size_t* out_length);
bool bc_core_format_double(char* buffer, size_t capacity, double value, int frac_digits, size_t* out_length);
bool bc_core_format_bytes_human_readable(char* buffer, size_t capacity, uint64_t bytes, size_t* out_length);
bool bc_core_format_duration_nanoseconds(char* buffer, size_t capacity, uint64_t nanoseconds, size_t* out_length);
bool bc_core_format_unicode_codepoint_escape(char* buffer, size_t capacity, uint32_t codepoint, size_t* out_length);

bool bc_core_writer_write_unsigned_integer_64_decimal(bc_core_writer_t* writer, uint64_t value);
bool bc_core_writer_write_unsigned_integer_64_hexadecimal(bc_core_writer_t* writer, uint64_t value);
bool bc_core_writer_write_unsigned_integer_64_hexadecimal_padded(bc_core_writer_t* writer, uint64_t value, size_t digits);
bool bc_core_writer_write_signed_integer_64(bc_core_writer_t* writer, int64_t value);
bool bc_core_writer_write_double(bc_core_writer_t* writer, double value, int frac_digits);
bool bc_core_writer_write_bytes_human_readable(bc_core_writer_t* writer, uint64_t bytes);
bool bc_core_writer_write_duration_nanoseconds(bc_core_writer_t* writer, uint64_t nanoseconds);
bool bc_core_writer_write_unicode_codepoint_escape(bc_core_writer_t* writer, uint32_t codepoint);

#endif /* BC_CORE_FORMAT_H */
