# bc-core

[![ci](https://github.com/Unmanaged-Bytes/bc-core/actions/workflows/ci.yml/badge.svg)](https://github.com/Unmanaged-Bytes/bc-core/actions/workflows/ci.yml)
[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](LICENSE)
![Language: C11](https://img.shields.io/badge/language-C11-informational)
![Platform: Linux](https://img.shields.io/badge/platform-Linux-lightgrey)

> **Scope.** Personal project, part of the `bc-*` ecosystem used by
> [`bc-hash`](https://github.com/Unmanaged-Bytes/bc-hash) and sibling
> libraries. Published here for transparency and reuse, not as a
> hardened product.
>
> **Support.** Issues and PRs are welcome but handled on a best-effort
> basis, whenever I have spare time — this is not a priority project
> and there is no SLA. Do not rely on a timely response.


## Requirements

- Debian 13 (trixie) or any Linux distro with glibc ≥ 2.38
- `meson >= 1.0`, `ninja-build`, `pkg-config`
- `libcmocka-dev` (tests, optional for end users)

## Install (Debian 13 trixie — production)

Download the latest `.deb` from the [GitHub Releases page](https://github.com/Unmanaged-Bytes/bc-core/releases), then:

```bash
sudo apt install ./libbc-core-dev_X.Y.Z-1_amd64.deb
pkg-config --cflags --libs bc-core
```

The package installs:
- Headers under `/usr/include/bc/` (`bc_core.h`, `bc_core_cpu.h`,
  `bc_core_hash.h`, `bc_core_math.h`, `bc_core_memory.h`)
- Static library at `/usr/lib/x86_64-linux-gnu/libbc-core.a`
- pkg-config descriptor at `/usr/lib/x86_64-linux-gnu/pkgconfig/bc-core.pc`

## License

MIT — see [LICENSE](LICENSE).
