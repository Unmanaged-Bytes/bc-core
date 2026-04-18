// SPDX-License-Identifier: MIT

#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <cmocka.h>

#include "bc_core.h"
#include "bc_core_cpu_features_internal.h"

#include <stdint.h>

static bc_core_cpu_features_t g_mock_features = {0};
static bool g_mock_detect_returns = true;

bool __wrap_bc_core_cpu_features_detect(bc_core_cpu_features_t* out_features)
{
    if (!g_mock_detect_returns) {
        return false;
    }
    *out_features = g_mock_features;
    return true;
}

extern void bc_core_crc32c_dispatch_init(void);
extern void bc_core_sha256_dispatch_init(void);
static void bc_core_checksum_dispatch_init(void)
{
    bc_core_crc32c_dispatch_init();
    bc_core_sha256_dispatch_init();
}

static void test_crc32c_no_sse42_returns_false(void** state)
{
    BC_UNUSED(state);

    g_mock_detect_returns = true;
    bc_core_zero(&g_mock_features, sizeof(g_mock_features));
    bc_core_checksum_dispatch_init();

    uint32_t hash = 0;
    bool success = bc_core_crc32c("test", 4, &hash);

    assert_false(success);
}

static void test_crc32c_update_no_sse42_returns_false(void** state)
{
    BC_UNUSED(state);

    g_mock_detect_returns = true;
    bc_core_zero(&g_mock_features, sizeof(g_mock_features));
    bc_core_checksum_dispatch_init();

    uint32_t hash = 0;
    bool success = bc_core_crc32c_update(0, "test", 4, &hash);

    assert_false(success);
}

static void test_crc32c_detect_failure_returns_false(void** state)
{
    BC_UNUSED(state);

    g_mock_detect_returns = false;
    bc_core_checksum_dispatch_init();

    uint32_t hash = 0;
    bool success = bc_core_crc32c("test", 4, &hash);

    assert_false(success);
}

static void test_crc32c_update_detect_failure_returns_false(void** state)
{
    BC_UNUSED(state);

    g_mock_detect_returns = false;
    bc_core_checksum_dispatch_init();

    uint32_t hash = 0;
    bool success = bc_core_crc32c_update(0, "test", 4, &hash);

    assert_false(success);
}

static void test_sha256_no_sha_feature_returns_false(void** state)
{
    BC_UNUSED(state);

    g_mock_detect_returns = true;
    bc_core_zero(&g_mock_features, sizeof(g_mock_features));
    bc_core_checksum_dispatch_init();

    uint8_t digest[32] = {0};
    bool success = bc_core_sha256("abc", 3, digest);

    assert_false(success);
}

static void test_sha256_detect_failure_returns_false(void** state)
{
    BC_UNUSED(state);

    g_mock_detect_returns = false;
    bc_core_checksum_dispatch_init();

    uint8_t digest[32] = {0};
    bool success = bc_core_sha256("abc", 3, digest);

    assert_false(success);
}

static void test_sha256_update_no_sha_feature_returns_false(void** state)
{
    BC_UNUSED(state);

    g_mock_detect_returns = true;
    bc_core_zero(&g_mock_features, sizeof(g_mock_features));
    g_mock_features.has_sha = true;
    bc_core_checksum_dispatch_init();

    bc_core_sha256_context_t ctx;
    bc_core_sha256_init(&ctx);

    g_mock_features.has_sha = false;
    bc_core_checksum_dispatch_init();

    bool success = bc_core_sha256_update(&ctx, "abc", 3);

    assert_false(success);
}

static void test_sha256_update_detect_failure_returns_false(void** state)
{
    BC_UNUSED(state);

    g_mock_detect_returns = true;
    g_mock_features.has_sha = true;
    bc_core_checksum_dispatch_init();

    bc_core_sha256_context_t ctx;
    bc_core_sha256_init(&ctx);

    g_mock_detect_returns = false;
    bc_core_checksum_dispatch_init();

    bool success = bc_core_sha256_update(&ctx, "abc", 3);

    assert_false(success);
}

static void test_sha256_finalize_no_sha_feature_returns_false(void** state)
{
    BC_UNUSED(state);

    g_mock_detect_returns = true;
    bc_core_zero(&g_mock_features, sizeof(g_mock_features));
    bc_core_checksum_dispatch_init();

    bc_core_sha256_context_t ctx;
    bc_core_sha256_init(&ctx);

    uint8_t digest[32] = {0};
    bool success = bc_core_sha256_finalize(&ctx, digest);

    assert_false(success);
}

static void test_sha256_finalize_detect_failure_returns_false(void** state)
{
    BC_UNUSED(state);

    g_mock_detect_returns = false;
    bc_core_checksum_dispatch_init();

    bc_core_sha256_context_t ctx;
    bc_core_sha256_init(&ctx);

    uint8_t digest[32] = {0};
    bool success = bc_core_sha256_finalize(&ctx, digest);

    assert_false(success);
}

static void test_crc32c_sse42_only_no_pclmul_returns_true(void** state)
{
    BC_UNUSED(state);

    g_mock_detect_returns = true;
    bc_core_zero(&g_mock_features, sizeof(g_mock_features));
    g_mock_features.has_sse42 = true;
    bc_core_checksum_dispatch_init();

    uint32_t hash = 0;
    bool success = bc_core_crc32c("123456789", 9, &hash);

    assert_true(success);
    assert_int_equal(hash, 0xE3069283u);
}

static void test_crc32c_update_sse42_only_no_pclmul_returns_true(void** state)
{
    BC_UNUSED(state);

    g_mock_detect_returns = true;
    bc_core_zero(&g_mock_features, sizeof(g_mock_features));
    g_mock_features.has_sse42 = true;
    bc_core_checksum_dispatch_init();

    uint32_t hash_a = 0;
    bc_core_crc32c_update(0, "1234", 4, &hash_a);

    uint32_t hash_ab = 0;
    bool success = bc_core_crc32c_update(hash_a, "56789", 5, &hash_ab);

    assert_true(success);
    assert_int_equal(hash_ab, 0xE3069283u);
}

static void test_crc32c_sse42_only_large_buffer_no_pclmul(void** state)
{
    BC_UNUSED(state);

    g_mock_detect_returns = true;
    bc_core_zero(&g_mock_features, sizeof(g_mock_features));
    g_mock_features.has_sse42 = true;
    bc_core_checksum_dispatch_init();

    unsigned char buffer[4096];
    bc_core_fill(buffer, sizeof(buffer), (unsigned char)0xAA);

    uint32_t hash = 0;
    bool success = bc_core_crc32c(buffer, sizeof(buffer), &hash);

    assert_true(success);
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_crc32c_no_sse42_returns_false),
        cmocka_unit_test(test_crc32c_update_no_sse42_returns_false),
        cmocka_unit_test(test_crc32c_detect_failure_returns_false),
        cmocka_unit_test(test_crc32c_update_detect_failure_returns_false),
        cmocka_unit_test(test_sha256_no_sha_feature_returns_false),
        cmocka_unit_test(test_sha256_detect_failure_returns_false),
        cmocka_unit_test(test_sha256_update_no_sha_feature_returns_false),
        cmocka_unit_test(test_sha256_update_detect_failure_returns_false),
        cmocka_unit_test(test_sha256_finalize_no_sha_feature_returns_false),
        cmocka_unit_test(test_sha256_finalize_detect_failure_returns_false),
        cmocka_unit_test(test_crc32c_sse42_only_no_pclmul_returns_true),
        cmocka_unit_test(test_crc32c_update_sse42_only_no_pclmul_returns_true),
        cmocka_unit_test(test_crc32c_sse42_only_large_buffer_no_pclmul),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
