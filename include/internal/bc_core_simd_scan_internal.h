// SPDX-License-Identifier: MIT

#ifndef BC_CORE_SIMD_SCAN_INTERNAL_H
#define BC_CORE_SIMD_SCAN_INTERNAL_H

#include <immintrin.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

__attribute__((target("avx512f,avx512bw"))) static inline bool bc_core_simd_scan_first_diff_avx512(const void* a, const void* b, size_t len,
                                                                                                   size_t* out_first_diff_offset)
{
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
            if (m0 != 0) {
                *out_first_diff_offset = offset + (size_t)__builtin_ctzll(m0);
                return true;
            }
            if (m1 != 0) {
                *out_first_diff_offset = offset + 64 + (size_t)__builtin_ctzll(m1);
                return true;
            }
            if (m2 != 0) {
                *out_first_diff_offset = offset + 128 + (size_t)__builtin_ctzll(m2);
                return true;
            }
            *out_first_diff_offset = offset + 192 + (size_t)__builtin_ctzll(m3);
            return true;
        }
        offset += 256;
    }

    while (offset + 64 <= len) {
        __m512i chunk_a = _mm512_loadu_si512((const __m512i*)(pointer_a + offset));
        __m512i chunk_b = _mm512_loadu_si512((const __m512i*)(pointer_b + offset));
        __mmask64 mismatch = _mm512_cmpneq_epi8_mask(chunk_a, chunk_b);
        if (mismatch != 0) {
            *out_first_diff_offset = offset + (size_t)__builtin_ctzll(mismatch);
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
            *out_first_diff_offset = offset + (size_t)__builtin_ctzll(mismatch);
            return true;
        }
    }

    *out_first_diff_offset = len;
    return true;
}

#endif
