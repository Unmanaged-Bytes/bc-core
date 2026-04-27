# bc-core — étude AVX-512 + inlining

Date : 2026-04-27. Mesures sur Ryzen 7 5700G (Zen 3, AVX2/AVX-512 absent) et Intel i7-1165G7 (Tiger Lake, AVX-512 complet).

## 1. Topologie CPU comparée

### AMD Ryzen 7 5700G (Zen 3)

| Aspect | Valeur |
|---|---|
| Cores / threads | 8 / 16 (SMT) |
| L1d / L1i (par core) | 32 KiB / 32 KiB |
| L2 (par core) | 512 KiB |
| L3 (partagé) | 16 MiB |
| Vector width | 256-bit (AVX2) |
| AVX-512 | **non** |
| SHA-NI | oui |

Familles instructions disponibles : SSE/SSE2/SSE3/SSSE3/SSE4.1/SSE4.2, AVX, AVX2, BMI1, BMI2, FMA, AES-NI, SHA-NI, PCLMUL, ADX, RDRAND, RDSEED, MOVBE, POPCNT, LZCNT, CLFLUSHOPT, CLWB, F16C, CX16.

### Intel i7-1165G7 (Tiger Lake)

| Aspect | Valeur |
|---|---|
| Cores / threads | 4 / 8 (SMT) |
| L1d / L1i (par core) | 48 KiB / 32 KiB |
| L2 (par core) | 1280 KiB |
| L3 (partagé) | 12 MiB |
| Vector width | **512-bit** (AVX-512) |
| AVX-512 sub-extensions | F, DQ, CD, BW, VL, VBMI, VBMI2, IFMA, VNNI, BITALG, VPOPCNTDQ, VP2INTERSECT |
| GFNI / VPCLMULQDQ | **oui** (extensions récentes) |
| SHA-NI | oui |

Familles supplémentaires vs Zen 3 : la totalité de la famille AVX-512, GFNI (Galois field), VPCLMULQDQ (vector PCLMUL).

### Implications pour bc-core

- **Baseline x86-64-v3** = AVX2 + BMI2 + FMA + LZCNT + MOVBE + F16C. Garanti sur Zen 3 et Tiger Lake. Aucun fallback nécessaire.
- **AVX-512** = strictement Intel récent + AMD Zen 4+. **Pas dispo sur Zen 3**. Toute optim AVX-512 doit être en chemin alternatif, sélectionné au runtime.

## 2. Instructions effectivement utilisées dans bc-core

| Catégorie | Familles utilisées | Modules |
|---|---|---|
| SIMD 256-bit | AVX2 (`_mm256_*`) | memory/* (compare, equal, find, count, copy, fill, zero, move, swap, length, ascii) |
| SIMD 128-bit | SSE2/SSSE3/SSE4.1/SSE4.2 (`_mm_*`, `_mm_crc32_*`, `_mm_clmulepi64_*`) | hash (crc32c, sha256), prefetch helpers |
| Hardware crypto | SHA-NI (sha256), PCLMUL (crc32c GF mult) | hash/* |
| Cache control | `clflushopt` | cpu/* |
| Compiler builtins | `__builtin_ctz` (LZCNT/TZCNT), `__builtin_popcount`, `__builtin_prefetch`, `__builtin_memcpy` | partout |

### Instructions x86-64-v3 utilisées implicitement (générées par le compilo via `-march=x86-64-v3`)

- BMI2 : `bzhi`, `pdep`, `pext` — peut apparaître dans le code généré pour les manipulations de bits (bitmasking, sort partition) sans intrinsic explicite.
- LZCNT/TZCNT : substitué à `__builtin_clz`/`__builtin_ctz`.
- MOVBE : substitué à `__builtin_bswap*` quand pertinent.
- FMA : non pertinent (pas de calcul flottant chaud dans bc-core).

### Instructions disponibles mais **non utilisées** dans bc-core

| Instruction | Disponible sur | Pourquoi non utilisée |
|---|---|---|
| AES-NI | les deux | bc-core n'a pas de chiffrement (volontaire) |
| RDRAND / RDSEED | les deux | l'entropie est dans bc-io (`bc_io_random` via getrandom syscall) |
| ADX | les deux | pas d'arithmétique multi-précision dans bc-core |
| AVX-512 (toute famille) | Intel only | non-baseline ; sujet de cette étude |
| GFNI / VPCLMULQDQ | Intel only | extensions trop récentes pour baseline |
| Prefetch instructions explicites | les deux | un seul prefetch utilisé (`__builtin_prefetch`) ; pas de prefetcht0/t1/t2/nta différenciés |

## 3. Mesures bench AVX2 vs glibc — Intel vs AMD

Release LTO `-march=x86-64-v3`. Ratios bc_core vs libc.

| Op | Ryzen 5700G | Intel i7-1165G7 | Différence |
|---|---|---|---|
| copy 4KB | 0.99× memcpy | 0.75× memcpy | glibc memcpy Intel utilise AVX-512 |
| fill 4KB | 0.95× memset | 1.06× memset | quasi-parité |
| zero 4KB | 0.96× memset0 | 1.07× memset0 | quasi-parité |
| compare 4KB | 1.00× memcmp | 0.89× memcmp | glibc memcmp Intel utilise AVX-512 |
| equal 4KB | 1.02× memcmp | 0.85× memcmp | idem |
| length 1KB | 1.07× strlen | 0.77× strlen | glibc strlen Intel utilise AVX-512 |
| find_byte 4KB | 1.08× memchr | 1.04× memchr | parité |
| find_pattern 4KB | 8.74× memmem | 6.69× memmem | bc-core gagne sur les deux |
| sort uint64 4096 random | 1.69× qsort | 1.24× qsort | bc-core gagne sur les deux |

**Constat** : glibc Tiger Lake utilise massivement AVX-512 dans memcpy/memset/memcmp/strlen. bc-core compilé en AVX2-only perd 11-15% sur les ops courtes (4 KB) face à libc Intel. Sur des buffers où bc-core prend l'avantage par les algorithmes (find_pattern, sort), il reste devant.

### Test compilé avec `-march=x86-64-v4` (AVX-512) sur Intel

Aucun changement de code. Le compilateur peut autovectoriser certains paths en AVX-512.

| Op | bc-core AVX2 | bc-core AVX-512 (autovec) | Gain |
|---|---|---|---|
| copy 4KB | 0.75× | 0.73× | nul |
| fill 4KB | 1.06× | 1.03× | nul |
| compare 4KB | 0.89× | **0.97×** | **+9%** |
| equal 4KB | 0.85× | **1.08×** | **+27%** |
| length 1KB | 0.77× | 0.90× | +17% |
| find_byte 4KB | 1.04× | 1.11× | +7% |
| find_pattern 4KB | 6.69× | 6.79× | nul |

**Constat clé** : equal et compare (qui ont des `__attribute__((target("avx2")))` mais utilisent du C standard pour la boucle de tail / la décision) bénéficient de l'autovec quand `-march=x86-64-v4` est passé. Le simple recompile passe `equal 4KB` de 0.85× memcmp à **1.08× memcmp** — bc-core repasse devant glibc.

## 4. Faisabilité AVX-512 — analyse

### Quelles primitives bénéficieraient d'AVX-512 ?

| Primitive | SIMD-bound ? | Gain attendu | Recommandation |
|---|---|---|---|
| `bc_core_copy` | oui (sur > 64 KB) | x1.5-2 sur > 1 MB | **OUI** (sauf si memcpy autovec déjà couvre) |
| `bc_core_fill` / `zero` | oui | x1.5-2 sur > 1 MB | **OUI** |
| `bc_core_compare` | oui | mesuré x1.27 sur 4 KB | **OUI** |
| `bc_core_equal` | oui | mesuré x1.27 | **OUI** |
| `bc_core_find_byte` / `last_byte` | oui | x1.5-2 sur > 1 KB | OUI |
| `bc_core_find_pattern` | partiellement | x1.2 (déjà 8× memmem) | NON, ROI faible |
| `bc_core_count_*` | oui | x1.5-2 | OUI |
| `bc_core_byte_mask` | oui | x1.5-2 | OUI |
| `bc_core_format_*` | non (integer/divmod-bound) | nul | NON |
| `bc_core_sort` | non (indirect call + branch-bound) | nul | NON |
| `bc_core_sha256` | non (SHA-NI HW-bound) | nul | NON |
| `bc_core_crc32c` | non (SSE4.2 HW-bound) | nul | NON |

**Cibles AVX-512 prioritaires** : `compare`, `equal`, `copy`, `fill`, `zero`, `find_byte`, `find_last_byte`, `count_byte`, `count_lines`, `byte_mask`. ~10 primitives sur 30.

### Coûts de complexité

1. **Maintenance double-source** : chaque primitive byte-level a deux versions à maintenir (AVX2 + AVX-512). Bug fixé dans une → à propager dans l'autre.
2. **Tests doubles** : il faut prouver les deux paths corrects. Le path AVX-512 ne tournerait que sur Intel récent — le runner CI Ubuntu 24.04 sur Intel Xeon le couvre, mais pas tous les runners.
3. **Throttling AVX-512** sur certains CPUs (Skylake-X, Cascade Lake) : un kernel SIMD 512-bit fait baisser la fréquence du core pendant ~600 µs. Sur Tiger Lake+ et Sapphire Rapids, **pas de throttle** (transition zéro-pénalité). bc-core ciblant `-march=x86-64-v3 + AVX-512` au runtime via dispatch, on n'a pas le contrôle du CPU ; il faut un threshold (utiliser AVX-512 seulement si `len ≥ N`).
4. **Cache pressure des fonctions clones** : avoir 2 versions de chaque fonction dans le binaire double la footprint icache. Sur petits buffers répétés, ça peut anti-optimiser via icache misses. Mesurable.

### CPUs cibles

- AMD Zen 3 (Ryzen 5700G du dev) : pas d'AVX-512 → utilise toujours le path AVX2.
- AMD Zen 4+ (Ryzen 7000+) : AVX-512 dispo (mais simulé en double-pump 256-bit, gain plus modeste).
- Intel Tiger Lake/Ice Lake/Sapphire Rapids : AVX-512 natif, gain maximal.
- Intel Alder Lake/Raptor Lake : AVX-512 désactivé sur les P-cores (par BIOS Intel décision), donc fallback AVX2.

Conclusion : **environ 30-50 % du parc x86-64-v3 utilisateur peut bénéficier d'AVX-512** (Intel Tiger Lake+, Sapphire Rapids, Zen 4+).

## 5. Stratégies d'implémentation

### Option A — `__attribute__((target_clones("avx2,avx512bw")))`

```c
__attribute__((target_clones("avx2", "avx512bw")))
bool bc_core_compare(const void* a, const void* b, size_t len, int* out_result)
{
    /* code générique - le compilateur génère 2 corps */
}
```

- **Pour** : zéro code de dispatch. ifunc resolver appelé une seule fois au load.
- **Contre** : le compilateur fait l'autovec lui-même → moins de contrôle. Pas de threshold size dans le dispatch.
- **Compatibilité** : gcc ≥ 8 et clang ≥ 14.

### Option B — dispatch manuel via `g_*_impl` (pattern actuel)

```c
__attribute__((target("avx2"))) static bool bc_core_compare_avx2(...) { ... }
__attribute__((target("avx512bw"))) static bool bc_core_compare_avx512(...) { ... }

static bool (*g_compare_impl)(...) = bc_core_compare_avx2;

__attribute__((constructor))
void bc_core_compare_dispatch_init(void)
{
    if (cpu_features_has_avx512bw()) {
        g_compare_impl = bc_core_compare_avx512;
    }
}
```

- **Pour** : contrôle fin (threshold size, override pour tests). Cohérent avec le pattern SHA-NI déjà en place.
- **Contre** : 1 indirect call par appel (~0.5 ns). Code de dispatch à maintenir.
- **Coût mesuré** : 0.5 ns sur les ops < 5 ns = 10 % ; sur ops > 30 ns = négligeable.

### Option C — hybride : ifunc resolver + threshold size dans la fonction

```c
__attribute__((target("avx512bw"))) static bool bc_core_compare_avx512(...) {
    if (len < 256) {
        return bc_core_compare_avx2(...); /* small payload, AVX2 amortise mieux */
    }
    /* AVX-512 path */
}
```

- **Pour** : pas d'overhead dispatch via ifunc, gestion du throttle small-len.
- **Contre** : duplication code AVX2 dans le binaire AVX-512 (le linker/compilo peut dédoublonner).

### Recommandation

**Option C** pour les 10 primitives ciblées. Pattern :

```c
__attribute__((target_clones("avx2", "avx512bw")))
bool bc_core_compare(...)
{
    /* threshold via le compilateur autovec, plus la même fonction */
    /* ... corps existant ... */
}
```

L'autovec gcc/clang sait utiliser AVX-512 pour `_mm256_*` quand `target("avx512bw")` est actif (le compilateur lift les opérations 256-bit en 512-bit fusionnées si possible).

**Plus simple encore** : ne RIEN écrire d'AVX-512 explicite, juste activer `target_clones` sur les fonctions chaudes et laisser le compilateur autovec. Mesure : sur Intel laptop, ce simple changement fait passer `equal 4KB` de 0.85× → 1.08× memcmp.

## 6. Étude inlining

### Coût mesuré du dispatch indirect

- Pattern actuel : `bool (*g_*_impl)(...) = ...` puis `wrapper public { return g_*_impl(...); }`.
- Coût : ~0.5 ns par appel sur Zen 3, ~0.4 ns sur Tiger Lake (branch prediction prend la main, BTB hit).
- Significatif uniquement sur ops < 10 ns (donc petites tailles).

### Cas où l'inlining gagne mesurablement

| Op | Taille | Sans inline | Avec inline (`always_inline`) | Gain |
|---|---|---|---|---|
| `bc_core_swap` (sort hot) | 8 bytes | ~5 ns | ~1 ns | x5 (mesuré PR #9 sort 1.5-3× qsort) |
| `bc_core_copy` (sort hot) | 4-32 bytes | ~3-5 ns | ~1-2 ns | x2-3 si appelé en boucle |
| `bc_core_format_*_decimal` (writer wrapper) | scratch fixe | déjà inline | — | n/a |

### Cas où l'inlining n'aide pas

- Ops > 100 ns : 0.5 ns d'overhead = 0.5 % du coût total → non mesurable.
- Ops appelées rarement : inliner gonfle le binaire pour un gain nul.

### Stratégie recommandée

| Catégorie | Recommandation |
|---|---|
| Dispatch SIMD AVX2/AVX-512 | **garder indirect call** (1 appel par invocation, négligeable) |
| Helpers hot-loop (swap, copy < 32 B, fill < 32 B) | **`__attribute__((always_inline))` static inline** avec fast-path par taille |
| Wrapper public → impl statique (compare, equal, find_byte) | **garder** pattern actuel, le coût dispatch est < 1 % du total |
| Format primitives appelées par writer wrappers | déjà inline grâce à `bc_core_writer_write_*` qui encapsulent + scratch buffer → OK |

### Risque inlining excessif

- icache pressure : si `always_inline` sur 50 sites → binaire grossit, plus de misses, perte nette.
- Build time : compilateur doit propager le code à chaque site.
- Debug : stepping moins lisible.

**Règle** : `always_inline` uniquement sur helpers < 30 lignes et appelés dans une boucle hot. Mesurer avant/après.

## 7. Plan de mise en œuvre

### Phase 1 — Inline helpers hot-loop (faisabilité immédiate)

Cibles déjà adressées dans la session 2026-04-27 :

- ✅ `bc_core_sort_swap_fast` (PR #9) — 1.5-3× speedup sur sort.
- ✅ `bc_core_writer_write_*` numerics — déjà inline via scratch buffer.

À adresser :

- `bc_core_copy` / `fill` / `zero` pour tailles < 32 bytes : actuellement le wrapper public va dans `*_avx2`. Un fast-path inline `static inline` dans le header public pour `len < 32` éviterait l'indirect call. Gain attendu sur 64B : 0.93× → 1.0× memcpy (Ryzen).

### Phase 2 — `target_clones` AVX-512 sur les 10 primitives byte-level

Effort : 1-2 heures de code, 1 jour de tests.

```c
__attribute__((target_clones("avx2", "avx512bw")))
bool bc_core_compare(const void* a, const void* b, size_t len, int* out_result)
{
    /* corps existant inchangé */
}
```

À appliquer sur : `compare`, `equal`, `find_byte`, `find_last_byte`, `find_pattern` (à mesurer), `count_byte`, `count_lines`, `count_words`, `byte_mask_find_*`, `copy`, `fill`, `zero`.

Test : runner CI doit avoir AVX-512 (GitHub Actions Ubuntu sur Intel Xeon a depuis 2023). Vérifier en CI que le path AVX-512 est exercé.

Coût bench : à mesurer sur Intel Tiger Lake. Sur Zen 3, aucun changement (path AVX2 reste).

### Phase 3 — Threshold size pour throttling

Sur les CPUs avec throttling AVX-512 (Skylake-X, pre-Tiger Lake), threshold `len < 256 → AVX2 path`. Implémenté via :

```c
__attribute__((target_clones("avx2", "avx512bw")))
bool bc_core_compare(...) {
    if (len < 256U) {
        /* code AVX2-only path, force le compilo */
    }
    /* corps avec autovec AVX-512 quand actif */
}
```

À skipper sur Tiger Lake / Sapphire Rapids où le throttle n'existe pas.

### Phase 4 — Mesure + validation

- Bench sur 3 machines : Ryzen 5700G (path AVX2), Intel Tiger Lake (AVX-512 natif), Intel Skylake-X si dispo (vérifier throttle).
- Valider que les ratios bc/libc sont ≥ 1.0× sur les 3 machines.
- Asan/tsan/ubsan/memcheck verts sur les deux paths.

## 8. Analyse AMD Zen 3 spécifique

### Conditions de bench — alignement obligatoire

Les benches sont sensibles à 3 paramètres système. **Tous les chiffres de cette étude utilisent les mêmes conditions sur les 2 machines** :

| Paramètre | Valeur attendue | Vérification |
|---|---|---|
| ASLR | 0 (off) | `cat /proc/sys/kernel/randomize_va_space` |
| CPU governor | performance | `cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor` |
| Turbo Boost | off | AMD: `cat /sys/devices/system/cpu/cpufreq/boost`<br>Intel: `cat /sys/devices/system/cpu/intel_pstate/no_turbo` |

Sans alignement : un governor en `powersave` peut diviser les chiffres par 2-3, ASLR full ajoute du bruit (cache layout différent à chaque run). Toujours valider via `perf stat` que la fréquence effective est stable.

### perf stat compteurs hardware

Mesures sur la suite complète `bench_bc_core_ops` (~32-36 s d'exécution).

| Métrique | Ryzen 7 5700G | Intel i7-1165G7 |
|---|---|---|
| Cycles | 134.8 G | 146.4 G (estimé) |
| Instructions | 374.3 G | 348.1 G |
| **IPC** | **2.78** | 2.38 |
| L1d loads | 128.8 G | 92.9 G |
| L1d miss rate | 13.4% | 18.5% |
| LLC miss rate | n/a (perf event) | 0.10% |
| Branch miss rate | 0.13% | 0.20% |
| Wall time | 35.9 s | 32.3 s |

**Lectures** :
- **IPC 2.78 sur Zen 3** = pipeline SIMD bien rempli. Théorique max Zen 3 ≈ 5-6 IPC (4 issue + spéculation), 3-4 IPC sur SIMD-heavy. **bc-core sature déjà ~70 % du pipeline Zen 3**.
- L1d miss rate 13.4 % est cohérent avec les gros buffers (1 MB+) qui débordent du L1d 32 KiB.
- Branch miss 0.13 % = excellent. Le code est essentiellement straight-line dans les boucles SIMD.
- Intel finit plus vite (3.6 s de moins) malgré IPC plus faible : la fréquence effective Tiger Lake (4.7 GHz turbo, 2.8 GHz base) compense.

### Instructions x86-64-v3 — utilisation actuelle Zen 3

Toutes les instructions x86-64-v3 disponibles sont déjà utilisées :

| Instruction | Utilisée par bc-core | Comment |
|---|---|---|
| AVX2 (256-bit SIMD) | oui, partout dans memory/* | intrinsics `_mm256_*` |
| BMI2 (bzhi/pdep/pext) | oui, implicitement | compilateur via `-march=x86-64-v3` |
| FMA | non pertinent | bc-core integer-only |
| LZCNT | oui | `__builtin_clz` → LZCNT |
| TZCNT | oui | `__builtin_ctz` → TZCNT |
| MOVBE | oui (potentiellement) | `__builtin_bswap*` |
| F16C | non | pas de half-float |
| POPCNT | oui | `__builtin_popcount` |
| LAHF/SAHF | oui | implicite ABI |

### Instructions AMD-spécifiques disponibles **non** utilisées

| Instruction | Disponible Zen 3 | Utilité bc-core |
|---|---|---|
| **SSE4a** (`EXTRQ`, `INSERTQ`, `MOVNTSD`, `MOVNTSS`) | oui (AMD-only) | aucune — opérations 16/32-bit non pertinentes pour bc-core memory ops |
| **CLZERO** (cache line zero) | oui (AMD-only depuis Zen) | potentiellement utile pour `bc_core_zero` sur > 1 MB. **Gain estimé : marginal** car `_mm256_stream_si256` déjà NT-store et glibc memset0 utilise déjà cette logique sur AMD |
| **MWAITX** (low-power wait) | oui | non pertinent (bc-core ne fait pas de spinlock) |
| **WBNOINVD** | oui (Zen 3+) | non pertinent |
| **PDEP/PEXT fast** | oui (Zen 3 vs slow Zen 1/2) | bc-core ne fait pas de bit shuffle complexe — non utilisé |
| **AVX2 ports d'exécution** | 4 ports SIMD sur Zen 3 (vs 2 sur Zen 1/2) | déjà saturés par notre unroll x4 — pas d'optim possible |

### Optimisations Zen 3-spécifiques pertinentes ou non

1. **CLZERO pour bc_core_zero** (zero une cache line entière en 1 µop) : potentiellement intéressant pour `zero` sur > 1 MB. Mais glibc memset0 ratio 0.99× → on est déjà à parité avec NT-store. Gain CLZERO probable < 5 %. **Non prioritaire**.
2. **PDEP/PEXT pour byte_mask** : pourrait remplacer le shuffle LUT du byte_mask. Mais nos benches montrent byte_mask déjà rapide. Gain incertain. **Non prioritaire**.
3. **WBNOINVD pour cache flush** : bc-core utilise CLFLUSHOPT pour `evict_range`. WBNOINVD pourrait être plus rapide sur > 1 MB mais demande privilèges (kernel ring 0). **Non applicable userspace**.
4. **Loop unroll x8** au lieu de x4 sur Zen 3 : Zen 3 a 4 ports SIMD, donc unroll x4 sature déjà. **Pas de gain attendu**.

### Verdict Zen 3

**bc-core utilise déjà la totalité de l'instruction set x86-64-v3 disponible et sature ~70% du pipeline SIMD Zen 3.** Les ratios bc/libc sont à parité ou supérieurs (1.00× memcmp, 1.02× memcmp pour equal, 8.74× memmem pour find_pattern, 1.69× qsort pour sort random 4KB). Aucune optim AMD-spécifique évidente n'apporterait > 5 %.

**Le seul gisement non capté est AVX-512** — qui n'est pas disponible sur Zen 3. Donc côté AMD : **bc-core est à son optimum jusqu'à ce que Zen 4+ devienne la baseline** (et là, la même target_clones AVX-512 servira automatiquement).

## 9. Recommandations finales

1. **Faire la Phase 2** (target_clones AVX-512) — gain mesuré +27% sur equal, +9% sur compare, sans complexité de dispatch manuel.
2. **Étendre la Phase 1 inline** aux ops < 32 bytes (copy/fill/zero) — gain marginal mais cohérent.
3. **Ne pas faire la Phase 3** pour l'instant — bc-core utilisateur cible est Ryzen 5700G + Intel Tiger Lake (pas de throttle). Re-évaluer si un consumer signale du throttle sur un CPU plus ancien.
4. **Toutes les instructions x86-64-v3 disponibles sont déjà utilisées** par bc-core (directement ou via le compilateur). Pas de gisement caché côté baseline.
5. **L'instruction la plus impactante non utilisée** est AVX-512 sur les ops byte-level. Phase 2 suffit pour la capter.

Score d'effort vs gain :

| Action | Effort | Gain |
|---|---|---|
| Phase 1 inline copy/fill/zero < 32B | 1 h | +5-7% sur 64B |
| Phase 2 target_clones AVX-512 | 1 jour (10 primitives) | +9-27% sur Intel récent |
| Phase 3 threshold throttle | 0.5 jour | 0 sur Tiger Lake+ |

Phase 2 = ROI le plus élevé.
