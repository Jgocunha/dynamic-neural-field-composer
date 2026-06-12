# Scripts

These scripts automate the three phases of working with the project: setting up dependencies, building, and installing. All scripts are designed to be run from the **project root** (one level above this folder), and all paths are derived from the script's own location so they work regardless of your current directory.

---

## Setup (run once on a fresh machine)

These scripts install all automated dependencies. See [Getting Started](../wiki/Getting-Started.md) for what you need to install manually before running these.

### `setup.sh` — Linux and macOS

```bash
chmod +x scripts/setup.sh
./scripts/setup.sh
```

What it does:
1. If `VCPKG_ROOT` is not set, clones vcpkg to `$HOME/vcpkg`, bootstraps it, and prints the line to add to your shell profile to persist the variable
2. Auto-detects your OS and CPU architecture to select the correct vcpkg triplet (`x64-linux`, `x64-osx`, or `arm64-osx`)
3. Installs all required vcpkg packages: `imgui`, `implot`, `imgui-node-editor`, `nlohmann-json`, `gtest`, `catch2`
4. Clones `imgui-platform-kit` into `deps/imgui-platform-kit/` (skipped if already present)
5. Builds and installs `imgui-platform-kit` into `deps/ipk-install/` (skipped if already present)

Idempotent — safe to re-run; already-completed steps are skipped.

### `setup.bat` — Windows

```bat
scripts\setup.bat
```

What it does:
1. If `VCPKG_ROOT` is not set, clones vcpkg to `C:\tools\vcpkg`, bootstraps it, and persists `VCPKG_ROOT` permanently via `setx`
2. Installs all required vcpkg packages for `x64-windows`: `imgui`, `implot`, `imgui-node-editor`, `nlohmann-json`, `gtest`, `catch2`
3. Clones `imgui-platform-kit` into `deps\imgui-platform-kit\` (skipped if already present)
4. Builds and installs `imgui-platform-kit` into `deps\ipk-install\` (skipped if already present)

Idempotent — safe to re-run; already-completed steps are skipped.

---

## Build (run after setup)

These scripts configure and build the project with CMake. They require setup to have been run first (i.e., `VCPKG_ROOT` must be set and `deps/ipk-install/` must exist).

### `build.sh` — Linux

```bash
chmod +x scripts/build.sh
./scripts/build.sh
```

Configures and builds in Release mode. Output lands in `build/linux-release/`.

### `build_macos.sh` — macOS

```bash
chmod +x scripts/build_macos.sh
./scripts/build_macos.sh
```

Auto-detects CPU architecture (`arm64` or `x64`). Configures and builds in Release mode. Output lands in `build/macos-release/`.

### `build.bat` — Windows

```bat
scripts\build.bat
```

Configures and builds both Release and Debug configurations with Ninja and the MSVC toolchain. Output lands in `build\x64-release\` and `build\x64-debug\`.

---

## Install (run after build)

These scripts install the compiled library, headers, and CMake config files to a local system prefix so downstream CMake projects can find the library via `find_package`.

### `install.sh` — Linux and macOS

```bash
./scripts/install.sh
```

Detects the OS and runs `cmake --install` on `build/linux-release/` (Linux) or `build/macos-release/` (macOS). Will prompt for sudo if not already running as root.

### `install.bat` — Windows

```bat
scripts\install.bat
```

Installs both Release and Debug configurations from `build\x64-release\` and `build\x64-debug\`. Auto-elevates via UAC if not running as administrator.
