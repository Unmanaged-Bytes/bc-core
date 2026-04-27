// SPDX-License-Identifier: MIT

#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <cmocka.h>

#include "bc_core.h"

#include <errno.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

extern ssize_t __real_write(int fd, const void* buffer, size_t count);

typedef enum {
    WRITE_BEHAVIOR_REAL,
    WRITE_BEHAVIOR_EINTR_THEN_REAL,
    WRITE_BEHAVIOR_RETURNS_ZERO,
} write_behavior_t;

static write_behavior_t g_write_behavior = WRITE_BEHAVIOR_REAL;
static int g_write_call_count = 0;

ssize_t __wrap_write(int fd, const void* buffer, size_t count)
{
    g_write_call_count += 1;
    switch (g_write_behavior) {
    case WRITE_BEHAVIOR_EINTR_THEN_REAL:
        if (g_write_call_count == 1) {
            errno = EINTR;
            return -1;
        }
        return __real_write(fd, buffer, count);
    case WRITE_BEHAVIOR_RETURNS_ZERO:
        return 0;
    case WRITE_BEHAVIOR_REAL:
    default:
        return __real_write(fd, buffer, count);
    }
}

static void test_wrap_eintr_retry_succeeds(void** state)
{
    BC_UNUSED(state);
    g_write_behavior = WRITE_BEHAVIOR_EINTR_THEN_REAL;
    g_write_call_count = 0;

    bc_core_writer_t writer;
    char buffer[16];
    int fd = STDERR_FILENO;
    assert_true(bc_core_writer_init(&writer, fd, buffer, sizeof(buffer)));
    assert_true(bc_core_writer_write_bytes(&writer, "hello", 5));
    assert_true(bc_core_writer_flush(&writer));
    assert_true(g_write_call_count >= 2);
    assert_false(bc_core_writer_has_error(&writer));

    g_write_behavior = WRITE_BEHAVIOR_REAL;
}

static void test_wrap_returns_zero_latches_error(void** state)
{
    BC_UNUSED(state);
    g_write_behavior = WRITE_BEHAVIOR_RETURNS_ZERO;
    g_write_call_count = 0;

    bc_core_writer_t writer;
    char buffer[16];
    assert_true(bc_core_writer_init(&writer, STDERR_FILENO, buffer, sizeof(buffer)));
    assert_true(bc_core_writer_write_bytes(&writer, "hello", 5));
    assert_false(bc_core_writer_flush(&writer));
    assert_true(bc_core_writer_has_error(&writer));

    g_write_behavior = WRITE_BEHAVIOR_REAL;
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_wrap_eintr_retry_succeeds),
        cmocka_unit_test(test_wrap_returns_zero_latches_error),
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}
