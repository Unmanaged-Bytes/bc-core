# bc-core — project context

Core C primitives for the `bc-*` ecosystem: cache-line-aware memory
helpers, SIMD-accelerated byte search, integer-safe math with overflow
checks, and CPU-dispatched hash primitives (SHA-256 with SHA-NI fast
path, CRC32C via SSE4.2). The foundation every other `bc-*` library
builds on.

## Runtime dependencies

- C11 compiler (gcc or clang)
- `meson >= 1.0`, `ninja-build`, `pkg-config`
- `libcmocka-dev` (tests only)

No sibling dependencies — bc-core sits at the root of the `bc-*`
dependency graph.

## Source layout

| Path | Role |
|---|---|
| `include/public/bc_core.h` | Umbrella include |
| `include/public/bc_core_cpu.h` | `BC_UNUSED`, CPUID-detected feature flags |
| `include/public/bc_core_hash.h` | SHA-256 (SHA-NI), CRC32C |
| `include/public/bc_core_math.h` | Overflow-safe math |
| `include/public/bc_core_memory.h` | `bc_core_copy`, `bc_core_zero`, `bc_core_equal`, `bc_core_length`, `bc_core_find_byte` |
| `include/internal/` | Internal-only headers (not installed) |
| `src/cpu/`, `src/hash/`, `src/math/`, `src/memory/` | Implementation |
| `tests/` | cmocka unit tests |
| `benchmarks/` | Optional perf micro-benchmarks |
| `fuzzing/` | libFuzzer harnesses |
| `debian/` | Debian packaging, `.deb` built by CI on `v*` tag |
| `.github/workflows/` | `ci.yml` (tests on push/PR), `release.yml` (tag → `.deb`) |

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
