// SPDX-License-Identifier: MIT

#include "bc_core.h"

#include <immintrin.h>
#include <stdint.h>

bool bc_core_zero_secure(volatile void* dst, size_t len)
{
    if (len == 0) {
        return true;
    }

    volatile unsigned char* destination = (volatile unsigned char*)dst;

    for (size_t byte_index = 0; byte_index < len; byte_index++) {
        destination[byte_index] = 0;
    }

    __asm__ volatile("" ::: "memory");

    return true;
}
