# Performance Benchmarking

The step loop is the hot path of this library — it runs thousands of times per second, and optimizing it is a recurring concern. This page covers the four performance tools in `tests/`, what each one measures, how to run them, and how to read what they print.

> **None of these are unit tests.** They are manual performance runs, not registered with `gtest_discover_tests`, and deliberately never run in CI (see [Why none of this runs in CI](#why-none-of-this-runs-in-ci)). For correctness testing, see [Testing](Testing.md).

---

## The idea: reproducible and attributable, not machine-agnostic

Wall-clock timing is never machine-agnostic. A result is only meaningful as a property of *(machine, toolchain, compiler flags, deck)* — the same code is legitimately faster on a different CPU, and a benchmark that pretends otherwise gives you a number you cannot act on.

So instead of chasing a portable absolute number, the tooling makes every measurement **reproducible** and **attributable**:

| Problem | How it is handled |
|---|---|
| A number from another machine is not comparable | Every run is stamped with a **fingerprint** over CPU, compiler, flag string, OS and version. Baselines are stored per fingerprint, and a mismatched comparison is **refused**, not warned about |
| Wall clock doesn't survive a change in problem size | The metric is **nanoseconds per field-cell per step**, not steps/sec |
| A single timing tells you nothing about noise | Every result is a **median over 5 runs with its interquartile range**; a run too noisy to trust is reported as *inconclusive*, never as a failure |
| A benchmark fixture drifting silently invalidates history | Each deck file is **hashed** into the output; a changed deck invalidates the comparison |
| "It got slower" isn't actionable | The gate names the **deck**, and the profiler then names the **element** |

---

## The four tools

| Tool | Question it answers | When to use it |
|---|---|---|
| **`dnf_composer_deckbench`** | *Did the simulation get slower than the recorded baseline?* | Before opening a PR that touches the step loop, convolution, or activation functions |
| **`dnf_composer_profiler`** | *Which element got slower?* | After the gate flags a deck |
| **`dnf_composer_kernelbench`** | *Is this specific kernel rewrite faster?* | While iterating on a convolution or activation kernel |
| **`dnf_composer_benchmark`** | *How does raw throughput trend over time?* | Occasionally, to append a session to the historical log |

All four are ordinary CMake targets:

```bash
cmake --build --preset release --target dnf_composer_deckbench --parallel 4
cmake --build --preset release --target dnf_composer_profiler --parallel 4
cmake --build --preset release --target dnf_composer_kernelbench --parallel 4
cmake --build --preset release --target dnf_composer_benchmark --parallel 4
```

`dnf_composer_kernelbench` needs the `benchmark` vcpkg package — `scripts/setup.sh` and `scripts/setup.bat` install it, so re-run setup if you configured the project before this was added.

**Always build Release.** A Debug perf measurement is meaningless here.

---

## The regression gate — `dnf_composer_deckbench`

### What it measures

Full `Simulation::step()` throughput over a fixed set of four committed simulation decks, reported as median ns per field-cell per step with IQR. It compares that against a baseline recorded on your machine and tells you whether anything regressed.

### The decks

The four decks come from the 606 already committed under `tests/validation/data/`, listed in `tests/benchmark/decks.json`. They are correctness fixtures first — each has a committed reference CSV — so a deck that regresses in *speed* can be handed straight to the matching validation suite to check its *numerics* didn't move too.

| Tier | Deck | Size | What it covers |
|---|---|---|---|
| `small` | `1d/.../sim_001_sigmoid_b100.json` | 100 (1D) | 1D convolution + the AVX2 sigmoid kernel |
| `medium` | `2d/.../sim_049_sigmoid_b100.json` | 50×50 | 2-term MexicanHat — two separable convolutions per step |
| `large-a` | `2d_spectral/.../golden_001_gauss_narrow.json` | 128×128 | 2D **direct** path (102 taps) |
| `large-b` | `2d_spectral/.../golden_002_gauss_wide.json` | 128×128 | 2D **spectral**/FFTW path (134 taps) |

`large-a` and `large-b` are the point of the set: the same architecture at the same grid, differing only in kernel width (5.0 vs 6.5), which is the sole reason one lands below and the other above `kFFTTapThreshold` (120). That makes the pair a controlled A/B across the dispatch boundary — a change that moves the crossover shows up as their relative timing inverting, rather than as a single number quietly drifting.

Full rationale, and the rules for changing the set, are in `tests/benchmark/DECKS.md`.

### Recording a baseline

Do this once on a known-good tree (typically `main`), before you start optimizing:

```powershell
# Windows
.\scripts\bench.ps1 .\build\release\tests\dnf_composer_deckbench.exe --record
```

```bash
# Linux / macOS
./scripts/bench.sh ./build/release/tests/dnf_composer_deckbench --record
```

This writes `tests/benchmark/baselines/<fingerprint>.json`. Baselines are committed — a repo may hold several, one per contributor machine, and each one only ever gets compared to runs from the same environment.

`--record` **refuses** to overwrite an existing baseline (pass `--force` if you mean it) and **refuses** to record at all if any deck came out noisy — a baseline recorded on a busy machine poisons every later comparison.

### Checking a change

```powershell
.\scripts\bench.ps1 .\build\release\tests\dnf_composer_deckbench.exe --check
```

```text
dnf_composer deckbench  (2000 steps x 5 runs, median)

small (1d/simulations/sim_001_sigmoid_b100.json):
  small                   877839.8 steps/s      11.39 ns/cell/step  (IQR 1.6%)

deck                         baseline        current      delta  verdict
----                         --------        -------      -----  -------
small                           11.39          11.44     +0.44%  OK
medium                          17.52          19.83    +13.19%  REGRESSED
large-a                         14.07          14.11     +0.28%  OK
large-b                          9.88           9.91     +0.30%  OK
(ns/field-cell/step, median; delta = current vs baseline; threshold 5.0%)

--check FAILED: see REGRESSED rows above.
```

### Exit codes

| Code | Meaning | What to do |
|---:|---|---|
| **0** | Every deck within threshold | Carry on |
| **1** | At least one deck regressed | Profile the named deck — see below |
| **2** | **Refused.** No baseline for this fingerprint, or a deck's hash no longer matches | Record a baseline first, or re-record deliberately if the deck change was intentional. This is not a failure — it is a comparison the tool won't fake |
| **3** | **Inconclusive.** Too noisy to conclude anything | Close what's competing for the CPU, wait a moment, re-run. A noisy run never fails the gate — a flaky gate gets ignored within a week |

Exit 3 takes precedence over exit 1: if any deck in the run was noisy, the whole run is inconclusive even if another deck looks regressed.

### The 5% threshold

`--threshold` defaults to **5%** on median `ns_per_cell_step`. That is a measured value, not a guess: across ten wrapped sessions on the reference machine, median relative spread was 1.89% and the worst single session was 3.79%, so 5% has real margin above the noise floor without being so loose it stops catching things. Override it with `--threshold 10` for a deliberately looser check.

### All flags

```text
dnf_composer_deckbench [--decks <manifest.json>] [--steps N] [--runs N]
                       [--json <out.json>] [--paths]
                       [--record [--force] | --check [--threshold PCT]]
```

| Flag | Default | Meaning |
|---|---|---|
| `--decks` | `tests/benchmark/decks.json` | Alternative deck manifest |
| `--steps` | 2000 | Timed steps per run (200 warmup steps are always discarded first) |
| `--runs` | 5 | Runs per deck; 5 is the smallest count giving a usable IQR |
| `--json` | `tests/benchmark/results/deckbench_<timestamp>_<fp>.json` | Where to write the machine-readable result |
| `--paths` | off | Verify convolution dispatch — see below |
| `--record` / `--check` | — | Baseline write / compare. Mutually exclusive |

### Verifying convolution dispatch — `--paths`

```bash
./build/release/tests/dnf_composer_deckbench --paths
```

For each `large*` deck this times the run three times — under `Auto`, `ForceDirect` and `ForceSpectral` (via `tools::math::ScopedConvolutionMode`) — and reports which path `Auto` actually took, by seeing which forced timing its own timing sits closest to:

```text
  large-a-forcedirect          ...    14.07 ns/cell/step  (IQR 0.9%)
  large-a-forcespectral        ...    21.44 ns/cell/step  (IQR 1.1%)
  large-a-auto                 ...    14.11 ns/cell/step  (IQR 1.0%)
    Auto observed: direct-2d (expected: direct-2d)
```

This is the only way to observe the dispatch decision from outside without instrumenting the library. A `*** MISMATCH ***` means either the dispatch rule in `tools::math::shouldUseSpectral2D` or the documentation in `tests/validation/data/2d_spectral/README.md` is now wrong — that finding outranks whatever the benchmark session was for.

`--paths` is ignored (with a warning) alongside `--record`/`--check`, which always compare the plain `Auto` measurement so every deck has exactly one entry.

---

## Machine hygiene — `scripts/bench.ps1` / `scripts/bench.sh`

Run every perf tool through the wrapper for your platform:

```powershell
.\scripts\bench.ps1 <exe> [args...]
```

```bash
./scripts/bench.sh <exe> [args...]
```

It pins the process to a single logical CPU, raises its priority, fixes the clock (on Windows: High Performance power plan with `PROCTHROTTLEMAX=99` to cap unpredictable turbo swings), restores everything afterwards including on Ctrl-C, and passes the tool's exit code straight through.

It also exports `DNFC_BENCH_AFFINITY`, `DNFC_BENCH_PRIORITY` and `DNFC_BENCH_POWER` so the state it achieved gets stamped into the tool's JSON output. If a step doesn't work on your system, the script warns on stderr and records what it actually managed rather than claiming a clean state it didn't reach.

The measured benefit is in the **tail**, not the typical case: across the reference measurement, median spread was essentially identical bare vs wrapped (1.90% vs 1.89%), but bare produced two sessions over 9% while the wrapped worst case was 3.79%. The wrapper removes bad runs; it does not make good runs better.

A few practical notes from real use:

- **Don't hammer `--check` in a loop** chasing a clean result. Roughly twenty back-to-back runs on a desktop CPU visibly degrade — thermal accumulation pushes IQRs into the teens on decks that were under 1% a few runs earlier. Leave gaps.
- **Never re-record a baseline to make a failing check pass.** That is deleting the evidence.
- `small` is the deck most prone to noise — it is the shortest run even at high step counts, so it is proportionally more sensitive to OS scheduling effects.

---

## Attribution — `dnf_composer_profiler --deck`

When the gate names a deck, this names the element inside it. It times each element's public `step(t, deltaT)` from outside — no library instrumentation involved — and reports each one's microseconds and share of the whole step.

```bash
# One deck
./build/release/tests/dnf_composer_profiler --deck tests/validation/data/2d/simulations/sim_049_sigmoid_b100.json --json out.json

# Every deck in a manifest
./build/release/tests/dnf_composer_profiler --decks tests/benchmark/decks.json --iters 20000   # default 5000
```

```text
medium (2d/simulations/sim_049_sigmoid_b100.json):
  mexican hat kernel 2d  mexican hat kernel 2d      69.412 us   55.8%
  neural field u         neural field 2d           52.180 us   41.9%
  gauss stimulus 2d      gauss stimulus 2d          2.903 us    2.3%
```

Read it by comparing shares, not absolutes: the element whose share grew is your suspect. With a deck *and* an element you have somewhere concrete to look, rather than "the simulation got slower."

| Flag | Default | Meaning |
|---|---|---|
| `--deck <path>` | — | Profile one committed simulation JSON |
| `--decks <manifest>` | — | Profile every deck in a manifest |
| `--iters N` | 5000 | Iterations per element |
| `--json <out>` | timestamped file | Machine-readable output |

Run with no arguments, the profiler keeps its original behaviour: a per-element-type sweep plus the hardcoded 1D/2D detection sims, appended as a session block to `tests/profiler/profile.md`.

---

## Kernel A/B — `dnf_composer_kernelbench`

Google Benchmark microbenchmarks on the hot kernels in isolation, with adaptive iteration counts and `DoNotOptimize`/`ClobberMemory` so nothing gets optimized away:

| Benchmark | Kernel |
|---|---|
| `BM_ConvValid1D` | `tools::math::conv_valid_into` — the 1D inner primitive |
| `BM_Conv2dSeparable` | `tools::math::conv2d_separable_into` — the 2D direct path |
| `BM_Conv2dSpectral` | `tools::math::SpectralConvolver2D::apply` — the FFTW path |
| `BM_SigmoidApply` | `element::SigmoidFunction::apply` — the AVX2 activation kernel |
| `BM_NeuralField2DStep` | `NeuralField2D::step` — input update, Euler integration and activation combined |

```bash
./build/release/tests/dnf_composer_kernelbench --benchmark_repetitions=5

# One benchmark family only
./build/release/tests/dnf_composer_kernelbench --benchmark_filter=BM_Conv2dSeparable

# Machine-readable
./build/release/tests/dnf_composer_kernelbench --benchmark_format=json --benchmark_out=kernels.json
```

Read the `_median` rows and ignore `_mean`. Every `Args({...})` size is chosen to match a real deck — 2500 is `medium`'s grid, 16384 is `large-a`/`large-b`'s, and the 128-grid `BM_Conv2dSeparable`/`BM_Conv2dSpectral` pair mirrors the tap counts either side of the dispatch threshold.

**This is not a substitute for the gate.** It measures kernels with hot caches and no surrounding simulation, so a win here does not automatically mean a win in `dnf_composer_deckbench`. Use it to choose between two implementations; use the gate to decide whether the change was actually worth shipping.

---

## Historical throughput — `dnf_composer_benchmark`

The long-running throughput sweep. It builds N independent fields (each a GaussStimulus + NeuralField + lateral GaussKernel + zero-amplitude NormalNoise), times Euler steps for N = 10/50/100 in both 1D (size 100) and 2D (50×50), and appends a timestamped session to `tests/benchmark/results.md` alongside a JSON sidecar in `tests/benchmark/results/`.

```bash
./build/release/tests/dnf_composer_benchmark              # 2000 steps x 5 runs
./build/release/tests/dnf_composer_benchmark 5000 9       # timed_steps, n_runs
```

> **Sessions logged before 2026-08-20 are not comparable to later ones** — the measurement changed shape (per-step duration handling, the ns/cell/step metric, the JSON sidecar). The caveat is repeated in the file's own header. Compare within an era, not across the discontinuity.

This is a trend log, not a gate. Use `dnf_composer_deckbench --check` for pass/fail.

---

## Reading the JSON output

Every tool writes the same envelope, so a result is interpretable months later without the conversation that produced it:

```json
{
  "config":      { "runs": 5, "timed_steps": 2000, "warmup_steps": 200, "...": "..." },
  "env": {
    "cpu": "AMD Ryzen 5 3600 6-Core Processor", "logical": 12,
    "compiler": "MSVC 19.44", "cxx_flags": "/DWIN32 /D_WINDOWS /EHsc /O2 /Ob2 /DNDEBUG",
    "avx2": true, "fftw": "3.3.10", "os": "Windows", "build_type": "Release",
    "dnfc_version": "2.10.1", "git": "4d74c4c2", "git_dirty": true,
    "hostname": "...", "affinity": "0x1", "priority": "high",
    "power_state": "high-performance,PROCTHROTTLEMAX=99"
  },
  "fingerprint": "d13b4022ff91ea5f",
  "results": [
    {
      "name": "small", "architecture": "detection-1d", "path": "direct-1d",
      "field_cells": 100, "deck_hash": "70f68a353e4329fa", "noisy": false,
      "ns_per_cell_step": { "median": 11.39, "q1": 11.34, "q3": 11.53,
                            "min": 11.16, "max": 11.69, "n": 9 },
      "steps_per_sec":    { "median": 877839.8, "...": "..." }
    }
  ]
}
```

Three fields to check before believing any number:

- **`git_dirty`** — `true` means the tree had uncommitted changes. The measurement may not correspond to any commit.
- **`power_state` / `affinity` / `priority`** — absent or `unrecorded` means the run was not wrapped, so it carries the wider bare noise floor.
- **`noisy`** — `true` means that deck's IQR/median exceeded 3%; the number is not trustworthy on its own.

`fingerprint` is a hash over the environment fields that affect timing. It is what baselines are keyed by, and what makes a cross-machine comparison get refused rather than silently reported.

---

## Typical workflow

```text
1. On main, once per machine:   bench.ps1 deckbench --record
2. Write your optimization.
3. Run the correctness suite:   dnf_composer_tests
4. Check the gate:              bench.ps1 deckbench --check
   → 0  ship it
   → 3  noisy — settle the machine, re-run
   → 2  baseline missing or deck changed — resolve deliberately
   → 1  regressed:
        a. profiler --deck <the named deck>   → which element
        b. kernelbench --benchmark_filter=... → iterate on the kernel
        c. back to step 4
5. Once the change lands on main, re-record the baseline so it tracks reality.
```

Step 3 is not optional: a faster simulation that computes different numbers is not an optimization. Every benchmark deck is also a validation fixture precisely so you can check both.

---

## Dashboard

`resources/benchmark/` has a Streamlit dashboard that reads every artifact the tools above produce — deckbench/baseline JSON, profiler element JSON, kernelbench's Google Benchmark output, and the two legacy markdown logs — and turns them into an Overview (latest gate verdict per deck), Trends (ns/cell/step over time, baseline and threshold overlaid), Decks (per-deck comparison and the direct/spectral dispatch crossover), Elements (per-element attribution and session-to-session deltas), Kernels, and Runs (every discovered artifact plus a hygiene checklist).

Double-click `run_dashboard.bat` (Windows) or `run_dashboard.sh` (Linux/macOS) — first run creates its own virtual environment at the repo root and installs what it needs, later runs skip straight to launching.

**It is read-only.** It never launches a perf tool itself — a benchmark measured next to a running web server and browser is a worse measurement than one taken alone. Run the tools yourself, then hit Refresh in the sidebar. See `resources/benchmark/README.md` for the one gotcha worth knowing: `--record`/`--check` write no JSON of their own, so a plain (no-flag) `dnf_composer_deckbench` run is what leaves a trend point for the dashboard to show.

---

## Troubleshooting

| Symptom | Cause | Fix |
|---|---|---|
| `--check refused: no baseline for this environment's fingerprint` | No baseline recorded for this machine/toolchain, or something in the environment changed (compiler upgrade, different flags) | `--record` on a known-good tree |
| `--check refused: ... DECK CHANGED SINCE BASELINE` | A deck file's hash no longer matches the baseline's | If the deck change was intended, `--record --force`; otherwise find out why a validation fixture was edited |
| `--record refused: <deck> is too noisy to trust` | Machine busy | Run under `scripts/bench.ps1` / `bench.sh` with nothing else competing |
| Every deck reports absurd regressions | Debug build, or a build without `/arch:AVX2` | Check `build_type` and `avx2` in the JSON `env` block |
| `--check` results drift between sessions on unchanged code | Normal between-session drift (typically within ±3%) | Re-run; if it persists past the threshold it is real |
| Missing `benchmark/benchmark.h` when building kernelbench | vcpkg `benchmark` package not installed | Re-run `scripts/setup.sh` / `scripts/setup.bat` |

---

## See also

- `resources/benchmark/README.md` — the dashboard: what it reads, how to run it
- `tests/benchmark/DECKS.md` — why those four decks, and the rules for changing them
- `tests/validation/data/2d_spectral/README.md` — tap counts and expected dispatch for the `large-a`/`large-b` pair
- [Testing](Testing.md) — the correctness suite
- [Architecture](Architecture.md) — where the step loop and convolution paths live
