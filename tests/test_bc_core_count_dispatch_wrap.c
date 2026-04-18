// SPDX-License-Identifier: MIT

#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <cmocka.h>

#include "bc_core.h"
#include "test_bc_core_dispatch_mock.h"

#include <stdlib.h>

extern void bc_core_count_dispatch_init(void);

/* ===== count_byte avx2 ===== */

static void test_count_byte_avx2_len_0(void** state)
{
    BC_UNUSED(state);

    g_mock_detect_returns = true;
    bc_core_zero(&g_mock_features, sizeof(g_mock_features));

    bc_core_count_dispatch_init();

    unsigned char* buf = aligned_alloc(64, 64);
    assert_non_null(buf);
    bc_core_fill(buf, 64, (unsigned char)0x42);

    size_t count = 99;
    bool success = bc_core_count_byte(buf, 0, 0x42, &count);

    assert_true(success);
    assert_int_equal(count, 0);

    free(buf);
}

static void test_count_byte_avx2_none(void** state)
{
    BC_UNUSED(state);

    g_mock_detect_returns = true;
    bc_core_zero(&g_mock_features, sizeof(g_mock_features));

    bc_core_count_dispatch_init();

    size_t len = 256;
    unsigned char* buf = aligned_alloc(64, len);
    assert_non_null(buf);
    bc_core_fill(buf, len, (unsigned char)0x00);

    size_t count = 99;
    bool success = bc_core_count_byte(buf, len, 0xFF, &count);

    assert_true(success);
    assert_int_equal(count, 0);

    free(buf);
}

static void test_count_byte_avx2_all(void** state)
{
    BC_UNUSED(state);

    g_mock_detect_returns = true;
    bc_core_zero(&g_mock_features, sizeof(g_mock_features));

    bc_core_count_dispatch_init();

    size_t len = 256;
    unsigned char* buf = aligned_alloc(64, len);
    assert_non_null(buf);
    bc_core_fill(buf, len, (unsigned char)0x42);

    size_t count = 0;
    bool success = bc_core_count_byte(buf, len, 0x42, &count);

    assert_true(success);
    assert_int_equal(count, 256);

    free(buf);
}

static void test_count_byte_avx2_known_count(void** state)
{
    BC_UNUSED(state);

    g_mock_detect_returns = true;
    bc_core_zero(&g_mock_features, sizeof(g_mock_features));

    bc_core_count_dispatch_init();

    size_t len = 257;
    unsigned char* buf = aligned_alloc(64, 320);
    assert_non_null(buf);
    bc_core_fill(buf, 320, (unsigned char)0x00);

    const size_t known_offsets[] = {0, 15, 63, 64, 65, 127, 128, 200, 255, 256};
    for (size_t i = 0; i < 10; i++) {
        buf[known_offsets[i]] = 0xAB;
    }

    size_t count = 0;
    bool success = bc_core_count_byte(buf, len, 0xAB, &count);

    assert_true(success);
    assert_int_equal(count, 10);

    free(buf);
}

static void test_count_byte_avx2_single(void** state)
{
    BC_UNUSED(state);

    g_mock_detect_returns = true;
    bc_core_zero(&g_mock_features, sizeof(g_mock_features));

    bc_core_count_dispatch_init();

    size_t len = 64;
    unsigned char* buf = aligned_alloc(64, len);
    assert_non_null(buf);
    bc_core_fill(buf, len, (unsigned char)0x00);
    buf[33] = 0xCD;

    size_t count = 0;
    bool success = bc_core_count_byte(buf, len, 0xCD, &count);

    assert_true(success);
    assert_int_equal(count, 1);

    free(buf);
}

/* ===== count_matching avx2 ===== */

static void test_count_matching_avx2_len_0(void** state)
{
    BC_UNUSED(state);

    g_mock_detect_returns = true;
    bc_core_zero(&g_mock_features, sizeof(g_mock_features));

    bc_core_count_dispatch_init();

    unsigned char table[256] = {0};
    table[0x42] = 1;

    unsigned char* buf = aligned_alloc(64, 64);
    assert_non_null(buf);
    bc_core_fill(buf, 64, (unsigned char)0x42);

    size_t count = 99;
    bool success = bc_core_count_matching(buf, 0, table, &count);

    assert_true(success);
    assert_int_equal(count, 0);

    free(buf);
}

static void test_count_matching_avx2_none_match(void** state)
{
    BC_UNUSED(state);

    g_mock_detect_returns = true;
    bc_core_zero(&g_mock_features, sizeof(g_mock_features));

    bc_core_count_dispatch_init();

    unsigned char table[256] = {0};

    size_t len = 256;
    unsigned char* buf = aligned_alloc(64, len);
    assert_non_null(buf);
    bc_core_fill(buf, len, (unsigned char)0xAA);

    size_t count = 99;
    bool success = bc_core_count_matching(buf, len, table, &count);

    assert_true(success);
    assert_int_equal(count, 0);

    free(buf);
}

static void test_count_matching_avx2_all_match(void** state)
{
    BC_UNUSED(state);

    g_mock_detect_returns = true;
    bc_core_zero(&g_mock_features, sizeof(g_mock_features));

    bc_core_count_dispatch_init();

    unsigned char table[256];
    bc_core_fill(table, sizeof(table), (unsigned char)1);

    size_t len = 256;
    unsigned char* buf = aligned_alloc(64, len);
    assert_non_null(buf);
    bc_core_fill(buf, len, (unsigned char)0x00);
    for (size_t i = 0; i < len; i++) {
        buf[i] = (unsigned char)i;
    }

    size_t count = 0;
    bool success = bc_core_count_matching(buf, len, table, &count);

    assert_true(success);
    assert_int_equal(count, len);

    free(buf);
}

static void test_count_matching_avx2_whitespace(void** state)
{
    BC_UNUSED(state);

    g_mock_detect_returns = true;
    bc_core_zero(&g_mock_features, sizeof(g_mock_features));

    bc_core_count_dispatch_init();

    unsigned char table[256] = {0};
    table[' '] = 1;
    table['\t'] = 1;
    table['\n'] = 1;

    size_t len = 256;
    unsigned char* buf = aligned_alloc(64, len);
    assert_non_null(buf);
    bc_core_fill(buf, len, (unsigned char)'x');

    buf[0] = ' ';
    buf[31] = '\t';
    buf[63] = '\n';
    buf[64] = ' ';
    buf[128] = '\t';
    buf[200] = '\n';
    buf[255] = ' ';

    size_t count = 0;
    bool success = bc_core_count_matching(buf, len, table, &count);

    assert_true(success);
    assert_int_equal(count, 7);

    free(buf);
}

/* ===== count_lines_with_pattern avx2 ===== */

static void test_count_lines_with_pattern_avx2_len_0(void** state)
{
    BC_UNUSED(state);

    g_mock_detect_returns = true;
    bc_core_zero(&g_mock_features, sizeof(g_mock_features));

    bc_core_count_dispatch_init();

    unsigned char* buf = aligned_alloc(64, 64);
    assert_non_null(buf);
    bc_core_fill(buf, 64, (unsigned char)'a');

    size_t count = 99;
    bool success = bc_core_count_lines_with_pattern(buf, 0, "foo", 3, &count);

    assert_true(success);
    assert_int_equal(count, 0);

    free(buf);
}

static void test_count_lines_with_pattern_avx2_no_match(void** state)
{
    BC_UNUSED(state);

    g_mock_detect_returns = true;
    bc_core_zero(&g_mock_features, sizeof(g_mock_features));

    bc_core_count_dispatch_init();

    const char* text = "hello world\nbar baz\nqux quux\n";
    size_t len = 29;

    size_t count = 99;
    bool success = bc_core_count_lines_with_pattern(text, len, "foo", 3, &count);

    assert_true(success);
    assert_int_equal(count, 0);
}

static void test_count_lines_with_pattern_avx2_one_match(void** state)
{
    BC_UNUSED(state);

    g_mock_detect_returns = true;
    bc_core_zero(&g_mock_features, sizeof(g_mock_features));

    bc_core_count_dispatch_init();

    const char* text = "hello world\nfoo bar\n";
    size_t len = 20;

    size_t count = 0;
    bool success = bc_core_count_lines_with_pattern(text, len, "foo", 3, &count);

    assert_true(success);
    assert_int_equal(count, 1);
}

static void test_count_lines_with_pattern_avx2_multiple_per_line(void** state)
{
    BC_UNUSED(state);

    g_mock_detect_returns = true;
    bc_core_zero(&g_mock_features, sizeof(g_mock_features));

    bc_core_count_dispatch_init();

    const char* text = "foofoo\n";
    size_t len = 7;

    size_t count = 0;
    bool success = bc_core_count_lines_with_pattern(text, len, "foo", 3, &count);

    assert_true(success);
    assert_int_equal(count, 1);
}

static void test_count_lines_with_pattern_avx2_across_boundary(void** state)
{
    BC_UNUSED(state);

    g_mock_detect_returns = true;
    bc_core_zero(&g_mock_features, sizeof(g_mock_features));

    bc_core_count_dispatch_init();

    size_t len = 128;
    unsigned char* buf = aligned_alloc(64, len);
    assert_non_null(buf);
    bc_core_fill(buf, len, (unsigned char)'x');

    buf[10] = 'f';
    buf[11] = 'o';
    buf[12] = 'o';
    buf[30] = '\n';

    buf[60] = 'f';
    buf[61] = 'o';
    buf[62] = 'o';
    buf[65] = '\n';

    buf[90] = '\n';

    size_t count = 0;
    bool success = bc_core_count_lines_with_pattern(buf, len, "foo", 3, &count);

    assert_true(success);
    assert_int_equal(count, 2);

    free(buf);
}

static void test_count_byte_avx2_large_spread(void** state)
{
    BC_UNUSED(state);

    g_mock_detect_returns = true;
    bc_core_zero(&g_mock_features, sizeof(g_mock_features));

    bc_core_count_dispatch_init();

    size_t len = 512;
    unsigned char* buf = aligned_alloc(64, len);
    assert_non_null(buf);
    bc_core_fill(buf, len, (unsigned char)0x00);

    const size_t known_offsets[] = {0, 50, 128, 200, 256, 300, 400, 511};
    for (size_t i = 0; i < 8; i++) {
        buf[known_offsets[i]] = 0xAB;
    }

    size_t count = 0;
    bool success = bc_core_count_byte(buf, len, 0xAB, &count);

    assert_true(success);
    assert_int_equal(count, 8);

    free(buf);
}

/* ===== count_matching — large buffers ===== */

static void test_count_matching_avx2_large(void** state)
{
    BC_UNUSED(state);

    g_mock_detect_returns = true;
    bc_core_zero(&g_mock_features, sizeof(g_mock_features));

    bc_core_count_dispatch_init();

    unsigned char table[256] = {0};
    table[' '] = 1;
    table['\t'] = 1;
    table['\n'] = 1;

    size_t len = 512;
    unsigned char* buf = aligned_alloc(64, len);
    assert_non_null(buf);
    bc_core_fill(buf, len, (unsigned char)'x');

    buf[0] = ' ';
    buf[128] = '\t';
    buf[256] = '\n';
    buf[300] = ' ';
    buf[400] = '\t';
    buf[511] = '\n';

    size_t count = 0;
    bool success = bc_core_count_matching(buf, len, table, &count);

    assert_true(success);
    assert_int_equal(count, 6);

    free(buf);
}

/* ===== count_lines_with_pattern — large buffers ===== */

static void test_count_lines_with_pattern_avx2_large(void** state)
{
    BC_UNUSED(state);

    g_mock_detect_returns = true;
    bc_core_zero(&g_mock_features, sizeof(g_mock_features));

    bc_core_count_dispatch_init();

    size_t len = 512;
    unsigned char* buf = aligned_alloc(64, len);
    assert_non_null(buf);
    bc_core_fill(buf, len, (unsigned char)'x');

    buf[10] = 'f';
    buf[11] = 'o';
    buf[12] = 'o';
    buf[30] = '\n';

    buf[150] = 'f';
    buf[151] = 'o';
    buf[152] = 'o';
    buf[180] = '\n';

    buf[280] = '\n';

    buf[350] = 'f';
    buf[351] = 'o';
    buf[352] = 'o';
    buf[370] = '\n';

    buf[450] = '\n';

    size_t count = 0;
    bool success = bc_core_count_lines_with_pattern(buf, len, "foo", 3, &count);

    assert_true(success);
    assert_int_equal(count, 3);

    free(buf);
}

/* ===== count_byte avx2 — batch > 127 ===== */

static void test_count_byte_avx2_batch_over_127(void** state)
{
    BC_UNUSED(state);

    g_mock_detect_returns = true;
    bc_core_zero(&g_mock_features, sizeof(g_mock_features));

    bc_core_count_dispatch_init();

    size_t buffer_len = 128 * 128 + 1;
    unsigned char* buf = malloc(buffer_len);
    assert_non_null(buf);
    bc_core_fill(buf, buffer_len, (unsigned char)0x00);
    buf[0] = 0xEE;
    buf[16384] = 0xEE;

    size_t count = 0;
    bool success = bc_core_count_byte(buf, buffer_len, 0xEE, &count);

    free(buf);
    assert_true(success);
    assert_int_equal(count, 2);
}

/* ===== count_matching avx2 — 32B loop only (no 128B batch) ===== */

static void test_count_matching_avx2_64b_buffer(void** state)
{
    BC_UNUSED(state);

    g_mock_detect_returns = true;
    bc_core_zero(&g_mock_features, sizeof(g_mock_features));

    bc_core_count_dispatch_init();

    unsigned char table[256] = {0};
    table[0x41] = 1;

    size_t len = 64;
    unsigned char* buf = aligned_alloc(64, len);
    assert_non_null(buf);
    bc_core_fill(buf, len, (unsigned char)0x00);
    buf[10] = 0x41;
    buf[50] = 0x41;

    size_t count = 0;
    bool success = bc_core_count_matching(buf, len, table, &count);

    assert_true(success);
    assert_int_equal(count, 2);

    free(buf);
}

static void test_count_matching_avx2_scalar_only(void** state)
{
    BC_UNUSED(state);

    g_mock_detect_returns = true;
    bc_core_zero(&g_mock_features, sizeof(g_mock_features));

    bc_core_count_dispatch_init();

    unsigned char table[256] = {0};
    table[0x41] = 1;

    size_t len = 31;
    unsigned char* buf = aligned_alloc(64, 64);
    assert_non_null(buf);
    bc_core_fill(buf, 64, (unsigned char)0x00);
    buf[5] = 0x41;
    buf[20] = 0x41;

    size_t count = 0;
    bool success = bc_core_count_matching(buf, len, table, &count);

    assert_true(success);
    assert_int_equal(count, 2);

    free(buf);
}

/* ===== count_lines_with_pattern avx2 — candidates after last newline ===== */

static void test_count_lines_with_pattern_avx2_candidate_after_newline(void** state)
{
    BC_UNUSED(state);

    g_mock_detect_returns = true;
    bc_core_zero(&g_mock_features, sizeof(g_mock_features));

    bc_core_count_dispatch_init();

    unsigned char data[64];
    bc_core_fill(data, sizeof(data), (unsigned char)'A');
    data[5] = '\n';
    data[20] = 'f';
    data[21] = 'o';
    data[22] = 'o';
    data[63] = '\n';
    const char* pattern = "foo";

    size_t count = 0;
    bool success = bc_core_count_lines_with_pattern(data, 64, pattern, 3, &count);

    assert_true(success);
    assert_int_equal(count, 1);
}

static void test_count_lines_with_pattern_avx2_last_line_no_newline(void** state)
{
    BC_UNUSED(state);

    g_mock_detect_returns = true;
    bc_core_zero(&g_mock_features, sizeof(g_mock_features));

    bc_core_count_dispatch_init();

    const char* text = "no match\nhello foo";
    size_t len = 18;
    const char* pattern = "foo";

    size_t count = 0;
    bool success = bc_core_count_lines_with_pattern(text, len, pattern, 3, &count);

    assert_true(success);
    assert_int_equal(count, 1);
}

static void test_count_dispatch_detect_failure_avx2_fallback(void** state)
{
    BC_UNUSED(state);

    g_mock_detect_returns = false;
    bc_core_count_dispatch_init();

    unsigned char buf[64];
    bc_core_fill(buf, 64, (unsigned char)0x41);
    size_t count = 0;
    bool success = bc_core_count_byte(buf, 64, 0x41, &count);

    assert_true(success);
    assert_int_equal(count, 64);

    g_mock_detect_returns = true;
    bc_core_zero(&g_mock_features, sizeof(g_mock_features));
    bc_core_count_dispatch_init();
}

int main(void)
{
    const struct CMUnitTest tests[] = {cmocka_unit_test(test_count_byte_avx2_len_0),
                                       cmocka_unit_test(test_count_byte_avx2_none),
                                       cmocka_unit_test(test_count_byte_avx2_all),
                                       cmocka_unit_test(test_count_byte_avx2_known_count),
                                       cmocka_unit_test(test_count_byte_avx2_single),
                                       cmocka_unit_test(test_count_byte_avx2_large_spread),
                                       cmocka_unit_test(test_count_matching_avx2_len_0),
                                       cmocka_unit_test(test_count_matching_avx2_none_match),
                                       cmocka_unit_test(test_count_matching_avx2_all_match),
                                       cmocka_unit_test(test_count_matching_avx2_whitespace),
                                       cmocka_unit_test(test_count_matching_avx2_large),
                                       cmocka_unit_test(test_count_lines_with_pattern_avx2_len_0),
                                       cmocka_unit_test(test_count_lines_with_pattern_avx2_no_match),
                                       cmocka_unit_test(test_count_lines_with_pattern_avx2_one_match),
                                       cmocka_unit_test(test_count_lines_with_pattern_avx2_multiple_per_line),
                                       cmocka_unit_test(test_count_lines_with_pattern_avx2_across_boundary),
                                       cmocka_unit_test(test_count_lines_with_pattern_avx2_large),
                                       cmocka_unit_test(test_count_byte_avx2_batch_over_127),
                                       cmocka_unit_test(test_count_matching_avx2_64b_buffer),
                                       cmocka_unit_test(test_count_matching_avx2_scalar_only),
                                       cmocka_unit_test(test_count_lines_with_pattern_avx2_candidate_after_newline),
                                       cmocka_unit_test(test_count_lines_with_pattern_avx2_last_line_no_newline),
                                       cmocka_unit_test(test_count_dispatch_detect_failure_avx2_fallback)};

    return cmocka_run_group_tests(tests, NULL, NULL);
}
