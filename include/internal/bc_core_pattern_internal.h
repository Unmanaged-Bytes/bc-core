// SPDX-License-Identifier: MIT

#ifndef BC_CORE_PATTERN_H
#define BC_CORE_PATTERN_H

#include <stdbool.h>
#include <stddef.h>

static inline bool bc_core_pattern_matches(const unsigned char* data, const unsigned char* pattern, size_t pattern_length)
{
    for (size_t i = 0; i < pattern_length; i++) {
        if (data[i] != pattern[i]) {
            return false;
        }
    }
    return true;
}

#endif
