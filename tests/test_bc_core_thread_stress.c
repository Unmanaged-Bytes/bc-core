// SPDX-License-Identifier: MIT

#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <cmocka.h>

#include "bc_core.h"

#include <pthread.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define BC_STRESS_THREAD_COUNT 16
#define BC_STRESS_ITERATIONS 5000
#define BC_STRESS_BUFFER_SIZE 4096

typedef struct {
    const unsigned char* shared_buffer_a;
    const unsigned char* shared_buffer_b;
    int worker_id;
    int compare_failures;
    int equal_failures;
    int find_failures;
    int count_failures;
    int hash_failures;
} compare_worker_state_t;

static void* compare_stress_worker(void* argument)
{
    compare_worker_state_t* state = (compare_worker_state_t*)argument;
    for (int iteration = 0; iteration < BC_STRESS_ITERATIONS; ++iteration) {
        bool equal_result = false;
        if (!bc_core_equal(state->shared_buffer_a, state->shared_buffer_a, BC_STRESS_BUFFER_SIZE, &equal_result)) {
            state->equal_failures += 1;
        }
        if (!equal_result) {
            state->equal_failures += 1;
        }
        if (!bc_core_equal(state->shared_buffer_a, state->shared_buffer_b, BC_STRESS_BUFFER_SIZE, &equal_result)) {
            state->equal_failures += 1;
        }
        if (equal_result) {
            state->equal_failures += 1;
        }

        int compare_result = 0;
        if (!bc_core_compare(state->shared_buffer_a, state->shared_buffer_a, BC_STRESS_BUFFER_SIZE, &compare_result)) {
            state->compare_failures += 1;
        }
        if (compare_result != 0) {
            state->compare_failures += 1;
        }

        size_t offset = 0;
        if (!bc_core_find_byte(state->shared_buffer_a, BC_STRESS_BUFFER_SIZE, 0xAA, &offset)) {
            state->find_failures += 1;
        }
        if (offset != BC_STRESS_BUFFER_SIZE - 1U) {
            state->find_failures += 1;
        }

        size_t count = 0;
        if (!bc_core_count_byte(state->shared_buffer_a, BC_STRESS_BUFFER_SIZE, 0xAA, &count)) {
            state->count_failures += 1;
        }
        if (count != 1U) {
            state->count_failures += 1;
        }

        uint32_t crc_value = 0xFFFFFFFFU;
        bc_core_crc32c(state->shared_buffer_a, BC_STRESS_BUFFER_SIZE, &crc_value);
        if (crc_value == 0xFFFFFFFFU) {
            state->hash_failures += 1;
        }
    }
    return NULL;
}

static void test_thread_stress_read_only_primitives(void** state)
{
    BC_UNUSED(state);

    unsigned char* buffer_a = malloc(BC_STRESS_BUFFER_SIZE);
    unsigned char* buffer_b = malloc(BC_STRESS_BUFFER_SIZE);
    assert_non_null(buffer_a);
    assert_non_null(buffer_b);

    for (size_t index = 0; index < BC_STRESS_BUFFER_SIZE; ++index) {
        buffer_a[index] = 0x42;
        buffer_b[index] = 0x55;
    }
    buffer_a[BC_STRESS_BUFFER_SIZE - 1U] = 0xAA;

    pthread_t threads[BC_STRESS_THREAD_COUNT];
    compare_worker_state_t states[BC_STRESS_THREAD_COUNT];
    memset(states, 0, sizeof(states));

    for (int worker_index = 0; worker_index < BC_STRESS_THREAD_COUNT; ++worker_index) {
        states[worker_index].shared_buffer_a = buffer_a;
        states[worker_index].shared_buffer_b = buffer_b;
        states[worker_index].worker_id = worker_index;
        int rc = pthread_create(&threads[worker_index], NULL, compare_stress_worker, &states[worker_index]);
        assert_int_equal(rc, 0);
    }
    for (int worker_index = 0; worker_index < BC_STRESS_THREAD_COUNT; ++worker_index) {
        pthread_join(threads[worker_index], NULL);
    }
    for (int worker_index = 0; worker_index < BC_STRESS_THREAD_COUNT; ++worker_index) {
        assert_int_equal(states[worker_index].equal_failures, 0);
        assert_int_equal(states[worker_index].compare_failures, 0);
        assert_int_equal(states[worker_index].find_failures, 0);
        assert_int_equal(states[worker_index].count_failures, 0);
        assert_int_equal(states[worker_index].hash_failures, 0);
    }

    free(buffer_a);
    free(buffer_b);
}

typedef struct {
    int worker_id;
    int sort_failures;
} sort_worker_state_t;

static bool less_than_uint64_stress(const void* left, const void* right, void* user_data)
{
    BC_UNUSED(user_data);
    return *(const uint64_t*)left < *(const uint64_t*)right;
}

static void* sort_stress_worker(void* argument)
{
    sort_worker_state_t* state = (sort_worker_state_t*)argument;
    enum { COUNT = 256 };
    uint64_t array[COUNT];
    uint32_t local_seed = (uint32_t)state->worker_id * 0x9E3779B9U + 1U;
    for (int iteration = 0; iteration < BC_STRESS_ITERATIONS / 10; ++iteration) {
        for (size_t index = 0; index < COUNT; ++index) {
            local_seed = local_seed * 1664525U + 1013904223U;
            array[index] = ((uint64_t)local_seed << 32) | (uint64_t)(local_seed * 2U);
        }
        if (!bc_core_sort_with_compare(array, COUNT, sizeof(uint64_t), less_than_uint64_stress, NULL)) {
            state->sort_failures += 1;
        }
        for (size_t index = 1; index < COUNT; ++index) {
            if (array[index - 1] > array[index]) {
                state->sort_failures += 1;
                break;
            }
        }
    }
    return NULL;
}

static void test_thread_stress_sort_per_worker_arrays(void** state)
{
    BC_UNUSED(state);

    pthread_t threads[BC_STRESS_THREAD_COUNT];
    sort_worker_state_t states[BC_STRESS_THREAD_COUNT];
    memset(states, 0, sizeof(states));
    for (int worker_index = 0; worker_index < BC_STRESS_THREAD_COUNT; ++worker_index) {
        states[worker_index].worker_id = worker_index;
        int rc = pthread_create(&threads[worker_index], NULL, sort_stress_worker, &states[worker_index]);
        assert_int_equal(rc, 0);
    }
    for (int worker_index = 0; worker_index < BC_STRESS_THREAD_COUNT; ++worker_index) {
        pthread_join(threads[worker_index], NULL);
    }
    for (int worker_index = 0; worker_index < BC_STRESS_THREAD_COUNT; ++worker_index) {
        assert_int_equal(states[worker_index].sort_failures, 0);
    }
}

typedef struct {
    int worker_id;
    int format_failures;
} format_worker_state_t;

static void* format_stress_worker(void* argument)
{
    format_worker_state_t* state = (format_worker_state_t*)argument;
    char buffer[64];
    size_t length = 0;
    for (int iteration = 0; iteration < BC_STRESS_ITERATIONS; ++iteration) {
        uint64_t value = (uint64_t)((state->worker_id + 1) * iteration);
        if (!bc_core_format_unsigned_integer_64_decimal(buffer, sizeof(buffer), value, &length)) {
            state->format_failures += 1;
        }
        if (!bc_core_format_unsigned_integer_64_hexadecimal(buffer, sizeof(buffer), value, &length)) {
            state->format_failures += 1;
        }
        if (!bc_core_format_signed_integer_64(buffer, sizeof(buffer), -(int64_t)value, &length)) {
            state->format_failures += 1;
        }
    }
    return NULL;
}

static void test_thread_stress_format(void** state)
{
    BC_UNUSED(state);

    pthread_t threads[BC_STRESS_THREAD_COUNT];
    format_worker_state_t states[BC_STRESS_THREAD_COUNT];
    memset(states, 0, sizeof(states));
    for (int worker_index = 0; worker_index < BC_STRESS_THREAD_COUNT; ++worker_index) {
        states[worker_index].worker_id = worker_index;
        int rc = pthread_create(&threads[worker_index], NULL, format_stress_worker, &states[worker_index]);
        assert_int_equal(rc, 0);
    }
    for (int worker_index = 0; worker_index < BC_STRESS_THREAD_COUNT; ++worker_index) {
        pthread_join(threads[worker_index], NULL);
    }
    for (int worker_index = 0; worker_index < BC_STRESS_THREAD_COUNT; ++worker_index) {
        assert_int_equal(states[worker_index].format_failures, 0);
    }
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_thread_stress_read_only_primitives),
        cmocka_unit_test(test_thread_stress_sort_per_worker_arrays),
        cmocka_unit_test(test_thread_stress_format),
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}
