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

/* ===== Streaming (NT stores) thresholds — measured Zen 3 + Tiger Lake =====
   Above these, BC_CORE_CACHE_POLICY_STREAMING outperforms (or equals) the
   cached path on bandwidth-bound primitives across both AMD Zen 3 and Intel
   Tiger Lake. Plat at 4 MiB:
   - On AMD Zen 3, copy crossover is ~1 MiB; we accept a marginal loss in the
     1-4 MiB range (no current consumer hits this band on the hot path).
   - On Intel Tiger Lake, copy STREAMING never wins regardless of size; 4 MiB
     keeps it from being engaged on workloads where it would regress.
   These constants double as compile-time fallbacks for
   bc_core_buffer_thresholds() when runtime cache detection fails. */
#define BC_BUFFER_COPY_STREAMING_THRESHOLD ((size_t)4 * 1024 * 1024)
#define BC_BUFFER_ZERO_STREAMING_THRESHOLD ((size_t)4 * 1024 * 1024)
#define BC_BUFFER_FILL_STREAMING_THRESHOLD ((size_t)4 * 1024 * 1024)
#define BC_BUFFER_COPY_STREAMING_THRESHOLD_FALLBACK BC_BUFFER_COPY_STREAMING_THRESHOLD
#define BC_BUFFER_ZERO_STREAMING_THRESHOLD_FALLBACK BC_BUFFER_ZERO_STREAMING_THRESHOLD
#define BC_BUFFER_FILL_STREAMING_THRESHOLD_FALLBACK BC_BUFFER_FILL_STREAMING_THRESHOLD

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
    BC_CORE_CACHE_POLICY_AUTO,
} bc_core_cache_policy_t;

/* Worker-aware buffer thresholds derived from the cache topology of the
   current host. l3_per_thread_bytes splits L3 by the worker_count_hint passed
   to bc_core_buffer_thresholds(); the *_streaming_threshold_bytes fields are
   floored at 4 MiB so that the streaming path is never engaged on buffers
   that fit comfortably in a per-thread L3 slice. */
typedef struct bc_core_buffer_thresholds {
    size_t l1_hot_bytes;
    size_t l2_hot_bytes;
    size_t l3_per_thread_bytes;
    size_t copy_streaming_threshold_bytes;
    size_t zero_streaming_threshold_bytes;
    size_t fill_streaming_threshold_bytes;
} bc_core_buffer_thresholds_t;

bool bc_core_chunk_size(size_t* out_size);
bool bc_core_preferred_alignment(size_t* out_alignment);
bool bc_core_cache_line_size(size_t* out_size);
bool bc_core_l1d_cache_size(size_t* out_size);
bool bc_core_l2_cache_size(size_t* out_size);
bool bc_core_l3_cache_size(size_t* out_size);
void bc_core_prefetch_read_range(const void* data, size_t len);
void bc_core_evict_range(const void* data, size_t len);

/* Compute buffer thresholds for the given concurrency level.
   worker_count_hint is the number of threads expected to share L3; pass 0
   or 1 for single-thread defaults. Returns false when cache size detection
   failed; in that case the out struct is filled with conservative
   compile-time fallbacks (BC_BUFFER_*) and the caller may still rely on
   the values. */
bool bc_core_buffer_thresholds(size_t worker_count_hint, bc_core_buffer_thresholds_t* out);

/* Convenience wrapper: uses sysconf(_SC_NPROCESSORS_ONLN) (capped to 1) as
   the worker count hint. */
bool bc_core_buffer_thresholds_default(bc_core_buffer_thresholds_t* out);

#endif /* BC_CORE_CPU_H */
