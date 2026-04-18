// SPDX-License-Identifier: MIT

#include "bc_core.h"

#include "bc_core_cache_sizes_internal.h"

#include <immintrin.h>
#include <stdint.h>

__attribute__((target("avx2"))) static bool bc_core_fill_avx2(void* dst, size_t len, unsigned char value)
{
    if (len == 0) {
        return true;
    }

    unsigned char* destination = (unsigned char*)dst;
    __m256i fill_vector = _mm256_set1_epi8((char)value);

    if (len < 32) {
        for (size_t byte_index = 0; byte_index < len; byte_index++) {
            destination[byte_index] = value;
        }
        return true;
    }

    if (len <= 128) {
        _mm256_storeu_si256((__m256i*)destination, fill_vector);
        if (len >= 64) {
            _mm256_storeu_si256((__m256i*)(destination + 32), fill_vector);
        }
        if (len >= 96) {
            _mm256_storeu_si256((__m256i*)(destination + 64), fill_vector);
        }
        _mm256_storeu_si256((__m256i*)(destination + len - 32), fill_vector);
        return true;
    }

    _mm256_storeu_si256((__m256i*)destination, fill_vector);

    unsigned char* aligned_destination = (unsigned char*)(((uintptr_t)destination + 32) & ~(uintptr_t)31);
    unsigned char* destination_end = destination + len;

    if (len > bc_core_cached_l3_cache_size()) { /* GCOVR_EXCL_BR_LINE -- NT stores > L3, tested by benchmarks */
        /* GCOVR_EXCL_START -- NT stores > L3, tested by benchmarks */
        const unsigned char* loop_end = destination_end - 127;
        while (aligned_destination < loop_end) {
            _mm256_stream_si256((__m256i*)aligned_destination, fill_vector);
            _mm256_stream_si256((__m256i*)(aligned_destination + 32), fill_vector);
            _mm256_stream_si256((__m256i*)(aligned_destination + 64), fill_vector);
            _mm256_stream_si256((__m256i*)(aligned_destination + 96), fill_vector);
            aligned_destination += 128;
        }
        const unsigned char* tail_end = destination_end - 31;
        while (aligned_destination < tail_end) {
            _mm256_stream_si256((__m256i*)aligned_destination, fill_vector);
            aligned_destination += 32;
        }
        _mm_sfence();
        /* GCOVR_EXCL_STOP */
    } else {
        const unsigned char* loop_end = destination_end - 127;
        while (aligned_destination < loop_end) {
            _mm256_store_si256((__m256i*)aligned_destination, fill_vector);
            _mm256_store_si256((__m256i*)(aligned_destination + 32), fill_vector);
            _mm256_store_si256((__m256i*)(aligned_destination + 64), fill_vector);
            _mm256_store_si256((__m256i*)(aligned_destination + 96), fill_vector);
            aligned_destination += 128;
        }
        const unsigned char* tail_end = destination_end - 31;
        while (aligned_destination < tail_end) {
            _mm256_store_si256((__m256i*)aligned_destination, fill_vector);
            aligned_destination += 32;
        }
    }

    _mm256_storeu_si256((__m256i*)(destination_end - 32), fill_vector);

    return true;
}

static bool (*g_fill_impl)(void*, size_t, unsigned char) = bc_core_fill_avx2;

__attribute__((target("avx2"))) static bool bc_core_fill_avx2_with_policy(void* dst, size_t len, unsigned char value,
                                                                          bc_core_cache_policy_t policy)
{
    if (len == 0) {
        return true;
    }

    unsigned char* destination = (unsigned char*)dst;
    __m256i fill_vector = _mm256_set1_epi8((char)value);

    if (len < 32) {
        for (size_t byte_index = 0; byte_index < len; byte_index++) {
            destination[byte_index] = value;
        }
        return true;
    }

    if (len <= 128) {
        _mm256_storeu_si256((__m256i*)destination, fill_vector);
        if (len >= 64) {
            _mm256_storeu_si256((__m256i*)(destination + 32), fill_vector);
        }
        if (len >= 96) {
            _mm256_storeu_si256((__m256i*)(destination + 64), fill_vector);
        }
        _mm256_storeu_si256((__m256i*)(destination + len - 32), fill_vector);
        return true;
    }

    _mm256_storeu_si256((__m256i*)destination, fill_vector);

    unsigned char* aligned_destination = (unsigned char*)(((uintptr_t)destination + 32) & ~(uintptr_t)31);
    unsigned char* destination_end = destination + len;

    const bool use_nt = (policy == BC_CORE_CACHE_POLICY_STREAMING) ? true
                        : (policy == BC_CORE_CACHE_POLICY_CACHED)  ? false
                                                                   : (len > bc_core_cached_l3_cache_size());

    if (use_nt) {
        const unsigned char* loop_end = destination_end - 127;
        while (aligned_destination < loop_end) {
            _mm256_stream_si256((__m256i*)aligned_destination, fill_vector);
            _mm256_stream_si256((__m256i*)(aligned_destination + 32), fill_vector);
            _mm256_stream_si256((__m256i*)(aligned_destination + 64), fill_vector);
            _mm256_stream_si256((__m256i*)(aligned_destination + 96), fill_vector);
            aligned_destination += 128;
        }
        const unsigned char* tail_end = destination_end - 31;
        while (aligned_destination < tail_end) {
            _mm256_stream_si256((__m256i*)aligned_destination, fill_vector);
            aligned_destination += 32;
        }
        _mm_sfence();
    } else {
        const unsigned char* loop_end = destination_end - 127;
        while (aligned_destination < loop_end) {
            _mm256_store_si256((__m256i*)aligned_destination, fill_vector);
            _mm256_store_si256((__m256i*)(aligned_destination + 32), fill_vector);
            _mm256_store_si256((__m256i*)(aligned_destination + 64), fill_vector);
            _mm256_store_si256((__m256i*)(aligned_destination + 96), fill_vector);
            aligned_destination += 128;
        }
        const unsigned char* tail_end = destination_end - 31;
        while (aligned_destination < tail_end) {
            _mm256_store_si256((__m256i*)aligned_destination, fill_vector);
            aligned_destination += 32;
        }
    }

    _mm256_storeu_si256((__m256i*)(destination_end - 32), fill_vector);

    return true;
}

static bool (*g_fill_with_policy_impl)(void*, size_t, unsigned char, bc_core_cache_policy_t) = bc_core_fill_avx2_with_policy;

__attribute__((constructor)) void bc_core_fill_dispatch_init(void)
{
    g_fill_impl = bc_core_fill_avx2;
    g_fill_with_policy_impl = bc_core_fill_avx2_with_policy;
}

bool bc_core_fill(void* dst, size_t len, unsigned char value)
{
    return g_fill_impl(dst, len, value);
}

bool bc_core_fill_with_policy(void* dst, size_t len, unsigned char value, bc_core_cache_policy_t policy)
{
    return g_fill_with_policy_impl(dst, len, value, policy);
}
