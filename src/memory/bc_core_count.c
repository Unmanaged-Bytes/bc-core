// SPDX-License-Identifier: MIT

#include "bc_core.h"

#include "bc_core_cpu_features_internal.h"
#include "bc_core_pattern_internal.h"

#include <immintrin.h>
#include <stdint.h>

__attribute__((target("avx512f,avx512bw"))) static bool bc_core_count_byte_avx512(const void* data, size_t len, unsigned char target,
                                                                                  size_t* out_count)
{
    if (len == 0) {
        *out_count = 0;
        return true;
    }

    const unsigned char* bytes = (const unsigned char*)data;
    const unsigned char* ptr = bytes;
    const unsigned char* end = bytes + len;
    size_t count = 0;
    __m512i target_vec = _mm512_set1_epi8((char)target);

    while (ptr + 256 <= end) {
        __m512i c0 = _mm512_loadu_si512((const __m512i*)(ptr + 0));
        __m512i c1 = _mm512_loadu_si512((const __m512i*)(ptr + 64));
        __m512i c2 = _mm512_loadu_si512((const __m512i*)(ptr + 128));
        __m512i c3 = _mm512_loadu_si512((const __m512i*)(ptr + 192));
        __mmask64 m0 = _mm512_cmpeq_epi8_mask(c0, target_vec);
        __mmask64 m1 = _mm512_cmpeq_epi8_mask(c1, target_vec);
        __mmask64 m2 = _mm512_cmpeq_epi8_mask(c2, target_vec);
        __mmask64 m3 = _mm512_cmpeq_epi8_mask(c3, target_vec);
        count += (size_t)__builtin_popcountll(m0);
        count += (size_t)__builtin_popcountll(m1);
        count += (size_t)__builtin_popcountll(m2);
        count += (size_t)__builtin_popcountll(m3);
        ptr += 256;
    }

    while (ptr + 64 <= end) {
        __m512i v = _mm512_loadu_si512((const __m512i*)ptr);
        __mmask64 mask = _mm512_cmpeq_epi8_mask(v, target_vec);
        count += (size_t)__builtin_popcountll(mask);
        ptr += 64;
    }

    if (ptr < end) {
        size_t tail = (size_t)(end - ptr);
        __mmask64 load_mask = (1ULL << tail) - 1ULL;
        __m512i v = _mm512_maskz_loadu_epi8(load_mask, ptr);
        __mmask64 mask = _mm512_mask_cmpeq_epi8_mask(load_mask, v, target_vec);
        count += (size_t)__builtin_popcountll(mask);
    }

    *out_count = count;
    return true;
}

__attribute__((target("avx2"))) static bool bc_core_count_byte_avx2(const void* data, size_t len, unsigned char target, size_t* out_count)
{
    if (len == 0) {
        *out_count = 0;
        return true;
    }

    const unsigned char* bytes = (const unsigned char*)data;
    const unsigned char* ptr = bytes;
    const unsigned char* end = bytes + len;
    size_t count = 0;
    __m256i target_vec = _mm256_set1_epi8((char)target);
    __m256i zero = _mm256_setzero_si256();

    while (ptr + 128 <= end) {
        __m256i accumulator0 = _mm256_setzero_si256();
        __m256i accumulator1 = _mm256_setzero_si256();
        __m256i accumulator2 = _mm256_setzero_si256();
        __m256i accumulator3 = _mm256_setzero_si256();
        size_t remaining_bytes = (size_t)(end - ptr);
        size_t batch_iterations = remaining_bytes / 128;
        if (batch_iterations > 255) { /* GCOVR_EXCL_BR_LINE -- epi8 accumulator overflow guard */
            batch_iterations = 255;   /* GCOVR_EXCL_LINE -- epi8 accumulator overflow guard */
        }

        for (size_t iteration = 0; iteration < batch_iterations; iteration++) {
            __m256i v0 = _mm256_cmpeq_epi8(_mm256_loadu_si256((const __m256i*)(ptr + 0)), target_vec);
            __m256i v1 = _mm256_cmpeq_epi8(_mm256_loadu_si256((const __m256i*)(ptr + 32)), target_vec);
            __m256i v2 = _mm256_cmpeq_epi8(_mm256_loadu_si256((const __m256i*)(ptr + 64)), target_vec);
            __m256i v3 = _mm256_cmpeq_epi8(_mm256_loadu_si256((const __m256i*)(ptr + 96)), target_vec);
            accumulator0 = _mm256_sub_epi8(accumulator0, v0);
            accumulator1 = _mm256_sub_epi8(accumulator1, v1);
            accumulator2 = _mm256_sub_epi8(accumulator2, v2);
            accumulator3 = _mm256_sub_epi8(accumulator3, v3);
            ptr += 128;
        }

        __m256i sad0 = _mm256_sad_epu8(accumulator0, zero);
        __m256i sad1 = _mm256_sad_epu8(accumulator1, zero);
        __m256i sad2 = _mm256_sad_epu8(accumulator2, zero);
        __m256i sad3 = _mm256_sad_epu8(accumulator3, zero);
        __m256i sad_result = _mm256_add_epi64(_mm256_add_epi64(sad0, sad1), _mm256_add_epi64(sad2, sad3));
        count += (size_t)_mm256_extract_epi64(sad_result, 0);
        count += (size_t)_mm256_extract_epi64(sad_result, 1);
        count += (size_t)_mm256_extract_epi64(sad_result, 2);
        count += (size_t)_mm256_extract_epi64(sad_result, 3);
    }

    while (ptr + 32 <= end) {
        __m256i v = _mm256_cmpeq_epi8(_mm256_loadu_si256((const __m256i*)ptr), target_vec);
        count += (size_t)__builtin_popcount((unsigned int)_mm256_movemask_epi8(v));
        ptr += 32;
    }

    while (ptr < end) {
        if (*ptr == target) {
            count++;
        }
        ptr++;
    }

    *out_count = count;
    return true;
}

__attribute__((target("avx2"))) static bool bc_core_count_matching_avx2(const void* data, size_t len, const unsigned char table[256],
                                                                        size_t* out_count)
{
    if (len == 0) {
        *out_count = 0;
        return true;
    }

    const unsigned char* ptr = (const unsigned char*)data;
    const unsigned char* end = ptr + len;
    size_t count = 0;

    unsigned char lo_lut_bytes_pass1[16];
    unsigned char hi_lut_bytes_pass1[16];
    unsigned char lo_lut_bytes_pass2[16];
    unsigned char hi_lut_bytes_pass2[16];

    for (int low_nibble = 0; low_nibble < 16; low_nibble++) {
        unsigned char mask_pass1 = 0;
        unsigned char mask_pass2 = 0;
        for (int high_nibble = 0; high_nibble < 8; high_nibble++) {
            if (table[high_nibble * 16 + low_nibble]) {
                mask_pass1 |= (unsigned char)(1 << high_nibble);
            }
        }
        for (int high_nibble = 8; high_nibble < 16; high_nibble++) {
            if (table[high_nibble * 16 + low_nibble]) {
                mask_pass2 |= (unsigned char)(1 << (high_nibble - 8));
            }
        }
        lo_lut_bytes_pass1[low_nibble] = mask_pass1;
        lo_lut_bytes_pass2[low_nibble] = mask_pass2;
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

    while (ptr + 128 <= end) {
        for (int chunk = 0; chunk < 4; chunk++) {
            __m256i data_vec = _mm256_loadu_si256((const __m256i*)(ptr + chunk * 32));
            __m256i low_nibbles = _mm256_and_si256(data_vec, nibble_mask);
            __m256i high_nibbles = _mm256_and_si256(_mm256_srli_epi16(data_vec, 4), nibble_mask);
            __m256i match1 = _mm256_and_si256(_mm256_shuffle_epi8(lo_lut1, low_nibbles), _mm256_shuffle_epi8(hi_lut1, high_nibbles));
            __m256i match2 = _mm256_and_si256(_mm256_shuffle_epi8(lo_lut2, low_nibbles), _mm256_shuffle_epi8(hi_lut2, high_nibbles));
            __m256i combined = _mm256_or_si256(match1, match2);
            __m256i is_nonzero = _mm256_cmpeq_epi8(combined, zero);
            int mask = ~_mm256_movemask_epi8(is_nonzero);
            count += (size_t)__builtin_popcount((unsigned int)mask);
        }
        ptr += 128;
    }

    while (ptr + 32 <= end) {
        __m256i data_vec = _mm256_loadu_si256((const __m256i*)ptr);
        __m256i low_nibbles = _mm256_and_si256(data_vec, nibble_mask);
        __m256i high_nibbles = _mm256_and_si256(_mm256_srli_epi16(data_vec, 4), nibble_mask);
        __m256i match1 = _mm256_and_si256(_mm256_shuffle_epi8(lo_lut1, low_nibbles), _mm256_shuffle_epi8(hi_lut1, high_nibbles));
        __m256i match2 = _mm256_and_si256(_mm256_shuffle_epi8(lo_lut2, low_nibbles), _mm256_shuffle_epi8(hi_lut2, high_nibbles));
        __m256i combined = _mm256_or_si256(match1, match2);
        __m256i is_nonzero = _mm256_cmpeq_epi8(combined, zero);
        int mask = ~_mm256_movemask_epi8(is_nonzero);
        count += (size_t)__builtin_popcount((unsigned int)mask);
        ptr += 32;
    }

    while (ptr < end) {
        if (table[*ptr]) {
            count++;
        }
        ptr++;
    }

    *out_count = count;
    return true;
}

__attribute__((target("avx2"))) static bool bc_core_count_lines_with_pattern_avx2(const void* data, size_t len, const void* pattern,
                                                                                  size_t pattern_len, size_t* out_count)
{
    if (len == 0 || pattern_len == 0) {
        *out_count = 0;
        return true;
    }

    const unsigned char* bytes = (const unsigned char*)data;
    const unsigned char* needle = (const unsigned char*)pattern;
    unsigned char first_byte = needle[0];
    size_t count = 0;
    size_t offset = 0;
    bool current_line_has_match = false;

    __m256i first_vec = _mm256_set1_epi8((char)first_byte);
    __m256i newline_vec = _mm256_set1_epi8('\n');

    size_t simd_limit = (pattern_len <= len) ? len - pattern_len + 1 : 0;

    while (offset + 32 <= simd_limit) {
        __m256i chunk_data = _mm256_loadu_si256((const __m256i*)(bytes + offset));

        unsigned int newline_mask = (unsigned int)_mm256_movemask_epi8(_mm256_cmpeq_epi8(chunk_data, newline_vec));

        unsigned int candidate_mask = (unsigned int)_mm256_movemask_epi8(_mm256_cmpeq_epi8(chunk_data, first_vec));

        if ((newline_mask | candidate_mask) == 0) {
            offset += 32;
            continue;
        }

        if (candidate_mask == 0) {
            if (current_line_has_match) {
                count++;
                current_line_has_match = false;
            }
            offset += 32;
            continue;
        }

        if (newline_mask == 0) {
            if (!current_line_has_match) {
                while (candidate_mask != 0) {
                    int bit_position = __builtin_ctz(candidate_mask);
                    candidate_mask &= candidate_mask - 1;
                    size_t absolute_position = offset + (size_t)bit_position;
                    if (bc_core_pattern_matches(bytes + absolute_position, needle, pattern_len)) {
                        current_line_has_match = true;
                        break;
                    }
                }
            }
            offset += 32;
            continue;
        }

        while (newline_mask != 0) {
            int newline_bit = __builtin_ctz(newline_mask);
            unsigned int candidates_before_newline = candidate_mask & ((1u << newline_bit) - 1);

            if (!current_line_has_match && candidates_before_newline != 0) {
                while (candidates_before_newline != 0) {
                    int bit_position = __builtin_ctz(candidates_before_newline);
                    candidates_before_newline &= candidates_before_newline - 1;
                    size_t absolute_position = offset + (size_t)bit_position;
                    if (bc_core_pattern_matches(bytes + absolute_position, needle, pattern_len)) {
                        current_line_has_match = true;
                        break;
                    }
                }
            }

            if (current_line_has_match) {
                count++;
            }
            current_line_has_match = false;
            candidate_mask &= (unsigned int)~(((uint64_t)1 << (newline_bit + 1)) - 1);
            newline_mask &= newline_mask - 1;
        }

        if (candidate_mask != 0) {
            while (candidate_mask != 0) {
                int bit_position = __builtin_ctz(candidate_mask);
                candidate_mask &= candidate_mask - 1;
                size_t absolute_position = offset + (size_t)bit_position;
                if (bc_core_pattern_matches(bytes + absolute_position, needle, pattern_len)) {
                    current_line_has_match = true;
                    break;
                }
            }
        }
        offset += 32;
    }

    while (offset < len) {
        if (bytes[offset] == '\n') {
            if (current_line_has_match) {
                count++;
            }
            current_line_has_match = false;
        } else if (!current_line_has_match && bytes[offset] == first_byte && offset + pattern_len <= len &&
                   bc_core_pattern_matches(bytes + offset, needle, pattern_len)) {
            current_line_has_match = true;
        }
        offset++;
    }

    if (current_line_has_match) {
        count++;
    }

    *out_count = count;
    return true;
}

static inline bool is_unicode_ws_2byte(unsigned char c1)
{
    return c1 == 0x85 || c1 == 0xA0;
}

static inline bool is_unicode_ws_3byte(unsigned char lead, unsigned char c1, unsigned char c2)
{
    if (lead == 0xE1) {
        return c1 == 0x9A && c2 == 0x80;
    }
    if (lead == 0xE2) {
        if (c1 == 0x80) {
            return (c2 >= 0x80 && c2 <= 0x8A) || c2 == 0xA8 || c2 == 0xA9 || c2 == 0xAF;
        }
        return c1 == 0x81 && c2 == 0x9F;
    }
    return lead == 0xE3 && c1 == 0x80 && c2 == 0x80;
}

__attribute__((target("avx2"))) static bool bc_core_count_words_avx2(const void* data, size_t len, bool* in_word_state, size_t* out_count)
{
    if (len == 0) {
        *out_count = 0;
        return true;
    }

    const unsigned char* ptr = (const unsigned char*)data;
    const unsigned char* end = ptr + len;
    size_t count = 0;

    unsigned char lo_lut_bytes[16] = {0};
    unsigned char hi_lut_bytes[16] = {0};

    lo_lut_bytes[0x0] = (1 << 2);
    lo_lut_bytes[0x9] = (1 << 0);
    lo_lut_bytes[0xA] = (1 << 0);
    lo_lut_bytes[0xB] = (1 << 0);
    lo_lut_bytes[0xC] = (1 << 0);
    lo_lut_bytes[0xD] = (1 << 0);

    for (int i = 0; i < 8; i++) {
        hi_lut_bytes[i] = (unsigned char)(1 << i);
    }

    __m256i lo_lut = _mm256_broadcastsi128_si256(_mm_loadu_si128((const __m128i*)lo_lut_bytes));
    __m256i hi_lut = _mm256_broadcastsi128_si256(_mm_loadu_si128((const __m128i*)hi_lut_bytes));
    __m256i nibble_mask = _mm256_set1_epi8(0x0F);
    __m256i zero = _mm256_setzero_si256();

    __m256i byte_c2 = _mm256_set1_epi8((char)0xC2);
    __m256i byte_e1 = _mm256_set1_epi8((char)0xE1);
    __m256i byte_e2 = _mm256_set1_epi8((char)0xE2);
    __m256i byte_e3 = _mm256_set1_epi8((char)0xE3);

    unsigned int carry = *in_word_state ? 0 : 1;
    unsigned int carry_ws_mask = 0;

    while (ptr + 32 <= end) {
        __m256i data_vec = _mm256_loadu_si256((const __m256i*)ptr);
        __m256i low_nibbles = _mm256_and_si256(data_vec, nibble_mask);
        __m256i high_nibbles = _mm256_and_si256(_mm256_srli_epi16(data_vec, 4), nibble_mask);
        __m256i lo_result = _mm256_shuffle_epi8(lo_lut, low_nibbles);
        __m256i hi_result = _mm256_shuffle_epi8(hi_lut, high_nibbles);
        __m256i match = _mm256_and_si256(lo_result, hi_result);
        __m256i not_ws = _mm256_cmpeq_epi8(match, zero);
        unsigned int ws_mask = ~(unsigned int)_mm256_movemask_epi8(not_ws);

        ws_mask |= carry_ws_mask;
        carry_ws_mask = 0;

        unsigned int high_bit_mask = (unsigned int)_mm256_movemask_epi8(data_vec);
        unsigned int lead_mask_val = 0;
        if (high_bit_mask != 0) {
            __m256i lead_any = _mm256_or_si256(_mm256_or_si256(_mm256_cmpeq_epi8(data_vec, byte_c2), _mm256_cmpeq_epi8(data_vec, byte_e1)),
                                               _mm256_or_si256(_mm256_cmpeq_epi8(data_vec, byte_e2), _mm256_cmpeq_epi8(data_vec, byte_e3)));
            lead_mask_val = (unsigned int)_mm256_movemask_epi8(lead_any);
        }

        while (lead_mask_val != 0) {
            int pos = __builtin_ctz(lead_mask_val);
            lead_mask_val &= lead_mask_val - 1;
            const unsigned char* lead_ptr = ptr + pos;
            size_t remaining = (size_t)(end - lead_ptr);
            unsigned char lead = lead_ptr[0];

            if (lead == 0xC2 && remaining >= 2 && is_unicode_ws_2byte(lead_ptr[1])) {
                ws_mask |= (1u << (unsigned int)pos);
                if (pos + 1 < 32) {
                    ws_mask |= (1u << (unsigned int)(pos + 1));
                } else {
                    carry_ws_mask |= 1u;
                }
            } else if (remaining >= 3 && is_unicode_ws_3byte(lead, lead_ptr[1], lead_ptr[2])) {
                ws_mask |= (1u << (unsigned int)pos);
                if (pos + 1 < 32) {
                    ws_mask |= (1u << (unsigned int)(pos + 1));
                } else {
                    carry_ws_mask |= 1u;
                }
                if (pos + 2 < 32) {
                    ws_mask |= (1u << (unsigned int)(pos + 2));
                } else {
                    carry_ws_mask |= (1u << (unsigned int)(pos + 2 - 32));
                }
            }
        }

        unsigned int non_ws_mask = ~ws_mask;
        unsigned int prev_ws = (ws_mask << 1) | carry;
        unsigned int word_starts = non_ws_mask & prev_ws;
        count += (size_t)__builtin_popcount(word_starts);
        carry = (ws_mask >> 31) & 1;
        ptr += 32;
    }

    bool in_word = (carry == 0);

    while (carry_ws_mask != 0 && ptr < end) {
        carry_ws_mask >>= 1;
        in_word = false;
        ptr++;
    }

    while (ptr < end) {
        unsigned char byte_value = *ptr;
        bool is_ws = (byte_value == 0x20 || (byte_value >= 0x09 && byte_value <= 0x0D));
        if (!is_ws) {
            size_t remaining = (size_t)(end - ptr);
            if (byte_value == 0xC2 && remaining >= 2 && is_unicode_ws_2byte(ptr[1])) {
                in_word = false;
                ptr += 2;
                continue;
            }
            if (remaining >= 3 && is_unicode_ws_3byte(byte_value, ptr[1], ptr[2])) {
                in_word = false;
                ptr += 3;
                continue;
            }
        }
        if (!is_ws && !in_word) {
            count++;
        }
        in_word = !is_ws;
        ptr++;
    }

    *in_word_state = in_word;
    *out_count = count;
    return true;
}

__attribute__((target("avx2"))) static bool bc_core_count_words_ascii_avx2(const void* data, size_t len, bool* in_word_state,
                                                                           size_t* out_count)
{
    if (len == 0) {
        *out_count = 0;
        return true;
    }

    const unsigned char* ptr = (const unsigned char*)data;
    const unsigned char* end = ptr + len;
    size_t count = 0;

    unsigned char lo_lut_bytes[16] = {0};
    unsigned char hi_lut_bytes[16] = {0};
    lo_lut_bytes[0x0] = (1 << 2);
    lo_lut_bytes[0x9] = (1 << 0);
    lo_lut_bytes[0xA] = (1 << 0);
    lo_lut_bytes[0xB] = (1 << 0);
    lo_lut_bytes[0xC] = (1 << 0);
    lo_lut_bytes[0xD] = (1 << 0);
    for (int i = 0; i < 8; i++) {
        hi_lut_bytes[i] = (unsigned char)(1 << i);
    }

    __m256i lo_lut = _mm256_broadcastsi128_si256(_mm_loadu_si128((const __m128i*)lo_lut_bytes));
    __m256i hi_lut = _mm256_broadcastsi128_si256(_mm_loadu_si128((const __m128i*)hi_lut_bytes));
    __m256i nibble_mask = _mm256_set1_epi8(0x0F);
    __m256i zero = _mm256_setzero_si256();

    unsigned int carry = *in_word_state ? 0 : 1;

    while (ptr + 32 <= end) {
        __m256i data_vec = _mm256_loadu_si256((const __m256i*)ptr);
        __m256i low_nibbles = _mm256_and_si256(data_vec, nibble_mask);
        __m256i high_nibbles = _mm256_and_si256(_mm256_srli_epi16(data_vec, 4), nibble_mask);
        __m256i lo_result = _mm256_shuffle_epi8(lo_lut, low_nibbles);
        __m256i hi_result = _mm256_shuffle_epi8(hi_lut, high_nibbles);
        __m256i match = _mm256_and_si256(lo_result, hi_result);
        __m256i not_ws = _mm256_cmpeq_epi8(match, zero);
        unsigned int ws_mask = ~(unsigned int)_mm256_movemask_epi8(not_ws);
        unsigned int non_ws_mask = ~ws_mask;
        unsigned int prev_ws = (ws_mask << 1) | carry;
        unsigned int word_starts = non_ws_mask & prev_ws;
        count += (size_t)__builtin_popcount(word_starts);
        carry = (ws_mask >> 31) & 1;
        ptr += 32;
    }

    bool in_word = (carry == 0);
    while (ptr < end) {
        unsigned char byte_value = *ptr;
        bool is_ws = (byte_value == 0x20 || (byte_value >= 0x09 && byte_value <= 0x0D));
        if (!is_ws && !in_word) {
            count++;
        }
        in_word = !is_ws;
        ptr++;
    }

    *in_word_state = in_word;
    *out_count = count;
    return true;
}

static bool (*g_count_byte_impl)(const void*, size_t, unsigned char, size_t*) = bc_core_count_byte_avx2;
static bool (*g_count_matching_impl)(const void*, size_t, const unsigned char[256], size_t*) = bc_core_count_matching_avx2;
static bool (*g_count_lines_with_pattern_impl)(const void*, size_t, const void*, size_t, size_t*) = bc_core_count_lines_with_pattern_avx2;
static bool (*g_count_words_impl)(const void*, size_t, bool*, size_t*) = bc_core_count_words_avx2;
static bool (*g_count_words_ascii_impl)(const void*, size_t, bool*, size_t*) = bc_core_count_words_ascii_avx2;

__attribute__((constructor)) void bc_core_count_dispatch_init(void)
{
    bc_core_cpu_features_t features = {0};
    bool detected = bc_core_cpu_features_detect(&features);
    bool use_avx512 = detected && features.has_avx512f && features.has_avx512bw;
    g_count_byte_impl = use_avx512 ? bc_core_count_byte_avx512 : bc_core_count_byte_avx2;
    g_count_matching_impl = bc_core_count_matching_avx2;
    g_count_lines_with_pattern_impl = bc_core_count_lines_with_pattern_avx2;
    g_count_words_impl = bc_core_count_words_avx2;
    g_count_words_ascii_impl = bc_core_count_words_ascii_avx2;
}

bool bc_core_count_byte(const void* data, size_t len, unsigned char target, size_t* out_count)
{
    return g_count_byte_impl(data, len, target, out_count);
}

bool bc_core_count_matching(const void* data, size_t len, const unsigned char table[256], size_t* out_count)
{
    return g_count_matching_impl(data, len, table, out_count);
}

bool bc_core_count_lines(const void* data, size_t len, size_t* out_count)
{
    return bc_core_count_byte(data, len, '\n', out_count);
}

bool bc_core_count_lines_with_pattern(const void* data, size_t len, const void* pattern, size_t pattern_len, size_t* out_count)
{
    return g_count_lines_with_pattern_impl(data, len, pattern, pattern_len, out_count);
}

bool bc_core_count_words(const void* data, size_t len, bool* in_word_state, size_t* out_count)
{
    return g_count_words_impl(data, len, in_word_state, out_count);
}

bool bc_core_count_words_ascii(const void* data, size_t len, bool* in_word_state, size_t* out_count)
{
    return g_count_words_ascii_impl(data, len, in_word_state, out_count);
}
