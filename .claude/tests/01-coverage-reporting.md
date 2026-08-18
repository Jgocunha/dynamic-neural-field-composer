# 01 — Honest coverage reporting (codecov.yml + gcovr excludes)

Goal: the headline number should measure the **testable core library**, not
interactive demos and GUI glue. This is pure configuration — no tests here.

## Root cause

- `.github/workflows/ci.yml` builds *everything* (library, 21 example exes,
  launcher apps, tests) with global `--coverage` flags (lines ~85–86), and the
  gcovr step (lines ~105–118) excludes only `vcpkg`, `tests/`,
  `imgui-platform-kit`, and `CMakeFiles` — **not `examples/`**. Result: ~1047
  lines of demo `main()`s at 0% sit in the denominator.
- There is **no `codecov.yml`** anywhere in the repo, so Codecov runs with all
  defaults: no ignores, no components, no sensible status thresholds.

## Checklist

- [x] Add `<repo-root>/codecov.yml` with:
  - `ignore:` for `dynamic-neural-field-composer/examples/**` and defensively
    `**/examples/**` (gcovr emits paths relative to `--root`, i.e. the nested
    project dir — `examples/foo.cpp` — but Codecov path-fixing may re-prefix
    them; covering both forms is harmless).
  - `component_management:` with two components so core vs GUI are visible
    separately on the dashboard:
    - `core`: `**/src/elements/**`, `**/src/simulation/**`, `**/src/tools/**`,
      `**/src/exceptions/**`, `**/src/element_parameters/**` (+ matching `include/`)
    - `gui`: `**/src/user_interface/**`, `**/src/application/**`, `**/src/visualization/**`
  - `coverage.status.project`: `target: auto, threshold: 1%` — informational,
    not punitive; tighten once the number stabilises.
  - `comment:` layout with `components` so PR comments show the split.
- [x] Mirror the exclusion in the gcovr invocation in `ci.yml`:
  `--exclude '.*/examples/.*'` next to the existing `--exclude '.*/tests/.*'`.
- [ ] (Optional, later) Gate examples behind a CMake option
  (`DNF_COMPOSER_BUILD_EXAMPLES`, default ON) and pass `OFF` in the coverage
  configure step — removes them from the build entirely, speeding up the
  coverage job. Root `CMakeLists.txt` adds `examples/` unconditionally (~line 355).
- [ ] (Optional, later) Split Codecov *flags* per layer by running gcovr twice
  (core-only and gui-only filters) and uploading two reports with
  `flags: core` / `flags: gui`. Components (above) already give the dashboard
  split, so only do this if per-flag status checks are wanted.

## Verification

- `curl --data-binary @codecov.yml https://codecov.io/validate` → must answer
  "Valid!".
- After merge to `main`, check the Codecov dashboard: `examples/` gone from the
  file tree, headline figure jumps (~+7% mechanically, more with the new tests),
  and the `core` / `gui` components report separately.
- The gcovr regex can be sanity-checked locally without running coverage:
  `echo 'dynamic-neural-field-composer/examples/multi_peak.cpp' | grep -E '.*/examples/.*'`.
