// SPDX-License-Identifier: MIT

#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <cmocka.h>

#include "bc_core.h"

#include <stdint.h>
#include <string.h>

static bool match_default(const char* pattern, const char* text)
{
    bool result = false;
    const bool ok = bc_core_glob_match(pattern, strlen(pattern), text, strlen(text), &result);
    assert_true(ok);
    return result;
}

static bool match_path(const char* pattern, const char* path)
{
    bool result = false;
    const bool ok = bc_core_glob_match_path(pattern, strlen(pattern), path, strlen(path), &result);
    assert_true(ok);
    return result;
}

static bool match_with(const char* pattern, const char* text, const bc_core_glob_options_t* options)
{
    bool result = false;
    const bool ok = bc_core_glob_match_with_options(pattern, strlen(pattern), text, strlen(text), options, &result);
    assert_true(ok);
    return result;
}

static void test_literal_exact_match(void** state)
{
    BC_UNUSED(state);
    assert_true(match_default("hello", "hello"));
    assert_false(match_default("hello", "Hello"));
    assert_false(match_default("hello", "hello!"));
    assert_false(match_default("hello", "hell"));
}

static void test_empty_pattern_empty_text(void** state)
{
    BC_UNUSED(state);
    assert_true(match_default("", ""));
}

static void test_empty_pattern_non_empty_text_fails(void** state)
{
    BC_UNUSED(state);
    assert_false(match_default("", "x"));
}

static void test_non_empty_pattern_empty_text_fails(void** state)
{
    BC_UNUSED(state);
    assert_false(match_default("x", ""));
}

static void test_star_matches_anything(void** state)
{
    BC_UNUSED(state);
    assert_true(match_default("*", ""));
    assert_true(match_default("*", "a"));
    assert_true(match_default("*", "hello world"));
}

static void test_star_prefix(void** state)
{
    BC_UNUSED(state);
    assert_true(match_default("*.c", "foo.c"));
    assert_true(match_default("*.c", ".c"));
    assert_false(match_default("*.c", "foo.h"));
}

static void test_star_suffix(void** state)
{
    BC_UNUSED(state);
    assert_true(match_default("test_*", "test_a"));
    assert_true(match_default("test_*", "test_"));
    assert_false(match_default("test_*", "test"));
}

static void test_star_middle(void** state)
{
    BC_UNUSED(state);
    assert_true(match_default("a*z", "az"));
    assert_true(match_default("a*z", "abz"));
    assert_true(match_default("a*z", "abcz"));
    assert_false(match_default("a*z", "ab"));
    assert_false(match_default("a*z", "az!"));
}

static void test_multiple_stars(void** state)
{
    BC_UNUSED(state);
    assert_true(match_default("*a*", "banana"));
    assert_true(match_default("a*b*c", "abc"));
    assert_true(match_default("a*b*c", "axxbyyc"));
    assert_false(match_default("a*b*c", "axxxx"));
}

static void test_question_mark_single_char(void** state)
{
    BC_UNUSED(state);
    assert_true(match_default("a?c", "abc"));
    assert_true(match_default("?", "x"));
    assert_false(match_default("?", ""));
    assert_false(match_default("?", "ab"));
    assert_false(match_default("a?c", "ac"));
}

static void test_question_marks_combined_with_star(void** state)
{
    BC_UNUSED(state);
    assert_true(match_default("?*?", "ab"));
    assert_true(match_default("?*?", "abc"));
    assert_false(match_default("?*?", "a"));
}

static void test_character_class_explicit(void** state)
{
    BC_UNUSED(state);
    assert_true(match_default("[abc]", "a"));
    assert_true(match_default("[abc]", "b"));
    assert_true(match_default("[abc]", "c"));
    assert_false(match_default("[abc]", "d"));
}

static void test_character_class_range(void** state)
{
    BC_UNUSED(state);
    assert_true(match_default("[a-z]", "m"));
    assert_true(match_default("[a-z]", "a"));
    assert_true(match_default("[a-z]", "z"));
    assert_false(match_default("[a-z]", "A"));
    assert_false(match_default("[0-9]", "a"));
    assert_true(match_default("[0-9]", "5"));
}

static void test_character_class_negation(void** state)
{
    BC_UNUSED(state);
    assert_false(match_default("[!abc]", "a"));
    assert_false(match_default("[!abc]", "b"));
    assert_true(match_default("[!abc]", "d"));
    assert_true(match_default("[!a-z]", "A"));
    assert_false(match_default("[!a-z]", "m"));
}

static void test_character_class_combined(void** state)
{
    BC_UNUSED(state);
    assert_true(match_default("[a-z0-9]", "k"));
    assert_true(match_default("[a-z0-9]", "7"));
    assert_false(match_default("[a-z0-9]", "K"));
}

static void test_character_class_in_pattern(void** state)
{
    BC_UNUSED(state);
    assert_true(match_default("file[0-9].txt", "file3.txt"));
    assert_false(match_default("file[0-9].txt", "filex.txt"));
}

static void test_character_class_unterminated(void** state)
{
    BC_UNUSED(state);
    assert_false(match_default("[abc", "a"));
    assert_false(match_default("[", "x"));
}

static void test_character_class_empty_brackets(void** state)
{
    BC_UNUSED(state);
    assert_false(match_default("[]", "x"));
}

static void test_escape_metachar(void** state)
{
    BC_UNUSED(state);
    assert_true(match_default("\\*", "*"));
    assert_false(match_default("\\*", "a"));
    assert_true(match_default("\\?", "?"));
    assert_true(match_default("a\\*b", "a*b"));
    assert_false(match_default("a\\*b", "axb"));
}

static void test_escape_disabled_via_options(void** state)
{
    BC_UNUSED(state);
    const bc_core_glob_options_t options = {false, false, false, false};
    assert_true(match_with("\\", "\\", &options));
    assert_false(match_with("\\*", "*", &options));
    assert_true(match_with("\\*", "\\anything", &options));
}

static void test_path_aware_star_does_not_cross_slash(void** state)
{
    BC_UNUSED(state);
    assert_false(match_path("*.c", "a/b.c"));
    assert_true(match_path("*.c", "foo.c"));
    assert_false(match_path("*", "a/b"));
    assert_true(match_path("*", "foo"));
}

static void test_path_aware_question_does_not_match_slash(void** state)
{
    BC_UNUSED(state);
    assert_false(match_path("a?b", "a/b"));
    assert_true(match_path("a?b", "axb"));
}

static void test_double_star_matches_zero_segments(void** state)
{
    BC_UNUSED(state);
    assert_true(match_path("**", ""));
    assert_true(match_path("**", "a"));
    assert_true(match_path("**", "a/b"));
    assert_true(match_path("**", "a/b/c/d"));
}

static void test_double_star_prefix(void** state)
{
    BC_UNUSED(state);
    assert_true(match_path("**/*.c", "foo.c"));
    assert_true(match_path("**/*.c", "a/foo.c"));
    assert_true(match_path("**/*.c", "a/b/foo.c"));
    assert_false(match_path("**/*.c", "a/foo.h"));
}

static void test_double_star_middle(void** state)
{
    BC_UNUSED(state);
    assert_true(match_path("src/**/test.c", "src/test.c"));
    assert_true(match_path("src/**/test.c", "src/foo/test.c"));
    assert_true(match_path("src/**/test.c", "src/foo/bar/test.c"));
    assert_false(match_path("src/**/test.c", "tests/test.c"));
}

static void test_path_default_options_no_double_star(void** state)
{
    BC_UNUSED(state);
    const bc_core_glob_options_t options = {false, false, false, true};
    assert_true(match_with("**", "anything/with/slashes", &options));
    assert_true(match_with("a*b", "a/b", &options));
}

static void test_case_insensitive_literals(void** state)
{
    BC_UNUSED(state);
    const bc_core_glob_options_t options = {true, false, false, true};
    assert_true(match_with("Hello", "hello", &options));
    assert_true(match_with("Hello", "HELLO", &options));
    assert_true(match_with("Hello", "HeLlO", &options));
    assert_false(match_with("Hello", "Helli", &options));
}

static void test_case_insensitive_class_range(void** state)
{
    BC_UNUSED(state);
    const bc_core_glob_options_t options = {true, false, false, true};
    assert_true(match_with("[a-z]", "M", &options));
    assert_true(match_with("[A-Z]", "m", &options));
    assert_true(match_with("[!a-z]", "1", &options));
    assert_false(match_with("[!a-z]", "M", &options));
}

static void test_case_insensitive_with_star(void** state)
{
    BC_UNUSED(state);
    const bc_core_glob_options_t options = {true, false, false, true};
    assert_true(match_with("*.C", "foo.c", &options));
    assert_true(match_with("*.c", "FOO.C", &options));
}

static void test_combined_path_case_double_star(void** state)
{
    BC_UNUSED(state);
    const bc_core_glob_options_t options = {true, true, true, true};
    assert_true(match_with("**/README.md", "src/sub/readme.MD", &options));
    assert_true(match_with("SRC/**/*.C", "src/test/main.c", &options));
    assert_false(match_with("**/foo.c", "bar.c", &options));
}

static void test_dot_is_literal(void** state)
{
    BC_UNUSED(state);
    assert_true(match_default(".gitignore", ".gitignore"));
    assert_false(match_default(".gitignore", "xgitignore"));
}

static void test_complex_pattern_path(void** state)
{
    BC_UNUSED(state);
    assert_true(match_path("test_*.c", "test_glob.c"));
    assert_false(match_path("test_*.c", "build/test_glob.c"));
    assert_true(match_path("**/test_*.c", "build/test_glob.c"));
}

static void test_only_slashes(void** state)
{
    BC_UNUSED(state);
    assert_true(match_path("/", "/"));
    assert_false(match_path("/", ""));
    assert_true(match_path("/*", "/foo"));
    assert_false(match_path("/*", "/a/b"));
}

static void test_star_after_double_star(void** state)
{
    BC_UNUSED(state);
    assert_true(match_path("**/*", "a"));
    assert_true(match_path("**/*", "a/b"));
    assert_true(match_path("**/*", "a/b/c"));
}

static void test_class_with_dash_first(void** state)
{
    BC_UNUSED(state);
    assert_true(match_default("[-a]", "-"));
    assert_true(match_default("[-a]", "a"));
    assert_false(match_default("[-a]", "b"));
}

static void test_class_negation_alone(void** state)
{
    BC_UNUSED(state);
    assert_false(match_default("[!]", "x"));
}

static void test_long_repeating_pattern_no_overflow(void** state)
{
    BC_UNUSED(state);
    assert_true(match_default("a*a*a*a*b", "aaaaab"));
    assert_false(match_default("a*a*a*a*b", "aaaaa"));
    assert_true(match_default("*a*a*a*a*b", "xxxaxxaxxxaxxxxab"));
}

static void test_pattern_with_only_question_marks(void** state)
{
    BC_UNUSED(state);
    assert_true(match_default("???", "abc"));
    assert_false(match_default("???", "ab"));
    assert_false(match_default("???", "abcd"));
}

static void test_options_path_aware_without_double_star(void** state)
{
    BC_UNUSED(state);
    const bc_core_glob_options_t options = {false, true, false, true};
    assert_false(match_with("**", "a/b", &options));
    assert_false(match_with("*", "a/b", &options));
    assert_true(match_with("a/b", "a/b", &options));
}

static void test_class_matching_slash_in_path_mode(void** state)
{
    BC_UNUSED(state);
    assert_true(match_path("a[/]b", "a/b"));
}

static void test_escape_inside_pattern(void** state)
{
    BC_UNUSED(state);
    assert_true(match_default("file\\[0\\].txt", "file[0].txt"));
    assert_false(match_default("file\\[0\\].txt", "file0.txt"));
}

static void test_star_at_end_after_match(void** state)
{
    BC_UNUSED(state);
    assert_true(match_default("hello*", "hello"));
    assert_true(match_default("hello*", "helloworld"));
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_literal_exact_match),
        cmocka_unit_test(test_empty_pattern_empty_text),
        cmocka_unit_test(test_empty_pattern_non_empty_text_fails),
        cmocka_unit_test(test_non_empty_pattern_empty_text_fails),
        cmocka_unit_test(test_star_matches_anything),
        cmocka_unit_test(test_star_prefix),
        cmocka_unit_test(test_star_suffix),
        cmocka_unit_test(test_star_middle),
        cmocka_unit_test(test_multiple_stars),
        cmocka_unit_test(test_question_mark_single_char),
        cmocka_unit_test(test_question_marks_combined_with_star),
        cmocka_unit_test(test_character_class_explicit),
        cmocka_unit_test(test_character_class_range),
        cmocka_unit_test(test_character_class_negation),
        cmocka_unit_test(test_character_class_combined),
        cmocka_unit_test(test_character_class_in_pattern),
        cmocka_unit_test(test_character_class_unterminated),
        cmocka_unit_test(test_character_class_empty_brackets),
        cmocka_unit_test(test_escape_metachar),
        cmocka_unit_test(test_escape_disabled_via_options),
        cmocka_unit_test(test_path_aware_star_does_not_cross_slash),
        cmocka_unit_test(test_path_aware_question_does_not_match_slash),
        cmocka_unit_test(test_double_star_matches_zero_segments),
        cmocka_unit_test(test_double_star_prefix),
        cmocka_unit_test(test_double_star_middle),
        cmocka_unit_test(test_path_default_options_no_double_star),
        cmocka_unit_test(test_case_insensitive_literals),
        cmocka_unit_test(test_case_insensitive_class_range),
        cmocka_unit_test(test_case_insensitive_with_star),
        cmocka_unit_test(test_combined_path_case_double_star),
        cmocka_unit_test(test_dot_is_literal),
        cmocka_unit_test(test_complex_pattern_path),
        cmocka_unit_test(test_only_slashes),
        cmocka_unit_test(test_star_after_double_star),
        cmocka_unit_test(test_class_with_dash_first),
        cmocka_unit_test(test_class_negation_alone),
        cmocka_unit_test(test_long_repeating_pattern_no_overflow),
        cmocka_unit_test(test_pattern_with_only_question_marks),
        cmocka_unit_test(test_options_path_aware_without_double_star),
        cmocka_unit_test(test_class_matching_slash_in_path_mode),
        cmocka_unit_test(test_escape_inside_pattern),
        cmocka_unit_test(test_star_at_end_after_match),
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}
