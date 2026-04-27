// SPDX-License-Identifier: MIT

#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <cmocka.h>

#include "bc_core.h"

#include <stdint.h>
#include <stdlib.h>

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
        cmocka_unit_test(test_sort_null_base_with_count_zero),
        cmocka_unit_test(test_sort_null_base_with_count_one),
        cmocka_unit_test(test_sort_null_base_with_count_many),
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}
