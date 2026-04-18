// SPDX-License-Identifier: MIT

#include "bc_core.h"

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/mman.h>
#include <time.h>

static void* bench_alloc(size_t size)
{
    void* ptr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (ptr == MAP_FAILED) {
        return NULL;
    }
    bc_core_zero(ptr, size);
    return ptr;
}

static void bench_free(void* ptr, size_t size)
{
    munmap(ptr, size);
}

static uint64_t now_ns(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ULL + (uint64_t)ts.tv_nsec;
}

#define ITERS 10000000
#define KB4 4096
#define KB64 65536
#define KB512 524288
#define MB1 (1024 * 1024)

/* ===== copy ===== */

static void bench_copy(const char* label, size_t size, size_t iters)
{
    uint8_t* src = bench_alloc(size);
    uint8_t* dst = bench_alloc(size);
    for (size_t i = 0; i < size; i++) {
        src[i] = (uint8_t)(i & 0xff);
    }

    uint64_t t0 = now_ns();
    for (size_t i = 0; i < iters; i++) {
        bc_core_copy(dst, src, size);
    }
    uint64_t elapsed = now_ns() - t0;

    double gbps = (double)(size * iters) / (double)elapsed;
    printf("  copy %-8s  %6.1f ns/op  %5.1f GB/s\n", label, (double)elapsed / (double)iters, gbps);

    bench_free(src, size);
    bench_free(dst, size);
}

/* ===== fill ===== */

static void bench_fill(const char* label, size_t size, size_t iters)
{
    uint8_t* dst = bench_alloc(size);

    uint64_t t0 = now_ns();
    for (size_t i = 0; i < iters; i++) {
        bc_core_fill(dst, size, 0xAB);
    }
    uint64_t elapsed = now_ns() - t0;

    double gbps = (double)(size * iters) / (double)elapsed;
    printf("  fill %-8s  %6.1f ns/op  %5.1f GB/s\n", label, (double)elapsed / (double)iters, gbps);

    bench_free(dst, size);
}

/* ===== zero ===== */

static void bench_zero(const char* label, size_t size, size_t iters)
{
    uint8_t* dst = bench_alloc(size);

    uint64_t t0 = now_ns();
    for (size_t i = 0; i < iters; i++) {
        bc_core_zero(dst, size);
    }
    uint64_t elapsed = now_ns() - t0;

    double gbps = (double)(size * iters) / (double)elapsed;
    printf("  zero %-8s  %6.1f ns/op  %5.1f GB/s\n", label, (double)elapsed / (double)iters, gbps);

    bench_free(dst, size);
}

/* ===== zero_secure ===== */

static void bench_zero_secure(const char* label, size_t size, size_t iters)
{
    uint8_t* dst = bench_alloc(size);
    bc_core_fill(dst, size, 0xFF);

    uint64_t t0 = now_ns();
    for (size_t i = 0; i < iters; i++) {
        bc_core_zero_secure(dst, size);
    }
    uint64_t elapsed = now_ns() - t0;

    double gbps = (double)(size * iters) / (double)elapsed;
    printf("  zero_secure %-4s  %6.1f ns/op  %5.1f GB/s\n", label, (double)elapsed / (double)iters, gbps);

    bench_free(dst, size);
}

/* ===== move ===== */

static void bench_move_overlap(const char* label, size_t size, size_t iters)
{
    uint8_t* buf = bench_alloc(size + 32);
    for (size_t i = 0; i < size + 32; i++) {
        buf[i] = (uint8_t)(i & 0xff);
    }

    uint64_t t0 = now_ns();
    for (size_t i = 0; i < iters; i++) {
        bc_core_move(buf, buf + 32, size);
    }
    uint64_t elapsed = now_ns() - t0;

    double gbps = (double)(size * iters) / (double)elapsed;
    printf("  move_overlap %-4s  %6.1f ns/op  %5.1f GB/s\n", label, (double)elapsed / (double)iters, gbps);

    bench_free(buf, size + 32);
}

static void bench_move_nooverlap(const char* label, size_t size, size_t iters)
{
    uint8_t* src = bench_alloc(size);
    uint8_t* dst = bench_alloc(size);
    for (size_t i = 0; i < size; i++) {
        src[i] = (uint8_t)(i & 0xff);
    }

    uint64_t t0 = now_ns();
    for (size_t i = 0; i < iters; i++) {
        bc_core_move(dst, src, size);
    }
    uint64_t elapsed = now_ns() - t0;

    double gbps = (double)(size * iters) / (double)elapsed;
    printf("  move_nooverlap %-2s  %6.1f ns/op  %5.1f GB/s\n", label, (double)elapsed / (double)iters, gbps);

    bench_free(src, size);
    bench_free(dst, size);
}

/* ===== length ===== */

static void bench_length(void)
{
    char buf[1024];
    for (size_t i = 0; i < sizeof(buf) - 1; i++) {
        buf[i] = 'a';
    }
    buf[sizeof(buf) - 1] = '\0';

    size_t out = 0;
    uint64_t t0 = now_ns();
    for (size_t i = 0; i < ITERS; i++) {
        bc_core_length(buf, '\0', &out);
    }
    uint64_t elapsed = now_ns() - t0;

    printf("  length 1KB     %6.1f ns/op  result=%zu\n", (double)elapsed / ITERS, out);
}

/* ===== find ===== */

static void bench_find_byte(void)
{
    uint8_t buf[KB4];
    for (size_t i = 0; i < KB4; i++) {
        buf[i] = (uint8_t)(i & 0xfe);
    }
    buf[KB4 - 1] = 0xff;

    size_t pos = 0;
    uint64_t t0 = now_ns();
    for (size_t i = 0; i < ITERS; i++) {
        bc_core_find_byte(buf, KB4, 0xff, &pos);
    }
    uint64_t elapsed = now_ns() - t0;

    printf("  find_byte 4KB (last)       %6.1f ns/op  pos=%zu\n", (double)elapsed / ITERS, pos);
}

static void bench_find_last_byte(void)
{
    uint8_t buf[KB4];
    for (size_t i = 0; i < KB4; i++) {
        buf[i] = (uint8_t)(i & 0xfe);
    }
    buf[0] = 0xff;

    size_t pos = 0;
    uint64_t t0 = now_ns();
    for (size_t i = 0; i < ITERS; i++) {
        bc_core_find_last_byte(buf, KB4, 0xff, &pos);
    }
    uint64_t elapsed = now_ns() - t0;

    printf("  find_last_byte 4KB (first) %6.1f ns/op  pos=%zu\n", (double)elapsed / ITERS, pos);
}

static void bench_find_any_byte(void)
{
    uint8_t buf[KB4];
    for (size_t i = 0; i < KB4; i++) {
        buf[i] = (uint8_t)((i & 0xf0) | 0x01);
    }
    buf[KB4 / 2] = 0xAB;

    const unsigned char targets[4] = {0xAB, 0xCD, 0xEF, 0x12};
    size_t pos = 0;
    uint64_t t0 = now_ns();
    for (size_t i = 0; i < ITERS; i++) {
        bc_core_find_any_byte(buf, KB4, targets, 4, &pos);
    }
    uint64_t elapsed = now_ns() - t0;

    printf("  find_any_byte 4KB (mid)    %6.1f ns/op  pos=%zu\n", (double)elapsed / ITERS, pos);
}

/* ===== count ===== */

static void bench_count_byte(void)
{
    uint8_t* buf = bench_alloc(KB64);
    for (size_t i = 0; i < KB64; i++) {
        buf[i] = (uint8_t)(i & 0xff);
    }

    size_t count = 0;
    size_t iters = 1000000;
    uint64_t t0 = now_ns();
    for (size_t i = 0; i < iters; i++) {
        bc_core_count_byte(buf, KB64, 0x41, &count);
    }
    uint64_t elapsed = now_ns() - t0;

    double gbps = (double)((uint64_t)KB64 * iters) / (double)elapsed;
    printf("  count_byte 64KB            %6.1f ns/op  %5.1f GB/s  count=%zu\n", (double)elapsed / (double)iters, gbps, count);

    bench_free(buf, KB64);
}

static void bench_count_lines(void)
{
    uint8_t* buf = bench_alloc(KB64);
    for (size_t i = 0; i < KB64; i++) {
        buf[i] = (i % 10 == 9) ? '\n' : 'a';
    }

    size_t count = 0;
    size_t iters = 1000000;
    uint64_t t0 = now_ns();
    for (size_t i = 0; i < iters; i++) {
        bc_core_count_lines(buf, KB64, &count);
    }
    uint64_t elapsed = now_ns() - t0;

    double gbps = (double)((uint64_t)KB64 * iters) / (double)elapsed;
    printf("  count_lines 64KB (~10%% nl) %6.1f ns/op  %5.1f GB/s  lines=%zu\n", (double)elapsed / (double)iters, gbps, count);

    bench_free(buf, KB64);
}

/* ===== compare ===== */

static void bench_compare(const char* label, size_t size, size_t iters)
{
    uint8_t* a = bench_alloc(size);
    uint8_t* b = bench_alloc(size);
    for (size_t i = 0; i < size; i++) {
        a[i] = b[i] = (uint8_t)(i & 0xff);
    }

    int result = 0;
    uint64_t t0 = now_ns();
    for (size_t i = 0; i < iters; i++) {
        bc_core_compare(a, b, size, &result);
    }
    uint64_t elapsed = now_ns() - t0;

    double gbps = (double)(size * iters) / (double)elapsed;
    printf("  compare %-6s  %6.1f ns/op  %5.1f GB/s  result=%d\n", label, (double)elapsed / (double)iters, gbps, result);

    bench_free(a, size);
    bench_free(b, size);
}

/* ===== swap ===== */

static void bench_swap(const char* label, size_t size, size_t iters)
{
    uint8_t* a = bench_alloc(size);
    uint8_t* b = bench_alloc(size);
    for (size_t i = 0; i < size; i++) {
        a[i] = (uint8_t)(i & 0xff);
        b[i] = (uint8_t)((i + 128) & 0xff);
    }

    uint64_t t0 = now_ns();
    for (size_t i = 0; i < iters; i++) {
        bc_core_swap(a, b, size);
    }
    uint64_t elapsed = now_ns() - t0;

    double gbps = (double)(size * 2 * iters) / (double)elapsed;
    printf("  swap %-8s  %6.1f ns/op  %5.1f GB/s\n", label, (double)elapsed / (double)iters, gbps);

    bench_free(a, size);
    bench_free(b, size);
}

/* ===== find_pattern ===== */

static void bench_find_pattern(void)
{
    uint8_t buf[KB4];
    bc_core_fill(buf, KB4, (unsigned char)0x00);
    const uint8_t pattern[6] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF};
    bc_core_copy(buf + KB4 - 6, pattern, 6);

    size_t pos = 0;
    uint64_t t0 = now_ns();
    for (size_t i = 0; i < ITERS; i++) {
        bc_core_find_pattern(buf, KB4, pattern, 6, &pos);
    }
    uint64_t elapsed = now_ns() - t0;

    printf("  find_pattern 4KB (end)     %6.1f ns/op  pos=%zu\n", (double)elapsed / ITERS, pos);
}

/* ===== count_words ===== */

static void bench_count_words(void)
{
    uint8_t* buf = bench_alloc(KB64);
    for (size_t i = 0; i < KB64; i++) {
        buf[i] = (i % 6 == 5) ? ' ' : 'a';
    }

    size_t count = 0;
    bool in_word = false;
    size_t iters = 1000000;
    uint64_t t0 = now_ns();
    for (size_t i = 0; i < iters; i++) {
        in_word = false;
        bc_core_count_words(buf, KB64, &in_word, &count);
    }
    uint64_t elapsed = now_ns() - t0;

    double gbps = (double)((uint64_t)KB64 * iters) / (double)elapsed;
    printf("  count_words 64KB (~17%% sp) %6.1f ns/op  %5.1f GB/s  words=%zu\n", (double)elapsed / (double)iters, gbps, count);

    bench_free(buf, KB64);
}

static void bench_count_words_ascii(void)
{
    uint8_t* buf = bench_alloc(KB64);
    for (size_t i = 0; i < KB64; i++) {
        buf[i] = (i % 6 == 5) ? ' ' : 'a';
    }

    size_t count = 0;
    bool in_word = false;
    size_t iters = 1000000;
    uint64_t t0 = now_ns();
    for (size_t i = 0; i < iters; i++) {
        in_word = false;
        bc_core_count_words_ascii(buf, KB64, &in_word, &count);
    }
    uint64_t elapsed = now_ns() - t0;

    double gbps = (double)((uint64_t)KB64 * iters) / (double)elapsed;
    printf("  count_words_ascii 64KB (~17%% sp) %6.1f ns/op  %5.1f GB/s  words=%zu\n", (double)elapsed / (double)iters, gbps, count);

    bench_free(buf, KB64);
}

/* ===== equal ===== */

static void bench_equal(void)
{
    uint8_t a[KB4];
    uint8_t b[KB4];
    for (size_t i = 0; i < KB4; i++) {
        a[i] = b[i] = (uint8_t)(i & 0xff);
    }

    bool eq = false;
    uint64_t t0 = now_ns();
    for (size_t i = 0; i < ITERS; i++) {
        bc_core_equal(a, b, KB4, &eq);
    }
    uint64_t elapsed = now_ns() - t0;

    double gbps = (double)(KB4 * (uint64_t)ITERS) / (double)elapsed;
    printf("  equal 4KB      %6.1f ns/op  %5.1f GB/s  eq=%d\n", (double)elapsed / ITERS, gbps, eq);
}

/* ===== crc32c ===== */

static void bench_crc32c_sweep(const char* label, size_t size, size_t iters)
{
    uint8_t* buf = bench_alloc(size);
    for (size_t i = 0; i < size; i++) {
        buf[i] = (uint8_t)(i & 0xff);
    }

    uint32_t crc = 0;
    uint64_t t0 = now_ns();
    for (size_t i = 0; i < iters; i++) {
        bc_core_crc32c(buf, size, &crc);
    }
    uint64_t elapsed = now_ns() - t0;

    double gbps = (double)(size * iters) / (double)elapsed;
    printf("  crc32c %-6s  %6.1f ns/op  %5.1f GB/s  crc=0x%08x\n", label, (double)elapsed / (double)iters, gbps, crc);

    bench_free(buf, size);
}

static void bench_crc32c_update_chain(void)
{
    uint8_t buf[KB4];
    for (size_t i = 0; i < KB4; i++) {
        buf[i] = (uint8_t)(i & 0xff);
    }

    size_t iters = 1000000;
    uint32_t crc = 0;
    uint64_t t0 = now_ns();
    for (size_t i = 0; i < iters; i++) {
        crc = 0;
        for (int j = 0; j < 10; j++) {
            bc_core_crc32c_update(crc, buf, KB4, &crc);
        }
    }
    uint64_t elapsed = now_ns() - t0;

    double ns_per_block = (double)elapsed / ((double)iters * 10.0);
    double gbps = (double)(KB4 * 10ULL * iters) / (double)elapsed;
    printf("  crc32c_update 10x4KB  %6.1f ns/block  %5.1f GB/s  crc=0x%08x\n", ns_per_block, gbps, crc);
}

/* ===== cache control ===== */

static void bench_prefetch_overhead(void)
{
    const size_t size = 4 * 1024 * 1024;
    uint8_t* buf = bench_alloc(size);
    bc_core_fill(buf, size, 0x42);

    size_t iters = 200;
    uint32_t crc = 0;

    uint64_t t0 = now_ns();
    for (size_t i = 0; i < iters; i++) {
        bc_core_crc32c(buf, size, &crc);
    }
    double gbps_warm = (double)(size * iters) / (double)(now_ns() - t0);

    t0 = now_ns();
    for (size_t i = 0; i < iters; i++) {
        bc_core_prefetch_read_range(buf, size);
        bc_core_crc32c(buf, size, &crc);
    }
    double gbps_pf = (double)(size * iters) / (double)(now_ns() - t0);

    printf("  prefetch_range 4MB  L3_warm: %5.2f GB/s  prefetch+crc: %5.2f GB/s\n", gbps_warm, gbps_pf);

    bench_free(buf, size);
}

static void bench_evict_effect(void)
{
    const size_t size = 4 * 1024 * 1024;
    uint8_t* buf = bench_alloc(size);
    bc_core_fill(buf, size, 0x42);

    size_t iters = 50;
    uint32_t crc = 0;

    bc_core_crc32c(buf, size, &crc);

    uint64_t t0 = now_ns();
    for (size_t i = 0; i < iters; i++) {
        bc_core_crc32c(buf, size, &crc);
    }
    double gbps_warm = (double)(size * iters) / (double)(now_ns() - t0);

    bc_core_evict_range(buf, size);
    uint64_t elapsed_cold = 0;
    for (size_t i = 0; i < iters; i++) {
        bc_core_evict_range(buf, size);
        uint64_t ts = now_ns();
        bc_core_crc32c(buf, size, &crc);
        elapsed_cold += now_ns() - ts;
    }
    double gbps_cold = (double)(size * iters) / (double)elapsed_cold;

    const size_t evict_iters = 500;
    t0 = now_ns();
    for (size_t i = 0; i < evict_iters; i++) {
        bc_core_evict_range(buf, size);
    }
    double gbps_evict = (double)(size * evict_iters) / (double)(now_ns() - t0);

    printf("  evict_range 4MB  evict_throughput: %5.2f GB/s  L3_warm: %5.2f GB/s  post_evict_cold: %5.2f GB/s\n", gbps_evict, gbps_warm,
           gbps_cold);

    bench_free(buf, size);
}

static void bench_copy_policy(void)
{
    uint8_t* src = bench_alloc(MB1);
    uint8_t* dst = bench_alloc(MB1);
    size_t iters = 10000;

    uint64_t t0 = now_ns();
    for (size_t i = 0; i < iters; i++) {
        bc_core_copy_with_policy(dst, src, MB1, BC_CORE_CACHE_POLICY_DEFAULT);
    }
    double gbps_default = (double)(MB1 * iters) / (double)(now_ns() - t0);

    t0 = now_ns();
    for (size_t i = 0; i < iters; i++) {
        bc_core_copy_with_policy(dst, src, MB1, BC_CORE_CACHE_POLICY_CACHED);
    }
    double gbps_cached = (double)(MB1 * iters) / (double)(now_ns() - t0);

    t0 = now_ns();
    for (size_t i = 0; i < iters; i++) {
        bc_core_copy_with_policy(dst, src, MB1, BC_CORE_CACHE_POLICY_STREAMING);
    }
    double gbps_streaming = (double)(MB1 * iters) / (double)(now_ns() - t0);

    printf("  copy_policy 1MB  default: %5.2f GB/s  cached: %5.2f GB/s  streaming: %5.2f GB/s\n", gbps_default, gbps_cached,
           gbps_streaming);

    bench_free(src, MB1);
    bench_free(dst, MB1);
}

/* ===== sha256 ===== */

static void bench_sha256_sweep(const char* label, size_t size, size_t iters)
{
    uint8_t* buf = bench_alloc(size);
    for (size_t i = 0; i < size; i++) {
        buf[i] = (uint8_t)(i & 0xff);
    }

    uint8_t hash[32];
    uint64_t t0 = now_ns();
    for (size_t i = 0; i < iters; i++) {
        bc_core_sha256(buf, size, hash);
    }
    uint64_t elapsed = now_ns() - t0;

    double mbps = (double)(size * iters) / (double)elapsed * 1000.0;
    printf("  sha256 %-6s  %7.0f ns/op  %6.0f MB/s\n", label, (double)elapsed / (double)iters, mbps);

    bench_free(buf, size);
}

static void bench_sha256_incremental(void)
{
    uint8_t buf[KB4];
    for (size_t i = 0; i < KB4; i++) {
        buf[i] = (uint8_t)(i & 0xff);
    }

    uint8_t hash[32];
    size_t iters = 100000;
    uint64_t t0 = now_ns();
    for (size_t i = 0; i < iters; i++) {
        bc_core_sha256_context_t ctx;
        bc_core_sha256_init(&ctx);
        bc_core_sha256_update(&ctx, buf, KB4);
        bc_core_sha256_update(&ctx, buf, KB4);
        bc_core_sha256_update(&ctx, buf, KB4);
        bc_core_sha256_update(&ctx, buf, KB4);
        bc_core_sha256_finalize(&ctx, hash);
    }
    uint64_t elapsed = now_ns() - t0;

    double mbps = (double)(4ULL * KB4 * iters) / (double)elapsed * 1000.0;
    printf("  sha256 incr 4x4KB  %7.0f ns/op  %6.0f MB/s\n", (double)elapsed / (double)iters, mbps);
}

int main(void)
{
    printf("bench_commun_ops\n\n");

    printf("--- copy ---\n");
    bench_copy("64B", 64, ITERS);
    bench_copy("4KB", KB4, ITERS);
    bench_copy("64KB", KB64, 500000);
    bench_copy("1MB", MB1, 10000);

    printf("\n--- fill ---\n");
    bench_fill("64B", 64, ITERS);
    bench_fill("4KB", KB4, ITERS);
    bench_fill("64KB", KB64, 500000);
    bench_fill("1MB", MB1, 10000);

    printf("\n--- zero ---\n");
    bench_zero("64B", 64, ITERS);
    bench_zero("4KB", KB4, ITERS);
    bench_zero("64KB", KB64, 500000);
    bench_zero("1MB", MB1, 10000);

    printf("\n--- zero_secure ---\n");
    bench_zero_secure("64B", 64, 1000000);
    bench_zero_secure("4KB", KB4, 100000);

    printf("\n--- move ---\n");
    bench_move_overlap("64B", 64, ITERS);
    bench_move_overlap("4KB", KB4, ITERS);
    bench_move_overlap("64KB", KB64, 500000);
    bench_move_nooverlap("64B", 64, ITERS);
    bench_move_nooverlap("4KB", KB4, ITERS);
    bench_move_nooverlap("64KB", KB64, 500000);

    printf("\n--- compare ---\n");
    bench_compare("4KB", KB4, ITERS);
    bench_compare("64KB", KB64, 500000);

    printf("\n--- swap ---\n");
    bench_swap("4KB", KB4, ITERS);
    bench_swap("64KB", KB64, 500000);

    printf("\n--- search ---\n");
    bench_length();
    bench_find_byte();
    bench_find_last_byte();
    bench_find_any_byte();
    bench_find_pattern();
    bench_equal();

    printf("\n--- count ---\n");
    bench_count_byte();
    bench_count_lines();
    bench_count_words();
    bench_count_words_ascii();

    printf("\n--- crc32c sweep ---\n");
    bench_crc32c_sweep("64B", 64, ITERS);
    bench_crc32c_sweep("512B", 512, ITERS);
    bench_crc32c_sweep("4KB", KB4, ITERS);
    bench_crc32c_sweep("64KB", KB64, 500000);
    bench_crc32c_sweep("512KB", KB512, 50000);
    bench_crc32c_sweep("1MB", MB1, 10000);
    printf("\n");
    bench_crc32c_update_chain();

    printf("\n--- cache control ---\n");
    bench_prefetch_overhead();
    bench_evict_effect();
    bench_copy_policy();

    printf("\n--- sha256 sweep ---\n");
    bench_sha256_sweep("64B", 64, 1000000);
    bench_sha256_sweep("512B", 512, 500000);
    bench_sha256_sweep("4KB", KB4, 100000);
    bench_sha256_sweep("64KB", KB64, 10000);
    printf("\n");
    bench_sha256_incremental();

    return 0;
}
