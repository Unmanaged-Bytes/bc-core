// SPDX-License-Identifier: MIT

#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <cmocka.h>

#include "bc_core.h"

#include <stdint.h>
#include <stdlib.h>
#include <sys/mman.h>

static bool less_than_int32(const void* left, const void* right, void* user_data)
{
    BC_UNUSED(user_data);
    int32_t left_value = *(const int32_t*)left;
    int32_t right_value = *(const int32_t*)right;
    return left_value < right_value;
}

static bool less_than_uint64(const void* left, const void* right, void* user_data)
{
    BC_UNUSED(user_data);
    uint64_t left_value = *(const uint64_t*)left;
    uint64_t right_value = *(const uint64_t*)right;
    return left_value < right_value;
}

static bool greater_than_int32(const void* left, const void* right, void* user_data)
{
    BC_UNUSED(user_data);
    int32_t left_value = *(const int32_t*)left;
    int32_t right_value = *(const int32_t*)right;
    return left_value > right_value;
}

typedef struct {
    int key;
    int payload;
} pair_t;

static bool less_than_pair_by_key(const void* left, const void* right, void* user_data)
{
    BC_UNUSED(user_data);
    const pair_t* left_pair = (const pair_t*)left;
    const pair_t* right_pair = (const pair_t*)right;
    return left_pair->key < right_pair->key;
}

static bool counting_less_than(const void* left, const void* right, void* user_data)
{
    size_t* counter = (size_t*)user_data;
    *counter += 1U;
    return *(const int32_t*)left < *(const int32_t*)right;
}

static void test_sort_empty(void** state)
{
    BC_UNUSED(state);
    int32_t array[1] = {42};
    assert_true(bc_core_sort_with_compare(array, 0, sizeof(int32_t), less_than_int32, NULL));
    assert_int_equal(array[0], 42);
}

static void test_sort_single(void** state)
{
    BC_UNUSED(state);
    int32_t array[1] = {42};
    assert_true(bc_core_sort_with_compare(array, 1, sizeof(int32_t), less_than_int32, NULL));
    assert_int_equal(array[0], 42);
}

static void test_sort_two_already_sorted(void** state)
{
    BC_UNUSED(state);
    int32_t array[] = {1, 2};
    assert_true(bc_core_sort_with_compare(array, 2, sizeof(int32_t), less_than_int32, NULL));
    assert_int_equal(array[0], 1);
    assert_int_equal(array[1], 2);
}

static void test_sort_two_reversed(void** state)
{
    BC_UNUSED(state);
    int32_t array[] = {5, 1};
    assert_true(bc_core_sort_with_compare(array, 2, sizeof(int32_t), less_than_int32, NULL));
    assert_int_equal(array[0], 1);
    assert_int_equal(array[1], 5);
}

static void test_sort_small_random(void** state)
{
    BC_UNUSED(state);
    int32_t array[] = {7, 3, 9, 1, 5, 8, 2, 6, 4, 0};
    size_t count = sizeof(array) / sizeof(array[0]);
    assert_true(bc_core_sort_with_compare(array, count, sizeof(int32_t), less_than_int32, NULL));
    for (size_t index = 0; index < count; ++index) {
        assert_int_equal(array[index], (int32_t)index);
    }
}

static void test_sort_large_random(void** state)
{
    BC_UNUSED(state);
    enum { COUNT = 1024 };
    int32_t* array = malloc(sizeof(int32_t) * COUNT);
    assert_non_null(array);
    srand(0x4242);
    for (size_t index = 0; index < COUNT; ++index) {
        array[index] = rand();
    }
    assert_true(bc_core_sort_with_compare(array, COUNT, sizeof(int32_t), less_than_int32, NULL));
    for (size_t index = 1; index < COUNT; ++index) {
        assert_true(array[index - 1] <= array[index]);
    }
    free(array);
}

static void test_sort_already_sorted(void** state)
{
    BC_UNUSED(state);
    enum { COUNT = 256 };
    int32_t array[COUNT];
    for (size_t index = 0; index < COUNT; ++index) {
        array[index] = (int32_t)index;
    }
    assert_true(bc_core_sort_with_compare(array, COUNT, sizeof(int32_t), less_than_int32, NULL));
    for (size_t index = 0; index < COUNT; ++index) {
        assert_int_equal(array[index], (int32_t)index);
    }
}

static void test_sort_reverse_sorted(void** state)
{
    BC_UNUSED(state);
    enum { COUNT = 256 };
    int32_t array[COUNT];
    for (size_t index = 0; index < COUNT; ++index) {
        array[index] = (int32_t)(COUNT - 1 - index);
    }
    assert_true(bc_core_sort_with_compare(array, COUNT, sizeof(int32_t), less_than_int32, NULL));
    for (size_t index = 0; index < COUNT; ++index) {
        assert_int_equal(array[index], (int32_t)index);
    }
}

static void test_sort_all_duplicates(void** state)
{
    BC_UNUSED(state);
    int32_t array[64];
    for (size_t index = 0; index < 64; ++index) {
        array[index] = 7;
    }
    assert_true(bc_core_sort_with_compare(array, 64, sizeof(int32_t), less_than_int32, NULL));
    for (size_t index = 0; index < 64; ++index) {
        assert_int_equal(array[index], 7);
    }
}

static void test_sort_mostly_duplicates(void** state)
{
    BC_UNUSED(state);
    int32_t array[] = {3, 1, 2, 1, 3, 2, 1, 3, 2, 1, 1, 2, 3, 2, 1};
    size_t count = sizeof(array) / sizeof(array[0]);
    assert_true(bc_core_sort_with_compare(array, count, sizeof(int32_t), less_than_int32, NULL));
    for (size_t index = 1; index < count; ++index) {
        assert_true(array[index - 1] <= array[index]);
    }
}

static void test_sort_descending_order(void** state)
{
    BC_UNUSED(state);
    int32_t array[] = {3, 1, 4, 1, 5, 9, 2, 6, 5, 3, 5, 8, 9, 7, 9};
    size_t count = sizeof(array) / sizeof(array[0]);
    assert_true(bc_core_sort_with_compare(array, count, sizeof(int32_t), greater_than_int32, NULL));
    for (size_t index = 1; index < count; ++index) {
        assert_true(array[index - 1] >= array[index]);
    }
}

static void test_sort_uint64(void** state)
{
    BC_UNUSED(state);
    uint64_t array[] = {UINT64_MAX, 0, 100, UINT64_MAX - 1U, 1, 50};
    size_t count = sizeof(array) / sizeof(array[0]);
    assert_true(bc_core_sort_with_compare(array, count, sizeof(uint64_t), less_than_uint64, NULL));
    assert_int_equal((int)array[0], 0);
    assert_int_equal((int)array[1], 1);
    assert_int_equal((int)array[2], 50);
    assert_int_equal((int)array[3], 100);
    assert_true(array[4] == UINT64_MAX - 1U);
    assert_true(array[5] == UINT64_MAX);
}

static void test_sort_struct_by_key(void** state)
{
    BC_UNUSED(state);
    pair_t array[] = {
        {3, 100}, {1, 200}, {4, 300}, {1, 400}, {5, 500},
    };
    size_t count = sizeof(array) / sizeof(array[0]);
    assert_true(bc_core_sort_with_compare(array, count, sizeof(pair_t), less_than_pair_by_key, NULL));
    for (size_t index = 1; index < count; ++index) {
        assert_true(array[index - 1].key <= array[index].key);
    }
}

static void test_sort_user_data_passed_through(void** state)
{
    BC_UNUSED(state);
    int32_t array[] = {5, 3, 8, 1, 9, 4, 2, 7, 6, 0};
    size_t comparison_count = 0;
    assert_true(bc_core_sort_with_compare(array, 10, sizeof(int32_t), counting_less_than, &comparison_count));
    assert_true(comparison_count > 0);
}

static void test_sort_rejects_null_compare(void** state)
{
    BC_UNUSED(state);
    int32_t array[] = {1, 2, 3};
    assert_false(bc_core_sort_with_compare(array, 3, sizeof(int32_t), NULL, NULL));
}

static void test_sort_rejects_zero_element_size(void** state)
{
    BC_UNUSED(state);
    int32_t array[] = {1, 2, 3};
    assert_false(bc_core_sort_with_compare(array, 3, 0, less_than_int32, NULL));
}

static bool less_than_first_byte(const void* left, const void* right, void* user_data)
{
    BC_UNUSED(user_data);
    return *(const unsigned char*)left < *(const unsigned char*)right;
}

static void run_sort_arbitrary_element_size(size_t element_size)
{
    enum { COUNT = 256 };
    unsigned char* array = malloc(element_size * COUNT);
    assert_non_null(array);
    /* First byte = key (descending in input), remaining bytes carry payload identity */
    for (size_t index = 0; index < COUNT; ++index) {
        unsigned char* element = array + index * element_size;
        element[0] = (unsigned char)(255U - index);
        for (size_t inner = 1; inner < element_size; ++inner) {
            element[inner] = (unsigned char)((index * 7U + inner) & 0xFFU);
        }
    }
    assert_true(bc_core_sort_with_compare(array, COUNT, element_size, less_than_first_byte, NULL));
    for (size_t index = 1; index < COUNT; ++index) {
        unsigned char prev_key = array[(index - 1) * element_size];
        unsigned char curr_key = array[index * element_size];
        assert_true(prev_key <= curr_key);
    }
    free(array);
}

static void test_sort_element_size_1(void** state)
{
    BC_UNUSED(state);
    run_sort_arbitrary_element_size(1);
}

static void test_sort_element_size_2(void** state)
{
    BC_UNUSED(state);
    run_sort_arbitrary_element_size(2);
}

static void test_sort_element_size_3(void** state)
{
    BC_UNUSED(state);
    run_sort_arbitrary_element_size(3);
}

static void test_sort_element_size_5(void** state)
{
    BC_UNUSED(state);
    run_sort_arbitrary_element_size(5);
}

static void test_sort_element_size_6(void** state)
{
    BC_UNUSED(state);
    run_sort_arbitrary_element_size(6);
}

static void test_sort_element_size_7(void** state)
{
    BC_UNUSED(state);
    run_sort_arbitrary_element_size(7);
}

static void test_sort_element_size_9(void** state)
{
    BC_UNUSED(state);
    run_sort_arbitrary_element_size(9);
}

static void test_sort_element_size_10(void** state)
{
    BC_UNUSED(state);
    run_sort_arbitrary_element_size(10);
}

static void test_sort_element_size_11(void** state)
{
    BC_UNUSED(state);
    run_sort_arbitrary_element_size(11);
}

static void test_sort_element_size_13(void** state)
{
    BC_UNUSED(state);
    run_sort_arbitrary_element_size(13);
}

static void test_sort_element_size_15(void** state)
{
    BC_UNUSED(state);
    run_sort_arbitrary_element_size(15);
}

static void test_sort_element_size_17(void** state)
{
    BC_UNUSED(state);
    run_sort_arbitrary_element_size(17);
}

static void test_sort_element_size_23(void** state)
{
    BC_UNUSED(state);
    run_sort_arbitrary_element_size(23);
}

static void test_sort_element_size_31(void** state)
{
    BC_UNUSED(state);
    run_sort_arbitrary_element_size(31);
}

static void test_sort_element_size_33(void** state)
{
    BC_UNUSED(state);
    run_sort_arbitrary_element_size(33);
}

static void test_sort_element_size_41(void** state)
{
    BC_UNUSED(state);
    run_sort_arbitrary_element_size(41);
}

static void test_sort_element_size_63(void** state)
{
    BC_UNUSED(state);
    run_sort_arbitrary_element_size(63);
}

static void test_sort_element_size_65(void** state)
{
    BC_UNUSED(state);
    run_sort_arbitrary_element_size(65);
}

static void test_sort_element_size_100(void** state)
{
    BC_UNUSED(state);
    run_sort_arbitrary_element_size(100);
}

static void test_sort_element_size_127(void** state)
{
    BC_UNUSED(state);
    run_sort_arbitrary_element_size(127);
}

static void test_sort_element_size_payload_integrity(void** state)
{
    BC_UNUSED(state);
    /* Verify that for non-power-of-2 sizes, the payload bytes travel with the key */
    typedef struct {
        unsigned char key;
        unsigned char tag[6]; /* 7-byte struct, hits the <=16 path */
    } seven_byte_struct_t;
    enum { COUNT = 64 };
    seven_byte_struct_t array[COUNT];
    for (size_t index = 0; index < COUNT; ++index) {
        array[index].key = (unsigned char)(COUNT - index);
        for (size_t inner = 0; inner < 6; ++inner) {
            array[index].tag[inner] = (unsigned char)(index * 7U + inner);
        }
    }
    assert_true(bc_core_sort_with_compare(array, COUNT, sizeof(seven_byte_struct_t), less_than_first_byte, NULL));
    /* Recover original index from sorted key (key = COUNT - original_index) */
    for (size_t index = 0; index < COUNT; ++index) {
        unsigned char key_value = array[index].key;
        unsigned char original_index = (unsigned char)(COUNT - key_value);
        for (size_t inner = 0; inner < 6; ++inner) {
            unsigned char expected = (unsigned char)((size_t)original_index * 7U + inner);
            assert_int_equal(array[index].tag[inner], expected);
        }
    }
}

static void test_sort_null_base_with_count_zero(void** state)
{
    BC_UNUSED(state);
    assert_true(bc_core_sort_with_compare(NULL, 0, sizeof(int32_t), less_than_int32, NULL));
}

static void test_sort_null_base_with_count_one(void** state)
{
    BC_UNUSED(state);
    assert_true(bc_core_sort_with_compare(NULL, 1, sizeof(int32_t), less_than_int32, NULL));
}

typedef struct {
    uint64_t key;
    uint32_t payload[3];
} large_struct_12_words_t;

static bool less_than_large_struct(const void* left, const void* right, void* user_data)
{
    BC_UNUSED(user_data);
    return ((const large_struct_12_words_t*)left)->key < ((const large_struct_12_words_t*)right)->key;
}

static void test_sort_element_size_4_uint32(void** state)
{
    BC_UNUSED(state);
    enum { COUNT = 64 };
    uint32_t array[COUNT];
    for (size_t index = 0; index < COUNT; ++index) {
        array[index] = (uint32_t)((COUNT - index) * 17U);
    }
    assert_true(bc_core_sort_with_compare(array, COUNT, sizeof(uint32_t), less_than_int32, NULL));
    for (size_t index = 1; index < COUNT; ++index) {
        assert_true(array[index - 1] <= array[index]);
    }
}

static void test_sort_element_size_8_uint64_already_sorted(void** state)
{
    BC_UNUSED(state);
    enum { COUNT = 128 };
    uint64_t array[COUNT];
    for (size_t index = 0; index < COUNT; ++index) {
        array[index] = (uint64_t)index;
    }
    assert_true(bc_core_sort_with_compare(array, COUNT, sizeof(uint64_t), less_than_uint64, NULL));
    for (size_t index = 0; index < COUNT; ++index) {
        assert_true(array[index] == (uint64_t)index);
    }
}

static void test_sort_element_size_16_pair(void** state)
{
    BC_UNUSED(state);
    typedef struct {
        uint64_t key;
        uint64_t payload;
    } pair16_t;
    enum { COUNT = 64 };
    pair16_t array[COUNT];
    for (size_t index = 0; index < COUNT; ++index) {
        array[index].key = (uint64_t)(COUNT - index);
        array[index].payload = (uint64_t)index;
    }
    assert_true(bc_core_sort_with_compare(array, COUNT, sizeof(pair16_t), less_than_uint64, NULL));
    for (size_t index = 1; index < COUNT; ++index) {
        assert_true(array[index - 1].key <= array[index].key);
    }
}

static void test_sort_element_size_24_large_struct(void** state)
{
    BC_UNUSED(state);
    enum { COUNT = 64 };
    large_struct_12_words_t array[COUNT];
    for (size_t index = 0; index < COUNT; ++index) {
        array[index].key = (uint64_t)(COUNT - index);
        array[index].payload[0] = (uint32_t)index;
        array[index].payload[1] = (uint32_t)(index * 2U);
        array[index].payload[2] = (uint32_t)(index * 3U);
    }
    assert_true(bc_core_sort_with_compare(array, COUNT, sizeof(large_struct_12_words_t), less_than_large_struct, NULL));
    for (size_t index = 1; index < COUNT; ++index) {
        assert_true(array[index - 1].key <= array[index].key);
    }
}

static void test_sort_element_size_32_struct(void** state)
{
    BC_UNUSED(state);
    typedef struct {
        uint64_t key;
        uint64_t payload[3];
    } struct32_t;
    enum { COUNT = 128 };
    struct32_t* array = malloc(sizeof(struct32_t) * COUNT);
    assert_non_null(array);
    for (size_t index = 0; index < COUNT; ++index) {
        array[index].key = (uint64_t)((COUNT - index) * 13U);
        array[index].payload[0] = (uint64_t)index;
        array[index].payload[1] = 0xDEADBEEFU;
        array[index].payload[2] = 0xCAFEBABEU;
    }
    assert_true(bc_core_sort_with_compare(array, COUNT, sizeof(struct32_t), less_than_uint64, NULL));
    for (size_t index = 1; index < COUNT; ++index) {
        assert_true(array[index - 1].key <= array[index].key);
        assert_true(array[index].payload[1] == 0xDEADBEEFU);
        assert_true(array[index].payload[2] == 0xCAFEBABEU);
    }
    free(array);
}

static void test_sort_element_size_64_huge_struct(void** state)
{
    BC_UNUSED(state);
    typedef struct {
        uint64_t key;
        uint64_t padding[7];
    } struct64_t;
    enum { COUNT = 64 };
    struct64_t* array = malloc(sizeof(struct64_t) * COUNT);
    assert_non_null(array);
    for (size_t index = 0; index < COUNT; ++index) {
        array[index].key = (uint64_t)(COUNT - index);
        for (size_t inner = 0; inner < 7; ++inner) {
            array[index].padding[inner] = (uint64_t)(index * 100U + inner);
        }
    }
    assert_true(bc_core_sort_with_compare(array, COUNT, sizeof(struct64_t), less_than_uint64, NULL));
    for (size_t index = 1; index < COUNT; ++index) {
        assert_true(array[index - 1].key <= array[index].key);
    }
    free(array);
}

static void test_sort_mmap_buffer(void** state)
{
    BC_UNUSED(state);
    enum { COUNT = 4096 };
    uint64_t* array = mmap(NULL, sizeof(uint64_t) * COUNT, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    assert_true(array != MAP_FAILED);
    for (size_t index = 0; index < COUNT; ++index) {
        array[index] = (uint64_t)((COUNT - index) * 31U);
    }
    assert_true(bc_core_sort_with_compare(array, COUNT, sizeof(uint64_t), less_than_uint64, NULL));
    for (size_t index = 1; index < COUNT; ++index) {
        assert_true(array[index - 1] <= array[index]);
    }
    munmap(array, sizeof(uint64_t) * COUNT);
}

static void test_sort_pathological_input_triggers_heap_fallback(void** state)
{
    BC_UNUSED(state);
    enum { COUNT = 4096 };
    int32_t* array = malloc(sizeof(int32_t) * COUNT);
    assert_non_null(array);
    /* Pathological: alternating max/min values that defeat mid pivot */
    for (size_t index = 0; index < COUNT; ++index) {
        if (index % 2U == 0U) {
            array[index] = (int32_t)(index / 2U);
        } else {
            array[index] = (int32_t)(COUNT - 1U - (index / 2U));
        }
    }
    assert_true(bc_core_sort_with_compare(array, COUNT, sizeof(int32_t), less_than_int32, NULL));
    for (size_t index = 1; index < COUNT; ++index) {
        assert_true(array[index - 1] <= array[index]);
    }
    free(array);
}

static void test_sort_many_duplicates_large(void** state)
{
    BC_UNUSED(state);
    enum { COUNT = 2048 };
    int32_t* array = malloc(sizeof(int32_t) * COUNT);
    assert_non_null(array);
    for (size_t index = 0; index < COUNT; ++index) {
        array[index] = (int32_t)(index % 7U);
    }
    assert_true(bc_core_sort_with_compare(array, COUNT, sizeof(int32_t), less_than_int32, NULL));
    for (size_t index = 1; index < COUNT; ++index) {
        assert_true(array[index - 1] <= array[index]);
    }
    free(array);
}

static void test_sort_null_base_with_count_many(void** state)
{
    BC_UNUSED(state);
    assert_false(bc_core_sort_with_compare(NULL, 10, sizeof(int32_t), less_than_int32, NULL));
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_sort_empty),
        cmocka_unit_test(test_sort_single),
        cmocka_unit_test(test_sort_two_already_sorted),
        cmocka_unit_test(test_sort_two_reversed),
        cmocka_unit_test(test_sort_small_random),
        cmocka_unit_test(test_sort_large_random),
        cmocka_unit_test(test_sort_already_sorted),
        cmocka_unit_test(test_sort_reverse_sorted),
        cmocka_unit_test(test_sort_all_duplicates),
        cmocka_unit_test(test_sort_mostly_duplicates),
        cmocka_unit_test(test_sort_descending_order),
        cmocka_unit_test(test_sort_uint64),
        cmocka_unit_test(test_sort_struct_by_key),
        cmocka_unit_test(test_sort_user_data_passed_through),
        cmocka_unit_test(test_sort_rejects_null_compare),
        cmocka_unit_test(test_sort_rejects_zero_element_size),
        cmocka_unit_test(test_sort_element_size_1),
        cmocka_unit_test(test_sort_element_size_2),
        cmocka_unit_test(test_sort_element_size_3),
        cmocka_unit_test(test_sort_element_size_5),
        cmocka_unit_test(test_sort_element_size_6),
        cmocka_unit_test(test_sort_element_size_7),
        cmocka_unit_test(test_sort_element_size_9),
        cmocka_unit_test(test_sort_element_size_10),
        cmocka_unit_test(test_sort_element_size_11),
        cmocka_unit_test(test_sort_element_size_13),
        cmocka_unit_test(test_sort_element_size_15),
        cmocka_unit_test(test_sort_element_size_17),
        cmocka_unit_test(test_sort_element_size_23),
        cmocka_unit_test(test_sort_element_size_31),
        cmocka_unit_test(test_sort_element_size_33),
        cmocka_unit_test(test_sort_element_size_41),
        cmocka_unit_test(test_sort_element_size_63),
        cmocka_unit_test(test_sort_element_size_65),
        cmocka_unit_test(test_sort_element_size_100),
        cmocka_unit_test(test_sort_element_size_127),
        cmocka_unit_test(test_sort_element_size_payload_integrity),
        cmocka_unit_test(test_sort_null_base_with_count_zero),
        cmocka_unit_test(test_sort_null_base_with_count_one),
        cmocka_unit_test(test_sort_element_size_4_uint32),
        cmocka_unit_test(test_sort_element_size_8_uint64_already_sorted),
        cmocka_unit_test(test_sort_element_size_16_pair),
        cmocka_unit_test(test_sort_element_size_24_large_struct),
        cmocka_unit_test(test_sort_element_size_32_struct),
        cmocka_unit_test(test_sort_element_size_64_huge_struct),
        cmocka_unit_test(test_sort_mmap_buffer),
        cmocka_unit_test(test_sort_pathological_input_triggers_heap_fallback),
        cmocka_unit_test(test_sort_many_duplicates_large),
        cmocka_unit_test(test_sort_null_base_with_count_many),
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}
