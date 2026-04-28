// SPDX-License-Identifier: MIT

#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <cmocka.h>

#include "bc_core.h"

#include <stdlib.h>
#include <unistd.h>

bool __real_bc_core_l1d_cache_size(size_t* out_size);
bool __real_bc_core_l2_cache_size(size_t* out_size);
bool __real_bc_core_l3_cache_size(size_t* out_size);

static bool g_l1d_use_mock = false;
static size_t g_l1d_mock_size = 0;
static bool g_l1d_mock_succeeds = true;

static bool g_l2_use_mock = false;
static size_t g_l2_mock_size = 0;
static bool g_l2_mock_succeeds = true;

static bool g_l3_use_mock = false;
static size_t g_l3_mock_size = 0;
static bool g_l3_mock_succeeds = true;

bool __wrap_bc_core_l1d_cache_size(size_t* out_size)
{
    if (g_l1d_use_mock) {
        if (!g_l1d_mock_succeeds) {
            return false;
        }
        *out_size = g_l1d_mock_size;
        return true;
    }
    return __real_bc_core_l1d_cache_size(out_size);
}

bool __wrap_bc_core_l2_cache_size(size_t* out_size)
{
    if (g_l2_use_mock) {
        if (!g_l2_mock_succeeds) {
            return false;
        }
        *out_size = g_l2_mock_size;
        return true;
    }
    return __real_bc_core_l2_cache_size(out_size);
}

bool __wrap_bc_core_l3_cache_size(size_t* out_size)
{
    if (g_l3_use_mock) {
        if (!g_l3_mock_succeeds) {
            return false;
        }
        *out_size = g_l3_mock_size;
        return true;
    }
    return __real_bc_core_l3_cache_size(out_size);
}

static void install_ryzen_5700g_mock(void)
{
    g_l1d_use_mock = true;
    g_l1d_mock_size = (size_t)32 * 1024;
    g_l1d_mock_succeeds = true;

    g_l2_use_mock = true;
    g_l2_mock_size = (size_t)512 * 1024;
    g_l2_mock_succeeds = true;

    g_l3_use_mock = true;
    g_l3_mock_size = (size_t)16 * 1024 * 1024;
    g_l3_mock_succeeds = true;
}

static void clear_mocks(void)
{
    g_l1d_use_mock = false;
    g_l2_use_mock = false;
    g_l3_use_mock = false;
}

static void test_single_thread_uses_full_l3_per_thread(void** state)
{
    BC_UNUSED(state);
    install_ryzen_5700g_mock();

    bc_core_buffer_thresholds_t thresholds;
    bool detected = bc_core_buffer_thresholds(1, &thresholds);

    assert_true(detected);
    assert_int_equal(thresholds.l1_hot_bytes, (size_t)16 * 1024);
    assert_int_equal(thresholds.l2_hot_bytes, (size_t)256 * 1024);
    assert_int_equal(thresholds.l3_per_thread_bytes, (size_t)16 * 1024 * 1024);
    assert_int_equal(thresholds.copy_streaming_threshold_bytes, (size_t)32 * 1024 * 1024);
    assert_int_equal(thresholds.zero_streaming_threshold_bytes, (size_t)32 * 1024 * 1024);
    assert_int_equal(thresholds.fill_streaming_threshold_bytes, (size_t)32 * 1024 * 1024);

    clear_mocks();
}

static void test_eight_workers_split_l3_evenly(void** state)
{
    BC_UNUSED(state);
    install_ryzen_5700g_mock();

    bc_core_buffer_thresholds_t thresholds;
    bool detected = bc_core_buffer_thresholds(8, &thresholds);

    assert_true(detected);
    assert_int_equal(thresholds.l3_per_thread_bytes, (size_t)2 * 1024 * 1024);
    assert_int_equal(thresholds.copy_streaming_threshold_bytes, (size_t)4 * 1024 * 1024);
    assert_int_equal(thresholds.zero_streaming_threshold_bytes, (size_t)4 * 1024 * 1024);
    assert_int_equal(thresholds.fill_streaming_threshold_bytes, (size_t)4 * 1024 * 1024);

    clear_mocks();
}

static void test_sixteen_workers_oversubscribe_floors_to_4_mib(void** state)
{
    BC_UNUSED(state);
    install_ryzen_5700g_mock();

    bc_core_buffer_thresholds_t thresholds;
    bool detected = bc_core_buffer_thresholds(16, &thresholds);

    assert_true(detected);
    assert_int_equal(thresholds.l3_per_thread_bytes, (size_t)1 * 1024 * 1024);
    assert_int_equal(thresholds.copy_streaming_threshold_bytes, (size_t)4 * 1024 * 1024);
    assert_int_equal(thresholds.zero_streaming_threshold_bytes, (size_t)4 * 1024 * 1024);
    assert_int_equal(thresholds.fill_streaming_threshold_bytes, (size_t)4 * 1024 * 1024);

    clear_mocks();
}

static void test_zero_worker_hint_treated_as_single_thread(void** state)
{
    BC_UNUSED(state);
    install_ryzen_5700g_mock();

    bc_core_buffer_thresholds_t thresholds_zero;
    bc_core_buffer_thresholds_t thresholds_one;
    bool detected_zero = bc_core_buffer_thresholds(0, &thresholds_zero);
    bool detected_one = bc_core_buffer_thresholds(1, &thresholds_one);

    assert_true(detected_zero);
    assert_true(detected_one);
    assert_int_equal(thresholds_zero.l3_per_thread_bytes, thresholds_one.l3_per_thread_bytes);
    assert_int_equal(thresholds_zero.copy_streaming_threshold_bytes, thresholds_one.copy_streaming_threshold_bytes);

    clear_mocks();
}

static void test_l3_detection_failure_returns_false_with_fallback_values(void** state)
{
    BC_UNUSED(state);
    install_ryzen_5700g_mock();
    g_l3_mock_succeeds = false;

    bc_core_buffer_thresholds_t thresholds;
    bool detected = bc_core_buffer_thresholds(8, &thresholds);

    assert_false(detected);
    assert_int_equal(thresholds.l1_hot_bytes, BC_BUFFER_L1_HOT_BYTES);
    assert_int_equal(thresholds.l2_hot_bytes, BC_BUFFER_L2_HOT_BYTES);
    assert_int_equal(thresholds.l3_per_thread_bytes, BC_BUFFER_L3_PER_THREAD_BYTES);
    assert_int_equal(thresholds.copy_streaming_threshold_bytes, BC_BUFFER_COPY_STREAMING_THRESHOLD_FALLBACK);
    assert_int_equal(thresholds.zero_streaming_threshold_bytes, BC_BUFFER_ZERO_STREAMING_THRESHOLD_FALLBACK);
    assert_int_equal(thresholds.fill_streaming_threshold_bytes, BC_BUFFER_FILL_STREAMING_THRESHOLD_FALLBACK);

    clear_mocks();
}

static void test_l1_detection_failure_returns_false_with_fallback_values(void** state)
{
    BC_UNUSED(state);
    install_ryzen_5700g_mock();
    g_l1d_mock_succeeds = false;

    bc_core_buffer_thresholds_t thresholds;
    bool detected = bc_core_buffer_thresholds(2, &thresholds);

    assert_false(detected);
    assert_int_equal(thresholds.copy_streaming_threshold_bytes, BC_BUFFER_COPY_STREAMING_THRESHOLD_FALLBACK);

    clear_mocks();
}

static void test_zero_l3_size_treated_as_failure(void** state)
{
    BC_UNUSED(state);
    install_ryzen_5700g_mock();
    g_l3_mock_size = 0;

    bc_core_buffer_thresholds_t thresholds;
    bool detected = bc_core_buffer_thresholds(4, &thresholds);

    assert_false(detected);
    assert_int_equal(thresholds.l3_per_thread_bytes, BC_BUFFER_L3_PER_THREAD_BYTES);

    clear_mocks();
}

static void test_default_uses_online_processor_count(void** state)
{
    BC_UNUSED(state);
    /* _default() snapshots the topology + nproc at process startup before
       any test runs; comparing it against an explicit call with the host's
       real parameters must match. Mocks must be off so the explicit call
       observes the same sizes as the cached snapshot. */
    clear_mocks();

    bc_core_buffer_thresholds_t thresholds_default;
    bool detected_default = bc_core_buffer_thresholds_default(&thresholds_default);

    long online_processors = sysconf(_SC_NPROCESSORS_ONLN);
    assert_true(online_processors > 0);

    bc_core_buffer_thresholds_t thresholds_explicit;
    bool detected_explicit = bc_core_buffer_thresholds((size_t)online_processors, &thresholds_explicit);

    assert_int_equal((int)detected_default, (int)detected_explicit);
    assert_int_equal(thresholds_default.l3_per_thread_bytes, thresholds_explicit.l3_per_thread_bytes);
    assert_int_equal(thresholds_default.copy_streaming_threshold_bytes, thresholds_explicit.copy_streaming_threshold_bytes);
}

static void test_real_hardware_thresholds_are_consistent(void** state)
{
    BC_UNUSED(state);
    clear_mocks();

    bc_core_buffer_thresholds_t thresholds;
    (void)bc_core_buffer_thresholds_default(&thresholds);

    assert_true(thresholds.l1_hot_bytes > 0);
    assert_true(thresholds.l2_hot_bytes >= thresholds.l1_hot_bytes);
    assert_true(thresholds.l3_per_thread_bytes >= thresholds.l2_hot_bytes);
    assert_true(thresholds.copy_streaming_threshold_bytes >= (size_t)4 * 1024 * 1024);
    assert_true(thresholds.zero_streaming_threshold_bytes >= (size_t)4 * 1024 * 1024);
    assert_true(thresholds.fill_streaming_threshold_bytes >= (size_t)4 * 1024 * 1024);
}

static void test_auto_policy_streams_above_per_thread_threshold(void** state)
{
    BC_UNUSED(state);
    install_ryzen_5700g_mock();

    const size_t large_size = (size_t)8 * 1024 * 1024;
    unsigned char* source = aligned_alloc(64, large_size);
    unsigned char* destination = aligned_alloc(64, large_size);
    assert_non_null(source);
    assert_non_null(destination);

    for (size_t index = 0; index < large_size; index += 4096U) {
        source[index] = (unsigned char)(index & 0xFFU);
    }

    bool result = bc_core_copy_with_policy_threaded(destination, source, large_size, BC_CORE_CACHE_POLICY_AUTO, 16);
    assert_true(result);
    for (size_t index = 0; index < large_size; index += 4096U) {
        assert_int_equal(destination[index], source[index]);
    }

    free(source);
    free(destination);
    clear_mocks();
}

static void test_auto_policy_uses_cached_path_for_small_buffer(void** state)
{
    BC_UNUSED(state);
    install_ryzen_5700g_mock();

    unsigned char source[1024];
    unsigned char destination[1024];
    for (size_t index = 0; index < sizeof(source); index++) {
        source[index] = (unsigned char)(index & 0xFFU);
    }

    bool result = bc_core_copy_with_policy_threaded(destination, source, sizeof(source), BC_CORE_CACHE_POLICY_AUTO, 16);
    assert_true(result);
    assert_memory_equal(destination, source, sizeof(source));

    clear_mocks();
}

static void test_zero_with_policy_threaded_auto_routes_correctly(void** state)
{
    BC_UNUSED(state);
    install_ryzen_5700g_mock();

    const size_t large_size = (size_t)8 * 1024 * 1024;
    unsigned char* destination = aligned_alloc(64, large_size);
    assert_non_null(destination);

    for (size_t index = 0; index < large_size; index += 4096U) {
        destination[index] = 0xFF;
    }

    bool result = bc_core_zero_with_policy_threaded(destination, large_size, BC_CORE_CACHE_POLICY_AUTO, 16);
    assert_true(result);
    for (size_t index = 0; index < large_size; index += 4096U) {
        assert_int_equal(destination[index], 0);
    }

    free(destination);
    clear_mocks();
}

static void test_fill_with_policy_threaded_auto_routes_correctly(void** state)
{
    BC_UNUSED(state);
    install_ryzen_5700g_mock();

    const size_t large_size = (size_t)8 * 1024 * 1024;
    unsigned char* destination = aligned_alloc(64, large_size);
    assert_non_null(destination);

    bool result = bc_core_fill_with_policy_threaded(destination, large_size, 0xCD, BC_CORE_CACHE_POLICY_AUTO, 16);
    assert_true(result);
    for (size_t index = 0; index < large_size; index += 4096U) {
        assert_int_equal(destination[index], 0xCD);
    }

    free(destination);
    clear_mocks();
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_single_thread_uses_full_l3_per_thread),
        cmocka_unit_test(test_eight_workers_split_l3_evenly),
        cmocka_unit_test(test_sixteen_workers_oversubscribe_floors_to_4_mib),
        cmocka_unit_test(test_zero_worker_hint_treated_as_single_thread),
        cmocka_unit_test(test_l3_detection_failure_returns_false_with_fallback_values),
        cmocka_unit_test(test_l1_detection_failure_returns_false_with_fallback_values),
        cmocka_unit_test(test_zero_l3_size_treated_as_failure),
        cmocka_unit_test(test_default_uses_online_processor_count),
        cmocka_unit_test(test_real_hardware_thresholds_are_consistent),
        cmocka_unit_test(test_auto_policy_streams_above_per_thread_threshold),
        cmocka_unit_test(test_auto_policy_uses_cached_path_for_small_buffer),
        cmocka_unit_test(test_zero_with_policy_threaded_auto_routes_correctly),
        cmocka_unit_test(test_fill_with_policy_threaded_auto_routes_correctly),
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}
