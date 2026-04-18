// SPDX-License-Identifier: MIT

#include "bc_core.h"

#include "bc_core_cpu_features_internal.h"

#include <immintrin.h>
#include <stdint.h>

__attribute__((target("avx2"))) static bool bc_core_swap_avx2(void* a, void* b, size_t len)
{
    if (len == 0) {
        return true;
    }

    unsigned char* buffer_a = (unsigned char*)a;
    unsigned char* buffer_b = (unsigned char*)b;
    size_t current_offset = 0;

    for (; current_offset + 32u <= len; current_offset += 32u) {
        __m256i chunk_from_a = _mm256_loadu_si256((const __m256i*)(buffer_a + current_offset));
        __m256i chunk_from_b = _mm256_loadu_si256((const __m256i*)(buffer_b + current_offset));
        _mm256_storeu_si256((__m256i*)(buffer_a + current_offset), chunk_from_b);
        _mm256_storeu_si256((__m256i*)(buffer_b + current_offset), chunk_from_a);
    }

    for (; current_offset < len; current_offset++) {
        unsigned char temporary_byte = buffer_a[current_offset];
        buffer_a[current_offset] = buffer_b[current_offset];
        buffer_b[current_offset] = temporary_byte;
    }

    return true;
}

static bool (*g_swap_impl)(void*, void*, size_t) = bc_core_swap_avx2;

__attribute__((constructor)) void bc_core_swap_dispatch_init(void)
{
    g_swap_impl = bc_core_swap_avx2;
}

bool bc_core_swap(void* a, void* b, size_t len)
{
    return g_swap_impl(a, b, len);
}
