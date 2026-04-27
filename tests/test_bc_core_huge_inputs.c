// SPDX-License-Identifier: MIT

#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <cmocka.h>

#include "bc_core.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#if defined(__has_include) && __has_include(<valgrind/valgrind.h>)
#include <valgrind/valgrind.h>
#else
#define RUNNING_ON_VALGRIND 0
#endif

#define BC_HUGE_SIZE_BELOW_INT_MAX ((size_t)2147483647U)
#define BC_HUGE_SIZE_ABOVE_INT_MAX ((size_t)2147483648U)
#define BC_HUGE_SIZE_BIG ((size_t)2400U * 1024U * 1024U)

static void* allocate_huge(size_t length)
{
#if defined(__SANITIZE_THREAD__)
    BC_UNUSED(length);
    return NULL;
#else
    void* mapping = mmap(NULL, length, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (mapping == MAP_FAILED) {
        return NULL;
    }
    return mapping;
#endif
}

static void release_huge(void* mapping, size_t length)
{
    if (mapping != NULL) {
        munmap(mapping, length);
    }
}

static void test_huge_zero_above_int_max(void** state)
{
    BC_UNUSED(state);
    void* buffer = allocate_huge(BC_HUGE_SIZE_ABOVE_INT_MAX);
    if (buffer == NULL) {
        skip();
        return;
    }
    assert_true(bc_core_zero(buffer, BC_HUGE_SIZE_ABOVE_INT_MAX));
    /* Sanity: probe a few offsets including just above the int boundary */
    unsigned char* probe = (unsigned char*)buffer;
    assert_int_equal(probe[0], 0);
    assert_int_equal(probe[BC_HUGE_SIZE_BELOW_INT_MAX], 0);
    assert_int_equal(probe[BC_HUGE_SIZE_ABOVE_INT_MAX - 1U], 0);
    release_huge(buffer, BC_HUGE_SIZE_ABOVE_INT_MAX);
}

static void test_huge_fill_above_int_max(void** state)
{
    BC_UNUSED(state);
    void* buffer = allocate_huge(BC_HUGE_SIZE_BIG);
    if (buffer == NULL) {
        skip();
        return;
    }
    assert_true(bc_core_fill(buffer, BC_HUGE_SIZE_BIG, 0xC3));
    unsigned char* probe = (unsigned char*)buffer;
    assert_int_equal(probe[0], 0xC3);
    assert_int_equal(probe[BC_HUGE_SIZE_BELOW_INT_MAX], 0xC3);
    assert_int_equal(probe[BC_HUGE_SIZE_BIG - 1U], 0xC3);
    release_huge(buffer, BC_HUGE_SIZE_BIG);
}

static void test_huge_copy_above_int_max(void** state)
{
    BC_UNUSED(state);
    void* source = allocate_huge(BC_HUGE_SIZE_BIG);
    void* destination = allocate_huge(BC_HUGE_SIZE_BIG);
    if (source == NULL || destination == NULL) {
        release_huge(source, BC_HUGE_SIZE_BIG);
        release_huge(destination, BC_HUGE_SIZE_BIG);
        skip();
        return;
    }
    assert_true(bc_core_fill(source, BC_HUGE_SIZE_BIG, 0xA5));
    assert_true(bc_core_copy(destination, source, BC_HUGE_SIZE_BIG));
    unsigned char* probe = (unsigned char*)destination;
    assert_int_equal(probe[0], 0xA5);
    assert_int_equal(probe[BC_HUGE_SIZE_BELOW_INT_MAX], 0xA5);
    assert_int_equal(probe[BC_HUGE_SIZE_BIG - 1U], 0xA5);
    release_huge(source, BC_HUGE_SIZE_BIG);
    release_huge(destination, BC_HUGE_SIZE_BIG);
}

static void test_huge_equal_above_int_max(void** state)
{
    BC_UNUSED(state);
    void* buffer_a = allocate_huge(BC_HUGE_SIZE_BIG);
    void* buffer_b = allocate_huge(BC_HUGE_SIZE_BIG);
    if (buffer_a == NULL || buffer_b == NULL) {
        release_huge(buffer_a, BC_HUGE_SIZE_BIG);
        release_huge(buffer_b, BC_HUGE_SIZE_BIG);
        skip();
        return;
    }
    assert_true(bc_core_fill(buffer_a, BC_HUGE_SIZE_BIG, 0x42));
    assert_true(bc_core_fill(buffer_b, BC_HUGE_SIZE_BIG, 0x42));
    bool equal_result = false;
    assert_true(bc_core_equal(buffer_a, buffer_b, BC_HUGE_SIZE_BIG, &equal_result));
    assert_true(equal_result);

    /* Diff at the very end (above the int boundary) */
    ((unsigned char*)buffer_b)[BC_HUGE_SIZE_BIG - 1U] = 0x43;
    assert_true(bc_core_equal(buffer_a, buffer_b, BC_HUGE_SIZE_BIG, &equal_result));
    assert_false(equal_result);

    release_huge(buffer_a, BC_HUGE_SIZE_BIG);
    release_huge(buffer_b, BC_HUGE_SIZE_BIG);
}

static void test_huge_compare_above_int_max(void** state)
{
    BC_UNUSED(state);
    void* buffer_a = allocate_huge(BC_HUGE_SIZE_BIG);
    void* buffer_b = allocate_huge(BC_HUGE_SIZE_BIG);
    if (buffer_a == NULL || buffer_b == NULL) {
        release_huge(buffer_a, BC_HUGE_SIZE_BIG);
        release_huge(buffer_b, BC_HUGE_SIZE_BIG);
        skip();
        return;
    }
    assert_true(bc_core_fill(buffer_a, BC_HUGE_SIZE_BIG, 0x42));
    assert_true(bc_core_fill(buffer_b, BC_HUGE_SIZE_BIG, 0x42));
    int compare_result = 99;
    assert_true(bc_core_compare(buffer_a, buffer_b, BC_HUGE_SIZE_BIG, &compare_result));
    assert_int_equal(compare_result, 0);

    ((unsigned char*)buffer_a)[BC_HUGE_SIZE_BIG - 1U] = 0x10;
    ((unsigned char*)buffer_b)[BC_HUGE_SIZE_BIG - 1U] = 0x80;
    assert_true(bc_core_compare(buffer_a, buffer_b, BC_HUGE_SIZE_BIG, &compare_result));
    assert_int_equal(compare_result, -1);

    release_huge(buffer_a, BC_HUGE_SIZE_BIG);
    release_huge(buffer_b, BC_HUGE_SIZE_BIG);
}

static void test_huge_find_byte_match_above_int_max(void** state)
{
    BC_UNUSED(state);
    void* buffer = allocate_huge(BC_HUGE_SIZE_BIG);
    if (buffer == NULL) {
        skip();
        return;
    }
    assert_true(bc_core_fill(buffer, BC_HUGE_SIZE_BIG, 0x42));
    /* Place a unique marker just above the 32-bit boundary */
    size_t marker_offset = BC_HUGE_SIZE_ABOVE_INT_MAX + 1024U;
    ((unsigned char*)buffer)[marker_offset] = 0xAA;

    size_t found_offset = 0;
    assert_true(bc_core_find_byte(buffer, BC_HUGE_SIZE_BIG, 0xAA, &found_offset));
    assert_int_equal(found_offset, marker_offset);

    release_huge(buffer, BC_HUGE_SIZE_BIG);
}

static void test_huge_count_byte_above_int_max(void** state)
{
    BC_UNUSED(state);
    void* buffer = allocate_huge(BC_HUGE_SIZE_BIG);
    if (buffer == NULL) {
        skip();
        return;
    }
    assert_true(bc_core_fill(buffer, BC_HUGE_SIZE_BIG, 0x00));
    /* Place 7 markers, with one above the int boundary */
    ((unsigned char*)buffer)[100U] = 0xFF;
    ((unsigned char*)buffer)[1000U] = 0xFF;
    ((unsigned char*)buffer)[100000U] = 0xFF;
    ((unsigned char*)buffer)[10000000U] = 0xFF;
    ((unsigned char*)buffer)[BC_HUGE_SIZE_BELOW_INT_MAX - 1U] = 0xFF;
    ((unsigned char*)buffer)[BC_HUGE_SIZE_ABOVE_INT_MAX] = 0xFF;
    ((unsigned char*)buffer)[BC_HUGE_SIZE_BIG - 1U] = 0xFF;

    size_t count = 0;
    assert_true(bc_core_count_byte(buffer, BC_HUGE_SIZE_BIG, 0xFF, &count));
    assert_int_equal(count, 7U);

    release_huge(buffer, BC_HUGE_SIZE_BIG);
}

static void test_huge_sha256_above_int_max(void** state)
{
    BC_UNUSED(state);
    if (RUNNING_ON_VALGRIND) {
        skip();
        return;
    }
    void* buffer = allocate_huge(BC_HUGE_SIZE_BIG);
    if (buffer == NULL) {
        skip();
        return;
    }
    assert_true(bc_core_fill(buffer, BC_HUGE_SIZE_BIG, 0x55));

    unsigned char digest_full[32];
    bc_core_sha256(buffer, BC_HUGE_SIZE_BIG, digest_full);

    /* Confirm same digest computed incrementally in 256 MB blocks */
    bc_core_sha256_context_t context;
    bc_core_sha256_init(&context);
    const size_t block_size = 256U * 1024U * 1024U;
    size_t offset = 0;
    while (offset < BC_HUGE_SIZE_BIG) {
        size_t chunk = block_size;
        if (offset + chunk > BC_HUGE_SIZE_BIG) {
            chunk = BC_HUGE_SIZE_BIG - offset;
        }
        bc_core_sha256_update(&context, (const unsigned char*)buffer + offset, chunk);
        offset += chunk;
    }
    unsigned char digest_incremental[32];
    bc_core_sha256_finalize(&context, digest_incremental);

    assert_memory_equal(digest_full, digest_incremental, 32);

    release_huge(buffer, BC_HUGE_SIZE_BIG);
}

static void test_huge_crc32c_above_int_max(void** state)
{
    BC_UNUSED(state);
    void* buffer = allocate_huge(BC_HUGE_SIZE_BIG);
    if (buffer == NULL) {
        skip();
        return;
    }
    assert_true(bc_core_fill(buffer, BC_HUGE_SIZE_BIG, 0xCC));

    uint32_t crc_full = 0;
    bc_core_crc32c(buffer, BC_HUGE_SIZE_BIG, &crc_full);

    /* Same value via update across two halves */
    uint32_t crc_first = 0;
    bc_core_crc32c(buffer, BC_HUGE_SIZE_BELOW_INT_MAX, &crc_first);
    uint32_t crc_total = 0;
    bc_core_crc32c_update(crc_first, (const unsigned char*)buffer + BC_HUGE_SIZE_BELOW_INT_MAX,
                          BC_HUGE_SIZE_BIG - BC_HUGE_SIZE_BELOW_INT_MAX, &crc_total);

    assert_int_equal(crc_full, crc_total);

    release_huge(buffer, BC_HUGE_SIZE_BIG);
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_huge_zero_above_int_max),       cmocka_unit_test(test_huge_fill_above_int_max),
        cmocka_unit_test(test_huge_copy_above_int_max),       cmocka_unit_test(test_huge_equal_above_int_max),
        cmocka_unit_test(test_huge_compare_above_int_max),    cmocka_unit_test(test_huge_find_byte_match_above_int_max),
        cmocka_unit_test(test_huge_count_byte_above_int_max), cmocka_unit_test(test_huge_sha256_above_int_max),
        cmocka_unit_test(test_huge_crc32c_above_int_max),
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}
