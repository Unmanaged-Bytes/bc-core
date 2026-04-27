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

/* ===== Cache-tier buffer budgets =====
   Compile-time conservative sizing for stack/heap buffers.
   L1: 50% (HOT) / 75% (FULL) of typical 32 KiB L1d.
   L2: 50% / 75% of typical 512 KiB L2/core (Zen 3 baseline ; Tiger Lake L2=1.25 MB → marge OK).
   L3 per-thread: 1 MiB assuming ~16 threads sharing 16 MiB L3.
   These are upper bounds for working sets that should remain capacity-miss-free in their tier. */
#define BC_BUFFER_L1_HOT_BYTES ((size_t)16 * 1024)
#define BC_BUFFER_L1_FULL_BYTES ((size_t)24 * 1024)
#define BC_BUFFER_L2_HOT_BYTES ((size_t)256 * 1024)
#define BC_BUFFER_L2_FULL_BYTES ((size_t)384 * 1024)
#define BC_BUFFER_L3_PER_THREAD_BYTES ((size_t)1024 * 1024)

/* ===== Streaming (NT stores) thresholds — measured on Zen 3 =====
   Above these, BC_CORE_CACHE_POLICY_STREAMING outperforms the cached path
   on bandwidth-bound primitives. Sized conservatively (lower bound) so the
   same constant works on both AMD Zen 3 and Intel Tiger Lake — refine when
   ws-laptop-00 measurements are available. */
#define BC_BUFFER_COPY_STREAMING_THRESHOLD ((size_t)1 * 1024 * 1024)
#define BC_BUFFER_ZERO_STREAMING_THRESHOLD ((size_t)4 * 1024 * 1024)
#define BC_BUFFER_FILL_STREAMING_THRESHOLD ((size_t)4 * 1024 * 1024)

/* Huge page eligibility threshold (2 MiB Linux x86-64). */
#define BC_BUFFER_HUGE_PAGE_THRESHOLD ((size_t)2 * 1024 * 1024)

/* Pads a struct field to align the next field on a fresh cache line.
   Use after a write-hot member that must not share a line with the next.
   N.B.: caller is responsible for `used_bytes` being accurate ; a
   _Static_assert in the consuming struct catches drift.
   Precondition: used_bytes % BC_CACHE_LINE_SIZE != 0 (otherwise the
   pad array would be zero-sized, which is non-standard C). */
#define BC_PAD_TO_CACHE_LINE(used_bytes) char _bc_pad[(BC_CACHE_LINE_SIZE - ((used_bytes) % BC_CACHE_LINE_SIZE)) % BC_CACHE_LINE_SIZE]

/* Wraps a single value such that it sits alone on its cache line.
   Prevents false sharing with whatever follows in the enclosing struct.
   sizeof(type) must be <= BC_CACHE_LINE_SIZE. */
#define BC_FALSE_SHARING_GUARD(type, name)                                                                                                 \
    BC_CACHE_LINE_ALIGNED type name;                                                                                                       \
    char _bc_pad_##name[BC_CACHE_LINE_SIZE - sizeof(type)]

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
