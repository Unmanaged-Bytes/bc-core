// SPDX-License-Identifier: MIT

#include "bc_core.h"

#include "bc_core_cpu_features_internal.h"

#include <immintrin.h>
#include <stdint.h>

__attribute__((target("avx2"))) static bool bc_core_move_avx2(void* dst, const void* src, size_t len)
{
    if (len == 0) {
        return true;
    }

    unsigned char* destination = (unsigned char*)dst;
    const unsigned char* source = (const unsigned char*)src;

    if (destination + len <= source || source + len <= destination) {
        return bc_core_copy(dst, src, len);
    }

    if (destination < source) {
        uintptr_t destination_address = (uintptr_t)destination;
        size_t head_byte_count = (32u - (destination_address & 31u)) & 31u;

        if (head_byte_count > len) {
            head_byte_count = len;
        }

        for (size_t byte_index = 0; byte_index < head_byte_count; byte_index++) {
            destination[byte_index] = source[byte_index];
        }

        size_t current_offset = head_byte_count;

        for (; current_offset + 32u <= len; current_offset += 32u) {
            __m256i source_chunk = _mm256_loadu_si256((const __m256i*)(source + current_offset));
            _mm256_storeu_si256((__m256i*)(destination + current_offset), source_chunk);
        }

        for (; current_offset < len; current_offset++) {
            destination[current_offset] = source[current_offset];
        }
    } else {
        size_t tail_byte_count = len & 31u;
        size_t aligned_end = len - tail_byte_count;

        for (size_t byte_index = len; byte_index > aligned_end; byte_index--) {
            destination[byte_index - 1] = source[byte_index - 1];
        }

        size_t current_offset = aligned_end;

        while (current_offset >= 32u) {
            current_offset -= 32u;
            __m256i source_chunk = _mm256_loadu_si256((const __m256i*)(source + current_offset));
            _mm256_storeu_si256((__m256i*)(destination + current_offset), source_chunk);
        }
    }

    return true;
}

static bool (*g_move_impl)(void*, const void*, size_t) = bc_core_move_avx2;

__attribute__((constructor)) void bc_core_move_dispatch_init(void)
{
    g_move_impl = bc_core_move_avx2;
}

bool bc_core_move(void* dst, const void* src, size_t len)
{
    return g_move_impl(dst, src, len);
}
