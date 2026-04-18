// SPDX-License-Identifier: MIT

#ifndef BC_CORE_CACHE_SIZES_H
#define BC_CORE_CACHE_SIZES_H

#include <stddef.h>

#define BC_CORE_L3_CACHE_SIZE_DEFAULT ((size_t)16 * 1024 * 1024)

void bc_core_cache_sizes_init(void);
size_t bc_core_cached_l1d_cache_size(void);
size_t bc_core_cached_l2_cache_size(void);
size_t bc_core_cached_l3_cache_size(void);

#endif /* BC_CORE_CACHE_SIZES_H */
