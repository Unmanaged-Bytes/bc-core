// SPDX-License-Identifier: MIT

#ifndef BC_CORE_ERROR_H
#define BC_CORE_ERROR_H

#include <stdbool.h>
#include <stddef.h>

typedef enum {
    BC_CORE_ERROR_NONE = 0,
    BC_CORE_ERROR_INVALID_ARGUMENT,
    BC_CORE_ERROR_BUFFER_TOO_SMALL,
    BC_CORE_ERROR_OVERFLOW,
    BC_CORE_ERROR_PARSE_FAILED,
    BC_CORE_ERROR_NOT_FOUND,
    BC_CORE_ERROR_PERMISSION_DENIED,
    BC_CORE_ERROR_RESOURCE_EXHAUSTED,
    BC_CORE_ERROR_INPUT_OUTPUT_FAILED,
    BC_CORE_ERROR_END_OF_FILE,
    BC_CORE_ERROR_BROKEN_PIPE,
    BC_CORE_ERROR_INTERRUPTED,
    BC_CORE_ERROR_UNAVAILABLE,
    BC_CORE_ERROR_INTERNAL
} bc_core_error_code_t;

bool bc_core_error_describe(bc_core_error_code_t code,
                            char* buffer, size_t capacity, size_t* out_length);
bool bc_core_error_name(bc_core_error_code_t code,
                        const char** out_name, size_t* out_name_length);
bool bc_core_error_from_system_errno(int system_errno,
                                     bc_core_error_code_t* out_code);

#endif /* BC_CORE_ERROR_H */
