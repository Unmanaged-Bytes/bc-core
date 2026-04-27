// SPDX-License-Identifier: MIT

#include "bc_core.h"

#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>

#define FMT_ITERS 5000000U

static uint64_t now_ns(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ULL + (uint64_t)ts.tv_nsec;
}

static void bench_uint64_dec(void)
{
    char buffer[24];
    volatile size_t sink = 0;

    uint64_t start_ours = now_ns();
    for (unsigned iteration = 0; iteration < FMT_ITERS; iteration++) {
        size_t length = 0;
        bc_core_format_unsigned_integer_64_decimal(buffer, sizeof(buffer), (uint64_t)iteration * 31U + 17U, &length);
        sink += length;
    }
    uint64_t elapsed_ours = now_ns() - start_ours;

    uint64_t start_snprintf = now_ns();
    for (unsigned iteration = 0; iteration < FMT_ITERS; iteration++) {
        int length = snprintf(buffer, sizeof(buffer), "%" PRIu64, (uint64_t)iteration * 31U + 17U);
        sink += (size_t)length;
    }
    uint64_t elapsed_snprintf = now_ns() - start_snprintf;

    printf("  uint64_dec  bc_core=%6.1f ns/op  snprintf=%6.1f ns/op  speedup=%4.2fx\n",
           (double)elapsed_ours / (double)FMT_ITERS,
           (double)elapsed_snprintf / (double)FMT_ITERS,
           (double)elapsed_snprintf / (double)elapsed_ours);
    (void)sink;
}

static void bench_uint64_hex(void)
{
    char buffer[24];
    volatile size_t sink = 0;

    uint64_t start_ours = now_ns();
    for (unsigned iteration = 0; iteration < FMT_ITERS; iteration++) {
        size_t length = 0;
        bc_core_format_unsigned_integer_64_hexadecimal(buffer, sizeof(buffer), (uint64_t)iteration * 0xDEADBEEFU, &length);
        sink += length;
    }
    uint64_t elapsed_ours = now_ns() - start_ours;

    uint64_t start_snprintf = now_ns();
    for (unsigned iteration = 0; iteration < FMT_ITERS; iteration++) {
        int length = snprintf(buffer, sizeof(buffer), "%" PRIx64, (uint64_t)iteration * 0xDEADBEEFU);
        sink += (size_t)length;
    }
    uint64_t elapsed_snprintf = now_ns() - start_snprintf;

    printf("  uint64_hex  bc_core=%6.1f ns/op  snprintf=%6.1f ns/op  speedup=%4.2fx\n",
           (double)elapsed_ours / (double)FMT_ITERS,
           (double)elapsed_snprintf / (double)FMT_ITERS,
           (double)elapsed_snprintf / (double)elapsed_ours);
    (void)sink;
}

static void bench_int64(void)
{
    char buffer[24];
    volatile size_t sink = 0;

    uint64_t start_ours = now_ns();
    for (unsigned iteration = 0; iteration < FMT_ITERS; iteration++) {
        size_t length = 0;
        int64_t value = (int64_t)iteration * ((iteration & 1U) ? 1 : -1);
        bc_core_format_signed_integer_64(buffer, sizeof(buffer), value, &length);
        sink += length;
    }
    uint64_t elapsed_ours = now_ns() - start_ours;

    uint64_t start_snprintf = now_ns();
    for (unsigned iteration = 0; iteration < FMT_ITERS; iteration++) {
        int64_t value = (int64_t)iteration * ((iteration & 1U) ? 1 : -1);
        int length = snprintf(buffer, sizeof(buffer), "%" PRId64, value);
        sink += (size_t)length;
    }
    uint64_t elapsed_snprintf = now_ns() - start_snprintf;

    printf("  int64       bc_core=%6.1f ns/op  snprintf=%6.1f ns/op  speedup=%4.2fx\n",
           (double)elapsed_ours / (double)FMT_ITERS,
           (double)elapsed_snprintf / (double)FMT_ITERS,
           (double)elapsed_snprintf / (double)elapsed_ours);
    (void)sink;
}

static void bench_double(void)
{
    char buffer[64];
    volatile size_t sink = 0;

    uint64_t start_ours = now_ns();
    for (unsigned iteration = 0; iteration < FMT_ITERS; iteration++) {
        size_t length = 0;
        double value = (double)iteration * 0.1 + 1.0;
        bc_core_format_double(buffer, sizeof(buffer), value, 3, &length);
        sink += length;
    }
    uint64_t elapsed_ours = now_ns() - start_ours;

    uint64_t start_snprintf = now_ns();
    for (unsigned iteration = 0; iteration < FMT_ITERS; iteration++) {
        double value = (double)iteration * 0.1 + 1.0;
        int length = snprintf(buffer, sizeof(buffer), "%.3f", value);
        sink += (size_t)length;
    }
    uint64_t elapsed_snprintf = now_ns() - start_snprintf;

    printf("  double      bc_core=%6.1f ns/op  snprintf=%6.1f ns/op  speedup=%4.2fx\n",
           (double)elapsed_ours / (double)FMT_ITERS,
           (double)elapsed_snprintf / (double)FMT_ITERS,
           (double)elapsed_snprintf / (double)elapsed_ours);
    (void)sink;
}

int main(void)
{
    printf("bench bc_core_format vs snprintf (%u iterations)\n", FMT_ITERS);
    bench_uint64_dec();
    bench_uint64_hex();
    bench_int64();
    bench_double();
    return 0;
}
