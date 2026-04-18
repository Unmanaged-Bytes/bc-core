// SPDX-License-Identifier: MIT

#include "bc_core_cache_sizes_internal.h"

#include "bc_core.h"

#include <stddef.h>

static size_t g_cached_l1d_cache_size = 0;
static size_t g_cached_l2_cache_size = 0;
static size_t g_cached_l3_cache_size = BC_CORE_L3_CACHE_SIZE_DEFAULT;

__attribute__((constructor)) void bc_core_cache_sizes_init(void)
{
    size_t l1d_size = 0;
    if (bc_core_l1d_cache_size(&l1d_size)) { /* GCOVR_EXCL_BR_LINE -- sysconf always succeeds on real hardware */
        g_cached_l1d_cache_size = l1d_size;
    }

    size_t l2_size = 0;
    if (bc_core_l2_cache_size(&l2_size)) { /* GCOVR_EXCL_BR_LINE -- sysconf always succeeds on real hardware */
        g_cached_l2_cache_size = l2_size;
    }

    size_t l3_size = 0;
    if (bc_core_l3_cache_size(&l3_size)) { /* GCOVR_EXCL_BR_LINE -- sysconf always succeeds on real hardware */
        g_cached_l3_cache_size = l3_size;
    }
}

size_t bc_core_cached_l1d_cache_size(void)
{
    return g_cached_l1d_cache_size;
}

size_t bc_core_cached_l2_cache_size(void)
{
    return g_cached_l2_cache_size;
}

size_t bc_core_cached_l3_cache_size(void)
{
    return g_cached_l3_cache_size;
}
