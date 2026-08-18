# Profiling dnf-composer with Very Sleepy

How to get a **symbol-resolved CPU profile** that shows where time goes *inside* each element's
`step()` (e.g. `conv_valid_into`, `unordered_map` lookups, `SigmoidFunction::apply`,
`zigguratNormal`), so optimization targets are **measured, not guessed**.

This is the procedure used to produce the "INTRA-STEP SAMPLING BREAKDOWN" section in
`dynamic-neural-field-composer/tests/profiler/profile.md`.

---

## Why this setup

- **Sampling profiler, not instrumentation.** Very Sleepy samples the running process — no code
  changes to the library, and it measures the real optimized binary.
- **You must build with symbols.** The default `build/release` is `/O2` with **no PDB**, so a sampler
  can only show raw addresses. We build a separate `RelWithDebInfo` directory that keeps the **same
  `/O2` optimization** but also emits a PDB (`/Zi` + linker `/DEBUG`). Profiling a `Debug` build would
  be meaningless — different codegen.
- **The `.sleepy` capture file is a ZIP of plain-text tables**, so the results can be parsed/scripted
  without opening the GUI.

## Prerequisites

- **Very Sleepy** installed (codersnotes / GitHub `VerySleepy/verysleepy`). This guide assumes
  `C:\dev-files\Very Sleepy\sleepy.exe`.
- **Visual Studio 2022** (for MSVC + Ninja). Build env comes from `vcvars64.bat`:
  `C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat`
- **vcpkg** at `%VCPKG_ROOT%` (e.g. `C:\dev-files\vcpkg`).
- The in-repo profiler target `dnf_composer_profiler` (under `tests/profiler/`), which loops every
  element type's `step()` many times. It takes an iteration count as `argv[1]`.

---

## Step 1 — Build a symbol-bearing, still-optimized binary

From a **Developer / vcvars64** shell (or wrap the commands with `vcvars64.bat && …`), in the inner
`dynamic-neural-field-composer/` source dir:

```bat
cmake -S . -B build\relwithdebinfo -G Ninja ^
  -DCMAKE_BUILD_TYPE=RelWithDebInfo ^
  -DCMAKE_TOOLCHAIN_FILE=%VCPKG_ROOT%/scripts/buildsystems/vcpkg.cmake ^
  -DCMAKE_CXX_FLAGS_RELWITHDEBINFO="/O2 /Ob2 /DNDEBUG /Zi" ^
  -DCMAKE_EXE_LINKER_FLAGS="/DEBUG" ^
  -DCMAKE_SHARED_LINKER_FLAGS="/DEBUG"

cmake --build build\relwithdebinfo --target dnf_composer_profiler
```

Verify the PDB exists (this is what lets Very Sleepy resolve function names):

```
build\relwithdebinfo\tests\dnf_composer_profiler.exe
build\relwithdebinfo\tests\dnf_composer_profiler.pdb     <-- ~28 MB, contains library symbols
```

> The static-library symbols get linked into the exe's PDB, so a single PDB resolves both the
> profiler harness and the dnf-composer functions (`conv_valid_into`, `updateBumps`, etc.).

## Step 2 — Size the run

`/r:` (run-and-profile) **cannot pass arguments to the exe**, and the default 20 000 iterations
finishes in a few seconds — too short to sample well. So we launch the exe ourselves with a high
iteration count and **attach by PID**. First, time a high-count run to know how long the capture
window should be:

```powershell
$exe = "build\relwithdebinfo\tests\dnf_composer_profiler.exe"
Measure-Command { & $exe 200000 }   # ~176 s on the reference machine
```

Pick a `/t:` (capture seconds) slightly under that wall-clock time.

## Step 3 — Capture (headless, attach-by-PID)

`sleepy.exe` flags (from its `/h` usage dialog):

| flag | meaning |
|------|---------|
| `/r:<exe>` | run an executable and profile it (no args possible) |
| `/a:<pid>` | **attach to a running process by PID** (what we use) |
| `/o:<file>` | save the captured profile (`.sleepy`) |
| `/t:<num>` | stop capturing automatically after N seconds |
| `/q` | quiet (no error dialogs) |
| `/mbt` | profile only the most-busy thread (profiler is single-threaded) |

```powershell
$sleepy = "C:\dev-files\Very Sleepy\sleepy.exe"
$exe    = "build\relwithdebinfo\tests\dnf_composer_profiler.exe"
$out    = "$env:TEMP\dnfc_capture.sleepy"

# 1. launch the profiler with a high iteration count
$prof = Start-Process $exe -ArgumentList "200000" -PassThru -WindowStyle Minimized
Start-Sleep -Milliseconds 400          # let it get past startup into the element loop

# 2. attach Very Sleepy by PID, capture ~170 s, most-busy thread
Start-Process $sleepy -ArgumentList "/a:$($prof.Id)", "/o:$out", "/t:170", "/q", "/mbt"
```

> **Do not** use a `.cmd`/`.bat` launcher with `/r:` — Very Sleepy profiles the launcher process
> (which exits immediately after spawning the real exe), and you get an almost-empty capture with
> `Filename: ?`. Attach-by-PID profiles the actual exe (`Filename:` shows the real path).

Wait for `sleepy.exe` to exit (it self-stops at `/t:`). A good capture is tens of KB with tens of
thousands of samples.

## Step 4 — Read the results

### Option A — GUI
Double-click the `.sleepy` file (or `File → Open`). Use the function list (self/exclusive %) and the
caller/callee tree to split shared functions like `conv_valid_into` by calling element. `File →
Export` writes a CSV.

### Option B — Script it (no GUI)
The `.sleepy` file is a ZIP containing plain text:

| entry | contents |
|-------|----------|
| `Stats.txt` | filename, duration, date, total sample count |
| `Symbols.txt` | `0xADDR "module" "function" "sourcefile" line` — symbol table (with source line!) |
| `IPCounts.txt` | first line = total seconds; then `0xADDR <seconds>` self-time per instruction |
| `Callstacks.txt` | `<seconds> <leafAddr> <…> <rootAddr>` — full stacks (for inclusive time + caller attribution) |

To analyze:
1. Parse `Symbols.txt` into `address → function`.
2. **Exclusive (self) time:** sum each stack's weight onto its **leaf** function in `Callstacks.txt`.
3. **Inclusive time:** add each stack's weight to **every distinct** function in that stack.
4. **Caller attribution** (e.g. which element drives `conv_valid_into`): for every stack containing
   the target function, find the nearest enclosing `…::<Element>::step` frame and credit it.

A reference PowerShell parser/attributor was used for this repo; the logic above is enough to
reproduce it. Output is a table like:

```
60.5%  conv_valid_into        (MexicanHat2D 39%, Oscillatory2D 28%, Gauss2D 15%, …)
10.4%  std::_Hash (components[...] lookups)   (MemoryTrace2D 75%, CorrNoise2D 24%)
 2.5%  SigmoidFunction::apply
 0.23% NeuralField2D::updateBumps   (negligible)
```

## Step 5 — Record it

Append a dated section to `tests/profiler/profile.md` (top self-time functions + per-element
attribution) so the intra-step cost profile is tracked over time alongside the harness's
whole-`step()` aggregates.

---

## Notes & gotchas
- Percentages are of total sampled CPU across the **whole** profiler sweep (all element types run
  sequentially), so they reflect each function's share of the combined workload — not a single element.
  Use caller attribution to isolate one element.
- Keep `/O2`. Never profile a `Debug` build.
- If symbols show as bare addresses, the PDB wasn't found — confirm `dnf_composer_profiler.pdb` sits
  next to the exe, and that you attached to the right PID.
- This complements `dnf_composer_profiler`'s own timing (whole-`step()` µs per element) and
  `dnf_composer_benchmark` (steps/sec). Sampling tells you *where inside* a step the time goes.
