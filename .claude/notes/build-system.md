# Build system

## Presets are the path-free way to build

`dynamic-neural-field-composer/CMakePresets.json` defines `release` and `debug` - both
Ninja, both taking the toolchain from `$env{VCPKG_ROOT}`, writing to `build/release` and
`build/debug`.

```bash
cmake --preset release
cmake --build --preset release --target dnf_composer_tests --parallel 4
```

This is why `build/release` is the live build tree: it is the preset's `binaryDir`, not
something anyone configured by hand.

## Test presets

`CMakePresets.json` has `testPresets` (`release`/`debug`, mirroring the configure/build
presets), so `ctest --preset release` works the same way `cmake --build --preset release`
does - no need to invoke the binary directly, though that still works too:

```bash
ctest --preset release --output-on-failure
./build/release/tests/dnf_composer_tests            # .exe on Windows
```

## The scripts build somewhere else

`scripts/build.bat`, `scripts/build.sh` and `scripts/build_macos.sh` do **not** use the
presets. They configure their own trees:

| Script | Tree |
|---|---|
| `build.bat` | `build/x64-release`, `build/x64-debug` |
| `build.sh` | `build/linux-release` |
| `build_macos.sh` | `build/macos-release` |

So a repo can hold several build trees configured by different tools with different
generators. When working in an existing tree rather than a preset one, read its
`CMakeCache.txt` for the generator and compiler instead of assuming.

## `scripts/build.bat` pins the VS install per build tree

It used to pick the compiler with `vswhere -latest` on every run. On a machine with more
than one Visual Studio installed, that could select a different VS than an existing build
tree was configured with, and the mismatched STL headers would fail every translation unit
with `STL1001: Unexpected compiler version`. It survived only because it always configured
from scratch in practice, so CMake and vcvars happened to agree.

Fixed: the script now records the VS install path it used in `build\.vsinstall` after a
successful `vswhere` lookup, and reuses that recorded path (skipping `vswhere`) as long as
it still has a `vcvars64.bat`. A fresh tree (no marker yet) behaves exactly as before. Only
deleting `build\.vsinstall` (or the whole `build\` tree) lets a newer "latest" VS take over.

A tree that already existed **before** this pinning landed has no marker either, so it looks
"fresh" to the check above. To not silently reintroduce the same bug for that one case, the
script also checks: no marker, but `build\x64-release\CMakeCache.txt` already exists → compare
its cached `CMAKE_CXX_COMPILER` against the freshly-detected VS (normalizing `\` to `/` first,
since CMake's cache uses forward slashes and `vswhere` doesn't) and refuse to proceed on a
mismatch, rather than guess. `scripts/README.md` documents the one-time manual fix (delete the
tree, or hand-write `build\.vsinstall`).

The marker write uses delayed expansion (`!VSINSTALL!`, script-wide via
`setlocal EnableDelayedExpansion`) rather than `%VSINSTALL%`, so a value containing `&`/`|`/`<`/
`>` can't be reinterpreted as command syntax when the line is parsed (delayed expansion
substitutes after parsing, not before) — flagged by CodeRabbit on PR #180 even though `VSINSTALL`
only ever comes from `vswhere` or this script's own marker file, neither realistically
attacker-controlled.

## Restoring a file by copy can leave Ninja with stale objects

Ninja decides what to rebuild from mtime. Restoring a source file with `Copy-Item` (or any
copy that preserves the original timestamp) can put back an *older* mtime than the object
compiled from the modified version - so Ninja considers the object up to date and skips
recompiling. The next test run then executes the **previous** code.

This bit during issue #128: after temporarily perturbing test expectations to prove the new
tests could fail, restoring the originals by copy left the perturbed objects in place, and
the suite reported 12 failures that no longer existed in any source file.

The tell is a failure you cannot find in the source. Confirm by grepping the sources for
whatever you changed; if they are clean, the binary is stale. Force the rebuild by touching
the files (updating mtime to now) rather than trusting the copy:

```bash
find <paths> -name '*.cpp' -exec touch {} +
```

Editing files in place (rather than restoring from a copy) does not have this problem, since
the edit sets a current mtime.

## Dependencies

`imgui-platform-kit` is consumed as an installed package found via `CMAKE_PREFIX_PATH`, not
built in-tree. A checkout with no local `deps/` still configures, provided the package is
installed somewhere CMake looks. Everything else comes from vcpkg via `VCPKG_ROOT`.

## Verified baseline

As of 2026-08-21, `dnf_composer_tests` builds clean and runs **1596 tests across 349 test
suites, all passing**. Use that as the reference when judging whether a failure is
pre-existing.

## Windows: `vcvars64.bat` clobbers `VCPKG_ROOT`

Loading the MSVC environment overwrites `VCPKG_ROOT` with Visual Studio's own bundled
vcpkg, which does not have this project's packages. Because `CMakePresets.json` expands
`$env{VCPKG_ROOT}` when the preset runs - i.e. after vcvars - `cmake --preset release`
picks up the wrong toolchain file.

`scripts/build.bat` guards against this explicitly, snapshotting the value into
`PROJECT_VCPKG_ROOT` before calling vcvars. Anyone invoking the presets by hand has to do
the same: capture `VCPKG_ROOT` first, then set it back after loading MSVC.

An already-configured tree hides the problem, since the cached toolchain wins and the
freshly-passed one is reported as unused. It bites on a fresh configure.
