// SPDX-License-Identifier: MIT

#include "bc_core.h"

#include "bc_core_cache_sizes_internal.h"
#include "bc_core_cpu_features_internal.h"

#include <immintrin.h>
#include <stdint.h>

__attribute__((target("avx512f,avx512bw"))) static bool bc_core_zero_avx512(void* dst, size_t len)
{
    if (len == 0) {
        return true;
    }

    unsigned char* destination = (unsigned char*)dst;
    __m512i zero_vector = _mm512_setzero_si512();

    if (len < 32) {
        for (size_t byte_index = 0; byte_index < len; byte_index++) {
            destination[byte_index] = 0;
        }
        return true;
    }

    if (len <= 64) {
        __m256i zero_vector_ymm = _mm256_setzero_si256();
        _mm256_storeu_si256((__m256i*)destination, zero_vector_ymm);
        _mm256_storeu_si256((__m256i*)(destination + len - 32), zero_vector_ymm);
        return true;
    }

    if (len <= 256) {
        _mm512_storeu_si512((__m512i*)destination, zero_vector);
        if (len >= 128) {
            _mm512_storeu_si512((__m512i*)(destination + 64), zero_vector);
        }
        if (len >= 192) {
            _mm512_storeu_si512((__m512i*)(destination + 128), zero_vector);
        }
        _mm512_storeu_si512((__m512i*)(destination + len - 64), zero_vector);
        return true;
    }

    _mm512_storeu_si512((__m512i*)destination, zero_vector);

    unsigned char* aligned_destination = (unsigned char*)(((uintptr_t)destination + 64) & ~(uintptr_t)63);
    unsigned char* destination_end = destination + len;

    if (len > bc_core_cached_l3_cache_size()) {
        /* GCOVR_EXCL_START -- NT stores > L3, tested by benchmarks */
        const unsigned char* loop_end = destination_end - 255;
        while (aligned_destination < loop_end) {
            _mm512_stream_si512((__m512i*)aligned_destination, zero_vector);
            _mm512_stream_si512((__m512i*)(aligned_destination + 64), zero_vector);
            _mm512_stream_si512((__m512i*)(aligned_destination + 128), zero_vector);
            _mm512_stream_si512((__m512i*)(aligned_destination + 192), zero_vector);
            aligned_destination += 256;
        }
        const unsigned char* tail_end = destination_end - 63;
        while (aligned_destination < tail_end) {
            _mm512_stream_si512((__m512i*)aligned_destination, zero_vector);
            aligned_destination += 64;
        }
        _mm_sfence();
        /* GCOVR_EXCL_STOP */
    } else {
        const unsigned char* loop_end = destination_end - 255;
        while (aligned_destination < loop_end) {
            _mm512_store_si512((__m512i*)aligned_destination, zero_vector);
            _mm512_store_si512((__m512i*)(aligned_destination + 64), zero_vector);
            _mm512_store_si512((__m512i*)(aligned_destination + 128), zero_vector);
            _mm512_store_si512((__m512i*)(aligned_destination + 192), zero_vector);
            aligned_destination += 256;
        }
        const unsigned char* tail_end = destination_end - 63;
        while (aligned_destination < tail_end) {
            _mm512_store_si512((__m512i*)aligned_destination, zero_vector);
            aligned_destination += 64;
        }
    }

    _mm512_storeu_si512((__m512i*)(destination_end - 64), zero_vector);
    return true;
}

__attribute__((target("avx2"))) static bool bc_core_zero_avx2(void* dst, size_t len)
{
    if (len == 0) {
        return true;
    }

    unsigned char* destination = (unsigned char*)dst;
    __m256i zero_vector = _mm256_setzero_si256();

    if (len < 32) {
        for (size_t byte_index = 0; byte_index < len; byte_index++) {
            destination[byte_index] = 0;
        }
        return true;
    }

    if (len <= 128) {
        _mm256_storeu_si256((__m256i*)destination, zero_vector);
        if (len >= 64) {
            _mm256_storeu_si256((__m256i*)(destination + 32), zero_vector);
        }
        if (len >= 96) {
            _mm256_storeu_si256((__m256i*)(destination + 64), zero_vector);
        }
        _mm256_storeu_si256((__m256i*)(destination + len - 32), zero_vector);
        return true;
    }

    _mm256_storeu_si256((__m256i*)destination, zero_vector);

    unsigned char* aligned_destination = (unsigned char*)(((uintptr_t)destination + 32) & ~(uintptr_t)31);
    unsigned char* destination_end = destination + len;

    if (len > bc_core_cached_l3_cache_size()) { /* GCOVR_EXCL_BR_LINE -- NT stores > L3, tested by benchmarks */
        /* GCOVR_EXCL_START -- NT stores > L3, tested by benchmarks */
        const unsigned char* loop_end = destination_end - 127;
        while (aligned_destination < loop_end) {
            _mm256_stream_si256((__m256i*)aligned_destination, zero_vector);
            _mm256_stream_si256((__m256i*)(aligned_destination + 32), zero_vector);
            _mm256_stream_si256((__m256i*)(aligned_destination + 64), zero_vector);
            _mm256_stream_si256((__m256i*)(aligned_destination + 96), zero_vector);
            aligned_destination += 128;
        }
        const unsigned char* tail_end = destination_end - 31;
        while (aligned_destination < tail_end) {
            _mm256_stream_si256((__m256i*)aligned_destination, zero_vector);
            aligned_destination += 32;
        }
        _mm_sfence();
        /* GCOVR_EXCL_STOP */
    } else {
        const unsigned char* loop_end = destination_end - 127;
        while (aligned_destination < loop_end) {
            _mm256_store_si256((__m256i*)aligned_destination, zero_vector);
            _mm256_store_si256((__m256i*)(aligned_destination + 32), zero_vector);
            _mm256_store_si256((__m256i*)(aligned_destination + 64), zero_vector);
            _mm256_store_si256((__m256i*)(aligned_destination + 96), zero_vector);
            aligned_destination += 128;
        }
        const unsigned char* tail_end = destination_end - 31;
        while (aligned_destination < tail_end) {
            _mm256_store_si256((__m256i*)aligned_destination, zero_vector);
            aligned_destination += 32;
        }
    }

    _mm256_storeu_si256((__m256i*)(destination_end - 32), zero_vector);

    return true;
}

static bool (*g_zero_impl)(void*, size_t) = bc_core_zero_avx2;

__attribute__((target("avx2"))) static bool bc_core_zero_avx2_with_policy(void* dst, size_t len, bc_core_cache_policy_t policy)
{
    if (len == 0) {
        return true;
    }

    unsigned char* destination = (unsigned char*)dst;
    __m256i zero_vector = _mm256_setzero_si256();

    if (len < 32) {
        for (size_t byte_index = 0; byte_index < len; byte_index++) {
            destination[byte_index] = 0;
        }
        return true;
    }

    if (len <= 128) {
        _mm256_storeu_si256((__m256i*)destination, zero_vector);
        if (len >= 64) {
            _mm256_storeu_si256((__m256i*)(destination + 32), zero_vector);
        }
        if (len >= 96) {
            _mm256_storeu_si256((__m256i*)(destination + 64), zero_vector);
        }
        _mm256_storeu_si256((__m256i*)(destination + len - 32), zero_vector);
        return true;
    }

    _mm256_storeu_si256((__m256i*)destination, zero_vector);

    unsigned char* aligned_destination = (unsigned char*)(((uintptr_t)destination + 32) & ~(uintptr_t)31);
    unsigned char* destination_end = destination + len;

    const bool use_nt = (policy == BC_CORE_CACHE_POLICY_STREAMING) ? true
                        : (policy == BC_CORE_CACHE_POLICY_CACHED)  ? false
                                                                   : (len > bc_core_cached_l3_cache_size());

    if (use_nt) {
        const unsigned char* loop_end = destination_end - 127;
        while (aligned_destination < loop_end) {
            _mm256_stream_si256((__m256i*)aligned_destination, zero_vector);
            _mm256_stream_si256((__m256i*)(aligned_destination + 32), zero_vector);
            _mm256_stream_si256((__m256i*)(aligned_destination + 64), zero_vector);
            _mm256_stream_si256((__m256i*)(aligned_destination + 96), zero_vector);
            aligned_destination += 128;
        }
        const unsigned char* tail_end = destination_end - 31;
        while (aligned_destination < tail_end) {
            _mm256_stream_si256((__m256i*)aligned_destination, zero_vector);
            aligned_destination += 32;
        }
        _mm_sfence();
    } else {
        const unsigned char* loop_end = destination_end - 127;
        while (aligned_destination < loop_end) {
            _mm256_store_si256((__m256i*)aligned_destination, zero_vector);
            _mm256_store_si256((__m256i*)(aligned_destination + 32), zero_vector);
            _mm256_store_si256((__m256i*)(aligned_destination + 64), zero_vector);
            _mm256_store_si256((__m256i*)(aligned_destination + 96), zero_vector);
            aligned_destination += 128;
        }
        const unsigned char* tail_end = destination_end - 31;
        while (aligned_destination < tail_end) {
            _mm256_store_si256((__m256i*)aligned_destination, zero_vector);
            aligned_destination += 32;
        }
    }

    _mm256_storeu_si256((__m256i*)(destination_end - 32), zero_vector);

    return true;
}

static bool (*g_zero_with_policy_impl)(void*, size_t, bc_core_cache_policy_t) = bc_core_zero_avx2_with_policy;

__attribute__((constructor)) void bc_core_zero_dispatch_init(void)
{
    bc_core_cpu_features_t features = {0};
    bool detected = bc_core_cpu_features_detect(&features);
    if (detected && features.has_avx512f && features.has_avx512bw) {
        g_zero_impl = bc_core_zero_avx512;
    }
    else {
        g_zero_impl = bc_core_zero_avx2;
    }
    g_zero_with_policy_impl = bc_core_zero_avx2_with_policy;
}

bool bc_core_zero(void* dst, size_t len)
{
    return g_zero_impl(dst, len);
}

bool bc_core_zero_with_policy(void* dst, size_t len, bc_core_cache_policy_t policy)
{
    return g_zero_with_policy_impl(dst, len, policy);
}
