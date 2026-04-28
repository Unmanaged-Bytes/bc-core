// SPDX-License-Identifier: MIT

#include "bc_core.h"

#include <pthread.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

typedef enum {
    MODE_DEFAULT,
    MODE_STREAMING,
    MODE_AUTO_THREADED,
} bench_mode_t;

typedef struct worker_args {
    uint8_t* source;
    uint8_t* destination;
    size_t buffer_size;
    size_t iterations;
    bench_mode_t mode;
    size_t worker_count_hint;
    pthread_barrier_t* start_barrier;
    pthread_barrier_t* finish_barrier;
} worker_args_t;

static void* worker_main(void* user_data)
{
    worker_args_t* args = (worker_args_t*)user_data;
    pthread_barrier_wait(args->start_barrier);
    for (size_t iteration = 0; iteration < args->iterations; iteration++) {
        switch (args->mode) {
        case MODE_DEFAULT:
            bc_core_copy_with_policy(args->destination, args->source, args->buffer_size, BC_CORE_CACHE_POLICY_DEFAULT);
            break;
        case MODE_STREAMING:
            bc_core_copy_with_policy(args->destination, args->source, args->buffer_size, BC_CORE_CACHE_POLICY_STREAMING);
            break;
        case MODE_AUTO_THREADED:
            bc_core_copy_with_policy_threaded(args->destination, args->source, args->buffer_size, BC_CORE_CACHE_POLICY_AUTO,
                                              args->worker_count_hint);
            break;
        }
    }
    pthread_barrier_wait(args->finish_barrier);
    return NULL;
}

static double measure_run(size_t buffer_size, size_t worker_count, size_t iterations, bench_mode_t mode)
{
    pthread_t* threads = calloc(worker_count, sizeof(pthread_t));
    worker_args_t* args = calloc(worker_count, sizeof(worker_args_t));
    uint8_t** sources = calloc(worker_count, sizeof(uint8_t*));
    uint8_t** destinations = calloc(worker_count, sizeof(uint8_t*));
    if (threads == NULL || args == NULL || sources == NULL || destinations == NULL) {
        free(threads);
        free(args);
        free(sources);
        free(destinations);
        return -1.0;
    }

    for (size_t worker_index = 0; worker_index < worker_count; worker_index++) {
        sources[worker_index] = bench_alloc(buffer_size);
        destinations[worker_index] = bench_alloc(buffer_size);
        if (sources[worker_index] == NULL || destinations[worker_index] == NULL) {
            for (size_t cleanup_index = 0; cleanup_index <= worker_index; cleanup_index++) {
                if (sources[cleanup_index] != NULL) {
                    bench_free(sources[cleanup_index], buffer_size);
                }
                if (destinations[cleanup_index] != NULL) {
                    bench_free(destinations[cleanup_index], buffer_size);
                }
            }
            free(threads);
            free(args);
            free(sources);
            free(destinations);
            return -1.0;
        }
        for (size_t byte_index = 0; byte_index < buffer_size; byte_index += 4096) {
            sources[worker_index][byte_index] = (uint8_t)((byte_index + worker_index) & 0xFF);
        }
    }

    pthread_barrier_t start_barrier;
    pthread_barrier_t finish_barrier;
    pthread_barrier_init(&start_barrier, NULL, (unsigned int)(worker_count + 1));
    pthread_barrier_init(&finish_barrier, NULL, (unsigned int)(worker_count + 1));

    for (size_t worker_index = 0; worker_index < worker_count; worker_index++) {
        args[worker_index].source = sources[worker_index];
        args[worker_index].destination = destinations[worker_index];
        args[worker_index].buffer_size = buffer_size;
        args[worker_index].iterations = iterations;
        args[worker_index].mode = mode;
        args[worker_index].worker_count_hint = worker_count;
        args[worker_index].start_barrier = &start_barrier;
        args[worker_index].finish_barrier = &finish_barrier;
        pthread_create(&threads[worker_index], NULL, worker_main, &args[worker_index]);
    }

    pthread_barrier_wait(&start_barrier);
    uint64_t time_start_ns = now_ns();
    pthread_barrier_wait(&finish_barrier);
    uint64_t elapsed_ns = now_ns() - time_start_ns;

    for (size_t worker_index = 0; worker_index < worker_count; worker_index++) {
        pthread_join(threads[worker_index], NULL);
    }

    pthread_barrier_destroy(&start_barrier);
    pthread_barrier_destroy(&finish_barrier);

    for (size_t worker_index = 0; worker_index < worker_count; worker_index++) {
        bench_free(sources[worker_index], buffer_size);
        bench_free(destinations[worker_index], buffer_size);
    }

    free(threads);
    free(args);
    free(sources);
    free(destinations);

    double total_bytes = (double)buffer_size * (double)iterations * (double)worker_count;
    double seconds = (double)elapsed_ns / 1e9;
    return total_bytes / seconds / 1e9;
}

static const char* mode_label(bench_mode_t mode)
{
    switch (mode) {
    case MODE_DEFAULT:
        return "DEFAULT";
    case MODE_STREAMING:
        return "STREAMING";
    case MODE_AUTO_THREADED:
        return "AUTO_THREADED";
    }
    return "?";
}

static size_t pick_iterations(size_t buffer_size)
{
    const size_t mib = (size_t)1024 * 1024;
    if (buffer_size <= mib) {
        return 4000;
    }
    if (buffer_size <= 4 * mib) {
        return 2000;
    }
    if (buffer_size <= 8 * mib) {
        return 1000;
    }
    return 500;
}

static void run_size_workers(FILE* json, size_t buffer_size, size_t worker_count, bool* first_entry)
{
    const size_t iterations = pick_iterations(buffer_size);
    const bench_mode_t modes[] = {MODE_DEFAULT, MODE_STREAMING, MODE_AUTO_THREADED};

    double throughput[3];
    for (size_t mode_index = 0; mode_index < 3; mode_index++) {
        double observed = measure_run(buffer_size, worker_count, iterations, modes[mode_index]);
        if (observed < 0.0) {
            observed = 0.0;
        }
        throughput[mode_index] = observed;
        if (json != NULL) {
            if (!*first_entry) {
                fprintf(json, ",\n");
            }
            *first_entry = false;
            fprintf(json,
                    "    {\"buffer_bytes\": %zu, \"workers\": %zu, \"iterations\": %zu, \"mode\": \"%s\", "
                    "\"aggregate_throughput_gbps\": %.3f}",
                    buffer_size, worker_count, iterations, mode_label(modes[mode_index]), observed);
        }
    }

    double auto_vs_streaming = (throughput[MODE_STREAMING] > 0.0) ? throughput[MODE_AUTO_THREADED] / throughput[MODE_STREAMING] : 0.0;
    double auto_vs_default = (throughput[MODE_DEFAULT] > 0.0) ? throughput[MODE_AUTO_THREADED] / throughput[MODE_DEFAULT] : 0.0;

    printf(
        "  size=%5zu KiB workers=%2zu  default=%6.2f GB/s  streaming=%6.2f GB/s  auto=%6.2f GB/s  auto/stream=%4.2fx auto/default=%4.2fx\n",
        buffer_size / 1024, worker_count, throughput[MODE_DEFAULT], throughput[MODE_STREAMING], throughput[MODE_AUTO_THREADED],
        auto_vs_streaming, auto_vs_default);
}

int main(int argc, char** argv)
{
    const size_t mib = (size_t)1024 * 1024;
    const size_t buffer_sizes[] = {1 * mib, 2 * mib, 4 * mib, 8 * mib, 16 * mib};
    const size_t worker_counts[] = {1, 4, 8, 16};
    const size_t buffer_size_count = sizeof(buffer_sizes) / sizeof(buffer_sizes[0]);
    const size_t worker_count_count = sizeof(worker_counts) / sizeof(worker_counts[0]);

    FILE* json = NULL;
    const char* json_path = NULL;
    if (argc >= 2) {
        json_path = argv[1];
        json = fopen(json_path, "w");
        if (json != NULL) {
            fprintf(json, "{\n  \"results\": [\n");
        }
    }

    bc_core_buffer_thresholds_t thresholds_default;
    (void)bc_core_buffer_thresholds_default(&thresholds_default);
    printf("bench_bc_core_thresholds — host topology snapshot\n");
    printf("  l1_hot=%zu KiB  l2_hot=%zu KiB  l3_per_thread=%zu KiB  copy_streaming=%zu KiB (default)\n",
           thresholds_default.l1_hot_bytes / 1024, thresholds_default.l2_hot_bytes / 1024, thresholds_default.l3_per_thread_bytes / 1024,
           thresholds_default.copy_streaming_threshold_bytes / 1024);

    bool first_entry = true;
    for (size_t size_index = 0; size_index < buffer_size_count; size_index++) {
        for (size_t worker_index = 0; worker_index < worker_count_count; worker_index++) {
            run_size_workers(json, buffer_sizes[size_index], worker_counts[worker_index], &first_entry);
        }
    }

    printf("\n--- mono-thread small-buffer no-regression sanity ---\n");
    const size_t small_sizes[] = {4 * 1024, 64 * 1024, 256 * 1024, 1 * mib};
    const size_t small_size_count = sizeof(small_sizes) / sizeof(small_sizes[0]);
    for (size_t size_index = 0; size_index < small_size_count; size_index++) {
        run_size_workers(json, small_sizes[size_index], 1, &first_entry);
    }

    if (json != NULL) {
        fprintf(json, "\n  ]\n}\n");
        fclose(json);
        printf("\njson written to %s\n", json_path);
    }

    return 0;
}
