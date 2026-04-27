// SPDX-License-Identifier: MIT

#include "bc_core.h"

#include <stdint.h>
#include <string.h>

static bool byte_in_targets(unsigned char byte_value, const unsigned char* targets, size_t target_count)
{
    for (size_t index = 0; index < target_count; ++index) {
        if (targets[index] == byte_value) {
            return true;
        }
    }
    return false;
}

int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if (size < 2U) {
        return 0;
    }

    size_t target_count = (size_t)data[0];
    if (target_count > size - 1U) {
        target_count = size - 1U;
    }

    const unsigned char* targets = (const unsigned char*)(data + 1);
    const unsigned char* haystack = (const unsigned char*)(data + 1U + target_count);
    size_t haystack_length = size - 1U - target_count;

    bc_core_byte_mask_t mask;
    bool prepared = bc_core_byte_mask_prepare(targets, target_count, &mask);
    if (!prepared) {
        return 0;
    }

    size_t in_offset = 0;
    bool found_in = bc_core_find_byte_in_mask(haystack, haystack_length, &mask, &in_offset);
    if (found_in) {
        if (in_offset >= haystack_length) {
            __builtin_trap();
        }
        if (!byte_in_targets(haystack[in_offset], targets, target_count)) {
            __builtin_trap();
        }
        for (size_t scan = 0; scan < in_offset; ++scan) {
            if (byte_in_targets(haystack[scan], targets, target_count)) {
                __builtin_trap();
            }
        }
    } else {
        for (size_t scan = 0; scan < haystack_length; ++scan) {
            if (byte_in_targets(haystack[scan], targets, target_count)) {
                __builtin_trap();
            }
        }
    }

    size_t out_offset = 0;
    bool found_out = bc_core_find_byte_not_in_mask(haystack, haystack_length, &mask, &out_offset);
    if (found_out) {
        if (out_offset >= haystack_length) {
            __builtin_trap();
        }
        if (byte_in_targets(haystack[out_offset], targets, target_count)) {
            __builtin_trap();
        }
        for (size_t scan = 0; scan < out_offset; ++scan) {
            if (!byte_in_targets(haystack[scan], targets, target_count)) {
                __builtin_trap();
            }
        }
    } else {
        for (size_t scan = 0; scan < haystack_length; ++scan) {
            if (!byte_in_targets(haystack[scan], targets, target_count)) {
                __builtin_trap();
            }
        }
    }

    return 0;
}
