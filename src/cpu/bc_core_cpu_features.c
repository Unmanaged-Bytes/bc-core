// SPDX-License-Identifier: MIT

#include <stdatomic.h>
#include <stdbool.h>
#include <stddef.h>
#include <cpuid.h>

#include "bc_core_cpu_features_internal.h"
#include "bc_core.h"

static _Atomic bool g_features_published = false;
static _Atomic bool g_features_publisher_claimed = false;
static bc_core_cpu_features_t g_features_cache;

static void detect_into(bc_core_cpu_features_t* out_features)
{
    out_features->has_sse2 = false;
    out_features->has_avx2 = false;
    out_features->has_sse42 = false;
    out_features->has_aes = false;
    out_features->has_pclmul = false;
    out_features->has_vpclmul = false;
    out_features->has_sha = false;

    unsigned int eax = 0;
    unsigned int ebx = 0;
    unsigned int ecx = 0;
    unsigned int edx = 0;

    __get_cpuid(1, &eax, &ebx, &ecx, &edx);

    out_features->has_sse2 = (edx & (1u << 26)) != 0;
    out_features->has_sse42 = (ecx & (1u << 20)) != 0;
    out_features->has_aes = (ecx & (1u << 25)) != 0;
    out_features->has_pclmul = (ecx & (1u << 1)) != 0;

    eax = 0;
    ebx = 0;
    ecx = 0;
    edx = 0;
    __get_cpuid_count(7, 0, &eax, &ebx, &ecx, &edx);
    out_features->has_avx2 = (ebx & (1u << 5)) != 0;
    out_features->has_sha = (ebx & (1u << 29)) != 0;
    out_features->has_vpclmul = (ecx & (1u << 10)) != 0;
}

bool bc_core_cpu_features_detect(bc_core_cpu_features_t* out_features)
{
    if (atomic_load_explicit(&g_features_published, memory_order_acquire)) {
        *out_features = g_features_cache;
        return true;
    }

    bc_core_cpu_features_t local;
    detect_into(&local);
    *out_features = local;

    bool expected = false;
    if (atomic_compare_exchange_strong_explicit(&g_features_publisher_claimed, &expected, true, memory_order_acq_rel,
                                                memory_order_relaxed)) {
        g_features_cache = local;
        atomic_store_explicit(&g_features_published, true, memory_order_release);
    }

    return true;
}
