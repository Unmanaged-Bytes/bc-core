// SPDX-License-Identifier: MIT

#include "bc_core.h"

#include <immintrin.h>
#include <stdint.h>

static void bc_core_ascii_lowercase_scalar(uint8_t* data, size_t len)
{
    for (size_t offset = 0; offset < len; ++offset) {
        uint8_t value = data[offset];
        if (value >= 'A' && value <= 'Z') {
            data[offset] = (uint8_t)(value + ('a' - 'A'));
        }
    }
}

__attribute__((target("sse2"))) static void bc_core_ascii_lowercase_sse2(uint8_t* data, size_t len)
{
    size_t offset = 0;
    const __m128i upper_threshold_low = _mm_set1_epi8('A' - 1);
    const __m128i upper_threshold_high = _mm_set1_epi8('Z' + 1);
    const __m128i case_delta = _mm_set1_epi8(0x20);
    while (offset + 16u <= len) {
        __m128i bytes = _mm_loadu_si128((const __m128i*)(data + offset));
        __m128i greater_than_min = _mm_cmpgt_epi8(bytes, upper_threshold_low);
        __m128i less_than_max = _mm_cmpgt_epi8(upper_threshold_high, bytes);
        __m128i is_upper = _mm_and_si128(greater_than_min, less_than_max);
        __m128i conditional_delta = _mm_and_si128(is_upper, case_delta);
        __m128i lowered = _mm_add_epi8(bytes, conditional_delta);
        _mm_storeu_si128((__m128i*)(data + offset), lowered);
        offset += 16u;
    }
    bc_core_ascii_lowercase_scalar(data + offset, len - offset);
}

__attribute__((target("avx2"))) static void bc_core_ascii_lowercase_avx2(uint8_t* data, size_t len)
{
    size_t offset = 0;
    const __m256i upper_threshold_low = _mm256_set1_epi8('A' - 1);
    const __m256i upper_threshold_high = _mm256_set1_epi8('Z' + 1);
    const __m256i case_delta = _mm256_set1_epi8(0x20);
    while (offset + 32u <= len) {
        __m256i bytes = _mm256_loadu_si256((const __m256i*)(data + offset));
        __m256i greater_than_min = _mm256_cmpgt_epi8(bytes, upper_threshold_low);
        __m256i less_than_max = _mm256_cmpgt_epi8(upper_threshold_high, bytes);
        __m256i is_upper = _mm256_and_si256(greater_than_min, less_than_max);
        __m256i conditional_delta = _mm256_and_si256(is_upper, case_delta);
        __m256i lowered = _mm256_add_epi8(bytes, conditional_delta);
        _mm256_storeu_si256((__m256i*)(data + offset), lowered);
        offset += 32u;
    }
    if (offset < len) {
        bc_core_ascii_lowercase_sse2(data + offset, len - offset);
    }
}

static void (*g_ascii_lowercase_impl)(uint8_t*, size_t) = bc_core_ascii_lowercase_avx2;

__attribute__((constructor)) void bc_core_ascii_dispatch_init(void)
{
    g_ascii_lowercase_impl = bc_core_ascii_lowercase_avx2;
}

bool bc_core_ascii_lowercase(void* data, size_t len)
{
    if (len == 0) {
        return true;
    }
    g_ascii_lowercase_impl((uint8_t*)data, len);
    return true;
}
