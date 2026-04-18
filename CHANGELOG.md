# Changelog

All notable changes to bc-core are documented here.

## [1.0.0]

Initial public release.

### Added

- **Memory primitives** (`bc_core_memory.h`): cache-line-aware
  `bc_core_copy`, `bc_core_zero`, SIMD-accelerated `bc_core_equal` /
  `bc_core_compare` / `bc_core_find_byte` / `bc_core_find_last_byte`,
  NUL-bounded `bc_core_length`, `bc_core_starts_with`.
- **Hash primitives** (`bc_core_hash.h`): SHA-256 with SHA-NI
  hardware-accelerated fast path, CRC32C (Castagnoli) via SSE4.2
  `crc32` instructions. Runtime CPUID dispatch picks the fastest
  available path.
- **Math primitives** (`bc_core_math.h`): overflow-safe
  `bc_core_safe_add`, `bc_core_safe_multiply`, clamp helpers.
- **CPU helpers** (`bc_core_cpu.h`): `BC_UNUSED(x)` macro, CPUID
  feature-flag probes.

### Quality

- Unit tests under `tests/`, built with cmocka.
- Sanitizers (asan / tsan / ubsan / memcheck) pass.
- cppcheck clean on the project sources.
- MIT-licensed, static `.a` published as Debian `.deb` via GitHub
  Releases.

[1.0.0]: https://github.com/Unmanaged-Bytes/bc-core/releases/tag/v1.0.0
