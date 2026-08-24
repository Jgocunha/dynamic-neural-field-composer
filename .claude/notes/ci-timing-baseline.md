# CI timing: where the time goes, and how to re-measure it

Baseline captured 2026-08-24 from run `32710766161` (push to `main`, all green), before the
`feat/optimizing-ci` changes. `CI` + `Static Analysis` ran in parallel, so end-to-end PR
feedback was governed by the slower of the two (~19 min for `CI`).

| Job | Total | Dominant steps |
|---|---:|---|
| `build-and-test-windows` | 1141s | Setup deps 135s, **Build 921s** (Release + Debug, serial), Test 41s |
| `sanitizers-linux (tsan)` | 694s | Build 182s, **Test 459s** |
| `sanitizers-linux (asan-ubsan)` | 652s | Build 256s, **Test 336s** |
| `build-and-test-linux (Debug, coverage)` | 499s | **Build 264s**, Test 162s |
| `build-and-test-linux (Release)` | 450s | Build 208s, Test 26s, **Smoke-run 146s** |
| `build-and-test-macos` | 227s | Build 147s, Test 41s |
| `clang-tidy` (Static Analysis) | 528s | **run-clang-tidy 463s** |

vcpkg caching already worked well pre-change (cache restore 2-3s, `setup.sh`/`.bat` 20-30s
once IPK is cached) — it was never the bottleneck. The time was almost entirely
**compilation** and **sanitizer test execution**.

## Root causes found

1. `scripts/build.bat` configured and built **both** `x64-release` and `x64-debug`
   serially, but CI only ever tested `x64-release` — the Debug half (~half of 921s) was
   dead weight sitting on the critical path.
2. No CMake option gated examples/benchmarks. `examples/CMakeLists.txt` registers 26
   executables and `tests/CMakeLists.txt` adds 4 more; the sanitizer and coverage jobs ran
   only `ctest` but still built all 30 of them.
3. Both `sanitizers-linux` matrix entries compiled at the CMake `Debug` default (`-O0`).
   Unoptimized code under TSan/ASan runs several times slower — this is most of the 459s
   TSan test step.
4. No compiler cache anywhere — every job recompiled the whole tree from scratch on every
   push.
5. No `concurrency` group on any workflow — a re-push to a PR let the superseded run keep
   consuming runners instead of being cancelled.
6. The example smoke-run looped over all 26 binaries serially under `xvfb-run`.
7. `run-clang-tidy` in `static-analysis.yml` analysed the whole `src/` tree in one
   sequential process.

## How to re-measure

Per-job step timings for the most recent `ci.yml` run:

```bash
gh run list --workflow=ci.yml --limit 1 --json databaseId --jq '.[0].databaseId' \
  | xargs -I{} gh api repos/{owner}/{repo}/actions/runs/{}/jobs \
    --jq '.jobs[] | "=== \(.name) \((.completed_at|fromdate)-(.started_at|fromdate))s",
          (.steps[] | select(.completed_at != null) |
           "   \(((.completed_at|fromdate)-(.started_at|fromdate)))s \(.name)")'
```

Swap `ci.yml` for `static-analysis.yml` for the clang-tidy shards. `fromdate` needs each
run's jobs to have both `started_at` and `completed_at` set (in-progress runs will error on
the missing field — wait for completion or drop steps with a null `completed_at`).

See `.claude/plans/` history (or the `feat/optimizing-ci` PR description) for the fixes
applied against this baseline: `--target dnf_composer_tests` on jobs that only run `ctest`,
`-O1` sanitizer builds, a parallel example smoke-run, splitting the Windows job into
parallel Release/Debug legs, `sccache` compiler caching (job-level `CMAKE_*_COMPILER_LAUNCHER`
env vars, read by CMake on first configure — no script changes needed except the Windows
Debug `/Z7` switch, since sccache can't cache MSVC's default `/Zi`), and a 3-way clang-tidy
shard.
