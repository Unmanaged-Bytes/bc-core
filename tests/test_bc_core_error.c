// SPDX-License-Identifier: MIT

#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <cmocka.h>

#include "bc_core.h"

#include <errno.h>
#include <string.h>

static void test_describe_none(void** state)
{
    BC_UNUSED(state);

    char buffer[64];
    size_t length = 0;
    bool success = bc_core_error_describe(BC_CORE_ERROR_NONE, buffer, sizeof(buffer), &length);

    assert_true(success);
    assert_int_equal(length, strlen("no error"));
    assert_memory_equal(buffer, "no error", length);
}

static void test_describe_invalid_argument(void** state)
{
    BC_UNUSED(state);

    char buffer[64];
    size_t length = 0;
    bool success = bc_core_error_describe(BC_CORE_ERROR_INVALID_ARGUMENT, buffer, sizeof(buffer), &length);

    assert_true(success);
    assert_int_equal(length, strlen("invalid argument"));
    assert_memory_equal(buffer, "invalid argument", length);
}

static void test_describe_all_known_codes(void** state)
{
    BC_UNUSED(state);

    bc_core_error_code_t codes[] = {
        BC_CORE_ERROR_NONE,
        BC_CORE_ERROR_INVALID_ARGUMENT,
        BC_CORE_ERROR_BUFFER_TOO_SMALL,
        BC_CORE_ERROR_OVERFLOW,
        BC_CORE_ERROR_PARSE_FAILED,
        BC_CORE_ERROR_NOT_FOUND,
        BC_CORE_ERROR_PERMISSION_DENIED,
        BC_CORE_ERROR_RESOURCE_EXHAUSTED,
        BC_CORE_ERROR_INPUT_OUTPUT_FAILED,
        BC_CORE_ERROR_END_OF_FILE,
        BC_CORE_ERROR_BROKEN_PIPE,
        BC_CORE_ERROR_INTERRUPTED,
        BC_CORE_ERROR_UNAVAILABLE,
        BC_CORE_ERROR_INTERNAL,
    };

    for (size_t index = 0; index < sizeof(codes) / sizeof(codes[0]); ++index) {
        char buffer[128];
        size_t length = 0;
        bool success = bc_core_error_describe(codes[index], buffer, sizeof(buffer), &length);
        assert_true(success);
        assert_in_range(length, 1, sizeof(buffer));
    }
}

static void test_describe_unknown_code(void** state)
{
    BC_UNUSED(state);

    char buffer[64];
    size_t length = 0;
    bool success = bc_core_error_describe((bc_core_error_code_t)9999, buffer, sizeof(buffer), &length);

    assert_false(success);
}

static void test_describe_buffer_too_small(void** state)
{
    BC_UNUSED(state);

    char buffer[3];
    size_t length = 0;
    bool success = bc_core_error_describe(BC_CORE_ERROR_INVALID_ARGUMENT, buffer, sizeof(buffer), &length);

    assert_false(success);
}

static void test_describe_capacity_zero(void** state)
{
    BC_UNUSED(state);

    char buffer[1];
    size_t length = 0;
    bool success = bc_core_error_describe(BC_CORE_ERROR_NONE, buffer, 0, &length);

    assert_false(success);
}

static void test_name_all_known_codes(void** state)
{
    BC_UNUSED(state);

    bc_core_error_code_t codes[] = {
        BC_CORE_ERROR_NONE,
        BC_CORE_ERROR_INVALID_ARGUMENT,
        BC_CORE_ERROR_BUFFER_TOO_SMALL,
        BC_CORE_ERROR_OVERFLOW,
        BC_CORE_ERROR_PARSE_FAILED,
        BC_CORE_ERROR_NOT_FOUND,
        BC_CORE_ERROR_PERMISSION_DENIED,
        BC_CORE_ERROR_RESOURCE_EXHAUSTED,
        BC_CORE_ERROR_INPUT_OUTPUT_FAILED,
        BC_CORE_ERROR_END_OF_FILE,
        BC_CORE_ERROR_BROKEN_PIPE,
        BC_CORE_ERROR_INTERRUPTED,
        BC_CORE_ERROR_UNAVAILABLE,
        BC_CORE_ERROR_INTERNAL,
    };

    for (size_t index = 0; index < sizeof(codes) / sizeof(codes[0]); ++index) {
        const char* name = NULL;
        size_t name_length = 0;
        bool success = bc_core_error_name(codes[index], &name, &name_length);
        assert_true(success);
        assert_non_null(name);
        assert_in_range(name_length, strlen("BC_CORE_ERROR_NONE"), 64);
        assert_memory_equal(name, "BC_CORE_ERROR_", strlen("BC_CORE_ERROR_"));
    }
}

static void test_name_specific_code(void** state)
{
    BC_UNUSED(state);

    const char* name = NULL;
    size_t name_length = 0;
    bool success = bc_core_error_name(BC_CORE_ERROR_OVERFLOW, &name, &name_length);

    assert_true(success);
    assert_int_equal(name_length, strlen("BC_CORE_ERROR_OVERFLOW"));
    assert_memory_equal(name, "BC_CORE_ERROR_OVERFLOW", name_length);
}

static void test_name_unknown_code(void** state)
{
    BC_UNUSED(state);

    const char* name = NULL;
    size_t name_length = 0;
    bool success = bc_core_error_name((bc_core_error_code_t)9999, &name, &name_length);

    assert_false(success);
}

static void test_from_system_errno_zero(void** state)
{
    BC_UNUSED(state);

    bc_core_error_code_t code = BC_CORE_ERROR_INTERNAL;
    bool success = bc_core_error_from_system_errno(0, &code);

    assert_true(success);
    assert_int_equal(code, BC_CORE_ERROR_NONE);
}

static void test_from_system_errno_einval(void** state)
{
    BC_UNUSED(state);

    bc_core_error_code_t code = BC_CORE_ERROR_NONE;
    bool success = bc_core_error_from_system_errno(EINVAL, &code);

    assert_true(success);
    assert_int_equal(code, BC_CORE_ERROR_INVALID_ARGUMENT);
}

static void test_from_system_errno_enoent(void** state)
{
    BC_UNUSED(state);

    bc_core_error_code_t code = BC_CORE_ERROR_NONE;
    bool success = bc_core_error_from_system_errno(ENOENT, &code);

    assert_true(success);
    assert_int_equal(code, BC_CORE_ERROR_NOT_FOUND);
}

static void test_from_system_errno_eintr(void** state)
{
    BC_UNUSED(state);

    bc_core_error_code_t code = BC_CORE_ERROR_NONE;
    bool success = bc_core_error_from_system_errno(EINTR, &code);

    assert_true(success);
    assert_int_equal(code, BC_CORE_ERROR_INTERRUPTED);
}

static void test_from_system_errno_eagain(void** state)
{
    BC_UNUSED(state);

    bc_core_error_code_t code = BC_CORE_ERROR_NONE;
    bool success = bc_core_error_from_system_errno(EAGAIN, &code);

    assert_true(success);
    assert_int_equal(code, BC_CORE_ERROR_UNAVAILABLE);
}

static void test_from_system_errno_eacces(void** state)
{
    BC_UNUSED(state);

    bc_core_error_code_t code = BC_CORE_ERROR_NONE;
    bool success = bc_core_error_from_system_errno(EACCES, &code);

    assert_true(success);
    assert_int_equal(code, BC_CORE_ERROR_PERMISSION_DENIED);
}

static void test_from_system_errno_enomem(void** state)
{
    BC_UNUSED(state);

    bc_core_error_code_t code = BC_CORE_ERROR_NONE;
    bool success = bc_core_error_from_system_errno(ENOMEM, &code);

    assert_true(success);
    assert_int_equal(code, BC_CORE_ERROR_RESOURCE_EXHAUSTED);
}

static void test_from_system_errno_eio(void** state)
{
    BC_UNUSED(state);

    bc_core_error_code_t code = BC_CORE_ERROR_NONE;
    bool success = bc_core_error_from_system_errno(EIO, &code);

    assert_true(success);
    assert_int_equal(code, BC_CORE_ERROR_INPUT_OUTPUT_FAILED);
}

static void test_from_system_errno_epipe(void** state)
{
    BC_UNUSED(state);

    bc_core_error_code_t code = BC_CORE_ERROR_NONE;
    bool success = bc_core_error_from_system_errno(EPIPE, &code);

    assert_true(success);
    assert_int_equal(code, BC_CORE_ERROR_BROKEN_PIPE);
}

static void test_from_system_errno_eoverflow(void** state)
{
    BC_UNUSED(state);

    bc_core_error_code_t code = BC_CORE_ERROR_NONE;
    bool success = bc_core_error_from_system_errno(EOVERFLOW, &code);

    assert_true(success);
    assert_int_equal(code, BC_CORE_ERROR_OVERFLOW);
}

static void test_from_system_errno_enosys(void** state)
{
    BC_UNUSED(state);

    bc_core_error_code_t code = BC_CORE_ERROR_NONE;
    bool success = bc_core_error_from_system_errno(ENOSYS, &code);

    assert_true(success);
    assert_int_equal(code, BC_CORE_ERROR_UNAVAILABLE);
}

static void test_from_system_errno_unknown(void** state)
{
    BC_UNUSED(state);

    bc_core_error_code_t code = BC_CORE_ERROR_NONE;
    bool success = bc_core_error_from_system_errno(9999, &code);

    assert_true(success);
    assert_int_equal(code, BC_CORE_ERROR_INTERNAL);
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_describe_none),
        cmocka_unit_test(test_describe_invalid_argument),
        cmocka_unit_test(test_describe_all_known_codes),
        cmocka_unit_test(test_describe_unknown_code),
        cmocka_unit_test(test_describe_buffer_too_small),
        cmocka_unit_test(test_describe_capacity_zero),
        cmocka_unit_test(test_name_all_known_codes),
        cmocka_unit_test(test_name_specific_code),
        cmocka_unit_test(test_name_unknown_code),
        cmocka_unit_test(test_from_system_errno_zero),
        cmocka_unit_test(test_from_system_errno_einval),
        cmocka_unit_test(test_from_system_errno_enoent),
        cmocka_unit_test(test_from_system_errno_eintr),
        cmocka_unit_test(test_from_system_errno_eagain),
        cmocka_unit_test(test_from_system_errno_eacces),
        cmocka_unit_test(test_from_system_errno_enomem),
        cmocka_unit_test(test_from_system_errno_eio),
        cmocka_unit_test(test_from_system_errno_epipe),
        cmocka_unit_test(test_from_system_errno_eoverflow),
        cmocka_unit_test(test_from_system_errno_enosys),
        cmocka_unit_test(test_from_system_errno_unknown),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
