// SPDX-License-Identifier: MIT

#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <cmocka.h>

#include "bc_core.h"

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
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
