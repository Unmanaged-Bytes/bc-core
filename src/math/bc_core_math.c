// SPDX-License-Identifier: MIT

#include "bc_core.h"

extern inline bool bc_core_safe_multiply(size_t a, size_t b, size_t* out_result);
extern inline bool bc_core_safe_add(size_t a, size_t b, size_t* out_result);
extern inline bool bc_core_align_up(size_t value, size_t alignment, size_t* out_result);
