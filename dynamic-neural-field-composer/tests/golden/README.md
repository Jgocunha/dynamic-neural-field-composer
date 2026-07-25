# Golden algebraic-equivalence test suite

**Goal.** Freeze the *numerical behaviour* of every DNF element so that any future
code change that alters an element's dynamics — its output for a fixed input and
parameter set — makes a test **fail**. These tests are a regression net against
silent changes to the maths ("algebraic equivalence to a golden reference").

## Two nets (both in `golden_test_utils.h`)

1. **Analytic equivalence** — `checkAgainstReference(name, production, reference)`.
   For every element with a closed-form definition, re-derive the maths
   **independently** in `reference/*.h` (the *golden implementation reference*)
   and assert the production element reproduces it element-wise to `kTol = 1e-9`.
   The reference must NOT call the production maths — re-express it from first
   principles (see `reference/ref_gauss_stimulus.h` for the exemplar).

2. **Frozen golden data** — the CSV side of `checkAgainstReference`, and
   `checkCharacterization(name, production)` for composed architectures where no
   closed form is practical. Deterministic outputs/trajectories are serialised
   **once** to `data/<name>.csv` and committed. Later runs load + compare.

Both fire together for analytic elements; architectures use characterization only.

## Conventions

- **Golden name** = a slug → `data/<slug>.csv`. Namespace it by element +
  dimensionality + regime, e.g. `gauss_kernel_2d_circular_s3_amp1`.
- **2D fields**: stored y-major, flattened to one CSV row (`field[y*size_x + x]`).
- **Trajectories**: one simulation step per CSV row (use `captureTrajectory`).
- **Determinism**: fix `dt`, step count, positions; no noise unless seeded.
- **Sweep regimes**, don't test a single point: vary sigma, position (incl.
  boundary/edge), circular vs non-circular, normalized, field size, dt, tau.
- Windows `min`/`max` macros bite — every `reference/*.h` must `#undef min/max`
  after its includes (see the exemplar).

## Regenerating golden data

```
DNF_UPDATE_GOLDEN=1  build/release/tests/dnf_composer_tests.exe --gtest_filter=Golden*
```
Missing CSVs are always captured; existing ones are overwritten **only** in
update mode. A diff in `data/` means dynamics changed — investigate, never
regenerate blindly to turn a red test green. Commit the CSVs.

## Build & run (Windows, MSVC + Ninja + vcpkg)

From PowerShell, in your worktree:
```powershell
$vs = "C:\Program Files\Microsoft Visual Studio\2022\Community"
Import-Module "$vs\Common7\Tools\Microsoft.VisualStudio.DevShell.dll"
Enter-VsDevShell -VsInstallPath $vs -DevCmdArguments '-arch=x64 -host_arch=x64' -SkipAutomaticLocation | Out-Null
$env:VCPKG_ROOT = "C:\dev-files\vcpkg"      # the real classic-mode cache (has imgui/implot);
                                            # the VS-bundled $vs\VC\vcpkg does NOT and fails find_package
cmake -S . -B build/release                 # reconfigure after adding a NEW .cpp
cmake --build build/release --target dnf_composer_tests
build\release\tests\dnf_composer_tests.exe --gtest_filter="Golden*"
```
`tests/golden/*.cpp` is auto-globbed into the `dnf_composer_tests` target
(`CONFIGURE_DEPENDS`) — reconfigure once when you add a new file; incremental
builds after that are fast. `GOLDEN_DATA_DIR` is injected by CMake.

## File layout & ownership (parallel build — keep files disjoint!)

```
tests/golden/
  README.md                       # this file 
  golden_test_utils.h             # shared harness 
  reference/                      # independent golden implementations
    ref_gauss_stimulus.h          # exemplar
    ref_<family>.h                # per agent
  data/                           # committed frozen CSVs
  test_golden_gauss_stimulus.cpp  # exemplar 
  test_golden_<family>.cpp        # per agent
```

Each agent owns **disjoint files** (own `reference/ref_*.h` + own
`test_golden_*.cpp`). Do not edit `golden_test_utils.h`, `CMakeLists.txt`, this
README, or another agent's files — propose harness changes to god.

### Coverage matrix (every element, 1D + 2D)

- **Agent A — Stimuli, Noise, Reshape**
  `gauss_stimulus_2d`, `boost_stimulus(_2d)`, `timed_gauss_stimulus(_2d)`,
  `normal_noise(_2d)`, `correlated_normal_noise(_2d)` (statistical golden —
  inspect the RNG/seed API first), `resize(_2d)`, `collapse`, `expand`.
  (1D `gauss_stimulus` is the exemplar — extend, don't duplicate.)
- **Agent B — Kernels & Couplings**
  `gauss_kernel(_2d)`, `mexican_hat_kernel(_2d)`, `oscillatory_kernel(_2d)`,
  `asymmetric_gauss_kernel(_2d)`, `field_coupling`, `gauss_field_coupling`.
  Kernels convolve their input — reference the kernel weights AND the
  convolution output on a fixed probe input.
- **Agent C — Fields, Dynamics & Architectures**
  `neural_field(_2d)` (independent Euler integration of the Amari equation over
  a trajectory), `memory_trace(_2d)`, `activation_function` (sigmoid /
  abs-sigmoid pointwise), and **composed architectures** (characterization):
  field+stimulus, field+self-excitation Gauss kernel → stable bump,
  field+Mexican-hat → selection, two coupled fields, memory-trace formation —
  capture multi-step trajectories.

All golden tests use the `Golden<Thing>` GTest suite prefix so `--gtest_filter=Golden*`
runs the whole suite.
