// SPDX-License-Identifier: MIT

#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <cmocka.h>

#include "bc_core.h"

#include <string.h>

static void test_ascii_lowercase_empty(void** state)
{
    (void)state;
    unsigned char buffer[1] = {0xAAu};
    assert_true(bc_core_ascii_lowercase(buffer, 0));
    assert_int_equal(buffer[0], 0xAAu);
}

static void test_ascii_lowercase_all_upper(void** state)
{
    (void)state;
    char buffer[27] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    assert_true(bc_core_ascii_lowercase(buffer, 26));
    assert_string_equal(buffer, "abcdefghijklmnopqrstuvwxyz");
}

static void test_ascii_lowercase_mixed_with_non_ascii(void** state)
{
    (void)state;
    unsigned char buffer[] = {'H', 'e', 'L', 'L', 'O', ' ', 0xC3u, 0x89u, 'X', 0};
    assert_true(bc_core_ascii_lowercase(buffer, 9));
    assert_int_equal(buffer[0], 'h');
    assert_int_equal(buffer[1], 'e');
    assert_int_equal(buffer[2], 'l');
    assert_int_equal(buffer[3], 'l');
    assert_int_equal(buffer[4], 'o');
    assert_int_equal(buffer[5], ' ');
    assert_int_equal(buffer[6], 0xC3u);
    assert_int_equal(buffer[7], 0x89u);
    assert_int_equal(buffer[8], 'x');
}

static void test_ascii_lowercase_avx2_full_register(void** state)
{
    (void)state;
    char buffer[33] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ123456";
    assert_true(bc_core_ascii_lowercase(buffer, 32));
    assert_string_equal(buffer, "abcdefghijklmnopqrstuvwxyz123456");
}

static void test_ascii_lowercase_crosses_32_byte_boundary(void** state)
{
    (void)state;
    char buffer[65];
    memcpy(buffer, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789AB", 64);
    buffer[64] = '\0';
    assert_true(bc_core_ascii_lowercase(buffer, 64));
    assert_string_equal(buffer, "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz0123456789ab");
}

static void test_ascii_lowercase_leaves_non_letters_untouched(void** state)
{
    (void)state;
    unsigned char buffer[257];
    for (size_t index = 0; index < 256; ++index) {
        buffer[index] = (unsigned char)index;
    }
    buffer[256] = 0;
    assert_true(bc_core_ascii_lowercase(buffer, 256));
    for (size_t index = 0; index < 256; ++index) {
        unsigned char expected = (unsigned char)index;
        if (expected >= 'A' && expected <= 'Z') {
            expected = (unsigned char)(expected + ('a' - 'A'));
        }
        assert_int_equal(buffer[index], expected);
    }
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_ascii_lowercase_empty),
        cmocka_unit_test(test_ascii_lowercase_all_upper),
        cmocka_unit_test(test_ascii_lowercase_mixed_with_non_ascii),
        cmocka_unit_test(test_ascii_lowercase_avx2_full_register),
        cmocka_unit_test(test_ascii_lowercase_crosses_32_byte_boundary),
        cmocka_unit_test(test_ascii_lowercase_leaves_non_letters_untouched),
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}
