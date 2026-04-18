// SPDX-License-Identifier: MIT

#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <cmocka.h>

#include "bc_core.h"

/* ===== bc_core_find_byte ===== */

static void test_find_byte_empty_buffer_returns_false(void** state)
{
    BC_UNUSED(state);

    const unsigned char data[1] = {0xAA};
    size_t offset = 99;

    bool found = bc_core_find_byte(data, 0, 0xAA, &offset);

    assert_false(found);
}

static void test_find_byte_single_byte_match_returns_true_at_offset_zero(void** state)
{
    BC_UNUSED(state);

    const unsigned char data[1] = {0x42};
    size_t offset = 99;

    bool found = bc_core_find_byte(data, 1, 0x42, &offset);

    assert_true(found);
    assert_int_equal(offset, 0);
}

static void test_find_byte_single_byte_no_match_returns_false(void** state)
{
    BC_UNUSED(state);

    const unsigned char data[1] = {0x42};
    size_t offset = 99;

    bool found = bc_core_find_byte(data, 1, 0xFF, &offset);

    assert_false(found);
}

static void test_find_byte_in_scalar_tail_31_bytes_returns_correct_offset(void** state)
{
    BC_UNUSED(state);

    unsigned char data[31];
    bc_core_fill(data, sizeof(data), (unsigned char)0x00);
    data[30] = 0xBB;
    size_t offset = 99;

    bool found = bc_core_find_byte(data, 31, 0xBB, &offset);

    assert_true(found);
    assert_int_equal(offset, 30);
}

static void test_find_byte_in_32byte_vector_block_returns_correct_offset(void** state)
{
    BC_UNUSED(state);

    unsigned char data[32];
    bc_core_fill(data, sizeof(data), (unsigned char)0x00);
    data[15] = 0xCC;
    size_t offset = 99;

    bool found = bc_core_find_byte(data, 32, 0xCC, &offset);

    assert_true(found);
    assert_int_equal(offset, 15);
}

static void test_find_byte_in_128byte_block_at_position_zero(void** state)
{
    BC_UNUSED(state);

    unsigned char data[128];
    bc_core_fill(data, sizeof(data), (unsigned char)0x00);
    data[0] = 0xAB;
    size_t offset = 99;

    bool found = bc_core_find_byte(data, 128, 0xAB, &offset);

    assert_true(found);
    assert_int_equal(offset, 0);
}

static void test_find_byte_in_128byte_block_at_position_32(void** state)
{
    BC_UNUSED(state);

    unsigned char data[128];
    bc_core_fill(data, sizeof(data), (unsigned char)0x00);
    data[32] = 0xAB;
    size_t offset = 99;

    bool found = bc_core_find_byte(data, 128, 0xAB, &offset);

    assert_true(found);
    assert_int_equal(offset, 32);
}

static void test_find_byte_in_128byte_block_at_position_64(void** state)
{
    BC_UNUSED(state);

    unsigned char data[128];
    bc_core_fill(data, sizeof(data), (unsigned char)0x00);
    data[64] = 0xAB;
    size_t offset = 99;

    bool found = bc_core_find_byte(data, 128, 0xAB, &offset);

    assert_true(found);
    assert_int_equal(offset, 64);
}

static void test_find_byte_in_128byte_block_at_position_96(void** state)
{
    BC_UNUSED(state);

    unsigned char data[128];
    bc_core_fill(data, sizeof(data), (unsigned char)0x00);
    data[96] = 0xAB;
    size_t offset = 99;

    bool found = bc_core_find_byte(data, 128, 0xAB, &offset);

    assert_true(found);
    assert_int_equal(offset, 96);
}

static void test_find_byte_in_129byte_buffer_target_in_tail(void** state)
{
    BC_UNUSED(state);

    unsigned char data[129];
    bc_core_fill(data, sizeof(data), (unsigned char)0x00);
    data[128] = 0xDE;
    size_t offset = 99;

    bool found = bc_core_find_byte(data, 129, 0xDE, &offset);

    assert_true(found);
    assert_int_equal(offset, 128);
}

static void test_find_byte_in_256byte_buffer_not_found_returns_false(void** state)
{
    BC_UNUSED(state);

    unsigned char data[256];
    bc_core_fill(data, sizeof(data), (unsigned char)0x01);
    size_t offset = 99;

    bool found = bc_core_find_byte(data, 256, 0xFF, &offset);

    assert_false(found);
}

static void test_find_byte_in_512byte_buffer_returns_first_occurrence(void** state)
{
    BC_UNUSED(state);

    unsigned char data[512];
    bc_core_fill(data, sizeof(data), (unsigned char)0x00);
    data[200] = 0x77;
    data[400] = 0x77;
    size_t offset = 99;

    bool found = bc_core_find_byte(data, 512, 0x77, &offset);

    assert_true(found);
    assert_int_equal(offset, 200);
}

static void test_find_byte_target_byte_value_zero(void** state)
{
    BC_UNUSED(state);

    unsigned char data[64];
    bc_core_fill(data, sizeof(data), (unsigned char)0xFF);
    data[40] = 0x00;
    size_t offset = 99;

    bool found = bc_core_find_byte(data, 64, 0x00, &offset);

    assert_true(found);
    assert_int_equal(offset, 40);
}

/* ===== bc_core_find_pattern ===== */

static void test_find_pattern_zero_pattern_len_returns_false(void** state)
{
    BC_UNUSED(state);

    const unsigned char data[4] = {0xAA, 0xBB, 0xCC, 0xDD};
    const unsigned char pattern[1] = {0xAA};
    size_t offset = 99;

    bool found = bc_core_find_pattern(data, 4, pattern, 0, &offset);

    assert_false(found);
}

static void test_find_pattern_empty_data_returns_false(void** state)
{
    BC_UNUSED(state);

    const unsigned char data[4] = {0xAA, 0xBB, 0xCC, 0xDD};
    const unsigned char pattern[2] = {0xAA, 0xBB};
    size_t offset = 99;

    bool found = bc_core_find_pattern(data, 0, pattern, 2, &offset);

    assert_false(found);
}

static void test_find_pattern_len_1_delegates_to_find_byte(void** state)
{
    BC_UNUSED(state);

    unsigned char data[16];
    bc_core_fill(data, sizeof(data), (unsigned char)0x00);
    data[7] = 0x5A;
    const unsigned char pattern[1] = {0x5A};
    size_t offset = 99;

    bool found = bc_core_find_pattern(data, 16, pattern, 1, &offset);

    assert_true(found);
    assert_int_equal(offset, 7);
}

static void test_find_pattern_longer_than_data_returns_false(void** state)
{
    BC_UNUSED(state);

    const unsigned char data[4] = {0xAA, 0xBB, 0xCC, 0xDD};
    const unsigned char pattern[8] = {0xAA, 0xBB, 0xCC, 0xDD, 0x00, 0x00, 0x00, 0x00};
    size_t offset = 99;

    bool found = bc_core_find_pattern(data, 4, pattern, 8, &offset);

    assert_false(found);
}

static void test_find_pattern_len_2_found_in_scalar_tail(void** state)
{
    BC_UNUSED(state);

    unsigned char data[10];
    bc_core_fill(data, sizeof(data), (unsigned char)0x00);
    data[8] = 0x12;
    data[9] = 0x34;
    const unsigned char pattern[2] = {0x12, 0x34};
    size_t offset = 99;

    bool found = bc_core_find_pattern(data, 10, pattern, 2, &offset);

    assert_true(found);
    assert_int_equal(offset, 8);
}

static void test_find_pattern_len_2_not_found_returns_false(void** state)
{
    BC_UNUSED(state);

    unsigned char data[16];
    bc_core_fill(data, sizeof(data), (unsigned char)0xAA);
    const unsigned char pattern[2] = {0x12, 0x34};
    size_t offset = 99;

    bool found = bc_core_find_pattern(data, 16, pattern, 2, &offset);

    assert_false(found);
}

static void test_find_pattern_len_3_with_false_first_byte_match(void** state)
{
    BC_UNUSED(state);

    /* Sequence starts with 0xAA but middle byte does not match */
    unsigned char data[16];
    bc_core_fill(data, sizeof(data), (unsigned char)0x00);
    data[0] = 0xAA;
    data[1] = 0xFF;
    data[2] = 0xCC;
    data[10] = 0xAA;
    data[11] = 0xBB;
    data[12] = 0xCC;
    const unsigned char pattern[3] = {0xAA, 0xBB, 0xCC};
    size_t offset = 99;

    bool found = bc_core_find_pattern(data, 16, pattern, 3, &offset);

    assert_true(found);
    assert_int_equal(offset, 10);
}

static void test_find_pattern_len_8_found_in_32byte_block(void** state)
{
    BC_UNUSED(state);

    unsigned char data[32];
    bc_core_fill(data, sizeof(data), (unsigned char)0x00);
    data[20] = 0x01;
    data[21] = 0x02;
    data[22] = 0x03;
    data[23] = 0x04;
    data[24] = 0x05;
    data[25] = 0x06;
    data[26] = 0x07;
    data[27] = 0x08;
    const unsigned char pattern[8] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
    size_t offset = 99;

    bool found = bc_core_find_pattern(data, 32, pattern, 8, &offset);

    assert_true(found);
    assert_int_equal(offset, 20);
}

static void test_find_pattern_len_33_found_in_128byte_block(void** state)
{
    BC_UNUSED(state);

    unsigned char data[256];
    bc_core_fill(data, sizeof(data), (unsigned char)0x00);
    unsigned char pattern[33];
    bc_core_fill(pattern, sizeof(pattern), (unsigned char)0xEF);
    bc_core_copy(data + 50, pattern, 33);
    size_t offset = 99;

    bool found = bc_core_find_pattern(data, 256, pattern, 33, &offset);

    assert_true(found);
    assert_int_equal(offset, 50);
}

static void test_find_pattern_len_2_found_in_32byte_block(void** state)
{
    BC_UNUSED(state);

    unsigned char data[64];
    bc_core_fill(data, sizeof(data), (unsigned char)0x00);
    data[16] = 0xAB;
    data[17] = 0xCD;
    const unsigned char pattern[2] = {0xAB, 0xCD};
    size_t offset = 99;

    bool found = bc_core_find_pattern(data, 64, pattern, 2, &offset);

    assert_true(found);
    assert_int_equal(offset, 16);
}

static void test_find_pattern_len_3_found_in_128byte_block(void** state)
{
    BC_UNUSED(state);

    unsigned char data[256];
    bc_core_fill(data, sizeof(data), (unsigned char)0x00);
    data[70] = 0x11;
    data[71] = 0x22;
    data[72] = 0x33;
    const unsigned char pattern[3] = {0x11, 0x22, 0x33};
    size_t offset = 99;

    bool found = bc_core_find_pattern(data, 256, pattern, 3, &offset);

    assert_true(found);
    assert_int_equal(offset, 70);
}

static void test_find_pattern_not_found_in_large_buffer_returns_false(void** state)
{
    BC_UNUSED(state);

    unsigned char data[512];
    bc_core_fill(data, sizeof(data), (unsigned char)0x01);
    const unsigned char pattern[3] = {0x01, 0x02, 0x01};
    size_t offset = 99;

    bool found = bc_core_find_pattern(data, 512, pattern, 3, &offset);

    assert_false(found);
}

static void test_find_pattern_len_2_found_in_128byte_block_first_chunk(void** state)
{
    BC_UNUSED(state);

    /* pattern_len==2, target in m0 (offset 0..31) of 128B block */
    unsigned char data[256];
    bc_core_fill(data, sizeof(data), (unsigned char)0x00);
    data[10] = 0xAB;
    data[11] = 0xCD;
    const unsigned char pattern[2] = {0xAB, 0xCD};
    size_t offset = 99;

    bool found = bc_core_find_pattern(data, 256, pattern, 2, &offset);

    assert_true(found);
    assert_int_equal(offset, 10);
}

static void test_find_pattern_len_2_found_in_128byte_block_fourth_chunk(void** state)
{
    BC_UNUSED(state);

    /* pattern_len==2, target in m3 (offset 96..127) of 128B block */
    unsigned char data[256];
    bc_core_fill(data, sizeof(data), (unsigned char)0x00);
    data[100] = 0xAB;
    data[101] = 0xCD;
    const unsigned char pattern[2] = {0xAB, 0xCD};
    size_t offset = 99;

    bool found = bc_core_find_pattern(data, 256, pattern, 2, &offset);

    assert_true(found);
    assert_int_equal(offset, 100);
}

static void test_find_pattern_128b_block_any_zero_first_block_skipped_match_in_second(void** state)
{
    BC_UNUSED(state);

    /*
     * Line 119: false branch of `if (_mm256_movemask_epi8(any) != 0)`.
     * The first 128-byte block contains neither the first byte nor the last byte of
     * the pattern, so `any` is all-zero and the block is skipped.
     * The pattern is present in the second 128-byte block.
     * Buffer: 256 bytes. pattern = {0xAA, 0xBB, 0xCC} (len=3).
     * Bytes 0..127: filled with 0x01 (no 0xAA, no 0xCC).
     * Bytes 128+50 .. 128+52: 0xAA, 0xBB, 0xCC (real match at offset 178).
     */
    unsigned char data[256];
    bc_core_fill(data, sizeof(data), (unsigned char)0x01);
    data[178] = 0xAA;
    data[179] = 0xBB;
    data[180] = 0xCC;
    const unsigned char pattern[3] = {0xAA, 0xBB, 0xCC};
    size_t offset = 99;

    bool found = bc_core_find_pattern(data, 256, pattern, 3, &offset);

    assert_true(found);
    assert_int_equal(offset, 178);
}

static void test_find_pattern_false_positive_in_m0_chunk_real_match_after(void** state)
{
    BC_UNUSED(state);

    /*
     * Lines 125: false branch of `pattern_len == 2 || pattern_matches(...)` in the m0 chunk.
     * A false positive in m0 (offset 0..31): first and last bytes match but middle does not.
     * The real match is in m1 (offset 32..63) of the same 128B block.
     * pattern = {0xAA, 0xBB, 0xCC} (len=3, first=0xAA, last=0xCC).
     * False positive at offset 5: data[5]=0xAA, data[6]=0xFF, data[7]=0xCC (middle wrong).
     * Real match at offset 35: data[35]=0xAA, data[36]=0xBB, data[37]=0xCC.
     * Buffer is 256 bytes (ensures 128B SIMD block is used).
     */
    unsigned char data[256];
    bc_core_fill(data, sizeof(data), (unsigned char)0x01);
    data[5] = 0xAA;
    data[6] = 0xFF;
    data[7] = 0xCC;
    data[35] = 0xAA;
    data[36] = 0xBB;
    data[37] = 0xCC;
    const unsigned char pattern[3] = {0xAA, 0xBB, 0xCC};
    size_t offset = 99;

    bool found = bc_core_find_pattern(data, 256, pattern, 3, &offset);

    assert_true(found);
    assert_int_equal(offset, 35);
}

static void test_find_pattern_false_positive_in_m1_chunk_real_match_after(void** state)
{
    BC_UNUSED(state);

    /*
     * Line 135: false branch of `pattern_len == 2 || pattern_matches(...)` in the m1 chunk.
     * False positive in m1 (offset 32..63): first and last bytes match but middle does not.
     * Real match in m2 (offset 64..95).
     * pattern = {0xAA, 0xBB, 0xCC} (len=3).
     * No 0xAA in m0, false positive at 40, real match at 70.
     */
    unsigned char data[256];
    bc_core_fill(data, sizeof(data), (unsigned char)0x01);
    data[40] = 0xAA;
    data[41] = 0xFF;
    data[42] = 0xCC;
    data[70] = 0xAA;
    data[71] = 0xBB;
    data[72] = 0xCC;
    const unsigned char pattern[3] = {0xAA, 0xBB, 0xCC};
    size_t offset = 99;

    bool found = bc_core_find_pattern(data, 256, pattern, 3, &offset);

    assert_true(found);
    assert_int_equal(offset, 70);
}

static void test_find_pattern_false_positive_in_m2_chunk_real_match_after(void** state)
{
    BC_UNUSED(state);

    /*
     * Line 145: false branch of `pattern_len == 2 || pattern_matches(...)` in the m2 chunk.
     * False positive in m2 (offset 64..95): first and last bytes match but middle does not.
     * Real match in m3 (offset 96..127).
     * pattern = {0xAA, 0xBB, 0xCC} (len=3).
     * No 0xAA in m0 or m1, false positive at 70, real match at 100.
     */
    unsigned char data[256];
    bc_core_fill(data, sizeof(data), (unsigned char)0x01);
    data[70] = 0xAA;
    data[71] = 0xFF;
    data[72] = 0xCC;
    data[100] = 0xAA;
    data[101] = 0xBB;
    data[102] = 0xCC;
    const unsigned char pattern[3] = {0xAA, 0xBB, 0xCC};
    size_t offset = 99;

    bool found = bc_core_find_pattern(data, 256, pattern, 3, &offset);

    assert_true(found);
    assert_int_equal(offset, 100);
}

static void test_find_pattern_false_positive_in_m3_chunk_real_match_in_next_block(void** state)
{
    BC_UNUSED(state);

    /*
     * Line 155: false branch of `pattern_len == 2 || pattern_matches(...)` in the m3 chunk.
     * False positive in m3 (offset 96..127): first and last bytes match but middle does not.
     * Real match in the next 128B block (offset 128+).
     * pattern = {0xAA, 0xBB, 0xCC} (len=3).
     * No 0xAA in m0/m1/m2, false positive at 100, real match at 150.
     * Buffer is 512 bytes so that the second 128B block is fully covered.
     */
    unsigned char data[512];
    bc_core_fill(data, sizeof(data), (unsigned char)0x01);
    data[100] = 0xAA;
    data[101] = 0xFF;
    data[102] = 0xCC;
    data[150] = 0xAA;
    data[151] = 0xBB;
    data[152] = 0xCC;
    const unsigned char pattern[3] = {0xAA, 0xBB, 0xCC};
    size_t offset = 99;

    bool found = bc_core_find_pattern(data, 512, pattern, 3, &offset);

    assert_true(found);
    assert_int_equal(offset, 150);
}

static void test_find_pattern_false_positive_in_32b_block_real_match_in_scalar_tail(void** state)
{
    BC_UNUSED(state);

    /*
     * Line 174: false branch of `pattern_len == 2 || pattern_matches(...)` in the 32B block loop.
     * Buffer is sized so the 128B loop is not used (len - (pattern_len-1) < 128).
     * 32B block loop is used. False positive in that block, real match in scalar tail.
     * pattern = {0xAA, 0xBB, 0xCC} (len=3, first=0xAA, last=0xCC).
     * simd_end = len - 2. Buffer len = 45.
     * 32B block loop: offset=0, offset+32=32 <= simd_end=43. One 32B block.
     * False positive at offset 5: data[5]=0xAA, data[6]=0xFF, data[7]=0xCC.
     * Real match in scalar tail at offset 42: data[42]=0xAA, data[43]=0xBB, data[44]=0xCC.
     */
    unsigned char data[45];
    bc_core_fill(data, sizeof(data), (unsigned char)0x01);
    data[5] = 0xAA;
    data[6] = 0xFF;
    data[7] = 0xCC;
    data[42] = 0xAA;
    data[43] = 0xBB;
    data[44] = 0xCC;
    const unsigned char pattern[3] = {0xAA, 0xBB, 0xCC};
    size_t offset = 99;

    bool found = bc_core_find_pattern(data, 45, pattern, 3, &offset);

    assert_true(found);
    assert_int_equal(offset, 42);
}

static void test_find_pattern_false_positive_in_scalar_tail_real_match_after(void** state)
{
    BC_UNUSED(state);

    /*
     * Line 184: false branch of `pattern_len == 2 || pattern_matches(...)` in the scalar tail.
     * The scalar tail loop processes candidates one by one. A false positive appears first
     * (first and last bytes match but middle does not), then the real match follows.
     * Use a small buffer entirely in scalar range (len < 32+pattern_len-1 => no SIMD blocks).
     * pattern = {0xAA, 0xBB, 0xCC} (len=3).
     * len=10. simd_end = 10-2 = 8. No 32B block (0+32 > 8). Scalar loop: offset 0..7.
     * False positive at 0: data[0]=0xAA, data[1]=0xFF, data[2]=0xCC.
     * Real match at 3: data[3]=0xAA, data[4]=0xBB, data[5]=0xCC.
     */
    unsigned char data[10];
    bc_core_fill(data, sizeof(data), (unsigned char)0x01);
    data[0] = 0xAA;
    data[1] = 0xFF;
    data[2] = 0xCC;
    data[3] = 0xAA;
    data[4] = 0xBB;
    data[5] = 0xCC;
    const unsigned char pattern[3] = {0xAA, 0xBB, 0xCC};
    size_t offset = 99;

    bool found = bc_core_find_pattern(data, 10, pattern, 3, &offset);

    assert_true(found);
    assert_int_equal(offset, 3);
}

static void test_find_pattern_scalar_tail_first_byte_matches_last_byte_does_not(void** state)
{
    BC_UNUSED(state);

    unsigned char data[10];
    bc_core_fill(data, sizeof(data), (unsigned char)0x01);
    data[0] = 0xAA;
    data[5] = 0xAA;
    data[6] = 0xBB;
    data[7] = 0xCC;
    const unsigned char pattern[3] = {0xAA, 0xBB, 0xCC};
    size_t offset = 99;

    bool found = bc_core_find_pattern(data, 10, pattern, 3, &offset);

    assert_true(found);
    assert_int_equal(offset, 5);
}

static void test_find_pattern_len_3_false_match_exhausts_all_candidates_in_128b_block(void** state)
{
    BC_UNUSED(state);

    /*
     * pattern_len==3, multiple false first+last matches in 128B block before the real match.
     * Forces the "mask &= mask - 1" loop to iterate more than once.
     */
    unsigned char data[256];
    bc_core_fill(data, sizeof(data), (unsigned char)0x00);
    /* False matches: first=0xAA, last=0xCC but middle wrong */
    data[5] = 0xAA;
    data[6] = 0xFF;
    data[7] = 0xCC;
    data[20] = 0xAA;
    data[21] = 0xFF;
    data[22] = 0xCC;
    /* Real match at 40 */
    data[40] = 0xAA;
    data[41] = 0xBB;
    data[42] = 0xCC;
    const unsigned char pattern[3] = {0xAA, 0xBB, 0xCC};
    size_t offset = 99;

    bool found = bc_core_find_pattern(data, 256, pattern, 3, &offset);

    assert_true(found);
    assert_int_equal(offset, 40);
}

/* ===== bc_core_find_last_byte ===== */

static void test_find_last_byte_empty_buffer_returns_false(void** state)
{
    BC_UNUSED(state);

    const unsigned char data[1] = {0xAA};
    size_t offset = 99;

    bool found = bc_core_find_last_byte(data, 0, 0xAA, &offset);

    assert_false(found);
}

static void test_find_last_byte_single_byte_match_returns_offset_zero(void** state)
{
    BC_UNUSED(state);

    const unsigned char data[1] = {0x55};
    size_t offset = 99;

    bool found = bc_core_find_last_byte(data, 1, 0x55, &offset);

    assert_true(found);
    assert_int_equal(offset, 0);
}

static void test_find_last_byte_found_in_scalar_tail_non_multiple_of_32(void** state)
{
    BC_UNUSED(state);

    /* len=35 => tail = 3 bytes (35 % 32), target in tail */
    unsigned char data[35];
    bc_core_fill(data, sizeof(data), (unsigned char)0x00);
    data[33] = 0xAA;
    size_t offset = 99;

    bool found = bc_core_find_last_byte(data, 35, 0xAA, &offset);

    assert_true(found);
    assert_int_equal(offset, 33);
}

static void test_find_last_byte_multiple_occurrences_returns_last(void** state)
{
    BC_UNUSED(state);

    unsigned char data[35];
    bc_core_fill(data, sizeof(data), (unsigned char)0x00);
    data[10] = 0xAA;
    data[33] = 0xAA;
    size_t offset = 99;

    bool found = bc_core_find_last_byte(data, 35, 0xAA, &offset);

    assert_true(found);
    assert_int_equal(offset, 33);
}

static void test_find_last_byte_exact_multiple_of_32_found_in_vector_block(void** state)
{
    BC_UNUSED(state);

    /* len=64, no tail, target in second-to-last 32B block */
    unsigned char data[64];
    bc_core_fill(data, sizeof(data), (unsigned char)0x00);
    data[10] = 0xBB;
    size_t offset = 99;

    bool found = bc_core_find_last_byte(data, 64, 0xBB, &offset);

    assert_true(found);
    assert_int_equal(offset, 10);
}

static void test_find_last_byte_exact_multiple_of_32_target_in_last_block(void** state)
{
    BC_UNUSED(state);

    /* len=64, target only in bytes [32..63] */
    unsigned char data[64];
    bc_core_fill(data, sizeof(data), (unsigned char)0x00);
    data[50] = 0xCC;
    size_t offset = 99;

    bool found = bc_core_find_last_byte(data, 64, 0xCC, &offset);

    assert_true(found);
    assert_int_equal(offset, 50);
}

static void test_find_last_byte_not_found_returns_false(void** state)
{
    BC_UNUSED(state);

    unsigned char data[64];
    bc_core_fill(data, sizeof(data), (unsigned char)0x00);
    size_t offset = 99;

    bool found = bc_core_find_last_byte(data, 64, 0xFF, &offset);

    assert_false(found);
}

static void test_find_last_byte_len_not_multiple_of_32_target_only_in_vector_part(void** state)
{
    BC_UNUSED(state);

    /* len=35 => tail=3, aligned=32, target only in [0..31] */
    unsigned char data[35];
    bc_core_fill(data, sizeof(data), (unsigned char)0x00);
    data[5] = 0xDD;
    size_t offset = 99;

    bool found = bc_core_find_last_byte(data, 35, 0xDD, &offset);

    assert_true(found);
    assert_int_equal(offset, 5);
}

static void test_find_last_byte_128B_target_at_position_zero(void** state)
{
    BC_UNUSED(state);

    /* len=128, target only at position 0: forces all 4 backward 32B blocks to be scanned */
    unsigned char data[128];
    bc_core_fill(data, sizeof(data), (unsigned char)0x00);
    data[0] = 0xAA;
    size_t offset = 99;

    bool found = bc_core_find_last_byte(data, 128, 0xAA, &offset);

    assert_true(found);
    assert_int_equal(offset, 0);
}

static void test_find_last_byte_128B_multiple_occurrences_returns_last(void** state)
{
    BC_UNUSED(state);

    /* len=128, target at 0 and at 120: must return 120 */
    unsigned char data[128];
    bc_core_fill(data, sizeof(data), (unsigned char)0x00);
    data[0] = 0xBB;
    data[120] = 0xBB;
    size_t offset = 99;

    bool found = bc_core_find_last_byte(data, 128, 0xBB, &offset);

    assert_true(found);
    assert_int_equal(offset, 120);
}

/* ===== bc_core_find_any_byte ===== */

static void test_find_any_byte_empty_buffer_returns_false(void** state)
{
    BC_UNUSED(state);

    const unsigned char data[4] = {0x01, 0x02, 0x03, 0x04};
    const unsigned char targets[1] = {0x01};
    size_t offset = 99;

    bool found = bc_core_find_any_byte(data, 0, targets, 1, &offset);

    assert_false(found);
}

static void test_find_any_byte_zero_target_count_returns_false(void** state)
{
    BC_UNUSED(state);

    const unsigned char data[4] = {0x01, 0x02, 0x03, 0x04};
    const unsigned char targets[1] = {0x01};
    size_t offset = 99;

    bool found = bc_core_find_any_byte(data, 4, targets, 0, &offset);

    assert_false(found);
}

static void test_find_any_byte_low_nibble_high_nibble_less_than_8_pass1(void** state)
{
    BC_UNUSED(state);

    /* 0x7F: high nibble = 7 < 8, goes to pass1 */
    unsigned char data[32];
    bc_core_fill(data, sizeof(data), (unsigned char)0x00);
    data[10] = 0x7F;
    const unsigned char targets[1] = {0x7F};
    size_t offset = 99;

    bool found = bc_core_find_any_byte(data, 32, targets, 1, &offset);

    assert_true(found);
    assert_int_equal(offset, 10);
}

static void test_find_any_byte_high_nibble_8_or_more_pass2(void** state)
{
    BC_UNUSED(state);

    /* 0x80: high nibble = 8 >= 8, goes to pass2 */
    unsigned char data[32];
    bc_core_fill(data, sizeof(data), (unsigned char)0x00);
    data[20] = 0x80;
    const unsigned char targets[1] = {0x80};
    size_t offset = 99;

    bool found = bc_core_find_any_byte(data, 32, targets, 1, &offset);

    assert_true(found);
    assert_int_equal(offset, 20);
}

static void test_find_any_byte_target_0xFF_high_nibble_F_pass2(void** state)
{
    BC_UNUSED(state);

    /* 0xFF: high nibble = 15 >= 8, goes to pass2 */
    unsigned char data[32];
    bc_core_fill(data, sizeof(data), (unsigned char)0x00);
    data[5] = 0xFF;
    const unsigned char targets[1] = {0xFF};
    size_t offset = 99;

    bool found = bc_core_find_any_byte(data, 32, targets, 1, &offset);

    assert_true(found);
    assert_int_equal(offset, 5);
}

static void test_find_any_byte_target_0x00_found_in_32byte_block(void** state)
{
    BC_UNUSED(state);

    /* 0x00: high nibble = 0 < 8, pass1 */
    unsigned char data[32];
    bc_core_fill(data, sizeof(data), (unsigned char)0x01);
    data[3] = 0x00;
    const unsigned char targets[1] = {0x00};
    size_t offset = 99;

    bool found = bc_core_find_any_byte(data, 32, targets, 1, &offset);

    assert_true(found);
    assert_int_equal(offset, 3);
}

static void test_find_any_byte_multiple_targets_both_passes_found_in_vector(void** state)
{
    BC_UNUSED(state);

    /* 0x1A (high nibble 1 < 8, pass1) and 0x9B (high nibble 9 >= 8, pass2) */
    unsigned char data[32];
    bc_core_fill(data, sizeof(data), (unsigned char)0x00);
    data[15] = 0x9B;
    data[25] = 0x1A;
    const unsigned char targets[2] = {0x1A, 0x9B};
    size_t offset = 99;

    bool found = bc_core_find_any_byte(data, 32, targets, 2, &offset);

    assert_true(found);
    assert_int_equal(offset, 15);
}

static void test_find_any_byte_found_in_scalar_tail(void** state)
{
    BC_UNUSED(state);

    /* Buffer of 35 bytes, target in position 33 (scalar tail) */
    unsigned char data[35];
    bc_core_fill(data, sizeof(data), (unsigned char)0x00);
    data[33] = 0x42;
    const unsigned char targets[1] = {0x42};
    size_t offset = 99;

    bool found = bc_core_find_any_byte(data, 35, targets, 1, &offset);

    assert_true(found);
    assert_int_equal(offset, 33);
}

static void test_find_any_byte_not_found_returns_false(void** state)
{
    BC_UNUSED(state);

    unsigned char data[64];
    bc_core_fill(data, sizeof(data), (unsigned char)0x01);
    const unsigned char targets[2] = {0xAA, 0xBB};
    size_t offset = 99;

    bool found = bc_core_find_any_byte(data, 64, targets, 2, &offset);

    assert_false(found);
}

static void test_find_any_byte_returns_first_occurrence_in_vector(void** state)
{
    BC_UNUSED(state);

    unsigned char data[64];
    bc_core_fill(data, sizeof(data), (unsigned char)0x00);
    data[10] = 0x7F;
    data[50] = 0x7F;
    const unsigned char targets[1] = {0x7F};
    size_t offset = 99;

    bool found = bc_core_find_any_byte(data, 64, targets, 1, &offset);

    assert_true(found);
    assert_int_equal(offset, 10);
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        /* find_byte */
        cmocka_unit_test(test_find_byte_empty_buffer_returns_false),
        cmocka_unit_test(test_find_byte_single_byte_match_returns_true_at_offset_zero),
        cmocka_unit_test(test_find_byte_single_byte_no_match_returns_false),
        cmocka_unit_test(test_find_byte_in_scalar_tail_31_bytes_returns_correct_offset),
        cmocka_unit_test(test_find_byte_in_32byte_vector_block_returns_correct_offset),
        cmocka_unit_test(test_find_byte_in_128byte_block_at_position_zero),
        cmocka_unit_test(test_find_byte_in_128byte_block_at_position_32),
        cmocka_unit_test(test_find_byte_in_128byte_block_at_position_64),
        cmocka_unit_test(test_find_byte_in_128byte_block_at_position_96),
        cmocka_unit_test(test_find_byte_in_129byte_buffer_target_in_tail),
        cmocka_unit_test(test_find_byte_in_256byte_buffer_not_found_returns_false),
        cmocka_unit_test(test_find_byte_in_512byte_buffer_returns_first_occurrence),
        cmocka_unit_test(test_find_byte_target_byte_value_zero),
        /* find_pattern */
        cmocka_unit_test(test_find_pattern_zero_pattern_len_returns_false),
        cmocka_unit_test(test_find_pattern_empty_data_returns_false),
        cmocka_unit_test(test_find_pattern_len_1_delegates_to_find_byte),
        cmocka_unit_test(test_find_pattern_longer_than_data_returns_false),
        cmocka_unit_test(test_find_pattern_len_2_found_in_scalar_tail),
        cmocka_unit_test(test_find_pattern_len_2_not_found_returns_false),
        cmocka_unit_test(test_find_pattern_len_3_with_false_first_byte_match),
        cmocka_unit_test(test_find_pattern_len_8_found_in_32byte_block),
        cmocka_unit_test(test_find_pattern_len_33_found_in_128byte_block),
        cmocka_unit_test(test_find_pattern_len_2_found_in_32byte_block),
        cmocka_unit_test(test_find_pattern_len_3_found_in_128byte_block),
        cmocka_unit_test(test_find_pattern_not_found_in_large_buffer_returns_false),
        cmocka_unit_test(test_find_pattern_len_2_found_in_128byte_block_first_chunk),
        cmocka_unit_test(test_find_pattern_len_2_found_in_128byte_block_fourth_chunk),
        cmocka_unit_test(test_find_pattern_len_3_false_match_exhausts_all_candidates_in_128b_block),
        cmocka_unit_test(test_find_pattern_128b_block_any_zero_first_block_skipped_match_in_second),
        cmocka_unit_test(test_find_pattern_false_positive_in_m0_chunk_real_match_after),
        cmocka_unit_test(test_find_pattern_false_positive_in_m1_chunk_real_match_after),
        cmocka_unit_test(test_find_pattern_false_positive_in_m2_chunk_real_match_after),
        cmocka_unit_test(test_find_pattern_false_positive_in_m3_chunk_real_match_in_next_block),
        cmocka_unit_test(test_find_pattern_false_positive_in_32b_block_real_match_in_scalar_tail),
        cmocka_unit_test(test_find_pattern_false_positive_in_scalar_tail_real_match_after),
        cmocka_unit_test(test_find_pattern_scalar_tail_first_byte_matches_last_byte_does_not),
        /* find_last_byte */
        cmocka_unit_test(test_find_last_byte_empty_buffer_returns_false),
        cmocka_unit_test(test_find_last_byte_single_byte_match_returns_offset_zero),
        cmocka_unit_test(test_find_last_byte_found_in_scalar_tail_non_multiple_of_32),
        cmocka_unit_test(test_find_last_byte_multiple_occurrences_returns_last),
        cmocka_unit_test(test_find_last_byte_exact_multiple_of_32_found_in_vector_block),
        cmocka_unit_test(test_find_last_byte_exact_multiple_of_32_target_in_last_block),
        cmocka_unit_test(test_find_last_byte_not_found_returns_false),
        cmocka_unit_test(test_find_last_byte_len_not_multiple_of_32_target_only_in_vector_part),
        cmocka_unit_test(test_find_last_byte_128B_target_at_position_zero),
        cmocka_unit_test(test_find_last_byte_128B_multiple_occurrences_returns_last),
        /* find_any_byte */
        cmocka_unit_test(test_find_any_byte_empty_buffer_returns_false),
        cmocka_unit_test(test_find_any_byte_zero_target_count_returns_false),
        cmocka_unit_test(test_find_any_byte_low_nibble_high_nibble_less_than_8_pass1),
        cmocka_unit_test(test_find_any_byte_high_nibble_8_or_more_pass2),
        cmocka_unit_test(test_find_any_byte_target_0xFF_high_nibble_F_pass2),
        cmocka_unit_test(test_find_any_byte_target_0x00_found_in_32byte_block),
        cmocka_unit_test(test_find_any_byte_multiple_targets_both_passes_found_in_vector),
        cmocka_unit_test(test_find_any_byte_found_in_scalar_tail),
        cmocka_unit_test(test_find_any_byte_not_found_returns_false),
        cmocka_unit_test(test_find_any_byte_returns_first_occurrence_in_vector),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
