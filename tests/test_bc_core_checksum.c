// SPDX-License-Identifier: MIT

#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <cmocka.h>

#include "bc_core.h"

#include <stdint.h>
#include <string.h>

/* ===== CRC32C ===== */

static void test_crc32c_empty_data_returns_zero(void** state)
{
    BC_UNUSED(state);

    uint32_t result = 0xDEADBEEFu;
    bool success = bc_core_crc32c("", 0, &result);

    assert_true(success);
    assert_int_equal(result, 0x00000000u);
}

static void test_crc32c_known_vector_123456789_returns_expected_hash(void** state)
{
    BC_UNUSED(state);

    const char* input = "123456789";
    uint32_t result = 0;
    bool success = bc_core_crc32c(input, 9, &result);

    assert_true(success);
    assert_int_equal(result, 0xE3069283u);
}

static void test_crc32c_single_byte_returns_true(void** state)
{
    BC_UNUSED(state);

    const uint8_t input[1] = {0x42};
    uint32_t result = 0;
    bool success = bc_core_crc32c(input, 1, &result);

    assert_true(success);
}

static void test_crc32c_seven_bytes_returns_true(void** state)
{
    BC_UNUSED(state);

    const uint8_t input[7] = {1, 2, 3, 4, 5, 6, 7};
    uint32_t result = 0;
    bool success = bc_core_crc32c(input, 7, &result);

    assert_true(success);
}

static void test_crc32c_eight_bytes_returns_true(void** state)
{
    BC_UNUSED(state);

    const uint8_t input[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    uint32_t result = 0;
    bool success = bc_core_crc32c(input, 8, &result);

    assert_true(success);
}

static void test_crc32c_fifteen_bytes_returns_true(void** state)
{
    BC_UNUSED(state);

    uint8_t input[15];
    bc_core_fill(input, sizeof(input), (unsigned char)0xAB);
    uint32_t result = 0;
    bool success = bc_core_crc32c(input, 15, &result);

    assert_true(success);
}

static void test_crc32c_sixteen_bytes_returns_true(void** state)
{
    BC_UNUSED(state);

    uint8_t input[16];
    bc_core_fill(input, sizeof(input), (unsigned char)0xCD);
    uint32_t result = 0;
    bool success = bc_core_crc32c(input, 16, &result);

    assert_true(success);
}

static void test_crc32c_thirtyone_bytes_returns_true(void** state)
{
    BC_UNUSED(state);

    uint8_t input[31];
    bc_core_fill(input, sizeof(input), (unsigned char)0x55);
    uint32_t result = 0;
    bool success = bc_core_crc32c(input, 31, &result);

    assert_true(success);
}

static void test_crc32c_thirtytwo_bytes_returns_true(void** state)
{
    BC_UNUSED(state);

    uint8_t input[32];
    bc_core_fill(input, sizeof(input), (unsigned char)0x77);
    uint32_t result = 0;
    bool success = bc_core_crc32c(input, 32, &result);

    assert_true(success);
}

static void test_crc32c_large_buffer_activates_pclmul_path(void** state)
{
    BC_UNUSED(state);

    uint8_t input[4096];
    bc_core_fill(input, sizeof(input), (unsigned char)0x5A);
    uint32_t result = 0;
    bool success = bc_core_crc32c(input, 4096, &result);

    assert_true(success);
}

static void test_crc32c_large_buffer_matches_incremental_update(void** state)
{
    BC_UNUSED(state);

    uint8_t input[4096];
    bc_core_fill(input, sizeof(input), (unsigned char)0x5A);

    uint32_t hash_direct = 0;
    bool success_direct = bc_core_crc32c(input, 4096, &hash_direct);

    uint32_t hash_update = 0;
    bool success_update = bc_core_crc32c_update(0, input, 4096, &hash_update);

    assert_true(success_direct);
    assert_true(success_update);
    assert_int_equal(hash_direct, hash_update);
}

/* ===== CRC32C update ===== */

static void test_crc32c_update_empty_data_returns_prev_unchanged(void** state)
{
    BC_UNUSED(state);

    uint32_t result = 0;
    bool success = bc_core_crc32c_update(0xCAFEBABEu, "", 0, &result);

    assert_true(success);
    assert_int_equal(result, 0xCAFEBABEu);
}

static void test_crc32c_update_incremental_matches_full_hash(void** state)
{
    BC_UNUSED(state);

    const char* part_a = "1234";
    const char* part_b = "56789";
    const char* full = "123456789";

    uint32_t hash_a = 0;
    bool success_a = bc_core_crc32c(part_a, 4, &hash_a);

    uint32_t hash_incremental = 0;
    bool success_incremental = bc_core_crc32c_update(hash_a, part_b, 5, &hash_incremental);

    uint32_t hash_full = 0;
    bool success_full = bc_core_crc32c(full, 9, &hash_full);

    assert_true(success_a);
    assert_true(success_incremental);
    assert_true(success_full);
    assert_int_equal(hash_incremental, hash_full);
}

static void test_crc32c_update_chained_zero_prev_matches_direct(void** state)
{
    BC_UNUSED(state);

    const uint8_t input[32] = {0};

    uint32_t hash_direct = 0;
    bool success_direct = bc_core_crc32c(input, 32, &hash_direct);

    uint32_t hash_update = 0;
    bool success_update = bc_core_crc32c_update(0, input, 32, &hash_update);

    assert_true(success_direct);
    assert_true(success_update);
    assert_int_equal(hash_direct, hash_update);
}

static bool sha256_is_available(void)
{
    uint8_t probe[BC_CORE_SHA256_DIGEST_SIZE];
    return bc_core_sha256("", 0, probe);
}

/* ===== SHA-256 one-shot ===== */

static void test_sha256_nist_empty_string_returns_known_digest(void** state)
{
    BC_UNUSED(state);

    if (!sha256_is_available())
        skip();

    const uint8_t expected[32] = {
        0xe3, 0xb0, 0xc4, 0x42, 0x98, 0xfc, 0x1c, 0x14, 0x9a, 0xfb, 0xf4, 0xc8, 0x99, 0x6f, 0xb9, 0x24,
        0x27, 0xae, 0x41, 0xe4, 0x64, 0x9b, 0x93, 0x4c, 0xa4, 0x95, 0x99, 0x1b, 0x78, 0x52, 0xb8, 0x55,
    };

    uint8_t digest[32] = {0};
    bool success = bc_core_sha256("", 0, digest);

    assert_true(success);
    assert_memory_equal(digest, expected, 32);
}

static void test_sha256_nist_abc_returns_known_digest(void** state)
{
    BC_UNUSED(state);

    if (!sha256_is_available())
        skip();

    const uint8_t expected[32] = {
        0xba, 0x78, 0x16, 0xbf, 0x8f, 0x01, 0xcf, 0xea, 0x41, 0x41, 0x40, 0xde, 0x5d, 0xae, 0x22, 0x23,
        0xb0, 0x03, 0x61, 0xa3, 0x96, 0x17, 0x7a, 0x9c, 0xb4, 0x10, 0xff, 0x61, 0xf2, 0x00, 0x15, 0xad,
    };

    uint8_t digest[32] = {0};
    bool success = bc_core_sha256("abc", 3, digest);

    assert_true(success);
    assert_memory_equal(digest, expected, 32);
}

static void test_sha256_nist_long_string_multiblock_returns_known_digest(void** state)
{
    BC_UNUSED(state);

    if (!sha256_is_available())
        skip();

    const char* input = "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq";
    const uint8_t expected[32] = {
        0x24, 0x8d, 0x6a, 0x61, 0xd2, 0x06, 0x38, 0xb8, 0xe5, 0xc0, 0x26, 0x93, 0x0c, 0x3e, 0x60, 0x39,
        0xa3, 0x3c, 0xe4, 0x59, 0x64, 0xff, 0x21, 0x67, 0xf6, 0xec, 0xed, 0xd4, 0x19, 0xdb, 0x06, 0xc1,
    };

    uint8_t digest[32] = {0};
    bool success = bc_core_sha256(input, strlen(input), digest);

    assert_true(success);
    assert_memory_equal(digest, expected, 32);
}

static void test_sha256_more_than_64_bytes_triggers_multiblock(void** state)
{
    BC_UNUSED(state);

    if (!sha256_is_available())
        skip();

    uint8_t input[128];
    bc_core_fill(input, sizeof(input), (unsigned char)0x61);

    uint8_t digest[32] = {0};
    bool success = bc_core_sha256(input, 128, digest);

    assert_true(success);
}

/* ===== SHA-256 init ===== */

static void test_sha256_init_sets_buffer_length_to_zero(void** state)
{
    BC_UNUSED(state);

    bc_core_sha256_context_t ctx;
    bool success = bc_core_sha256_init(&ctx);

    assert_true(success);
    assert_int_equal(ctx.buffer_length, 0);
}

static void test_sha256_init_sets_total_length_to_zero(void** state)
{
    BC_UNUSED(state);

    bc_core_sha256_context_t ctx;
    bool success = bc_core_sha256_init(&ctx);

    assert_true(success);
    assert_int_equal(ctx.total_length, 0);
}

/* ===== SHA-256 update ===== */

static void test_sha256_update_empty_data_returns_true_without_modifying_length(void** state)
{
    BC_UNUSED(state);

    bc_core_sha256_context_t ctx;
    bc_core_sha256_init(&ctx);

    bool success = bc_core_sha256_update(&ctx, "", 0);

    assert_true(success);
    assert_int_equal(ctx.total_length, 0);
    assert_int_equal(ctx.buffer_length, 0);
}

static void test_sha256_update_small_chunk_fills_buffer_partially(void** state)
{
    BC_UNUSED(state);

    if (!sha256_is_available())
        skip();

    bc_core_sha256_context_t ctx;
    bc_core_sha256_init(&ctx);

    const char* chunk = "hello";
    bool success = bc_core_sha256_update(&ctx, chunk, 5);

    assert_true(success);
    assert_int_equal(ctx.buffer_length, 5);
    assert_int_equal(ctx.total_length, 5);
}

static void test_sha256_update_chunk_larger_than_block_forces_block_processing(void** state)
{
    BC_UNUSED(state);

    if (!sha256_is_available())
        skip();

    bc_core_sha256_context_t ctx;
    bc_core_sha256_init(&ctx);

    uint8_t large_chunk[128];
    bc_core_fill(large_chunk, sizeof(large_chunk), (unsigned char)0x42);

    bool success = bc_core_sha256_update(&ctx, large_chunk, 128);

    assert_true(success);
    assert_int_equal(ctx.total_length, 128);
    assert_int_equal(ctx.buffer_length, 0);
}

static void test_sha256_update_filling_existing_partial_buffer_then_overflow(void** state)
{
    BC_UNUSED(state);

    if (!sha256_is_available())
        skip();

    bc_core_sha256_context_t ctx;
    bc_core_sha256_init(&ctx);

    uint8_t first_chunk[32];
    bc_core_fill(first_chunk, sizeof(first_chunk), (unsigned char)0x11);
    bc_core_sha256_update(&ctx, first_chunk, 32);

    uint8_t second_chunk[64];
    bc_core_fill(second_chunk, sizeof(second_chunk), (unsigned char)0x22);
    bool success = bc_core_sha256_update(&ctx, second_chunk, 64);

    assert_true(success);
    assert_int_equal(ctx.total_length, 96);
}

/* ===== SHA-256 finalize ===== */

static void test_sha256_finalize_after_init_update_abc_matches_oneshot(void** state)
{
    BC_UNUSED(state);

    if (!sha256_is_available())
        skip();

    uint8_t digest_incremental[32] = {0};
    bc_core_sha256_context_t ctx;
    bc_core_sha256_init(&ctx);
    bc_core_sha256_update(&ctx, "abc", 3);
    bool success_finalize = bc_core_sha256_finalize(&ctx, digest_incremental);

    uint8_t digest_oneshot[32] = {0};
    bool success_oneshot = bc_core_sha256("abc", 3, digest_oneshot);

    assert_true(success_finalize);
    assert_true(success_oneshot);
    assert_memory_equal(digest_incremental, digest_oneshot, 32);
}

static void test_sha256_finalize_nist_empty_matches_known_digest(void** state)
{
    BC_UNUSED(state);

    if (!sha256_is_available())
        skip();

    const uint8_t expected[32] = {
        0xe3, 0xb0, 0xc4, 0x42, 0x98, 0xfc, 0x1c, 0x14, 0x9a, 0xfb, 0xf4, 0xc8, 0x99, 0x6f, 0xb9, 0x24,
        0x27, 0xae, 0x41, 0xe4, 0x64, 0x9b, 0x93, 0x4c, 0xa4, 0x95, 0x99, 0x1b, 0x78, 0x52, 0xb8, 0x55,
    };

    uint8_t digest[32] = {0};
    bc_core_sha256_context_t ctx;
    bc_core_sha256_init(&ctx);
    bool success = bc_core_sha256_finalize(&ctx, digest);

    assert_true(success);
    assert_memory_equal(digest, expected, 32);
}

static void test_sha256_finalize_nist_abc_matches_known_digest(void** state)
{
    BC_UNUSED(state);

    if (!sha256_is_available())
        skip();

    const uint8_t expected[32] = {
        0xba, 0x78, 0x16, 0xbf, 0x8f, 0x01, 0xcf, 0xea, 0x41, 0x41, 0x40, 0xde, 0x5d, 0xae, 0x22, 0x23,
        0xb0, 0x03, 0x61, 0xa3, 0x96, 0x17, 0x7a, 0x9c, 0xb4, 0x10, 0xff, 0x61, 0xf2, 0x00, 0x15, 0xad,
    };

    uint8_t digest[32] = {0};
    bc_core_sha256_context_t ctx;
    bc_core_sha256_init(&ctx);
    bc_core_sha256_update(&ctx, "abc", 3);
    bool success = bc_core_sha256_finalize(&ctx, digest);

    assert_true(success);
    assert_memory_equal(digest, expected, 32);
}

static void test_sha256_finalize_multiupdate_single_chars_matches_oneshot(void** state)
{
    BC_UNUSED(state);

    if (!sha256_is_available())
        skip();

    uint8_t digest_incremental[32] = {0};
    bc_core_sha256_context_t ctx;
    bc_core_sha256_init(&ctx);
    bc_core_sha256_update(&ctx, "a", 1);
    bc_core_sha256_update(&ctx, "b", 1);
    bc_core_sha256_update(&ctx, "c", 1);
    bool success_finalize = bc_core_sha256_finalize(&ctx, digest_incremental);

    uint8_t digest_oneshot[32] = {0};
    bool success_oneshot = bc_core_sha256("abc", 3, digest_oneshot);

    assert_true(success_finalize);
    assert_true(success_oneshot);
    assert_memory_equal(digest_incremental, digest_oneshot, 32);
}

static void test_sha256_finalize_long_data_with_padding_overflow_matches_oneshot(void** state)
{
    BC_UNUSED(state);

    if (!sha256_is_available())
        skip();

    uint8_t input[57];
    bc_core_fill(input, sizeof(input), (unsigned char)0x41);

    uint8_t digest_incremental[32] = {0};
    bc_core_sha256_context_t ctx;
    bc_core_sha256_init(&ctx);
    bc_core_sha256_update(&ctx, input, 57);
    bool success_finalize = bc_core_sha256_finalize(&ctx, digest_incremental);

    uint8_t digest_oneshot[32] = {0};
    bool success_oneshot = bc_core_sha256(input, 57, digest_oneshot);

    assert_true(success_finalize);
    assert_true(success_oneshot);
    assert_memory_equal(digest_incremental, digest_oneshot, 32);
}

static void test_sha256_incremental_4x4kb_matches_oneshot(void** state)
{
    BC_UNUSED(state);

    if (!sha256_is_available())
        skip();

    uint8_t buf[4096 * 4];
    for (size_t i = 0; i < sizeof(buf); i++) {
        buf[i] = (unsigned char)(i & 0xff);
    }

    uint8_t digest_incremental[32] = {0};
    bc_core_sha256_context_t ctx;
    bc_core_sha256_init(&ctx);
    bc_core_sha256_update(&ctx, buf, 4096);
    bc_core_sha256_update(&ctx, buf + 4096, 4096);
    bc_core_sha256_update(&ctx, buf + 8192, 4096);
    bc_core_sha256_update(&ctx, buf + 12288, 4096);
    bool success_inc = bc_core_sha256_finalize(&ctx, digest_incremental);

    uint8_t digest_oneshot[32] = {0};
    bool success_one = bc_core_sha256(buf, sizeof(buf), digest_oneshot);

    assert_true(success_inc);
    assert_true(success_one);
    assert_memory_equal(digest_incremental, digest_oneshot, 32);
}

static void test_crc32c_exact_3072_bytes_pclmul_no_remainder(void** state)
{
    BC_UNUSED(state);

    unsigned char* buffer = malloc(3072);
    assert_non_null(buffer);
    bc_core_fill(buffer, 3072, (unsigned char)0xAB);

    uint32_t hash = 0;
    bool success = bc_core_crc32c(buffer, 3072, &hash);

    assert_true(success);
    assert_int_not_equal(hash, 0);

    uint32_t hash_update = 0;
    bool success_update = bc_core_crc32c_update(0, buffer, 3072, &hash_update);
    assert_true(success_update);
    assert_int_equal(hash, hash_update);

    free(buffer);
}

static void test_crc32c_exact_6144_bytes_pclmul_two_rounds_no_remainder(void** state)
{
    BC_UNUSED(state);

    unsigned char* buffer = malloc(6144);
    assert_non_null(buffer);
    bc_core_fill(buffer, 6144, (unsigned char)0xCD);

    uint32_t hash = 0;
    bool success = bc_core_crc32c(buffer, 6144, &hash);

    assert_true(success);
    assert_int_not_equal(hash, 0);

    free(buffer);
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_crc32c_empty_data_returns_zero),
        cmocka_unit_test(test_crc32c_known_vector_123456789_returns_expected_hash),
        cmocka_unit_test(test_crc32c_single_byte_returns_true),
        cmocka_unit_test(test_crc32c_seven_bytes_returns_true),
        cmocka_unit_test(test_crc32c_eight_bytes_returns_true),
        cmocka_unit_test(test_crc32c_fifteen_bytes_returns_true),
        cmocka_unit_test(test_crc32c_sixteen_bytes_returns_true),
        cmocka_unit_test(test_crc32c_thirtyone_bytes_returns_true),
        cmocka_unit_test(test_crc32c_thirtytwo_bytes_returns_true),
        cmocka_unit_test(test_crc32c_large_buffer_activates_pclmul_path),
        cmocka_unit_test(test_crc32c_large_buffer_matches_incremental_update),
        cmocka_unit_test(test_crc32c_update_empty_data_returns_prev_unchanged),
        cmocka_unit_test(test_crc32c_update_incremental_matches_full_hash),
        cmocka_unit_test(test_crc32c_update_chained_zero_prev_matches_direct),
        cmocka_unit_test(test_sha256_nist_empty_string_returns_known_digest),
        cmocka_unit_test(test_sha256_nist_abc_returns_known_digest),
        cmocka_unit_test(test_sha256_nist_long_string_multiblock_returns_known_digest),
        cmocka_unit_test(test_sha256_more_than_64_bytes_triggers_multiblock),
        cmocka_unit_test(test_sha256_init_sets_buffer_length_to_zero),
        cmocka_unit_test(test_sha256_init_sets_total_length_to_zero),
        cmocka_unit_test(test_sha256_update_empty_data_returns_true_without_modifying_length),
        cmocka_unit_test(test_sha256_update_small_chunk_fills_buffer_partially),
        cmocka_unit_test(test_sha256_update_chunk_larger_than_block_forces_block_processing),
        cmocka_unit_test(test_sha256_update_filling_existing_partial_buffer_then_overflow),
        cmocka_unit_test(test_sha256_finalize_after_init_update_abc_matches_oneshot),
        cmocka_unit_test(test_sha256_finalize_nist_empty_matches_known_digest),
        cmocka_unit_test(test_sha256_finalize_nist_abc_matches_known_digest),
        cmocka_unit_test(test_sha256_finalize_multiupdate_single_chars_matches_oneshot),
        cmocka_unit_test(test_sha256_finalize_long_data_with_padding_overflow_matches_oneshot),
        cmocka_unit_test(test_sha256_incremental_4x4kb_matches_oneshot),
        cmocka_unit_test(test_crc32c_exact_3072_bytes_pclmul_no_remainder),
        cmocka_unit_test(test_crc32c_exact_6144_bytes_pclmul_two_rounds_no_remainder),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
