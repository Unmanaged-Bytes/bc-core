// SPDX-License-Identifier: MIT

#define _DEFAULT_SOURCE

#include "bc_core.h"

#include "bc_core_buffer_thresholds_internal.h"

#include <unistd.h>

#include <stdbool.h>
#include <stddef.h>

static bc_core_buffer_thresholds_t g_default_thresholds_cache;
static bool g_default_thresholds_cache_detected = false;

static size_t streaming_target(size_t l3_per_thread_bytes, size_t fallback)
{
    const size_t target = (size_t)2 * l3_per_thread_bytes;
    return target > fallback ? target : fallback;
}

static void fill_with_compile_time_fallbacks(bc_core_buffer_thresholds_t* out)
{
    out->l1_hot_bytes = BC_BUFFER_L1_HOT_BYTES;
    out->l2_hot_bytes = BC_BUFFER_L2_HOT_BYTES;
    out->l3_per_thread_bytes = BC_BUFFER_L3_PER_THREAD_BYTES;
    out->copy_streaming_threshold_bytes = BC_BUFFER_COPY_STREAMING_THRESHOLD_FALLBACK;
    out->zero_streaming_threshold_bytes = BC_BUFFER_ZERO_STREAMING_THRESHOLD_FALLBACK;
    out->fill_streaming_threshold_bytes = BC_BUFFER_FILL_STREAMING_THRESHOLD_FALLBACK;
}

bool bc_core_buffer_thresholds(size_t worker_count_hint, bc_core_buffer_thresholds_t* out)
{
    const size_t effective_workers = worker_count_hint > 0 ? worker_count_hint : (size_t)1;

    size_t l1d_cache_size_bytes = 0;
    size_t l2_cache_size_bytes = 0;
    size_t l3_cache_size_bytes = 0;

    const bool l1d_detected = bc_core_l1d_cache_size(&l1d_cache_size_bytes);
    const bool l2_detected = bc_core_l2_cache_size(&l2_cache_size_bytes);
    const bool l3_detected = bc_core_l3_cache_size(&l3_cache_size_bytes);

    if (!l1d_detected || !l2_detected || !l3_detected || l1d_cache_size_bytes == 0 || l2_cache_size_bytes == 0 ||
        l3_cache_size_bytes == 0) {
        fill_with_compile_time_fallbacks(out);
        return false;
    }

    out->l1_hot_bytes = l1d_cache_size_bytes / 2;
    out->l2_hot_bytes = l2_cache_size_bytes / 2;
    out->l3_per_thread_bytes = l3_cache_size_bytes / effective_workers;

    out->copy_streaming_threshold_bytes = streaming_target(out->l3_per_thread_bytes, BC_BUFFER_COPY_STREAMING_THRESHOLD_FALLBACK);
    out->zero_streaming_threshold_bytes = streaming_target(out->l3_per_thread_bytes, BC_BUFFER_ZERO_STREAMING_THRESHOLD_FALLBACK);
    out->fill_streaming_threshold_bytes = streaming_target(out->l3_per_thread_bytes, BC_BUFFER_FILL_STREAMING_THRESHOLD_FALLBACK);
    return true;
}

__attribute__((constructor)) static void bc_core_buffer_thresholds_default_init(void)
{
    long online_processors = sysconf(_SC_NPROCESSORS_ONLN);
    size_t worker_count_hint = (online_processors > 0) ? (size_t)online_processors : (size_t)1;
    g_default_thresholds_cache_detected = bc_core_buffer_thresholds(worker_count_hint, &g_default_thresholds_cache);
}

bool bc_core_buffer_thresholds_default(bc_core_buffer_thresholds_t* out)
{
    *out = g_default_thresholds_cache;
    return g_default_thresholds_cache_detected;
}

size_t bc_core_buffer_thresholds_default_copy_streaming_bytes(void)
{
    return g_default_thresholds_cache.copy_streaming_threshold_bytes;
}

size_t bc_core_buffer_thresholds_default_zero_streaming_bytes(void)
{
    return g_default_thresholds_cache.zero_streaming_threshold_bytes;
}

size_t bc_core_buffer_thresholds_default_fill_streaming_bytes(void)
{
    return g_default_thresholds_cache.fill_streaming_threshold_bytes;
}
