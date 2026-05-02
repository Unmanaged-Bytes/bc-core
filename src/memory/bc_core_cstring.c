// SPDX-License-Identifier: MIT

#include "bc_core_memory.h"

#include <stdbool.h>
#include <stddef.h>

bool bc_core_cstring_equal(const char* left, const char* right, bool* out_equal)
{
    size_t left_length = 0;
    size_t right_length = 0;
    if (!bc_core_length(left, (unsigned char)'\0', &left_length)) {
        return false;
    }
    if (!bc_core_length(right, (unsigned char)'\0', &right_length)) {
        return false;
    }
    if (left_length != right_length) {
        *out_equal = false;
        return true;
    }
    return bc_core_equal(left, right, left_length, out_equal);
}

bool bc_core_cstring_compare(const char* left, const char* right, int* out_order)
{
    size_t left_length = 0;
    size_t right_length = 0;
    if (!bc_core_length(left, (unsigned char)'\0', &left_length)) {
        return false;
    }
    if (!bc_core_length(right, (unsigned char)'\0', &right_length)) {
        return false;
    }
    const size_t shared_length = left_length < right_length ? left_length : right_length;
    int prefix_order = 0;
    if (!bc_core_compare(left, right, shared_length, &prefix_order)) {
        return false;
    }
    if (prefix_order != 0) {
        *out_order = prefix_order;
        return true;
    }
    if (left_length < right_length) {
        *out_order = -1;
    } else if (left_length > right_length) {
        *out_order = 1;
    } else {
        *out_order = 0;
    }
    return true;
}
