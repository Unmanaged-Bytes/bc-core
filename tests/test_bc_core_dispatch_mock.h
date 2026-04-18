// SPDX-License-Identifier: MIT

#ifndef TEST_BC_CORE_DISPATCH_MOCK_H
#define TEST_BC_CORE_DISPATCH_MOCK_H

#include "bc_core_cpu_features_internal.h"

#include <stdbool.h>

static bc_core_cpu_features_t g_mock_features = {0};
static bool g_mock_detect_returns = true;

/* GCOVR_EXCL_START */
bool __wrap_bc_core_cpu_features_detect(bc_core_cpu_features_t* out_features)
{
    if (!g_mock_detect_returns) {
        return false;
    }
    *out_features = g_mock_features;
    return true;
}
/* GCOVR_EXCL_STOP */

#endif
