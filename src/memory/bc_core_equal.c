// SPDX-License-Identifier: MIT

#include "bc_core.h"

#include "bc_core_cpu_features_internal.h"

#include <immintrin.h>
#include <stdint.h>

__attribute__((target("avx512f,avx512bw"))) static bool bc_core_equal_avx512(const void* a, const void* b, size_t len,
                                                                             bool* out_equal)
{
    if (len == 0) {
        *out_equal = true;
        return true;
    }

    const unsigned char* pointer_a = (const unsigned char*)a;
    const unsigned char* pointer_b = (const unsigned char*)b;
    size_t offset = 0;

    while (offset + 256 <= len) {
        __m512i a0 = _mm512_loadu_si512((const __m512i*)(pointer_a + offset));
        __m512i a1 = _mm512_loadu_si512((const __m512i*)(pointer_a + offset + 64));
        __m512i a2 = _mm512_loadu_si512((const __m512i*)(pointer_a + offset + 128));
        __m512i a3 = _mm512_loadu_si512((const __m512i*)(pointer_a + offset + 192));
        __m512i b0 = _mm512_loadu_si512((const __m512i*)(pointer_b + offset));
        __m512i b1 = _mm512_loadu_si512((const __m512i*)(pointer_b + offset + 64));
        __m512i b2 = _mm512_loadu_si512((const __m512i*)(pointer_b + offset + 128));
        __m512i b3 = _mm512_loadu_si512((const __m512i*)(pointer_b + offset + 192));

        __mmask64 m0 = _mm512_cmpneq_epi8_mask(a0, b0);
        __mmask64 m1 = _mm512_cmpneq_epi8_mask(a1, b1);
        __mmask64 m2 = _mm512_cmpneq_epi8_mask(a2, b2);
        __mmask64 m3 = _mm512_cmpneq_epi8_mask(a3, b3);

        if ((m0 | m1 | m2 | m3) != 0) {
            *out_equal = false;
            return true;
        }
        offset += 256;
    }

    while (offset + 64 <= len) {
        __m512i chunk_a = _mm512_loadu_si512((const __m512i*)(pointer_a + offset));
        __m512i chunk_b = _mm512_loadu_si512((const __m512i*)(pointer_b + offset));
        __mmask64 mismatch = _mm512_cmpneq_epi8_mask(chunk_a, chunk_b);
        if (mismatch != 0) {
            *out_equal = false;
            return true;
        }
        offset += 64;
    }

    if (offset < len) {
        size_t tail = len - offset;
        __mmask64 mask = (1ULL << tail) - 1ULL;
        __m512i chunk_a = _mm512_maskz_loadu_epi8(mask, pointer_a + offset);
        __m512i chunk_b = _mm512_maskz_loadu_epi8(mask, pointer_b + offset);
        __mmask64 mismatch = _mm512_mask_cmpneq_epi8_mask(mask, chunk_a, chunk_b);
        if (mismatch != 0) {
            *out_equal = false;
            return true;
        }
    }

    *out_equal = true;
    return true;
}

__attribute__((target("avx2"))) static bool bc_core_equal_avx2(const void* a, const void* b, size_t len, bool* out_equal)
{
    if (len == 0) {
        *out_equal = true;
        return true;
    }

    const unsigned char* pointer_a = (const unsigned char*)a;
    const unsigned char* pointer_b = (const unsigned char*)b;

    const __m256i ones = _mm256_set1_epi8((char)0xFF);
    size_t offset = 0;

    while (offset + 128 <= len) {
        __m256i a0 = _mm256_loadu_si256((const __m256i*)(pointer_a + offset));
        __m256i a1 = _mm256_loadu_si256((const __m256i*)(pointer_a + offset + 32));
        __m256i a2 = _mm256_loadu_si256((const __m256i*)(pointer_a + offset + 64));
        __m256i a3 = _mm256_loadu_si256((const __m256i*)(pointer_a + offset + 96));
        __m256i b0 = _mm256_loadu_si256((const __m256i*)(pointer_b + offset));
        __m256i b1 = _mm256_loadu_si256((const __m256i*)(pointer_b + offset + 32));
        __m256i b2 = _mm256_loadu_si256((const __m256i*)(pointer_b + offset + 64));
        __m256i b3 = _mm256_loadu_si256((const __m256i*)(pointer_b + offset + 96));

        __m256i eq0 = _mm256_cmpeq_epi8(a0, b0);
        __m256i eq1 = _mm256_cmpeq_epi8(a1, b1);
        __m256i eq2 = _mm256_cmpeq_epi8(a2, b2);
        __m256i eq3 = _mm256_cmpeq_epi8(a3, b3);

        __m256i combined = _mm256_and_si256(_mm256_and_si256(eq0, eq1), _mm256_and_si256(eq2, eq3));
        if (!_mm256_testc_si256(combined, ones)) {
            *out_equal = false;
            return true;
        }
        offset += 128;
    }

    while (offset + 32 <= len) {
        __m256i chunk_a = _mm256_loadu_si256((const __m256i*)(pointer_a + offset));
        __m256i chunk_b = _mm256_loadu_si256((const __m256i*)(pointer_b + offset));
        __m256i comparison_result = _mm256_cmpeq_epi8(chunk_a, chunk_b);
        unsigned int movemask_result = (unsigned int)_mm256_movemask_epi8(comparison_result);
        if (movemask_result != 0xFFFFFFFFu) {
            *out_equal = false;
            return true;
        }
        offset += 32;
    }

    while (offset < len) {
        if (pointer_a[offset] != pointer_b[offset]) {
            *out_equal = false;
            return true;
        }
        offset += 1;
    }

    *out_equal = true;
    return true;
}

static bool (*g_equal_impl)(const void*, const void*, size_t, bool*) = bc_core_equal_avx2;

__attribute__((constructor)) void bc_core_equal_dispatch_init(void)
{
    bc_core_cpu_features_t features = {0};
    bool detected = bc_core_cpu_features_detect(&features);
    if (detected && features.has_avx512f && features.has_avx512bw) {
        g_equal_impl = bc_core_equal_avx512;
    }
    else {
        g_equal_impl = bc_core_equal_avx2;
    }
}

bool bc_core_equal(const void* a, const void* b, size_t len, bool* out_equal)
{
    return g_equal_impl(a, b, len, out_equal);
}
