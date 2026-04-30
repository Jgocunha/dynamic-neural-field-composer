# Changelog

All notable changes to this project will be documented in this file.
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
- macOS release matrix extended to include `macos-13` (Intel x64), producing both
  `macos-arm64.tar.gz` and `macos-x64.tar.gz` release artifacts
- vcpkg cache key for macOS release jobs now includes the triplet to prevent
  cross-architecture cache collisions
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
