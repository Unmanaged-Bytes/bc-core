// SPDX-License-Identifier: MIT

#include "bc_core.h"

#include "bc_core_cpu_features_internal.h"

#include <stddef.h>
#include <stdint.h>

#if defined(__x86_64__) || defined(__i386__)
#include <immintrin.h>

static const uint32_t sha256_round_constants[64] = {
    0x428A2F98u, 0x71374491u, 0xB5C0FBCFu, 0xE9B5DBA5u, 0x3956C25Bu, 0x59F111F1u, 0x923F82A4u, 0xAB1C5ED5u, 0xD807AA98u, 0x12835B01u,
    0x243185BEu, 0x550C7DC3u, 0x72BE5D74u, 0x80DEB1FEu, 0x9BDC06A7u, 0xC19BF174u, 0xE49B69C1u, 0xEFBE4786u, 0x0FC19DC6u, 0x240CA1CCu,
    0x2DE92C6Fu, 0x4A7484AAu, 0x5CB0A9DCu, 0x76F988DAu, 0x983E5152u, 0xA831C66Du, 0xB00327C8u, 0xBF597FC7u, 0xC6E00BF3u, 0xD5A79147u,
    0x06CA6351u, 0x14292967u, 0x27B70A85u, 0x2E1B2138u, 0x4D2C6DFCu, 0x53380D13u, 0x650A7354u, 0x766A0ABBu, 0x81C2C92Eu, 0x92722C85u,
    0xA2BFE8A1u, 0xA81A664Bu, 0xC24B8B70u, 0xC76C51A3u, 0xD192E819u, 0xD6990624u, 0xF40E3585u, 0x106AA070u, 0x19A4C116u, 0x1E376C08u,
    0x2748774Cu, 0x34B0BCB5u, 0x391C0CB3u, 0x4ED8AA4Au, 0x5B9CCA4Fu, 0x682E6FF3u, 0x748F82EEu, 0x78A5636Fu, 0x84C87814u, 0x8CC70208u,
    0x90BEFFFAu, 0xA4506CEBu, 0xBEF9A3F7u, 0xC67178F2u,
};

__attribute__((target("sha,sse4.1,ssse3"))) static void bc_core_sha256_shani_block(uint32_t state[8], const uint8_t block[64])
{
    __m128i shuf_mask = _mm_set_epi64x(0x0C0D0E0F08090A0BLL, 0x0405060700010203LL);
    __m128i abef_save = _mm_loadu_si128((const __m128i*)state);
    __m128i cdgh_save = _mm_loadu_si128((const __m128i*)(state + 4));

    __m128i temp = _mm_shuffle_epi32(abef_save, 0xB1);
    cdgh_save = _mm_shuffle_epi32(cdgh_save, 0x1B);
    __m128i state_abef = _mm_alignr_epi8(temp, cdgh_save, 8);
    __m128i state_cdgh = _mm_blend_epi16(cdgh_save, temp, 0xF0);

    __m128i abef_orig = state_abef;
    __m128i cdgh_orig = state_cdgh;

    __m128i message0 = _mm_shuffle_epi8(_mm_loadu_si128((const __m128i*)(block)), shuf_mask);
    __m128i message1 = _mm_shuffle_epi8(_mm_loadu_si128((const __m128i*)(block + 16)), shuf_mask);
    __m128i message2 = _mm_shuffle_epi8(_mm_loadu_si128((const __m128i*)(block + 32)), shuf_mask);
    __m128i message3 = _mm_shuffle_epi8(_mm_loadu_si128((const __m128i*)(block + 48)), shuf_mask);

    __m128i msg_temp;

    msg_temp = _mm_add_epi32(message0, _mm_loadu_si128((const __m128i*)&sha256_round_constants[0]));
    state_cdgh = _mm_sha256rnds2_epu32(state_cdgh, state_abef, msg_temp);
    msg_temp = _mm_shuffle_epi32(msg_temp, 0x0E);
    state_abef = _mm_sha256rnds2_epu32(state_abef, state_cdgh, msg_temp);

    msg_temp = _mm_add_epi32(message1, _mm_loadu_si128((const __m128i*)&sha256_round_constants[4]));
    state_cdgh = _mm_sha256rnds2_epu32(state_cdgh, state_abef, msg_temp);
    msg_temp = _mm_shuffle_epi32(msg_temp, 0x0E);
    state_abef = _mm_sha256rnds2_epu32(state_abef, state_cdgh, msg_temp);
    message0 = _mm_sha256msg1_epu32(message0, message1);

    msg_temp = _mm_add_epi32(message2, _mm_loadu_si128((const __m128i*)&sha256_round_constants[8]));
    state_cdgh = _mm_sha256rnds2_epu32(state_cdgh, state_abef, msg_temp);
    msg_temp = _mm_shuffle_epi32(msg_temp, 0x0E);
    state_abef = _mm_sha256rnds2_epu32(state_abef, state_cdgh, msg_temp);
    message1 = _mm_sha256msg1_epu32(message1, message2);

    msg_temp = _mm_add_epi32(message3, _mm_loadu_si128((const __m128i*)&sha256_round_constants[12]));
    state_cdgh = _mm_sha256rnds2_epu32(state_cdgh, state_abef, msg_temp);
    msg_temp = _mm_shuffle_epi32(msg_temp, 0x0E);
    state_abef = _mm_sha256rnds2_epu32(state_abef, state_cdgh, msg_temp);
    message0 = _mm_add_epi32(message0, _mm_alignr_epi8(message3, message2, 4));
    message0 = _mm_sha256msg2_epu32(message0, message3);
    message2 = _mm_sha256msg1_epu32(message2, message3);

#define SHA256_ROUND4(r, ma, mb, mc, md)                                                                                                   \
    do {                                                                                                                                   \
        msg_temp = _mm_add_epi32(ma, _mm_loadu_si128((const __m128i*)&sha256_round_constants[r]));                                         \
        state_cdgh = _mm_sha256rnds2_epu32(state_cdgh, state_abef, msg_temp);                                                              \
        msg_temp = _mm_shuffle_epi32(msg_temp, 0x0E);                                                                                      \
        state_abef = _mm_sha256rnds2_epu32(state_abef, state_cdgh, msg_temp);                                                              \
        mb = _mm_add_epi32(mb, _mm_alignr_epi8(ma, md, 4));                                                                                \
        mb = _mm_sha256msg2_epu32(mb, ma);                                                                                                 \
        md = _mm_sha256msg1_epu32(md, ma);                                                                                                 \
    } while (0)

    SHA256_ROUND4(16, message0, message1, message2, message3);
    SHA256_ROUND4(20, message1, message2, message3, message0);
    SHA256_ROUND4(24, message2, message3, message0, message1);
    SHA256_ROUND4(28, message3, message0, message1, message2);
    SHA256_ROUND4(32, message0, message1, message2, message3);
    SHA256_ROUND4(36, message1, message2, message3, message0);
    SHA256_ROUND4(40, message2, message3, message0, message1);
    SHA256_ROUND4(44, message3, message0, message1, message2);
    SHA256_ROUND4(48, message0, message1, message2, message3);

#undef SHA256_ROUND4

    msg_temp = _mm_add_epi32(message1, _mm_loadu_si128((const __m128i*)&sha256_round_constants[52]));
    state_cdgh = _mm_sha256rnds2_epu32(state_cdgh, state_abef, msg_temp);
    msg_temp = _mm_shuffle_epi32(msg_temp, 0x0E);
    state_abef = _mm_sha256rnds2_epu32(state_abef, state_cdgh, msg_temp);
    message2 = _mm_add_epi32(message2, _mm_alignr_epi8(message1, message0, 4));
    message2 = _mm_sha256msg2_epu32(message2, message1);

    msg_temp = _mm_add_epi32(message2, _mm_loadu_si128((const __m128i*)&sha256_round_constants[56]));
    state_cdgh = _mm_sha256rnds2_epu32(state_cdgh, state_abef, msg_temp);
    msg_temp = _mm_shuffle_epi32(msg_temp, 0x0E);
    state_abef = _mm_sha256rnds2_epu32(state_abef, state_cdgh, msg_temp);
    message3 = _mm_add_epi32(message3, _mm_alignr_epi8(message2, message1, 4));
    message3 = _mm_sha256msg2_epu32(message3, message2);

    msg_temp = _mm_add_epi32(message3, _mm_loadu_si128((const __m128i*)&sha256_round_constants[60]));
    state_cdgh = _mm_sha256rnds2_epu32(state_cdgh, state_abef, msg_temp);
    msg_temp = _mm_shuffle_epi32(msg_temp, 0x0E);
    state_abef = _mm_sha256rnds2_epu32(state_abef, state_cdgh, msg_temp);

    state_abef = _mm_add_epi32(state_abef, abef_orig);
    state_cdgh = _mm_add_epi32(state_cdgh, cdgh_orig);

    temp = _mm_shuffle_epi32(state_abef, 0x1B);
    state_cdgh = _mm_shuffle_epi32(state_cdgh, 0xB1);
    state_abef = _mm_blend_epi16(temp, state_cdgh, 0xF0);
    state_cdgh = _mm_alignr_epi8(state_cdgh, temp, 8);

    _mm_storeu_si128((__m128i*)state, state_abef);
    _mm_storeu_si128((__m128i*)(state + 4), state_cdgh);
}

#endif /* x86 */

static const uint32_t sha256_initial_state[8] = {
    0x6A09E667u, 0xBB67AE85u, 0x3C6EF372u, 0xA54FF53Au, 0x510E527Fu, 0x9B05688Cu, 0x1F83D9ABu, 0x5BE0CD19u,
};

static bool g_sha256_available = false;

__attribute__((constructor)) void bc_core_sha256_dispatch_init(void)
{
    g_sha256_available = false;

    bc_core_cpu_features_t features;
    if (!bc_core_cpu_features_detect(&features)) {
        return;
    }

#if defined(__x86_64__) || defined(__i386__)
    if (features.has_sha) {
        g_sha256_available = true;
    }
#endif
}

static void bc_core_sha256_pad_and_finalize(uint32_t state[8], const uint8_t* remainder, size_t remainder_length, uint64_t total_length)
{
    uint8_t padding_block[64];
    bc_core_zero(padding_block, sizeof(padding_block));
    bc_core_copy(padding_block, remainder, remainder_length);
    padding_block[remainder_length] = 0x80;

#if defined(__x86_64__) || defined(__i386__)
    if (remainder_length >= 56) {
        bc_core_sha256_shani_block(state, padding_block);
        bc_core_zero(padding_block, sizeof(padding_block));
    }
#endif

    uint64_t total_bits = total_length * 8;
    padding_block[56] = (uint8_t)(total_bits >> 56);
    padding_block[57] = (uint8_t)(total_bits >> 48);
    padding_block[58] = (uint8_t)(total_bits >> 40);
    padding_block[59] = (uint8_t)(total_bits >> 32);
    padding_block[60] = (uint8_t)(total_bits >> 24);
    padding_block[61] = (uint8_t)(total_bits >> 16);
    padding_block[62] = (uint8_t)(total_bits >> 8);
    padding_block[63] = (uint8_t)(total_bits);
#if defined(__x86_64__) || defined(__i386__)
    bc_core_sha256_shani_block(state, padding_block);
#endif
}

static void bc_core_sha256_state_to_digest(const uint32_t state[8], uint8_t digest[32])
{
    for (int i = 0; i < 8; i++) {
        digest[i * 4 + 0] = (uint8_t)(state[i] >> 24);
        digest[i * 4 + 1] = (uint8_t)(state[i] >> 16);
        digest[i * 4 + 2] = (uint8_t)(state[i] >> 8);
        digest[i * 4 + 3] = (uint8_t)(state[i]);
    }
}

bool bc_core_sha256(const void* data, size_t len, uint8_t out_hash[BC_CORE_SHA256_DIGEST_SIZE])
{
    if (!g_sha256_available) {
        return false;
    }

#if defined(__x86_64__) || defined(__i386__)
    const uint8_t* bytes = (const uint8_t*)data;
    uint32_t state[8];
    bc_core_copy(state, sha256_initial_state, sizeof(state));

    size_t offset = 0;
    while (offset + 64 <= len) {
        bc_core_sha256_shani_block(state, bytes + offset);
        offset += 64;
    }

    bc_core_sha256_pad_and_finalize(state, bytes + offset, len - offset, len);
    bc_core_sha256_state_to_digest(state, out_hash);
    return true;
#else
    BC_UNUSED(data);
    BC_UNUSED(len);
    BC_UNUSED(out_hash);
    return false;
#endif
}

bool bc_core_sha256_init(bc_core_sha256_context_t* ctx)
{
    bc_core_copy(ctx->state, sha256_initial_state, sizeof(ctx->state));
    ctx->buffer_length = 0;
    ctx->total_length = 0;
    return true;
}

bool bc_core_sha256_update(bc_core_sha256_context_t* ctx, const void* data, size_t len)
{
    if (len == 0) {
        return true;
    }

    if (!g_sha256_available) {
        return false;
    }

#if defined(__x86_64__) || defined(__i386__)
    const uint8_t* bytes = (const uint8_t*)data;

    ctx->total_length += len;

    if (ctx->buffer_length > 0) {
        size_t needed = BC_CORE_SHA256_BLOCK_SIZE - ctx->buffer_length;
        if (len < needed) {
            bc_core_copy(ctx->buffer + ctx->buffer_length, bytes, len);
            ctx->buffer_length += len;
            return true;
        }
        bc_core_copy(ctx->buffer + ctx->buffer_length, bytes, needed);
        bc_core_sha256_shani_block(ctx->state, ctx->buffer);
        bytes += needed;
        len -= needed;
        ctx->buffer_length = 0;
    }

    while (len >= BC_CORE_SHA256_BLOCK_SIZE) {
        bc_core_sha256_shani_block(ctx->state, bytes);
        bytes += BC_CORE_SHA256_BLOCK_SIZE;
        len -= BC_CORE_SHA256_BLOCK_SIZE;
    }

    if (len > 0) {
        bc_core_copy(ctx->buffer, bytes, len);
        ctx->buffer_length = len;
    }

    return true;
#else
    BC_UNUSED(ctx);
    BC_UNUSED(data);
    return false;
#endif
}

bool bc_core_sha256_finalize(bc_core_sha256_context_t* ctx, uint8_t out_hash[BC_CORE_SHA256_DIGEST_SIZE])
{
    if (!g_sha256_available) {
        return false;
    }

#if defined(__x86_64__) || defined(__i386__)
    bc_core_sha256_pad_and_finalize(ctx->state, ctx->buffer, ctx->buffer_length, ctx->total_length);
    bc_core_sha256_state_to_digest(ctx->state, out_hash);
    return true;
#else
    BC_UNUSED(ctx);
    BC_UNUSED(out_hash);
    return false;
#endif
}
