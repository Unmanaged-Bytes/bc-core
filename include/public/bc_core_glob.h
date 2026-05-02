// SPDX-License-Identifier: MIT

#ifndef BC_CORE_GLOB_H
#define BC_CORE_GLOB_H

#include <stdbool.h>
#include <stddef.h>

typedef struct {
    bool case_insensitive;
    bool path_segments_aware;
    bool double_star_recursive;
    bool escape_with_backslash;
} bc_core_glob_options_t;

bool bc_core_glob_match(const char* pattern, size_t pattern_length, const char* text, size_t text_length, bool* out_matches);

bool bc_core_glob_match_path(const char* pattern, size_t pattern_length, const char* path, size_t path_length, bool* out_matches);

bool bc_core_glob_match_with_options(const char* pattern, size_t pattern_length, const char* text, size_t text_length,
                                     const bc_core_glob_options_t* options, bool* out_matches);

#endif /* BC_CORE_GLOB_H */
