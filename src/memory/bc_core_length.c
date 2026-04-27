// SPDX-License-Identifier: MIT

#include "bc_core.h"
#include "bc_core_cpu_features_internal.h"

#include <immintrin.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

__attribute__((target("avx512f,avx512bw"), no_sanitize("address"))) static bool length_avx512(const void* data, unsigned char terminator,
                                                                                                size_t* out_length)
{
    const unsigned char* bytes = (const unsigned char*)data;
    __m512i target_vec = _mm512_set1_epi8((char)terminator);

    const unsigned char* aligned_ptr = (const unsigned char*)((uintptr_t)bytes & ~(uintptr_t)63);
    size_t prefix_count = (size_t)(bytes - aligned_ptr);
    __m512i first_block = _mm512_load_si512((const __m512i*)aligned_ptr);
    __mmask64 first_mask = _mm512_cmpeq_epi8_mask(first_block, target_vec);
    first_mask >>= prefix_count;
    if (first_mask != 0) {
        *out_length = (size_t)__builtin_ctzll(first_mask);
        return true;
    }

    const unsigned char* ptr = aligned_ptr + 64;

    while ((uintptr_t)ptr & 255) {
        __mmask64 mask = _mm512_cmpeq_epi8_mask(_mm512_load_si512((const __m512i*)ptr), target_vec);
        if (mask) {
            *out_length = (size_t)(ptr - bytes) + (size_t)__builtin_ctzll(mask);
            return true;
        }
        ptr += 64;
    }

    for (;;) {
        __m512i c0 = _mm512_load_si512((const __m512i*)(ptr + 0));
        __m512i c1 = _mm512_load_si512((const __m512i*)(ptr + 64));
        __m512i c2 = _mm512_load_si512((const __m512i*)(ptr + 128));
        __m512i c3 = _mm512_load_si512((const __m512i*)(ptr + 192));
        __mmask64 m0 = _mm512_cmpeq_epi8_mask(c0, target_vec);
        __mmask64 m1 = _mm512_cmpeq_epi8_mask(c1, target_vec);
        __mmask64 m2 = _mm512_cmpeq_epi8_mask(c2, target_vec);
        __mmask64 m3 = _mm512_cmpeq_epi8_mask(c3, target_vec);

        if ((m0 | m1 | m2 | m3) != 0) {
            if (m0) {
                *out_length = (size_t)(ptr - bytes) + (size_t)__builtin_ctzll(m0);
                return true;
            }
            if (m1) {
                *out_length = (size_t)(ptr - bytes) + 64 + (size_t)__builtin_ctzll(m1);
                return true;
            }
            if (m2) {
                *out_length = (size_t)(ptr - bytes) + 128 + (size_t)__builtin_ctzll(m2);
                return true;
            }
            *out_length = (size_t)(ptr - bytes) + 192 + (size_t)__builtin_ctzll(m3);
            return true;
        }
        ptr += 256;
    }
}

__attribute__((target("avx2"), no_sanitize("address"))) static bool length_avx2(const void* data, unsigned char terminator,
                                                                                size_t* out_length)
{
    const unsigned char* bytes = (const unsigned char*)data;
    __m256i target_vec = _mm256_set1_epi8((char)terminator);

    const unsigned char* aligned_ptr = (const unsigned char*)((uintptr_t)bytes & ~(uintptr_t)31);
    size_t prefix_count = (size_t)(bytes - aligned_ptr);
    __m256i first_block = _mm256_load_si256((const __m256i*)aligned_ptr);
    unsigned int first_mask = (unsigned int)_mm256_movemask_epi8(_mm256_cmpeq_epi8(first_block, target_vec));
    first_mask >>= prefix_count;
    if (first_mask != 0) {
        *out_length = (size_t)__builtin_ctz(first_mask);
        return true;
    }

    const unsigned char* ptr = aligned_ptr + 32;

    while ((uintptr_t)ptr & 127) {
        unsigned int mask = (unsigned int)_mm256_movemask_epi8(_mm256_cmpeq_epi8(_mm256_load_si256((const __m256i*)ptr), target_vec));
        if (mask) {
            *out_length = (size_t)(ptr - bytes) + (size_t)__builtin_ctz(mask);
            return true;
        }
        ptr += 32;
    }

    for (;;) {
        __m256i v0 = _mm256_cmpeq_epi8(_mm256_load_si256((const __m256i*)(ptr + 0)), target_vec);
        __m256i v1 = _mm256_cmpeq_epi8(_mm256_load_si256((const __m256i*)(ptr + 32)), target_vec);
        __m256i v2 = _mm256_cmpeq_epi8(_mm256_load_si256((const __m256i*)(ptr + 64)), target_vec);
        __m256i v3 = _mm256_cmpeq_epi8(_mm256_load_si256((const __m256i*)(ptr + 96)), target_vec);
        __m256i any = _mm256_or_si256(_mm256_or_si256(v0, v1), _mm256_or_si256(v2, v3));

        if ((unsigned int)_mm256_movemask_epi8(any) != 0) {
            unsigned int mask;
            mask = (unsigned int)_mm256_movemask_epi8(v0);
            if (mask) {
                *out_length = (size_t)(ptr - bytes) + (size_t)__builtin_ctz(mask);
                return true;
            }
            mask = (unsigned int)_mm256_movemask_epi8(v1);
            if (mask) {
                *out_length = (size_t)(ptr - bytes) + 32 + (size_t)__builtin_ctz(mask);
                return true;
            }
            mask = (unsigned int)_mm256_movemask_epi8(v2);
            if (mask) {
                *out_length = (size_t)(ptr - bytes) + 64 + (size_t)__builtin_ctz(mask);
                return true;
            }
            mask = (unsigned int)_mm256_movemask_epi8(v3);
            *out_length = (size_t)(ptr - bytes) + 96 + (size_t)__builtin_ctz(mask);
            return true;
        }
        ptr += 128;
    }
}

static bool (*g_length_impl)(const void*, unsigned char, size_t*) = length_avx2;

__attribute__((constructor)) void bc_core_length_dispatch_init(void)
{
    bc_core_cpu_features_t features = {0};
    bool detected = bc_core_cpu_features_detect(&features);
    if (detected && features.has_avx512f && features.has_avx512bw) {
        g_length_impl = length_avx512;
    }
    else {
        g_length_impl = length_avx2;
    }
}

bool bc_core_length(const void* data, unsigned char terminator, size_t* out_length)
{
#if defined(__SANITIZE_ADDRESS__)
    const unsigned char* ptr = (const unsigned char*)data;
    size_t length = 0;
    while (ptr[length] != terminator) {
        length++;
    }
    *out_length = length;
    return true;
#else
    return g_length_impl(data, terminator, out_length);
#endif
}
