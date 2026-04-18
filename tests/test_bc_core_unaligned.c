// SPDX-License-Identifier: MIT

#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <cmocka.h>

#include "bc_core.h"

#include <stdint.h>

static void test_load_u16_aligned(void** state)
{
    BC_UNUSED(state);

    const uint8_t buffer[4] = {0xAA, 0xBB, 0xCC, 0xDD};
    uint16_t value = 0;
    bc_core_load_u16_unaligned(buffer, &value);
    assert_int_equal(value, (uint16_t)0xBBAA);
}

static void test_load_u16_at_odd_offset(void** state)
{
    BC_UNUSED(state);

    const uint8_t buffer[4] = {0x00, 0xAA, 0xBB, 0x00};
    uint16_t value = 0;
    bc_core_load_u16_unaligned(buffer + 1, &value);
    assert_int_equal(value, (uint16_t)0xBBAA);
}

static void test_load_u32_aligned(void** state)
{
    BC_UNUSED(state);

    const uint8_t buffer[8] = {0x11, 0x22, 0x33, 0x44, 0x00, 0x00, 0x00, 0x00};
    uint32_t value = 0;
    bc_core_load_u32_unaligned(buffer, &value);
    assert_int_equal(value, (uint32_t)0x44332211);
}

static void test_load_u32_at_each_odd_offset(void** state)
{
    BC_UNUSED(state);

    const uint8_t buffer[8] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x00, 0x00, 0x00};
    uint32_t value = 0;
    bc_core_load_u32_unaligned(buffer + 1, &value);
    assert_int_equal(value, (uint32_t)0x44332211);

    const uint8_t buffer2[8] = {0x00, 0x00, 0x00, 0x11, 0x22, 0x33, 0x44, 0x00};
    value = 0;
    bc_core_load_u32_unaligned(buffer2 + 3, &value);
    assert_int_equal(value, (uint32_t)0x44332211);
}

static void test_load_u64_aligned(void** state)
{
    BC_UNUSED(state);

    const uint8_t buffer[16] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0, 0, 0, 0, 0, 0, 0, 0};
    uint64_t value = 0;
    bc_core_load_u64_unaligned(buffer, &value);
    assert_int_equal(value, (uint64_t)0x8877665544332211ULL);
}

static void test_load_u64_at_each_odd_offset(void** state)
{
    BC_UNUSED(state);

    uint8_t buffer[16] = {0};
    for (size_t i = 0; i < 8; i++) {
        buffer[1 + i] = (uint8_t)(0x11 * (i + 1));
    }
    uint64_t value = 0;
    bc_core_load_u64_unaligned(buffer + 1, &value);
    assert_int_equal(value, (uint64_t)0x8877665544332211ULL);

    uint8_t buffer2[16] = {0};
    for (size_t i = 0; i < 8; i++) {
        buffer2[7 + i] = (uint8_t)(0x11 * (i + 1));
    }
    value = 0;
    bc_core_load_u64_unaligned(buffer2 + 7, &value);
    assert_int_equal(value, (uint64_t)0x8877665544332211ULL);
}

static void test_store_u16_writes_exact_bytes(void** state)
{
    BC_UNUSED(state);

    uint8_t buffer[4] = {0};
    bc_core_store_u16_unaligned(buffer + 1, (uint16_t)0xBEEF);
    assert_int_equal(buffer[0], 0x00);
    assert_int_equal(buffer[1], 0xEF);
    assert_int_equal(buffer[2], 0xBE);
    assert_int_equal(buffer[3], 0x00);
}

static void test_store_u32_writes_exact_bytes(void** state)
{
    BC_UNUSED(state);

    uint8_t buffer[8] = {0};
    bc_core_store_u32_unaligned(buffer + 1, (uint32_t)0xDEADBEEF);
    assert_int_equal(buffer[0], 0x00);
    assert_int_equal(buffer[1], 0xEF);
    assert_int_equal(buffer[2], 0xBE);
    assert_int_equal(buffer[3], 0xAD);
    assert_int_equal(buffer[4], 0xDE);
    assert_int_equal(buffer[5], 0x00);
}

static void test_store_u64_writes_exact_bytes(void** state)
{
    BC_UNUSED(state);

    uint8_t buffer[16] = {0};
    bc_core_store_u64_unaligned(buffer + 3, (uint64_t)0xCAFEF00DDEADBEEFULL);
    assert_int_equal(buffer[0], 0x00);
    assert_int_equal(buffer[1], 0x00);
    assert_int_equal(buffer[2], 0x00);
    assert_int_equal(buffer[3], 0xEF);
    assert_int_equal(buffer[4], 0xBE);
    assert_int_equal(buffer[5], 0xAD);
    assert_int_equal(buffer[6], 0xDE);
    assert_int_equal(buffer[7], 0x0D);
    assert_int_equal(buffer[8], 0xF0);
    assert_int_equal(buffer[9], 0xFE);
    assert_int_equal(buffer[10], 0xCA);
    assert_int_equal(buffer[11], 0x00);
}

static void test_roundtrip_u16_all_offsets(void** state)
{
    BC_UNUSED(state);

    for (size_t offset = 0; offset < 8; offset++) {
        uint8_t buffer[16] = {0};
        const uint16_t original = (uint16_t)(0x1234 + offset);
        bc_core_store_u16_unaligned(buffer + offset, original);
        uint16_t loaded = 0;
        bc_core_load_u16_unaligned(buffer + offset, &loaded);
        assert_int_equal(loaded, original);
    }
}

static void test_roundtrip_u32_all_offsets(void** state)
{
    BC_UNUSED(state);

    for (size_t offset = 0; offset < 8; offset++) {
        uint8_t buffer[16] = {0};
        const uint32_t original = (uint32_t)(0xDEADBEEFu + offset);
        bc_core_store_u32_unaligned(buffer + offset, original);
        uint32_t loaded = 0;
        bc_core_load_u32_unaligned(buffer + offset, &loaded);
        assert_int_equal(loaded, original);
    }
}

static void test_roundtrip_u64_all_offsets(void** state)
{
    BC_UNUSED(state);

    for (size_t offset = 0; offset < 8; offset++) {
        uint8_t buffer[24] = {0};
        const uint64_t original = (uint64_t)(0xCAFEF00DDEADBEEFULL + offset);
        bc_core_store_u64_unaligned(buffer + offset, original);
        uint64_t loaded = 0;
        bc_core_load_u64_unaligned(buffer + offset, &loaded);
        assert_int_equal(loaded, original);
    }
}

static void test_boundary_values(void** state)
{
    BC_UNUSED(state);

    uint8_t buffer[16] = {0};

    bc_core_store_u16_unaligned(buffer, (uint16_t)0);
    uint16_t loaded16 = 0xFFFF;
    bc_core_load_u16_unaligned(buffer, &loaded16);
    assert_int_equal(loaded16, 0);

    bc_core_store_u16_unaligned(buffer, (uint16_t)UINT16_MAX);
    loaded16 = 0;
    bc_core_load_u16_unaligned(buffer, &loaded16);
    assert_int_equal(loaded16, UINT16_MAX);

    bc_core_store_u32_unaligned(buffer, 0u);
    uint32_t loaded32 = 0xFFFFFFFFu;
    bc_core_load_u32_unaligned(buffer, &loaded32);
    assert_int_equal(loaded32, 0u);

    bc_core_store_u32_unaligned(buffer, UINT32_MAX);
    loaded32 = 0;
    bc_core_load_u32_unaligned(buffer, &loaded32);
    assert_int_equal(loaded32, UINT32_MAX);

    bc_core_store_u64_unaligned(buffer, 0ull);
    uint64_t loaded64 = 0xFFFFFFFFFFFFFFFFULL;
    bc_core_load_u64_unaligned(buffer, &loaded64);
    assert_int_equal(loaded64, 0ull);

    bc_core_store_u64_unaligned(buffer, UINT64_MAX);
    loaded64 = 0;
    bc_core_load_u64_unaligned(buffer, &loaded64);
    assert_int_equal(loaded64, UINT64_MAX);
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_load_u16_aligned),
        cmocka_unit_test(test_load_u16_at_odd_offset),
        cmocka_unit_test(test_load_u32_aligned),
        cmocka_unit_test(test_load_u32_at_each_odd_offset),
        cmocka_unit_test(test_load_u64_aligned),
        cmocka_unit_test(test_load_u64_at_each_odd_offset),
        cmocka_unit_test(test_store_u16_writes_exact_bytes),
        cmocka_unit_test(test_store_u32_writes_exact_bytes),
        cmocka_unit_test(test_store_u64_writes_exact_bytes),
        cmocka_unit_test(test_roundtrip_u16_all_offsets),
        cmocka_unit_test(test_roundtrip_u32_all_offsets),
        cmocka_unit_test(test_roundtrip_u64_all_offsets),
        cmocka_unit_test(test_boundary_values),
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}
