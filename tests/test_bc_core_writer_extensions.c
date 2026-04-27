// SPDX-License-Identifier: MIT

#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <cmocka.h>

#include "bc_core.h"

#include <fcntl.h>
#include <string.h>
#include <unistd.h>

static void test_init_standard_error_sets_stderr_fd(void** state)
{
    BC_UNUSED(state);

    bc_core_writer_t writer;
    char buffer[64];
    bool success = bc_core_writer_init_standard_error(&writer, buffer, sizeof(buffer));

    assert_true(success);
    assert_int_equal(writer.fd, STDERR_FILENO);
    assert_int_equal(writer.error_latched, 0);
    assert_int_equal(writer.position, 0);
    assert_int_equal(writer.capacity, sizeof(buffer));
}

static void test_init_standard_output_sets_stdout_fd(void** state)
{
    BC_UNUSED(state);

    bc_core_writer_t writer;
    char buffer[64];
    bool success = bc_core_writer_init_standard_output(&writer, buffer, sizeof(buffer));

    assert_true(success);
    assert_int_equal(writer.fd, STDOUT_FILENO);
}

static void test_init_buffer_only_sets_negative_fd(void** state)
{
    BC_UNUSED(state);

    bc_core_writer_t writer;
    char buffer[64];
    bool success = bc_core_writer_init_buffer_only(&writer, buffer, sizeof(buffer));

    assert_true(success);
    assert_int_equal(writer.fd, -1);
    assert_int_equal(writer.error_latched, 0);
    assert_int_equal(writer.position, 0);
    assert_int_equal(writer.capacity, sizeof(buffer));
}

static void test_init_buffer_only_rejects_null_buffer(void** state)
{
    BC_UNUSED(state);

    bc_core_writer_t writer;
    bool success = bc_core_writer_init_buffer_only(&writer, NULL, 64);

    assert_false(success);
}

static void test_init_buffer_only_rejects_zero_capacity(void** state)
{
    BC_UNUSED(state);

    bc_core_writer_t writer;
    char buffer[64];
    bool success = bc_core_writer_init_buffer_only(&writer, buffer, 0);

    assert_false(success);
}

static void test_buffer_only_accumulates_writes(void** state)
{
    BC_UNUSED(state);

    bc_core_writer_t writer;
    char buffer[64];
    assert_true(bc_core_writer_init_buffer_only(&writer, buffer, sizeof(buffer)));

    assert_true(bc_core_writer_write_bytes(&writer, "hello ", 6));
    assert_true(bc_core_writer_write_bytes(&writer, "world", 5));

    const char* data = NULL;
    size_t length = 0;
    assert_true(bc_core_writer_buffer_data(&writer, &data, &length));
    assert_int_equal(length, 11);
    assert_memory_equal(data, "hello world", 11);
}

static void test_buffer_only_flush_is_noop(void** state)
{
    BC_UNUSED(state);

    bc_core_writer_t writer;
    char buffer[64];
    assert_true(bc_core_writer_init_buffer_only(&writer, buffer, sizeof(buffer)));

    assert_true(bc_core_writer_write_bytes(&writer, "data", 4));
    assert_true(bc_core_writer_flush(&writer));

    const char* data = NULL;
    size_t length = 0;
    assert_true(bc_core_writer_buffer_data(&writer, &data, &length));
    assert_int_equal(length, 4);
    assert_memory_equal(data, "data", 4);
}

static void test_buffer_only_overflow_latches_error(void** state)
{
    BC_UNUSED(state);

    bc_core_writer_t writer;
    char buffer[8];
    assert_true(bc_core_writer_init_buffer_only(&writer, buffer, sizeof(buffer)));

    assert_true(bc_core_writer_write_bytes(&writer, "12345", 5));
    bool success = bc_core_writer_write_bytes(&writer, "67890", 5);

    assert_false(success);
    assert_true(bc_core_writer_has_error(&writer));
}

static void test_buffer_only_write_char_overflow(void** state)
{
    BC_UNUSED(state);

    bc_core_writer_t writer;
    char buffer[3];
    assert_true(bc_core_writer_init_buffer_only(&writer, buffer, sizeof(buffer)));

    assert_true(bc_core_writer_write_char(&writer, 'a'));
    assert_true(bc_core_writer_write_char(&writer, 'b'));
    assert_true(bc_core_writer_write_char(&writer, 'c'));
    bool success = bc_core_writer_write_char(&writer, 'd');

    assert_false(success);
    assert_true(bc_core_writer_has_error(&writer));
}

static void test_buffer_data_rejects_null(void** state)
{
    BC_UNUSED(state);

    bc_core_writer_t writer;
    char buffer[8];
    assert_true(bc_core_writer_init_buffer_only(&writer, buffer, sizeof(buffer)));

    size_t length = 0;
    assert_false(bc_core_writer_buffer_data(&writer, NULL, &length));

    const char* data = NULL;
    assert_false(bc_core_writer_buffer_data(&writer, &data, NULL));
}

static void test_write_error_description_round_trip(void** state)
{
    BC_UNUSED(state);

    bc_core_writer_t writer;
    char buffer[128];
    assert_true(bc_core_writer_init_buffer_only(&writer, buffer, sizeof(buffer)));

    assert_true(bc_core_writer_write_error_description(&writer, BC_CORE_ERROR_INVALID_ARGUMENT));

    const char* data = NULL;
    size_t length = 0;
    assert_true(bc_core_writer_buffer_data(&writer, &data, &length));
    assert_int_equal(length, strlen("invalid argument"));
    assert_memory_equal(data, "invalid argument", length);
}

static void test_write_error_description_unknown_code(void** state)
{
    BC_UNUSED(state);

    bc_core_writer_t writer;
    char buffer[128];
    assert_true(bc_core_writer_init_buffer_only(&writer, buffer, sizeof(buffer)));

    bool success = bc_core_writer_write_error_description(&writer, (bc_core_error_code_t)9999);

    assert_false(success);
    assert_true(bc_core_writer_has_error(&writer));
}

static void test_write_error_description_after_error_latched(void** state)
{
    BC_UNUSED(state);

    bc_core_writer_t writer;
    char buffer[8];
    assert_true(bc_core_writer_init_buffer_only(&writer, buffer, sizeof(buffer)));

    assert_false(bc_core_writer_write_bytes(&writer, "123456789", 9));
    assert_true(bc_core_writer_has_error(&writer));

    bool success = bc_core_writer_write_error_description(&writer, BC_CORE_ERROR_NONE);

    assert_false(success);
}

static void test_write_error_description_unknown_with_fd_writer(void** state)
{
    BC_UNUSED(state);
    bc_core_writer_t writer;
    char buffer[64];
    assert_true(bc_core_writer_init_standard_error(&writer, buffer, sizeof(buffer)));
    assert_false(bc_core_writer_write_error_description(&writer, (bc_core_error_code_t)9999));
    assert_true(bc_core_writer_has_error(&writer));
}

static void test_init_rejects_negative_fd(void** state)
{
    BC_UNUSED(state);
    bc_core_writer_t writer;
    char buffer[16];
    assert_false(bc_core_writer_init(&writer, -1, buffer, sizeof(buffer)));
}

static void test_init_rejects_null_buffer(void** state)
{
    BC_UNUSED(state);
    bc_core_writer_t writer;
    assert_false(bc_core_writer_init(&writer, 1, NULL, 64));
}

static void test_init_rejects_zero_capacity(void** state)
{
    BC_UNUSED(state);
    bc_core_writer_t writer;
    char buffer[16];
    assert_false(bc_core_writer_init(&writer, 1, buffer, 0));
}

static void test_flush_after_error_latched_returns_false(void** state)
{
    BC_UNUSED(state);
    bc_core_writer_t writer;
    char buffer[4];
    assert_true(bc_core_writer_init_buffer_only(&writer, buffer, sizeof(buffer)));
    assert_false(bc_core_writer_write_bytes(&writer, "abcdef", 6));
    assert_false(bc_core_writer_flush(&writer));
}

static void test_write_bytes_zero_length_is_noop(void** state)
{
    BC_UNUSED(state);
    bc_core_writer_t writer;
    char buffer[16];
    assert_true(bc_core_writer_init_buffer_only(&writer, buffer, sizeof(buffer)));
    assert_true(bc_core_writer_write_bytes(&writer, "ignored", 0));

    const char* data = NULL;
    size_t length = 99;
    assert_true(bc_core_writer_buffer_data(&writer, &data, &length));
    assert_int_equal(length, 0);
}

static void test_write_bytes_after_error_latched(void** state)
{
    BC_UNUSED(state);
    bc_core_writer_t writer;
    char buffer[4];
    assert_true(bc_core_writer_init_buffer_only(&writer, buffer, sizeof(buffer)));
    assert_false(bc_core_writer_write_bytes(&writer, "abcdef", 6));
    assert_false(bc_core_writer_write_bytes(&writer, "xx", 2));
}

static void test_write_char_after_error_latched(void** state)
{
    BC_UNUSED(state);
    bc_core_writer_t writer;
    char buffer[2];
    assert_true(bc_core_writer_init_buffer_only(&writer, buffer, sizeof(buffer)));
    assert_true(bc_core_writer_write_char(&writer, 'a'));
    assert_true(bc_core_writer_write_char(&writer, 'b'));
    assert_false(bc_core_writer_write_char(&writer, 'c'));
    assert_false(bc_core_writer_write_char(&writer, 'd'));
}

static void test_write_uint64_hexadecimal_padded_invalid_digits_zero(void** state)
{
    BC_UNUSED(state);
    bc_core_writer_t writer;
    char buffer[64];
    assert_true(bc_core_writer_init_buffer_only(&writer, buffer, sizeof(buffer)));
    assert_false(bc_core_writer_write_unsigned_integer_64_hexadecimal_padded(&writer, 0xABU, 0));
    assert_true(bc_core_writer_has_error(&writer));
}

static void test_write_uint64_hexadecimal_padded_invalid_digits_too_many(void** state)
{
    BC_UNUSED(state);
    bc_core_writer_t writer;
    char buffer[64];
    assert_true(bc_core_writer_init_buffer_only(&writer, buffer, sizeof(buffer)));
    assert_false(bc_core_writer_write_unsigned_integer_64_hexadecimal_padded(&writer, 0xABU, 17));
    assert_true(bc_core_writer_has_error(&writer));
}

static void test_write_double_invalid_frac_digits_negative(void** state)
{
    BC_UNUSED(state);
    bc_core_writer_t writer;
    char buffer[64];
    assert_true(bc_core_writer_init_buffer_only(&writer, buffer, sizeof(buffer)));
    assert_false(bc_core_writer_write_double(&writer, 1.0, -1));
    assert_true(bc_core_writer_has_error(&writer));
}

static void test_write_double_invalid_frac_digits_too_many(void** state)
{
    BC_UNUSED(state);
    bc_core_writer_t writer;
    char buffer[64];
    assert_true(bc_core_writer_init_buffer_only(&writer, buffer, sizeof(buffer)));
    assert_false(bc_core_writer_write_double(&writer, 1.0, 19));
    assert_true(bc_core_writer_has_error(&writer));
}

static void test_write_unicode_invalid_codepoint_above_max(void** state)
{
    BC_UNUSED(state);
    bc_core_writer_t writer;
    char buffer[64];
    assert_true(bc_core_writer_init_buffer_only(&writer, buffer, sizeof(buffer)));
    assert_false(bc_core_writer_write_unicode_codepoint_escape(&writer, 0x110000U));
    assert_true(bc_core_writer_has_error(&writer));
}

static void test_write_unicode_invalid_codepoint_surrogate(void** state)
{
    BC_UNUSED(state);
    bc_core_writer_t writer;
    char buffer[64];
    assert_true(bc_core_writer_init_buffer_only(&writer, buffer, sizeof(buffer)));
    assert_false(bc_core_writer_write_unicode_codepoint_escape(&writer, 0xD800U));
    assert_true(bc_core_writer_has_error(&writer));
}

static void test_dev_full_flush_fail(void** state)
{
    BC_UNUSED(state);
    int fd = open("/dev/full", O_WRONLY);
    if (fd < 0) {
        skip();
        return;
    }
    bc_core_writer_t writer;
    char buffer[16];
    assert_true(bc_core_writer_init(&writer, fd, buffer, sizeof(buffer)));
    /* Fill the buffer then flush -> /dev/full returns ENOSPC */
    assert_true(bc_core_writer_write_bytes(&writer, "0123456789ABCDEF", 16));
    assert_false(bc_core_writer_flush(&writer));
    assert_true(bc_core_writer_has_error(&writer));
    close(fd);
}

static void test_dev_full_write_bytes_large(void** state)
{
    BC_UNUSED(state);
    int fd = open("/dev/full", O_WRONLY);
    if (fd < 0) {
        skip();
        return;
    }
    bc_core_writer_t writer;
    char buffer[16];
    assert_true(bc_core_writer_init(&writer, fd, buffer, sizeof(buffer)));
    char large[64];
    for (size_t index = 0; index < sizeof(large); ++index) {
        large[index] = (char)('A' + (index % 26));
    }
    assert_false(bc_core_writer_write_bytes(&writer, large, sizeof(large)));
    assert_true(bc_core_writer_has_error(&writer));
    close(fd);
}

static void test_dev_full_write_char_after_full_buffer(void** state)
{
    BC_UNUSED(state);
    int fd = open("/dev/full", O_WRONLY);
    if (fd < 0) {
        skip();
        return;
    }
    bc_core_writer_t writer;
    char buffer[2];
    assert_true(bc_core_writer_init(&writer, fd, buffer, sizeof(buffer)));
    assert_true(bc_core_writer_write_char(&writer, 'a'));
    assert_true(bc_core_writer_write_char(&writer, 'b'));
    assert_false(bc_core_writer_write_char(&writer, 'c'));
    close(fd);
}

static void test_write_cstring_writes_full_string(void** state)
{
    BC_UNUSED(state);
    bc_core_writer_t writer;
    char buffer[64];
    assert_true(bc_core_writer_init_buffer_only(&writer, buffer, sizeof(buffer)));
    assert_true(bc_core_writer_write_cstring(&writer, "hello"));

    const char* data = NULL;
    size_t length = 0;
    assert_true(bc_core_writer_buffer_data(&writer, &data, &length));
    assert_int_equal(length, 5);
    assert_memory_equal(data, "hello", 5);
}

static void test_write_cstring_empty_string_is_noop(void** state)
{
    BC_UNUSED(state);
    bc_core_writer_t writer;
    char buffer[64];
    assert_true(bc_core_writer_init_buffer_only(&writer, buffer, sizeof(buffer)));
    assert_true(bc_core_writer_write_cstring(&writer, ""));

    const char* data = NULL;
    size_t length = 0;
    assert_true(bc_core_writer_buffer_data(&writer, &data, &length));
    assert_int_equal(length, 0);
}

static void test_write_cstring_after_error_latched(void** state)
{
    BC_UNUSED(state);
    bc_core_writer_t writer;
    char buffer[4];
    assert_true(bc_core_writer_init_buffer_only(&writer, buffer, sizeof(buffer)));
    assert_false(bc_core_writer_write_cstring(&writer, "this is too long"));
    assert_true(writer.error_latched != 0);
    assert_false(bc_core_writer_write_cstring(&writer, "x"));
}

static void test_write_cstring_overflow_latches_error(void** state)
{
    BC_UNUSED(state);
    bc_core_writer_t writer;
    char buffer[3];
    assert_true(bc_core_writer_init_buffer_only(&writer, buffer, sizeof(buffer)));
    assert_false(bc_core_writer_write_cstring(&writer, "abcd"));
    assert_true(writer.error_latched != 0);
}

static void test_write_cstring_long_runtime_string(void** state)
{
    BC_UNUSED(state);
    char source[256];
    for (size_t i = 0; i < sizeof(source) - 1U; i++) {
        source[i] = (char)('a' + (i % 26));
    }
    source[sizeof(source) - 1U] = 0;

    bc_core_writer_t writer;
    char buffer[512];
    assert_true(bc_core_writer_init_buffer_only(&writer, buffer, sizeof(buffer)));
    assert_true(bc_core_writer_write_cstring(&writer, source));

    const char* data = NULL;
    size_t length = 0;
    assert_true(bc_core_writer_buffer_data(&writer, &data, &length));
    assert_int_equal(length, sizeof(source) - 1U);
    assert_memory_equal(data, source, sizeof(source) - 1U);
}

static void test_destroy_returns_flush_status(void** state)
{
    BC_UNUSED(state);
    int fd = open("/dev/full", O_WRONLY);
    if (fd < 0) {
        skip();
        return;
    }
    bc_core_writer_t writer;
    char buffer[16];
    assert_true(bc_core_writer_init(&writer, fd, buffer, sizeof(buffer)));
    assert_true(bc_core_writer_write_bytes(&writer, "data", 4));
    assert_false(bc_core_writer_destroy(&writer));
    close(fd);
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_init_standard_error_sets_stderr_fd),
        cmocka_unit_test(test_init_standard_output_sets_stdout_fd),
        cmocka_unit_test(test_init_buffer_only_sets_negative_fd),
        cmocka_unit_test(test_init_buffer_only_rejects_null_buffer),
        cmocka_unit_test(test_init_buffer_only_rejects_zero_capacity),
        cmocka_unit_test(test_buffer_only_accumulates_writes),
        cmocka_unit_test(test_buffer_only_flush_is_noop),
        cmocka_unit_test(test_buffer_only_overflow_latches_error),
        cmocka_unit_test(test_buffer_only_write_char_overflow),
        cmocka_unit_test(test_buffer_data_rejects_null),
        cmocka_unit_test(test_write_error_description_round_trip),
        cmocka_unit_test(test_write_error_description_unknown_code),
        cmocka_unit_test(test_write_error_description_after_error_latched),
        cmocka_unit_test(test_write_error_description_unknown_with_fd_writer),
        cmocka_unit_test(test_init_rejects_negative_fd),
        cmocka_unit_test(test_init_rejects_null_buffer),
        cmocka_unit_test(test_init_rejects_zero_capacity),
        cmocka_unit_test(test_flush_after_error_latched_returns_false),
        cmocka_unit_test(test_write_bytes_zero_length_is_noop),
        cmocka_unit_test(test_write_bytes_after_error_latched),
        cmocka_unit_test(test_write_char_after_error_latched),
        cmocka_unit_test(test_write_uint64_hexadecimal_padded_invalid_digits_zero),
        cmocka_unit_test(test_write_uint64_hexadecimal_padded_invalid_digits_too_many),
        cmocka_unit_test(test_write_double_invalid_frac_digits_negative),
        cmocka_unit_test(test_write_double_invalid_frac_digits_too_many),
        cmocka_unit_test(test_write_unicode_invalid_codepoint_above_max),
        cmocka_unit_test(test_write_unicode_invalid_codepoint_surrogate),
        cmocka_unit_test(test_dev_full_flush_fail),
        cmocka_unit_test(test_dev_full_write_bytes_large),
        cmocka_unit_test(test_dev_full_write_char_after_full_buffer),
        cmocka_unit_test(test_write_cstring_writes_full_string),
        cmocka_unit_test(test_write_cstring_empty_string_is_noop),
        cmocka_unit_test(test_write_cstring_after_error_latched),
        cmocka_unit_test(test_write_cstring_overflow_latches_error),
        cmocka_unit_test(test_write_cstring_long_runtime_string),
        cmocka_unit_test(test_destroy_returns_flush_status),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
