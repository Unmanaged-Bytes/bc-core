// SPDX-License-Identifier: MIT

#ifndef BC_CORE_HASH_H
#define BC_CORE_HASH_H

#include "bc_core_cpu.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define BC_CORE_SHA256_DIGEST_SIZE 32
#define BC_CORE_SHA256_BLOCK_SIZE 64

typedef struct {
    uint32_t state[8] BC_ALIGN(16);
    uint8_t buffer[BC_CORE_SHA256_BLOCK_SIZE];
    size_t buffer_length;
    uint64_t total_length;
} bc_core_sha256_context_t;

bool bc_core_crc32c(const void* data, size_t len, uint32_t* out_hash);
bool bc_core_crc32c_update(uint32_t prev, const void* data, size_t len, uint32_t* out_hash);
bool bc_core_sha256(const void* data, size_t len, uint8_t out_hash[BC_CORE_SHA256_DIGEST_SIZE]);
bool bc_core_sha256_init(bc_core_sha256_context_t* ctx);
bool bc_core_sha256_update(bc_core_sha256_context_t* ctx, const void* data, size_t len);
bool bc_core_sha256_finalize(bc_core_sha256_context_t* ctx, uint8_t out_hash[BC_CORE_SHA256_DIGEST_SIZE]);

#endif /* BC_CORE_HASH_H */
