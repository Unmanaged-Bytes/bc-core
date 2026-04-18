// SPDX-License-Identifier: MIT

#include "bc_core.h"

#include "bc_core_cpu_features_internal.h"

#include <stddef.h>
#include <stdint.h>

#if defined(__x86_64__) || defined(__i386__)
#include <immintrin.h>
#include <wmmintrin.h>

#define BC_CRC32C_PCLMUL_K1 0x546EA6B0U
#define BC_CRC32C_PCLMUL_K2 0x68D4E827U
#define BC_CRC32C_PCLMUL_MU 0x11F91CAF6ULL
#define BC_CRC32C_PCLMUL_P_NORMAL 0x11EDC6F41ULL

#define BC_CRC32C_PCLMUL_THRESHOLD 3072U

__attribute__((target("sse4.2"))) static bool bc_core_crc32c_sse42(uint32_t init, const void* data, size_t len, uint32_t* out_hash)
{
    const unsigned char* bytes = (const unsigned char*)data;
    uint64_t crc = (uint64_t)(init ^ 0xFFFFFFFFu);
    size_t i = 0;

    for (; i + 32 <= len; i += 32) {
        uint64_t chunk0;
        uint64_t chunk1;
        uint64_t chunk2;
        uint64_t chunk3;
        bc_core_load_u64_unaligned(bytes + i, &chunk0);
        bc_core_load_u64_unaligned(bytes + i + 8, &chunk1);
        bc_core_load_u64_unaligned(bytes + i + 16, &chunk2);
        bc_core_load_u64_unaligned(bytes + i + 24, &chunk3);
        crc = _mm_crc32_u64((unsigned long long)crc, (unsigned long long)chunk0);
        crc = _mm_crc32_u64((unsigned long long)crc, (unsigned long long)chunk1);
        crc = _mm_crc32_u64((unsigned long long)crc, (unsigned long long)chunk2);
        crc = _mm_crc32_u64((unsigned long long)crc, (unsigned long long)chunk3);
    }

    for (; i + 8 <= len; i += 8) {
        uint64_t chunk;
        bc_core_load_u64_unaligned(bytes + i, &chunk);
        crc = _mm_crc32_u64((unsigned long long)crc, (unsigned long long)chunk);
    }

    for (; i < len; i++) {
        crc = (uint64_t)_mm_crc32_u8((uint32_t)crc, (unsigned char)bytes[i]);
    }

    *out_hash = (uint32_t)crc ^ 0xFFFFFFFFu;
    return true;
}

static inline uint32_t bc_core_bit_reverse32(uint32_t x)
{
    x = ((x & 0x55555555u) << 1) | ((x >> 1) & 0x55555555u);
    x = ((x & 0x33333333u) << 2) | ((x >> 2) & 0x33333333u);
    x = ((x & 0x0F0F0F0Fu) << 4) | ((x >> 4) & 0x0F0F0F0Fu);
    x = ((x & 0x00FF00FFu) << 8) | ((x >> 8) & 0x00FF00FFu);
    return (x << 16) | (x >> 16);
}

__attribute__((target("sse4.2,pclmul"))) static uint32_t bc_core_barrett_reduce(uint64_t a)
{
    __m128i a_vec = _mm_cvtsi64_si128((int64_t)a);
    __m128i mu = _mm_cvtsi64_si128((int64_t)BC_CRC32C_PCLMUL_MU);
    __m128i a_hi = _mm_srli_epi64(a_vec, 32);
    __m128i q2_128 = _mm_clmulepi64_si128(a_hi, mu, 0x00);
    __m128i q2 = _mm_srli_epi64(q2_128, 32);
    __m128i p_vec = _mm_cvtsi64_si128((int64_t)BC_CRC32C_PCLMUL_P_NORMAL);
    __m128i q3_128 = _mm_clmulepi64_si128(q2, p_vec, 0x00);
    uint32_t a_low = (uint32_t)_mm_cvtsi128_si64(a_vec);
    uint32_t q3_low = (uint32_t)_mm_cvtsi128_si64(q3_128);
    return a_low ^ q3_low;
}

__attribute__((target("sse4.2,pclmul"))) static bool bc_core_crc32c_pclmul(uint32_t init, const void* data, size_t len, uint32_t* out_hash)
{
    const unsigned char* bytes = (const unsigned char*)data;
    uint64_t accumulator0 = (uint64_t)(init ^ 0xFFFFFFFFu);
    uint64_t accumulator1 = 0;
    uint64_t accumulator2 = 0;
    size_t position = 0;
    uint32_t combined = (uint32_t)accumulator0;

    while (len >= 3072) {
        if (len >= 6144) {
            __builtin_prefetch(bytes + position + 3072, 0, 1);
            __builtin_prefetch(bytes + position + 3072 + 1024, 0, 1);
            __builtin_prefetch(bytes + position + 3072 + 2048, 0, 1);
        }
        for (size_t iteration = 0; iteration < 128; iteration += 4) {
            uint64_t c00, c01, c02, c03;
            uint64_t c10, c11, c12, c13;
            uint64_t c20, c21, c22, c23;
            size_t base = position + (iteration * 8);
            bc_core_load_u64_unaligned(bytes + base, &c00);
            bc_core_load_u64_unaligned(bytes + base + 1024, &c10);
            bc_core_load_u64_unaligned(bytes + base + 2048, &c20);
            bc_core_load_u64_unaligned(bytes + base + 8, &c01);
            bc_core_load_u64_unaligned(bytes + base + 1024 + 8, &c11);
            bc_core_load_u64_unaligned(bytes + base + 2048 + 8, &c21);
            bc_core_load_u64_unaligned(bytes + base + 16, &c02);
            bc_core_load_u64_unaligned(bytes + base + 1024 + 16, &c12);
            bc_core_load_u64_unaligned(bytes + base + 2048 + 16, &c22);
            bc_core_load_u64_unaligned(bytes + base + 24, &c03);
            bc_core_load_u64_unaligned(bytes + base + 1024 + 24, &c13);
            bc_core_load_u64_unaligned(bytes + base + 2048 + 24, &c23);
            accumulator0 = _mm_crc32_u64((unsigned long long)accumulator0, (unsigned long long)c00);
            accumulator1 = _mm_crc32_u64((unsigned long long)accumulator1, (unsigned long long)c10);
            accumulator2 = _mm_crc32_u64((unsigned long long)accumulator2, (unsigned long long)c20);
            accumulator0 = _mm_crc32_u64((unsigned long long)accumulator0, (unsigned long long)c01);
            accumulator1 = _mm_crc32_u64((unsigned long long)accumulator1, (unsigned long long)c11);
            accumulator2 = _mm_crc32_u64((unsigned long long)accumulator2, (unsigned long long)c21);
            accumulator0 = _mm_crc32_u64((unsigned long long)accumulator0, (unsigned long long)c02);
            accumulator1 = _mm_crc32_u64((unsigned long long)accumulator1, (unsigned long long)c12);
            accumulator2 = _mm_crc32_u64((unsigned long long)accumulator2, (unsigned long long)c22);
            accumulator0 = _mm_crc32_u64((unsigned long long)accumulator0, (unsigned long long)c03);
            accumulator1 = _mm_crc32_u64((unsigned long long)accumulator1, (unsigned long long)c13);
            accumulator2 = _mm_crc32_u64((unsigned long long)accumulator2, (unsigned long long)c23);
        }

        __m128i rev0 = _mm_cvtsi32_si128((int32_t)bc_core_bit_reverse32((uint32_t)accumulator0));
        __m128i rev1 = _mm_cvtsi32_si128((int32_t)bc_core_bit_reverse32((uint32_t)accumulator1));
        __m128i k1 = _mm_cvtsi32_si128((int32_t)BC_CRC32C_PCLMUL_K1);
        __m128i k2 = _mm_cvtsi32_si128((int32_t)BC_CRC32C_PCLMUL_K2);
        __m128i x0 = _mm_clmulepi64_si128(rev0, k1, 0x00);
        __m128i x1 = _mm_clmulepi64_si128(rev1, k2, 0x00);
        __m128i xor_result = _mm_xor_si128(x0, x1);
        uint64_t poly64 = (uint64_t)_mm_cvtsi128_si64(xor_result);
        uint32_t reduced = bc_core_barrett_reduce(poly64);
        combined = bc_core_bit_reverse32(reduced) ^ (uint32_t)accumulator2;

        accumulator0 = (uint64_t)combined;
        accumulator1 = 0;
        accumulator2 = 0;

        position += 3072;
        len -= 3072;
    }

    if (len == 0) {
        *out_hash = combined ^ 0xFFFFFFFFu;
        return true;
    }

    return bc_core_crc32c_sse42(combined ^ 0xFFFFFFFFu, bytes + position, len, out_hash);
}

#endif /* x86 */

typedef enum {
    BC_CORE_CRC32C_LEVEL_NONE = 0,
    BC_CORE_CRC32C_LEVEL_SSE42 = 1,
    BC_CORE_CRC32C_LEVEL_PCLMUL = 2,
} bc_core_crc32c_level_t;

static bc_core_crc32c_level_t g_crc32c_level = BC_CORE_CRC32C_LEVEL_NONE;

__attribute__((constructor)) void bc_core_crc32c_dispatch_init(void)
{
    g_crc32c_level = BC_CORE_CRC32C_LEVEL_NONE;

    bc_core_cpu_features_t features;
    if (!bc_core_cpu_features_detect(&features)) {
        return;
    }

#if defined(__x86_64__) || defined(__i386__)
    if (features.has_sse42 && features.has_pclmul) {
        g_crc32c_level = BC_CORE_CRC32C_LEVEL_PCLMUL;
    } else if (features.has_sse42) {
        g_crc32c_level = BC_CORE_CRC32C_LEVEL_SSE42;
    }
#endif
}

bool bc_core_crc32c(const void* data, size_t len, uint32_t* out_hash)
{
    if (len == 0) {
        *out_hash = 0;
        return true;
    }

#if defined(__x86_64__) || defined(__i386__)
    if (g_crc32c_level == BC_CORE_CRC32C_LEVEL_PCLMUL && len >= BC_CRC32C_PCLMUL_THRESHOLD) {
        return bc_core_crc32c_pclmul(0, data, len, out_hash);
    }
    if (g_crc32c_level >= BC_CORE_CRC32C_LEVEL_SSE42) {
        return bc_core_crc32c_sse42(0, data, len, out_hash);
    }
#endif

    return false;
}

bool bc_core_crc32c_update(uint32_t prev, const void* data, size_t len, uint32_t* out_hash)
{
    if (len == 0) {
        *out_hash = prev;
        return true;
    }

#if defined(__x86_64__) || defined(__i386__)
    if (g_crc32c_level == BC_CORE_CRC32C_LEVEL_PCLMUL && len >= BC_CRC32C_PCLMUL_THRESHOLD) {
        return bc_core_crc32c_pclmul(prev, data, len, out_hash);
    }
    if (g_crc32c_level >= BC_CORE_CRC32C_LEVEL_SSE42) {
        return bc_core_crc32c_sse42(prev, data, len, out_hash);
    }
#endif

    return false;
}
