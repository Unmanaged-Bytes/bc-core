// SPDX-License-Identifier: MIT

#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <cmocka.h>

#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "bc_core.h"
#include "bc_core_cpu_features_internal.h"

#define CONCURRENT_THREAD_COUNT 16

typedef struct {
    pthread_barrier_t* entry_barrier;
    bc_core_cpu_features_t observed;
    bool detect_returned_true;
} concurrent_context_t;

static void* detect_entry(void* argument)
{
    concurrent_context_t* context = (concurrent_context_t*)argument;
    pthread_barrier_wait(context->entry_barrier);
    context->detect_returned_true = bc_core_cpu_features_detect(&context->observed);
    return NULL;
}

static void test_cpu_features_detect_concurrent_is_race_free(void** state)
{
    BC_UNUSED(state);

    pthread_barrier_t entry_barrier;
    assert_int_equal(pthread_barrier_init(&entry_barrier, NULL, CONCURRENT_THREAD_COUNT), 0);

    pthread_t threads[CONCURRENT_THREAD_COUNT];
    concurrent_context_t contexts[CONCURRENT_THREAD_COUNT] = {0};

    for (size_t thread_index = 0U; thread_index < CONCURRENT_THREAD_COUNT; ++thread_index) {
        contexts[thread_index].entry_barrier = &entry_barrier;
        assert_int_equal(pthread_create(&threads[thread_index], NULL, detect_entry, &contexts[thread_index]), 0);
    }

    for (size_t thread_index = 0U; thread_index < CONCURRENT_THREAD_COUNT; ++thread_index) {
        assert_int_equal(pthread_join(threads[thread_index], NULL), 0);
    }

    assert_int_equal(pthread_barrier_destroy(&entry_barrier), 0);

    for (size_t thread_index = 0U; thread_index < CONCURRENT_THREAD_COUNT; ++thread_index) {
        assert_true(contexts[thread_index].detect_returned_true);
        if (thread_index > 0U) {
            assert_memory_equal(&contexts[0].observed, &contexts[thread_index].observed, sizeof(bc_core_cpu_features_t));
        }
    }
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_cpu_features_detect_concurrent_is_race_free),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
