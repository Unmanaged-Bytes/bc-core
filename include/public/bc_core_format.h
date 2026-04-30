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

#endif /* BC_CORE_FORMAT_H */
