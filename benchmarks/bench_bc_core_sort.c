// SPDX-License-Identifier: MIT

#include "bc_core.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static uint64_t now_ns(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ULL + (uint64_t)ts.tv_nsec;
}

static bool less_than_uint64(const void* left, const void* right, void* user_data)
{
    (void)user_data;
    return *(const uint64_t*)left < *(const uint64_t*)right;
}

static int qsort_compare_uint64(const void* left, const void* right)
{
    uint64_t left_value = *(const uint64_t*)left;
    uint64_t right_value = *(const uint64_t*)right;
    if (left_value < right_value) {
        return -1;
    }
    if (left_value > right_value) {
        return 1;
    }
    return 0;
}

static void fill_random(uint64_t* array, size_t count, uint32_t seed)
{
    uint32_t state = seed;
    for (size_t index = 0; index < count; ++index) {
        state = state * 1664525U + 1013904223U;
        uint64_t high = state;
        state = state * 1664525U + 1013904223U;
        array[index] = (high << 32) | state;
    }
}

static void fill_ascending(uint64_t* array, size_t count)
{
    for (size_t index = 0; index < count; ++index) {
        array[index] = (uint64_t)index;
    }
}

static void fill_descending(uint64_t* array, size_t count)
{
    for (size_t index = 0; index < count; ++index) {
        array[index] = (uint64_t)(count - 1U - index);
    }
}

typedef enum {
    PATTERN_RANDOM,
    PATTERN_ASCENDING,
    PATTERN_DESCENDING,
} pattern_t;

static const char* pattern_label(pattern_t pattern)
{
    switch (pattern) {
    case PATTERN_RANDOM:
        return "random";
    case PATTERN_ASCENDING:
        return "ascending";
    case PATTERN_DESCENDING:
        return "descending";
    }
    return "?";
}

static void prepare(uint64_t* array, size_t count, pattern_t pattern, uint32_t seed)
{
    switch (pattern) {
    case PATTERN_RANDOM:
        fill_random(array, count, seed);
        break;
    case PATTERN_ASCENDING:
        fill_ascending(array, count);
        break;
    case PATTERN_DESCENDING:
        fill_descending(array, count);
        break;
    }
}

static void run_bench(size_t count, pattern_t pattern, size_t iters)
{
    uint64_t* original = malloc(sizeof(uint64_t) * count);
    uint64_t* working = malloc(sizeof(uint64_t) * count);
    if (original == NULL || working == NULL) {
        free(original);
        free(working);
        return;
    }

    prepare(original, count, pattern, 0xC0FFEEU);

    uint64_t bc_total = 0;
    for (size_t iteration = 0; iteration < iters; ++iteration) {
        memcpy(working, original, sizeof(uint64_t) * count);
        uint64_t t0 = now_ns();
        (void)bc_core_sort_with_compare(working, count, sizeof(uint64_t), less_than_uint64, NULL);
        bc_total += now_ns() - t0;
    }

    uint64_t libc_total = 0;
    for (size_t iteration = 0; iteration < iters; ++iteration) {
        memcpy(working, original, sizeof(uint64_t) * count);
        uint64_t t0 = now_ns();
        qsort(working, count, sizeof(uint64_t), qsort_compare_uint64);
        libc_total += now_ns() - t0;
    }

    double bc_per = (double)bc_total / (double)iters;
    double libc_per = (double)libc_total / (double)iters;
    double ratio = libc_per / bc_per;
    printf("  sort %-10s n=%-6zu  bc=%9.0f ns  qsort=%9.0f ns  ratio=%4.2fx\n", pattern_label(pattern), count, bc_per, libc_per, ratio);

    free(original);
    free(working);
}

int main(void)
{
    printf("--- bc_core_sort_with_compare vs qsort (uint64_t arrays) ---\n");
    const size_t sizes[] = {16U, 64U, 256U, 1024U, 4096U, 16384U, 65536U};
    const size_t size_count = sizeof(sizes) / sizeof(sizes[0]);
    const size_t iters_per_size[] = {200000U, 50000U, 10000U, 2000U, 500U, 100U, 20U};

    for (size_t index = 0; index < size_count; ++index) {
        run_bench(sizes[index], PATTERN_RANDOM, iters_per_size[index]);
    }
    printf("\n");
    for (size_t index = 0; index < size_count; ++index) {
        run_bench(sizes[index], PATTERN_ASCENDING, iters_per_size[index]);
    }
    printf("\n");
    for (size_t index = 0; index < size_count; ++index) {
        run_bench(sizes[index], PATTERN_DESCENDING, iters_per_size[index]);
    }

    return 0;
}
