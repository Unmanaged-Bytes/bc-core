// SPDX-License-Identifier: MIT

#include "bc_core.h"

#include "bc_core_align_prologue_internal.h"
#include "bc_core_buffer_thresholds_internal.h"
#include "bc_core_cache_sizes_internal.h"
#include "bc_core_cpu_features_internal.h"

#include <immintrin.h>
#include <stdint.h>

__attribute__((target("avx512f,avx512bw"))) static bool bc_core_copy_avx512(void* dst, const void* src, size_t len)
{
    if (len == 0) {
        return true;
    }

    unsigned char* destination = (unsigned char*)dst;
    const unsigned char* source = (const unsigned char*)src;

    if (len < 32) {
        for (size_t byte_index = 0; byte_index < len; byte_index++) {
            destination[byte_index] = source[byte_index];
        }
        return true;
    }

    if (len <= 64) {
        __m256i head = _mm256_loadu_si256((const __m256i*)source);
        _mm256_storeu_si256((__m256i*)destination, head);
        __m256i tail = _mm256_loadu_si256((const __m256i*)(source + len - 32));
        _mm256_storeu_si256((__m256i*)(destination + len - 32), tail);
        return true;
    }

    if (len <= 256) {
        __m512i head = _mm512_loadu_si512((const __m512i*)source);
        _mm512_storeu_si512((__m512i*)destination, head);
        if (len > 192) {
            __m512i mid1 = _mm512_loadu_si512((const __m512i*)(source + 64));
            __m512i mid2 = _mm512_loadu_si512((const __m512i*)(source + 128));
            _mm512_storeu_si512((__m512i*)(destination + 64), mid1);
            _mm512_storeu_si512((__m512i*)(destination + 128), mid2);
        } else if (len > 128) {
            __m512i mid = _mm512_loadu_si512((const __m512i*)(source + 64));
            _mm512_storeu_si512((__m512i*)(destination + 64), mid);
        }
        __m512i tail = _mm512_loadu_si512((const __m512i*)(source + len - 64));
        _mm512_storeu_si512((__m512i*)(destination + len - 64), tail);
        return true;
    }

    __m512i head_chunk = _mm512_loadu_si512((const __m512i*)source);
    _mm512_storeu_si512((__m512i*)destination, head_chunk);

    size_t head_byte_count = bc_core_simd_align_dst_to_64_offset(destination);
    unsigned char* aligned_destination = destination + head_byte_count;
    const unsigned char* source_at_aligned = source + head_byte_count;
    unsigned char* destination_end = destination + len;

    if (bc_core_simd_should_stream_above_half_l3(len)) {
        /* GCOVR_EXCL_START -- NT stores > L3/2, tested by benchmarks */
        const unsigned char* loop_end = destination_end - 255;
        while (aligned_destination < loop_end) {
            __m512i c0 = _mm512_loadu_si512((const __m512i*)source_at_aligned);
            __m512i c1 = _mm512_loadu_si512((const __m512i*)(source_at_aligned + 64));
            __m512i c2 = _mm512_loadu_si512((const __m512i*)(source_at_aligned + 128));
            __m512i c3 = _mm512_loadu_si512((const __m512i*)(source_at_aligned + 192));
            _mm512_stream_si512((__m512i*)aligned_destination, c0);
            _mm512_stream_si512((__m512i*)(aligned_destination + 64), c1);
            _mm512_stream_si512((__m512i*)(aligned_destination + 128), c2);
            _mm512_stream_si512((__m512i*)(aligned_destination + 192), c3);
            source_at_aligned += 256;
            aligned_destination += 256;
        }
        const unsigned char* tail_end = destination_end - 63;
        while (aligned_destination < tail_end) {
            __m512i chunk = _mm512_loadu_si512((const __m512i*)source_at_aligned);
            _mm512_stream_si512((__m512i*)aligned_destination, chunk);
            source_at_aligned += 64;
            aligned_destination += 64;
        }
        _mm_sfence();
        /* GCOVR_EXCL_STOP */
    } else {
        const unsigned char* loop_end = destination_end - 255;
        while (aligned_destination < loop_end) {
            __m512i c0 = _mm512_loadu_si512((const __m512i*)source_at_aligned);
            __m512i c1 = _mm512_loadu_si512((const __m512i*)(source_at_aligned + 64));
            __m512i c2 = _mm512_loadu_si512((const __m512i*)(source_at_aligned + 128));
            __m512i c3 = _mm512_loadu_si512((const __m512i*)(source_at_aligned + 192));
            _mm512_store_si512((__m512i*)aligned_destination, c0);
            _mm512_store_si512((__m512i*)(aligned_destination + 64), c1);
            _mm512_store_si512((__m512i*)(aligned_destination + 128), c2);
            _mm512_store_si512((__m512i*)(aligned_destination + 192), c3);
            source_at_aligned += 256;
            aligned_destination += 256;
        }
        const unsigned char* tail_end = destination_end - 63;
        while (aligned_destination < tail_end) {
            __m512i chunk = _mm512_loadu_si512((const __m512i*)source_at_aligned);
            _mm512_store_si512((__m512i*)aligned_destination, chunk);
            source_at_aligned += 64;
            aligned_destination += 64;
        }
    }

    __m512i tail_chunk = _mm512_loadu_si512((const __m512i*)(source + len - 64));
    _mm512_storeu_si512((__m512i*)(destination_end - 64), tail_chunk);
    return true;
}

__attribute__((target("avx2"))) static bool bc_core_copy_avx2(void* dst, const void* src, size_t len)
{
    if (len == 0) {
        return true;
    }

    unsigned char* destination = (unsigned char*)dst;
    const unsigned char* source = (const unsigned char*)src;

    if (len < 32) {
        for (size_t byte_index = 0; byte_index < len; byte_index++) {
            destination[byte_index] = source[byte_index];
        }
        return true;
    }

    if (len <= 128) {
        __m256i head = _mm256_loadu_si256((const __m256i*)source);
        _mm256_storeu_si256((__m256i*)destination, head);
        if (len > 96) {
            __m256i mid1 = _mm256_loadu_si256((const __m256i*)(source + 32));
            __m256i mid2 = _mm256_loadu_si256((const __m256i*)(source + 64));
            _mm256_storeu_si256((__m256i*)(destination + 32), mid1);
            _mm256_storeu_si256((__m256i*)(destination + 64), mid2);
        } else if (len > 64) {
            __m256i mid = _mm256_loadu_si256((const __m256i*)(source + 32));
            _mm256_storeu_si256((__m256i*)(destination + 32), mid);
        }
        __m256i tail = _mm256_loadu_si256((const __m256i*)(source + len - 32));
        _mm256_storeu_si256((__m256i*)(destination + len - 32), tail);
        return true;
    }

    const size_t l3_cache_size = bc_core_cached_l3_cache_size();
    const size_t nt_threshold = (l3_cache_size > 0) ? l3_cache_size / 2 : (4 * 1024 * 1024);

    __m256i head_chunk = _mm256_loadu_si256((const __m256i*)source);
    _mm256_storeu_si256((__m256i*)destination, head_chunk);

    unsigned char* aligned_destination = (unsigned char*)(((uintptr_t)destination + 32) & ~(uintptr_t)31);
    size_t head_byte_count = (size_t)(aligned_destination - destination);
    const unsigned char* source_at_aligned = source + head_byte_count;
    unsigned char* destination_end = destination + len;

    if (len > nt_threshold) { /* GCOVR_EXCL_BR_LINE -- NT stores > L3/2, tested by benchmarks */
        /* GCOVR_EXCL_START -- NT stores > L3/2, tested by benchmarks */
        const unsigned char* loop_end = destination_end - 127;
        while (aligned_destination < loop_end) {
            __m256i c0 = _mm256_loadu_si256((const __m256i*)source_at_aligned);
            __m256i c1 = _mm256_loadu_si256((const __m256i*)(source_at_aligned + 32));
            __m256i c2 = _mm256_loadu_si256((const __m256i*)(source_at_aligned + 64));
            __m256i c3 = _mm256_loadu_si256((const __m256i*)(source_at_aligned + 96));
            _mm256_stream_si256((__m256i*)aligned_destination, c0);
            _mm256_stream_si256((__m256i*)(aligned_destination + 32), c1);
            _mm256_stream_si256((__m256i*)(aligned_destination + 64), c2);
            _mm256_stream_si256((__m256i*)(aligned_destination + 96), c3);
            source_at_aligned += 128;
            aligned_destination += 128;
        }
        const unsigned char* tail_end = destination_end - 31;
        while (aligned_destination < tail_end) {
            __m256i chunk = _mm256_loadu_si256((const __m256i*)source_at_aligned);
            _mm256_stream_si256((__m256i*)aligned_destination, chunk);
            source_at_aligned += 32;
            aligned_destination += 32;
        }
        _mm_sfence();
        /* GCOVR_EXCL_STOP */
    } else {
        const unsigned char* loop_end = destination_end - 127;
        while (aligned_destination < loop_end) {
            __m256i c0 = _mm256_loadu_si256((const __m256i*)source_at_aligned);
            __m256i c1 = _mm256_loadu_si256((const __m256i*)(source_at_aligned + 32));
            __m256i c2 = _mm256_loadu_si256((const __m256i*)(source_at_aligned + 64));
            __m256i c3 = _mm256_loadu_si256((const __m256i*)(source_at_aligned + 96));
            _mm256_store_si256((__m256i*)aligned_destination, c0);
            _mm256_store_si256((__m256i*)(aligned_destination + 32), c1);
            _mm256_store_si256((__m256i*)(aligned_destination + 64), c2);
            _mm256_store_si256((__m256i*)(aligned_destination + 96), c3);
            source_at_aligned += 128;
            aligned_destination += 128;
        }
        const unsigned char* tail_end = destination_end - 31;
        while (aligned_destination < tail_end) {
            __m256i chunk = _mm256_loadu_si256((const __m256i*)source_at_aligned);
            _mm256_store_si256((__m256i*)aligned_destination, chunk);
            source_at_aligned += 32;
            aligned_destination += 32;
        }
    }

    __m256i tail_chunk = _mm256_loadu_si256((const __m256i*)(source + len - 32));
    _mm256_storeu_si256((__m256i*)(destination_end - 32), tail_chunk);
    return true;
}

static bool (*g_copy_impl)(void*, const void*, size_t) = bc_core_copy_avx2;

__attribute__((target("avx2"))) static bool bc_core_copy_avx2_with_policy_internal(void* dst, const void* src, size_t len,
                                                                                   bc_core_cache_policy_t policy,
                                                                                   size_t auto_streaming_threshold_bytes)
{
    if (len == 0) {
        return true;
    }

    unsigned char* destination = (unsigned char*)dst;
    const unsigned char* source = (const unsigned char*)src;

    if (len < 32) {
        for (size_t byte_index = 0; byte_index < len; byte_index++) {
            destination[byte_index] = source[byte_index];
        }
        return true;
    }

    if (len <= 128) {
        __m256i head = _mm256_loadu_si256((const __m256i*)source);
        _mm256_storeu_si256((__m256i*)destination, head);
        if (len > 96) {
            __m256i mid1 = _mm256_loadu_si256((const __m256i*)(source + 32));
            __m256i mid2 = _mm256_loadu_si256((const __m256i*)(source + 64));
            _mm256_storeu_si256((__m256i*)(destination + 32), mid1);
            _mm256_storeu_si256((__m256i*)(destination + 64), mid2);
        } else if (len > 64) {
            __m256i mid = _mm256_loadu_si256((const __m256i*)(source + 32));
            _mm256_storeu_si256((__m256i*)(destination + 32), mid);
        }
        __m256i tail = _mm256_loadu_si256((const __m256i*)(source + len - 32));
        _mm256_storeu_si256((__m256i*)(destination + len - 32), tail);
        return true;
    }

    const size_t l3_cache_size = bc_core_cached_l3_cache_size();
    const size_t default_nt_threshold = (l3_cache_size > 0) ? l3_cache_size / 2 : (4 * 1024 * 1024);

    bool use_nt;
    if (policy == BC_CORE_CACHE_POLICY_STREAMING) {
        use_nt = true;
    } else if (policy == BC_CORE_CACHE_POLICY_CACHED) {
        use_nt = false;
    } else if (policy == BC_CORE_CACHE_POLICY_AUTO) {
        use_nt = (len >= auto_streaming_threshold_bytes);
    } else {
        use_nt = (len > default_nt_threshold);
    }

    __m256i head_chunk = _mm256_loadu_si256((const __m256i*)source);
    _mm256_storeu_si256((__m256i*)destination, head_chunk);

    unsigned char* aligned_destination = (unsigned char*)(((uintptr_t)destination + 32) & ~(uintptr_t)31);
    size_t head_byte_count = (size_t)(aligned_destination - destination);
    const unsigned char* source_at_aligned = source + head_byte_count;
    unsigned char* destination_end = destination + len;

    if (use_nt) {
        const unsigned char* loop_end = destination_end - 127;
        while (aligned_destination < loop_end) {
            __m256i c0 = _mm256_loadu_si256((const __m256i*)source_at_aligned);
            __m256i c1 = _mm256_loadu_si256((const __m256i*)(source_at_aligned + 32));
            __m256i c2 = _mm256_loadu_si256((const __m256i*)(source_at_aligned + 64));
            __m256i c3 = _mm256_loadu_si256((const __m256i*)(source_at_aligned + 96));
            _mm256_stream_si256((__m256i*)aligned_destination, c0);
            _mm256_stream_si256((__m256i*)(aligned_destination + 32), c1);
            _mm256_stream_si256((__m256i*)(aligned_destination + 64), c2);
            _mm256_stream_si256((__m256i*)(aligned_destination + 96), c3);
            source_at_aligned += 128;
            aligned_destination += 128;
        }
        const unsigned char* tail_end = destination_end - 31;
        while (aligned_destination < tail_end) {
            __m256i chunk = _mm256_loadu_si256((const __m256i*)source_at_aligned);
            _mm256_stream_si256((__m256i*)aligned_destination, chunk);
            source_at_aligned += 32;
            aligned_destination += 32;
        }
        _mm_sfence();
    } else {
        const unsigned char* loop_end = destination_end - 127;
        while (aligned_destination < loop_end) {
            __m256i c0 = _mm256_loadu_si256((const __m256i*)source_at_aligned);
            __m256i c1 = _mm256_loadu_si256((const __m256i*)(source_at_aligned + 32));
            __m256i c2 = _mm256_loadu_si256((const __m256i*)(source_at_aligned + 64));
            __m256i c3 = _mm256_loadu_si256((const __m256i*)(source_at_aligned + 96));
            _mm256_store_si256((__m256i*)aligned_destination, c0);
            _mm256_store_si256((__m256i*)(aligned_destination + 32), c1);
            _mm256_store_si256((__m256i*)(aligned_destination + 64), c2);
            _mm256_store_si256((__m256i*)(aligned_destination + 96), c3);
            source_at_aligned += 128;
            aligned_destination += 128;
        }
        const unsigned char* tail_end = destination_end - 31;
        while (aligned_destination < tail_end) {
            __m256i chunk = _mm256_loadu_si256((const __m256i*)source_at_aligned);
            _mm256_store_si256((__m256i*)aligned_destination, chunk);
            source_at_aligned += 32;
            aligned_destination += 32;
        }
    }

    __m256i tail_chunk = _mm256_loadu_si256((const __m256i*)(source + len - 32));
    _mm256_storeu_si256((__m256i*)(destination_end - 32), tail_chunk);
    return true;
}

static bool (*g_copy_with_policy_impl)(void*, const void*, size_t, bc_core_cache_policy_t, size_t) = bc_core_copy_avx2_with_policy_internal;

__attribute__((constructor)) void bc_core_copy_dispatch_init(void)
{
    bc_core_cpu_features_t features = {0};
    bool detected = bc_core_cpu_features_detect(&features);
    if (detected && features.has_avx512f && features.has_avx512bw) {
        g_copy_impl = bc_core_copy_avx512;
    } else {
        g_copy_impl = bc_core_copy_avx2;
    }
    g_copy_with_policy_impl = bc_core_copy_avx2_with_policy_internal;
}

bool bc_core_copy(void* dst, const void* src, size_t len)
{
    return g_copy_impl(dst, src, len);
}

bool bc_core_copy_with_policy(void* dst, const void* src, size_t len, bc_core_cache_policy_t policy)
{
    return g_copy_with_policy_impl(dst, src, len, policy, bc_core_buffer_thresholds_default_copy_streaming_bytes());
}

bool bc_core_copy_with_policy_threaded(void* dst, const void* src, size_t len, bc_core_cache_policy_t policy, size_t worker_count_hint)
{
    bc_core_buffer_thresholds_t thresholds;
    bc_core_buffer_thresholds(worker_count_hint, &thresholds);
    return g_copy_with_policy_impl(dst, src, len, policy, thresholds.copy_streaming_threshold_bytes);
}
