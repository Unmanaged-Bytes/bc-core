# bc-core — project context

Core C primitives for the `bc-*` ecosystem: cache-line-aware memory
helpers, SIMD-accelerated byte search, integer-safe math with overflow
checks, and CPU-dispatched hash primitives (SHA-256 with SHA-NI fast
path, CRC32C via SSE4.2). The foundation every other `bc-*` library
builds on.

## Invariants (do not break)

- **No comments in `.c` files** — code names itself. Public `.h`
  may carry one-line contracts if the signature is insufficient.
- **No defensive null-checks at function entry.** Return `false`
  on legitimate failure; never assert in production paths.
- **SPDX-License-Identifier: MIT** header on every `.c` and `.h`.
- **Strict C11** with `-Wall -Wextra -Wpedantic -Werror`.
- **Sanitizers (asan/tsan/ubsan/memcheck) stay green** in CI.
- **cppcheck stays clean**; never edit `cppcheck-suppressions.txt`
  to hide real findings.
- **CPUID dispatch is runtime, not compile-time** — binaries built
  on any x86_64-v2 host must still pick up SHA-NI / AVX2 / BMI2
  at runtime when available.
