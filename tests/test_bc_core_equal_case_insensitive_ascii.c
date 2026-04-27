// SPDX-License-Identifier: MIT

#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <cmocka.h>

#include "bc_core.h"

#include <stdint.h>
#include <string.h>
#include <strings.h>

static void test_empty_strings_equal(void** state)
{
    BC_UNUSED(state);
    bool out = false;
    assert_true(bc_core_equal_case_insensitive_ascii("", 0, "", 0, &out));
    assert_true(out);
}

static void test_length_mismatch_not_equal(void** state)
{
    BC_UNUSED(state);
    bool out = true;
    assert_true(bc_core_equal_case_insensitive_ascii("abc", 3, "abcd", 4, &out));
    assert_false(out);
}

static void test_short_literals_equal_same_case(void** state)
{
    BC_UNUSED(state);
    bool out = false;
    assert_true(bc_core_equal_case_insensitive_ascii("true", 4, "true", 4, &out));
    assert_true(out);
}

static void test_short_literals_equal_mixed_case(void** state)
{
    BC_UNUSED(state);
    bool out = false;
    assert_true(bc_core_equal_case_insensitive_ascii("True", 4, "TRUE", 4, &out));
    assert_true(out);
    out = false;
    assert_true(bc_core_equal_case_insensitive_ascii("FaLsE", 5, "false", 5, &out));
    assert_true(out);
}

static void test_short_literals_not_equal(void** state)
{
    BC_UNUSED(state);
    bool out = true;
    assert_true(bc_core_equal_case_insensitive_ascii("true", 4, "TruF", 4, &out));
    assert_false(out);
}

static void test_at_symbol_not_equal_to_backtick(void** state)
{
    BC_UNUSED(state);
    /* '@' = 0x40, '`' = 0x60. They differ by 0x20 BUT neither is a letter.
       A naive (a | 0x20) == (b | 0x20) test would say equal — wrong. */
    bool out = true;
    assert_true(bc_core_equal_case_insensitive_ascii("@", 1, "`", 1, &out));
    assert_false(out);
}

static void test_open_bracket_not_equal_to_open_brace(void** state)
{
    BC_UNUSED(state);
    /* '[' = 0x5B, '{' = 0x7B. Differ by 0x20 but not letters. */
    bool out = true;
    assert_true(bc_core_equal_case_insensitive_ascii("[", 1, "{", 1, &out));
    assert_false(out);
}

static void test_digit_not_equal_to_letter(void** state)
{
    BC_UNUSED(state);
    /* '0' = 0x30, 'P' = 0x50. Differ by 0x20 but '0' is a digit. */
    bool out = true;
    assert_true(bc_core_equal_case_insensitive_ascii("0", 1, "P", 1, &out));
    assert_false(out);
}

static void test_high_bytes_distinguished(void** state)
{
    BC_UNUSED(state);
    unsigned char a[1] = {0xC0};
    unsigned char b[1] = {0xE0};
    bool out = true;
    assert_true(bc_core_equal_case_insensitive_ascii(a, 1, b, 1, &out));
    assert_false(out);
}

static void test_boundary_length_one(void** state)
{
    BC_UNUSED(state);
    bool out = false;
    assert_true(bc_core_equal_case_insensitive_ascii("a", 1, "A", 1, &out));
    assert_true(out);
    out = false;
    assert_true(bc_core_equal_case_insensitive_ascii("z", 1, "Z", 1, &out));
    assert_true(out);
}

static void test_boundary_length_31_simd_tail(void** state)
{
    BC_UNUSED(state);
    char a[31];
    char b[31];
    for (size_t i = 0; i < 31; i++) {
        a[i] = (char)('A' + (i % 26));
        b[i] = (char)('a' + (i % 26));
    }
    bool out = false;
    assert_true(bc_core_equal_case_insensitive_ascii(a, 31, b, 31, &out));
    assert_true(out);
}

static void test_boundary_length_32_full_simd_chunk(void** state)
{
    BC_UNUSED(state);
    char a[32];
    char b[32];
    for (size_t i = 0; i < 32; i++) {
        a[i] = (char)('A' + (i % 26));
        b[i] = (char)('a' + (i % 26));
    }
    bool out = false;
    assert_true(bc_core_equal_case_insensitive_ascii(a, 32, b, 32, &out));
    assert_true(out);
}

static void test_boundary_length_33_simd_plus_tail(void** state)
{
    BC_UNUSED(state);
    char a[33];
    char b[33];
    for (size_t i = 0; i < 33; i++) {
        a[i] = (char)('A' + (i % 26));
        b[i] = (char)('a' + (i % 26));
    }
    bool out = false;
    assert_true(bc_core_equal_case_insensitive_ascii(a, 33, b, 33, &out));
    assert_true(out);
}

static void test_boundary_length_127_simd_unrolled_tail(void** state)
{
    BC_UNUSED(state);
    char a[127];
    char b[127];
    for (size_t i = 0; i < 127; i++) {
        a[i] = (char)('A' + (i % 26));
        b[i] = (char)('a' + (i % 26));
    }
    bool out = false;
    assert_true(bc_core_equal_case_insensitive_ascii(a, 127, b, 127, &out));
    assert_true(out);
}

static void test_boundary_length_128_unrolled_chunk(void** state)
{
    BC_UNUSED(state);
    char a[128];
    char b[128];
    for (size_t i = 0; i < 128; i++) {
        a[i] = (char)('A' + (i % 26));
        b[i] = (char)('a' + (i % 26));
    }
    bool out = false;
    assert_true(bc_core_equal_case_insensitive_ascii(a, 128, b, 128, &out));
    assert_true(out);
}

static void test_mismatch_in_first_chunk(void** state)
{
    BC_UNUSED(state);
    char a[64];
    char b[64];
    memset(a, 'A', 64);
    memset(b, 'a', 64);
    b[5] = '@';
    bool out = true;
    assert_true(bc_core_equal_case_insensitive_ascii(a, 64, b, 64, &out));
    assert_false(out);
}

static void test_mismatch_in_unrolled_chunk(void** state)
{
    BC_UNUSED(state);
    char a[256];
    char b[256];
    memset(a, 'X', 256);
    memset(b, 'x', 256);
    b[200] = '#';
    bool out = true;
    assert_true(bc_core_equal_case_insensitive_ascii(a, 256, b, 256, &out));
    assert_false(out);
}

static void test_mismatch_in_scalar_tail(void** state)
{
    BC_UNUSED(state);
    char a[35];
    char b[35];
    memset(a, 'M', 35);
    memset(b, 'm', 35);
    b[34] = '?';
    bool out = true;
    assert_true(bc_core_equal_case_insensitive_ascii(a, 35, b, 35, &out));
    assert_false(out);
}

static void test_full_ascii_table_round_trip(void** state)
{
    BC_UNUSED(state);
    /* For every byte 0..255, compare with itself (must be equal) and
       with byte ^ 0x20 (equal iff both are ASCII letters). */
    for (int byte_value = 0; byte_value < 256; byte_value++) {
        unsigned char a = (unsigned char)byte_value;
        unsigned char b = (unsigned char)(byte_value ^ 0x20);
        bool out_self = false;
        assert_true(bc_core_equal_case_insensitive_ascii(&a, 1, &a, 1, &out_self));
        assert_true(out_self);

        bool out_pair = true;
        assert_true(bc_core_equal_case_insensitive_ascii(&a, 1, &b, 1, &out_pair));
        bool both_letters = ((a >= 'A' && a <= 'Z') || (a >= 'a' && a <= 'z'));
        assert_int_equal(out_pair ? 1 : 0, both_letters ? 1 : 0);
    }
}

static void test_invariant_matches_strncasecmp_ascii(void** state)
{
    BC_UNUSED(state);
    static const char* const samples[] = {
        "true",
        "True",
        "TRUE",
        "TrUe",
        "false",
        "FALSE",
        "0",
        "1",
        "yes",
        "no",
        "no_match",
        "abcdefghijklmnopqrstuvwxyz",
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ",
    };
    const size_t count = sizeof(samples) / sizeof(samples[0]);
    for (size_t i = 0; i < count; i++) {
        for (size_t j = 0; j < count; j++) {
            size_t len_i = strlen(samples[i]);
            size_t len_j = strlen(samples[j]);
            bool bc_out = false;
            assert_true(bc_core_equal_case_insensitive_ascii(samples[i], len_i, samples[j], len_j, &bc_out));
            bool reference = (len_i == len_j) && (strncasecmp(samples[i], samples[j], len_i) == 0);
            assert_int_equal(bc_out ? 1 : 0, reference ? 1 : 0);
        }
    }
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_empty_strings_equal),
        cmocka_unit_test(test_length_mismatch_not_equal),
        cmocka_unit_test(test_short_literals_equal_same_case),
        cmocka_unit_test(test_short_literals_equal_mixed_case),
        cmocka_unit_test(test_short_literals_not_equal),
        cmocka_unit_test(test_at_symbol_not_equal_to_backtick),
        cmocka_unit_test(test_open_bracket_not_equal_to_open_brace),
        cmocka_unit_test(test_digit_not_equal_to_letter),
        cmocka_unit_test(test_high_bytes_distinguished),
        cmocka_unit_test(test_boundary_length_one),
        cmocka_unit_test(test_boundary_length_31_simd_tail),
        cmocka_unit_test(test_boundary_length_32_full_simd_chunk),
        cmocka_unit_test(test_boundary_length_33_simd_plus_tail),
        cmocka_unit_test(test_boundary_length_127_simd_unrolled_tail),
        cmocka_unit_test(test_boundary_length_128_unrolled_chunk),
        cmocka_unit_test(test_mismatch_in_first_chunk),
        cmocka_unit_test(test_mismatch_in_unrolled_chunk),
        cmocka_unit_test(test_mismatch_in_scalar_tail),
        cmocka_unit_test(test_full_ascii_table_round_trip),
        cmocka_unit_test(test_invariant_matches_strncasecmp_ascii),
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}
