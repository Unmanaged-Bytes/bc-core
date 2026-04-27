// SPDX-License-Identifier: MIT

#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <cmocka.h>

#include "bc_core.h"

#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>

#define BC_TEST_PAYLOAD_LENGTH ((size_t)256)

static unsigned char bc_test_static_buffer_a[BC_TEST_PAYLOAD_LENGTH];
static unsigned char bc_test_static_buffer_b[BC_TEST_PAYLOAD_LENGTH];

static void prime_buffer(unsigned char* buffer, size_t length, unsigned char value)
{
    bc_core_fill(buffer, length, value);
}

static void test_equal_malloc_buffers_identical(void** state)
{
    BC_UNUSED(state);
    unsigned char* a = malloc(BC_TEST_PAYLOAD_LENGTH);
    unsigned char* b = malloc(BC_TEST_PAYLOAD_LENGTH);
    assert_non_null(a);
    assert_non_null(b);
    prime_buffer(a, BC_TEST_PAYLOAD_LENGTH, (unsigned char)0x55);
    prime_buffer(b, BC_TEST_PAYLOAD_LENGTH, (unsigned char)0x55);
    bool equal_result = false;
    assert_true(bc_core_equal(a, b, BC_TEST_PAYLOAD_LENGTH, &equal_result));
    assert_true(equal_result);
    free(a);
    free(b);
}

static void test_equal_aligned_alloc_8_byte_alignment(void** state)
{
    BC_UNUSED(state);
    unsigned char* a = aligned_alloc(8, BC_TEST_PAYLOAD_LENGTH);
    unsigned char* b = aligned_alloc(8, BC_TEST_PAYLOAD_LENGTH);
    assert_non_null(a);
    assert_non_null(b);
    prime_buffer(a, BC_TEST_PAYLOAD_LENGTH, (unsigned char)0xA5);
    prime_buffer(b, BC_TEST_PAYLOAD_LENGTH, (unsigned char)0xA5);
    bool equal_result = false;
    assert_true(bc_core_equal(a, b, BC_TEST_PAYLOAD_LENGTH, &equal_result));
    assert_true(equal_result);
    free(a);
    free(b);
}

static void test_equal_aligned_alloc_64_byte_alignment(void** state)
{
    BC_UNUSED(state);
    unsigned char* a = aligned_alloc(64, BC_TEST_PAYLOAD_LENGTH);
    unsigned char* b = aligned_alloc(64, BC_TEST_PAYLOAD_LENGTH);
    assert_non_null(a);
    assert_non_null(b);
    prime_buffer(a, BC_TEST_PAYLOAD_LENGTH, (unsigned char)0x33);
    prime_buffer(b, BC_TEST_PAYLOAD_LENGTH, (unsigned char)0x33);
    bool equal_result = false;
    assert_true(bc_core_equal(a, b, BC_TEST_PAYLOAD_LENGTH, &equal_result));
    assert_true(equal_result);
    free(a);
    free(b);
}

static void test_equal_aligned_alloc_4096_page_alignment(void** state)
{
    BC_UNUSED(state);
    unsigned char* a = aligned_alloc(4096, 4096);
    unsigned char* b = aligned_alloc(4096, 4096);
    assert_non_null(a);
    assert_non_null(b);
    prime_buffer(a, 4096, (unsigned char)0x77);
    prime_buffer(b, 4096, (unsigned char)0x77);
    bool equal_result = false;
    assert_true(bc_core_equal(a, b, 4096, &equal_result));
    assert_true(equal_result);
    free(a);
    free(b);
}

static void test_equal_mmap_anonymous_page_aligned(void** state)
{
    BC_UNUSED(state);
    unsigned char* a = mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    unsigned char* b = mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    assert_true(a != MAP_FAILED);
    assert_true(b != MAP_FAILED);
    prime_buffer(a, 4096, (unsigned char)0xC3);
    prime_buffer(b, 4096, (unsigned char)0xC3);
    bool equal_result = false;
    assert_true(bc_core_equal(a, b, 4096, &equal_result));
    assert_true(equal_result);
    munmap(a, 4096);
    munmap(b, 4096);
}

static void test_equal_stack_buffers(void** state)
{
    BC_UNUSED(state);
    unsigned char a[BC_TEST_PAYLOAD_LENGTH];
    unsigned char b[BC_TEST_PAYLOAD_LENGTH];
    prime_buffer(a, BC_TEST_PAYLOAD_LENGTH, (unsigned char)0x88);
    prime_buffer(b, BC_TEST_PAYLOAD_LENGTH, (unsigned char)0x88);
    bool equal_result = false;
    assert_true(bc_core_equal(a, b, BC_TEST_PAYLOAD_LENGTH, &equal_result));
    assert_true(equal_result);
}

static void test_equal_static_buffers(void** state)
{
    BC_UNUSED(state);
    prime_buffer(bc_test_static_buffer_a, BC_TEST_PAYLOAD_LENGTH, (unsigned char)0xDD);
    prime_buffer(bc_test_static_buffer_b, BC_TEST_PAYLOAD_LENGTH, (unsigned char)0xDD);
    bool equal_result = false;
    assert_true(bc_core_equal(bc_test_static_buffer_a, bc_test_static_buffer_b, BC_TEST_PAYLOAD_LENGTH, &equal_result));
    assert_true(equal_result);
}

static void test_equal_mixed_sources_mmap_vs_malloc(void** state)
{
    BC_UNUSED(state);
    unsigned char* a = mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    unsigned char* b = malloc(4096);
    assert_true(a != MAP_FAILED);
    assert_non_null(b);
    prime_buffer(a, 4096, (unsigned char)0x11);
    prime_buffer(b, 4096, (unsigned char)0x11);
    bool equal_result = false;
    assert_true(bc_core_equal(a, b, 4096, &equal_result));
    assert_true(equal_result);
    munmap(a, 4096);
    free(b);
}

static void test_equal_unaligned_offsets_across_alignments(void** state)
{
    BC_UNUSED(state);
    static const size_t offsets[] = {1, 3, 5, 7, 13, 17, 23, 29, 31, 33, 47, 63, 65, 95, 97, 127};
    static const size_t lengths[] = {32, 64, 128, 129, 192, 256};

    for (size_t offset_index = 0; offset_index < sizeof(offsets) / sizeof(offsets[0]); ++offset_index) {
        for (size_t length_index = 0; length_index < sizeof(lengths) / sizeof(lengths[0]); ++length_index) {
            size_t offset = offsets[offset_index];
            size_t length = lengths[length_index];
            size_t padded_size = offset + length + 64;
            unsigned char* a = malloc(padded_size);
            unsigned char* b = malloc(padded_size);
            assert_non_null(a);
            assert_non_null(b);
            prime_buffer(a, padded_size, (unsigned char)0x6B);
            prime_buffer(b, padded_size, (unsigned char)0x6B);
            bool equal_result = false;
            assert_true(bc_core_equal(a + offset, b + offset, length, &equal_result));
            assert_true(equal_result);

            b[offset + length - 1] = (unsigned char)0xFA;
            equal_result = true;
            assert_true(bc_core_equal(a + offset, b + offset, length, &equal_result));
            assert_false(equal_result);
            free(a);
            free(b);
        }
    }
}

static void test_compare_mmap_vs_malloc_difference_at_boundary(void** state)
{
    BC_UNUSED(state);
    unsigned char* a = mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    unsigned char* b = malloc(4096);
    assert_true(a != MAP_FAILED);
    assert_non_null(b);
    prime_buffer(a, 4096, (unsigned char)0x42);
    prime_buffer(b, 4096, (unsigned char)0x42);
    a[127] = (unsigned char)0x80;
    b[127] = (unsigned char)0x10;
    int compare_result = 99;
    assert_true(bc_core_compare(a, b, 4096, &compare_result));
    assert_int_equal(compare_result, 1);
    munmap(a, 4096);
    free(b);
}

static void test_compare_aligned_alloc_4096_difference_at_seam_128(void** state)
{
    BC_UNUSED(state);
    unsigned char* a = aligned_alloc(4096, 4096);
    unsigned char* b = aligned_alloc(4096, 4096);
    assert_non_null(a);
    assert_non_null(b);
    prime_buffer(a, 4096, (unsigned char)0x42);
    prime_buffer(b, 4096, (unsigned char)0x42);
    a[128] = (unsigned char)0x10;
    b[128] = (unsigned char)0x80;
    int compare_result = 99;
    assert_true(bc_core_compare(a, b, 4096, &compare_result));
    assert_int_equal(compare_result, -1);
    free(a);
    free(b);
}

static void test_compare_unaligned_offsets_across_alignments(void** state)
{
    BC_UNUSED(state);
    static const size_t offsets[] = {1, 7, 13, 31, 33, 63, 95, 127};
    static const size_t lengths[] = {64, 128, 129, 192, 256, 384};

    for (size_t offset_index = 0; offset_index < sizeof(offsets) / sizeof(offsets[0]); ++offset_index) {
        for (size_t length_index = 0; length_index < sizeof(lengths) / sizeof(lengths[0]); ++length_index) {
            size_t offset = offsets[offset_index];
            size_t length = lengths[length_index];
            size_t padded_size = offset + length + 64;
            unsigned char* a = malloc(padded_size);
            unsigned char* b = malloc(padded_size);
            assert_non_null(a);
            assert_non_null(b);
            prime_buffer(a, padded_size, (unsigned char)0x55);
            prime_buffer(b, padded_size, (unsigned char)0x55);
            int compare_result = 99;
            assert_true(bc_core_compare(a + offset, b + offset, length, &compare_result));
            assert_int_equal(compare_result, 0);

            a[offset + length / 2] = (unsigned char)0x10;
            b[offset + length / 2] = (unsigned char)0x80;
            assert_true(bc_core_compare(a + offset, b + offset, length, &compare_result));
            assert_int_equal(compare_result, -1);
            free(a);
            free(b);
        }
    }
}

static void test_equal_at_mmap_page_boundary(void** state)
{
    BC_UNUSED(state);
    unsigned char* region = mmap(NULL, 8192, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    assert_true(region != MAP_FAILED);
    prime_buffer(region, 8192, (unsigned char)0xAB);
    bool equal_result = false;
    assert_true(bc_core_equal(region + 4032, region + 4032 + 64, 128, &equal_result));
    assert_true(equal_result);
    munmap(region, 8192);
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_equal_malloc_buffers_identical),
        cmocka_unit_test(test_equal_aligned_alloc_8_byte_alignment),
        cmocka_unit_test(test_equal_aligned_alloc_64_byte_alignment),
        cmocka_unit_test(test_equal_aligned_alloc_4096_page_alignment),
        cmocka_unit_test(test_equal_mmap_anonymous_page_aligned),
        cmocka_unit_test(test_equal_stack_buffers),
        cmocka_unit_test(test_equal_static_buffers),
        cmocka_unit_test(test_equal_mixed_sources_mmap_vs_malloc),
        cmocka_unit_test(test_equal_unaligned_offsets_across_alignments),
        cmocka_unit_test(test_compare_mmap_vs_malloc_difference_at_boundary),
        cmocka_unit_test(test_compare_aligned_alloc_4096_difference_at_seam_128),
        cmocka_unit_test(test_compare_unaligned_offsets_across_alignments),
        cmocka_unit_test(test_equal_at_mmap_page_boundary),
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}
