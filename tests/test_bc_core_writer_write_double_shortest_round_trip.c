// SPDX-License-Identifier: MIT

#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <cmocka.h>

#include "bc_core.h"

#include <float.h>
#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static void test_smoke_simple_value(void** state)
{
    BC_UNUSED(state);
    bc_core_writer_t writer;
    char buffer[64];
    assert_true(bc_core_writer_init_buffer_only(&writer, buffer, sizeof(buffer)));
    assert_true(bc_core_writer_write_double_shortest_round_trip(&writer, 1.5));

    const char* data = NULL;
    size_t length = 0;
    assert_true(bc_core_writer_buffer_data(&writer, &data, &length));
    assert_true(length > 0U);

    char nul_terminated[64];
    memcpy(nul_terminated, data, length);
    nul_terminated[length] = '\0';
    double parsed = strtod(nul_terminated, NULL);
    assert_true(parsed == 1.5);
}

static void test_smoke_round_trip_dbl_max(void** state)
{
    BC_UNUSED(state);
    bc_core_writer_t writer;
    char buffer[128];
    assert_true(bc_core_writer_init_buffer_only(&writer, buffer, sizeof(buffer)));
    assert_true(bc_core_writer_write_double_shortest_round_trip(&writer, DBL_MAX));

    const char* data = NULL;
    size_t length = 0;
    assert_true(bc_core_writer_buffer_data(&writer, &data, &length));

    char nul_terminated[64];
    assert_true(length < sizeof(nul_terminated));
    memcpy(nul_terminated, data, length);
    nul_terminated[length] = '\0';
    double parsed = strtod(nul_terminated, NULL);
    assert_true(parsed == DBL_MAX);
}

static void test_smoke_negative_zero_sign_preserved(void** state)
{
    BC_UNUSED(state);
    bc_core_writer_t writer;
    char buffer[64];
    assert_true(bc_core_writer_init_buffer_only(&writer, buffer, sizeof(buffer)));
    double minus_zero = -0.0;
    assert_true(bc_core_writer_write_double_shortest_round_trip(&writer, minus_zero));

    const char* data = NULL;
    size_t length = 0;
    assert_true(bc_core_writer_buffer_data(&writer, &data, &length));

    char nul_terminated[64];
    memcpy(nul_terminated, data, length);
    nul_terminated[length] = '\0';
    double parsed = strtod(nul_terminated, NULL);
    assert_true(signbit(parsed) != 0);
}

static void test_buffer_overflow_latches_error(void** state)
{
    BC_UNUSED(state);
    bc_core_writer_t writer;
    /* Tiny buffer-only writer: write_bytes will fail because shortest-round-trip
       formatting of 0.1 produces ~18 chars. */
    char buffer[4];
    assert_true(bc_core_writer_init_buffer_only(&writer, buffer, sizeof(buffer)));
    assert_false(bc_core_writer_write_double_shortest_round_trip(&writer, 0.1));
    assert_true(bc_core_writer_has_error(&writer));
}

static void test_repeated_writes_concatenate(void** state)
{
    BC_UNUSED(state);
    bc_core_writer_t writer;
    char buffer[128];
    assert_true(bc_core_writer_init_buffer_only(&writer, buffer, sizeof(buffer)));
    assert_true(bc_core_writer_write_double_shortest_round_trip(&writer, 1.0));
    assert_true(bc_core_writer_write_bytes(&writer, ",", 1));
    assert_true(bc_core_writer_write_double_shortest_round_trip(&writer, 2.0));

    const char* data = NULL;
    size_t length = 0;
    assert_true(bc_core_writer_buffer_data(&writer, &data, &length));
    assert_true(length >= 3U);
    /* Locate the comma we wrote between values. */
    bool comma_found = false;
    for (size_t scan_index = 0; scan_index < length; scan_index++) {
        if (data[scan_index] == ',') {
            comma_found = true;
            break;
        }
    }
    assert_true(comma_found);
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_smoke_simple_value),
        cmocka_unit_test(test_smoke_round_trip_dbl_max),
        cmocka_unit_test(test_smoke_negative_zero_sign_preserved),
        cmocka_unit_test(test_buffer_overflow_latches_error),
        cmocka_unit_test(test_repeated_writes_concatenate),
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}
