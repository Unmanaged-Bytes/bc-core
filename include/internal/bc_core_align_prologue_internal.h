// SPDX-License-Identifier: MIT

#ifndef BC_CORE_ALIGN_PROLOGUE_INTERNAL_H
#define BC_CORE_ALIGN_PROLOGUE_INTERNAL_H

#include "bc_core_cache_sizes_internal.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

static inline size_t bc_core_simd_align_dst_to_64_offset(const unsigned char* destination)
{
    uintptr_t aligned = ((uintptr_t)destination + 64) & ~(uintptr_t)63;
    return (size_t)(aligned - (uintptr_t)destination);
}

static inline bool bc_core_simd_should_stream_above_l3(size_t len)
{
    return len > bc_core_cached_l3_cache_size();
}

static inline bool bc_core_simd_should_stream_above_half_l3(size_t len)
{
    const size_t l3_cache_size = bc_core_cached_l3_cache_size();
    const size_t threshold = (l3_cache_size > 0) ? l3_cache_size / 2 : (4 * 1024 * 1024);
    return len > threshold;
}

#endif
