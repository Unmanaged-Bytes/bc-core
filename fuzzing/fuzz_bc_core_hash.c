// SPDX-License-Identifier: MIT

#include "bc_core.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void fuzz_crc32c(const uint8_t* data, size_t size)
{
    uint32_t hash = 0;
    bc_core_crc32c(data, size, &hash);

    uint32_t prev = 0;
    for (size_t chunk = 1; chunk <= size && chunk <= 64; chunk *= 2) {
        size_t offset = 0;
        prev = 0;
        while (offset < size) {
            size_t step = (offset + chunk <= size) ? chunk : size - offset;
            uint32_t next = 0;
            bc_core_crc32c_update(prev, data + offset, step, &next);
            prev = next;
            offset += step;
        }
    }
    (void)prev;
}

static void fuzz_sha256(const uint8_t* data, size_t size)
{
    uint8_t digest[BC_CORE_SHA256_DIGEST_SIZE];
    bc_core_sha256(data, size, digest);

    bc_core_sha256_context_t ctx;
    bc_core_sha256_init(&ctx);
    for (size_t offset = 0; offset < size; offset += 17) {
        size_t step = (offset + 17 <= size) ? 17 : size - offset;
        bc_core_sha256_update(&ctx, data + offset, step);
    }
    bc_core_sha256_finalize(&ctx, digest);
}

int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    fuzz_crc32c(data, size);
    fuzz_sha256(data, size);
    return 0;
}

#ifndef BC_FUZZ_LIBFUZZER
int main(int argc, char** argv)
{
    if (argc < 2) {
        fprintf(stderr, "usage: %s <iterations> [seed]\n", argv[0]);
        return 2;
    }
    unsigned long iterations = strtoul(argv[1], NULL, 10);
    unsigned long seed = (argc >= 3) ? strtoul(argv[2], NULL, 10) : 0;
    srand((unsigned int)seed);

    uint8_t buffer[8192];
    for (unsigned long i = 0; i < iterations; i++) {
        size_t len = (size_t)(rand() % (int)sizeof(buffer));
        for (size_t j = 0; j < len; j++) {
            buffer[j] = (uint8_t)(rand() & 0xFF);
        }
        LLVMFuzzerTestOneInput(buffer, len);
    }
    return 0;
}
#endif
