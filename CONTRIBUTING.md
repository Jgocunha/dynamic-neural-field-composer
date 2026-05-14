# Contributing

Thank you for your interest in contributing to **dynamic-neural-field-composer**. This document covers how to report issues, set up a development environment, submit changes, and (for maintainers) cut a release.

---

## Reporting bugs and requesting features

Open an issue on GitHub using the appropriate template:

- [Bug report](https://github.com/Jgocunha/dynamic-neural-field-composer/issues/new?template=bug_report.md) — OS, compiler, steps to reproduce, expected vs. actual behaviour, log output
- [Feature request](https://github.com/Jgocunha/dynamic-neural-field-composer/issues/new?template=feature_request.md) — what you need, use case, why the existing API does not cover it

---

## Development environment

**Prerequisites**

| Requirement | Minimum |
|---|---|
| C++ compiler | C++20 (MSVC 2022, GCC 11+, Clang 13+, Apple Clang 13+) |
| CMake | 3.20 |
| vcpkg | Any recent — set `VCPKG_ROOT` |

**Build**

```bat
# Windows
build.bat
```

```bash
# Linux
./build.sh

# macOS
./build_macos.sh
```

See [Getting Started](dynamic-neural-field-composer/wiki/Getting-Started.md) for full instructions and manual CMake options.

---

## Making changes

1. Fork the repository and create a branch from `main`.
2. Use a descriptive branch name: `fix/element-factory-null-check`, `feat/new-kernel-type`.
3. Keep changes focused — one logical change per PR.

**Code style**

New code should follow the principles in *Clean Code* by Robert C. Martin: meaningful names, small focused functions, no redundant comments, and code that reads like prose. See [Jgocunha/command-line-parser](https://github.com/Jgocunha/command-line-parser). The short version:

- C++20, matching the style of the surrounding file.
- No raw owning pointers — use `std::shared_ptr` / `std::unique_ptr` as the rest of the codebase does.
- No comments that explain *what* the code does; only add one if the *why* is non-obvious.
- Prefer clarity over cleverness — if a reader needs to pause to understand a line, rewrite it.

The existing codebase does not perfectly meet these standards everywhere — test coverage has (a lot of) gaps, and some areas could be refactored. If you spot something that falls short of Clean Code principles, opening an issue is a welcome contribution in itself. What matters is that **new changes move things in the right direction**.

**Tests**

Every change to element behaviour, simulation logic, or utilities must be covered by a test in `tests/`. The test executable is `dnf_composer_tests` (Google Test).

```bash
ctest --build-config Release --output-on-failure
```

**Doxygen**

Public API additions or changes to existing parameters require updated Doxygen comment blocks in the corresponding header under `include/`.

**Wiki**

If you change element parameters, UI behaviour, or the build process in a user-visible way, update the relevant page in `dynamic-neural-field-composer/wiki/`.

---

## Submitting a pull request

1. Ensure all CI checks pass (Windows, Linux, macOS).
2. Fill in the PR description: what changed, why, and how to verify it.
3. Link any related issues.

A maintainer will review and merge once CI is green and the checklist above is met.

---

## Release process (maintainers)

1. **Decide the version bump** following [Semantic Versioning](https://semver.org):
   - `PATCH` — bug fix, no API change
   - `MINOR` — new feature or platform support, backwards-compatible
   - `MAJOR` — breaking API change

2. **Update the version** in `dynamic-neural-field-composer/CMakeLists.txt`:
   ```cmake
   set(DNF_COMPOSER_VERSION_MAJOR X ...)
   set(DNF_COMPOSER_VERSION_MINOR Y ...)
   set(DNF_COMPOSER_VERSION_PATCH Z ...)
   ```

3. **Add a changelog entry** at the top of `CHANGELOG.md`:
   ```markdown
   ## [X.Y.Z] - YYYY-MM-DD

   ### Added
   ### Fixed
   ### Build
   ### Documentation
   ```
   Only include sections that have content.

4. Commit with message `release: vX.Y.Z`, then tag and push:
   ```bash
   git tag vX.Y.Z
   git push origin vX.Y.Z
   ```
   The release CI jobs trigger on the tag and produce platform archives.
