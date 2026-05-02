// SPDX-License-Identifier: MIT

#include "bc_core_glob.h"

#include <stdint.h>

static char bc_core_glob_to_lower(char value)
{
    if (value >= 'A' && value <= 'Z') {
        return (char)(value + 32);
    }
    return value;
}

static bool bc_core_glob_chars_equal(char left, char right, bool case_insensitive)
{
    if (case_insensitive) {
        return bc_core_glob_to_lower(left) == bc_core_glob_to_lower(right);
    }
    return left == right;
}

static bool bc_core_glob_parse_class(const char* class_text, size_t class_length, char target, bool case_insensitive,
                                     size_t* out_close_offset, bool* out_match)
{
    if (class_length == 0) {
        return false;
    }
    size_t index = 0;
    bool negate = false;
    if (class_text[index] == '!') {
        negate = true;
        index += 1;
    }
    bool matched = false;
    bool consumed_first = false;
    while (index < class_length) {
        if (class_text[index] == ']' && consumed_first) {
            *out_close_offset = index;
            *out_match = matched != negate;
            return true;
        }
        if (index + 2 < class_length && class_text[index + 1] == '-' && class_text[index + 2] != ']') {
            char range_low = class_text[index];
            char range_high = class_text[index + 2];
            if (case_insensitive) {
                range_low = bc_core_glob_to_lower(range_low);
                range_high = bc_core_glob_to_lower(range_high);
            }
            char target_normalized = case_insensitive ? bc_core_glob_to_lower(target) : target;
            if (target_normalized >= range_low && target_normalized <= range_high) {
                matched = true;
            }
            index += 3;
        } else {
            if (bc_core_glob_chars_equal(class_text[index], target, case_insensitive)) {
                matched = true;
            }
            index += 1;
        }
        consumed_first = true;
    }
    return false;
}

static bool bc_core_glob_match_recursive(const char* pattern, size_t pattern_length, size_t pattern_index, const char* text,
                                         size_t text_length, size_t text_index, const bc_core_glob_options_t* options)
{
    while (pattern_index < pattern_length) {
        const char pattern_char = pattern[pattern_index];

        if (pattern_char == '*') {
            const bool is_double =
                options->double_star_recursive && pattern_index + 1 < pattern_length && pattern[pattern_index + 1] == '*';
            const size_t star_width = is_double ? 2u : 1u;
            const size_t after_star = pattern_index + star_width;

            if (is_double && after_star < pattern_length && pattern[after_star] == '/') {
                if (bc_core_glob_match_recursive(pattern, pattern_length, after_star + 1u, text, text_length, text_index, options)) {
                    return true;
                }
            }

            if (bc_core_glob_match_recursive(pattern, pattern_length, after_star, text, text_length, text_index, options)) {
                return true;
            }

            size_t scan_index = text_index;
            while (scan_index < text_length) {
                if (!is_double && options->path_segments_aware && text[scan_index] == '/') {
                    return false;
                }
                scan_index += 1;
                if (bc_core_glob_match_recursive(pattern, pattern_length, after_star, text, text_length, scan_index, options)) {
                    return true;
                }
            }
            return false;
        }

        if (text_index >= text_length) {
            return false;
        }

        if (pattern_char == '\\' && options->escape_with_backslash && pattern_index + 1 < pattern_length) {
            if (!bc_core_glob_chars_equal(pattern[pattern_index + 1], text[text_index], options->case_insensitive)) {
                return false;
            }
            pattern_index += 2;
            text_index += 1;
            continue;
        }

        if (pattern_char == '?') {
            if (options->path_segments_aware && text[text_index] == '/') {
                return false;
            }
            pattern_index += 1;
            text_index += 1;
            continue;
        }

        if (pattern_char == '[') {
            size_t close_offset = 0;
            bool class_match = false;
            if (!bc_core_glob_parse_class(&pattern[pattern_index + 1], pattern_length - pattern_index - 1, text[text_index],
                                          options->case_insensitive, &close_offset, &class_match)) {
                return false;
            }
            if (!class_match) {
                return false;
            }
            pattern_index += close_offset + 2u;
            text_index += 1;
            continue;
        }

        if (!bc_core_glob_chars_equal(pattern_char, text[text_index], options->case_insensitive)) {
            return false;
        }
        pattern_index += 1;
        text_index += 1;
    }

    return text_index == text_length;
}

bool bc_core_glob_match(const char* pattern, size_t pattern_length, const char* text, size_t text_length, bool* out_matches)
{
    const bc_core_glob_options_t default_options = {false, false, false, true};
    return bc_core_glob_match_with_options(pattern, pattern_length, text, text_length, &default_options, out_matches);
}

bool bc_core_glob_match_path(const char* pattern, size_t pattern_length, const char* path, size_t path_length, bool* out_matches)
{
    const bc_core_glob_options_t path_options = {false, true, true, true};
    return bc_core_glob_match_with_options(pattern, pattern_length, path, path_length, &path_options, out_matches);
}

bool bc_core_glob_match_with_options(const char* pattern, size_t pattern_length, const char* text, size_t text_length,
                                     const bc_core_glob_options_t* options, bool* out_matches)
{
    *out_matches = bc_core_glob_match_recursive(pattern, pattern_length, 0u, text, text_length, 0u, options);
    return true;
}
