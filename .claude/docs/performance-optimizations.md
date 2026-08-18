# dnf-composer performance optimizations — full report

A record of every performance change made to dnf-composer during the 2026-06 optimization effort:
what was changed, why (the measurement that motivated it), how it works, its measured impact, and
the correctness gate it passed. Also documents the attempts that were **rejected or reverted**, so
future work doesn't re-tread them.

All raw numbers live in `tests/profiler/profile.md` (dated sessions), `tests/benchmark/results.md`,
and the commit messages cited below. Sessions from 2026-07-29 onward self-report hardware/toolchain
in a per-session **Env:** line (CPU, OS, compiler, build type, runtime AVX2 dispatch, FFTW version,
git commit) — only compare sessions with matching **Env:** lines directly. Earlier sessions predate
that line and all ran on the reference dev machine (MSVC 19.44, /O2 /arch:AVX2, Windows 11).
Field sizes: 1D = 100 samples, 2D = 50×50.

---

## 1. Headline results

### End-to-end throughput (median steps/sec, canonical detection field per N)

| dim / N | baseline | final | gain | Cedar (FP32) | dnfc vs Cedar |
|---------|---------:|------:|-----:|-------------:|--------------:|
| 2D N=10  | 253  | **2008** | 7.9× | 1545 | **+30%** |
| 2D N=50  | 49.7 | **334**  | 6.7× | 297  | **+12%** |
| 2D N=100 | 24.8 | **161**  | 6.5× | 144  | **+12%** |
| 1D N=10  | —    | **61601** | —   | 3057 | **20×** |
| 1D N=100 | 2647¹ | **5877** | 2.2× | 297 | **20×** |

dnf-composer went from ~6× *slower* than Cedar in 2D to **12–30% faster at every size — while
computing in full float64 against Cedar's float32** (OpenCV-backed) implementation.
¹ 1D baseline is from the first in-repo benchmark session (mid-effort); the 2D baseline is the
original pre-optimization measurement from the cross-framework benchmarking repo.

### Per-element step() cost, 2D 50×50 (profiler aggregate, µs/step)

| element | first profiler session | final | gain |
|---------|----------------------:|------:|-----:|
| NeuralField2D | 50.1 | **12.7** | 3.9× |
| GaussKernel2D | 71.7 | **24.8** | 2.9× |
| MexicanHatKernel2D | 188.7 | **75.9** | 2.5× |
| OscillatoryKernel2D | 130.0 | **55.3** | 2.4× |
| AsymmetricGaussKernel2D | 69.8 | **23.4** | 3.0× |
| NormalNoise2D | 79.0 | **12.7** | 6.2× |
| CorrelatedNormalNoise2D | 105.8 | **24.6** | 4.3× |
| MemoryTrace2D | 101.3 | **3.0** | **33×** |

(Those "first session" numbers already include the earliest fixes; before *any* work the field step
alone was ~165 µs and a kernel step ~114 µs.)

Total sampled CPU for the profiler's full element sweep fell from **170 s → 64 s** across the
sampling sessions.

---

## 2. Methodology — measure first, gate everything

Two principles governed every change:

1. **No optimization without a measurement pointing at it.** The original premise ("2D convolution
   is the bottleneck") turned out to be wrong at first — early profiling showed noise RNG, sigmoid
   overflow, and string-map lookups dominated. Later, after those were fixed, convolution genuinely
   became (and remains) the top cost. Each round re-measured before acting.
2. **Every change passes an explicit correctness gate before it's kept:**
   - **Bit-identical changes** (allocation removal, dead-work elision, lookup hoists,
     order-preserving SIMD): golden conv tests in `tests/tools/test_math.cpp` compare against the
     scalar reference at **1e-12**, plus the full unit suite.
   - **Summation-order-changing changes** (symmetric folding): the **FieldDynamics validation
     suite** (`tests/validation/`, 300 1D + 300 2D sims re-run live against vendored reference CSVs)
     must pass at **abs 1e-4**. A margin probe (tolerance tightened to 1e-9) verifies how much
     headroom remains.
   - Anything that failed a gate was **reverted** (see §5).

Tooling built/used for this (see the companion docs):
- `tests/profiler/` — per-element step() timing, appends dated sessions to `profile.md`.
- `tests/benchmark/` — throughput (steps/sec) for 1D/2D at N=10/50/100, appends to `results.md`.
- `tests/validation/` — the 1e-4 field-dynamics regression gate (this suite caught two real bugs
  during the work: a scratch-buffer overflow and the first folding attempt).
- **Very Sleepy CPU sampling** for function- and line-level attribution inside step() —
  procedure in [very-sleepy-profiling.md](very-sleepy-profiling.md). Line-level sampling found
  several of the wins below (e.g. the ziggurat magic-static guard).

---

## 3. The optimizations (by commit)

### `64573a1` — first round: fix the incidental hot paths + build the measurement infra

Profiling overturned the "convolution is the bottleneck" assumption — at that point conv was ~11%
of the 2D step; the big costs were incidental:

| change | files | why / effect |
|--------|-------|--------------|
| **Sigmoid exponent clamp ±88** | `src/elements/activation_function.cpp` | `exp(-100·x)` at steepness 100 hit the float overflow slow path on every cell. Clamping the exponent (numerically a no-op — already saturated) cut the field step **165→56 µs**. |
| **Kernel `components["output"]` hoist** | `gauss_kernel_2d.cpp` + 3 kernels | An `unordered_map<string>` hash lookup **per cell** (2500/step). Hoisted to a local ref: kernel step **114→48 µs**. |
| **`/arch:AVX2` on the library** | `CMakeLists.txt` | Enables the SIMD work below; scoped to the library target; `/fp:precise` kept. |
| **Conv2dScratch** | `include/tools/math.h` + kernel elements | `conv2d_separable_into` allocated six vectors per call, every field, every step. Caller-owned scratch, sized once in `init()`. |
| **Branch-free conv inner loops** | `math.h` | Contiguous, bounds-check-free interior loops (prerequisite for later vectorization). |
| **NeuralField2D `visited_` hoist** | `neural_field_2d.{h,cpp}` | Bump-detection flood-fill allocated `vector<bool>(2500)` every step → reusable member. |
| **Noise amp==0 short-circuit** | 4 noise elements | A disabled (amplitude 0) noise element still drew 2500 Gaussians/step (~47 µs of pure waste). |
| **xoshiro256+ + ziggurat RNG** | `src/tools/math.cpp`, `math.h` (`fillNormal`) | Replaced `std::mt19937` + `std::normal_distribution` (slow on MSVC) with a fast PRNG + exact ziggurat sampler, filling output **in place** (no per-step temp vector). NormalNoise2D **80→14.7 µs (5.4×)**. Distribution preserved (mean 0, var 1 — pinned by statistical unit tests); the random stream is not, which is acceptable for noise. |

Also in this commit: the validation, benchmark, and profiler infrastructure itself, plus two bug
fixes the new validation suite caught (Conv2dScratch sizing overflow crashing mexican-hat 2D;
removal of the first, dynamics-breaking folding attempt).

### `10424a6` — vectorize the convolution inner loop; kill the remaining per-cell lookups

Very Sleepy sampling (85k samples) showed `conv_valid_into` at **60.5% of all CPU** and
`unordered_map` hashing at **10.4%** (75% of it MemoryTrace2D, 24% CorrelatedNormalNoise2D).

- **Kernel pre-reversal + 4-wide AVX2 across outputs** (`math.h`): the inner dot product read the
  kernel backwards (`k[M-1-m]`), blocking vectorization; and as a strict-FP64 *reduction* MSVC
  can't auto-vectorize it (lane-parallel summation reorders adds). Solution: pre-reverse the kernel
  once (same summation order → bit-identical), then compute **4 adjacent outputs at once** with
  `__m256d` FMA — one accumulator lane per output, so **no reduction is reordered**. Bit-identical
  (golden tests at 1e-12). Conv self-time **102.9→45.4 s**; kernels ~2× (GaussKernel2D 71→35 µs,
  MexicanHat2D 189→84, Oscillatory2D 130→57).
- **Per-cell map-lookup hoists** (`memory_trace{,_2d}.cpp`, `correlated_normal_noise_2d.cpp`):
  these still indexed `components["input"]/["output"]` inside their per-cell loops. Hoisted to
  local pointers. **MemoryTrace2D 102→3.1 µs (33×** — it was almost entirely lookup overhead).

### `ee1c947` — vectorize the sigmoid

After the conv fix, `SigmoidFunction::apply` + `expf` were ~20% of CPU (scalar `expf` per cell).
Replaced with an 8-wide AVX2 Cephes `exp256_ps` (range reduction + degree-6 minimax polynomial,
~1e-7 accurate), scalar fallback retained behind the `DNFC_HAVE_AVX2` guard. Sigmoid is an
elementwise **map** (no reduction), so the gate is the 1e-4 suite — passed across all 400 sigmoid
sims. **NeuralField2D 50→14 µs (3.5×)**; sigmoid sampled self-time 13.5→3.5 s.

### `a77f5db` — symmetric-kernel folding (the successful second attempt)

For symmetric kernels (Gauss, correlated-noise, the symmetric axis of Asymmetric), compute
`out[i] = k[c]·w[c] + Σ_{j<c} k[j]·(w[j] + w[2c−j])` — **half the multiplies**, vectorized 4
outputs at a time. This *does* reorder the per-output summation, so it is gated on the 1e-4 suite,
not bit-identity. A margin probe (tolerance tightened to 1e-9) showed worst-case deviation across
all 600 sims stays at **~5.0e-5 — the reference-CSV truncation floor** — i.e. folding adds no error
beyond what's already in the stored references (the earlier scalar fold had failed this; the
vectorized pair-first form is numerically tamer). Non-symmetric kernels fall through to the
bit-identical path. **GaussKernel2D 35→27 µs, Asymmetric2D 35→27, MexicanHat2D 84→78**; pushed the
2D benchmark past Cedar at every N.

### `6c8a8f2` — dead-work elision round (line-level sampling finds)

| change | files | evidence → effect |
|--------|-------|-------------------|
| **updateInput zero-fill elision** | `src/elements/element.cpp` | `updateInput` (runs for *every* element, every step) did `fill(0)` then `+=` each source. Now copies the first source and adds the rest — one full pass over the buffer elided. Bit-identical. **~2 µs off every element** (e.g. NeuralField2D 14.4→12.7). |
| **amplitudeGlobal==0 accumulate guard** | 4 2D kernel `.cpp` | The O(N) `accumulate(input)` for the global offset was the dominant line of each kernel's own step body (~0.3–0.7 s each in the sample) — computed even when `amplitudeGlobal == 0`. Guarded the accumulate and the per-cell offset add. Bit-identical. ~1–2 µs off kernels with the offset disabled (Oscillatory/Asymmetric defaults). |
| **Ziggurat table-accessor hoist** | `src/tools/math.cpp` | Line-level sampling showed the single biggest line *inside* `zigguratNormal` (2.09 of 5.56 s) was `zigTables()` — the function-local static's **thread-safe-init guard, re-checked on every one of the 2500 samples/step**. The table ref is now fetched once per `fillNormal` batch. Same tables, same stream, bit-identical. **NormalNoise2D 14.8→12.7 µs (−14%)**, CorrelatedNoise2D 27→24.6. |

### `1fe55cf` — thread-safety hardening (correctness, not perf)

Not an optimization, but part of the same effort: made the logger (no shared global instance,
atomic `minLogLevel`, mutex-serialized sinks) and the element/plot ID counters (`std::atomic`)
safe for the supported one-Simulation-per-thread parallel model used by consumers like neat-dnfs.
See [threading.md](threading.md).

---

## 4. Why the SIMD changes are safe (the FP64 story)

The project constraint is full FP64 with `/fp:precise` — no `/fp:fast`, no float path. That
forbids the compiler (and us) from reordering floating-point *reductions*. The conv vectorization
respects this by parallelizing **across independent outputs** (each output's tap sum keeps its
scalar order → bit-identical). Folding *does* reorder — which is why it alone is gated on the
1e-4 dynamics suite instead, with a measured 2× margin. The sigmoid is a map (no reduction), so a
~1e-7-accurate SIMD exp is safely inside the same gate.

## 5. Tried and rejected / reverted — do not re-tread

| attempt | outcome |
|---------|---------|
| **Scalar symmetric folding (v1)** | Perturbed bistable abssigmoid memory attractors past 1e-4 (caught by the validation suite, sims 049/050, dev up to 3.7). Reverted; later replaced by the vectorized pair-first fold that passes. |
| **y-pass transpose** (cache-friendly column pass) | No measurable win; reverted. Later line-level sampling confirmed the column gather was never the hotspot. |
| **Circular-extension gather → block-copy** | Driver line dropped, but the `std::copy` runs surfaced as `memmove` and the total was flat (98.7→99.2 s). Reverted — measurement said no. |
| **Benchmark `setComputeStateMetrics(false)`** | Wrong approach (the program must do its real work); reverted. The bump-detection cost was instead reduced (buffer hoist) and later measured at only 0.2–2% — never worth more. |
| **FFT convolution libraries (FFTW/pocketfft/pffft)** | Web survey: our separable kernels (13–60 taps/axis) sit at/below the ~64-tap FFT crossover; direct separable filtering beats FFT 1.6–2.1× in FP64 at these aperture sizes. Also, no library escapes the FP-reduction reordering question. No dependency added. |
| **Intra-step element parallelism** | Elements update **in place** and read upstream outputs mid-step (Gauss-Seidel over a *cyclic* field↔kernel graph) — concurrent element stepping both data-races and changes the dynamics (Jacobi). Rejected; see [threading.md](threading.md). Across-field (multi-Simulation) parallelism is the supported model. |
| **Kernel component-pointer member caching** | Evaluated and skipped: after the earlier hoists no per-cell lookups remain — only a once-per-step ref bind (~0.1 µs). Churn without benefit. |
| **Box-Muller SIMD noise** | Considered for `zigguratNormal`; line-level sampling showed the actual hotspot was the magic-static guard (fixed for free), not the sampler arithmetic. Deferred unless noise re-emerges as a cost. |

## 6. Current state & remaining headroom

From the latest sampling session (total sweep 64 s):

| % CPU | function | status |
|------:|----------|--------|
| ~52% | `conv_valid_into` | **FMA-bound** — the structural wins (across-output SIMD, folding) are taken; what remains is the arithmetic itself. |
| ~15% | `conv2d_separable_into` driver | gather/scatter orchestration; block-copy variant measured flat. |
| ~5%  | `zigguratNormal` | post-hoist; further gains need a different sampler (measure-and-maybe). |
| ~5%  | `SigmoidFunction::apply` | already AVX2. |

Known structural notes:
- **MexicanHatKernel2D** is inherently ~2× a folded conv (exc + inh over the same input); only the
  x-pass gather is shareable (~10% ceiling — not taken).
- **OscillatoryKernel2D cannot fold at 50×50**: its wide kernel is clamped by `computeKernelRange`
  to the asymmetric `{24, 25}` half-ranges (floor/ceil of half the field), producing an
  even-length, off-center tap vector. Forcing symmetry would alter the kernel → fails 1e-4.
- **The remaining big lever is threading** — across independent fields/Simulations (the supported
  model, see [threading.md](threading.md)); intra-step threading is off the table for the reasons
  in §5.

## 7. Reproducing the measurements

- Aggregate per-element timing: build & run `dnf_composer_profiler [iters]` → appends to
  `tests/profiler/profile.md`, including an **Env:** line with this machine's hardware/toolchain.
- Throughput: `dnf_composer_benchmark [steps] [runs]` → appends to `tests/benchmark/results.md`,
  including the **Env:** line and a single-field calibration figure/ratio table so runs from
  different machines can be compared approximately.
- Function/line-level attribution: Very Sleepy on a RelWithDebInfo build —
  [very-sleepy-profiling.md](very-sleepy-profiling.md).
- Correctness gates: `dnf_composer_tests` (full suite incl. golden conv tests) and
  `--gtest_filter=FieldDynamics*` (the 1e-4 dynamics gate).
