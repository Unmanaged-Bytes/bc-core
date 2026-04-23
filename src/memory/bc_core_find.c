// SPDX-License-Identifier: MIT

#include "bc_core.h"

#include "bc_core_cpu_features_internal.h"
#include "bc_core_pattern_internal.h"

#include <immintrin.h>
#include <stdint.h>

static bool verify_pattern_candidate(const unsigned char* haystack, const unsigned char* needle, size_t pattern_length,
                                     size_t candidate_offset, size_t* out_offset)
{
    if (pattern_length == 2 || bc_core_pattern_matches(haystack + candidate_offset + 1, needle + 1, pattern_length - 2)) {
        *out_offset = candidate_offset;
        return true;
    }
    return false;
}

__attribute__((target("avx2"))) static bool bc_core_find_byte_avx2(const void* data, size_t len, unsigned char target, size_t* out_offset)
{
    if (len == 0) {
        return false;
    }

    const unsigned char* bytes = (const unsigned char*)data;
    const unsigned char* ptr = bytes;
    const unsigned char* end = bytes + len;
    __m256i target_vec = _mm256_set1_epi8((char)target);

    while (ptr + 128 <= end) {
        __m256i v0 = _mm256_cmpeq_epi8(_mm256_loadu_si256((const __m256i*)(ptr + 0)), target_vec);
        __m256i v1 = _mm256_cmpeq_epi8(_mm256_loadu_si256((const __m256i*)(ptr + 32)), target_vec);
        __m256i v2 = _mm256_cmpeq_epi8(_mm256_loadu_si256((const __m256i*)(ptr + 64)), target_vec);
        __m256i v3 = _mm256_cmpeq_epi8(_mm256_loadu_si256((const __m256i*)(ptr + 96)), target_vec);
        __m256i any = _mm256_or_si256(_mm256_or_si256(v0, v1), _mm256_or_si256(v2, v3));

        if (_mm256_movemask_epi8(any) != 0) {
            int m;
            m = _mm256_movemask_epi8(v0);
            if (m) {
                *out_offset = (size_t)(ptr - bytes) + 0 + (size_t)__builtin_ctz((unsigned int)m);
                return true;
            }
            m = _mm256_movemask_epi8(v1);
            if (m) {
                *out_offset = (size_t)(ptr - bytes) + 32 + (size_t)__builtin_ctz((unsigned int)m);
                return true;
            }
            m = _mm256_movemask_epi8(v2);
            if (m) {
                *out_offset = (size_t)(ptr - bytes) + 64 + (size_t)__builtin_ctz((unsigned int)m);
                return true;
            }
            m = _mm256_movemask_epi8(v3);
            *out_offset = (size_t)(ptr - bytes) + 96 + (size_t)__builtin_ctz((unsigned int)m);
            return true;
        }
        ptr += 128;
    }

    while (ptr + 32 <= end) {
        __m256i v = _mm256_cmpeq_epi8(_mm256_loadu_si256((const __m256i*)ptr), target_vec);
        int mask = _mm256_movemask_epi8(v);
        if (mask != 0) {
            *out_offset = (size_t)(ptr - bytes) + (size_t)__builtin_ctz((unsigned int)mask);
            return true;
        }
        ptr += 32;
    }

    while (ptr < end) {
        if (*ptr == target) {
            *out_offset = (size_t)(ptr - bytes);
            return true;
        }
        ptr++;
    }

    return false;
}

__attribute__((target("avx2"))) static bool bc_core_find_pattern_avx2(const void* data, size_t len, const void* pattern, size_t pattern_len,
                                                                      size_t* out_offset)
{
    if (pattern_len == 0 || len == 0) {
        return false;
    }
    if (pattern_len == 1) {
        return bc_core_find_byte(data, len, *(const unsigned char*)pattern, out_offset);
    }
    if (pattern_len > len) {
        return false;
    }

    const unsigned char* hay = (const unsigned char*)data;
    const unsigned char* needle = (const unsigned char*)pattern;
    unsigned char first = needle[0];
    unsigned char last = needle[pattern_len - 1];
    __m256i first_vec = _mm256_set1_epi8((char)first);
    __m256i last_vec = _mm256_set1_epi8((char)last);
    size_t simd_end = len - (pattern_len - 1);
    size_t offset = 0;

    while (offset + 128 <= simd_end) {
        __m256i m0 =
            _mm256_and_si256(_mm256_cmpeq_epi8(_mm256_loadu_si256((const __m256i*)(hay + offset + 0)), first_vec),
                             _mm256_cmpeq_epi8(_mm256_loadu_si256((const __m256i*)(hay + offset + 0 + pattern_len - 1)), last_vec));
        __m256i m1 =
            _mm256_and_si256(_mm256_cmpeq_epi8(_mm256_loadu_si256((const __m256i*)(hay + offset + 32)), first_vec),
                             _mm256_cmpeq_epi8(_mm256_loadu_si256((const __m256i*)(hay + offset + 32 + pattern_len - 1)), last_vec));
        __m256i m2 =
            _mm256_and_si256(_mm256_cmpeq_epi8(_mm256_loadu_si256((const __m256i*)(hay + offset + 64)), first_vec),
                             _mm256_cmpeq_epi8(_mm256_loadu_si256((const __m256i*)(hay + offset + 64 + pattern_len - 1)), last_vec));
        __m256i m3 =
            _mm256_and_si256(_mm256_cmpeq_epi8(_mm256_loadu_si256((const __m256i*)(hay + offset + 96)), first_vec),
                             _mm256_cmpeq_epi8(_mm256_loadu_si256((const __m256i*)(hay + offset + 96 + pattern_len - 1)), last_vec));
        __m256i any = _mm256_or_si256(_mm256_or_si256(m0, m1), _mm256_or_si256(m2, m3));

        if (_mm256_movemask_epi8(any) != 0) {
            unsigned int mask;
            mask = (unsigned int)_mm256_movemask_epi8(m0);
            while (mask != 0) {
                int bit = __builtin_ctz(mask);
                size_t candidate = offset + 0 + (size_t)bit;
                if (verify_pattern_candidate(hay, needle, pattern_len, candidate, out_offset)) {
                    return true;
                }
                mask &= mask - 1;
            }
            mask = (unsigned int)_mm256_movemask_epi8(m1);
            while (mask != 0) {
                int bit = __builtin_ctz(mask);
                size_t candidate = offset + 32 + (size_t)bit;
                if (verify_pattern_candidate(hay, needle, pattern_len, candidate, out_offset)) {
                    return true;
                }
                mask &= mask - 1;
            }
            mask = (unsigned int)_mm256_movemask_epi8(m2);
            while (mask != 0) {
                int bit = __builtin_ctz(mask);
                size_t candidate = offset + 64 + (size_t)bit;
                if (verify_pattern_candidate(hay, needle, pattern_len, candidate, out_offset)) {
                    return true;
                }
                mask &= mask - 1;
            }
            mask = (unsigned int)_mm256_movemask_epi8(m3);
            while (mask != 0) {
                int bit = __builtin_ctz(mask);
                size_t candidate = offset + 96 + (size_t)bit;
                if (verify_pattern_candidate(hay, needle, pattern_len, candidate, out_offset)) {
                    return true;
                }
                mask &= mask - 1;
            }
        }
        offset += 128;
    }

    while (offset + 32 <= simd_end) {
        __m256i fc = _mm256_loadu_si256((const __m256i*)(hay + offset));
        __m256i lc = _mm256_loadu_si256((const __m256i*)(hay + offset + pattern_len - 1));
        unsigned int mask =
            (unsigned int)_mm256_movemask_epi8(_mm256_and_si256(_mm256_cmpeq_epi8(fc, first_vec), _mm256_cmpeq_epi8(lc, last_vec)));

        while (mask != 0) {
            int bit = __builtin_ctz(mask);
            size_t candidate = offset + (size_t)bit;
            if (verify_pattern_candidate(hay, needle, pattern_len, candidate, out_offset)) {
                return true;
            }
            mask &= mask - 1;
        }
        offset += 32;
    }

    while (offset + pattern_len <= len) {
        if (hay[offset] == first && hay[offset + pattern_len - 1] == last &&
            verify_pattern_candidate(hay, needle, pattern_len, offset, out_offset)) {
            return true;
        }
        offset++;
    }

    return false;
}

__attribute__((target("avx2"))) static bool bc_core_find_last_byte_avx2(const void* data, size_t len, unsigned char target,
                                                                        size_t* out_offset)
{
    if (len == 0) {
        return false;
    }

    const unsigned char* bytes = (const unsigned char*)data;
    __m256i target_vec = _mm256_set1_epi8((char)target);

    size_t tail_length = len % 32;
    size_t aligned_end = len - tail_length;

    if (tail_length > 0) {
        for (size_t i = len; i > aligned_end; i--) {
            if (bytes[i - 1] == target) {
                *out_offset = i - 1;
                return true;
            }
        }
    }

    size_t position = aligned_end;
    while (position >= 128) {
        position -= 128;
        __m256i v3 = _mm256_cmpeq_epi8(_mm256_loadu_si256((const __m256i*)(bytes + position + 96)), target_vec);
        __m256i v2 = _mm256_cmpeq_epi8(_mm256_loadu_si256((const __m256i*)(bytes + position + 64)), target_vec);
        __m256i v1 = _mm256_cmpeq_epi8(_mm256_loadu_si256((const __m256i*)(bytes + position + 32)), target_vec);
        __m256i v0 = _mm256_cmpeq_epi8(_mm256_loadu_si256((const __m256i*)(bytes + position + 0)), target_vec);
        __m256i any = _mm256_or_si256(_mm256_or_si256(v0, v1), _mm256_or_si256(v2, v3));

        if (_mm256_movemask_epi8(any) != 0) {
            int m;
            m = _mm256_movemask_epi8(v3);
            if (m) {
                *out_offset = position + 96 + (size_t)(31 - __builtin_clz((unsigned int)m));
                return true;
            }
            m = _mm256_movemask_epi8(v2);
            if (m) {
                *out_offset = position + 64 + (size_t)(31 - __builtin_clz((unsigned int)m));
                return true;
            }
            m = _mm256_movemask_epi8(v1);
            if (m) {
                *out_offset = position + 32 + (size_t)(31 - __builtin_clz((unsigned int)m));
                return true;
            }
            m = _mm256_movemask_epi8(v0);
            *out_offset = position + 0 + (size_t)(31 - __builtin_clz((unsigned int)m));
            return true;
        }
    }

    while (position >= 32) {
        position -= 32;
        __m256i v = _mm256_cmpeq_epi8(_mm256_loadu_si256((const __m256i*)(bytes + position)), target_vec);
        int mask = _mm256_movemask_epi8(v);
        if (mask != 0) {
            *out_offset = position + (size_t)(31 - __builtin_clz((unsigned int)mask));
            return true;
        }
    }

    return false;
}

__attribute__((target("avx2"))) static bool bc_core_find_any_byte_avx2(const void* data, size_t len, const unsigned char* targets,
                                                                       size_t target_count, size_t* out_offset)
{
    if (len == 0 || target_count == 0) {
        return false;
    }

    const unsigned char* bytes = (const unsigned char*)data;

    unsigned char lo_lut_bytes_pass1[16] __attribute__((aligned(16)));
    unsigned char hi_lut_bytes_pass1[16] __attribute__((aligned(16)));
    unsigned char lo_lut_bytes_pass2[16] __attribute__((aligned(16)));
    unsigned char hi_lut_bytes_pass2[16] __attribute__((aligned(16)));

    for (int i = 0; i < 16; i++) {
        lo_lut_bytes_pass1[i] = 0;
        hi_lut_bytes_pass1[i] = 0;
        lo_lut_bytes_pass2[i] = 0;
        hi_lut_bytes_pass2[i] = 0;
    }

    for (size_t t = 0; t < target_count; t++) {
        unsigned char byte_value = targets[t];
        int low_nibble = byte_value & 0x0F;
        int high_nibble = (byte_value >> 4) & 0x0F;
        if (high_nibble < 8) {
            lo_lut_bytes_pass1[low_nibble] |= (unsigned char)(1 << high_nibble);
        } else {
            lo_lut_bytes_pass2[low_nibble] |= (unsigned char)(1 << (high_nibble - 8));
        }
    }

    for (int high_nibble = 0; high_nibble < 16; high_nibble++) {
        hi_lut_bytes_pass1[high_nibble] = (unsigned char)((high_nibble < 8) ? (1 << high_nibble) : 0);
        hi_lut_bytes_pass2[high_nibble] = (unsigned char)((high_nibble >= 8) ? (1 << (high_nibble - 8)) : 0);
    }

    __m256i lo_lut1 = _mm256_broadcastsi128_si256(_mm_loadu_si128((const __m128i*)lo_lut_bytes_pass1));
    __m256i hi_lut1 = _mm256_broadcastsi128_si256(_mm_loadu_si128((const __m128i*)hi_lut_bytes_pass1));
    __m256i lo_lut2 = _mm256_broadcastsi128_si256(_mm_loadu_si128((const __m128i*)lo_lut_bytes_pass2));
    __m256i hi_lut2 = _mm256_broadcastsi128_si256(_mm_loadu_si128((const __m128i*)hi_lut_bytes_pass2));
    __m256i nibble_mask = _mm256_set1_epi8(0x0F);
    __m256i zero = _mm256_setzero_si256();

    const unsigned char* ptr = bytes;
    const unsigned char* end = bytes + len;

    while (ptr + 32 <= end) {
        __m256i data_vec = _mm256_loadu_si256((const __m256i*)ptr);
        __m256i low_nibbles = _mm256_and_si256(data_vec, nibble_mask);
        __m256i high_nibbles = _mm256_and_si256(_mm256_srli_epi16(data_vec, 4), nibble_mask);
        __m256i match1 = _mm256_and_si256(_mm256_shuffle_epi8(lo_lut1, low_nibbles), _mm256_shuffle_epi8(hi_lut1, high_nibbles));
        __m256i match2 = _mm256_and_si256(_mm256_shuffle_epi8(lo_lut2, low_nibbles), _mm256_shuffle_epi8(hi_lut2, high_nibbles));
        __m256i combined = _mm256_or_si256(match1, match2);
        __m256i is_nonzero = _mm256_cmpeq_epi8(combined, zero);
        int mask = ~_mm256_movemask_epi8(is_nonzero);
        if (mask != 0) {
            *out_offset = (size_t)(ptr - bytes) + (size_t)__builtin_ctz((unsigned int)mask);
            return true;
        }
        ptr += 32;
    }

    while (ptr < end) {
        unsigned char byte_value = *ptr;
        for (size_t t = 0; t < target_count; t++) {
            if (byte_value == targets[t]) {
                *out_offset = (size_t)(ptr - bytes);
                return true;
            }
        }
        ptr++;
    }

    return false;
}

static bool (*g_find_byte_impl)(const void*, size_t, unsigned char, size_t*) = bc_core_find_byte_avx2;
static bool (*g_find_pattern_impl)(const void*, size_t, const void*, size_t, size_t*) = bc_core_find_pattern_avx2;
static bool (*g_find_last_byte_impl)(const void*, size_t, unsigned char, size_t*) = bc_core_find_last_byte_avx2;
static bool (*g_find_any_byte_impl)(const void*, size_t, const unsigned char*, size_t, size_t*) = bc_core_find_any_byte_avx2;

__attribute__((constructor)) void bc_core_find_dispatch_init(void)
{
    g_find_byte_impl = bc_core_find_byte_avx2;
    g_find_pattern_impl = bc_core_find_pattern_avx2;
    g_find_last_byte_impl = bc_core_find_last_byte_avx2;
    g_find_any_byte_impl = bc_core_find_any_byte_avx2;
}

bool bc_core_find_byte(const void* data, size_t len, unsigned char target, size_t* out_offset)
{
    return g_find_byte_impl(data, len, target, out_offset);
}

bool bc_core_find_pattern(const void* data, size_t len, const void* pattern, size_t pattern_len, size_t* out_offset)
{
    return g_find_pattern_impl(data, len, pattern, pattern_len, out_offset);
}

bool bc_core_find_last_byte(const void* data, size_t len, unsigned char target, size_t* out_offset)
{
    return g_find_last_byte_impl(data, len, target, out_offset);
}

bool bc_core_find_any_byte(const void* data, size_t len, const unsigned char* targets, size_t target_count, size_t* out_offset)
{
    return g_find_any_byte_impl(data, len, targets, target_count, out_offset);
}

bool bc_core_byte_mask_prepare(const unsigned char* targets, size_t target_count, bc_core_byte_mask_t* out_mask)
{
    for (int i = 0; i < 16; i++) {
        out_mask->lut_lo_pass1[i] = 0;
        out_mask->lut_lo_pass2[i] = 0;
        out_mask->lut_hi_pass1[i] = (unsigned char)((i < 8) ? (1 << i) : 0);
        out_mask->lut_hi_pass2[i] = (unsigned char)((i >= 8) ? (1 << (i - 8)) : 0);
    }
    for (size_t t = 0; t < target_count; t++) {
        unsigned char byte_value = targets[t];
        int low_nibble = byte_value & 0x0F;
        int high_nibble = (byte_value >> 4) & 0x0F;
        if (high_nibble < 8) {
            out_mask->lut_lo_pass1[low_nibble] |= (unsigned char)(1 << high_nibble);
        } else {
            out_mask->lut_lo_pass2[low_nibble] |= (unsigned char)(1 << (high_nibble - 8));
        }
    }
    return true;
}

bool bc_core_byte_mask_prepare_predicate(bool (*predicate)(unsigned char byte, void* user_data), void* user_data,
                                         bc_core_byte_mask_t* out_mask)
{
    unsigned char targets[256];
    size_t count = 0;
    for (int byte_value = 0; byte_value < 256; byte_value++) {
        if (predicate((unsigned char)byte_value, user_data)) {
            targets[count++] = (unsigned char)byte_value;
        }
    }
    return bc_core_byte_mask_prepare(targets, count, out_mask);
}

__attribute__((target("avx2"))) static bool bc_core_find_byte_in_mask_avx2(const void* data, size_t len,
                                                                            const bc_core_byte_mask_t* mask, size_t* out_offset)
{
    if (len == 0) {
        return false;
    }

    const unsigned char* bytes = (const unsigned char*)data;
    __m256i lo_lut1 = _mm256_broadcastsi128_si256(_mm_loadu_si128((const __m128i*)mask->lut_lo_pass1));
    __m256i hi_lut1 = _mm256_broadcastsi128_si256(_mm_loadu_si128((const __m128i*)mask->lut_hi_pass1));
    __m256i lo_lut2 = _mm256_broadcastsi128_si256(_mm_loadu_si128((const __m128i*)mask->lut_lo_pass2));
    __m256i hi_lut2 = _mm256_broadcastsi128_si256(_mm_loadu_si128((const __m128i*)mask->lut_hi_pass2));
    __m256i nibble_mask = _mm256_set1_epi8(0x0F);
    __m256i zero = _mm256_setzero_si256();

    const unsigned char* ptr = bytes;
    const unsigned char* end = bytes + len;

    while (ptr + 32 <= end) {
        __m256i data_vec = _mm256_loadu_si256((const __m256i*)ptr);
        __m256i low_nibbles = _mm256_and_si256(data_vec, nibble_mask);
        __m256i high_nibbles = _mm256_and_si256(_mm256_srli_epi16(data_vec, 4), nibble_mask);
        __m256i match1 = _mm256_and_si256(_mm256_shuffle_epi8(lo_lut1, low_nibbles), _mm256_shuffle_epi8(hi_lut1, high_nibbles));
        __m256i match2 = _mm256_and_si256(_mm256_shuffle_epi8(lo_lut2, low_nibbles), _mm256_shuffle_epi8(hi_lut2, high_nibbles));
        __m256i combined = _mm256_or_si256(match1, match2);
        __m256i is_nonzero = _mm256_cmpeq_epi8(combined, zero);
        int match_mask = ~_mm256_movemask_epi8(is_nonzero);
        if (match_mask != 0) {
            *out_offset = (size_t)(ptr - bytes) + (size_t)__builtin_ctz((unsigned int)match_mask);
            return true;
        }
        ptr += 32;
    }

    while (ptr < end) {
        unsigned char byte_value = *ptr;
        int low_nibble = byte_value & 0x0F;
        int high_nibble = (byte_value >> 4) & 0x0F;
        unsigned char combined = 0;
        if (high_nibble < 8) {
            combined = (unsigned char)(mask->lut_lo_pass1[low_nibble] & (1 << high_nibble));
        } else {
            combined = (unsigned char)(mask->lut_lo_pass2[low_nibble] & (1 << (high_nibble - 8)));
        }
        if (combined != 0) {
            *out_offset = (size_t)(ptr - bytes);
            return true;
        }
        ptr++;
    }

    return false;
}

__attribute__((target("avx2"))) static bool bc_core_find_byte_not_in_mask_avx2(const void* data, size_t len,
                                                                                const bc_core_byte_mask_t* mask, size_t* out_offset)
{
    if (len == 0) {
        return false;
    }

    const unsigned char* bytes = (const unsigned char*)data;
    __m256i lo_lut1 = _mm256_broadcastsi128_si256(_mm_loadu_si128((const __m128i*)mask->lut_lo_pass1));
    __m256i hi_lut1 = _mm256_broadcastsi128_si256(_mm_loadu_si128((const __m128i*)mask->lut_hi_pass1));
    __m256i lo_lut2 = _mm256_broadcastsi128_si256(_mm_loadu_si128((const __m128i*)mask->lut_lo_pass2));
    __m256i hi_lut2 = _mm256_broadcastsi128_si256(_mm_loadu_si128((const __m128i*)mask->lut_hi_pass2));
    __m256i nibble_mask = _mm256_set1_epi8(0x0F);
    __m256i zero = _mm256_setzero_si256();

    const unsigned char* ptr = bytes;
    const unsigned char* end = bytes + len;

    while (ptr + 32 <= end) {
        __m256i data_vec = _mm256_loadu_si256((const __m256i*)ptr);
        __m256i low_nibbles = _mm256_and_si256(data_vec, nibble_mask);
        __m256i high_nibbles = _mm256_and_si256(_mm256_srli_epi16(data_vec, 4), nibble_mask);
        __m256i match1 = _mm256_and_si256(_mm256_shuffle_epi8(lo_lut1, low_nibbles), _mm256_shuffle_epi8(hi_lut1, high_nibbles));
        __m256i match2 = _mm256_and_si256(_mm256_shuffle_epi8(lo_lut2, low_nibbles), _mm256_shuffle_epi8(hi_lut2, high_nibbles));
        __m256i combined = _mm256_or_si256(match1, match2);
        __m256i is_zero = _mm256_cmpeq_epi8(combined, zero);
        int miss_mask = _mm256_movemask_epi8(is_zero);
        if (miss_mask != 0) {
            *out_offset = (size_t)(ptr - bytes) + (size_t)__builtin_ctz((unsigned int)miss_mask);
            return true;
        }
        ptr += 32;
    }

    while (ptr < end) {
        unsigned char byte_value = *ptr;
        int low_nibble = byte_value & 0x0F;
        int high_nibble = (byte_value >> 4) & 0x0F;
        unsigned char combined = 0;
        if (high_nibble < 8) {
            combined = (unsigned char)(mask->lut_lo_pass1[low_nibble] & (1 << high_nibble));
        } else {
            combined = (unsigned char)(mask->lut_lo_pass2[low_nibble] & (1 << (high_nibble - 8)));
        }
        if (combined == 0) {
            *out_offset = (size_t)(ptr - bytes);
            return true;
        }
        ptr++;
    }

    return false;
}

bool bc_core_find_byte_in_mask(const void* data, size_t len, const bc_core_byte_mask_t* mask, size_t* out_offset)
{
    return bc_core_find_byte_in_mask_avx2(data, len, mask, out_offset);
}

bool bc_core_find_byte_not_in_mask(const void* data, size_t len, const bc_core_byte_mask_t* mask, size_t* out_offset)
{
    return bc_core_find_byte_not_in_mask_avx2(data, len, mask, out_offset);
}
