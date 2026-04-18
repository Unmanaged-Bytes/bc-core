// SPDX-License-Identifier: MIT

#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <cmocka.h>

#include "bc_core.h"

#include <string.h>

static void test_count_words_ascii_empty_returns_zero(void** state)
{
    BC_UNUSED(state);
    bool in_word = false;
    size_t count = 999;
    bool success = bc_core_count_words_ascii("anything", 0, &in_word, &count);
    assert_true(success);
    assert_int_equal(count, 0);
}

static void test_count_words_ascii_single_word_scalar_tail(void** state)
{
    BC_UNUSED(state);
    bool in_word = false;
    size_t count = 0;
    const char text[] = "hello";
    bool success = bc_core_count_words_ascii(text, strlen(text), &in_word, &count);
    assert_true(success);
    assert_int_equal(count, 1);
    assert_true(in_word);
}

static void test_count_words_ascii_multiple_words_scalar(void** state)
{
    BC_UNUSED(state);
    bool in_word = false;
    size_t count = 0;
    const char text[] = "one two three";
    bool success = bc_core_count_words_ascii(text, strlen(text), &in_word, &count);
    assert_true(success);
    assert_int_equal(count, 3);
}

static void test_count_words_ascii_leading_trailing_whitespace(void** state)
{
    BC_UNUSED(state);
    bool in_word = false;
    size_t count = 0;
    const char text[] = "  alpha beta  ";
    bool success = bc_core_count_words_ascii(text, strlen(text), &in_word, &count);
    assert_true(success);
    assert_int_equal(count, 2);
    assert_false(in_word);
}

static void test_count_words_ascii_all_whitespace_types(void** state)
{
    BC_UNUSED(state);
    bool in_word = false;
    size_t count = 0;
    const char text[] = "a\tb\nc\vd\fe\rf g";
    bool success = bc_core_count_words_ascii(text, strlen(text), &in_word, &count);
    assert_true(success);
    assert_int_equal(count, 7);
}

static void test_count_words_ascii_avx2_full_32_byte_block(void** state)
{
    BC_UNUSED(state);
    bool in_word = false;
    size_t count = 0;
    char text[64];
    memset(text, 'x', sizeof(text));
    text[10] = ' ';
    text[20] = ' ';
    text[30] = ' ';
    text[40] = ' ';
    text[50] = ' ';
    bool success = bc_core_count_words_ascii(text, sizeof(text), &in_word, &count);
    assert_true(success);
    assert_int_equal(count, 6);
}

static void test_count_words_ascii_crossing_block_boundary(void** state)
{
    BC_UNUSED(state);
    bool in_word = false;
    size_t count = 0;
    char text[96];
    memset(text, 'a', sizeof(text));
    text[31] = ' ';
    text[32] = ' ';
    text[63] = ' ';
    bool success = bc_core_count_words_ascii(text, sizeof(text), &in_word, &count);
    assert_true(success);
    assert_int_equal(count, 3);
}

static void test_count_words_ascii_resume_in_word(void** state)
{
    BC_UNUSED(state);
    bool in_word = true;
    size_t count = 0;
    const char text[] = "tail more";
    bool success = bc_core_count_words_ascii(text, strlen(text), &in_word, &count);
    assert_true(success);
    assert_int_equal(count, 1);
    assert_true(in_word);
}

static void test_count_words_ascii_only_whitespace(void** state)
{
    BC_UNUSED(state);
    bool in_word = false;
    size_t count = 123;
    const char text[] = "     \t\n\r  ";
    bool success = bc_core_count_words_ascii(text, strlen(text), &in_word, &count);
    assert_true(success);
    assert_int_equal(count, 0);
    assert_false(in_word);
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_count_words_ascii_empty_returns_zero),
        cmocka_unit_test(test_count_words_ascii_single_word_scalar_tail),
        cmocka_unit_test(test_count_words_ascii_multiple_words_scalar),
        cmocka_unit_test(test_count_words_ascii_leading_trailing_whitespace),
        cmocka_unit_test(test_count_words_ascii_all_whitespace_types),
        cmocka_unit_test(test_count_words_ascii_avx2_full_32_byte_block),
        cmocka_unit_test(test_count_words_ascii_crossing_block_boundary),
        cmocka_unit_test(test_count_words_ascii_resume_in_word),
        cmocka_unit_test(test_count_words_ascii_only_whitespace),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
