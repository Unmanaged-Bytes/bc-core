// SPDX-License-Identifier: MIT

#include "bc_core_cpu.h"

#include <immintrin.h>
#include <stddef.h>

void bc_core_prefetch_read_range(const void* data, size_t len)
{
    const char* ptr = (const char*)data;
    const char* end = ptr + len;
    for (; ptr < end; ptr += 64) {
        bc_core_prefetch_low_locality(ptr);
    }
}

__attribute__((target("clflushopt"))) void bc_core_evict_range(const void* data, size_t len)
{
    const char* ptr = (const char*)data;
    const char* end = ptr + len;
    for (; ptr < end; ptr += 64) {
        _mm_clflushopt((void*)ptr);
    }
    _mm_sfence();
}
