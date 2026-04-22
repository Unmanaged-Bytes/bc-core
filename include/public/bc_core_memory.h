// SPDX-License-Identifier: MIT

#ifndef BC_CORE_MEMORY_H
#define BC_CORE_MEMORY_H

#include "bc_core_cpu.h"

#include <stdbool.h>
#include <stddef.h>

bool bc_core_zero(void* dst, size_t len);
bool bc_core_fill(void* dst, size_t len, unsigned char value);
bool bc_core_zero_secure(volatile void* dst, size_t len);
bool bc_core_copy(void* dst, const void* src, size_t len);
bool bc_core_move(void* dst, const void* src, size_t len);
bool bc_core_equal(const void* a, const void* b, size_t len, bool* out_equal);
bool bc_core_compare(const void* a, const void* b, size_t len, int* out_result);
bool bc_core_swap(void* a, void* b, size_t len);

bool bc_core_find_byte(const void* data, size_t len, unsigned char target, size_t* out_offset);
bool bc_core_find_pattern(const void* data, size_t len, const void* pattern, size_t pattern_len, size_t* out_offset);
bool bc_core_find_last_byte(const void* data, size_t len, unsigned char target, size_t* out_offset);
bool bc_core_find_any_byte(const void* data, size_t len, const unsigned char* targets, size_t target_count, size_t* out_offset);
bool bc_core_length(const void* data, unsigned char terminator, size_t* out_length);

bool bc_core_count_byte(const void* data, size_t len, unsigned char target, size_t* out_count);
bool bc_core_count_matching(const void* data, size_t len, const unsigned char table[256], size_t* out_count);
bool bc_core_count_lines(const void* data, size_t len, size_t* out_count);
bool bc_core_count_lines_with_pattern(const void* data, size_t len, const void* pattern, size_t pattern_len, size_t* out_count);
bool bc_core_count_words(const void* data, size_t len, bool* in_word_state, size_t* out_count);
bool bc_core_count_words_ascii(const void* data, size_t len, bool* in_word_state, size_t* out_count);

bool bc_core_ascii_lowercase(void* data, size_t len);

void bc_core_contains_byte(const void* data, size_t len, unsigned char target, bool* out_found);
void bc_core_contains_pattern(const void* data, size_t len, const void* pattern, size_t pattern_len, bool* out_found);
void bc_core_starts_with(const void* data, size_t len, const void* prefix, size_t prefix_len, bool* out_result);
void bc_core_ends_with(const void* data, size_t len, const void* suffix, size_t suffix_len, bool* out_result);

bool bc_core_copy_with_policy(void* dst, const void* src, size_t len, bc_core_cache_policy_t policy);
bool bc_core_fill_with_policy(void* dst, size_t len, unsigned char value, bc_core_cache_policy_t policy);
bool bc_core_zero_with_policy(void* dst, size_t len, bc_core_cache_policy_t policy);

#endif /* BC_CORE_MEMORY_H */
