// SPDX-License-Identifier: MIT

#ifndef BC_CORE_SORT_H
#define BC_CORE_SORT_H

#include <stdbool.h>
#include <stddef.h>

typedef bool (*bc_core_sort_less_than_function)(const void* left, const void* right, void* user_data);

bool bc_core_sort_with_compare(void* base, size_t count, size_t element_size, bc_core_sort_less_than_function less_than, void* user_data);

#endif /* BC_CORE_SORT_H */
