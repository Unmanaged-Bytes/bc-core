// SPDX-License-Identifier: MIT

#include "bc_core.h"

#include <stdint.h>
#include <string.h>

int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if (size < sizeof(size_t) * 3U) {
        return 0;
    }

    size_t a = 0;
    size_t b = 0;
    size_t c = 0;
    memcpy(&a, data, sizeof(size_t));
    memcpy(&b, data + sizeof(size_t), sizeof(size_t));
    memcpy(&c, data + 2U * sizeof(size_t), sizeof(size_t));

    size_t multiply_result = 0;
    bool multiply_ok = bc_core_safe_multiply(a, b, &multiply_result);
    if (multiply_ok) {
        if (a != 0U && multiply_result / a != b) {
            __builtin_trap();
        }
    } else {
        if (a != 0U && b != 0U) {
            size_t back = multiply_result / a;
            (void)back;
        }
    }

    size_t add_result = 0;
    bool add_ok = bc_core_safe_add(a, b, &add_result);
    if (add_ok) {
        if (add_result < a || add_result < b) {
            __builtin_trap();
        }
        if (add_result != a + b) {
            __builtin_trap();
        }
    }

    size_t alignment = c;
    if (alignment != 0U && (alignment & (alignment - 1U)) == 0U) {
        size_t align_result = 0;
        bool align_ok = bc_core_align_up(a, alignment, &align_result);
        if (align_ok) {
            if (align_result < a) {
                __builtin_trap();
            }
            if ((align_result & (alignment - 1U)) != 0U) {
                __builtin_trap();
            }
            if (align_result - a >= alignment) {
                __builtin_trap();
            }
        }
    }

    uint16_t value_16 = 0;
    uint32_t value_32 = 0;
    uint64_t value_64 = 0;
    if (size >= 16U) {
        bc_core_load_u16_unaligned(data, &value_16);
        bc_core_load_u32_unaligned(data + 2U, &value_32);
        bc_core_load_u64_unaligned(data + 8U, &value_64);

        /* Tile bytes 0..15 contiguously: u16 at 0, u16 at 2, u32 at 4, u64 at 8.
           Otherwise gap bytes remain uninitialized vs. data and memcmp false-positives. */
        uint16_t value_16_b = 0;
        uint32_t value_32_b = 0;
        bc_core_load_u16_unaligned(data + 2U, &value_16_b);
        bc_core_load_u32_unaligned(data + 4U, &value_32_b);

        unsigned char scratch[16] = {0};
        bc_core_store_u16_unaligned(scratch, value_16);
        bc_core_store_u16_unaligned(scratch + 2U, value_16_b);
        bc_core_store_u32_unaligned(scratch + 4U, value_32_b);
        bc_core_store_u64_unaligned(scratch + 8U, value_64);

        if (memcmp(scratch, data, 16U) != 0) {
            __builtin_trap();
        }
        (void)value_32;
    }

    return 0;
}
