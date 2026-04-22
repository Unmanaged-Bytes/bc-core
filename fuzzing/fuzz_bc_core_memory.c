// SPDX-License-Identifier: MIT

#include "bc_core.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if (size < 2) {
        return 0;
    }

    size_t half = size / 2;
    const uint8_t* left = data;
    const uint8_t* right = data + half;
    size_t len = size - half;
    if (len > half) {
        len = half;
    }

    uint8_t scratch[4096];
    if (len > sizeof(scratch)) {
        len = sizeof(scratch);
    }

    bc_core_copy(scratch, left, len);
    bool equal = false;
    bc_core_equal(scratch, left, len, &equal);
    int cmp = 0;
    bc_core_compare(left, right, len, &cmp);

    bool found = false;
    bc_core_contains_byte(left, len, (unsigned char)data[0], &found);

    size_t offset = 0;
    bc_core_find_byte(left, len, (unsigned char)data[0], &offset);

    size_t count = 0;
    bc_core_count_byte(left, len, (unsigned char)data[0], &count);

    bc_core_zero(scratch, len);
    bc_core_fill(scratch, len, (unsigned char)data[0]);
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
