// SPDX-License-Identifier: MIT

#ifndef BC_CORE_MATH_H
#define BC_CORE_MATH_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

static inline void bc_core_load_u16_unaligned(const void* src, uint16_t* out_value)
{
    __builtin_memcpy(out_value, src, sizeof(*out_value));
}

static inline void bc_core_load_u32_unaligned(const void* src, uint32_t* out_value)
{
    __builtin_memcpy(out_value, src, sizeof(*out_value));
}

static inline void bc_core_load_u64_unaligned(const void* src, uint64_t* out_value)
{
    __builtin_memcpy(out_value, src, sizeof(*out_value));
}

static inline void bc_core_store_u16_unaligned(void* dst, uint16_t value)
{
    __builtin_memcpy(dst, &value, sizeof(value));
}

static inline void bc_core_store_u32_unaligned(void* dst, uint32_t value)
{
    __builtin_memcpy(dst, &value, sizeof(value));
}

static inline void bc_core_store_u64_unaligned(void* dst, uint64_t value)
{
    __builtin_memcpy(dst, &value, sizeof(value));
}

static inline bool bc_core_safe_multiply(size_t a, size_t b, size_t* out_result)
{
    return !__builtin_mul_overflow(a, b, out_result);
}

static inline bool bc_core_safe_add(size_t a, size_t b, size_t* out_result)
{
    return !__builtin_add_overflow(a, b, out_result);
}

static inline bool bc_core_align_up(size_t value, size_t alignment, size_t* out_result)
{
    size_t addend;
    if (!bc_core_safe_add(value, alignment - 1, &addend)) {
        return false;
    }
    *out_result = addend & ~(alignment - 1);
    return true;
}

#endif /* BC_CORE_MATH_H */
