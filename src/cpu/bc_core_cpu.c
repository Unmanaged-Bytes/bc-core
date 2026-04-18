// SPDX-License-Identifier: MIT

#define _DEFAULT_SOURCE

#include "bc_core.h"

#include <unistd.h>

#include <stdbool.h>
#include <stddef.h>

static size_t g_chunk_size = 32;

static bool query_sysconf(int name, size_t* out_size)
{
    long result = sysconf(name);

    if (result <= 0) {
        return false;
    }

    *out_size = (size_t)result;
    return true;
}

__attribute__((constructor)) void bc_core_cpu_dispatch_init(void)
{
    g_chunk_size = 32;
}

bool bc_core_cache_line_size(size_t* out_size)
{
    return query_sysconf(_SC_LEVEL1_DCACHE_LINESIZE, out_size);
}

bool bc_core_l1d_cache_size(size_t* out_size)
{
    return query_sysconf(_SC_LEVEL1_DCACHE_SIZE, out_size);
}

bool bc_core_l2_cache_size(size_t* out_size)
{
    return query_sysconf(_SC_LEVEL2_CACHE_SIZE, out_size);
}

bool bc_core_l3_cache_size(size_t* out_size)
{
    return query_sysconf(_SC_LEVEL3_CACHE_SIZE, out_size);
}

bool bc_core_chunk_size(size_t* out_size)
{
    *out_size = g_chunk_size;
    return true;
}

bool bc_core_preferred_alignment(size_t* out_alignment)
{
    *out_alignment = 64;
    return true;
}
