// SPDX-License-Identifier: MIT

#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <cmocka.h>

#include "bc_core.h"

#include <stdint.h>
#include <string.h>

static void test_base64_encode_rfc4648_vectors(void** state)
{
    BC_UNUSED(state);
    char buffer[64];
    size_t length = 0;

    assert_true(bc_core_format_base64_encode("", 0, buffer, sizeof(buffer), &length));
    assert_int_equal((int)length, 0);

    assert_true(bc_core_format_base64_encode("f", 1, buffer, sizeof(buffer), &length));
    assert_int_equal((int)length, 4);
    assert_memory_equal(buffer, "Zg==", 4);

    assert_true(bc_core_format_base64_encode("fo", 2, buffer, sizeof(buffer), &length));
    assert_int_equal((int)length, 4);
    assert_memory_equal(buffer, "Zm8=", 4);

    assert_true(bc_core_format_base64_encode("foo", 3, buffer, sizeof(buffer), &length));
    assert_int_equal((int)length, 4);
    assert_memory_equal(buffer, "Zm9v", 4);

    assert_true(bc_core_format_base64_encode("foob", 4, buffer, sizeof(buffer), &length));
    assert_int_equal((int)length, 8);
    assert_memory_equal(buffer, "Zm9vYg==", 8);

    assert_true(bc_core_format_base64_encode("fooba", 5, buffer, sizeof(buffer), &length));
    assert_int_equal((int)length, 8);
    assert_memory_equal(buffer, "Zm9vYmE=", 8);

    assert_true(bc_core_format_base64_encode("foobar", 6, buffer, sizeof(buffer), &length));
    assert_int_equal((int)length, 8);
    assert_memory_equal(buffer, "Zm9vYmFy", 8);
}

static void test_base64_decode_rfc4648_vectors(void** state)
{
    BC_UNUSED(state);
    uint8_t buffer[16];
    size_t length = 0;

    assert_true(bc_core_format_base64_decode("Zg==", 4, buffer, sizeof(buffer), &length));
    assert_int_equal((int)length, 1);
    assert_memory_equal(buffer, "f", 1);

    assert_true(bc_core_format_base64_decode("Zm8=", 4, buffer, sizeof(buffer), &length));
    assert_int_equal((int)length, 2);
    assert_memory_equal(buffer, "fo", 2);

    assert_true(bc_core_format_base64_decode("Zm9v", 4, buffer, sizeof(buffer), &length));
    assert_int_equal((int)length, 3);
    assert_memory_equal(buffer, "foo", 3);

    assert_true(bc_core_format_base64_decode("Zm9vYmFy", 8, buffer, sizeof(buffer), &length));
    assert_int_equal((int)length, 6);
    assert_memory_equal(buffer, "foobar", 6);

    assert_true(bc_core_format_base64_decode("", 0, buffer, sizeof(buffer), &length));
    assert_int_equal((int)length, 0);
}

static void test_base64_round_trip_binary(void** state)
{
    BC_UNUSED(state);
    uint8_t source[64];
    for (size_t index = 0; index < sizeof(source); index++) {
        source[index] = (uint8_t)(index * 7u + 3u);
    }
    char encoded[128];
    size_t encoded_length = 0;
    assert_true(bc_core_format_base64_encode(source, sizeof(source), encoded, sizeof(encoded), &encoded_length));

    uint8_t decoded[64];
    size_t decoded_length = 0;
    assert_true(bc_core_format_base64_decode(encoded, encoded_length, decoded, sizeof(decoded), &decoded_length));
    assert_int_equal((int)decoded_length, (int)sizeof(source));
    assert_memory_equal(decoded, source, sizeof(source));
}

static void test_base64_decode_rejects_invalid(void** state)
{
    BC_UNUSED(state);
    uint8_t buffer[16];
    size_t length = 0;
    assert_false(bc_core_format_base64_decode("Zm9v!", 5, buffer, sizeof(buffer), &length));
    assert_false(bc_core_format_base64_decode("@@@@", 4, buffer, sizeof(buffer), &length));
}

static void test_base64_encode_buffer_too_small(void** state)
{
    BC_UNUSED(state);
    char buffer[3];
    size_t length = 0;
    assert_false(bc_core_format_base64_encode("foo", 3, buffer, sizeof(buffer), &length));
}

static void test_base64_encoded_length_helper(void** state)
{
    BC_UNUSED(state);
    assert_int_equal((int)bc_core_format_base64_encoded_length(0), 0);
    assert_int_equal((int)bc_core_format_base64_encoded_length(1), 4);
    assert_int_equal((int)bc_core_format_base64_encoded_length(3), 4);
    assert_int_equal((int)bc_core_format_base64_encoded_length(6), 8);
    assert_int_equal((int)bc_core_format_base64_encoded_length(100), 136);
}

static void test_base32_encode_rfc4648_vectors(void** state)
{
    BC_UNUSED(state);
    char buffer[64];
    size_t length = 0;

    assert_true(bc_core_format_base32_encode("", 0, buffer, sizeof(buffer), &length));
    assert_int_equal((int)length, 0);

    assert_true(bc_core_format_base32_encode("f", 1, buffer, sizeof(buffer), &length));
    assert_int_equal((int)length, 8);
    assert_memory_equal(buffer, "MY======", 8);

    assert_true(bc_core_format_base32_encode("fo", 2, buffer, sizeof(buffer), &length));
    assert_int_equal((int)length, 8);
    assert_memory_equal(buffer, "MZXQ====", 8);

    assert_true(bc_core_format_base32_encode("foo", 3, buffer, sizeof(buffer), &length));
    assert_int_equal((int)length, 8);
    assert_memory_equal(buffer, "MZXW6===", 8);

    assert_true(bc_core_format_base32_encode("foob", 4, buffer, sizeof(buffer), &length));
    assert_int_equal((int)length, 8);
    assert_memory_equal(buffer, "MZXW6YQ=", 8);

    assert_true(bc_core_format_base32_encode("fooba", 5, buffer, sizeof(buffer), &length));
    assert_int_equal((int)length, 8);
    assert_memory_equal(buffer, "MZXW6YTB", 8);

    assert_true(bc_core_format_base32_encode("foobar", 6, buffer, sizeof(buffer), &length));
    assert_int_equal((int)length, 16);
    assert_memory_equal(buffer, "MZXW6YTBOI======", 16);
}

static void test_base32_decode_rfc4648_vectors(void** state)
{
    BC_UNUSED(state);
    uint8_t buffer[16];
    size_t length = 0;

    assert_true(bc_core_format_base32_decode("MY======", 8, buffer, sizeof(buffer), &length));
    assert_int_equal((int)length, 1);
    assert_memory_equal(buffer, "f", 1);

    assert_true(bc_core_format_base32_decode("MZXQ====", 8, buffer, sizeof(buffer), &length));
    assert_int_equal((int)length, 2);
    assert_memory_equal(buffer, "fo", 2);

    assert_true(bc_core_format_base32_decode("MZXW6YTBOI======", 16, buffer, sizeof(buffer), &length));
    assert_int_equal((int)length, 6);
    assert_memory_equal(buffer, "foobar", 6);

    assert_true(bc_core_format_base32_decode("", 0, buffer, sizeof(buffer), &length));
    assert_int_equal((int)length, 0);
}

static void test_base32_round_trip_random(void** state)
{
    BC_UNUSED(state);
    uint8_t source[100];
    for (size_t index = 0; index < sizeof(source); index++) {
        source[index] = (uint8_t)((index * 13u + 11u) ^ 0x55u);
    }
    char encoded[256];
    size_t encoded_length = 0;
    assert_true(bc_core_format_base32_encode(source, sizeof(source), encoded, sizeof(encoded), &encoded_length));

    uint8_t decoded[100];
    size_t decoded_length = 0;
    assert_true(bc_core_format_base32_decode(encoded, encoded_length, decoded, sizeof(decoded), &decoded_length));
    assert_int_equal((int)decoded_length, (int)sizeof(source));
    assert_memory_equal(decoded, source, sizeof(source));
}

static void test_base32_decode_rejects_invalid(void** state)
{
    BC_UNUSED(state);
    uint8_t buffer[16];
    size_t length = 0;
    assert_false(bc_core_format_base32_decode("MY======X", 9, buffer, sizeof(buffer), &length));
    assert_false(bc_core_format_base32_decode("11111111", 8, buffer, sizeof(buffer), &length));
    assert_false(bc_core_format_base32_decode("MY===", 5, buffer, sizeof(buffer), &length));
}

static void test_base32_decode_invalid_padding_count(void** state)
{
    BC_UNUSED(state);
    uint8_t buffer[16];
    size_t length = 0;
    assert_false(bc_core_format_base32_decode("MZXW6YT=", 8, buffer, sizeof(buffer), &length));
}

static void test_base32_encoded_length_helper(void** state)
{
    BC_UNUSED(state);
    assert_int_equal((int)bc_core_format_base32_encoded_length(0), 0);
    assert_int_equal((int)bc_core_format_base32_encoded_length(1), 8);
    assert_int_equal((int)bc_core_format_base32_encoded_length(5), 8);
    assert_int_equal((int)bc_core_format_base32_encoded_length(6), 16);
    assert_int_equal((int)bc_core_format_base32_encoded_length(10), 16);
}

static void test_hex_dump_default_options(void** state)
{
    BC_UNUSED(state);
    const char* sample = "Hello World!";
    char buffer[256];
    size_t length = 0;
    assert_true(bc_core_format_hex_dump(sample, 12, 0, NULL, buffer, sizeof(buffer), &length));
    assert_memory_equal(buffer, "00000000  ", 10);
    assert_memory_equal(buffer + 10, "48 65 6c 6c 6f 20 57 6f 72 6c 64 21 ", 36);
    assert_true(buffer[length - 15] == '|');
    assert_memory_equal(&buffer[length - 14], "Hello World!", 12);
    assert_true(buffer[length - 2] == '|');
    assert_true(buffer[length - 1] == '\n');
}

static void test_hex_dump_two_lines(void** state)
{
    BC_UNUSED(state);
    const char* sample = "0123456789abcdef0123456789abcdef";
    char buffer[512];
    size_t length = 0;
    assert_true(bc_core_format_hex_dump(sample, 32, 0, NULL, buffer, sizeof(buffer), &length));
    assert_true(length > 0);
    bool first_offset_seen = false;
    bool second_offset_seen = false;
    for (size_t index = 0; index < length - 8u; index++) {
        if (memcmp(&buffer[index], "00000000", 8) == 0) {
            first_offset_seen = true;
        }
        if (memcmp(&buffer[index], "00000010", 8) == 0) {
            second_offset_seen = true;
        }
    }
    assert_true(first_offset_seen);
    assert_true(second_offset_seen);
}

static void test_hex_dump_no_offset_no_ascii(void** state)
{
    BC_UNUSED(state);
    const bc_core_format_hex_dump_options_t options = {4u, false, false};
    char buffer[256];
    size_t length = 0;
    assert_true(bc_core_format_hex_dump("abcd", 4, 0, &options, buffer, sizeof(buffer), &length));
    assert_int_equal((int)length, 14);
    assert_memory_equal(buffer, "61 62 63 64  \n", 14);
}

static void test_hex_dump_with_base_offset(void** state)
{
    BC_UNUSED(state);
    char buffer[256];
    size_t length = 0;
    assert_true(bc_core_format_hex_dump("ab", 2, 0x1000u, NULL, buffer, sizeof(buffer), &length));
    assert_memory_equal(buffer, "00001000", 8);
}

static void test_hex_dump_partial_last_line_padded(void** state)
{
    BC_UNUSED(state);
    char buffer[256];
    size_t length = 0;
    assert_true(bc_core_format_hex_dump("abc", 3, 0, NULL, buffer, sizeof(buffer), &length));
    assert_true(length > 0);
    assert_true(buffer[length - 1] == '\n');
}

static void test_hex_dump_empty_input(void** state)
{
    BC_UNUSED(state);
    char buffer[64];
    size_t length = 99;
    assert_true(bc_core_format_hex_dump("", 0, 0, NULL, buffer, sizeof(buffer), &length));
    assert_int_equal((int)length, 0);
}

static void test_hex_dump_buffer_too_small(void** state)
{
    BC_UNUSED(state);
    char buffer[8];
    size_t length = 0;
    assert_false(bc_core_format_hex_dump("Hello World!", 12, 0, NULL, buffer, sizeof(buffer), &length));
}

static void test_hex_dump_zero_bytes_per_line_fails(void** state)
{
    BC_UNUSED(state);
    const bc_core_format_hex_dump_options_t options = {0u, true, true};
    char buffer[64];
    size_t length = 0;
    assert_false(bc_core_format_hex_dump("abc", 3, 0, &options, buffer, sizeof(buffer), &length));
}

static void test_hex_dump_required_capacity_helper(void** state)
{
    BC_UNUSED(state);
    assert_int_equal((int)bc_core_format_hex_dump_required_capacity(0, NULL), 0);
    const size_t small = bc_core_format_hex_dump_required_capacity(16, NULL);
    assert_true(small > 0);
    const size_t big = bc_core_format_hex_dump_required_capacity(1024, NULL);
    assert_true(big > small);
}

static void test_hex_dump_non_printable_replaced_with_dot(void** state)
{
    BC_UNUSED(state);
    const uint8_t input[] = {0x00u, 0x7Fu, 0xFFu, 'A'};
    char buffer[256];
    size_t length = 0;
    assert_true(bc_core_format_hex_dump(input, sizeof(input), 0, NULL, buffer, sizeof(buffer), &length));
    bool found_dots = false;
    for (size_t index = 0; index + 4 <= length; index++) {
        if (memcmp(&buffer[index], "...A", 4) == 0) {
            found_dots = true;
            break;
        }
    }
    assert_true(found_dots);
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_base64_encode_rfc4648_vectors),
        cmocka_unit_test(test_base64_decode_rfc4648_vectors),
        cmocka_unit_test(test_base64_round_trip_binary),
        cmocka_unit_test(test_base64_decode_rejects_invalid),
        cmocka_unit_test(test_base64_encode_buffer_too_small),
        cmocka_unit_test(test_base64_encoded_length_helper),
        cmocka_unit_test(test_base32_encode_rfc4648_vectors),
        cmocka_unit_test(test_base32_decode_rfc4648_vectors),
        cmocka_unit_test(test_base32_round_trip_random),
        cmocka_unit_test(test_base32_decode_rejects_invalid),
        cmocka_unit_test(test_base32_decode_invalid_padding_count),
        cmocka_unit_test(test_base32_encoded_length_helper),
        cmocka_unit_test(test_hex_dump_default_options),
        cmocka_unit_test(test_hex_dump_two_lines),
        cmocka_unit_test(test_hex_dump_no_offset_no_ascii),
        cmocka_unit_test(test_hex_dump_with_base_offset),
        cmocka_unit_test(test_hex_dump_partial_last_line_padded),
        cmocka_unit_test(test_hex_dump_empty_input),
        cmocka_unit_test(test_hex_dump_buffer_too_small),
        cmocka_unit_test(test_hex_dump_zero_bytes_per_line_fails),
        cmocka_unit_test(test_hex_dump_required_capacity_helper),
        cmocka_unit_test(test_hex_dump_non_printable_replaced_with_dot),
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}
