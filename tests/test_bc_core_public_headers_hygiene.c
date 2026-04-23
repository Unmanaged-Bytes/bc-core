// SPDX-License-Identifier: MIT

#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <cmocka.h>

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef BC_CORE_SOURCE_ROOT
#define BC_CORE_SOURCE_ROOT "."
#endif

static const char* const public_headers[] = {
    "include/public/bc_core.h",
    "include/public/bc_core_cpu.h",
    "include/public/bc_core_hash.h",
    "include/public/bc_core_math.h",
    "include/public/bc_core_memory.h",
    NULL,
};

static const char* const forbidden_tokens[] = {
    "immintrin",
    "x86intrin",
    "_mm_",
    "_mm256_",
    "_mm512_",
    "__m128",
    "__m256",
    "__m512",
    NULL,
};

static char* slurp_file(const char* path)
{
    FILE* file = fopen(path, "rb");
    if (file == NULL) {
        return NULL;
    }
    if (fseek(file, 0, SEEK_END) != 0) {
        fclose(file);
        return NULL;
    }
    long size = ftell(file);
    if (size < 0) {
        fclose(file);
        return NULL;
    }
    if (fseek(file, 0, SEEK_SET) != 0) {
        fclose(file);
        return NULL;
    }
    char* buffer = (char*)malloc((size_t)size + 1U);
    if (buffer == NULL) {
        fclose(file);
        return NULL;
    }
    size_t read_bytes = fread(buffer, 1U, (size_t)size, file);
    fclose(file);
    buffer[read_bytes] = '\0';
    return buffer;
}

static void test_public_headers_isa_neutral(void** state)
{
    (void)state;
    char path_buffer[1024];

    for (size_t header_index = 0U; public_headers[header_index] != NULL; ++header_index) {
        int written = snprintf(path_buffer, sizeof(path_buffer), "%s/%s",
                               BC_CORE_SOURCE_ROOT, public_headers[header_index]);
        assert_true(written > 0 && (size_t)written < sizeof(path_buffer));

        char* content = slurp_file(path_buffer);
        if (content == NULL) {
            fail_msg("cannot read public header %s", path_buffer);
        }

        for (size_t token_index = 0U; forbidden_tokens[token_index] != NULL; ++token_index) {
            const char* match = strstr(content, forbidden_tokens[token_index]);
            if (match != NULL) {
                fail_msg("public header %s contains forbidden token '%s' (first match offset=%ld)",
                         public_headers[header_index],
                         forbidden_tokens[token_index],
                         (long)(match - content));
            }
        }

        free(content);
    }
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_public_headers_isa_neutral),
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}
