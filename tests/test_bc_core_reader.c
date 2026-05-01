// SPDX-License-Identifier: MIT

#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <cmocka.h>

#include "bc_core.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static int open_temp_with_content(const void* data, size_t length)
{
    char path[64];
    strcpy(path, "/tmp/bc_core_reader_XXXXXX");
    int fd = mkstemp(path);
    if (fd < 0) {
        return -1;
    }
    unlink(path);
    if (write(fd, data, length) != (ssize_t)length) {
        close(fd);
        return -1;
    }
    if (lseek(fd, 0, SEEK_SET) != 0) {
        close(fd);
        return -1;
    }
    return fd;
}

static void test_reader_read_bytes(void** state)
{
    BC_UNUSED(state);
    const char* content = "abcdefghij";
    int fd = open_temp_with_content(content, 10U);
    assert_int_not_equal(fd, -1);

    bc_core_reader_t reader;
    char buffer[4];
    assert_true(bc_core_reader_init(&reader, fd, buffer, sizeof(buffer)));

    char output[16] = {0};
    size_t read_count = 0;

    assert_true(bc_core_reader_read(&reader, output, 3U, &read_count));
    assert_int_equal(read_count, 3);
    assert_memory_equal(output, "abc", 3);

    assert_true(bc_core_reader_read(&reader, output, 6U, &read_count));
    assert_int_equal(read_count, 1);
    assert_memory_equal(output, "d", 1);

    assert_true(bc_core_reader_read(&reader, output, 10U, &read_count));
    assert_int_equal(read_count, 4);
    assert_memory_equal(output, "efgh", 4);

    assert_true(bc_core_reader_read(&reader, output, 10U, &read_count));
    assert_int_equal(read_count, 2);
    assert_memory_equal(output, "ij", 2);

    assert_true(bc_core_reader_read(&reader, output, 10U, &read_count));
    assert_int_equal(read_count, 0);
    assert_true(bc_core_reader_is_eof(&reader));

    assert_true(bc_core_reader_destroy(&reader));
    close(fd);
}

static void test_reader_read_exact_ok(void** state)
{
    BC_UNUSED(state);
    const char* content = "0123456789ABCDEF";
    int fd = open_temp_with_content(content, 16U);
    assert_int_not_equal(fd, -1);

    bc_core_reader_t reader;
    char buffer[8];
    assert_true(bc_core_reader_init(&reader, fd, buffer, sizeof(buffer)));

    char output[16] = {0};
    assert_true(bc_core_reader_read_exact(&reader, output, 16U));
    assert_memory_equal(output, content, 16U);

    assert_true(bc_core_reader_destroy(&reader));
    close(fd);
}

static void test_reader_read_exact_eof(void** state)
{
    BC_UNUSED(state);
    const char* content = "short";
    int fd = open_temp_with_content(content, 5U);
    assert_int_not_equal(fd, -1);

    bc_core_reader_t reader;
    char buffer[8];
    assert_true(bc_core_reader_init(&reader, fd, buffer, sizeof(buffer)));

    char output[16] = {0};
    assert_false(bc_core_reader_read_exact(&reader, output, 10U));

    assert_true(bc_core_reader_destroy(&reader));
    close(fd);
}

static void test_reader_read_line_multiline(void** state)
{
    BC_UNUSED(state);
    const char* content = "first\nsecond\nthird";
    int fd = open_temp_with_content(content, 18U);
    assert_int_not_equal(fd, -1);

    bc_core_reader_t reader;
    char buffer[64];
    assert_true(bc_core_reader_init(&reader, fd, buffer, sizeof(buffer)));

    const char* line = NULL;
    size_t length = 0;

    assert_true(bc_core_reader_read_line(&reader, &line, &length));
    assert_int_equal(length, 5);
    assert_memory_equal(line, "first", 5);

    assert_true(bc_core_reader_read_line(&reader, &line, &length));
    assert_int_equal(length, 6);
    assert_memory_equal(line, "second", 6);

    assert_true(bc_core_reader_read_line(&reader, &line, &length));
    assert_int_equal(length, 5);
    assert_memory_equal(line, "third", 5);

    assert_false(bc_core_reader_read_line(&reader, &line, &length));
    assert_true(bc_core_reader_is_eof(&reader));

    assert_true(bc_core_reader_destroy(&reader));
    close(fd);
}

static void test_reader_read_line_refill_needed(void** state)
{
    BC_UNUSED(state);
    const char* content = "ab\ncd\nef\ngh\nij\n";
    int fd = open_temp_with_content(content, 15U);
    assert_int_not_equal(fd, -1);

    bc_core_reader_t reader;
    char buffer[6];
    assert_true(bc_core_reader_init(&reader, fd, buffer, sizeof(buffer)));

    const char* line = NULL;
    size_t length = 0;

    for (size_t line_index = 0; line_index < 5U; line_index++) {
        assert_true(bc_core_reader_read_line(&reader, &line, &length));
        assert_int_equal(length, 2);
        assert_int_equal(line[0], (char)('a' + (char)(line_index * 2U)));
        assert_int_equal(line[1], (char)('b' + (char)(line_index * 2U)));
    }

    assert_false(bc_core_reader_read_line(&reader, &line, &length));
    assert_true(bc_core_reader_is_eof(&reader));

    assert_true(bc_core_reader_destroy(&reader));
    close(fd);
}

static void test_reader_read_line_long_line_error(void** state)
{
    BC_UNUSED(state);
    char content[32];
    memset(content, 'x', sizeof(content));
    int fd = open_temp_with_content(content, sizeof(content));
    assert_int_not_equal(fd, -1);

    bc_core_reader_t reader;
    char buffer[8];
    assert_true(bc_core_reader_init(&reader, fd, buffer, sizeof(buffer)));

    const char* line = NULL;
    size_t length = 0;
    assert_false(bc_core_reader_read_line(&reader, &line, &length));
    assert_true(bc_core_reader_has_error(&reader));

    assert_true(bc_core_reader_destroy(&reader));
    close(fd);
}

static void test_reader_read_max_len_zero(void** state)
{
    BC_UNUSED(state);
    int fd = open_temp_with_content("hello", 5);
    assert_true(fd >= 0);

    bc_core_reader_t reader;
    char buffer[16];
    assert_true(bc_core_reader_init(&reader, fd, buffer, sizeof(buffer)));

    char destination[8];
    size_t bytes_read = 99;
    assert_true(bc_core_reader_read(&reader, destination, 0, &bytes_read));
    assert_int_equal(bytes_read, 0);

    bc_core_reader_destroy(&reader);
    close(fd);
}

static void test_reader_read_eof_returns_zero(void** state)
{
    BC_UNUSED(state);
    int fd = open_temp_with_content("ab", 2);
    assert_true(fd >= 0);

    bc_core_reader_t reader;
    char buffer[16];
    assert_true(bc_core_reader_init(&reader, fd, buffer, sizeof(buffer)));

    char destination[8];
    size_t bytes_read = 0;
    assert_true(bc_core_reader_read(&reader, destination, sizeof(destination), &bytes_read));
    assert_int_equal(bytes_read, 2);

    bytes_read = 99;
    assert_true(bc_core_reader_read(&reader, destination, sizeof(destination), &bytes_read));
    assert_int_equal(bytes_read, 0);
    assert_true(bc_core_reader_is_eof(&reader));

    bc_core_reader_destroy(&reader);
    close(fd);
}

static void test_reader_read_invalid_fd_latches_error(void** state)
{
    BC_UNUSED(state);
    int fd = open_temp_with_content("data", 4);
    assert_true(fd >= 0);

    bc_core_reader_t reader;
    char buffer[16];
    assert_true(bc_core_reader_init(&reader, fd, buffer, sizeof(buffer)));

    close(fd);

    char destination[8];
    size_t bytes_read = 0;
    assert_false(bc_core_reader_read(&reader, destination, sizeof(destination), &bytes_read));
    assert_true(bc_core_reader_has_error(&reader));

    bc_core_reader_destroy(&reader);
}

static void test_reader_read_after_error_latched(void** state)
{
    BC_UNUSED(state);
    int fd = open_temp_with_content("data", 4);
    assert_true(fd >= 0);

    bc_core_reader_t reader;
    char buffer[16];
    assert_true(bc_core_reader_init(&reader, fd, buffer, sizeof(buffer)));
    close(fd);

    char destination[8];
    size_t bytes_read = 0;
    (void)bc_core_reader_read(&reader, destination, sizeof(destination), &bytes_read);

    assert_false(bc_core_reader_read(&reader, destination, sizeof(destination), &bytes_read));
    bc_core_reader_destroy(&reader);
}

static void test_reader_read_exact_after_error_latched(void** state)
{
    BC_UNUSED(state);
    int fd = open_temp_with_content("data", 4);
    assert_true(fd >= 0);

    bc_core_reader_t reader;
    char buffer[16];
    assert_true(bc_core_reader_init(&reader, fd, buffer, sizeof(buffer)));
    close(fd);

    char destination[8];
    (void)bc_core_reader_read_exact(&reader, destination, 8);

    assert_false(bc_core_reader_read_exact(&reader, destination, 4));
    bc_core_reader_destroy(&reader);
}

static void test_reader_read_line_after_error_latched(void** state)
{
    BC_UNUSED(state);
    int fd = open_temp_with_content("data", 4);
    assert_true(fd >= 0);

    bc_core_reader_t reader;
    char buffer[16];
    assert_true(bc_core_reader_init(&reader, fd, buffer, sizeof(buffer)));
    close(fd);

    const char* line = NULL;
    size_t length = 0;
    (void)bc_core_reader_read_line(&reader, &line, &length);

    assert_false(bc_core_reader_read_line(&reader, &line, &length));
    bc_core_reader_destroy(&reader);
}

static void test_reader_destroy_clears_fields(void** state)
{
    BC_UNUSED(state);
    int fd = open_temp_with_content("data", 4);
    assert_true(fd >= 0);

    bc_core_reader_t reader;
    char buffer[16];
    assert_true(bc_core_reader_init(&reader, fd, buffer, sizeof(buffer)));
    assert_true(bc_core_reader_destroy(&reader));
    assert_int_equal(reader.fd, -1);
    assert_null(reader.buffer);
    assert_int_equal(reader.capacity, 0);

    close(fd);
}

static void test_reader_is_eof_initially_false(void** state)
{
    BC_UNUSED(state);
    int fd = open_temp_with_content("hi", 2);
    assert_true(fd >= 0);

    bc_core_reader_t reader;
    char buffer[16];
    assert_true(bc_core_reader_init(&reader, fd, buffer, sizeof(buffer)));
    assert_false(bc_core_reader_is_eof(&reader));

    bc_core_reader_destroy(&reader);
    close(fd);
}

static void test_reader_has_error_initially_false(void** state)
{
    BC_UNUSED(state);
    int fd = open_temp_with_content("hi", 2);
    assert_true(fd >= 0);

    bc_core_reader_t reader;
    char buffer[16];
    assert_true(bc_core_reader_init(&reader, fd, buffer, sizeof(buffer)));
    assert_false(bc_core_reader_has_error(&reader));

    bc_core_reader_destroy(&reader);
    close(fd);
}

static void test_reader_third_read_after_eof_latched(void** state)
{
    BC_UNUSED(state);
    int fd = open_temp_with_content("ab", 2);
    assert_true(fd >= 0);

    bc_core_reader_t reader;
    char buffer[16];
    assert_true(bc_core_reader_init(&reader, fd, buffer, sizeof(buffer)));

    char destination[8];
    size_t bytes_read = 0;
    assert_true(bc_core_reader_read(&reader, destination, sizeof(destination), &bytes_read));
    assert_int_equal(bytes_read, 2);
    assert_true(bc_core_reader_read(&reader, destination, sizeof(destination), &bytes_read));
    assert_int_equal(bytes_read, 0);
    assert_true(bc_core_reader_read(&reader, destination, sizeof(destination), &bytes_read));
    assert_int_equal(bytes_read, 0);

    bc_core_reader_destroy(&reader);
    close(fd);
}

static void test_reader_read_line_eof_with_residual(void** state)
{
    BC_UNUSED(state);
    int fd = open_temp_with_content("trailing", 8);
    assert_true(fd >= 0);

    bc_core_reader_t reader;
    char buffer[16];
    assert_true(bc_core_reader_init(&reader, fd, buffer, sizeof(buffer)));

    const char* line = NULL;
    size_t length = 0;
    assert_true(bc_core_reader_read_line(&reader, &line, &length));
    assert_int_equal(length, 8);
    assert_memory_equal(line, "trailing", 8);

    assert_false(bc_core_reader_read_line(&reader, &line, &length));

    bc_core_reader_destroy(&reader);
    close(fd);
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_reader_read_bytes),
        cmocka_unit_test(test_reader_read_exact_ok),
        cmocka_unit_test(test_reader_read_exact_eof),
        cmocka_unit_test(test_reader_read_line_multiline),
        cmocka_unit_test(test_reader_read_line_refill_needed),
        cmocka_unit_test(test_reader_read_line_long_line_error),
        cmocka_unit_test(test_reader_read_max_len_zero),
        cmocka_unit_test(test_reader_read_eof_returns_zero),
        cmocka_unit_test(test_reader_read_invalid_fd_latches_error),
        cmocka_unit_test(test_reader_read_after_error_latched),
        cmocka_unit_test(test_reader_read_exact_after_error_latched),
        cmocka_unit_test(test_reader_read_line_after_error_latched),
        cmocka_unit_test(test_reader_destroy_clears_fields),
        cmocka_unit_test(test_reader_is_eof_initially_false),
        cmocka_unit_test(test_reader_has_error_initially_false),
        cmocka_unit_test(test_reader_third_read_after_eof_latched),
        cmocka_unit_test(test_reader_read_line_eof_with_residual),
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}
