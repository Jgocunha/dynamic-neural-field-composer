# The CI vcpkg cache can hide a missing dependency for weeks

Every vcpkg-using job clones vcpkg HEAD and restores only the vcpkg `installed/` tree from the
Actions cache. The file whose hash forms the key differs per workflow, and each has a
prefix-only `restore-keys` fallback:

| Workflow | Key hashes | Restore-key prefix |
|---|---|---|
| `ci.yml` (Linux, sanitizers, macOS) | `scripts/setup.sh` | `vcpkg-<os>-` (macOS: `vcpkg-<os>-<triplet>-`) |
| `ci.yml` (Windows) | `scripts/setup.bat` | `vcpkg-<os>-` |
| `release.yml` | `.github/workflows/release.yml` | `vcpkg-<os>-` (macOS: `vcpkg-<os>-arm64-osx-`) |
| `static-analysis.yml` | `.github/workflows/static-analysis.yml` | `vcpkg-clang-tidy-<os>-` |

`ci.yml` and `release.yml` share the `vcpkg-<os>-` prefix, so either can restore the other's
cache. Because of the prefix fallback, editing the keyed file does not give a clean install:
it only misses the exact key and restores the nearest older cache. A package that was once
installed stays in that cache after it is removed from the install list, so anything still
depending on it keeps working.

GitHub evicts caches unused for 7 days. The next run after a quiet week installs from scratch
against the current vcpkg HEAD, and every gap surfaces at once, in a commit that did not
cause it. This happened on 2026-09-24: a mode-only commit to `main` failed on every OS because

- `chore: drop unused catch2 dependency` (792ffcf, 2026-08-14) removed Catch2, but
  `imgui-platform-kit` configures its own tests by default and `find_package(Catch2 3 REQUIRED)`s
  it. Fix: configure it with `-DIMGUI_PLATFORM_KIT_BUILD_TESTS=OFF` everywhere it is built.
- vcpkg's `pthread-stubs` (GLFW -> libx11 -> libxcb) needs `autoconf-archive`, which the
  ubuntu runners do not ship.

When a dependency-step failure appears on an unrelated commit, check the "Cache vcpkg
packages" step for `Cache not found` before suspecting the commit. To reproduce locally,
hide a package from CMake with `-DCMAKE_DISABLE_FIND_PACKAGE_<Name>=ON`.
