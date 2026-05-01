# bc-core

[![ci](https://github.com/Unmanaged-Bytes/bc-core/actions/workflows/ci.yml/badge.svg)](https://github.com/Unmanaged-Bytes/bc-core/actions/workflows/ci.yml)
[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](LICENSE)
![Language: C11](https://img.shields.io/badge/language-C11-informational)
![Platform: Linux](https://img.shields.io/badge/platform-Linux-lightgrey)

Core C primitives for the `bc-*` ecosystem: cache-line-aware memory helpers, SIMD-accelerated byte search, integer-safe math with overflow checks, and CPU-dispatched hash primitives (SHA-256 with SHA-NI fast path, CRC32C via SSE4.2).

> **Scope.** Personal project, part of the `bc-*` ecosystem used by
> [`bc-hash`](https://github.com/Unmanaged-Bytes/bc-hash) and sibling
> libraries. Published here for transparency and reuse, not as a
> hardened product.
>
> **Support.** Issues and PRs are welcome but handled on a best-effort
> basis, whenever I have spare time — this is not a priority project
> and there is no SLA. Do not rely on a timely response.


## License

MIT — see [LICENSE](LICENSE).
