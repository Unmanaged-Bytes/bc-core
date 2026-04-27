// SPDX-License-Identifier: MIT

#include "bc_core_sort.h"
#include "bc_core_memory.h"

#define BC_CORE_SORT_INSERTION_THRESHOLD ((size_t)16)

static size_t bc_core_sort_log2_floor(size_t value)
{
    size_t result = 0;
    while (value > 1U) {
        value >>= 1U;
        ++result;
    }
    return result;
}

static void bc_core_sort_insertion(unsigned char* base, size_t count, size_t element_size, bc_core_sort_less_than_function less_than,
                                   void* user_data)
{
    for (size_t outer = 1; outer < count; ++outer) {
        size_t inner = outer;
        while (inner > 0) {
            unsigned char* current = base + inner * element_size;
            unsigned char* previous = base + (inner - 1U) * element_size;
            if (!less_than(current, previous, user_data)) {
                break;
            }
            (void)bc_core_swap(current, previous, element_size);
            --inner;
        }
    }
}

static void bc_core_sort_sift_down(unsigned char* base, size_t start, size_t end, size_t element_size,
                                   bc_core_sort_less_than_function less_than, void* user_data)
{
    size_t root = start;
    while ((root * 2U) + 1U <= end) {
        size_t child = (root * 2U) + 1U;
        if (child + 1U <= end) {
            unsigned char* left_child = base + child * element_size;
            unsigned char* right_child = base + (child + 1U) * element_size;
            if (less_than(left_child, right_child, user_data)) {
                child += 1U;
            }
        }
        unsigned char* root_pointer = base + root * element_size;
        unsigned char* child_pointer = base + child * element_size;
        if (!less_than(root_pointer, child_pointer, user_data)) {
            return;
        }
        (void)bc_core_swap(root_pointer, child_pointer, element_size);
        root = child;
    }
}

static void bc_core_sort_heap(unsigned char* base, size_t count, size_t element_size, bc_core_sort_less_than_function less_than,
                              void* user_data)
{
    size_t start_plus_one = (count - 2U) / 2U + 1U;
    while (start_plus_one > 0U) {
        size_t start = start_plus_one - 1U;
        bc_core_sort_sift_down(base, start, count - 1U, element_size, less_than, user_data);
        --start_plus_one;
    }

    size_t end = count - 1U;
    while (end > 0U) {
        unsigned char* end_pointer = base + end * element_size;
        (void)bc_core_swap(base, end_pointer, element_size);
        --end;
        bc_core_sort_sift_down(base, 0U, end, element_size, less_than, user_data);
    }
}

static void bc_core_sort_introsort(unsigned char* base, size_t count, size_t element_size, bc_core_sort_less_than_function less_than,
                                   void* user_data, size_t depth_limit)
{
    while (count > BC_CORE_SORT_INSERTION_THRESHOLD) {
        if (depth_limit == 0U) {
            bc_core_sort_heap(base, count, element_size, less_than, user_data);
            return;
        }
        depth_limit -= 1U;

        size_t pivot_index = count / 2U;
        unsigned char* last = base + (count - 1U) * element_size;
        (void)bc_core_swap(base + pivot_index * element_size, last, element_size);

        size_t store = 0;
        for (size_t scan = 0; scan < count - 1U; ++scan) {
            unsigned char* candidate = base + scan * element_size;
            if (less_than(candidate, last, user_data)) {
                unsigned char* destination = base + store * element_size;
                (void)bc_core_swap(candidate, destination, element_size);
                store += 1U;
            }
        }
        unsigned char* pivot_final = base + store * element_size;
        (void)bc_core_swap(pivot_final, last, element_size);

        size_t left_count = store;
        size_t right_count = count - store - 1U;
        unsigned char* right_base = base + (store + 1U) * element_size;

        if (left_count < right_count) {
            bc_core_sort_introsort(base, left_count, element_size, less_than, user_data, depth_limit);
            base = right_base;
            count = right_count;
        } else {
            bc_core_sort_introsort(right_base, right_count, element_size, less_than, user_data, depth_limit);
            count = left_count;
        }
    }

    bc_core_sort_insertion(base, count, element_size, less_than, user_data);
}

bool bc_core_sort_with_compare(void* base, size_t count, size_t element_size, bc_core_sort_less_than_function less_than, void* user_data)
{
    if (less_than == NULL) {
        return false;
    }
    if (element_size == 0U) {
        return false;
    }
    if (count <= 1U) {
        return true;
    }
    if (base == NULL) {
        return false;
    }

    size_t depth_limit = 2U * bc_core_sort_log2_floor(count);
    bc_core_sort_introsort((unsigned char*)base, count, element_size, less_than, user_data, depth_limit);
    return true;
}
