// SPDX-License-Identifier: MIT

#include "bc_core.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if (size < 3) {
        return 0;
    }

    const size_t pattern_length_raw = (size_t)data[0];
    size_t pattern_length = pattern_length_raw % 32;
    if (pattern_length == 0) {
        pattern_length = 1;
    }
    if (pattern_length + 1 >= size) {
        pattern_length = size > 1 ? 1 : 0;
    }
    const uint8_t* pattern = data + 1;
    const uint8_t* haystack = data + 1 + pattern_length;
    const size_t haystack_length = size - 1 - pattern_length;

    size_t offset = 0;
    bc_core_find_pattern(haystack, haystack_length, pattern, pattern_length, &offset);

    size_t last_offset = 0;
    bc_core_find_last_byte(haystack, haystack_length, pattern[0], &last_offset);

    unsigned char targets[4] = {pattern[0], 0x0a, 0x00, 0xff};
    size_t any_offset = 0;
    bc_core_find_any_byte(haystack, haystack_length, targets, sizeof(targets), &any_offset);

    size_t line_count = 0;
    bc_core_count_lines(haystack, haystack_length, &line_count);

    size_t line_with_pattern_count = 0;
    bc_core_count_lines_with_pattern(haystack, haystack_length, pattern, pattern_length, &line_with_pattern_count);

    bool word_state = false;
    size_t word_count = 0;
    bc_core_count_words(haystack, haystack_length, &word_state, &word_count);

    bool starts = false;
    bc_core_starts_with(haystack, haystack_length, pattern, pattern_length, &starts);

    bool ends = false;
    bc_core_ends_with(haystack, haystack_length, pattern, pattern_length, &ends);

    bool contains = false;
    bc_core_contains_pattern(haystack, haystack_length, pattern, pattern_length, &contains);

    return 0;
}

#ifndef BC_FUZZ_LIBFUZZER
int main(int argc, char** argv)
{
    if (argc < 2) {
        fprintf(stderr, "usage: %s <iterations> [seed]\n", argv[0]);
        return 2;
    }
    const unsigned long iterations = strtoul(argv[1], NULL, 10);
    const unsigned long seed = (argc >= 3) ? strtoul(argv[2], NULL, 10) : 0;
    srand((unsigned int)seed);

    uint8_t buffer[8192];
    for (unsigned long i = 0; i < iterations; i++) {
        const size_t length = (size_t)(rand() % (int)sizeof(buffer));
        for (size_t j = 0; j < length; j++) {
            buffer[j] = (uint8_t)(rand() & 0xFF);
        }
        LLVMFuzzerTestOneInput(buffer, length);
    }
    return 0;
}
#endif
