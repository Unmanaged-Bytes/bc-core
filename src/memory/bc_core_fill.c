// SPDX-License-Identifier: MIT

#include "bc_core.h"

#include "bc_core_align_prologue_internal.h"
#include "bc_core_buffer_thresholds_internal.h"
#include "bc_core_cache_sizes_internal.h"
#include "bc_core_cpu_features_internal.h"

#include <immintrin.h>
#include <stdint.h>

__attribute__((target("avx512f,avx512bw"))) static bool bc_core_fill_avx512(void* dst, size_t len, unsigned char value)
{
    if (len == 0) {
        return true;
    }

    unsigned char* destination = (unsigned char*)dst;
    __m512i fill_vector = _mm512_set1_epi8((char)value);

    if (len < 32) {
        for (size_t byte_index = 0; byte_index < len; byte_index++) {
            destination[byte_index] = value;
        }
        return true;
    }

    if (len <= 64) {
        __m256i fill_vector_ymm = _mm256_set1_epi8((char)value);
        _mm256_storeu_si256((__m256i*)destination, fill_vector_ymm);
        _mm256_storeu_si256((__m256i*)(destination + len - 32), fill_vector_ymm);
        return true;
    }

    if (len <= 256) {
        _mm512_storeu_si512((__m512i*)destination, fill_vector);
        if (len >= 128) {
            _mm512_storeu_si512((__m512i*)(destination + 64), fill_vector);
        }
        if (len >= 192) {
            _mm512_storeu_si512((__m512i*)(destination + 128), fill_vector);
        }
        _mm512_storeu_si512((__m512i*)(destination + len - 64), fill_vector);
        return true;
    }

    _mm512_storeu_si512((__m512i*)destination, fill_vector);

    unsigned char* aligned_destination = destination + bc_core_simd_align_dst_to_64_offset(destination);
    unsigned char* destination_end = destination + len;

    if (bc_core_simd_should_stream_above_l3(len)) {
        /* GCOVR_EXCL_START -- NT stores > L3, tested by benchmarks */
        const unsigned char* loop_end = destination_end - 255;
        while (aligned_destination < loop_end) {
            _mm512_stream_si512((__m512i*)aligned_destination, fill_vector);
            _mm512_stream_si512((__m512i*)(aligned_destination + 64), fill_vector);
            _mm512_stream_si512((__m512i*)(aligned_destination + 128), fill_vector);
            _mm512_stream_si512((__m512i*)(aligned_destination + 192), fill_vector);
            aligned_destination += 256;
        }
        const unsigned char* tail_end = destination_end - 63;
        while (aligned_destination < tail_end) {
            _mm512_stream_si512((__m512i*)aligned_destination, fill_vector);
            aligned_destination += 64;
        }
        _mm_sfence();
        /* GCOVR_EXCL_STOP */
    } else {
        const unsigned char* loop_end = destination_end - 255;
        while (aligned_destination < loop_end) {
            _mm512_store_si512((__m512i*)aligned_destination, fill_vector);
            _mm512_store_si512((__m512i*)(aligned_destination + 64), fill_vector);
            _mm512_store_si512((__m512i*)(aligned_destination + 128), fill_vector);
            _mm512_store_si512((__m512i*)(aligned_destination + 192), fill_vector);
            aligned_destination += 256;
        }
        const unsigned char* tail_end = destination_end - 63;
        while (aligned_destination < tail_end) {
            _mm512_store_si512((__m512i*)aligned_destination, fill_vector);
            aligned_destination += 64;
        }
    }

    _mm512_storeu_si512((__m512i*)(destination_end - 64), fill_vector);
    return true;
}

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

__attribute__((target("avx2"))) static bool bc_core_fill_avx2_with_policy_internal(void* dst, size_t len, unsigned char value,
                                                                                   bc_core_cache_policy_t policy,
                                                                                   size_t auto_streaming_threshold_bytes)
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

    bool use_nt;
    if (policy == BC_CORE_CACHE_POLICY_STREAMING) {
        use_nt = true;
    } else if (policy == BC_CORE_CACHE_POLICY_CACHED) {
        use_nt = false;
    } else if (policy == BC_CORE_CACHE_POLICY_AUTO) {
        use_nt = (len >= auto_streaming_threshold_bytes);
    } else {
        use_nt = (len > bc_core_cached_l3_cache_size());
    }

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

static bool (*g_fill_with_policy_impl)(void*, size_t, unsigned char, bc_core_cache_policy_t,
                                       size_t) = bc_core_fill_avx2_with_policy_internal;

__attribute__((constructor)) void bc_core_fill_dispatch_init(void)
{
    bc_core_cpu_features_t features = {0};
    bool detected = bc_core_cpu_features_detect(&features);
    if (detected && features.has_avx512f && features.has_avx512bw) {
        g_fill_impl = bc_core_fill_avx512;
    } else {
        g_fill_impl = bc_core_fill_avx2;
    }
    g_fill_with_policy_impl = bc_core_fill_avx2_with_policy_internal;
}

bool bc_core_fill(void* dst, size_t len, unsigned char value)
{
    return g_fill_impl(dst, len, value);
}

bool bc_core_fill_with_policy(void* dst, size_t len, unsigned char value, bc_core_cache_policy_t policy)
{
    return g_fill_with_policy_impl(dst, len, value, policy, bc_core_buffer_thresholds_default_fill_streaming_bytes());
}

bool bc_core_fill_with_policy_threaded(void* dst, size_t len, unsigned char value, bc_core_cache_policy_t policy, size_t worker_count_hint)
{
    bc_core_buffer_thresholds_t thresholds;
    bc_core_buffer_thresholds(worker_count_hint, &thresholds);
    return g_fill_with_policy_impl(dst, len, value, policy, thresholds.fill_streaming_threshold_bytes);
}
