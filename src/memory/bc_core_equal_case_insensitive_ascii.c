// SPDX-License-Identifier: MIT

#include "bc_core.h"

#include "bc_core_cpu_features_internal.h"

#include <immintrin.h>
#include <stdint.h>

static inline unsigned char to_lower_ascii_byte(unsigned char value)
{
    if (value >= 'A' && value <= 'Z') {
        return (unsigned char)(value | 0x20U);
    }
    return value;
}

__attribute__((target("avx2"))) static inline __m256i lower_ascii_letters_avx2(__m256i value)
{
    const __m256i ord_uppercase_A_minus_one = _mm256_set1_epi8((char)('A' - 1));
    const __m256i ord_uppercase_Z_plus_one = _mm256_set1_epi8((char)('Z' + 1));
    __m256i above_A = _mm256_cmpgt_epi8(value, ord_uppercase_A_minus_one);
    __m256i below_Z = _mm256_cmpgt_epi8(ord_uppercase_Z_plus_one, value);
    __m256i is_uppercase = _mm256_and_si256(above_A, below_Z);
    __m256i case_bit = _mm256_and_si256(is_uppercase, _mm256_set1_epi8(0x20));
    return _mm256_or_si256(value, case_bit);
}

__attribute__((target("avx2"))) static bool bc_core_equal_case_insensitive_ascii_avx2(const void* a, size_t length_a, const void* b,
                                                                                      size_t length_b, bool* out_equal)
{
    if (length_a != length_b) {
        *out_equal = false;
        return true;
    }
    if (length_a == 0) {
        *out_equal = true;
        return true;
    }

    const unsigned char* pointer_a = (const unsigned char*)a;
    const unsigned char* pointer_b = (const unsigned char*)b;
    const __m256i ones = _mm256_set1_epi8((char)0xFF);
    size_t offset = 0;

    while (offset + 128 <= length_a) {
        __m256i a0 = _mm256_loadu_si256((const __m256i*)(pointer_a + offset));
        __m256i a1 = _mm256_loadu_si256((const __m256i*)(pointer_a + offset + 32));
        __m256i a2 = _mm256_loadu_si256((const __m256i*)(pointer_a + offset + 64));
        __m256i a3 = _mm256_loadu_si256((const __m256i*)(pointer_a + offset + 96));
        __m256i b0 = _mm256_loadu_si256((const __m256i*)(pointer_b + offset));
        __m256i b1 = _mm256_loadu_si256((const __m256i*)(pointer_b + offset + 32));
        __m256i b2 = _mm256_loadu_si256((const __m256i*)(pointer_b + offset + 64));
        __m256i b3 = _mm256_loadu_si256((const __m256i*)(pointer_b + offset + 96));

        __m256i eq0 = _mm256_cmpeq_epi8(lower_ascii_letters_avx2(a0), lower_ascii_letters_avx2(b0));
        __m256i eq1 = _mm256_cmpeq_epi8(lower_ascii_letters_avx2(a1), lower_ascii_letters_avx2(b1));
        __m256i eq2 = _mm256_cmpeq_epi8(lower_ascii_letters_avx2(a2), lower_ascii_letters_avx2(b2));
        __m256i eq3 = _mm256_cmpeq_epi8(lower_ascii_letters_avx2(a3), lower_ascii_letters_avx2(b3));

        __m256i combined = _mm256_and_si256(_mm256_and_si256(eq0, eq1), _mm256_and_si256(eq2, eq3));
        if (!_mm256_testc_si256(combined, ones)) {
            *out_equal = false;
            return true;
        }
        offset += 128;
    }

    while (offset + 32 <= length_a) {
        __m256i chunk_a = _mm256_loadu_si256((const __m256i*)(pointer_a + offset));
        __m256i chunk_b = _mm256_loadu_si256((const __m256i*)(pointer_b + offset));
        __m256i comparison = _mm256_cmpeq_epi8(lower_ascii_letters_avx2(chunk_a), lower_ascii_letters_avx2(chunk_b));
        unsigned int movemask = (unsigned int)_mm256_movemask_epi8(comparison);
        if (movemask != 0xFFFFFFFFu) {
            *out_equal = false;
            return true;
        }
        offset += 32;
    }

    while (offset < length_a) {
        if (to_lower_ascii_byte(pointer_a[offset]) != to_lower_ascii_byte(pointer_b[offset])) {
            *out_equal = false;
            return true;
        }
        offset += 1;
    }

    *out_equal = true;
    return true;
}

static bool (*g_equal_case_insensitive_ascii_impl)(const void*, size_t, const void*, size_t,
                                                   bool*) = bc_core_equal_case_insensitive_ascii_avx2;

__attribute__((constructor)) void bc_core_equal_case_insensitive_ascii_dispatch_init(void)
{
    g_equal_case_insensitive_ascii_impl = bc_core_equal_case_insensitive_ascii_avx2;
}

bool bc_core_equal_case_insensitive_ascii(const void* a, size_t length_a, const void* b, size_t length_b, bool* out_equal)
{
    return g_equal_case_insensitive_ascii_impl(a, length_a, b, length_b, out_equal);
}
