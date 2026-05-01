// SPDX-License-Identifier: MIT

// Test-only macros for ld --wrap= mocks.  Two recurring patterns are
// extracted so each wrapped symbol is one line of declaration instead
// of an 8-12 line boilerplate block:
//
//   BC_TEST_WRAP_FAIL_ON_CALL  — mock that records call counts and can
//                                fail on the Nth call, otherwise forwards.
//   BC_TEST_WRAP_SHOULD_FAIL   — mock with a single bool flag, fails when
//                                set, otherwise forwards.
//
// Both expand to:
//   - static counters / flags named after the symbol,
//   - an extern declaration of __real_<sym>,
//   - the __wrap_<sym> implementation forwarding to __real_<sym>.
//
// Argument lists are passed twice as parenthesised tuples: once for the
// declaration (with types) and once for the call (without types).

#ifndef BC_CORE_TEST_WRAP_H
#define BC_CORE_TEST_WRAP_H

#include <stdbool.h>

#define BC_TEST_WRAP_FAIL_ON_CALL(sym, ret_type, fail_value, decl_args, call_args)                                                         \
    static int sym##_call_count = 0;                                                                                                       \
    static int sym##_fail_on_call = 0;                                                                                                     \
    static int sym##_fail_from_call = 0;                                                                                                   \
    extern ret_type __real_##sym decl_args;                                                                                                \
    ret_type __wrap_##sym decl_args                                                                                                        \
    {                                                                                                                                      \
        sym##_call_count++;                                                                                                                \
        if (sym##_fail_on_call > 0 && sym##_call_count == sym##_fail_on_call) {                                                            \
            return fail_value;                                                                                                             \
        }                                                                                                                                  \
        if (sym##_fail_from_call > 0 && sym##_call_count >= sym##_fail_from_call) {                                                        \
            return fail_value;                                                                                                             \
        }                                                                                                                                  \
        return __real_##sym call_args;                                                                                                     \
    }

#define BC_TEST_WRAP_RESET_FAIL_ON_CALL(sym)                                                                                               \
    do {                                                                                                                                   \
        sym##_call_count = 0;                                                                                                              \
        sym##_fail_on_call = 0;                                                                                                            \
        sym##_fail_from_call = 0;                                                                                                          \
    } while (0)

#define BC_TEST_WRAP_SHOULD_FAIL(sym, ret_type, fail_value, decl_args, call_args)                                                          \
    static bool mock_##sym##_should_fail = false;                                                                                          \
    extern ret_type __real_##sym decl_args;                                                                                                \
    ret_type __wrap_##sym decl_args                                                                                                        \
    {                                                                                                                                      \
        if (mock_##sym##_should_fail) {                                                                                                    \
            return fail_value;                                                                                                             \
        }                                                                                                                                  \
        return __real_##sym call_args;                                                                                                     \
    }

#define BC_TEST_WRAP_RESET_SHOULD_FAIL(sym)                                                                                                \
    do {                                                                                                                                   \
        mock_##sym##_should_fail = false;                                                                                                  \
    } while (0)

#endif
