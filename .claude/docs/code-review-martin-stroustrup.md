# A Conversation About dynamic-neural-field-composer

**Robert C. Martin & Bjarne Stroustrup — an imagined code review**

> Reviewed at commit `3c9e7e1`, 2026-07-02. All file/line references were verified against
> the source at that commit. Paths are relative to the repo root; the code lives under the
> nested `dynamic-neural-field-composer/` folder. The voices are imagined; the findings are real.

---

## Opening impressions

**BJARNE:** Before we sharpen the knives — this is a serious piece of work. A C++20 dynamic
neural field simulator with a real-time ImGui front end, JSON persistence, a node-graph
editor, and it builds and tests on three platforms in CI. The headers use `#pragma once`
consistently, I see `std::ranges`, `std::format`, `<numbers>`, `[[nodiscard]]`, `explicit`
constructors, a `thread_local` RNG in the math layer, and `std::unique_ptr` with a proper
virtual `clone()` for the polymorphic activation functions. Someone has been paying attention
to the last decade of C++.

**BOB:** Agreed, and let me add: the Doxygen comments on the core headers are genuinely good.
The Amari-equation documentation on `NeuralField`
([neural_field.h](dynamic-neural-field-composer/include/elements/neural_field.h)) and the
cross-framework activation-function tables are the kind of documentation that earns its keep.
The `Collapse` element ([collapse.cpp](dynamic-neural-field-composer/src/elements/collapse.cpp))
is a model of defensive validation — it rejects non-2D inputs, size mismatches, and multiple
inputs with clear messages. And `tools/test_math.cpp` contains *real* tests: exact numeric
assertions on convolution, Gaussians summing to one, and — I was pleased to see —
out-of-bounds safety cases. That's rare.

**BJARNE:** The allocation-free `_into` variants in
[math.h](dynamic-neural-field-composer/include/tools/math.h) — `conv_valid_into`,
`conv_same_into`, `reduce2DAxis_into` — are a proper zero-overhead redesign of the hot path.
And the trait-based dependency injection in `Application::addWindow<T>` using `requires` and
detection idioms is the best-designed piece of the application layer.

**BOB:** So the compliments are sincere. Now let's talk about what would stop me merging this
if it came across my desk.

---

## 1. One name, two definitions — the ODR landmine

**BJARNE:** The most dangerous thing in this codebase isn't a bug you can see run. It's a
violation of the One Definition Rule. Every parameter struct is defined **twice**, under the
**same fully qualified name**, with **different layouts**:

| Type | Canonical (used) | Orphan duplicate |
|---|---|---|
| `GaussKernelParameters` | [gauss_kernel.h](dynamic-neural-field-composer/include/elements/gauss_kernel.h) (`final`, has `amplitudeGlobal`) | [gauss_kernel_parameters.h:9](dynamic-neural-field-composer/include/element_parameters/gauss_kernel_parameters.h#L9) (no `amplitudeGlobal`) |
| `MexicanHatKernelParameters` | `elements/mexican_hat_kernel.h` | `element_parameters/mexican_hat_kernel_parameters.h` |
| `GaussStimulusParameters` | `elements/gauss_stimulus.h` | `element_parameters/gauss_stimulus_parameters.h` |
| `NeuralFieldParameters` | `elements/neural_field.h` | `element_parameters/neural_field_parameters.h` |
| `NormalNoiseParameters` | `elements/normal_noise.h` | `element_parameters/normal_noise_parameters.h` |

The orphan headers are included only by their own `.cpp` files, so today the two definitions
never meet in one translation unit — but both emit out-of-line member functions into the
same library. `dnf_composer::element::GaussKernelParameters` has two sizes depending on which
TU you ask. That is undefined behavior by definition, and the moment anyone includes both
headers it becomes a hard redefinition error. The kernel implementation uses
`parameters.amplitudeGlobal` ([gauss_kernel.cpp:67](dynamic-neural-field-composer/src/elements/gauss_kernel.cpp#L67)),
which the orphan struct doesn't even have — so the `element_parameters/*_parameters.{h,cpp}`
files are superseded dead code that still compiles into your binary.

**BOB:** This is what happens when you refactor and leave the corpse in the repo. Dead code
isn't neutral — it lies. A new contributor opens `include/element_parameters/`, sees a
plausible-looking `GaussKernelParameters`, includes it, and either gets a build break or —
worse — a struct that silently lacks a field the simulation depends on. **Delete the orphans.**
Keep `element_parameters.{h,cpp}` (the legitimate base classes) and remove the rest.

---

## 2. Ownership and lifetimes

**BJARNE:** Now, resource management. Three findings, in descending order of subtlety.

**First: reference cycles by construction.** Look at
[element.h:32-33](dynamic-neural-field-composer/include/elements/element.h#L32-L33):

```cpp
std::unordered_map<std::shared_ptr<Element>, std::string> inputs;
std::unordered_map<std::shared_ptr<Element>, std::string> outputs;
```

Both directions of every connection are *owning* pointers. Connect field A to kernel B and
A holds a `shared_ptr` to B while B holds a `shared_ptr` to A. That is a guaranteed cycle.
Destroy the `Simulation` without explicitly calling `removeInputs`/`removeOutputs` on every
element and the whole graph keeps itself alive forever. The idiomatic fix is well known:
one direction owns, the back-edge observes. Make `outputs` hold `weak_ptr`.

**Second: a raw-pointer cache that can dangle.** `Element::buildInputCache()`
([element.cpp:141](dynamic-neural-field-composer/src/elements/element.cpp#L141)) stores
`compVec.data()` pointers into *other elements'* component vectors:

```cpp
struct CachedInput { const double* src; std::size_t size; };  // element.h:35
```

Invalidation is handled for the element's own resize, but when a **producer** is resized —
`Simulation::changeDimensions` calls `removeOutputs()` on it — its consumers' cached `src`
pointers are never reset. The next `updateInput()` reads freed memory. It happens to work
today only because the UI path calls a full `init()` afterwards, which rebuilds every cache.
Nothing enforces that ordering. A cache of raw pointers into containers owned by someone
else, with invalidation by convention — that is precisely the kind of design C++ gives you
the tools to avoid.

**Third, and related:** `updateInput()`
([element.cpp:161](dynamic-neural-field-composer/src/elements/element.cpp#L161)) iterates
over the *producer's* cached size and writes into `inputPtr` sized for the *consumer*:

```cpp
for (const auto& [src, size] : cachedInputs)
    for (std::size_t i = 0; i < size; ++i)
        inputPtr[i] += src[i];
```

Sizes are validated once, at `addInput` time. Grow a producer's output while a connection
persists and this overruns the buffer. One `assert`/clamp would make the invariant explicit.

**BOB:** Add a fourth: a plain leak. `NodeGraphWindow`'s constructor calls
`ImNodeEditor::CreateEditor(&config)` and its destructor is `= default` — there is no
`DestroyEditor` anywhere in the codebase
([node_graph_window.cpp](dynamic-neural-field-composer/src/user_interface/node_graph_window.cpp)).
Raw owning pointer, no cleanup. Wrap it in a `unique_ptr` with a custom deleter and the
problem ceases to exist as a category.

---

## 3. Bugs you can demonstrate today

**BOB:** Let's list the ones that are simply wrong, because these become issues verbatim.

**Erase-during-iteration UB.** `Visualization::render()`
([visualization.cpp:~207-263](dynamic-neural-field-composer/src/visualization/visualization.cpp#L207-L263))
range-for-iterates the `plots` map and calls `removePlot(plotID)` — which erases from that
same map — from inside the loop. That invalidates the iterator the range-for is about to
increment. Undefined behavior. And the mid-loop stale-data path does `removePlot(...); return;`,
which silently skips rendering every remaining plot for that frame. Collect IDs, erase after
the loop.

**BJARNE:** **A buffer overflow waiting for a long path.**
[file_dialog.h:379](dynamic-neural-field-composer/include/tools/file_dialog.h#L379) and
[:392](dynamic-neural-field-composer/include/tools/file_dialog.h#L392):

```cpp
snprintf(buffer, path.length() + 1, "%s", path.c_str());
```

The second argument of `snprintf` must be the **destination** size. The caller passes a
fixed `char[500]`; pick a file whose path is 500+ characters and this writes past the end.
The function even has a `buffer_size` parameter — currently annotated `[[maybe_unused]]`.
Use it. Better: return a `std::string` and delete the C buffer entirely.

**BOB:** **Two settings that silently never apply.**
[application.cpp:152](dynamic-neural-field-composer/src/application/application.cpp#L152) and
[:158](dynamic-neural-field-composer/src/application/application.cpp#L158):

```cpp
auto io = ImGui::GetIO();   // ImGuiIO by VALUE — a copy
io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;   // written to the copy, discarded
```

`GetIO()` returns a reference; `auto` deduces a value. Keyboard navigation is never enabled,
and in `appendFonts()` the `io.FontDefault = g_MediumMediumFont;` assignment is likewise lost
(font *building* still works only because `io.Fonts` is a pointer into shared state). Two
characters — `auto&` — fix it. This is also Exhibit A for the testing discussion: no test
could have caught it because no test touches this layer at all.

**BJARNE:** **Unchecked downcasts in the public factory.** The pattern in
[element_factory.cpp:18-19](dynamic-neural-field-composer/src/elements/element_factory.cpp#L18-L19),
repeated roughly thirty times:

```cpp
const auto params = dynamic_cast<const NeuralFieldParameters*>(&elementSpecificParameters);
return std::make_shared<NeuralField>(elementCommonParameters, *params);  // UB if null
```

Pass the wrong parameter subtype — an easy user mistake, this is the public creation API —
and you dereference `nullptr`. Check and throw. Related: `createElement` returns `nullptr`
for an unknown type rather than throwing, pushing null checks onto every caller.

**Half-constructed objects.** The `Element` constructor
([element.cpp:8-18](dynamic-neural-field-composer/src/elements/element.cpp#L8-L18)) logs and
`return`s on an invalid size, leaving `components` empty. Every later access throws a
confusing "component not found" or silently no-ops on size zero. A constructor that cannot
establish its invariants must throw — the exception type and error code already exist, and
`GaussStimulus` does exactly this correctly.

**A comparison that compares the wrong thing.**
[neural_field.h:37](dynamic-neural-field-composer/include/elements/neural_field.h#L37) —
`NeuralFieldParameters::operator==` compares `activationFunction` as smart-pointer
*identity*, not value. Two fields with identical sigmoid parameters compare unequal. The
activation-function classes define value equality; it's simply never called.

**Divergent numerics between "equivalent" paths.** `SigmoidFunction::apply`
([activation_function.cpp:19-26](dynamic-neural-field-composer/src/elements/activation_function.cpp#L19-L26))
computes in `float` while the `operator()` path uses `double` via `math::sigmoid`. For a
simulation whose stability detection is threshold-driven (`0.895` — we'll get to that magic
number), two code paths giving different answers for the same field is a reproducibility
hazard, not a style nit.

**BOB:** And the medium/minor tier, briefly:

- `std::exit(0)` from the Ctrl+Q handler
  ([main_menu_bar.cpp:338](dynamic-neural-field-composer/src/user_interface/main_menu_bar.cpp#L338))
  skips `Application::close()`, GUI shutdown, and all destructors. Signal the main loop instead.
- Heatmap manual dimensions: `rows = y_max/y_step`, `cols = x_max/x_step`, then
  `PlotHeatmap(..., rows, cols, ...)` with **no** check that `rows*cols <= data.size()`
  ([heatmap.cpp:~216-237](dynamic-neural-field-composer/src/visualization/heatmap.cpp#L216-L237)).
  The math layer guards its bounds carefully; this path doesn't.
- `obtainCircularVector` does `contents[indices[i] - 1]` — 1-based indexing with no bounds
  check ([math.h:133](dynamic-neural-field-composer/include/tools/math.h#L133)); a malformed
  kernel range from JSON reads out of bounds.
- `Simulation::run` documents "milliseconds" but loops in simulation-time units, and then
  unconditionally calls `close()`, zeroing all component data the user might want to inspect
  ([simulation.cpp:242-254](dynamic-neural-field-composer/src/simulation/simulation.cpp#L242-L254)).
- Inconsistent not-found contracts: `getElement(string)` returns `nullptr`,
  `getElement(int)` throws — same name, opposite error models.
- `ElementIdentifiers::uniqueIdentifierCounter` is a non-atomic `static int`; `std::localtime`
  is used ([simulation.cpp:554](dynamic-neural-field-composer/src/simulation/simulation.cpp#L554));
  the app is single-threaded *today*, so these are landmines, not explosions.
- `NormalNoise`/`CorrelatedNormalNoise` allocate fresh vectors **every step** — the kernels
  were refactored to scratch buffers; the noise elements were forgotten.
- The global logger: `static Logger logger(...)` **in a header** gives one copy per
  translation unit, and the free `log()` function *reassigns the global on every call*
  ([logger.h](dynamic-neural-field-composer/include/tools/logger.h),
  [logger.cpp](dynamic-neural-field-composer/src/tools/logger.cpp)) — with a default level
  that contradicts the header's. Fragile now, racy the day a worker thread appears.

---

## 4. Modern C++ — the gap between what's used and what's needed

**BJARNE:** The project *knows* modern C++ — that's what makes the inconsistencies stand out.

- **Unscoped enums in 2026.** `ElementLabel` and `ActivationFunctionType` are plain
  `enum ... : int` ([element_parameters.h:11](dynamic-neural-field-composer/include/element_parameters/element_parameters.h#L11),
  [activation_function.h:23](dynamic-neural-field-composer/include/elements/activation_function.h#L23)),
  leaking ~30 names into the namespace and converting implicitly to `int`. `ErrorCode` and
  `ElementCategory` already use `enum class` — finish the job. And
  [plot_control_window.cpp:19](dynamic-neural-field-composer/src/user_interface/plot_control_window.cpp#L19)
  does `reinterpret_cast<int*>(&selectedPlotType)` on a scoped enum — that's not a cast,
  that's a bet on layout.
- **Rule of five, half-applied.** `NeuralFieldParameters` declares copy operations for its
  `unique_ptr` member but no moves — so moves silently degrade to copies — and the two copy
  paths *disagree*: the copy constructor substitutes a default `SigmoidFunction(0,10)` when
  the source is null; copy assignment resets to null. Pick one semantic.
- **Const-correctness and value traffic.** `getComponent`, `getComponentPtr`, `getInputs`,
  `hasInput`, `hasOutput` mutate nothing yet are non-const
  ([element.h:70-126](dynamic-neural-field-composer/include/elements/element.h#L70-L126)).
  `getComponent` returns `vector<double>` by value; `Visualization::getPlots()` returns the
  whole map by value and is called **every frame**. C++20 has `std::span<const double>` for
  exactly these read paths. Trivial getters lack `noexcept`.
- **Stringly-typed hot paths.** Components are addressed as `components["output"]` — a hash
  of a string per access, *inside `step()`*. The 1D kernels were optimized to cache raw
  pointers; the 2D kernels still do repeated map lookups per step
  ([gauss_kernel_2d.cpp:79-91](dynamic-neural-field-composer/src/elements/gauss_kernel_2d.cpp#L79-L91)).
  Inconsistent, and a real per-step cost. An `enum class`-keyed component set, or cached
  references established at `init()`, would give both safety and speed.
- **C-style residue in the UI layer.** `char[128]`/`char[500]` buffers, `snprintf`, `sscanf`,
  `strlen` throughout `main_menu_bar.cpp`, `heatmap.cpp`, `field_metrics_window.cpp`;
  `#undef min/max/ERROR` scattered to fight `<windows.h>`; 18 `inline ImFont* g_*` globals
  re-`extern`-declared ad hoc in nearly every UI translation unit.

**BOB:** I'd summarize your list in one sentence: the load-bearing invariants live in
comments and conventions instead of in types. That's the theme.

---

## 5. Duplication — Bob's turn

**BOB:** Now my favorite subject. Duplication is the root of most evil in software, and this
codebase has it in four distinct strains.

**Strain one: 1D/2D copy-paste.** `gauss_kernel.cpp` vs `gauss_kernel_2d.cpp`,
`mexican_hat_kernel` vs `_2d`, `normal_noise` vs `_2d` — and worse, `GaussKernel::step` and
`MexicanHatKernel::step` are near-identical
([gauss_kernel.cpp:52-71](dynamic-neural-field-composer/src/elements/gauss_kernel.cpp#L52-L71) ≈
mexican_hat_kernel.cpp:56-75); only kernel *construction* differs. The
convolve-plus-global-offset tail is pasted across at least four kernel types. That's a
`Kernel::applyConvolution()` base method begging to exist.

**Strain two: the factory.** [element_factory.cpp](dynamic-neural-field-composer/src/elements/element_factory.cpp)
is ~270 lines of **two parallel dispatch tables** — a lambda map *and* a 30-case `switch` —
that must be kept in lockstep by hand. Adding one element type means editing the enum, the
string map, the category table, both factory tables, and the umbrella header. Every one of
those edits is an opportunity to forget one.

**Strain three: the god windows.** `element_window.cpp` is **~2,600 lines** with ~34
`modifyElementXxx` methods; `simulation_window.cpp` has ~30 `addElementXxx` methods; one per
element type × {1D, 2D}, each block near-identical. `MainMenuBar::renderMainMenuBar` carries
its own confession in a comment: *"Clang-Tidy: cognitive complexity of 79 (threshold 25)."*
When the code documents its own violation and nobody acts on it, the comment has become
wallpaper. And these render methods do file I/O and simulation control inline —
`modifyElementFieldCoupling` builds paths, calls `create_directories`, and reads weight files
from inside a *render* function
([element_window.cpp:~1347](dynamic-neural-field-composer/src/user_interface/element_window.cpp#L1347)).
Rendering, persistence, and domain logic in one method: that's three reasons to change, which
is two too many.

**Strain four: the examples.** All 22 `examples/*.cpp` repeat the same ~35-line
try/catch/window-setup/run-loop boilerplate verbatim. One `runExample(setup)` helper removes
hundreds of lines and leaves each example showing only what it exists to show: the DNF wiring.

And the small stuff that grates: `if (found) return true; return false;` appears six times
([element.cpp:113-115](dynamic-neural-field-composer/src/elements/element.cpp#L113-L115) among
others) — that's spelled `return found;`. `NeuralField::updateBumps` is ~120 lines, four
levels deep, with the velocity/acceleration block copy-pasted **three times inside the same
function**, an off-by-one between bump start (`(i+1)*d_x`) and end (`i*d_x`) positions, and
magic numbers — `0.00001`, `2.0`, a default stability threshold of `0.895` — with no named
constant or rationale. Commented-out code lingers in `lineplot.cpp:186-205`, `heatmap.cpp`,
`math.h`. Namespace style flips between `namespace dnf_composer::element {}` and the nested
form file by file. None of these are hard to fix. All of them together tell a reader:
nobody's watching.

**BJARNE:** One architectural note that belongs here: the **`tools` layer depends upward**.
`tools/logger.h` includes `imgui-platform-kit/log_window.h`, and `logger.cpp` includes
`application/application.h` and `user_interface/log_window.h`. A utility layer that cannot
exist without the GUI is not a utility layer. Give the logger a sink interface and let the
UI register itself.

**BOB:** That's the Dependency Inversion Principle, and I couldn't have set up the example
better myself.

---

## 6. Testing — the honest conversation

**BOB:** The project has 39 test files, GTest wired into CTest, run on Linux, Windows, and
macOS in CI, with Codecov. That puts it ahead of most research code, and I want that on the
record. `test_math.cpp` asserts exact convolution values and bounds safety.
`test_simulation_extended.cpp` covers pause/resume semantics, deep-copy, self-assignment.
`test_neural_field.cpp` checks an *analytic* fixed point. Those are real tests.

Now the other half of the record:

- **`user_interface/` — 17 source files, zero tests.** **`visualization/` — 5 source files,
  zero tests.** And before anyone says "you can't test GUIs": the plot bookkeeping, the
  series add/remove logic, and the field-metrics computations need no GL context. The
  `ImGuiIO` copy bug and the erase-during-iteration UB both live in this untested third of
  the codebase. That is not a coincidence. Untested code is where bugs go to live.
- **Kernel tests are structural, not numerical.** `test_kernels.cpp` checks construction,
  get/set round-trips, `clone`, and that the kernel "has positive and negative values."
  Nobody asserts a Gaussian kernel contains *correct Gaussian values*. For a numerics
  library, kernel generation is the product — test it to the decimal.
- `test_application.cpp` has **two** tests. The 22 examples are compiled but never executed —
  a headless smoke run of each in CI would catch whole categories of regression for free.
- The six concrete parameter structs have no dedicated tests (exercised only indirectly).
- And a housekeeping absurdity: **catch2 is installed by every setup script and every CI
  job, and used by exactly zero files.**

**BJARNE:** I'd add the flip side: the parts of the numeric core that *are* tested are tested
well, and CONTRIBUTING.md openly admits "test coverage has (a lot of) gaps." Honesty in
documentation counts for something. But for this kind of code I would rank two additions
above all others: **compiler warnings and sanitizers**. There is not one `-Wall`, `-Wextra`,
or `/W4` in the entire build, and no ASan/UBSan job in CI — for a library doing manual buffer
indexing, where the math tests themselves acknowledge out-of-bounds risk. An UBSan run would
likely flag the erase-during-iteration and the dangling-cache paths *mechanically*. The
compiler is the cheapest reviewer you will ever hire; this project has it muted.

---

## 7. Build and CI

**BJARNE:** The CMake is largely modern and target-based — `target_compile_features(...
cxx_std_20)`, generator expressions, proper `install(EXPORT)` and package-config. Credit due.
Four problems:

1. **No dependency pinning.** There is no `vcpkg.json` manifest; every setup script and CI
   job clones vcpkg at HEAD, and `imgui-platform-kit` is cloned from a personal repo at HEAD.
   Any upstream change can break every build and release non-deterministically. A manifest
   with a `builtin-baseline`, and a pinned commit for the platform kit, fixes this outright.
2. **Absolute build-machine paths compiled into shipped binaries** — `PROJECT_DIR` and
   `OUTPUT_DIRECTORY` are baked in at configure time
   ([CMakeLists.txt:43-48](dynamic-neural-field-composer/dynamic-neural-field-composer/CMakeLists.txt#L43-L48)).
   There is already a runtime exe-relative `getResourceRoot()`; lean on it.
3. The build hard-fails without `VCPKG_ROOT` and `include()`s the vcpkg toolchain from inside
   the project file — fighting the standard `-DCMAKE_TOOLCHAIN_FILE` flow CI also uses, so
   the toolchain loads twice.
4. The `INSTALL_INTERFACE` include dir says `include` while headers install under
   `include/dnf_composer/` ([CMakeLists.txt:242](dynamic-neural-field-composer/dynamic-neural-field-composer/CMakeLists.txt#L242)
   vs [:288](dynamic-neural-field-composer/dynamic-neural-field-composer/CMakeLists.txt#L288)) —
   downstream `find_package` consumers may not find what the docs promise.

**BOB:** The CI *surface* is impressive — seven workflows, three OSes, docs deploy, releases,
even automated triage. But look closer:

- The static-analysis badge is **decorative**: `.clang-tidy` sets `WarningsAsErrors: ''`,
  so the job cannot fail no matter what clang-tidy finds. A gate that can't close isn't a gate.
- Cache keys hash **files that don't exist** —
  [static-analysis.yml:32](/.github/workflows/static-analysis.yml#L32) hashes
  `dynamic-neural-field-composer/build.sh` and release.yml hashes `build.sh`/`build_macos.sh`
  at the root; the scripts live in `scripts/`. `hashFiles` on a missing path returns empty,
  so those caches never invalidate on dependency changes.
- Coverage runs on one config (Linux Debug) with `fail_ci_if_error: false` — upload failures
  vanish silently.
- CONTRIBUTING.md tells contributors to run `build.bat` from the root (it's in `scripts/`)
  and claims GCC 11+ while README and CI require GCC 13+. Stale instructions are worse than
  none; they cost a new contributor their first hour.

---

## Closing

**BJARNE:** My summary: this is a capable, genuinely modern C++20 codebase whose core
numerical design — the `_into` math layer, the element/step architecture — is sound. Its
problems are concentrated and nameable: one ODR violation to delete, an ownership model that
needs `weak_ptr` and needs the raw-pointer cache made safe, a handful of demonstrable bugs,
and a build that runs with the compiler's warnings switched off. None of it is exotic. All
of it is fixable in weeks, not months.

**BOB:** And mine: the difference between this project and a great one is discipline at the
edges — delete the dead code, break up the god files, extract the duplication, and above all
put tests around the third of the codebase that has none, because that's exactly where we
found the bugs. The team clearly knows how to write good tests; they've written some. Now
write the rest.

---

# Actionable Items

Issue-ready. Severity: **Critical** (UB/memory-safety, fix first), **High**, **Medium**,
**Low/Chore**.

### Critical

**1. Remove duplicate `*Parameters` definitions (ODR violation)**
Files: `include/element_parameters/{gauss_kernel,mexican_hat_kernel,gauss_stimulus,neural_field,normal_noise}_parameters.h` + matching `src/element_parameters/*.cpp`, `CMakeLists.txt` source lists.
Five parameter structs are defined twice under the same qualified name with different layouts; both variants compile into the library. Delete the orphaned superseded files (keep `element_parameters.{h,cpp}`).
*Accept:* orphan files removed from tree and build; library builds and all tests pass; grep shows exactly one definition per parameter struct.

**2. Fix erase-during-iteration UB in `Visualization::render()`**
File: `src/visualization/visualization.cpp` (~207-263).
`removePlot()` erases from the `plots` map inside a range-for over it; the stale-data path also `return`s, skipping remaining plots for the frame. Collect IDs to remove, erase after the loop; don't abort the frame.
*Accept:* no erase inside iteration; closing one plot renders all others that frame; add a unit test for the removal bookkeeping.

**3. Fix `snprintf` destination-size bug in file dialog**
File: `include/tools/file_dialog.h:379,392`.
Second argument is `path.length()+1` (source length) instead of destination size; caller's buffer is `char[500]`. Use the existing `buffer_size` parameter (drop its `[[maybe_unused]]`), or refactor to return `std::string`.
*Accept:* both call sites bounded by destination size; a 600-char path truncates safely; test added.

**4. Make the raw input cache resize-safe and bounds-guarded**
Files: `include/elements/element.h:35-38`, `src/elements/element.cpp:141,161`, `src/simulation/simulation.cpp` (`changeDimensions`).
Producer resize leaves consumers' cached `data()` pointers dangling; `updateInput` writes producer-sized data into a consumer-sized buffer with no guard. Invalidate consumers' caches whenever a producer's components resize (or rebuild caches automatically), and guard/assert sizes in `updateInput`.
*Accept:* resizing a connected producer then stepping does not touch freed memory (verify under ASan); size mismatch produces a clear error, not an overrun.

### High

**5. Break the `shared_ptr` ownership cycle in element connections**
Files: `include/elements/element.h:32-33` + users.
`inputs` and `outputs` both hold `shared_ptr<Element>`; every connection is a cycle. Make the back-edge (`outputs`) `weak_ptr`.
*Accept:* destroying a `Simulation` without manual teardown frees all elements (ASan/leak check in a test).

**6. Null-check `dynamic_cast` in `ElementFactory`; throw on unknown type**
File: `src/elements/element_factory.cpp` (pattern ×~30, `createElement` returns).
*Accept:* wrong parameter subtype and unknown `ElementLabel` both throw a descriptive `Exception`; tests cover both paths.

**7. `auto io = ImGui::GetIO()` copies — keyboard nav and default font never applied**
File: `src/application/application.cpp:152,158`. Change to `ImGuiIO&`.
*Accept:* `NavEnableKeyboard` flag verified set after `init()`; default font takes effect.

**8. Destroy the node-editor context**
File: `src/user_interface/node_graph_window.cpp` (+ header).
`CreateEditor` without `DestroyEditor`. Own the context via RAII (unique_ptr + custom deleter) or an explicit destructor.
*Accept:* `DestroyEditor` called exactly once per window lifetime; no leak under ASan.

**9. Enable compiler warnings; add a sanitizer CI leg**
Files: `dynamic-neural-field-composer/CMakeLists.txt`, `.github/workflows/ci.yml`.
No `-Wall/-Wextra`/`/W4` and no sanitizers anywhere. Add per-target warnings (option-gated), fix fallout, add one Linux ASan+UBSan job running ctest.
*Accept:* warnings on in CI; sanitizer job green and required.

**10. Pin dependencies: vcpkg manifest + baseline; pin `imgui-platform-kit`**
Files: new `vcpkg.json`, `scripts/setup.*`, all workflows.
CI clones vcpkg HEAD; platform kit cloned at HEAD from a personal repo. Add manifest with `builtin-baseline`; pin the platform-kit commit; drop the never-used catch2 dependency while at it.
*Accept:* two CI runs a week apart resolve identical dependency versions; catch2 gone from setup and CI.

**11. Element constructor must throw on invalid size, not half-construct**
File: `src/elements/element.cpp:8-18`.
*Accept:* invalid `ElementDimensions` throws `Exception(ELEM_INVALID_SIZE)`; test added; no code path observes an `Element` without its components.

### Medium

**12. `NeuralFieldParameters`: value-equality and rule-of-five**
File: `include/elements/neural_field.h` (op== at :37; copy ops).
`operator==` compares `unique_ptr` addresses; copy-ctor and copy-assign disagree on null handling; no move ops. Compare activation functions by value; unify copy semantics; add moves.
*Accept:* two identically-parameterized fields compare equal; move doesn't degrade to copy; tests cover ==, copy, move.

**13. Unify sigmoid numerics (float vs double paths)**
File: `src/elements/activation_function.cpp:19-26` vs `tools/math.h::sigmoid`.
*Accept:* both paths compute in double; a test asserts they agree to 1e-12 on a sample field.

**14. Bounds-check heatmap manual dimensions and `obtainCircularVector` indices**
Files: `src/visualization/heatmap.cpp:~216-237`, `include/tools/math.h:133,254`.
*Accept:* `rows*cols > data.size()` clamps/errors instead of OOB read; circular-index path validates or documents+asserts its 1-based invariant; tests added.

**15. Replace `std::exit(0)` with a clean-shutdown signal**
File: `src/user_interface/main_menu_bar.cpp:58,338`.
*Accept:* Quit/Ctrl+Q close via the main loop; `Application::close()` and destructors run.

**16. Fix the logger: single instance, no per-call reassignment, no upward dependency**
Files: `include/tools/logger.h`, `src/tools/logger.cpp`.
Header-`static` instance duplicates per TU; `log()` reassigns the global each call; defaults contradict; `tools` includes `user_interface`/`application`. Make it a single instance behind a function, and give it a sink interface the UI registers (dependency inversion).
*Accept:* `tools/` has no include of `application/` or `user_interface/`; one logger instance; consistent default level.

**17. `Simulation::run` semantics; unify not-found contracts**
Files: `src/simulation/simulation.cpp:242-254,431,444`, `simulation.h:69`.
Fix the "milliseconds" doc (it's simulation time); stop unconditionally `close()`-wiping data after `run()` (or make it opt-in); make `getElement(string)` and `getElement(int)` share one error model.
*Accept:* docs match behavior; component data inspectable after `run()`; both overloads behave identically on miss; tests updated.

**18. Fix CI enforcement: clang-tidy gate + cache keys + coverage upload**
Files: `.clang-tidy`, `.github/workflows/static-analysis.yml:32`, `release.yml:47,156,254`, `ci.yml:127`.
Set `WarningsAsErrors` to a meaningful set (staged); correct `hashFiles` paths to `scripts/…`; reconsider `fail_ci_if_error: false`.
*Accept:* introducing a tidy violation fails the job; cache keys change when setup scripts change.

**19. Fix install-interface include mismatch; stop baking absolute paths into binaries**
File: `dynamic-neural-field-composer/CMakeLists.txt:43-48,242,288`.
*Accept:* a fresh external project consumes the installed package with documented includes; shipped binaries contain no configure-machine paths (rely on `getResourceRoot()`).

### Testing (High as a group)

**20. Add tests for `visualization/` data logic and headless-testable UI logic**
Zero tests today for 22 files across `visualization/` and `user_interface/`; plot bookkeeping and field-metrics math need no GL context. Start with `Visualization` add/remove/render bookkeeping (would have caught item 2) and `field_metrics_window` computations.
*Accept:* ≥1 real behavioral test file per testable class in `visualization/`; metrics math covered.

**21. Make kernel tests value-exact; test parameter structs**
File: `tests/elements/test_kernels.cpp` + new.
Assert exact expected kernel values (Gaussian/Mexican-hat/oscillatory, circular and non-circular, 1D and 2D) against analytic references, not "has positive and negative values."
*Accept:* each kernel type has at least one exact-values test; the six concrete parameter structs have construction/equality/toString tests.

**22. Smoke-run the examples in CI (headless)**
22 examples compile but never execute. Add a headless/short-run mode (N steps, no GUI or off-screen) and a CI step running each.
*Accept:* every example runs N steps and exits 0 in at least the Linux CI job.

### Low / Chore

**23. De-duplicate: kernel `step()` tail → shared `Kernel` method; single factory dispatch table; `runExample()` helper for all 22 examples; split `element_window.cpp` / `simulation_window.cpp` per-element panels; extract triplicated bump-velocity logic and fix the start/end off-by-one in `NeuralField::updateBumps` (name the `0.895` / `0.00001` / `2.0` constants).**
*Accept:* adding a new element type touches ≤2 places; no window file >800 lines; examples contain only DNF wiring.

**24. Modern-C++ sweep: `enum class` for `ElementLabel`/`ActivationFunctionType`; remove `reinterpret_cast<int*>` on scoped enum (plot_control_window.cpp:19); const-correct getters + `noexcept`; return `std::span`/const-ref instead of by-value copies (`getComponent`, `getPlots()` per frame); cache 2D-kernel component lookups like the 1D ones; atomic ID counter; `safe_localtime` everywhere; scratch buffers for noise elements; delete commented-out code; unify namespace style; `return found;` ×6.**
*Accept:* clang-tidy modernize-* checks pass on touched files; no per-frame map copies; no unscoped enums in `element/`.

**25. Docs housekeeping: fix CONTRIBUTING.md script paths (`scripts/…`) and the GCC 11-vs-13 contradiction; Doxygen the undocumented headers (all of `user_interface/`, most of `visualization/`, `tools/{logger,utils,profiling}.h`).**
*Accept:* CONTRIBUTING commands work copy-paste on a fresh clone; Doxygen `WARN_NO_PARAMDOC` count materially reduced.

---

*Both reviewers agree: items 1–4 first (memory safety and UB), then 9–10 (make the toolchain
catch the next regression before a human has to), then the testing group — because every bug
in this review that wasn't in the math layer was in code no test touches.*
