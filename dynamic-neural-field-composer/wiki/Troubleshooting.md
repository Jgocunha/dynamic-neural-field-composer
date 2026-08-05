# Troubleshooting

Common problems building and running dnf-composer, and how to fix them. Entries are grouped by phase (build, startup, simulation runtime, `.dnf` loading) and, where possible, quote the actual error text or log message produced by the code so you can match what you're seeing.

---

## Build issues

### `ERROR: This project requires VCPKG.`

This is a `FATAL_ERROR` raised directly in the top-level `CMakeLists.txt` when the `VCPKG_ROOT` environment variable is not set. The project's CMake configuration unconditionally includes `${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake` as its toolchain file, so `VCPKG_ROOT` must exist before you invoke CMake at all.

**Fix:** run the setup script for your platform first — it clones and bootstraps vcpkg and prints the `VCPKG_ROOT` export needed by your shell (see [Getting Started → Quick setup](Getting-Started#quick-setup-recommended)):

```bash
# Windows
scripts\setup.bat

# Linux / macOS
./scripts/setup.sh
```

If you set `VCPKG_ROOT` manually instead, make sure it's exported/persisted in the **same shell** (or system environment on Windows) that invokes CMake — `setup.sh` only exports it for the current session and prints the line to add to `~/.bashrc`/`~/.zshrc`.

### Build fails after pulling new changes / after switching branches

`deps/ipk-install/` (the built `imgui-platform-kit`) and the vcpkg package cache can go stale relative to a newer `vcpkg.json` or a newer `imgui-platform-kit` revision. Re-running the setup script is idempotent — already-completed steps are skipped, so it's safe to re-run whenever a build starts failing for no obvious reason:

```bash
scripts\setup.bat   # or scripts/setup.sh
scripts\build.bat   # or scripts/build.sh / build_macos.sh
```

### GCC version errors on Linux (concepts / `<format>` / C++20 features)

The project requires **GCC 13 or later** (see [Getting Started → Prerequisites](Getting-Started#linux)). If GCC 13 isn't your system default, older GCC versions will fail on C++20 features used throughout the codebase (e.g. `std::format`, ranges). Set it explicitly:

```bash
sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-13 100
sudo update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-13 100
```

### Missing OpenGL/X11 development headers on Linux (GUI build fails)

The GUI links against OpenGL/GLFW via `imgui-platform-kit`. If configure or link fails with missing `GL/gl.h`, `X11/Xlib.h`, or similar, install the dev packages:

```bash
sudo apt-get install libgl1-mesa-dev libglu1-mesa-dev libglfw3-dev libxrandr-dev libxinerama-dev libxcursor-dev libxi-dev libxext-dev
```

### macOS: `cmake` not found even with Xcode Command Line Tools installed

Xcode CLT provides Clang and Git but **not** CMake. Install it separately: `brew install cmake` (or download from cmake.org). CMake 3.20+ is required either way.

---

## Simulation / element runtime errors

These are the actual exceptions and log messages the library produces for common misconfigurations, from `include/exceptions/exception.h` / `src/exceptions/exception.cpp` and the element `.cpp` files under `src/elements/`.

### `Exception`: "Gaussian stimulus (...) position is out of range, must be between 0 and size."

Thrown from the `GaussStimulus`/`GaussStimulus2D`/`TimedGaussStimulus`/`TimedGaussStimulus2D` constructors (`ErrorCode::GAUSS_STIMULUS_POSITION_OUT_OF_RANGE`) when `position` doesn't satisfy `0 <= position < x_max` (or the equivalent for `position_x`/`position_y` against the field's own dimensions). This is thrown immediately at construction time, so it will surface as an uncaught `dnf_composer::Exception` unless your `main()` wraps element creation in a `try`/`catch` — see the `catch (const dnf_composer::Exception& ex)` block in any `examples/*.cpp` file for the pattern to copy.

**Fix:** make sure `position` is within `[0, x_max)` for the field it will ultimately be connected to — note the position is checked against the **stimulus's own** `x_max` at construction, so if the stimulus and the field it feeds have different sizes, set the stimulus's dimensions to match the field's, or bridge them with a `Resize` element (see [Element Reference → Bridging fields of different sizes](Element-Reference#bridging-fields-of-different-sizes)).

### `Exception`: "Invalid size for element (...)."

`ErrorCode::ELEM_INVALID_SIZE`, thrown e.g. from `Collapse`/`Expand` when the configured dimensions are inconsistent with the axis being kept/broadcast. Double-check that a `Collapse`'s own (1D) output size matches the kept axis of its `inputDimensions`, and that an `Expand`'s profile-axis size matches its 1D input's size — see [Element Reference → Collapse](Element-Reference#collapse) and [→ Expand](Element-Reference#expand).

It is also thrown directly from the base `Element` constructor (so from **any** element type, not just `Collapse`/`Expand`) if `ElementCommonParameters.dimensionParameters.size` is not positive. In practice this can only happen if an `ElementDimensions` that was already validated at construction is mutated afterward (its fields are public) into an invalid state before being handed to an element's constructor — `ElementDimensions`'s own constructors already reject a non-positive extent/step/sample-count, so a size `<= 0` can no longer be produced through them. Previously this path logged an `ERROR` and silently produced a broken element (empty `components`, so `getComponentPtr("output")` failed with a confusing `ELEM_COMP_NOT_FOUND` later, or `getSize()` returned `0` and downstream loops silently no-op); the constructor now fails loudly at the point of construction instead.

### `ERROR`: "Input '...' has a different size than '...'."

Logged (not thrown) from `Element::addInput()` at `ERROR` level when you connect two elements whose component sizes don't match — the connection is silently rejected (`addInput` returns without throwing, so check your console/log output if a wire-up seems to have no effect).

**Fix:** either resize one side to match, or insert a `Resize`/`Resize2D`/`Collapse`/`Expand` element between them (see [Element Reference](Element-Reference) for each).

### Log error: "Input is null." / "Input '...' already exists."

Both logged from `Element::addInput()` (and the overridden `Resize`/`Collapse`/`Expand` versions) — passing a null/expired `shared_ptr`, or calling `addInput()` twice with the same source element, is a no-op with a logged error rather than a crash. `Resize`/`Resize2D`/`Collapse` additionally reject a **second** input entirely ("`Resize '...' already has an input; only one input is allowed.`") since they're single-input elements by design.

### `Exception`: "Element with id ... not found." / "... already exists."

`ErrorCode::SIM_ELEM_NOT_FOUND` / `SIM_ELEM_ALREADY_EXISTS`, thrown by `Simulation` when you call `getElement("name")`/`removeElement("name")` on a name that isn't registered, or `addElement(...)` with a `uniqueName` that's already in use. Element names must be unique within a `Simulation` — this mirrors the duplicate-name guard applied when loading a `.dnf` file (see below).

---

## Loading a `.dnf` file

`SimulationFileManager::loadElementsFromJson()` (see [.dnf File Schema](DNF-File-Schema)) is defensive: most problems are **logged as errors and skipped** rather than thrown, so a malformed file can partially load. Check your log output for these exact messages:

| Log message | Cause | What happens |
|---|---|---|
| `Unable to open file to load simulation: <path>.` | The `.dnf` path doesn't exist or isn't readable. | Load aborts immediately, simulation unchanged. |
| `Error reading JSON file: <exception text>` | The file isn't valid JSON (syntax error from a hand edit, truncated write, etc.). | Load aborts immediately. |
| `Invalid simulation file: "elements" is not an array: <path>` | Root is an object but its `"elements"` key isn't a JSON array. | Load aborts. |
| `Invalid simulation file: "identifier" is not a string: <path>` | `"identifier"` present but not a string. | That field is skipped; element loading still proceeds. |
| `Invalid simulation file: "deltaT" is not a valid positive number: <path>` | `"deltaT"` is present but ≤ 0, non-finite (NaN/Inf), or not a number. | `deltaT` is left at whatever the `Simulation` already had; element loading still proceeds. |
| `Invalid simulation file: unexpected JSON root type: <path>` | Root is neither an object nor a bare array (e.g. a JSON string or number). | Load aborts. |
| `Duplicate element name '<name>' in file - skipping this element.` | Two elements share the same `uniqueName`. | Only the first occurrence (and its interactions) is loaded; later duplicates and their `inputs` entries are skipped. |
| `Element label not recognized.` | An element's `"label"` string doesn't match any known `ElementLabel` (e.g. a file written by a newer version of the library with a new element type). | That element is skipped entirely — including on the interaction-wiring pass, so anything connected to it is dropped too. |
| `Skipping interaction '<src>' -> '<dst>': one or both elements were not loaded.` | An `inputs` entry references a `uniqueName` that was never created (dropped by one of the cases above, or simply misspelled). | That single interaction is skipped; the rest of the file continues loading. |

**General fix:** if a `.dnf` file loads "wrong" (missing elements or connections, unexpected `deltaT`), check the console/log output first — the loader almost never throws, so problems show up as log lines, not exceptions. Validate the file's JSON syntax (e.g. `python -m json.tool your-file.dnf` or any JSON linter) before assuming a code bug.

### `FieldCoupling` weights didn't load

`tryReadWeights()` logs `"No weights file found for '<name>' at: <path>. Starting with zero weights."` at **INFO** level (not an error) when `<coupling_name>_weights.txt` doesn't exist next to the `.dnf` file — this is expected when loading a `.dnf` whose coupling weights have not previously been saved or whose sidecar file is missing, and not itself a bug. If you expected existing weights to load and see this message instead, check that the `_weights.txt` file is in the **same directory as the `.dnf` file** — weight paths are resolved relative to the `.dnf` file's parent directory, not the working directory the executable was launched from.

If the file exists but its size doesn't match the coupling's current input/output dimensions, `readWeights()` logs `"Weight matrix read from file has a different size than expected! Expected: <N>, Got: <M>"` and leaves the weight matrix untouched (zeros) rather than partially loading it — this usually means the field sizes changed since the weights were saved.
