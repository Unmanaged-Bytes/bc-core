// SPDX-License-Identifier: MIT

#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <cmocka.h>
#include "bc_core.h"
#include <sys/mman.h>
#include <unistd.h>

static size_t page_size;

static int setup(void** state)
{
    BC_UNUSED(state);
    page_size = (size_t)sysconf(_SC_PAGESIZE);
    return 0;
}

static void copy_at_page_boundary(size_t copy_len)
{
    unsigned char* two_pages = mmap(NULL, page_size * 2, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    assert_true(two_pages != MAP_FAILED);

    int protect_result = mprotect(two_pages + page_size, page_size, PROT_NONE);
    assert_int_equal(protect_result, 0);

    unsigned char* source = two_pages + page_size - copy_len;
    for (size_t i = 0; i < copy_len; i++) {
        source[i] = (unsigned char)(i ^ 0xA5);
    }

    unsigned char destination[256];
    bc_core_zero(destination, sizeof(destination));

    bool result = bc_core_copy(destination, source, copy_len);

    assert_true(result);
    for (size_t i = 0; i < copy_len; i++) {
        assert_int_equal(destination[i], (unsigned char)(i ^ 0xA5));
    }

    munmap(two_pages, page_size * 2);
}

static void test_copy_page_boundary_len_33(void** state)
{
    BC_UNUSED(state);
    copy_at_page_boundary(33);
}

static void test_copy_page_boundary_len_48(void** state)
{
    BC_UNUSED(state);
    copy_at_page_boundary(48);
}

static void test_copy_page_boundary_len_63(void** state)
{
    BC_UNUSED(state);
    copy_at_page_boundary(63);
}

static void test_copy_page_boundary_len_64(void** state)
{
    BC_UNUSED(state);
    copy_at_page_boundary(64);
}

static void test_copy_page_boundary_len_65(void** state)
{
    BC_UNUSED(state);
    copy_at_page_boundary(65);
}

static void test_copy_page_boundary_len_80(void** state)
{
    BC_UNUSED(state);
    copy_at_page_boundary(80);
}

static void test_copy_page_boundary_len_95(void** state)
{
    BC_UNUSED(state);
    copy_at_page_boundary(95);
}

static void test_copy_page_boundary_len_96(void** state)
{
    BC_UNUSED(state);
    copy_at_page_boundary(96);
}

static void test_copy_page_boundary_len_97(void** state)
{
    BC_UNUSED(state);
    copy_at_page_boundary(97);
}

static void test_copy_page_boundary_len_128(void** state)
{
    BC_UNUSED(state);
    copy_at_page_boundary(128);
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup(test_copy_page_boundary_len_33, setup), cmocka_unit_test_setup(test_copy_page_boundary_len_48, setup),
        cmocka_unit_test_setup(test_copy_page_boundary_len_63, setup), cmocka_unit_test_setup(test_copy_page_boundary_len_64, setup),
        cmocka_unit_test_setup(test_copy_page_boundary_len_65, setup), cmocka_unit_test_setup(test_copy_page_boundary_len_80, setup),
        cmocka_unit_test_setup(test_copy_page_boundary_len_95, setup), cmocka_unit_test_setup(test_copy_page_boundary_len_96, setup),
        cmocka_unit_test_setup(test_copy_page_boundary_len_97, setup), cmocka_unit_test_setup(test_copy_page_boundary_len_128, setup),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
