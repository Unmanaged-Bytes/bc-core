// SPDX-License-Identifier: MIT

#ifndef BC_CORE_CPU_H
#define BC_CORE_CPU_H

#include <stdbool.h>
#include <stddef.h>

#define BC_UNUSED(x) ((void)(x))
#define BC_CACHE_LINE_SIZE 64
#define BC_ALIGN(n) __attribute__((aligned(n)))
#define BC_CACHE_LINE_ALIGNED BC_ALIGN(BC_CACHE_LINE_SIZE)
#define bc_core_prefetch_for_read(addr) __builtin_prefetch((addr), 0, 3)
#define bc_core_prefetch_for_write(addr) __builtin_prefetch((addr), 1, 3)
#define bc_core_spin_pause() __asm__ __volatile__("pause" ::: "memory")

typedef enum {
    BC_CORE_CACHE_POLICY_DEFAULT,
    BC_CORE_CACHE_POLICY_CACHED,
    BC_CORE_CACHE_POLICY_STREAMING,
} bc_core_cache_policy_t;

bool bc_core_chunk_size(size_t* out_size);
bool bc_core_preferred_alignment(size_t* out_alignment);
bool bc_core_cache_line_size(size_t* out_size);
bool bc_core_l1d_cache_size(size_t* out_size);
bool bc_core_l2_cache_size(size_t* out_size);
bool bc_core_l3_cache_size(size_t* out_size);
void bc_core_prefetch_read_range(const void* data, size_t len);
void bc_core_evict_range(const void* data, size_t len);

#endif /* BC_CORE_CPU_H */
