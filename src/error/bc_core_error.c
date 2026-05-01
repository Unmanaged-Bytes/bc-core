// SPDX-License-Identifier: MIT

#include "bc_core_error.h"
#include "bc_core_memory.h"

#include <errno.h>

typedef struct {
    bc_core_error_code_t code;
    const char* name;
    size_t name_length;
    const char* description;
    size_t description_length;
} bc_core_error_entry_t;

#define BC_CORE_ERROR_ENTRY(code_value, name_literal, description_literal)                                                                 \
    {(code_value), (name_literal), sizeof(name_literal) - 1U, (description_literal), sizeof(description_literal) - 1U}

static const bc_core_error_entry_t bc_core_error_table[] = {
    BC_CORE_ERROR_ENTRY(BC_CORE_ERROR_NONE, "BC_CORE_ERROR_NONE", "no error"),
    BC_CORE_ERROR_ENTRY(BC_CORE_ERROR_INVALID_ARGUMENT, "BC_CORE_ERROR_INVALID_ARGUMENT", "invalid argument"),
    BC_CORE_ERROR_ENTRY(BC_CORE_ERROR_BUFFER_TOO_SMALL, "BC_CORE_ERROR_BUFFER_TOO_SMALL", "destination buffer too small"),
    BC_CORE_ERROR_ENTRY(BC_CORE_ERROR_OVERFLOW, "BC_CORE_ERROR_OVERFLOW", "arithmetic or capacity overflow"),
    BC_CORE_ERROR_ENTRY(BC_CORE_ERROR_PARSE_FAILED, "BC_CORE_ERROR_PARSE_FAILED", "parse failed"),
    BC_CORE_ERROR_ENTRY(BC_CORE_ERROR_NOT_FOUND, "BC_CORE_ERROR_NOT_FOUND", "not found"),
    BC_CORE_ERROR_ENTRY(BC_CORE_ERROR_PERMISSION_DENIED, "BC_CORE_ERROR_PERMISSION_DENIED", "permission denied"),
    BC_CORE_ERROR_ENTRY(BC_CORE_ERROR_RESOURCE_EXHAUSTED, "BC_CORE_ERROR_RESOURCE_EXHAUSTED", "resource exhausted"),
    BC_CORE_ERROR_ENTRY(BC_CORE_ERROR_INPUT_OUTPUT_FAILED, "BC_CORE_ERROR_INPUT_OUTPUT_FAILED", "input or output operation failed"),
    BC_CORE_ERROR_ENTRY(BC_CORE_ERROR_END_OF_FILE, "BC_CORE_ERROR_END_OF_FILE", "end of file reached"),
    BC_CORE_ERROR_ENTRY(BC_CORE_ERROR_BROKEN_PIPE, "BC_CORE_ERROR_BROKEN_PIPE", "broken pipe"),
    BC_CORE_ERROR_ENTRY(BC_CORE_ERROR_INTERRUPTED, "BC_CORE_ERROR_INTERRUPTED", "operation interrupted"),
    BC_CORE_ERROR_ENTRY(BC_CORE_ERROR_UNAVAILABLE, "BC_CORE_ERROR_UNAVAILABLE", "resource unavailable"),
    BC_CORE_ERROR_ENTRY(BC_CORE_ERROR_INTERNAL, "BC_CORE_ERROR_INTERNAL", "internal error"),
};

static const size_t bc_core_error_table_count = sizeof(bc_core_error_table) / sizeof(bc_core_error_table[0]);

static const bc_core_error_entry_t* bc_core_error_lookup(bc_core_error_code_t code)
{
    for (size_t index = 0; index < bc_core_error_table_count; ++index) {
        if (bc_core_error_table[index].code == code) {
            return &bc_core_error_table[index];
        }
    }
    return NULL;
}

bool bc_core_error_describe(bc_core_error_code_t code, char* buffer, size_t capacity, size_t* out_length)
{
    const bc_core_error_entry_t* entry = bc_core_error_lookup(code);
    if (entry == NULL) {
        return false;
    }
    if (entry->description_length > capacity) {
        return false;
    }
    (void)bc_core_copy(buffer, entry->description, entry->description_length);
    *out_length = entry->description_length;
    return true;
}

bool bc_core_error_name(bc_core_error_code_t code, const char** out_name, size_t* out_name_length)
{
    const bc_core_error_entry_t* entry = bc_core_error_lookup(code);
    if (entry == NULL) {
        return false;
    }
    *out_name = entry->name;
    *out_name_length = entry->name_length;
    return true;
}

bool bc_core_error_from_system_errno(int system_errno, bc_core_error_code_t* out_code)
{
    switch (system_errno) {
    case 0:
        *out_code = BC_CORE_ERROR_NONE;
        return true;
    case EINVAL:
        *out_code = BC_CORE_ERROR_INVALID_ARGUMENT;
        return true;
    case ERANGE:
    case EOVERFLOW:
        *out_code = BC_CORE_ERROR_OVERFLOW;
        return true;
    case ENOENT:
    case ESRCH:
        *out_code = BC_CORE_ERROR_NOT_FOUND;
        return true;
    case EACCES:
    case EPERM:
        *out_code = BC_CORE_ERROR_PERMISSION_DENIED;
        return true;
    case ENOMEM:
    case EMFILE:
    case ENFILE:
    case ENOSPC:
    case EDQUOT:
        *out_code = BC_CORE_ERROR_RESOURCE_EXHAUSTED;
        return true;
    case EIO:
        *out_code = BC_CORE_ERROR_INPUT_OUTPUT_FAILED;
        return true;
    case EPIPE:
        *out_code = BC_CORE_ERROR_BROKEN_PIPE;
        return true;
    case EINTR:
        *out_code = BC_CORE_ERROR_INTERRUPTED;
        return true;
    case EAGAIN:
    case EBUSY:
        *out_code = BC_CORE_ERROR_UNAVAILABLE;
        return true;
    case ENOSYS:
    case EOPNOTSUPP:
        *out_code = BC_CORE_ERROR_UNAVAILABLE;
        return true;
    default:
        *out_code = BC_CORE_ERROR_INTERNAL;
        return true;
    }
}
