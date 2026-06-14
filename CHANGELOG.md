# Changelog

All notable changes to this project will be documented in this file.

## [2.9.3] - 2026-06-14

### Changed
- The C++20 requirement is now attached to the library target via
  `target_compile_features(dynamic-neural-field-composer PUBLIC cxx_std_20)` instead of the
  global `CMAKE_CXX_STANDARD` variables, so it propagates to tests, examples, and downstream
  consumers of the installed/exported target (#78)
- Replaced `std::endl` with `'\n'` in `tools/logger.cpp` and `tools/profiling.cpp` to avoid
  unnecessary per-line stream flushes on the hot logging/profiling paths (#73, #79)

## [2.9.2] - 2026-06-07

### Fixed
- Duplicate element names in a `.dnf` file are now rejected on load
  (`SimulationFileManager::jsonToElements`): the first occurrence is kept and later
  duplicates — along with their interactions — are skipped with a clear ERROR, instead of the
  duplicate's input edges being silently wired onto the first element and corrupting the loaded
  graph (#44)

## [2.9.1] - 2026-06-07

### Changed
- Legacy `toString()` string builders in the parameter and plot-parameter classes
  (`GaussKernelParameters`, `GaussStimulusParameters`, `MexicanHatKernelParameters`,
  `NeuralFieldParameters` / `NeuralFieldBump` / `NeuralFieldState`, `PlotDimensions`,
  `PlotAnnotations`, `PlotCommonParameters`) now use C++20 `std::format` instead of
  `+= std::to_string(...)` chains, with `{:.2f}` precision for floating-point fields (#62, #64)

### Fixed
- Removed two unreachable `return` statements left after the `std::format` refactor in
  `NeuralFieldParameters::toString()` and `NeuralFieldBump::toString()` that referenced
  deleted locals and broke compilation
- Removed the illegal/redundant `override` specifier from the out-of-line
  `GaussKernelParameters::toString()` and `GaussStimulusParameters::toString()` definitions
  (`override` is only valid on the in-class declaration, which already has it)

## [2.9.0] - 2026-06-03

### Added
- **`Collapse` element** (2D→1D) — reduces a 2D field along one axis to a 1D output
  using a selectable compression (`sum` / `average` / `maximum` / `minimum`) and a
  selectable kept axis (X or Y). Lets a 2D field's marginal drive a 1D field
- **`Expand` element** (1D→2D) — broadcasts a 1D profile into a 2D output (a "ridge"),
  repeating along the chosen axis. Lets a 1D feature field drive a 2D map
- Both elements are single-input and integrate across the suite: factory registration
  (`COLLAPSE`, `EXPAND` labels), `SimulationWindow` add-element cards, `ElementWindow`
  editable **Input dimensions** / **Output dimensions** sections, and `NodeGraphWindow`
  inspector entries. Added `example_dimensionality_collapse_expand` (four mixed-dimensionality
  models, some chained with `Resize`/`Resize2D`) plus `test_collapse` / `test_expand`
- Element "type" badges/categories across the Element, Remove, and Log-parameters panels
  now resolve through a single shared `ElementCategory` table (`element_parameters.h`);
  `Resize`/`Resize2D`/`Collapse`/`Expand` no longer show as "Unknown"

### Fixed
- **Save/load crash for reshape elements** — `Resize`, `Resize2D`, `Collapse`, and
  `Expand` were never serialized by `SimulationFileManager`: saving wrote them without
  parameters and loading dropped them, corrupting (and crashing) any architecture that
  used them. They now round-trip fully (including input dimensions). Loading is resilient
  to older/hand-edited `.dnf` files missing keys, and interactions referencing an
  uncreated element are skipped with a warning instead of producing a half-wired graph
- `Collapse`/`Expand` now reject inputs of the wrong dimensionality (Collapse requires
  2D, Expand 1D) and a source size that doesn't match the kept/profile axis, and throw
  on a mismatched output/input size at configuration time — instead of silently producing
  a truncated or stretched result. `changeInputDimensions()` severs connections before
  resizing to avoid a stale/dangling input cache
- Hardened the 2D reduce/broadcast math helpers against non-positive dimensions and an
  undersized field buffer (no over-allocation or out-of-bounds access)

### Documentation
- Wiki element-suite pages updated for `Collapse` / `Expand` (`Element-Reference`,
  `Elements`, `Examples`); added a mandatory **JSON serialization** step to
  *How to Add New Elements* so new elements are saved/loaded (the gap behind the crash)

## [2.8.0] - 2026-06-03

### Added
- **`Resize` / `Resize2D` elements** — resample an input field of spatial size N to a
  user-specified size M via `linear`, `nearest`, or `cubic` interpolation, bridging
  neural fields that operate at different spatial resolutions. Single-input by design
  (additional inputs are rejected to keep the input buffer consistent)
- Resize integration across the suite: factory registration (`RESIZE`, `RESIZE_2D`
  labels), `SimulationWindow` add-element cards, `ElementWindow` editable **Input
  dimensions** / **Output dimensions** sections, and `NodeGraphWindow` inspector entries
- `example_resize` example demonstrating both 1D and 2D resampling architectures
  (stimulus → field u → kernel u-u → resize u-v → field v, with v at a different size),
  plus `test_resize` and `test_resize_2d` unit tests

### Removed
- Deprecated cross-dimension kernel capability: the optional `outputFieldDimensions`
  parameter on `GaussKernel`, `MexicanHatKernel`, `OscillatoryKernel`, and
  `AsymmetricGaussKernel` (and the associated **Output Size** / **Output Step** UI
  controls) has been removed. Use the standalone `Resize` / `Resize2D` elements to
  resample between neural fields of different spatial sizes

### Documentation
- Wiki element-suite pages updated for the new elements: documented `Resize` / `Resize2D`
  in `Element-Reference`, `Elements`, and `Examples`; updated `Application-and-UI` to
  describe the Resize input/output dimension controls; removed the `outputFieldDimensions`
  **Cross-dimension kernels** section

## [2.7.1] - 2026-06-01

### Documentation
- README overhaul: pre-compiled binary download instructions, a section listing projects and
  publications that use dnf-composer, and refreshed headline images
- Wiki accuracy pass: removed the non-existent `UIMode` description from the architecture page,
  corrected the claim that `Simulation::save()`/`read()` open a file dialog (they do not — an empty
  path falls back to the `data/` directory; the picker is provided by the GUI menu bar), and
  reframed `ElementFactory` as one of two construction paths alongside direct `std::make_shared`
- Documented `imgui.ini` (dynamic-layout persistence and how to reset it), the
  `style_light_green_accent.json` theme file and how to personalize it, and that `.dnf` files are
  plain, hand-editable JSON
- Example guide now follows the `example_<name>` target naming convention used by the built-in
  examples
- Added low-dependency / easy-build-from-source emphasis to the home and getting-started pages

### Changed
- `HelpWindow` content and navigation layout refreshed; tip rendering simplified to a single
  variadic helper

### Fixed
- Resolved `-Wformat-security` warnings on GCC/Clang by passing runtime strings as `"%s"` arguments
  in `Logger::log_ui`, `LogWindow::addLog` call sites, and `FileDialog` error text
- Corrected format specifiers in logging and error messages

### Build
- Compressed bundled image assets (headline and background images, logo) and removed unused style
  and image resources, significantly reducing repository size

## [2.7.0] - 2026-05-31

### Added
- **`SimulationRecorder`** — records element output components to timestamped CSV files with
  configurable sampling interval (ticks or milliseconds); supports `startRecording()`,
  `stopRecording()`, `stopAll()`, and `takeSnapshot()` for single-frame exports
- **Simulation recording UI** — `SimulationWindow` exposes recording controls: element/component
  selector, interval configuration, and start/stop buttons
- 2D CSV metadata: a `# size_x=N,size_y=M` comment line is prepended to CSV files produced for
  2D elements so downstream readers can reconstruct the grid layout
- New simulation configuration: weighted field couplings example (`.dnf` format)
- `scripts/setup.bat` and `scripts/setup.sh` automate vcpkg bootstrapping, package installation,
  and imgui-platform-kit build+install on a fresh machine
- `scripts/README.md` documenting all setup, build, and install scripts with platform notes

### Changed
- **Simulation file format** renamed from `.json` to `.dnf`; directory structure changed to
  `data/simulations/<name>/<name>.dnf` (one folder per simulation)
- All build and install scripts moved from the project root into `scripts/`
- imgui-platform-kit now built twice — separate `build-release` / `build-debug` directories each
  with their own `CMAKE_INSTALL_PREFIX` — to prevent CRT mismatch between Release and Debug
  configurations on Windows

### Build
- Windows CI: `gtest_discover_tests` uses `DISCOVERY_MODE PRE_TEST` (CMake 3.18+) to defer test
  binary execution to CTest run time, avoiding the `_NOT_BUILT` sentinel when vcpkg DLLs are absent
  from PATH at build time
- Windows CI test step prepends `C:\Program Files\CMake\bin` to PATH so cmake 3.31 is used for
  GTest discovery (Strawberry Perl ships cmake 3.29, which cannot run the generated discovery
  scripts that require CMake 3.30+)

### Tests
- `SimulationRecorderState`, `SimulationRecorderFile`, `SimulationRecorderSampling`,
  `SimulationRecorderSnapshot`, and `SimulationRecorderTicks` suites covering recording
  lifecycle, CSV structure, tick-interval sampling, stop semantics, and snapshot export
- `test_simulation_file_manager.cpp` updated for the `.dnf` folder-based file structure

## [2.6.0] - 2026-05-28

### Added
- **`ControlBarWindow`** and **`StatusBarWindow`** — persistent playback controls and simulation
  status strip displayed at all times
- **`StaticLayoutWindow`** — fixed panel layout integrating node graph, control bar, status bar,
  and log window into a single coherent workspace
- **`HelpWindow`** — in-app help overlay accessible from the menu bar
- NodeGraphWindow: mini-map, node-overlap prevention during drag, improved pin interaction and
  connection logic, flag to prevent bringing the window to front on focus
- Dynamic ImGui style applied from a JSON configuration file at startup; new modern light-theme
  style configuration added
- Memory usage display in the status bar (`getProcessMemoryMb`)
- New examples: travelling bump (1D/2D), boost detection (1D/2D), selection instability (1D/2D),
  memory trace (1D/2D), Hebbian learning, weighted field couplings, multi-peak (1D/2D), Gaussian
  field coupling, timed stimuli (1D/2D)

### Fixed
- `GaussFieldCoupling` double-click heatmap was vertically mirrored relative to the
  visualization-window and inline node-card views; corrected `ImPlot::PlotHeatmap` bounds to
  `ImPlotPoint(0, rows), ImPlotPoint(cols, 0)` so input index 0 appears at the bottom in all
  three views
- `GaussFieldCoupling` coupling table now shows column headers ("x in", "x out", "amp", "width")
  and an outer border, making coupling parameters immediately identifiable
- Coupling table button column was oversized due to double-counted `CellPadding.x`; fixed by
  removing the redundant term from the fixed column width calculation
- Y-major 2D indexing standardised across `GaussStimulus2D`, all 2D kernel elements, and
  `tools::conv2d`; unit tests updated to match the corrected `index = yi * size_x + xi` convention
- `Simulation::renameElement` silent early-returns replaced with `WARNING` log messages so UI
  rename failures are visible to users; successful renames now emit an `INFO` log
- `style.cpp` exception thrown when the JSON style file cannot be opened now includes the failing
  path as `errorElement` for easier diagnosis
- Missing `#include <cstring>` in `element_window.cpp` caused `std::strncpy` build error on
  GCC / Linux
- `imgui.ini` added to `.gitignore` to prevent tracking of per-machine UI layout state

## [2.5.0] - 2026-05-18

### Added
- **2D element stack**: `NeuralField2D`, `GaussStimulus2D`, `GaussKernel2D`,
  `MexicanHatKernel2D`, `OscillatoryKernel2D`, `AsymmetricGaussKernel2D`,
  `CorrelatedNormalNoise2D`, `NormalNoise2D`, `BoostStimulus2D`, `MemoryTrace2D`,
  `TimedGaussStimulus2D` — full 2D DNF stack mirroring the 1D API
- **`FieldProjection` element**: projects a 2D field onto one axis (row or column
  sum/mean) to produce a 1D output; UI and JSON serialisation included
- **`FieldCoupling::addInput()`**: runtime method to wire a new upstream source into a
  FieldCoupling element; regression test added
- `all-elements.json` reference simulation covering every element type
- Heatmap colormap selector and scale controls for 2D field rendering in NodeGraphWindow
- Adaptive row/column layout in NodeGraphWindow for multi-element displays

### Fixed
- `ElementDimensions` default constructor did not assign `dimensionality`; derived sizes
  were computed before the field was set, leaving objects in an inconsistent state
- Static analysis (`clang-tidy`) violations resolved:
  - `[[nodiscard]]` added to `Exception::what/getErrorCode/getErrorMessage` and all
    `toString()` declarations in `element_parameters.h`
  - `ErrorCode` base narrowed from `int` to `std::uint8_t`
  - Braces added to all bare `if`/`for`/`while` bodies in `simulation.cpp` and
    `main_menu_bar.cpp`
  - C-style `char[]` buffers replaced with `std::array<char, N>` in `main_menu_bar.cpp`
    and `lineplot.cpp`
  - Narrowing `size_t → double` in `LinePlot::render()` made explicit via `static_cast`
- Missing `#include <cstring>` in `node_graph_window.cpp` (caused `std::strcmp` build error)
- `sizeof(file_dialog_buffer)` returned pointer size (8) instead of buffer size (500);
  fixed by using `path.size()` directly in the `FileDialog::ShowFileDialog` call
- `OscillatoryKernel2DParameters`: `normalized` default corrected to `true`

### Tests
- Regression: `FieldCoupling::addInput` preserves learning activity after runtime input addition

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
