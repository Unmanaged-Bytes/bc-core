// SPDX-License-Identifier: MIT

#ifndef BC_CORE_PARSE_H
#define BC_CORE_PARSE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

bool bc_core_parse_unsigned_integer_64_decimal(const char* text, size_t length, uint64_t* out_value, size_t* out_consumed);
bool bc_core_parse_signed_integer_64_decimal(const char* text, size_t length, int64_t* out_value, size_t* out_consumed);

#endif /* BC_CORE_PARSE_H */
