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

#define TEMP_BUFFER_SIZE (4U * 1024U)

static int open_temp_file(char* out_path)
{
    strcpy(out_path, "/tmp/bc_core_writer_XXXXXX");
    int fd = mkstemp(out_path);
    return fd;
}

static size_t read_all(const char* path, char* buffer, size_t capacity)
{
    FILE* file = fopen(path, "rb");
    if (file == NULL) {
        return 0;
    }
    size_t length = fread(buffer, 1U, capacity, file);
    fclose(file);
    return length;
}

static void test_writer_init_bad_args(void** state)
{
    BC_UNUSED(state);
    bc_core_writer_t writer;
    char buffer[64];

    assert_false(bc_core_writer_init(&writer, -1, buffer, sizeof(buffer)));
    assert_false(bc_core_writer_init(&writer, 1, NULL, sizeof(buffer)));
    assert_false(bc_core_writer_init(&writer, 1, buffer, 0));
}

static void test_writer_write_bytes_small(void** state)
{
    BC_UNUSED(state);
    char path[64];
    int fd = open_temp_file(path);
    assert_int_not_equal(fd, -1);

    bc_core_writer_t writer;
    char buffer[64];
    assert_true(bc_core_writer_init(&writer, fd, buffer, sizeof(buffer)));
    assert_true(bc_core_writer_write_bytes(&writer, "hello", 5U));
    assert_true(bc_core_writer_write_bytes(&writer, ", world\n", 8U));
    assert_true(bc_core_writer_destroy(&writer));
    close(fd);

    char read_buffer[64];
    size_t length = read_all(path, read_buffer, sizeof(read_buffer));
    assert_int_equal(length, 13);
    assert_memory_equal(read_buffer, "hello, world\n", 13);

    unlink(path);
}

static void test_writer_write_bytes_overflow_chain(void** state)
{
    BC_UNUSED(state);
    char path[64];
    int fd = open_temp_file(path);
    assert_int_not_equal(fd, -1);

    bc_core_writer_t writer;
    char buffer[8];
    assert_true(bc_core_writer_init(&writer, fd, buffer, sizeof(buffer)));

    for (int chunk_index = 0; chunk_index < 10; chunk_index++) {
        assert_true(bc_core_writer_write_bytes(&writer, "abcdef", 6U));
    }
    assert_true(bc_core_writer_destroy(&writer));
    close(fd);

    char read_buffer[128] = {0};
    size_t length = read_all(path, read_buffer, sizeof(read_buffer));
    assert_int_equal(length, 60);
    for (size_t copy_index = 0; copy_index < 10U; copy_index++) {
        assert_memory_equal(read_buffer + copy_index * 6U, "abcdef", 6U);
    }
    unlink(path);
}

static void test_writer_write_bytes_bypass_buffer(void** state)
{
    BC_UNUSED(state);
    char path[64];
    int fd = open_temp_file(path);
    assert_int_not_equal(fd, -1);

    bc_core_writer_t writer;
    char buffer[16];
    assert_true(bc_core_writer_init(&writer, fd, buffer, sizeof(buffer)));
    assert_true(bc_core_writer_write_bytes(&writer, "head", 4U));

    char payload[64];
    for (size_t byte_index = 0; byte_index < sizeof(payload); byte_index++) {
        payload[byte_index] = (char)('A' + (byte_index % 26U));
    }
    assert_true(bc_core_writer_write_bytes(&writer, payload, sizeof(payload)));
    assert_true(bc_core_writer_write_bytes(&writer, "!", 1U));
    assert_true(bc_core_writer_destroy(&writer));
    close(fd);

    char read_buffer[128] = {0};
    size_t length = read_all(path, read_buffer, sizeof(read_buffer));
    assert_int_equal(length, 4U + sizeof(payload) + 1U);
    assert_memory_equal(read_buffer, "head", 4U);
    assert_memory_equal(read_buffer + 4U, payload, sizeof(payload));
    assert_int_equal((unsigned char)read_buffer[4U + sizeof(payload)], '!');

    unlink(path);
}

static void test_writer_formatters(void** state)
{
    BC_UNUSED(state);
    char path[64];
    int fd = open_temp_file(path);
    assert_int_not_equal(fd, -1);

    bc_core_writer_t writer;
    char buffer[256];
    assert_true(bc_core_writer_init(&writer, fd, buffer, sizeof(buffer)));

    assert_true(bc_core_writer_write_uint64_dec(&writer, 12345U));
    assert_true(bc_core_writer_write_char(&writer, ' '));
    assert_true(bc_core_writer_write_int64(&writer, -42));
    assert_true(bc_core_writer_write_char(&writer, ' '));
    assert_true(bc_core_writer_write_uint64_hex(&writer, 0xBEEFU));
    assert_true(bc_core_writer_write_char(&writer, ' '));
    assert_true(bc_core_writer_write_uint64_hex_padded(&writer, 0x2AU, 4U));
    assert_true(bc_core_writer_write_char(&writer, ' '));
    assert_true(bc_core_writer_write_double(&writer, 2.5, 2));
    assert_true(bc_core_writer_write_char(&writer, ' '));
    assert_true(bc_core_writer_write_bytes_human(&writer, 2048U));
    assert_true(bc_core_writer_write_char(&writer, ' '));
    assert_true(bc_core_writer_write_duration_ns(&writer, 1500U));

    assert_true(bc_core_writer_destroy(&writer));
    close(fd);

    char read_buffer[256] = {0};
    size_t length = read_all(path, read_buffer, sizeof(read_buffer));
    read_buffer[length] = '\0';

    assert_string_equal(read_buffer, "12345 -42 beef 002a 2.50 2.00 KB 1.500us");

    unlink(path);
}

static void test_writer_puts_macro(void** state)
{
    BC_UNUSED(state);
    char path[64];
    int fd = open_temp_file(path);
    assert_int_not_equal(fd, -1);

    bc_core_writer_t writer;
    char buffer[64];
    assert_true(bc_core_writer_init(&writer, fd, buffer, sizeof(buffer)));
    assert_true(BC_CORE_WRITER_PUTS(&writer, "literal\n"));
    assert_true(bc_core_writer_destroy(&writer));
    close(fd);

    char read_buffer[64] = {0};
    size_t length = read_all(path, read_buffer, sizeof(read_buffer));
    assert_int_equal(length, 8);
    assert_memory_equal(read_buffer, "literal\n", 8);
    unlink(path);
}

static void test_writer_error_latched_after_bad_fd(void** state)
{
    BC_UNUSED(state);
    int devnull = open("/dev/null", O_WRONLY);
    assert_int_not_equal(devnull, -1);

    bc_core_writer_t writer;
    char buffer[8];
    assert_true(bc_core_writer_init(&writer, devnull, buffer, sizeof(buffer)));
    assert_true(bc_core_writer_write_bytes(&writer, "hello", 5U));
    close(devnull);

    assert_false(bc_core_writer_write_bytes(&writer, "more data that spills past the buffer boundary", 46U));
    assert_true(bc_core_writer_has_error(&writer));
    assert_false(bc_core_writer_write_bytes(&writer, "after error", 11U));
    assert_false(bc_core_writer_flush(&writer));
}

static void test_writer_empty_write(void** state)
{
    BC_UNUSED(state);
    char path[64];
    int fd = open_temp_file(path);
    assert_int_not_equal(fd, -1);

    bc_core_writer_t writer;
    char buffer[16];
    assert_true(bc_core_writer_init(&writer, fd, buffer, sizeof(buffer)));
    assert_true(bc_core_writer_write_bytes(&writer, "x", 0U));
    assert_true(bc_core_writer_flush(&writer));
    assert_true(bc_core_writer_destroy(&writer));
    close(fd);

    char read_buffer[16] = {0};
    size_t length = read_all(path, read_buffer, sizeof(read_buffer));
    assert_int_equal(length, 0);
    unlink(path);
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_writer_init_bad_args),
        cmocka_unit_test(test_writer_write_bytes_small),
        cmocka_unit_test(test_writer_write_bytes_overflow_chain),
        cmocka_unit_test(test_writer_write_bytes_bypass_buffer),
        cmocka_unit_test(test_writer_formatters),
        cmocka_unit_test(test_writer_puts_macro),
        cmocka_unit_test(test_writer_error_latched_after_bad_fd),
        cmocka_unit_test(test_writer_empty_write),
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}
