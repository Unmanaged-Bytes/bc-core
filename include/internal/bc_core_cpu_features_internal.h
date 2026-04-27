// SPDX-License-Identifier: MIT

#ifndef BC_CORE_CPU_FEATURES_INTERNAL_H
#define BC_CORE_CPU_FEATURES_INTERNAL_H

#include <stdbool.h>

typedef struct {
    bool has_sse2;
    bool has_avx2;
    bool has_avx512f;
    bool has_avx512bw;
    bool has_sse42;
    bool has_aes;
    bool has_pclmul;
    bool has_vpclmul;
    bool has_sha;
} bc_core_cpu_features_t;

bool bc_core_cpu_features_detect(bc_core_cpu_features_t* out_features);

#endif /* BC_CORE_CPU_FEATURES_INTERNAL_H */
