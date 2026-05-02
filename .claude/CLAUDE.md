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
- **x86-64-v3 baseline assumed** — AVX2 + BMI2 + FMA + LZCNT + MOVBE
  are required at build time AND at runtime. Release builds pass
  `-march=x86-64-v3`; binaries produced by bc-core (and transitively
  any downstream bc-* consumer built in release) will SIGILL on a
  host below x86-64-v3. SHA-NI remains runtime-dispatched via CPUID
  (hardware-optional feature, not part of v3). No scalar fallback
  is maintained for AVX2/BMI2 primitives — the expert target is
  post-2013 x86 only.
- **Public headers are ISA-neutral** — no `<immintrin.h>`,
  `<x86intrin.h>`, or `_mm_*` / `__m128/256/512` tokens in
  `include/public/*.h`. Consumers must be able to preprocess bc-core
  headers without a vector-ISA include. SIMD intrinsics stay in
  `.c` files only. Enforced by
  `tests/test_bc_core_public_headers_hygiene.c`.

## Position dans le DAG (D063)

bc-core est **la racine** du DAG. Aucune dépendance externe sauf libc.
Les wrappers `bc_core_*` autorisent `<string.h>` UNIQUEMENT en interne
ici (`src/memory/*.c` qui implémentent equal/length/copy/zero/fill/find
via SIMD ou `__builtin_*`).

Tout downstream bc-* (libs et binaires) doit consommer ces primitives
au lieu de `<string.h>` directement.

Voir `~/workspace/applications/CLAUDE.md` § 7 pour la doctrine D063
complète.
