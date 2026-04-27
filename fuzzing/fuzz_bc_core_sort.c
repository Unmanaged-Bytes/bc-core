// SPDX-License-Identifier: MIT

#include "bc_core.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static bool less_than_uint64(const void* left, const void* right, void* user_data)
{
    (void)user_data;
    return *(const uint64_t*)left < *(const uint64_t*)right;
}

static bool less_than_int32(const void* left, const void* right, void* user_data)
{
    (void)user_data;
    return *(const int32_t*)left < *(const int32_t*)right;
}

#define BC_FUZZ_MAX_ELEMENTS ((size_t)8192)

static void check_sorted_uint64(const uint64_t* array, size_t count)
{
    for (size_t index = 1; index < count; ++index) {
        if (array[index - 1U] > array[index]) {
            __builtin_trap();
        }
    }
}

static void check_sorted_int32(const int32_t* array, size_t count)
{
    for (size_t index = 1; index < count; ++index) {
        if (array[index - 1] > array[index]) {
            __builtin_trap();
        }
    }
}

int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if (size < 2U) {
        return 0;
    }

    uint8_t selector = data[0];
    const uint8_t* payload = data + 1;
    size_t payload_size = size - 1U;

    if ((selector & 1U) == 0U) {
        size_t count = payload_size / sizeof(uint64_t);
        if (count > BC_FUZZ_MAX_ELEMENTS) {
            count = BC_FUZZ_MAX_ELEMENTS;
        }
        if (count == 0) {
            return 0;
        }
        uint64_t* array = (uint64_t*)malloc(count * sizeof(uint64_t));
        if (array == NULL) {
            return 0;
        }
        memcpy(array, payload, count * sizeof(uint64_t));
        bool ok = bc_core_sort_with_compare(array, count, sizeof(uint64_t), less_than_uint64, NULL);
        if (ok) {
            check_sorted_uint64(array, count);
        }
        free(array);
    } else {
        size_t count = payload_size / sizeof(int32_t);
        if (count > BC_FUZZ_MAX_ELEMENTS) {
            count = BC_FUZZ_MAX_ELEMENTS;
        }
        if (count == 0) {
            return 0;
        }
        int32_t* array = (int32_t*)malloc(count * sizeof(int32_t));
        if (array == NULL) {
            return 0;
        }
        memcpy(array, payload, count * sizeof(int32_t));
        bool ok = bc_core_sort_with_compare(array, count, sizeof(int32_t), less_than_int32, NULL);
        if (ok) {
            check_sorted_int32(array, count);
        }
        free(array);
    }

    return 0;
}
