// SPDX-License-Identifier: MIT

#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <cmocka.h>

#include "bc_core.h"

#include <stdlib.h>

/* ===== bc_core_count_byte ===== */

static void test_count_byte_empty_buffer_returns_zero(void** state)
{
    BC_UNUSED(state);

    const unsigned char data[1] = {0xAA};
    size_t count = 99;

    bool success = bc_core_count_byte(data, 0, 0xAA, &count);

    assert_true(success);
    assert_int_equal(count, 0);
}

static void test_count_byte_single_byte_match_returns_one(void** state)
{
    BC_UNUSED(state);

    const unsigned char data[1] = {0x42};
    size_t count = 0;

    bool success = bc_core_count_byte(data, 1, 0x42, &count);

    assert_true(success);
    assert_int_equal(count, 1);
}

static void test_count_byte_single_byte_no_match_returns_zero(void** state)
{
    BC_UNUSED(state);

    const unsigned char data[1] = {0x42};
    size_t count = 99;

    bool success = bc_core_count_byte(data, 1, 0xFF, &count);

    assert_true(success);
    assert_int_equal(count, 0);
}

static void test_count_byte_in_scalar_tail_returns_correct_count(void** state)
{
    BC_UNUSED(state);

    /* 31 bytes, all scalar path */
    unsigned char data[31];
    bc_core_fill(data, sizeof(data), (unsigned char)0x00);
    data[5] = 0xAB;
    data[20] = 0xAB;
    size_t count = 0;

    bool success = bc_core_count_byte(data, 31, 0xAB, &count);

    assert_true(success);
    assert_int_equal(count, 2);
}

static void test_count_byte_in_32byte_block_returns_correct_count(void** state)
{
    BC_UNUSED(state);

    unsigned char data[32];
    bc_core_fill(data, sizeof(data), (unsigned char)0x00);
    data[0] = 0xCC;
    data[15] = 0xCC;
    data[31] = 0xCC;
    size_t count = 0;

    bool success = bc_core_count_byte(data, 32, 0xCC, &count);

    assert_true(success);
    assert_int_equal(count, 3);
}

static void test_count_byte_in_128byte_batch_returns_correct_count(void** state)
{
    BC_UNUSED(state);

    unsigned char data[128];
    bc_core_fill(data, sizeof(data), (unsigned char)0x00);
    data[0] = 0xDD;
    data[32] = 0xDD;
    data[64] = 0xDD;
    data[96] = 0xDD;
    size_t count = 0;

    bool success = bc_core_count_byte(data, 128, 0xDD, &count);

    assert_true(success);
    assert_int_equal(count, 4);
}

static void test_count_byte_zero_occurrences_in_large_buffer_returns_zero(void** state)
{
    BC_UNUSED(state);

    unsigned char data[256];
    bc_core_fill(data, sizeof(data), (unsigned char)0x01);
    size_t count = 99;

    bool success = bc_core_count_byte(data, 256, 0xFF, &count);

    assert_true(success);
    assert_int_equal(count, 0);
}

static void test_count_byte_forces_batch_iteration_limit_127(void** state)
{
    BC_UNUSED(state);

    /*
     * 255*128 + 1 = 32641 bytes.
     * batch_iterations for first outer iteration = 32641/128 = 255 > 127 => clamped to 127.
     * This exercises the "if (batch_iterations > 127)" branch.
     * Place a small number of targets to avoid accumulator byte-overflow.
     */
    size_t buffer_len = 255 * 128 + 1;
    unsigned char* data = malloc(buffer_len);
    assert_non_null(data);
    bc_core_fill(data, buffer_len, (unsigned char)0x00);
    data[0] = 0xEE;
    data[16000] = 0xEE;
    data[32000] = 0xEE;
    size_t count = 0;

    bool success = bc_core_count_byte(data, buffer_len, 0xEE, &count);

    free(data);
    assert_true(success);
    assert_int_equal(count, 3);
}

static void test_count_byte_target_value_zero_counted_correctly(void** state)
{
    BC_UNUSED(state);

    unsigned char data[128];
    bc_core_fill(data, sizeof(data), (unsigned char)0xFF);
    data[10] = 0x00;
    data[100] = 0x00;
    size_t count = 0;

    bool success = bc_core_count_byte(data, 128, 0x00, &count);

    assert_true(success);
    assert_int_equal(count, 2);
}

/* ===== bc_core_count_matching ===== */

static void test_count_matching_empty_buffer_returns_zero(void** state)
{
    BC_UNUSED(state);

    const unsigned char data[4] = {0x01, 0x02, 0x03, 0x04};
    unsigned char table[256];
    bc_core_zero(table, sizeof(table));
    table[0x01] = 1;
    size_t count = 99;

    bool success = bc_core_count_matching(data, 0, table, &count);

    assert_true(success);
    assert_int_equal(count, 0);
}

static void test_count_matching_table_with_low_high_nibble_bytes_pass1(void** state)
{
    BC_UNUSED(state);

    /* Bytes with high nibble < 8 use pass1. Use 0x1A (high nibble=1) and 0x7F (high nibble=7). */
    unsigned char data[32];
    bc_core_fill(data, sizeof(data), (unsigned char)0x00);
    data[3] = 0x1A;
    data[20] = 0x7F;
    unsigned char table[256];
    bc_core_zero(table, sizeof(table));
    table[0x1A] = 1;
    table[0x7F] = 1;
    size_t count = 0;

    bool success = bc_core_count_matching(data, 32, table, &count);

    assert_true(success);
    assert_int_equal(count, 2);
}

static void test_count_matching_table_with_high_nibble_pass2_bytes(void** state)
{
    BC_UNUSED(state);

    /* Bytes with high nibble >= 8 use pass2. Use 0x80 (high=8) and 0xFF (high=F). */
    unsigned char data[32];
    bc_core_fill(data, sizeof(data), (unsigned char)0x00);
    data[5] = 0x80;
    data[25] = 0xFF;
    unsigned char table[256];
    bc_core_zero(table, sizeof(table));
    table[0x80] = 1;
    table[0xFF] = 1;
    size_t count = 0;

    bool success = bc_core_count_matching(data, 32, table, &count);

    assert_true(success);
    assert_int_equal(count, 2);
}

static void test_count_matching_in_128byte_block_4_chunks(void** state)
{
    BC_UNUSED(state);

    /* 128 bytes => 4 chunks of 32 in the SIMD path */
    unsigned char data[128];
    bc_core_fill(data, sizeof(data), (unsigned char)0x00);
    data[10] = 0x41; /* pass1: high nibble 4 */
    data[42] = 0x41;
    data[74] = 0x41;
    data[106] = 0x41;
    unsigned char table[256];
    bc_core_zero(table, sizeof(table));
    table[0x41] = 1;
    size_t count = 0;

    bool success = bc_core_count_matching(data, 128, table, &count);

    assert_true(success);
    assert_int_equal(count, 4);
}

static void test_count_matching_in_32byte_block_returns_correct_count(void** state)
{
    BC_UNUSED(state);

    unsigned char data[32];
    bc_core_fill(data, sizeof(data), (unsigned char)0x00);
    data[0] = 0x61;  /* 'a' */
    data[16] = 0x62; /* 'b' */
    data[31] = 0x63; /* 'c' */
    unsigned char table[256];
    bc_core_zero(table, sizeof(table));
    table[0x61] = 1;
    table[0x62] = 1;
    table[0x63] = 1;
    size_t count = 0;

    bool success = bc_core_count_matching(data, 32, table, &count);

    assert_true(success);
    assert_int_equal(count, 3);
}

static void test_count_matching_in_scalar_tail_returns_correct_count(void** state)
{
    BC_UNUSED(state);

    /* 35 bytes => 32 SIMD + 3 scalar */
    unsigned char data[35];
    bc_core_fill(data, sizeof(data), (unsigned char)0x00);
    data[32] = 0xA5;
    data[33] = 0xA5;
    unsigned char table[256];
    bc_core_zero(table, sizeof(table));
    table[0xA5] = 1;
    size_t count = 0;

    bool success = bc_core_count_matching(data, 35, table, &count);

    assert_true(success);
    assert_int_equal(count, 2);
}

static void test_count_matching_zero_matches_returns_zero(void** state)
{
    BC_UNUSED(state);

    unsigned char data[64];
    bc_core_fill(data, sizeof(data), (unsigned char)0x01);
    unsigned char table[256];
    bc_core_zero(table, sizeof(table));
    table[0xFF] = 1;
    size_t count = 99;

    bool success = bc_core_count_matching(data, 64, table, &count);

    assert_true(success);
    assert_int_equal(count, 0);
}

static void test_count_matching_both_passes_mixed_in_same_buffer(void** state)
{
    BC_UNUSED(state);

    /* 0x3C: high nibble 3 < 8 => pass1; 0xC3: high nibble C >= 8 => pass2 */
    unsigned char data[32];
    bc_core_fill(data, sizeof(data), (unsigned char)0x00);
    data[4] = 0x3C;
    data[20] = 0xC3;
    unsigned char table[256];
    bc_core_zero(table, sizeof(table));
    table[0x3C] = 1;
    table[0xC3] = 1;
    size_t count = 0;

    bool success = bc_core_count_matching(data, 32, table, &count);

    assert_true(success);
    assert_int_equal(count, 2);
}

/* ===== bc_core_count_lines ===== */

static void test_count_lines_delegates_to_count_byte_newline(void** state)
{
    BC_UNUSED(state);

    const char* text = "line1\nline2\nline3\n";
    size_t count = 0;

    bool success = bc_core_count_lines(text, 18, &count);

    assert_true(success);
    assert_int_equal(count, 3);
}

static void test_count_lines_no_newline_returns_zero(void** state)
{
    BC_UNUSED(state);

    const char* text = "no newlines here";
    size_t count = 99;

    bool success = bc_core_count_lines(text, 16, &count);

    assert_true(success);
    assert_int_equal(count, 0);
}

/* ===== bc_core_count_lines_with_pattern ===== */

static void test_count_lines_with_pattern_empty_data_returns_zero(void** state)
{
    BC_UNUSED(state);

    const char* text = "hello\n";
    const char* pattern = "hello";
    size_t count = 99;

    bool success = bc_core_count_lines_with_pattern(text, 0, pattern, 5, &count);

    assert_true(success);
    assert_int_equal(count, 0);
}

static void test_count_lines_with_pattern_empty_pattern_returns_zero(void** state)
{
    BC_UNUSED(state);

    const char* text = "hello\n";
    const char* pattern = "hello";
    size_t count = 99;

    bool success = bc_core_count_lines_with_pattern(text, 6, pattern, 0, &count);

    assert_true(success);
    assert_int_equal(count, 0);
}

static void test_count_lines_with_pattern_line_contains_pattern_counted(void** state)
{
    BC_UNUSED(state);

    const char* text = "hello world\n";
    const char* pattern = "world";
    size_t count = 0;

    bool success = bc_core_count_lines_with_pattern(text, 12, pattern, 5, &count);

    assert_true(success);
    assert_int_equal(count, 1);
}

static void test_count_lines_with_pattern_line_without_pattern_not_counted(void** state)
{
    BC_UNUSED(state);

    const char* text = "hello world\nfoo bar\n";
    const char* pattern = "world";
    size_t count = 0;

    bool success = bc_core_count_lines_with_pattern(text, 20, pattern, 5, &count);

    assert_true(success);
    assert_int_equal(count, 1);
}

static void test_count_lines_with_pattern_last_line_no_newline_counted(void** state)
{
    BC_UNUSED(state);

    /* Last line has no trailing newline but contains pattern */
    const char* text = "foo\nhello";
    const char* pattern = "hello";
    size_t count = 0;

    bool success = bc_core_count_lines_with_pattern(text, 9, pattern, 5, &count);

    assert_true(success);
    assert_int_equal(count, 1);
}

static void test_count_lines_with_pattern_at_start_of_line(void** state)
{
    BC_UNUSED(state);

    const char* text = "pattern at start\nno match\n";
    const char* pattern = "pattern";
    size_t count = 0;

    bool success = bc_core_count_lines_with_pattern(text, 26, pattern, 7, &count);

    assert_true(success);
    assert_int_equal(count, 1);
}

static void test_count_lines_with_pattern_at_end_of_line(void** state)
{
    BC_UNUSED(state);

    const char* text = "the end pattern\nno match\n";
    const char* pattern = "pattern";
    size_t count = 0;

    bool success = bc_core_count_lines_with_pattern(text, 25, pattern, 7, &count);

    assert_true(success);
    assert_int_equal(count, 1);
}

static void test_count_lines_with_pattern_false_first_byte_match_on_line_boundary(void** state)
{
    BC_UNUSED(state);

    /*
     * Pattern "xy", text "x\ny\n".
     * 'x' at offset 0 is the first byte of the pattern, but offset 0 + 2 = 2 > len of line 1.
     * pattern_matches would check bytes[0..1] = "x\n" != "xy" => no match.
     * Line 1 ends at '\n' with no match. Line 2 "y\n" has no 'x' => no match.
     * Expected count = 0.
     */
    const char* text = "x\ny\n";
    const char* pattern = "xy";
    size_t count = 99;

    bool success = bc_core_count_lines_with_pattern(text, 4, pattern, 2, &count);

    assert_true(success);
    assert_int_equal(count, 0);
}

static void test_count_lines_with_pattern_multiple_lines_each_matching(void** state)
{
    BC_UNUSED(state);

    const char* text = "foo pattern\nbar pattern\nbaz no match\n";
    const char* pattern = "pattern";
    size_t count = 0;

    bool success = bc_core_count_lines_with_pattern(text, 37, pattern, 7, &count);

    assert_true(success);
    assert_int_equal(count, 2);
}

static void test_count_lines_with_pattern_simd_path_no_newline_no_candidate(void** state)
{
    BC_UNUSED(state);

    /*
     * SIMD branch 1: (newline_mask | candidate_mask) == 0 => skip.
     * Buffer > 32 bytes, no newlines and no first-byte-of-pattern in the
     * first 32-byte chunk. Use pattern starting with 'X', fill chunk with 'A'.
     */
    unsigned char data[64];
    bc_core_fill(data, sizeof(data), (unsigned char)'A');
    data[33] = 'X';
    data[34] = 'Y';
    data[35] = '\n';
    const char* pattern = "XY";
    size_t count = 0;

    bool success = bc_core_count_lines_with_pattern(data, 64, pattern, 2, &count);

    assert_true(success);
    assert_int_equal(count, 1);
}

static void test_count_lines_with_pattern_simd_path_newlines_no_candidates(void** state)
{
    BC_UNUSED(state);

    /*
     * SIMD branch 2: candidate_mask == 0 but newline_mask != 0.
     * First chunk has newlines but no first byte of pattern.
     * A line that had a prior match should be counted when the newline is seen.
     * We need current_line_has_match=true going into the chunk.
     * Craft: line1 has pattern but spans two chunks (match found before chunk boundary),
     * then chunk has only newlines.
     *
     * Simpler: make first chunk have pattern at byte 0..pattern_len, newline at byte 31.
     * Then second chunk has only newlines. current_line_has_match becomes true in scalar
     * prefix... but we need SIMD to trigger it.
     *
     * Build a buffer where:
     * - bytes 0..1 = "Xz" (match, pattern "Xz")
     * - bytes 2..31 = 'A' (no newline, no first byte 'X')
     * - bytes 32..63 = all '\n'  => candidate_mask=0, newline_mask=0xFFFFFFFF
     * The SIMD loop starts at offset=0. First chunk: candidate at 0, newlines=none in [0..31].
     * => branch 3 (newline_mask==0, candidates exist). current_line_has_match set to true.
     * Second chunk: candidate_mask=0, newlines present => branch 2 counts line.
     */
    unsigned char data[64];
    bc_core_fill(data, sizeof(data), (unsigned char)'A');
    data[0] = 'X';
    data[1] = 'z';
    bc_core_fill(data + 32, 32, (unsigned char)'\n');
    const char* pattern = "Xz";
    size_t count = 0;

    bool success = bc_core_count_lines_with_pattern(data, 64, pattern, 2, &count);

    assert_true(success);
    assert_int_equal(count, 1);
}

static void test_count_lines_with_pattern_simd_path_candidates_no_newlines(void** state)
{
    BC_UNUSED(state);

    /*
     * SIMD branch 3: newline_mask == 0, candidate_mask != 0.
     * First chunk has candidates (first byte of pattern) but no newlines.
     * Pattern "Xz", chunk[0]='X', chunk[1]='z', rest='A'.
     * Line ends in scalar tail after the SIMD window.
     */
    unsigned char data[64];
    bc_core_fill(data, sizeof(data), (unsigned char)'A');
    data[0] = 'X';
    data[1] = 'z';
    data[63] = '\n';
    const char* pattern = "Xz";
    size_t count = 0;

    bool success = bc_core_count_lines_with_pattern(data, 64, pattern, 2, &count);

    assert_true(success);
    assert_int_equal(count, 1);
}

static void test_count_lines_with_pattern_simd_path_interleaved_newlines_and_candidates(void** state)
{
    BC_UNUSED(state);

    /*
     * SIMD branch 4: both newline_mask and candidate_mask non-zero in same chunk.
     * Build: "hello Xz\nworld Xz\n............" (all in one 32-byte chunk, padded to 64).
     * Use pattern "Xz".
     * Line 1: "hello Xz" => match => count 1 at '\n' at byte 8.
     * Line 2: "world Xz" => match => count 2 at '\n' at byte 18.
     * Then filler to 64 bytes.
     */
    unsigned char data[64];
    bc_core_fill(data, sizeof(data), (unsigned char)'A');

    /* Build chunk so newlines and 'X' are both present */
    const char* line1 = "hello Xz\n"; /* len=9 */
    const char* line2 = "world Xz\n"; /* len=9 */
    bc_core_copy(data, line1, 9);
    bc_core_copy(data + 9, line2, 9);
    /* bytes 18..63 = 'A' (no 'X', no '\n') */

    const char* pattern = "Xz";
    size_t count = 0;

    bool success = bc_core_count_lines_with_pattern(data, 64, pattern, 2, &count);

    assert_true(success);
    assert_int_equal(count, 2);
}

static void test_count_lines_with_pattern_simd_newlines_only_chunk_counts_prior_match(void** state)
{
    BC_UNUSED(state);

    /*
     * Cover lines 206-208: candidate_mask==0 and current_line_has_match==true.
     * First chunk (offset=0..31): pattern found, no newline => current_line_has_match=true.
     * Second chunk (offset=32..63): all newlines, no candidate 'X'.
     * simd_limit must include both chunks: need len - pattern_len + 1 >= 64 => len >= 65.
     * Build: bytes 0..1 = "Xz" (match), bytes 2..31 = 'A', bytes 32..63 = '\n', bytes 64 = 'A'.
     */
    unsigned char data[65];
    bc_core_fill(data, sizeof(data), (unsigned char)'A');
    data[0] = 'X';
    data[1] = 'z';
    bc_core_fill(data + 32, 32, (unsigned char)'\n');
    data[64] = 'A';
    const char* pattern = "Xz";
    size_t count = 0;

    bool success = bc_core_count_lines_with_pattern(data, 65, pattern, 2, &count);

    assert_true(success);
    assert_int_equal(count, 1);
}

static void test_count_lines_with_pattern_simd_candidates_after_last_newline_in_chunk(void** state)
{
    BC_UNUSED(state);

    /*
     * Cover lines 257-264: after processing all newlines in a chunk, there are
     * remaining candidates (bits after the last newline bit) and current_line_has_match is false.
     * Build: in one 32-byte chunk, put newline at position 5, then 'X' at position 20.
     * simd_limit must cover this chunk: len - pattern_len + 1 >= 32 => len >= 33.
     * No match before the newline (no 'X' before bit 5). After newline, 'X' at bit 20 with "Xz".
     * Bytes: 0..4='A', 5='\n', 6..19='A', 20='X', 21='z', 22..63='A', then trailing newline.
     */
    unsigned char data[64];
    bc_core_fill(data, sizeof(data), (unsigned char)'A');
    data[5] = '\n';
    data[20] = 'X';
    data[21] = 'z';
    data[63] = '\n';
    const char* pattern = "Xz";
    size_t count = 0;

    bool success = bc_core_count_lines_with_pattern(data, 64, pattern, 2, &count);

    assert_true(success);
    assert_int_equal(count, 1);
}

static void test_count_lines_with_pattern_simd_newline_at_last_bit_no_shift_overflow(void** state)
{
    BC_UNUSED(state);

    /*
     * Regression (fuzzer finding): when the only newline in a SIMD chunk lands at bit 31,
     * the mask-clear step computed 1u << (newline_bit + 1) = 1u << 32 on a 32-bit unsigned,
     * which is undefined behaviour. Trigger it: candidate at bit 0, newline at bit 31,
     * both non-zero so the inner while loop runs and executes the clear.
     */
    unsigned char data[64];
    bc_core_fill(data, sizeof(data), (unsigned char)'A');
    data[0] = 'x';
    data[31] = '\n';
    const char* pattern = "x";
    size_t count = 0;

    bool success = bc_core_count_lines_with_pattern(data, sizeof(data), pattern, 1, &count);

    assert_true(success);
    assert_int_equal(count, 1);
}

static void test_count_lines_with_pattern_simd_no_newline_false_positive_then_real_match(void** state)
{
    BC_UNUSED(state);

    /*
     * Lines 216 + 220: inside `newline_mask == 0` block with `!current_line_has_match` true.
     * The while loop (line 216) iterates with candidate_mask != 0.
     * Line 220 false branch: first candidate is a false positive (first byte matches but
     * pattern does not). Second candidate is the real match.
     * pattern="Xyz" (len=3, first='X', last='z').
     * False positive at offset 0: data[0]='X', data[1]='Q', data[2]='z' (middle wrong).
     * Real match at offset 3: data[3]='X', data[4]='y', data[5]='z'.
     * Buffer len=35, simd_limit=33. Chunk 0 (offset=0): 32<=33. No newlines => newline_mask=0.
     * candidate_mask has bits 0 and 3 set ('X' at 0 and 3).
     * Iteration 1: candidate 0, pattern_matches false (line 220 false). Continue.
     * Iteration 2: candidate 3, pattern_matches true. Break.
     */
    unsigned char data[35];
    bc_core_fill(data, sizeof(data), (unsigned char)'A');
    data[0] = 'X';
    data[1] = 'Q';
    data[2] = 'z';
    data[3] = 'X';
    data[4] = 'y';
    data[5] = 'z';
    data[34] = '\n';
    const char* pattern = "Xyz";
    size_t count = 0;

    bool success = bc_core_count_lines_with_pattern(data, 35, pattern, 3, &count);

    assert_true(success);
    assert_int_equal(count, 1);
}

static void test_count_lines_with_pattern_simd_no_newline_all_false_positives_exhausted(void** state)
{
    BC_UNUSED(state);

    /*
     * Line 216: while loop exits via condition false (candidate_mask becomes 0 without break).
     * Line 220: false branch on every iteration (all candidates are false positives).
     * pattern="Xyz" (len=3, first='X', last='z').
     * Two false positives in the chunk: data[0]='X','Q','z' and data[3]='X','Q','z'.
     * No real match. The while loop runs twice, each time line 220 is false, no break.
     * After 2 iterations candidate_mask=0 => while condition false => line 216 exit covered.
     * Buffer len=35, simd_limit=33. Chunk 0 (0..31): no newlines.
     */
    unsigned char data[35];
    bc_core_fill(data, sizeof(data), (unsigned char)'A');
    data[0] = 'X';
    data[1] = 'Q';
    data[2] = 'z';
    data[3] = 'X';
    data[4] = 'Q';
    data[5] = 'z';
    const char* pattern = "Xyz";
    size_t count = 0;

    bool success = bc_core_count_lines_with_pattern(data, 35, pattern, 3, &count);

    assert_true(success);
    assert_int_equal(count, 0);
}

static void test_count_lines_with_pattern_simd_interleaved_prior_match_skips_search(void** state)
{
    BC_UNUSED(state);

    /*
     * Line 235: false branch via first condition (`!current_line_has_match` is false).
     * current_line_has_match is already true when a chunk with both newlines and candidates
     * is processed. The `!current_line_has_match && candidates_before_newline != 0` condition
     * short-circuits to false because current_line_has_match is already true.
     * Build:
     * - Chunk 1 (0..31): 'X' at 0, 'y' at 1, 'z' at 2, no newlines => match found.
     * - Chunk 2 (32..63): newline at position 5 (byte 37), 'X' at position 2 (byte 34).
     *   candidate before newline: bit 2 (byte 34='X'). current_line_has_match=true.
     *   => condition 235 false via first member.
     * simd_limit = 66 - 3 + 1 = 64. Chunk 1: 0+32=32<=64. Chunk 2: 32+32=64<=64. OK.
     * pattern="Xyz" (len=3).
     */
    unsigned char data[66];
    bc_core_fill(data, sizeof(data), (unsigned char)'A');
    data[0] = 'X';
    data[1] = 'y';
    data[2] = 'z';
    data[34] = 'X';
    data[37] = '\n';
    data[65] = '\n';
    const char* pattern = "Xyz";
    size_t count = 0;

    bool success = bc_core_count_lines_with_pattern(data, 66, pattern, 3, &count);

    assert_true(success);
    assert_int_equal(count, 1);
}

static void test_count_lines_with_pattern_simd_interleaved_false_positive_before_newline(void** state)
{
    BC_UNUSED(state);

    /*
     * Lines 236 + 240: while loop inside `!current_line_has_match && candidates_before_newline != 0`.
     * candidates_before_newline != 0 but the candidate is a false positive (line 240 false branch).
     * pattern="Xyz" (len=3, first='X', last='z').
     * In a chunk with newline and candidates before the newline:
     * - 'X' at bit 0 (position 0): false positive — data[0]='X', data[1]='Q', data[2]='z'.
     * - newline at bit 10 (position 10): no 'X' before bit 10 except the false positive at 0.
     * - candidates_before_newline has bit 0 set. Loop runs (line 236). Line 240 false: no match.
     * Line continues. After newline processing: current_line_has_match=false, count unchanged.
     * A real match later: data[32]='X', data[33]='y', data[34]='z', data[63]='\n'.
     * simd_limit = 64 - 3 + 1 = 62. Chunk 0 (0..31): 32<=62. Chunk 1 (32..63): 64>62. Chunk 1 in scalar.
     */
    unsigned char data[64];
    bc_core_fill(data, sizeof(data), (unsigned char)'A');
    data[0] = 'X';
    data[1] = 'Q';
    data[2] = 'z';
    data[10] = '\n';
    data[32] = 'X';
    data[33] = 'y';
    data[34] = 'z';
    data[63] = '\n';
    const char* pattern = "Xyz";
    size_t count = 0;

    bool success = bc_core_count_lines_with_pattern(data, 64, pattern, 3, &count);

    assert_true(success);
    assert_int_equal(count, 1);
}

static void test_count_lines_with_pattern_simd_after_newline_false_positive_no_match(void** state)
{
    BC_UNUSED(state);

    /*
     * Lines 256 + 257 + 261: after processing all newlines in a mixed chunk, remaining
     * candidates exist (line 256 true, 257 while runs) but all are false positives (261 false).
     * pattern="Xyz" (len=3, first='X', last='z').
     * Chunk with: newline at bit 5, 'X' at bit 20 (false positive: data[20]='X', data[21]='Q',
     * data[22]='z'), no real match in this chunk.
     * current_line_has_match=false going in. After newline: candidate_mask bit 20 remains.
     * Lines 256-261 execute: absolute_position=20, 20+3=23<=len, pattern_matches => false (data[21]='Q').
     * Line 261 false covered. current_line_has_match stays false. No trailing newline for this chunk.
     * simd_limit = 35 - 3 + 1 = 33. Chunk 0 (0..31): 32<=33. OK.
     */
    unsigned char data[35];
    bc_core_fill(data, sizeof(data), (unsigned char)'A');
    data[5] = '\n';
    data[20] = 'X';
    data[21] = 'Q';
    data[22] = 'z';
    const char* pattern = "Xyz";
    size_t count = 0;

    bool success = bc_core_count_lines_with_pattern(data, 35, pattern, 3, &count);

    assert_true(success);
    assert_int_equal(count, 0);
}

static void test_count_lines_with_pattern_pattern_longer_than_data_simd_limit_zero(void** state)
{
    BC_UNUSED(state);

    /*
     * Line 191: ternary false branch — pattern_len > len.
     * data="abc" (len=3), pattern="abcde" (pattern_len=5).
     * simd_limit = 0, SIMD loop skipped, scalar loop runs but
     * offset + pattern_len (0+5=5) > len (3) so no match.
     * Expected count = 0.
     */
    const char* data = "abc";
    const char* pattern = "abcde";
    size_t count = 99;

    bool success = bc_core_count_lines_with_pattern(data, 3, pattern, 5, &count);

    assert_true(success);
    assert_int_equal(count, 0);
}

static void test_count_lines_with_pattern_simd_newlines_no_candidates_no_prior_match(void** state)
{
    BC_UNUSED(state);

    /*
     * Line 206: false branch of `if (current_line_has_match)` when candidate_mask==0 and
     * newline_mask!=0 and current_line_has_match==false.
     * Build: first chunk (0..31) has newlines but no first byte of pattern 'X'.
     * current_line_has_match starts at false.
     * Result: the if block is not entered (false branch), count stays 0.
     * Add a second chunk so simd_limit covers at least 32 bytes.
     * data: bytes 0..31 = all '\n' (no 'X'), bytes 32..63 = 'A' (no 'X').
     * pattern = "Xz" (len=2). simd_limit = 64 - 2 + 1 = 63 >= 32. Loop runs.
     */
    unsigned char data[64];
    bc_core_fill(data, 32, (unsigned char)'\n');
    bc_core_fill(data + 32, 32, (unsigned char)'A');
    const char* pattern = "Xz";
    size_t count = 99;

    bool success = bc_core_count_lines_with_pattern(data, 64, pattern, 2, &count);

    assert_true(success);
    assert_int_equal(count, 0);
}

static void test_count_lines_with_pattern_simd_no_newline_already_matched_skips_search(void** state)
{
    BC_UNUSED(state);

    /*
     * Line 215: false branch of `if (!current_line_has_match)` when newline_mask==0
     * and current_line_has_match is already true.
     * Chunk 1 (0..31): 'X' at 0, 'z' at 1, rest 'A'. No newline. Sets current_line_has_match=true.
     * Chunk 2 (32..63): 'X' at 32, 'z' at 33, rest 'A'. No newline. current_line_has_match=true
     * already so `if (!current_line_has_match)` is NOT entered (false path covered).
     * simd_limit = len - 2 + 1. With len=66: simd_limit=65. Chunk 1: 0+32=32<=65. OK.
     * Chunk 2: 32+32=64<=65. OK. No newlines in bytes 0..63.
     * Trailing newline at 65 ensures the matched line is counted in the scalar tail.
     */
    unsigned char data[66];
    bc_core_fill(data, sizeof(data), (unsigned char)'A');
    data[0] = 'X';
    data[1] = 'z';
    data[32] = 'X';
    data[33] = 'z';
    data[65] = '\n';
    const char* pattern = "Xz";
    size_t count = 0;

    bool success = bc_core_count_lines_with_pattern(data, 66, pattern, 2, &count);

    assert_true(success);
    assert_int_equal(count, 1);
}

static void test_count_lines_with_pattern_scalar_first_byte_at_end_no_room_for_pattern(void** state)
{
    BC_UNUSED(state);

    /*
     * Line 277: scalar path condition — bytes[offset] == first_byte but
     * offset + pattern_len > len. The first byte of the pattern appears at
     * the very end of the buffer but there is not enough room for the full pattern.
     * data: 8 bytes of 'A' followed by 'X' (first byte of "Xz") at offset 8. len=9.
     * pattern_len=2: offset(8) + 2 = 10 > 9 => condition false, no match.
     * No newlines => count = 0 (last line has no match).
     * simd_limit = 9 - 2 + 1 = 8. SIMD loop needs offset+32 <= 8 which is false, skipped.
     * Scalar loop: offset=0..8. At offset=8: bytes[8]='X'==first_byte but 8+2=10>9 => false.
     */
    unsigned char data[9];
    bc_core_fill(data, sizeof(data), (unsigned char)'A');
    data[8] = 'X';
    const char* pattern = "Xz";
    size_t count = 99;

    bool success = bc_core_count_lines_with_pattern(data, 9, pattern, 2, &count);

    assert_true(success);
    assert_int_equal(count, 0);
}

static void test_count_lines_with_pattern_pattern_matched_multiple_times_on_same_line_counts_once(void** state)
{
    BC_UNUSED(state);

    const char* text = "foo foo foo\n";
    const char* pattern = "foo";
    size_t count = 0;

    bool success = bc_core_count_lines_with_pattern(text, 12, pattern, 3, &count);

    assert_true(success);
    assert_int_equal(count, 1);
}

/* ===== bc_core_count_words ===== */

static void test_count_words_empty_buffer_returns_zero(void** state)
{
    BC_UNUSED(state);

    bool in_word = false;
    size_t count = 99;

    bool success = bc_core_count_words("anything", 0, &in_word, &count);

    assert_true(success);
    assert_int_equal(count, 0);
    assert_false(in_word);
}

static void test_count_words_single_word_scalar_tail(void** state)
{
    BC_UNUSED(state);

    const char* text = "hello";
    bool in_word = false;
    size_t count = 0;

    bool success = bc_core_count_words(text, 5, &in_word, &count);

    assert_true(success);
    assert_int_equal(count, 1);
    assert_true(in_word);
}

static void test_count_words_multiple_words_separated_by_spaces(void** state)
{
    BC_UNUSED(state);

    const char* text = "one two three four";
    bool in_word = false;
    size_t count = 0;

    bool success = bc_core_count_words(text, 18, &in_word, &count);

    assert_true(success);
    assert_int_equal(count, 4);
    assert_true(in_word);
}

static void test_count_words_leading_and_trailing_whitespace(void** state)
{
    BC_UNUSED(state);

    const char* text = "   word   ";
    bool in_word = false;
    size_t count = 0;

    bool success = bc_core_count_words(text, 10, &in_word, &count);

    assert_true(success);
    assert_int_equal(count, 1);
    assert_false(in_word);
}

static void test_count_words_only_whitespace_returns_zero(void** state)
{
    BC_UNUSED(state);

    const char* text = "   \t\n\r\v\f  ";
    bool in_word = false;
    size_t count = 99;

    bool success = bc_core_count_words(text, 10, &in_word, &count);

    assert_true(success);
    assert_int_equal(count, 0);
    assert_false(in_word);
}

static void test_count_words_all_whitespace_variants_separate_words(void** state)
{
    BC_UNUSED(state);

    const char* text = "a\tb\nc\vd\fe\rf g";
    bool in_word = false;
    size_t count = 0;

    bool success = bc_core_count_words(text, 13, &in_word, &count);

    assert_true(success);
    assert_int_equal(count, 7);
    assert_true(in_word);
}

static void test_count_words_simd_path_32_bytes_one_word(void** state)
{
    BC_UNUSED(state);

    unsigned char text[32];
    bc_core_fill(text, sizeof(text), (unsigned char)'x');
    bool in_word = false;
    size_t count = 0;

    bool success = bc_core_count_words(text, 32, &in_word, &count);

    assert_true(success);
    assert_int_equal(count, 1);
    assert_true(in_word);
}

static void test_count_words_simd_path_32_bytes_all_whitespace(void** state)
{
    BC_UNUSED(state);

    unsigned char text[32];
    bc_core_fill(text, sizeof(text), (unsigned char)' ');
    bool in_word = false;
    size_t count = 99;

    bool success = bc_core_count_words(text, 32, &in_word, &count);

    assert_true(success);
    assert_int_equal(count, 0);
    assert_false(in_word);
}

static void test_count_words_simd_path_64_bytes_alternating_words(void** state)
{
    BC_UNUSED(state);

    unsigned char text[64];
    for (size_t i = 0; i < 64; i++) {
        text[i] = (unsigned char)((i % 2 == 0) ? 'a' : ' ');
    }
    bool in_word = false;
    size_t count = 0;

    bool success = bc_core_count_words(text, 64, &in_word, &count);

    assert_true(success);
    assert_int_equal(count, 32);
    assert_false(in_word);
}

static void test_count_words_simd_tail_spans_vector_boundary(void** state)
{
    BC_UNUSED(state);

    unsigned char text[40];
    bc_core_fill(text, sizeof(text), (unsigned char)' ');
    text[35] = 'z';
    text[36] = 'z';
    bool in_word = false;
    size_t count = 0;

    bool success = bc_core_count_words(text, 40, &in_word, &count);

    assert_true(success);
    assert_int_equal(count, 1);
    assert_false(in_word);
}

static void test_count_words_incremental_state_carries_across_calls(void** state)
{
    BC_UNUSED(state);

    bool in_word = false;
    size_t count_first = 0;
    size_t count_second = 0;

    bool success_first = bc_core_count_words("hel", 3, &in_word, &count_first);
    bool success_second = bc_core_count_words("lo world", 8, &in_word, &count_second);

    assert_true(success_first);
    assert_true(success_second);
    assert_int_equal(count_first, 1);
    assert_int_equal(count_second, 1);
    assert_true(in_word);
}

static void test_count_words_control_byte_below_tab_counts_as_non_whitespace(void** state)
{
    BC_UNUSED(state);

    const unsigned char text[5] = {0x01, 0x02, 0x03, 0x04, 0x05};
    bool in_word = false;
    size_t count = 0;

    bool success = bc_core_count_words(text, 5, &in_word, &count);

    assert_true(success);
    assert_int_equal(count, 1);
    assert_true(in_word);
}

static void test_count_words_carry_word_end_at_32_byte_boundary(void** state)
{
    BC_UNUSED(state);

    unsigned char text[64];
    bc_core_fill(text, sizeof(text), (unsigned char)'a');
    text[32] = ' ';
    text[33] = 'b';
    bool in_word = false;
    size_t count = 0;

    bool success = bc_core_count_words(text, 64, &in_word, &count);

    assert_true(success);
    assert_int_equal(count, 2);
    assert_true(in_word);
}

/* ===== Unicode whitespace count_words tests ===== */

static void test_count_words_unicode_nbsp_c2a0(void** state)
{
    BC_UNUSED(state);
    const char text[] = "hello\xC2\xA0world";
    size_t count = 0;
    bool in_word = false;
    assert_true(bc_core_count_words(text, sizeof(text) - 1, &in_word, &count));
    assert_int_equal(count, 2);
}

static void test_count_words_unicode_nel_c285(void** state)
{
    BC_UNUSED(state);
    const char text[] = "hello\xC2\x85world";
    size_t count = 0;
    bool in_word = false;
    assert_true(bc_core_count_words(text, sizeof(text) - 1, &in_word, &count));
    assert_int_equal(count, 2);
}

static void test_count_words_unicode_em_space_e28083(void** state)
{
    BC_UNUSED(state);
    const char text[] = "hello\xE2\x80\x83world";
    size_t count = 0;
    bool in_word = false;
    assert_true(bc_core_count_words(text, sizeof(text) - 1, &in_word, &count));
    assert_int_equal(count, 2);
}

static void test_count_words_unicode_ideographic_space_e38080(void** state)
{
    BC_UNUSED(state);
    const char text[] = "hello\xE3\x80\x80world";
    size_t count = 0;
    bool in_word = false;
    assert_true(bc_core_count_words(text, sizeof(text) - 1, &in_word, &count));
    assert_int_equal(count, 2);
}

static void test_count_words_unicode_ogham_e19a80(void** state)
{
    BC_UNUSED(state);
    const char text[] = "hello\xE1\x9A\x80world";
    size_t count = 0;
    bool in_word = false;
    assert_true(bc_core_count_words(text, sizeof(text) - 1, &in_word, &count));
    assert_int_equal(count, 2);
}

static void test_count_words_unicode_line_separator_e280a8(void** state)
{
    BC_UNUSED(state);
    const char text[] = "hello\xE2\x80\xA8world";
    size_t count = 0;
    bool in_word = false;
    assert_true(bc_core_count_words(text, sizeof(text) - 1, &in_word, &count));
    assert_int_equal(count, 2);
}

static void test_count_words_unicode_narrow_nbsp_e280af(void** state)
{
    BC_UNUSED(state);
    const char text[] = "hello\xE2\x80\xAFworld";
    size_t count = 0;
    bool in_word = false;
    assert_true(bc_core_count_words(text, sizeof(text) - 1, &in_word, &count));
    assert_int_equal(count, 2);
}

static void test_count_words_unicode_medium_math_space_e2819f(void** state)
{
    BC_UNUSED(state);
    const char text[] = "hello\xE2\x81\x9Fworld";
    size_t count = 0;
    bool in_word = false;
    assert_true(bc_core_count_words(text, sizeof(text) - 1, &in_word, &count));
    assert_int_equal(count, 2);
}

static void test_count_words_unicode_all_2byte_ws_only(void** state)
{
    BC_UNUSED(state);
    const char text[] = "\xC2\x85\xC2\xA0\xC2\x85";
    size_t count = 0;
    bool in_word = false;
    assert_true(bc_core_count_words(text, sizeof(text) - 1, &in_word, &count));
    assert_int_equal(count, 0);
}

static void test_count_words_unicode_mixed_ascii_and_multibyte(void** state)
{
    BC_UNUSED(state);
    const char text[] = "a b\xC2\xA0"
                        "c\xE2\x80\x83"
                        "d";
    size_t count = 0;
    bool in_word = false;
    assert_true(bc_core_count_words(text, sizeof(text) - 1, &in_word, &count));
    assert_int_equal(count, 4);
}

static void test_count_words_unicode_partial_lead_at_end(void** state)
{
    BC_UNUSED(state);
    const char text[] = "hello\xC2";
    size_t count = 0;
    bool in_word = false;
    assert_true(bc_core_count_words(text, sizeof(text) - 1, &in_word, &count));
    assert_int_equal(count, 1);
    assert_true(in_word);
}

static void test_count_words_unicode_incomplete_3byte_at_end(void** state)
{
    BC_UNUSED(state);
    const char text[] = "hello\xE2\x80";
    size_t count = 0;
    bool in_word = false;
    assert_true(bc_core_count_words(text, sizeof(text) - 1, &in_word, &count));
    assert_int_equal(count, 1);
    assert_true(in_word);
}

static void test_count_words_unicode_non_ws_multibyte_not_treated_as_ws(void** state)
{
    BC_UNUSED(state);
    const char text[] = "hello\xC3\xA9world";
    size_t count = 0;
    bool in_word = false;
    assert_true(bc_core_count_words(text, sizeof(text) - 1, &in_word, &count));
    assert_int_equal(count, 1);
}

static void test_count_words_unicode_simd_boundary_2byte(void** state)
{
    BC_UNUSED(state);
    unsigned char text[64];
    bc_core_fill(text, 64, 'a');
    text[31] = 0xC2;
    text[32] = 0xA0;
    text[33] = 'b';
    size_t count = 0;
    bool in_word = false;
    assert_true(bc_core_count_words(text, 64, &in_word, &count));
    assert_int_equal(count, 2);
}

static void test_count_words_unicode_en_quad_range(void** state)
{
    BC_UNUSED(state);
    const char text[] = "a\xE2\x80\x80"
                        "b\xE2\x80\x8A"
                        "c";
    size_t count = 0;
    bool in_word = false;
    assert_true(bc_core_count_words(text, sizeof(text) - 1, &in_word, &count));
    assert_int_equal(count, 3);
}

static void test_count_words_unicode_pure_ascii_no_regression(void** state)
{
    BC_UNUSED(state);
    unsigned char text[256];
    bc_core_fill(text, sizeof(text), 'a');
    for (size_t i = 0; i < sizeof(text); i += 8) {
        text[i] = ' ';
    }
    size_t count = 0;
    bool in_word = false;
    assert_true(bc_core_count_words(text, sizeof(text), &in_word, &count));
    assert_int_equal(count, 32);
}

static void test_count_words_unicode_nbsp_at_simd_pos30_both_in_chunk(void** state)
{
    BC_UNUSED(state);

    unsigned char text[64];
    bc_core_fill(text, sizeof(text), (unsigned char)'a');
    text[30] = 0xC2;
    text[31] = 0xA0;
    text[32] = 'b';

    bool in_word = false;
    size_t count = 0;
    bool success = bc_core_count_words(text, 64, &in_word, &count);

    assert_true(success);
    assert_int_equal(count, 2);
}

static void test_count_words_unicode_3byte_ws_at_simd_pos29_all_in_chunk(void** state)
{
    BC_UNUSED(state);

    unsigned char text[64];
    bc_core_fill(text, sizeof(text), (unsigned char)'a');
    text[29] = 0xE2;
    text[30] = 0x80;
    text[31] = 0x83;

    bool in_word = false;
    size_t count = 0;
    bool success = bc_core_count_words(text, 64, &in_word, &count);

    assert_true(success);
    assert_int_equal(count, 2);
}

static void test_count_words_unicode_nbsp_at_simd_pos31_carry(void** state)
{
    BC_UNUSED(state);

    unsigned char text[34];
    bc_core_fill(text, sizeof(text), (unsigned char)'a');
    text[31] = 0xC2;
    text[32] = 0xA0;
    text[33] = 'b';

    bool in_word = false;
    size_t count = 0;
    bool success = bc_core_count_words(text, 34, &in_word, &count);

    assert_true(success);
    assert_int_equal(count, 2);
}

static void test_count_words_unicode_3byte_ws_at_simd_pos30_carry_third(void** state)
{
    BC_UNUSED(state);

    unsigned char text[64];
    bc_core_fill(text, sizeof(text), (unsigned char)'a');
    text[30] = 0xE2;
    text[31] = 0x80;
    text[32] = 0x83;

    bool in_word = false;
    size_t count = 0;
    bool success = bc_core_count_words(text, 64, &in_word, &count);

    assert_true(success);
    assert_int_equal(count, 2);
}

static void test_count_words_unicode_3byte_ws_at_simd_pos31_carry_two(void** state)
{
    BC_UNUSED(state);

    unsigned char text[35];
    bc_core_fill(text, sizeof(text), (unsigned char)'a');
    text[31] = 0xE2;
    text[32] = 0x80;
    text[33] = 0x83;
    text[34] = 'b';

    bool in_word = false;
    size_t count = 0;
    bool success = bc_core_count_words(text, 35, &in_word, &count);

    assert_true(success);
    assert_int_equal(count, 2);
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        /* count_byte */
        cmocka_unit_test(test_count_byte_empty_buffer_returns_zero),
        cmocka_unit_test(test_count_byte_single_byte_match_returns_one),
        cmocka_unit_test(test_count_byte_single_byte_no_match_returns_zero),
        cmocka_unit_test(test_count_byte_in_scalar_tail_returns_correct_count),
        cmocka_unit_test(test_count_byte_in_32byte_block_returns_correct_count),
        cmocka_unit_test(test_count_byte_in_128byte_batch_returns_correct_count),
        cmocka_unit_test(test_count_byte_zero_occurrences_in_large_buffer_returns_zero),
        cmocka_unit_test(test_count_byte_forces_batch_iteration_limit_127),
        cmocka_unit_test(test_count_byte_target_value_zero_counted_correctly),
        /* count_matching */
        cmocka_unit_test(test_count_matching_empty_buffer_returns_zero),
        cmocka_unit_test(test_count_matching_table_with_low_high_nibble_bytes_pass1),
        cmocka_unit_test(test_count_matching_table_with_high_nibble_pass2_bytes),
        cmocka_unit_test(test_count_matching_in_128byte_block_4_chunks),
        cmocka_unit_test(test_count_matching_in_32byte_block_returns_correct_count),
        cmocka_unit_test(test_count_matching_in_scalar_tail_returns_correct_count),
        cmocka_unit_test(test_count_matching_zero_matches_returns_zero),
        cmocka_unit_test(test_count_matching_both_passes_mixed_in_same_buffer),
        /* count_lines */
        cmocka_unit_test(test_count_lines_delegates_to_count_byte_newline),
        cmocka_unit_test(test_count_lines_no_newline_returns_zero),
        /* count_lines_with_pattern */
        cmocka_unit_test(test_count_lines_with_pattern_empty_data_returns_zero),
        cmocka_unit_test(test_count_lines_with_pattern_empty_pattern_returns_zero),
        cmocka_unit_test(test_count_lines_with_pattern_line_contains_pattern_counted),
        cmocka_unit_test(test_count_lines_with_pattern_line_without_pattern_not_counted),
        cmocka_unit_test(test_count_lines_with_pattern_last_line_no_newline_counted),
        cmocka_unit_test(test_count_lines_with_pattern_at_start_of_line),
        cmocka_unit_test(test_count_lines_with_pattern_at_end_of_line),
        cmocka_unit_test(test_count_lines_with_pattern_false_first_byte_match_on_line_boundary),
        cmocka_unit_test(test_count_lines_with_pattern_multiple_lines_each_matching),
        cmocka_unit_test(test_count_lines_with_pattern_simd_path_no_newline_no_candidate),
        cmocka_unit_test(test_count_lines_with_pattern_simd_path_newlines_no_candidates),
        cmocka_unit_test(test_count_lines_with_pattern_simd_path_candidates_no_newlines),
        cmocka_unit_test(test_count_lines_with_pattern_simd_path_interleaved_newlines_and_candidates),
        cmocka_unit_test(test_count_lines_with_pattern_simd_newlines_only_chunk_counts_prior_match),
        cmocka_unit_test(test_count_lines_with_pattern_simd_candidates_after_last_newline_in_chunk),
        cmocka_unit_test(test_count_lines_with_pattern_simd_newline_at_last_bit_no_shift_overflow),
        cmocka_unit_test(test_count_lines_with_pattern_pattern_matched_multiple_times_on_same_line_counts_once),
        cmocka_unit_test(test_count_lines_with_pattern_pattern_longer_than_data_simd_limit_zero),
        cmocka_unit_test(test_count_lines_with_pattern_simd_newlines_no_candidates_no_prior_match),
        cmocka_unit_test(test_count_lines_with_pattern_simd_no_newline_already_matched_skips_search),
        cmocka_unit_test(test_count_lines_with_pattern_scalar_first_byte_at_end_no_room_for_pattern),
        cmocka_unit_test(test_count_lines_with_pattern_simd_no_newline_false_positive_then_real_match),
        cmocka_unit_test(test_count_lines_with_pattern_simd_no_newline_all_false_positives_exhausted),
        cmocka_unit_test(test_count_lines_with_pattern_simd_interleaved_prior_match_skips_search),
        cmocka_unit_test(test_count_lines_with_pattern_simd_interleaved_false_positive_before_newline),
        cmocka_unit_test(test_count_lines_with_pattern_simd_after_newline_false_positive_no_match),
        /* count_words */
        cmocka_unit_test(test_count_words_empty_buffer_returns_zero),
        cmocka_unit_test(test_count_words_single_word_scalar_tail),
        cmocka_unit_test(test_count_words_multiple_words_separated_by_spaces),
        cmocka_unit_test(test_count_words_leading_and_trailing_whitespace),
        cmocka_unit_test(test_count_words_only_whitespace_returns_zero),
        cmocka_unit_test(test_count_words_all_whitespace_variants_separate_words),
        cmocka_unit_test(test_count_words_simd_path_32_bytes_one_word),
        cmocka_unit_test(test_count_words_simd_path_32_bytes_all_whitespace),
        cmocka_unit_test(test_count_words_simd_path_64_bytes_alternating_words),
        cmocka_unit_test(test_count_words_simd_tail_spans_vector_boundary),
        cmocka_unit_test(test_count_words_incremental_state_carries_across_calls),
        cmocka_unit_test(test_count_words_control_byte_below_tab_counts_as_non_whitespace),
        cmocka_unit_test(test_count_words_carry_word_end_at_32_byte_boundary),
        /* count_words Unicode */
        cmocka_unit_test(test_count_words_unicode_nbsp_c2a0),
        cmocka_unit_test(test_count_words_unicode_nel_c285),
        cmocka_unit_test(test_count_words_unicode_em_space_e28083),
        cmocka_unit_test(test_count_words_unicode_ideographic_space_e38080),
        cmocka_unit_test(test_count_words_unicode_ogham_e19a80),
        cmocka_unit_test(test_count_words_unicode_line_separator_e280a8),
        cmocka_unit_test(test_count_words_unicode_narrow_nbsp_e280af),
        cmocka_unit_test(test_count_words_unicode_medium_math_space_e2819f),
        cmocka_unit_test(test_count_words_unicode_all_2byte_ws_only),
        cmocka_unit_test(test_count_words_unicode_mixed_ascii_and_multibyte),
        cmocka_unit_test(test_count_words_unicode_partial_lead_at_end),
        cmocka_unit_test(test_count_words_unicode_incomplete_3byte_at_end),
        cmocka_unit_test(test_count_words_unicode_non_ws_multibyte_not_treated_as_ws),
        cmocka_unit_test(test_count_words_unicode_simd_boundary_2byte),
        cmocka_unit_test(test_count_words_unicode_en_quad_range),
        cmocka_unit_test(test_count_words_unicode_pure_ascii_no_regression),
        cmocka_unit_test(test_count_words_unicode_nbsp_at_simd_pos30_both_in_chunk),
        cmocka_unit_test(test_count_words_unicode_3byte_ws_at_simd_pos29_all_in_chunk),
        cmocka_unit_test(test_count_words_unicode_nbsp_at_simd_pos31_carry),
        cmocka_unit_test(test_count_words_unicode_3byte_ws_at_simd_pos30_carry_third),
        cmocka_unit_test(test_count_words_unicode_3byte_ws_at_simd_pos31_carry_two),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
