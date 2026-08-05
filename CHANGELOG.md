# Changelog

All notable changes to this project will be documented in this file.

## [Unreleased]

### Fixed
- `Element::addInput()` inferred whether an element manages its own input-shape
  validation from a buffer-length heuristic (`getComponentPtr("input")->size() ==
  getSize()`), which wrongly rejected valid dimension-bridging connections whenever
  the source's flattened size coincided with the target's own size — e.g. a
  degenerate 2D `5x1` source collapsing into a 1D size-5 `Collapse` output, or a 1D
  size-5 source broadcast into a degenerate 2D `5x1` `Expand` output. Elements that
  size their own "input" component from their own parameters (`Collapse`, `Expand`,
  `Resize`, `Resize2D`, `FieldCoupling`, `GaussFieldCoupling`) now declare that
  explicitly via a new `Element::bridgesDimensions()` virtual instead, so the
  dimensionality/shape check is skipped only when the element positively opts in,
  never inferred from a coincidental buffer size

## [2.10.0] - 2026-08-05

### Fixed
- `NodeGraphWindow` created an imgui-node-editor context in its constructor but never
  destroyed it — the destructor was `= default` and nothing in the codebase called
  `ImNodeEditor::DestroyEditor()`. `EditorContext::~EditorContext` is what deletes every
  node, pin and link object the context owns and frees its splitter memory, so skipping
  it leaked the whole graph, once per window built. Because every File→Open rebuilds the
  window set, the leak grew with each reopened simulation. The context is now held in a
  `unique_ptr` with a `DestroyEditor` deleter, so it is released on every exit path
  rather than depending on a destructor body remembering to do it (#115)
- Connected elements were never destroyed. `addInput()` records a connection on both
  endpoints — the consumer keeps its source in `inputs`, the source keeps the consumer
  in `outputs` — so any two connected elements held `shared_ptr`s to each other. Neither
  `Simulation::clean()` (which only cleared its own vector) nor destroying the
  `Simulation` broke that cycle, so every connected element in every simulation leaked;
  the canonical field↔self-kernel architecture leaked doubly. `clean()` and
  `~Simulation()` now sever all connections before releasing the elements (#112)
- Two unguarded out-of-bounds reads (#121). A heatmap in manual-dimension mode
  computed `rows = y_max / y_step`, `cols = x_max / x_step` from user-editable axis
  fields and handed `rows * cols` to `ImPlot::PlotHeatmap` with no check against the
  actual data size, so a mismatched setting read past the buffer; columns are now
  clamped so `rows * cols` fits the data (rows, and the configured aspect ratio, are
  preserved) and a warning is logged on change. `obtainCircularVector` /
  `obtainCircularVector_into` indexed `contents[indices[i] - 1]` with no check, so an
  index of `0` read `contents[-1]`; both now validate the whole index set once per
  call — off the per-element copy loop, to keep the convolution hot path
  vectorizable — and return zeros with a logged error instead of reading out of bounds
- Every `ElementFactory` creator lambda (~30 of them) did
  `dynamic_cast<const XParameters*>(&elementSpecificParameters)` and immediately
  dereferenced the result without a null check; passing the wrong
  `ElementSpecificParameters` subtype for a given `ElementLabel` was undefined
  behavior in the public element-creation API. `createElement` also returned
  `nullptr` for an unregistered/unknown `ElementLabel` instead of throwing,
  pushing a null check onto every caller. Both `createElement` overloads now
  throw a descriptive `Exception` on a parameter-type mismatch or an unknown
  `ElementLabel`, in place of the dynamic_cast dereference and the nullptr
  returns (#113)
- `NeuralFieldParameters::operator==` compared the `activationFunction` `unique_ptr` by
  address instead of by value, so two independently-constructed parameter sets with
  identical activation functions compared unequal. It now dispatches to the concrete
  activation function's own value `operator==` (guarding against two different
  activation-function types with numerically-coincidental fields wrongly comparing
  equal). Separately, the copy constructor and copy assignment disagreed on null-source
  handling (the constructor substituted a default `SigmoidFunction(0, 10)`; assignment
  reset to `nullptr`, which could leave a `NeuralField` dereferencing a null activation
  function on the next `init()`/`step()`); both now agree on the constructor's
  default-substitution policy. Move constructor and move assignment were also missing
  entirely, so moves silently degraded into deep-cloning copies; both are now declared
  and transfer ownership directly. Copy assignment now clones before assigning any
  member, so a throwing `clone()` leaves the destination unchanged (#119)
- The element-creation forms in the GUI caught nothing, while the library has been
  deliberately moving toward failing loudly (`ElementDimensions`, the `Element` base
  constructor, `ElementFactory` all throw on invalid input). Those forms run inside the
  ImGui render loop, so a user typing a bad size or dimension could send an exception
  straight out of the frame and terminate the application. All 28 creation call sites
  now funnel through `describeElementCreationFailure()`, which reports the failure as
  an inline message under the **Add element** button instead. On the frame **Add** is
  pressed the guard covers the whole render-and-construct step for the parameter form;
  every other frame renders unguarded, so a genuine rendering fault still surfaces as
  itself (#146)

- `SimulationFileManager` pre-checked `x_max`/`d_x` for non-positive values before
  constructing an element but never checked `y_max`/`d_y`, so a malformed `.dnf` was
  reported cleanly on one axis and thrown from deep inside the load on the other. Both
  axes now report the same way. `y_max`/`d_y` remain optional, so files that omit them
  still load (#146)

- Both axis pre-checks only tested for a positive value, but the loader converts
  `x_max`/`y_max` with `get<int>()`. An extent that does not fit in an `int` was
  therefore an out-of-range floating-to-integer cast — undefined behaviour, which in a
  debug build aborted the process outright rather than reporting a malformed file. Both
  extents must now be whole numbers in `int` range, and both step sizes finite. An
  integer-valued float (`"x_max": 50.0`) still loads (#146)

- `loadElementsFromJson()` called `jsonToElements()` outside any `try`, so a contract the
  pre-check does not re-derive — such as `ElementDimensions`' samples-per-axis ceiling,
  reachable from a valid `x_max` with a tiny `d_x` — escaped the loader entirely and, in
  the GUI, unwound out of the render loop. Element construction is now guarded and a
  failure is reported as a malformed file; anything the aborted load had already added is
  rolled back, leaving elements the caller held beforehand untouched (#146)

- A failed load rolled back the elements it had added but kept the `identifier` and
  `deltaT` it had already read from the same file, leaving the simulation renamed and
  re-timed while holding none of that file's elements. Both are now restored when the
  load aborts (#146)

- A file rejected by the up-front element validation was still followed by
  `Simulation loaded from: <path>` at INFO level, so the log reported success directly
  after reporting the file as invalid. That path now aborts the load like any other
  failure (#146)
- **File → Quit** and **Ctrl+Q** called `std::exit(0)` from inside the ImGui render
  callback, terminating the process without unwinding the stack: `Application::close()`
  never ran and no destructor fired. Both now call the new `Application::requestQuit()`,
  which `hasGUIBeenClosed()` reports, so the ordinary main loop falls through to
  `close()` and shuts down normally. Existing main loops need no change (#122)
- `Element`'s constructor logged an error and did a bare `return` when
  `dimensionParameters.size` was non-positive, leaving `commonParameters` at its default
  value and `components` completely empty instead of failing to construct. Callers then
  hit a confusing `ELEM_COMP_NOT_FOUND` from `getComponentPtr("output")`, or saw
  `getSize() == 0` and had downstream loops silently no-op, rather than learning at
  construction time that the object was never valid. The constructor now throws
  `Exception(ErrorCode::ELEM_INVALID_SIZE, ...)` instead, matching the validation
  `GaussStimulus` already performs for its own parameters (#118)
- `Application::enableKeyboardShortcuts()` and `Application::appendFonts()` bound
  `ImGui::GetIO()` — which returns `ImGuiIO&` — with `auto io = ...`, copying the
  struct by value. Every write through `io` (`ConfigFlags |=
  ImGuiConfigFlags_NavEnableKeyboard`, `FontDefault = ...`) landed on a discarded
  temporary, so keyboard navigation never actually enabled and ImGui's default font
  was never actually applied, even though the intended font/config values were
  computed correctly. Both now bind `ImGuiIO&` by reference, so the changes persist
  on the real global IO (#114)
- `NodeGraphWindow` called `ImNodeEditor::EndCreate()` only on the
  `BeginCreate() == true` path. `BeginCreate()` marks the creator action active
  *before* it can return false, and only `EndCreate()` clears that flag, so any
  frame without a create action left the action stuck active — tripping
  `IM_ASSERT(false == m_InActive)` on the next frame, and silently breaking
  drag-to-connect in builds with asserts compiled out. `EndCreate()` is now called
  unconditionally, matching upstream's own examples. Found by the new headless UI
  suite (#127)
- Two logger tests passed or failed purely on the order the suites happened to run in.
  `Logger::minLogLevel` is process-wide state, and `tests/simulation/test_thread_safety.cpp`
  and `tests/validation/validation_common.h` raised it to `FATAL` without ever restoring
  it, silently suppressing the console output that later suites assert on. Both sites now
  use an RAII guard that restores the previous level on scope exit
- The same class of shared-state problem inside `NodeGraphWindow`: its hover timers,
  EMA-smoothed colormap ranges, and pending click-to-click pin were function-local
  statics keyed by node id and element name, so two tests reusing an element name
  shared cache entries and results depended on test order. They now live in one
  place with a `NodeGraphWindow::resetTransientStateForTesting()` entry point that
  the UI test fixture calls between tests
  - The issue-triage workflow closed newly filed issues as duplicates of themselves —
  the issue list handed to Gemini for duplicate detection was fetched after the issue
  was opened, so it contained the issue being triaged, and nothing rejected a
  self-referential `duplicate_of` before closing. The triaged issue is now filtered out
  of that list, and a close only happens when `duplicate_of` is numeric and refers to a
  different issue. Secondary labels are also no longer word-split, so `good first issue`
  and `help wanted` are applied as single labels instead of failing the step (#159)
  - The `doc-sync` check went red on every open PR once the Gemini free tier's 20
  requests a day were spent, reporting a quota error that said nothing about the PR
  under review. Quota exhaustion is now tolerated with a warning. The gate fails
  closed: an error that is not positively identified as a quota or rate limit — and
  an empty one — still fails the job (#148)

### Added
- `Application::requestQuit()` / `isQuitRequested()` — ask the application to shut down
  at the end of the current frame, and query whether a shutdown was requested (#122)
- Headless ImGui test harness (`tests/user_interface/ui_test_harness.h`) that drives
  real `render()` calls with no window and no OpenGL context, plus ~190 tests across
  the user-interface and visualization layers, so those files are genuinely exercised
  rather than sitting at 0% in the coverage denominator (#127)
- `Logger::getMinLogLevel()`, so callers that temporarily raise the log threshold can
  restore the previous value instead of assuming the default
- `SigmoidFunction.ApplyAgreesWithOperatorCallAcrossRegimes`: pins `apply()` (the path
  `NeuralField::calculateOutput()` takes every step) to `operator()` within 1e-12 across
  five steepness/shift regimes. The two once disagreed — `apply()` computed in float32
  while `operator()` used float64, so the same field gave different results depending on
  which ran, a reproducibility hazard for threshold-driven stability detection. Both have
  been float64 since "Sigmoid to float64 end-to-end"; this closes the acceptance criterion
  that was never covered. The old split shows up as a ~2e-7 discrepancy here, five orders
  of magnitude above the tolerance (#120)

### Documentation
- `tests/golden/test_golden_activation.cpp` still described `SigmoidFunction::apply()` as
  computing in float32 and the reference as mirroring that; both have been float64 for
  some time and `reference/ref_activation.h` already said so
- `CONTRIBUTING.md` told contributors to run `build.bat` / `./build.sh` / `./build_macos.sh`
  from the repository root, but those scripts live in `dynamic-neural-field-composer/scripts/`,
  so every build command failed on a fresh clone. It also listed a GCC 11+ minimum while the
  README and CI both require GCC 13+, gave a `ctest --build-config Release` invocation that
  cannot work against the single-config build trees the scripts produce (and was run from a
  directory with no test configuration), and linked to `wiki/Getting-Started.md` when the page
  is `wiki/Getting Started.md`. All corrected, with the setup step and the per-platform CTest
  directories documented (#132)

## [2.9.6] - 2026-07-31

### Fixed
- `Element`'s input cache stored a `{pointer, size}` snapshot of each connected source
  taken once when the cache was built; if a source was resized via `changeDimensions()`
  afterward, the size snapshot went stale and `updateInput()` could read past (or
  under-read) the source's actual current buffer. The cache now stores a
  `const vector<double>*` and re-reads `.size()` on every call, so it can never be
  stale; a source that grows past what the receiver's buffer can hold is now
  proactively disconnected with a warning instead of silently corrupting memory (#40)
- `SimulationRecorder::startRecording` called the throwing `create_directories`
  overload (result never checked) then opened the file; on failure it logged and
  returned `void`, so the caller believed recording was active while nothing was
  written. `startRecording` now returns `bool` and returns `false` on every failure
  path (directory could not be created, or the file could not be opened) before a
  session is created, so `isRecording()`/`hasActiveRecordings()` reflect reality by
  construction; the GUI recomputes its recording-state flags accordingly (#43)
- `gaussNorm` divided by the Gaussian sum with no guard; a degenerate width (σ→0) or
  otherwise near-zero/non-finite sum produced NaN/Inf that silently propagated through
  every connected field. Now guards the denominator and returns a safe zero vector
  with a logged warning instead (#42)
- `ElementDimensions{N}` (single int) selects the field dimensionality (must be 1 or
  2), while `ElementDimensions{N, d_x}` builds a 1D field of length `N` — a one-argument
  difference with opposite meaning. An invalid single-int value previously logged an
  `ERROR` but still returned a usable 100-cell object, silently mislabeling the
  requested size (and, at larger `N` in a separate downstream benchmark, tripping a
  stack-buffer overrun). The single-int constructor now throws on an invalid
  dimensionality, and all three constructors validate extent/spacing/sample-count
  (non-positive, non-finite, or overflowing `size_x * size_y`) before it can reach a
  buffer allocation (#86)
- A malformed or truncated `.dnf` file crashed deserialization with an unhandled
  `nlohmann::json` exception instead of failing cleanly, and could leave the
  simulation half-loaded. `SimulationFileManager::jsonToElements` now validates every
  element's required fields (`uniqueName`, `label`, `x_max`, `d_x`) in a pass over the
  whole file before constructing anything, so a malformed entry anywhere aborts the
  load with a descriptive error and no partial mutation (#39)
- `PlotControlWindow`'s "quick populate" action added new 2D neural fields as line
  plots instead of heatmaps (#57)

### Changed
- Converted ~71 log-message-building call sites across 19 `src/` files from manual
  `+`/`std::to_string`/`ostringstream` concatenation to `std::format`; message text is
  preserved verbatim, only the construction mechanism changed (#61)
- Removed dead commented-out code from the line-plot renderer and replaced hardcoded
  raw ImPlot axis indices (`Axes[0]`, `Axes[3]`) with the named `ImAxis_X1`/`ImAxis_Y1`
  constants (#52)

### Added
- Unit tests for the plot parameter classes (`PlotDimensions`, `PlotAnnotations`,
  `PlotCommonParameters`, `PlotType`), which previously had no direct coverage (#66)
- Tutorial, Parameter Tuning Guide, `.dnf` File Schema, and Troubleshooting wiki pages (#56)

## [2.9.5] - 2026-07-30

### Added
- Extended the hybrid direct/FFT convolution path (previously `MexicanHatKernel2D`
  only) to `GaussKernel2D`, `AsymmetricGaussKernel2D`, `OscillatoryKernel2D`, and
  `CorrelatedNormalNoise2D`, via a shared dispatch rule (`tools::math::shouldUseSpectral2D`)
  and wrap-embedding helpers (`embedWrapped1D`, `buildWrappedSeparableKernel2D`) hoisted
  out of `mexican_hat_kernel_2d.cpp` into `tools/math.h` and `tools/fft_convolution.h`
- Added a process-global `tools::math::ConvolutionMode` override
  (`Auto`/`ForceDirect`/`ForceSpectral`, via `ScopedConvolutionMode`) as a test seam for
  building direct/spectral twins of the same element configuration
- `SpectralConvolver2D::init()` now no-ops when re-initialized at an unchanged size
  (only `setKernel()` re-runs), and plans with `FFTW_ESTIMATE` instead of `FFTW_MEASURE`,
  so `setParameters()` — called on every frame while a width slider is dragged — no longer
  triggers FFTW timing trials on the UI thread
- `tools::math::seedNormal()`: deterministic re-seed of the thread-local normal generator
  behind `fillNormal`, enabling reproducible direct-vs-spectral comparisons for elements
  whose input is itself randomly generated (`CorrelatedNormalNoise2D`)
- 128x128 golden fixtures under `tests/validation/data/2d_spectral/`, chosen to straddle
  the spectral dispatch threshold, plus a regeneration tool
  (`dnf_composer_regen_spectral_golden`) and `SpectralGolden2D` regression tests pinning
  both the direct and spectral paths against numerical/qualitative drift
- `fftw3` added to `scripts/setup.bat`/`setup.sh` vcpkg package lists (previously missing
  despite being a hard `find_package(... REQUIRED)` dependency, so a fresh clone could not
  configure)

### Fixed
- `CorrelatedNormalNoise2D::init()` now clamps kernel support to the field size per axis via
  `computeKernelRange` (matching every other 2D kernel element), instead of the previous
  unclamped `halfWidth = 5*width`: a wide `width` on a small field (e.g. `width=3.0` on a
  10x10 field) produced a negative starting index from `createExtendedIndex`, which
  `conv2d_separable_into`'s circular x-pass then read as an out-of-bounds offset before the
  row buffer — a real, reachable heap OOB read (the UI allows `width` up to 30)
- `Element`'s implicit copy constructor/assignment copied `inputPtr` (a raw pointer into the
  object's own `components["input"]`) by value, so any element cloned via the
  `make_shared<T>(*this)` pattern after it had already stepped once would alias the
  *source's* input buffer instead of its own, silently dropping input on the clone. `Element`
  now has an explicit copy constructor/assignment that resets the input cache, forcing a
  correct rebuild on first `updateInput()` — the same recovery path `changeDimensions()` /
  `addInput()` / `removeInput()` already use
- `OscillatoryKernel2D::clone()` now copy-constructs (`make_shared<OscillatoryKernel2D>(*this)`)
  instead of using the parameter constructor, matching every other element: the previous form
  left `kernel_1d_x/y`, `extIndex_x/y`, and the scratch buffers default-constructed empty,
  which is unsound whenever a clone is stepped without an intervening `init()` (e.g.
  `Simulation`'s copy constructor deep-copies elements via `clone()` without calling `init()`)
- `Element::removeOutputs()` erased the receiver's `inputs` map entry but left the receiver's
  `inputPtr`/`cachedInputs_` pointing at the removed element's `components["output"]` buffer;
  once that element was destroyed, the receiver's next `step()` read freed memory. `inputPtr` is
  now reset on the receiver so the cache rebuilds on the next `updateInput()`

## [2.9.4] - 2026-07-10

### Fixed
- Fixed two data races (reported by the downstream `neat-dnfs` TSan CI job, see
  `.claude/reports/dnf_composer-tsan-data-races.md`) that surfaced under any concurrent use of
  `Simulation`, e.g. evaluating multiple simulations in parallel via `std::async`:
  - `Simulation::generateUniqueIdentifier()` and `SimulationRecorder`'s internal timestamp
    helper used `std::localtime`, which returns a pointer to shared static storage; both now use
    the existing reentrant `tools::utils::safe_localtime()` helper
  - `tools::utils::getResourceRoot()`'s function-local `static const` initialization is now
    guarded by an explicit `std::call_once` instead of relying solely on compiler-provided
    thread-safe statics
- Fixed a third, previously-unreported data race found while writing the regression test above:
  `LogWindow::addLog()`/`renderContent()`/`clean()` mutated and iterated the shared static `logs`
  vector with no locking, and `Logger::log()` reassigned a shared static `Logger` instance —
  since every `Simulation` construction logs a message, concurrent construction reliably
  corrupted the heap. `logs` access is now guarded by a mutex, and `Logger::log()` no longer
  mutates shared state (the unused shared `Logger` instance was removed)

### CI
- Added a `tsan` leg to the `sanitizers-linux` job (in addition to the existing ASan+UBSan leg)
  to catch data races in CI
- Added CodeRabbit configuration (`.coderabbit.yaml`) for automatic PR code review and
  external contributor onboarding (free for public repositories, no API key required)
- Added `gemini-issue-triage.yml` workflow: classifies new issues, creates labels
  idempotently, checks for duplicates, and posts a welcome comment using the Gemini API
- Added `gemini-doc-sync.yml` workflow: audits Doxygen, wiki, README, and CHANGELOG
  coverage on PRs that touch `include/**` and posts a checklist comment using the Gemini API
- Added `vcpkg-maintenance.yml` workflow: monthly cron that reads vcpkg port manifests and
  opens a GitHub issue with a package version table (no LLM — pure shell + jq)
- Fixed `gemini-issue-triage.yml`: removed unused `maintenance` label from triage creation,
  used multiline EOF delimiter for step output to avoid shell escaping issues, close issues
  automatically when Gemini detects a duplicate, and switched secondary label allowlist
  validation to word-boundary matching to prevent partial-string false positives

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
  noise with a normalized Gaussian kernel (parameters: `amplitude`, `width`, `circular`)
- `AbsSigmoidFunction` activation function: rational sigmoid `σ(u) = 0.5·(1 + β·u / (1 + β·|u|))`
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
