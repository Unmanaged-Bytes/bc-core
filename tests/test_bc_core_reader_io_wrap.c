// SPDX-License-Identifier: MIT

#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <cmocka.h>

#include "bc_core.h"

#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

extern ssize_t __real_read(int fd, void* buffer, size_t count);

typedef enum {
    READ_BEHAVIOR_REAL,
    READ_BEHAVIOR_EINTR_THEN_REAL,
} read_behavior_t;

static read_behavior_t g_read_behavior = READ_BEHAVIOR_REAL;
static int g_read_call_count = 0;

ssize_t __wrap_read(int fd, void* buffer, size_t count)
{
    g_read_call_count += 1;
    if (g_read_behavior == READ_BEHAVIOR_EINTR_THEN_REAL && g_read_call_count == 1) {
        errno = EINTR;
        return -1;
    }
    return __real_read(fd, buffer, count);
}

static int open_temp_with_content(const void* data, size_t length)
{
    char path[64];
    strcpy(path, "/tmp/bc_core_reader_wrap_XXXXXX");
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

static void test_wrap_eintr_retry_succeeds(void** state)
{
    BC_UNUSED(state);
    g_read_behavior = READ_BEHAVIOR_EINTR_THEN_REAL;
    g_read_call_count = 0;

    int fd = open_temp_with_content("hello", 5);
    assert_true(fd >= 0);

    bc_core_reader_t reader;
    char buffer[16];
    assert_true(bc_core_reader_init(&reader, fd, buffer, sizeof(buffer)));

    char destination[8];
    size_t bytes_read = 0;
    assert_true(bc_core_reader_read(&reader, destination, sizeof(destination), &bytes_read));
    assert_int_equal(bytes_read, 5);
    assert_memory_equal(destination, "hello", 5);
    assert_true(g_read_call_count >= 2);
    assert_false(bc_core_reader_has_error(&reader));

    bc_core_reader_destroy(&reader);
    close(fd);
    g_read_behavior = READ_BEHAVIOR_REAL;
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_wrap_eintr_retry_succeeds),
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}
