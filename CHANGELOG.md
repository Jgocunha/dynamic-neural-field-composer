# Changelog

All notable changes to this project will be documented in this file.
## [2.4.1] - 2026-05-08

### Performance
- `NeuralField::updateState()` now executes a single fused O(N) pass over the activation array
  (via cached `act_` raw pointer) to compute sum, L2-norm, min, and max simultaneously, replacing
  five separate O(N) passes with five string hash-map lookups each
- `NeuralField::updateBumps()` eliminates per-step heap allocation by swapping into a persistent
  `prevBumps_` scratch buffer (`O(1)`) instead of copying the bump vector every step
- `Element::updateInput()` caches raw `double*` pointers to the own input buffer and each upstream
  element's output component at `buildInputCache()` time, removing repeated string hash-map lookups
  from the hot step path
- `ActivationFunction::applyInPlace()` overload added; `NeuralField::calculateOutput()` and all
  concrete activation functions updated to apply the nonlinearity without an extra buffer write
- `GaussKernel`, `MexicanHatKernel`, `AsymmetricGaussKernel`, `OscillatoryKernel`: convolution now
  accumulates directly into the output component, eliminating a full-field temporary copy per step

### Fixed
- `Simulation::addElement()` did not call `element->init()` after emplacing the element; adding a
  `NeuralField` (or any element with raw-pointer members assigned only in `init()`) through the GUI
  while the simulation was running left `act_`, `inp_`, and `rest_` as `nullptr`, causing an access
  violation (exit code `0xC0000005`) on the next `step()` call. Fixed by calling `element->init()`
  immediately after `elements.emplace_back(element)`
- `data/exports/` was absent from packed releases because CMake's `install(DIRECTORY ...)` silently
  skips empty directories; added a `.gitkeep` placeholder so the folder is present in all release
  archives alongside `data/simulations/` and `data/inter-field-synaptic-connections/`
- `GaussStimulus::init()` reassigned `components["input"]` with `= std::vector<double>(...)`,
  freeing the allocation that `inputPtr_` still referenced; subsequent `updateInput()` wrote
  through a dangling pointer causing heap corruption (visible as `SIGABRT` on macOS).
  Fixed by replacing the reassignment with `std::ranges::fill`
- `FieldCoupling::updateOutput()` and `GaussFieldCoupling::updateOutput()` reallocated
  `components["output"]` on every step, silently invalidating any downstream element's cached
  input pointer. Fixed by replacing both reassignments with `std::ranges::fill`
- `Element` input cache (`inputPtr_`, `cachedInputs_`) was not invalidated when inputs were
  added, removed, or when dimensions changed, risking stale or dangling pointer use.
  `inputPtr_` is now reset to `nullptr` in `addInput()`, `removeInput()` (both overloads),
  `removeInputs()`, and `changeDimensions()`
- `NeuralField::updateBumps()`: a bump that remained above threshold through the last field
  index was pushed without finalizing `endPosition`, `centroid`, or scaling `width` to spatial
  units, and without performing velocity/acceleration matching against the previous step's bumps.
  The trailing `if (inBump)` block now fully finalizes the bump before pushing it

### Added
- `Element::setComputeStateMetrics(bool)` / `Simulation::setMeasureStepDuration(bool)`:
  opt-out flags for headless batch runs where bump data and timing are never needed

### Documentation
- Doxygen inline comments added to `NeuralFieldState::previousActivationSum/Avg/Norm`
- `setComputeStateMetrics()` doc comment updated to reflect the new single-pass implementation
- `wiki/Element-Reference.md`: `AbsSigmoidFunction` added to the NeuralField activation
  functions table

### Tests
- `NeuralFieldBumps.BumpDetectedWhenActivationAboveThreshold`
- `NeuralFieldBumps.BumpCentroidNearStimulusPosition`
- `NeuralFieldBumps.BumpVelocityNonZeroWhenStimulusMoves`
- `NeuralFieldState.HighestActivationAboveRestingLevelUnderStimulus`
- `NeuralFieldAbsSigmoid.*` suite: construction, output near zero at resting level, rises under
  stimulus, and `getParameters`/`setParameters` round-trip with `AbsSigmoidFunction`

## [2.4.0] - 2026-05-08

### Added
- `CorrelatedNormalNoise` element: spatially correlated Gaussian noise via convolution of white
  noise with a normalized Gaussian kernel (parameters: `amplitude`, `width`, `circular`);
  cedar-equivalent to `NeuralField::NoiseCorrelationKernel`
- `AbsSigmoidFunction` activation function: rational sigmoid `σ(u) = 0.5·(1 + β·u / (1 + β·|u|))`,
  algebraically equivalent to cedar's default `AbsSigmoid`; enables cedar-exact simulations
- Cross-framework Doxygen equivalence tables in `activation_function.h` documenting the
  correspondence between dnf-composer, cedar, and cosivina activation functions
- `circular` checkbox exposed in the Element Control UI for `CorrelatedNormalNoise`

### Fixed
- Null-pointer dereference in `SimulationFileManager::jsonToElements()`: `activationFunction`
  now defaults to `SigmoidFunction(0.0, 10.0)` when the JSON field is absent or the type is
  unrecognised, preventing a crash on malformed simulation files
- `CorrelatedNormalNoise::init()` now clamps `width` to a minimum of `1e-3` to prevent
  NaN/inf when `width` is zero or near-zero
- Duplicate `install(FILES ...)` rule for `correlated_normal_noise.h` removed from
  `CMakeLists.txt`

### Tests
- Unit tests for `CorrelatedNormalNoise`: output size, zero amplitude, non-zero amplitude,
  circular vs. linear convolution, and parameter round-trip via `setParameters`/`getParameters`
- Unit tests for `AbsSigmoidFunction`: monotonicity, fixed point at `u=x_shift`, saturation
  at high beta, output-size invariance, and equality operator
- `NeuralField` integration tests for the AbsSigmoid activation path (detection and memory
  instability scenarios)
- Corrected `AbsSigmoidFunction` test: replaced the incorrect assumption that AbsSigmoid and
  logistic sigmoid agree to `< 0.001` at `beta=50` (they do not — they are different function
  families) with a test verifying their shared properties (equal at origin, same asymptotes)

## [2.3.1] - 2026-04-30

### Added
- `CONTRIBUTING.md` — dev setup, code style (Clean Code principles), test/Doxygen/wiki
  expectations, PR checklist, and release process
- GitHub issue templates for bug reports and feature requests
- GitHub PR template

### Build
- clang-tidy static analysis: new `.github/workflows/static-analysis.yml` job runs
  `run-clang-tidy` on Ubuntu on every push and PR to `main`; `.clang-tidy` config enables
  `bugprone-*`, `modernize-*`, `readability-*`, `clang-analyzer-*`, and `performance-*`
- `CMAKE_EXPORT_COMPILE_COMMANDS ON` added to `CMakeLists.txt` to produce
  `compile_commands.json` on Unix generators (required by clang-tidy)
- macOS release artifact is now `macos-arm64.tar.gz` (Apple Silicon only); Intel (`macos-13`)
  runners were dropped due to GitHub Actions deprecation and chronic queue exhaustion
- PVS-Studio licence header comments removed from all source files

### Documentation
- README Static Analysis badge added
- README Contributing section links to `CONTRIBUTING.md`

## [2.3.0] - 2026-04-30

### Added
- macOS support: builds on Apple Silicon (`arm64-osx`) and Intel (`x64-osx`) via a new
  `build_macos.sh` script that auto-detects architecture, installs vcpkg dependencies, and
  builds imgui-platform-kit from source
- CI job `build-and-test-macos` on `macos-latest` covering configure, build, and test
- Release workflow `release-macos` job producing
  `dynamic-neural-field-composer-<version>-macos-<arch>.tar.gz` artifacts

### Fixed
- `getResourceRoot()` on macOS now uses `_NSGetExecutablePath` (with dynamic-buffer retry)
  instead of the Linux-only `readlink("/proc/self/exe")`
- `exportComponentToFile` timestamp replaced `std::chrono::zoned_time` / `std::format`
  (unsupported on Apple Clang) with portable `localtime` / `put_time`
- `LogWindow::addLog` annotation corrected from `IM_FMTARGS(3)` to `IM_FMTARGS(2)` —
  Apple Clang rejects the wrong index for `static` functions
- `find_package(OpenGL REQUIRED)` added before `find_package(imgui-platform-kit)` so the
  `OpenGL::GL` target is defined when CMake resolves the kit's exported link interface

### Build
- `catch2` added to vcpkg install in all CI and release jobs (required by imgui-platform-kit)
- vcpkg cache key for the macOS CI job now includes the resolved triplet to prevent
  cross-architecture cache pollution

### Documentation
- README and wiki Getting Started updated for macOS prerequisites, build steps, and binary paths
- GCC minimum version aligned to `GCC 11+` across README and wiki

## [2.2.0] - 2026-04-29

### Added
- `AsymmetricGaussKernel` now supports cross-dimension output via the optional
  `outputFieldDimensions` parameter, matching the capability already present in
  `GaussKernel`, `MexicanHatKernel`, and `OscillatoryKernel`
- Element Control UI now exposes **Size** and **Step** dimension controls for all
  element types, and **Output Size** / **Output Step** controls for all kernel types,
  enabling runtime resizing and cross-dimension kernel configuration without recompiling
- 7 new cross-dimension tests for `AsymmetricGaussKernel` in
  `test_kernel_cross_dimension.cpp`

### Documentation
- Doxygen class-doc blocks for all four kernel headers updated to describe
  cross-dimension resampling behaviour
- Wiki `Element-Reference`: `outputFieldDimensions` documented in all four kernel
  parameter tables; new **Cross-dimension kernels** section added
- Wiki `Elements`: `changeDimensions()` added to the Element base class interface
- Wiki `Application-and-UI`: `ElementWindow` description updated to cover
  dimension and output-dimension controls
  
## [2.1.2] - 2026-04-28

### Fixed
- File dialogs and coupling weight loaders now resolve to the correct runtime path
  (`<install-dir>/data/`) instead of the compile-time source path baked in at build time

### Build
- Example executables (`ex_*`) included in the release `bin/` folder
- `data/` folder (simulation JSONs and coupling weight files) included in the release package
- Release archive description updated to reflect new contents
