# Test-Coverage Roadmap

Actionable plan for [issue #55](https://github.com/Jgocunha/dynamic-neural-field-composer/issues/55):
make the coverage number honest, then raise real coverage where it matters.
Each numbered doc is a self-contained work package — a future session can pick up
any unchecked box cold.

All paths below are relative to the nested project root
(`dynamic-neural-field-composer/`) unless prefixed with `<repo-root>/`.

## The picture (as of 2026-07, v2.1.1)

Headline Codecov figure: **45.25%** (6777 / 14976 lines). That number is misleading:

| Bucket | Lines | Covered | Why |
|---|---|---|---|
| `examples/` (21 demo `main()`s) | ~1047 | 0% | Interactive demos; never meant to be unit-tested, but sit in the denominator (~7% penalty) |
| GUI `src/` (`user_interface/`, `application/`, `visualization/`) | large | low | Needs a rendering context; only headless slices are testable |
| Core (`elements/`, `simulation/`, `tools/`, `exceptions/`) | large | ~97% | Genuinely well tested |

So there are two independent jobs:

1. **Reporting** — stop counting code that cannot be unit-tested → [01-coverage-reporting.md](01-coverage-reporting.md)
2. **Real gaps** — the core has a few genuinely unverified areas, headlined by
   field-metrics math → [02-field-metrics.md](02-field-metrics.md),
   [03-math-and-tools.md](03-math-and-tools.md),
   [04-simulation-and-exceptions.md](04-simulation-and-exceptions.md),
   [05-gui-headless.md](05-gui-headless.md)

## Priority order

1. **[01-coverage-reporting.md](01-coverage-reporting.md)** — fast, high signal. codecov.yml + gcovr excludes.
2. **[02-field-metrics.md](02-field-metrics.md)** — the issue's headline test gap. 2D bump metrics are *completely* unverified; 1D has untested branches (wrap-around, end-of-field, acceleration).
3. **[03-math-and-tools.md](03-math-and-tools.md)** — core numerics with zero tests: 2D separable convolution, resampling family, learning rules, `Timer`.
4. **[04-simulation-and-exceptions.md](04-simulation-and-exceptions.md)** — `renameElement`, timing accessors, recorder branches, missing `ErrorCode` cases.
5. **[05-gui-headless.md](05-gui-headless.md)** — `Visualization` plot management is public, pure logic, testable today; plus documented extraction candidates for later.

## Test infrastructure facts

- **Framework:** GoogleTest. One executable, `dnf_composer_tests`, listing every
  test `.cpp` explicitly in `tests/CMakeLists.txt` (new files must be added there).
  Registered with CTest via `gtest_discover_tests(... DISCOVERY_MODE PRE_TEST)`.
- **Coverage job:** `.github/workflows/ci.yml` `build-and-test-linux` job, Debug
  matrix entry with `coverage: true`. gcovr invocation ~lines 105–118, Codecov
  upload ~lines 120–127. Runs only on `push`.
- **Running locally (Windows):** configure/build via `scripts/build.bat` (Release)
  or a CMake preset dir under `build/`, then `ctest` in the build dir, or run
  `dnf_composer_tests.exe --gtest_filter=Suite.*` directly.

## Conventions to follow (learned from the existing suite)

- **Approx comparisons:** `EXPECT_NEAR(value, expected, tol)` with an explicit
  tolerance (`1e-9`/`1e-12` for pure math, looser for field dynamics). For vectors,
  `dnf_composer::tools::math::compareVectors(a, b, threshold)` is available.
- **File-local factories, not shared headers:** each test file defines its own
  small `makeField`/`makeStimulus`/`makeNFP` helpers (see
  `tests/elements/test_neural_field.cpp:18-38`,
  `tests/simulation/test_simulation_file_manager.cpp:47-82`). Copy the pattern;
  don't introduce a shared test header unless duplication becomes silly.
- **Temp dirs for file I/O:** fixture with `SetUp()`/`TearDown()` creating a
  per-test dir under `fs::temp_directory_path()` — pattern at
  `tests/simulation/test_simulation_file_manager.cpp:85-103`. The recorder tests
  instead clean `data/<simId>/` under `tools::utils::getResourceRoot()`
  (`tests/simulation/test_simulation_recorder.cpp:58-63`).
- **Test data fixtures:** `.dnf` scenario files live in `data/` and are addressed
  via the compile-time `OUTPUT_DIRECTORY` define (root `CMakeLists.txt:45-48`).
- **Headless GUI rule:** never call anything that reaches `ImGui::*`/OpenGL —
  no `Application::init()/step()/close()`, no `Plot::render()`,
  no `Visualization::render()/renderTile()`. Constructors, accessors, and the
  plot-management API are safe (see [05-gui-headless.md](05-gui-headless.md)).
- **Field-dynamics tests:** drive a `Simulation` with a `GaussStimulus` wired via
  `createInteraction`, step ~200x to converge, then assert on state. A
  `GaussStimulusParameters{sigma, amplitude, position, circular, normalized}`
  amplitude of 30 at sigma 5 reliably produces a suprathreshold bump on a
  100-unit field with resting level −5.

## Definition of done (per doc)

Every checked box means: test written, added to `tests/CMakeLists.txt` if a new
file, builds warning-clean, passes locally and in CI, and the assertion would
actually fail if the covered logic regressed (no `EXPECT_NO_THROW`-only tests
for logic that computes values).
