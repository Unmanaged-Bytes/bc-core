// SPDX-License-Identifier: MIT

#include "bc_core.h"

#include <stddef.h>

void bc_core_contains_byte(const void* data, size_t len, unsigned char target, bool* out_found)
{
    size_t found_offset;
    *out_found = bc_core_find_byte(data, len, target, &found_offset);
}

void bc_core_contains_pattern(const void* data, size_t len, const void* pattern, size_t pattern_len, bool* out_found)
{
    size_t found_offset;
    *out_found = bc_core_find_pattern(data, len, pattern, pattern_len, &found_offset);
}

void bc_core_starts_with(const void* data, size_t len, const void* prefix, size_t prefix_len, bool* out_result)
{
    if (prefix_len > len) {
        *out_result = false;
        return;
    }

    bc_core_equal(data, prefix, prefix_len, out_result);
}

void bc_core_ends_with(const void* data, size_t len, const void* suffix, size_t suffix_len, bool* out_result)
{
    if (suffix_len > len) {
        *out_result = false;
        return;
    }

    bc_core_equal((const char*)data + len - suffix_len, suffix, suffix_len, out_result);
}
