// SPDX-License-Identifier: MIT

#include "bc_core.h"

#include "bc_core_cpu_features_internal.h"

#include <immintrin.h>
#include <stdint.h>

__attribute__((target("avx2"))) static bool bc_core_equal_avx2(const void* a, const void* b, size_t len, bool* out_equal)
{
    if (len == 0) {
        *out_equal = true;
        return true;
    }

    const unsigned char* pointer_a = (const unsigned char*)a;
    const unsigned char* pointer_b = (const unsigned char*)b;

    size_t full_chunk_count = len / 32;
    size_t remaining_bytes = len % 32;

    for (size_t chunk_index = 0; chunk_index < full_chunk_count; chunk_index++) {
        size_t offset = chunk_index * 32;
        __m256i chunk_a = _mm256_loadu_si256((const __m256i*)(pointer_a + offset));
        __m256i chunk_b = _mm256_loadu_si256((const __m256i*)(pointer_b + offset));
        __m256i comparison_result = _mm256_cmpeq_epi8(chunk_a, chunk_b);
        unsigned int movemask_result = (unsigned int)_mm256_movemask_epi8(comparison_result);
        if (movemask_result != 0xFFFFFFFFu) {
            *out_equal = false;
            return true;
        }
    }

    size_t tail_offset = full_chunk_count * 32;
    for (size_t byte_index = 0; byte_index < remaining_bytes; byte_index++) {
        if (pointer_a[tail_offset + byte_index] != pointer_b[tail_offset + byte_index]) {
            *out_equal = false;
            return true;
        }
    }

    *out_equal = true;
    return true;
}

static bool (*g_equal_impl)(const void*, const void*, size_t, bool*) = bc_core_equal_avx2;

__attribute__((constructor)) void bc_core_equal_dispatch_init(void)
{
    g_equal_impl = bc_core_equal_avx2;
}

bool bc_core_equal(const void* a, const void* b, size_t len, bool* out_equal)
{
    return g_equal_impl(a, b, len, out_equal);
}
