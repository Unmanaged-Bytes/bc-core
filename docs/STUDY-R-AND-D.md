# bc-core — étude R&D : optimisations mathématiques + API consumers

Date : 2026-04-27. Suite de `STUDY-AVX512-INLINE.md`. Deux axes :

1. **Math/SIMD avancé** — techniques extraites de la littérature publique (Lemire,
   StringZilla, BurntSushi, Intel/AMD optimization manuals) et applicables à
   bc-core sur baseline x86-64-v3 (et Phase 2 AVX-512).
2. **API extension consumers** — inventaire des patterns dupliqués dans les libs
   et tools aval, et proposition d'API à promouvoir dans bc-core.

Le but n'est pas d'élargir bc-core "au cas où" : chaque proposition doit citer
au moins un consumer concret aujourd'hui. Pas de bloat.

---

## Partie A — Optimisations math/SIMD identifiées

### A.1 État de l'art parcouru

| Technique | Source | Nature | Pertinent pour bc-core ? |
|---|---|---|---|
| 4-bit lookup `pshufb` + popcount | Daniel Lemire 2018, `lemire/Code-used-on-Daniel-Lemire-s-blog` | Itérer les bits set d'un masque 64-bit en 4-bit chunks → 7.65 GB/s vs 0.66 GB/s naïf | Non — bc-core consomme `__builtin_ctzll` qui se compile en `tzcnt` (1 cycle). Le 4-bit lookup brille seulement pour density élevée (>50% bits set). |
| `_mm512_cmpeq_epi8_mask` direct | Intel intrinsics guide | AVX-512 retourne `__mmask64` au lieu de `__m512i` → économise 1 `movemask` par chunk | **Oui** — Phase 2 (cf STUDY-AVX512-INLINE) sur compare/equal/find_byte/count_byte. |
| SWAR parse 8 ASCII digits | Lemire 2018, `lemire.me/blog/2018/09/30/quickly-parsing-eight-digits/` | Parse 8 chars '0'..'9' en 4 mul/shift sur un `uint64_t` chargé sans alignement | **Oui** — bc_core_parse_unsigned_integer_64_decimal (proposé) : 5 strtoul recensés en consumer. |
| Jeaiii itoa (déjà utilisé) | Junekey Jeon 2021 | Format décimal lookup 2-digit table + branchless | Déjà appliqué dans `bc_core_format_unsigned_integer_64_decimal`. Confirmé optimal (2.89× snprintf). |
| `VPCLMULQDQ` 256/512-bit | Intel ICL+ | CRC sur 4×128-bit ou 4×256-bit par cycle | Intel-only. Phase 2 candidat sur crc32c >= 4 KB. Gain estimé +30-50%. Non disponible Zen 3. |
| `GFNI` byte-bitmask | Intel ICL+, AMD Zen 4+ | Galois field affine → 8×8 bit transpose / popcount-byte / xor-table en 1 instruction | Non disponible Zen 3. Pas de consumer concret bc-core actuel. |
| 2-stage memchr (large stride + refine) | BurntSushi `memchr` crate | Lit 64 B/iter en SSE2/AVX2, refine au byte une fois mask non-zero | Déjà appliqué via unroll x4 dans `bc_core_find_byte`. Confirmé 1.06-1.08× memchr. |
| Prefix-scan/scanline (Hillis-Steele, Blelloch) | Hillis & Steele 1986 | Cumul running-sum vectorisé en log(n) passes | Aucun consumer bc-core actuel ne cumul de séries. Reporter. |
| StringZilla AVX2/AVX-512 dispatch | ashvardanian/StringZilla | UTF-8 + memmem vectorisé, baseline runtime-dispatched | bc_core_find_pattern bat déjà memmem 8.74× via Boyer-Moore-Horspool. Pas de gain à attendre. |
| Bit-twiddling Hacks (Sean Anderson, Stanford) | `graphics.stanford.edu/~seander/bithacks.html` | Branch-free min/max, log2, popcount, parity | bc-core consomme déjà `__builtin_popcount`/`__builtin_clz` (génère LZCNT/POPCNT en x86-64-v3). Aucun gain résiduel. |

### A.2 Techniques actionnables sur bc-core baseline x86-64-v3

#### A.2.1 SWAR parse 8 digits décimaux (NOUVEAU API)

**Pattern** (Lemire) :

```c
static inline uint64_t parse_eight_digits_swar(const char* p) {
    uint64_t chunk;
    __builtin_memcpy(&chunk, p, 8);
    chunk -= 0x3030303030303030ULL;          /* '0' */
    chunk = (chunk * (1U + (10U << 8))) >> 8;
    chunk &= 0x00FF00FF00FF00FFULL;
    chunk = (chunk * (1U + (100U << 16))) >> 16;
    chunk &= 0x0000FFFF0000FFFFULL;
    chunk = (chunk * (1U + (10000ULL << 32))) >> 32;
    return chunk & 0xFFFFFFFFULL;
}
```

Bench typique en littérature : ~0.6 ns pour 8 digits (vs ~2-3 ns en boucle char-par-char ou via `strtoul`).

**Application bc-core** : `bc_core_parse_unsigned_integer_64_decimal` (cf Partie B,
proposé). Boucle = un appel SWAR par 8-digit chunk, puis tail char-par-char.
Cible perf : 4-8× plus rapide que `strtoul` sur input ≥ 8 chiffres.

**Coût implémentation** : ~1 jour (impl + tests boundary + fuzz). Risque : **bas**
(SWAR pur, pas d'intrinsic, compile partout).

#### A.2.2 AVX-512 `kandn`/`kortest` pour multi-chunk equality (Phase 2)

Couvert dans STUDY-AVX512-INLINE §4 : `_mm512_cmpeq_epi8_mask` retourne directement
un `__mmask64`, donc 4 chunks → `kor` de 4 masques + `kortestq` au lieu d'un AND
de 4 vecteurs + `_mm256_testc_si256`. Gain estimé +5-10% supplémentaire vs AVX2
unrolled, en plus du doublement de throughput dû au passage à 64 B/chunk.

Pas d'action immédiate ici : déjà tracké dans STUDY-AVX512-INLINE.

#### A.2.3 Réduction de prologue/épilogue sur primitives < 64 B

Observation `STUDY-AVX512-INLINE §6` : indirect call dispatch coûte ~0.5 ns,
significatif sur ops < 5 ns. Math associé : pour une primitive qui traite N
bytes en ~K ns (parties : dispatch ~0.5 ns + body ~(K-0.5) ns), le ratio
dispatch/body devient critique quand N → 0.

Optimisation mathématique = **inline avec fast-path par taille** dans le header
public, ce qui supprime le dispatch :

```c
/* dans bc_core_memory.h, version proposée pour copy ≤ 32 B */
static inline __attribute__((always_inline))
void bc_core_copy_inline_small(void* dst, const void* src, size_t n) {
    /* 4 fast-paths sans intrinsic AVX */
    if (n == 0) return;
    if (n <= 8)  { uint64_t v; __builtin_memcpy(&v, src, n); __builtin_memcpy(dst, &v, n); return; }
    if (n <= 16) { uint64_t lo, hi; __builtin_memcpy(&lo, src, 8); __builtin_memcpy(&hi, (const char*)src + n - 8, 8);
                   __builtin_memcpy(dst, &lo, 8); __builtin_memcpy((char*)dst + n - 8, &hi, 8); return; }
    /* ... overlap-based, classique */
}
```

**Contrainte ISA-neutre** : pas de `__m256i` dans le header. C'est exactement
le pattern proposé dans STUDY-AVX512-INLINE §6.4 (Phase 1).

### A.3 Techniques rejetées (pas de gain mesurable ou hors scope)

- **GFNI / VPCLMULQDQ** : Intel-only, pas de consumer concret bc-core, reporter.
- **Vectorized strtod (Daniel Lemire `fast_double_parser`)** : 1500+ lignes,
  utile uniquement si un consumer bc-core devient un parseur JSON chaud. Aucun
  cas actuel — `strtod` 2× usage est en CLI parsing (one-shot, non chaud).
  Reporter.
- **AVX-512 VBMI permute** : utile pour des transcodages exotiques (UTF-8→UTF-32),
  pas dans le scope bc-core.
- **AMX (Advanced Matrix Extensions)** : matriciel uniquement, hors scope.
- **NEON / SVE** : on cible x86-64. Pas de portage ARM prévu (différent du
  scope rolling release).

### A.4 Verdict Partie A

Une seule technique math activable maintenant **avec consumer concret** :
**SWAR digit parsing** pour le module `bc_core_parse_*` proposé en Partie B.

Les autres optims (AVX-512, inline fast-path) sont déjà tracées dans
STUDY-AVX512-INLINE et restent valables. Pas de double-trackage.

---

## Partie B — API extension pour consumers

### B.1 Inventaire des appels libc résiduels dans les consumers

Source : `grep -rh --include="*.c" libraries/*/src tools/*/src` (en excluant tests
et benchs).

| Appel libc | Occurrences src | Sites concernés | bc_core remplacement |
|---|---|---|---|
| `fprintf` | 135 | bc-runtime/cli (60+), bc-runtime/error_collector, bc-hrbl/export | `bc_core_writer_t` + `bc_core_writer_write_*` (existant) |
| `snprintf` | 37 | bc-hrbl/export (12 numériques + unicode), bc-runtime/cli (3), tools (22 dont CLI specs) | `bc_core_format_*` + `bc_core_writer_*` (existant) |
| `fwrite` | 8 | bc-hrbl/export (3 streams) | `bc_core_writer_*` (existant) |
| `strtoul` | 5 | tools/bc-count/cli (1), tools/bc-hash/cli (1), tools/bc-duplicate/cli (1), tools/bc-seek/cli (1), bc-containers/test (1) | **NOUVEAU** `bc_core_parse_unsigned_integer_64_decimal` |
| `strtod` | 2 | tools/bc-duplicate/bench/cache (1), tools/bc-hash/bench/cache (1) | **NOUVEAU** `bc_core_parse_double` (à différer) |
| `strerror` | 3 | bc-runtime/error_collector | `bc_core_error_describe` (existant via Phase A0) |
| `qsort` | 10 | bc-containers/vector (1), bc-hrbl/export (2), tools/bc-hash/diff,worker (3), tools/bc-duplicate/grouping (2), tools/bc-hash/bench (1), tools/bc-duplicate/bench (1) | `bc_core_sort_with_compare` (existant via Phase A3) |

Note : la migration **fprintf/snprintf/fwrite/qsort/strerror** est déjà sur le
plan (Axe B). Cette étude se concentre sur les API **manquantes** — celles que
les consumers ne peuvent pas couvrir avec bc-core actuel.

### B.2 Patterns dupliqués dans les consumers (helpers static)

| Pattern dupliqué | Sites | Lignes copiées | Promotion bc-core ? |
|---|---|---|---|
| `case_insensitive_equal(a, len_a, b, len_b)` | `libraries/bc-runtime/src/config/bc_runtime_config.c:598` (utilisé 8× pour "true"/"false"/"1"/"0"/"yes"/"no") | 1 site interne, mais usage clé pour parsing config | **Oui — promouvoir** : `bc_core_equal_case_insensitive_ascii`. Prochain consumer probable : tout parsing CLI ou config (bc-foyer, bc-hrbl import futur). |
| `write_cstring(writer, value)` / `write_string(writer, value)` (= `write_bytes(writer, s, strlen(s))`) | `tools/bc-count/src/output/bc_count_output.c:10`, `libraries/bc-runtime/src/app/bc_runtime_error_collector.c:91`, `libraries/bc-hrbl/src/export/bc_hrbl_export.c` (variants) | 3+ sites | **Oui — promouvoir** : `bc_core_writer_write_cstring(writer, "...")`. Le `BC_CORE_WRITER_PUTS` macro existe mais l'utilisateur préfère éviter les macros. |
| Literal write `bc_core_writer_write_bytes(w, "\n", 1)` ou similaire avec compile-time length | 14+ occurrences `tools/`, `libraries/bc-hrbl/src/export/` | 14 sites | **Oui — couvert par `bc_core_writer_write_cstring`** ci-dessus si `__builtin_strlen` disponible. Sinon ajouter une variante `_with_length(w, "\n", 1)` (mais trivialement écrite avec write_bytes). |
| `parse_double(text, &out)` wrapping `strtod` + erreur | `tools/bc-duplicate/src/bench/bc_duplicate_throughput_cache.c:89`, `tools/bc-hash/src/bench/bc_hash_throughput_cache.c:84` | 2 sites | **Reporter** : double-parsing rapide est un gros chantier (Lemire fast_double_parser ≈ 1500 lignes pour battre strtod). Les 2 sites actuels sont dans des bench caches one-shot, perf non critique. |
| `compare_double` qsort comparator | `tools/bc-duplicate/src/bench/bc_duplicate_throughput.c:53`, `tools/bc-hash/src/bench/bc_hash_throughput.c:51` | 2 sites | **Pas via bc-core** : dépend du sens du tri. Couvrir via `bc_core_sort_with_compare` (existant) — le caller fournit le comparator. |
| CLI `parse_uint(value, &out)` wrapping `strtoul` | `tools/bc-count/cli`, `tools/bc-hash/cli`, `tools/bc-duplicate/cli`, `tools/bc-seek/cli` | 4-5 sites | **Oui — promouvoir** : `bc_core_parse_unsigned_integer_64_decimal`. SWAR fast-path. |

### B.3 API hot-path — utilisation actuelle confirme la pertinence

Top-10 API bc-core (occurrences toutes catégories confondues) :

```
125  bc_core_fill
122  bc_core_zero
112  bc_core_copy
 61  bc_core_writer_write_char
 54  bc_core_writer_t
 45  bc_core_length
 37  bc_core_safe_multiply
 32  bc_core_safe_add
 31  bc_core_equal
 30  bc_core_writer_write_uint64_dec   (← nom legacy, à renommer en _unsigned_integer_64_decimal)
```

Implications :
- `fill/zero/copy` cumulent 359 sites consommateurs. Toute optim < 1 ns par
  appel (Phase 1 inline fast-path) génèrerait des micro-gains très répandus.
  C'est le point d'entrée **#1** pour l'inlining (cf STUDY-AVX512-INLINE §6).
- `writer_write_char` 61× appels — l'inlining fast-path "buffer pas plein"
  (déjà partiellement le cas) reste pertinent. Vérifier que le hot-path est
  effectivement inlined par le compilo en LTO.
- `bc_core_writer_write_uint64_dec` 30× : la migration vers le nouveau nom
  (`_unsigned_integer_64_decimal`) doit être planifiée avec attention car
  c'est la rupture API la plus impactante. Couverte par Axe B Phase 1.

### B.4 Propositions d'API à ajouter dans bc-core

Listées par priorité (consumer concret + ROI).

#### B.4.1 `bc_core_writer_write_cstring` (P1)

```c
/* dans bc_core_io.h */
bool bc_core_writer_write_cstring(bc_core_writer_t* writer, const char* cstring);
```

- **Sémantique** : strlen + write_bytes. Sortie identique à `BC_CORE_WRITER_PUTS`
  pour les littéraux (compilo constant-folde `__builtin_strlen` sur les literals).
- **Consumers immédiats** : 3 helpers static dupliqués (`write_cstring`/`write_string`)
  + 14 sites `write_bytes(w, "literal", N)` qui peuvent migrer.
- **Implémentation** : `return bc_core_writer_write_bytes(writer, cstring, bc_core_length(cstring));`
- **Coût** : 30 minutes (impl + 1 test cmocka).
- **Risque** : nul. Pas de macro, pas de comportement nouveau, pas d'overhead
  vs la macro existante (en LTO).

#### B.4.2 `bc_core_equal_case_insensitive_ascii` (P1)

```c
/* dans bc_core_memory.h */
bool bc_core_equal_case_insensitive_ascii(const void* a, size_t length_a,
                                          const void* b, size_t length_b);
```

- **Sémantique** : compare a/b sans casse, ASCII strict (`'A'..'Z' ↔ 'a'..'z'`,
  pas de UTF-8 fold ni locale). Retourne `true` si égales.
- **Consumers immédiats** : `bc_runtime_config_*` (8 sites parsing booléens) ; futurs
  parseurs CLI ou config dans tools si on développe `bc-foyer` côté Python ou
  un import HRBL.
- **Implémentation** : SIMD AVX2 — chaque chunk de 32 bytes :
  ```
  __m256i va = _mm256_loadu_si256(...);
  __m256i vb = _mm256_loadu_si256(...);
  __m256i va_lower = _mm256_or_si256(va, _mm256_set1_epi8(0x20));
  __m256i vb_lower = _mm256_or_si256(vb, _mm256_set1_epi8(0x20));
  /* mais seulement si byte ∈ [A..Z] ou [a..z] — sinon laisser inchangé */
  ```
  Pattern : masque `(byte >= 'A' && byte <= 'Z') | (byte >= 'a' && byte <= 'z')`
  → OR avec 0x20 conditionné par le masque. 4-5 ops vector par chunk.
- **Coût** : ~1 jour (impl SIMD + tests + bench vs strncasecmp).
- **Risque** : moyen. Tests boundary nécessaires (0x40 = '@', 0x5B = '[' ;
  bytes hors range ASCII ne doivent PAS être lower-cased).

#### B.4.3 `bc_core_parse_unsigned_integer_64_decimal` (P1)

```c
/* dans bc_core_format.h ou nouveau bc_core_parse.h */
bool bc_core_parse_unsigned_integer_64_decimal(const char* text, size_t length,
                                               uint64_t* out_value, size_t* out_consumed);
```

- **Sémantique** : parse les digits 0-9 depuis `text[0]`, s'arrête au premier
  non-digit ou à `length`. Retourne `true` si ≥ 1 digit consommé et pas
  d'overflow ; `out_consumed` indique combien de chars consommés.
- **Pas** : sign, espaces, base autre que 10, locale. C'est le pattern simple
  des CLI value parsers actuels (`strtoul(value, &end_pointer, 10)`).
- **Consumers immédiats** : 4 CLI parsers (bc-count, bc-hash, bc-duplicate, bc-seek)
  + futurs parseurs config bc-runtime.
- **Implémentation** : SWAR 8-digit chunk (cf §A.2.1) + tail char-par-char.
  Cible perf : 4-8× strtoul sur input ≥ 8 chiffres.
- **Coût** : ~1 jour (impl SWAR + tests boundary 0/1/7/8/9/16/17/19/20 digits +
  overflow + fuzz invariant `parsed == strtoull when valid`).
- **Risque** : bas (SWAR pur, pas d'intrinsic). Edge cases : `0` seul, max
  `18446744073709551615` (20 chars), overflow `> 2^64-1`.

#### B.4.4 `bc_core_parse_signed_integer_64_decimal` (P2)

```c
bool bc_core_parse_signed_integer_64_decimal(const char* text, size_t length,
                                             int64_t* out_value, size_t* out_consumed);
```

- **Sémantique** : optionnel signe `-`/`+`, puis digits 0-9.
- **Consumers immédiats** : aucun direct, mais probable lors de la migration
  de `bc-runtime/config` sur `bc_core_parse_*` (parsing entiers signés).
- **Implémentation** : wrapper sur unsigned + clip range.
- **Coût** : ~30 minutes (après B.4.3).
- **Décision** : **inclure dans la même PR** que B.4.3 — coût marginal négligeable,
  cohérence d'API.

#### B.4.5 `bc_core_parse_double` (REPORTER)

- **Consumers** : 2 sites (bench cache one-shot).
- **Justification report** : (a) charge d'implémentation très élevée pour
  battre strtod (Lemire `fast_double_parser` = 1500 lignes + sub-normaux + GMP
  edge cases) ; (b) consumers actuels non-critiques perf ; (c) Phase A0 plan
  initial l'avait déjà mis en "report" — pas de raison de bouger.
- **Action** : si un futur consumer JSON-chaud émerge, rouvrir.

#### B.4.6 Inline fast-paths pour copy/fill/zero (Phase 1 STUDY-AVX512-INLINE)

Déjà couvert dans STUDY-AVX512-INLINE §6. Pas de double-trackage.

### B.5 API rejetées (pas de consumer concret)

| Proposition | Pourquoi rejetée |
|---|---|
| `bc_core_string_concat / split / trim` | Aucun consumer C actuel. Les outils manipulent des `(ptr, length)` slices, pas des C-strings. |
| `bc_core_writer_write_labeled_*` (key/value) | Domain-specific (bc-count, bc-hash output formats). Reste static dans le tool. |
| `bc_core_writer_write_padded_string` | Aucun consumer. |
| `bc_core_to_lower_ascii` (in-place) | bc_core_equal_case_insensitive_ascii suffit pour les usages actuels (compare). Si jamais besoin de produire la version lower-case, on l'ajoute à ce moment. |
| `bc_core_format_signed_integer_32` / `_unsigned_integer_32_*` | `_64_*` couvre déjà tous les cas (les uint32 promotent gratuitement). Pas de gain. |
| `bc_core_parse_size_t` / `_intmax_t` | API redondante avec `_unsigned_integer_64`. Le caller cast. |
| `bc_core_format_float_32` (vs double) | Aucun consumer. Tous les benches utilisent `double`. |
| Variantes `_with_error_code` | Convention bc-core = `bool` + `out_param` + `bc_core_error_t` séparé. Pas de double-API. |

---

## Partie C — Plan d'attaque proposé

| Phase | Items | Effort | PR | Précondition |
|---|---|---|---|---|
| **R&D-1** | `bc_core_writer_write_cstring` | 30 min | 1 PR seul | Aucune |
| **R&D-2** | `bc_core_equal_case_insensitive_ascii` (impl SIMD AVX2 + scalar) | 1 jour | 1 PR | Aucune |
| **R&D-3** | `bc_core_parse_unsigned_integer_64_decimal` + `_signed_integer_64_decimal` (SWAR + tail) | 1 jour | 1 PR | Aucune |
| Phase 1 inline | `bc_core_copy_inline_small` / `_fill_` / `_zero_` ≤ 32 B (cf STUDY-AVX512-INLINE) | 1 jour | 1 PR | R&D-1/2/3 livrés (évite les conflits sur include/public/) |
| Phase 2 AVX-512 | `target_clones("avx2,avx512bw")` sur 10 byte-level primitives | 2-3 jours | 1-2 PRs | Phase 1 livrée |
| Axe B | Migrations consumers (rename `bc_core_fmt_*` → `_format_*`, qsort → bc_core_sort, strerror → bc_core_error_describe, snprintf → bc_core_format_*) | 4-5 jours | 1 PR par repo consumer | R&D-1/2/3 livrés |

Total Phase R&D (Partie B propositions) : **~2.5 jours, 3 PRs**.
Total cumulé avec Phase 1 + Phase 2 + Axe B : ~10-13 jours.

### Vérifications par PR

Standard bc-core (cf `feedback_c_perf_test_bench_fuzz.md`) :
1. `bc-format` (clang-format)
2. `bc-test` (cmocka, viser 100% line + ≥ 90% branch)
3. `bc-sanitize asan/tsan/ubsan/memcheck` — 4 verts
4. `bc-check` (cppcheck clean)
5. Compile + tests sous **gcc ET clang**
6. Bench vs libc équivalent (strtoul / strncasecmp / write+strlen)
7. Fuzz harness libFuzzer pour les nouvelles primitives parse + equal_case_insensitive
8. Multi-arch via CI GitHub (Intel Xeon) + local Ryzen 5700G

### Critères de réussite numériques attendus

| API | Cible perf |
|---|---|
| `bc_core_parse_unsigned_integer_64_decimal` (≥ 8 digits) | ≥ 4× strtoul |
| `bc_core_equal_case_insensitive_ascii` (≥ 32 bytes) | ≥ 2× strncasecmp |
| `bc_core_writer_write_cstring` | parité avec `BC_CORE_WRITER_PUTS` macro (LTO inline) |
| Phase 1 inline copy ≤ 32 B | ≥ 1.5× version dispatch (gain ~0.5 ns) |

---

## Partie D — Conclusion

### D.1 Math/SIMD (Partie A)

**Une seule technique math actionnable** sans étendre le scope ISA : SWAR
parse de 8 digits décimaux (Lemire 2018), exploitable via la nouvelle API
`bc_core_parse_unsigned_integer_64_decimal`.

Les autres techniques (AVX-512, GFNI, VPCLMULQDQ, fast_double_parser)
restent intéressantes mais soit déjà tracées (STUDY-AVX512-INLINE), soit
sans consumer concret aujourd'hui (donc reportées).

### D.2 API consumers (Partie B)

**3 nouvelles API à promouvoir**, chacune avec ≥ 2 sites consumer concrets :

1. `bc_core_writer_write_cstring` — supprime 14+ sites de duplication.
2. `bc_core_equal_case_insensitive_ascii` — supprime 1 helper static, débloque
   futurs parseurs config.
3. `bc_core_parse_unsigned_integer_64_decimal` (+ signed sibling) — supprime
   4-5 sites `strtoul`, exploite SWAR Partie A.

Toutes les autres propositions sont **rejetées ou reportées** faute de consumer.

### D.3 Cohérence avec les contraintes architecturales

- Public headers ISA-neutres : ✅ (le SIMD reste dans .c).
- `bool + out_param` partout : ✅.
- Pas de macros publiques : ✅ (write_cstring est une fonction).
- Pas de bloat : ✅ (chaque ajout ≥ 2 consumers).
- x86-64-v3 baseline : ✅ (SWAR + AVX2 standards).
- Pas de défaillance silencieuse : ✅ (`out_consumed` permet au caller de
  voir combien de chars ont été lus).

### D.4 Recommandation finale

**Démarrer par R&D-1 → R&D-2 → R&D-3 en séquentiel** (3 PRs, ~2.5 jours).
Ces ajouts débloquent ensuite Axe B (migrations consumers) sans introduire
de dépendance entre Phase 1/Phase 2 inline/AVX-512 et le travail de
nettoyage des consumers — qui peut tourner en parallèle ensuite.

Phase 1 inline et Phase 2 AVX-512 restent actionnables après R&D-3, dans
l'ordre établi par STUDY-AVX512-INLINE.
