# dynamic-neural-field-composer

[![CI](https://github.com/Jgocunha/dynamic-neural-field-composer/actions/workflows/ci.yml/badge.svg)](https://github.com/Jgocunha/dynamic-neural-field-composer/actions/workflows/ci.yml)
[![Release](https://github.com/Jgocunha/dynamic-neural-field-composer/actions/workflows/release.yml/badge.svg)](https://github.com/Jgocunha/dynamic-neural-field-composer/actions/workflows/release.yml)
[![Latest Release](https://img.shields.io/github/v/release/Jgocunha/dynamic-neural-field-composer)](https://github.com/Jgocunha/dynamic-neural-field-composer/releases/latest)
[![codecov](https://codecov.io/gh/Jgocunha/dynamic-neural-field-composer/graph/badge.svg)](https://codecov.io/gh/Jgocunha/dynamic-neural-field-composer)
[![Docs](https://img.shields.io/badge/docs-doxygen-blue)](https://jgocunha.github.io/dynamic-neural-field-composer/)
[![Wiki](https://img.shields.io/badge/wiki-github-blue)](https://github.com/Jgocunha/dynamic-neural-field-composer/wiki)

![logo](./dynamic-neural-field-composer/resources/images/logo.png)

A C++20 library and interactive application for building and simulating **Dynamic Neural Field (DNF)** architectures.

---

## About

Dynamic Neural Fields model how neuron populations represent and transform information over continuous dimensions (position, direction, color). They produce emergent behaviours — working memory, winner-take-all selection, sequence generation — from local excitation and surround inhibition.

This library lets you design, connect, simulate, and visualize DNF architectures at runtime, either programmatically or through a visual node-graph editor. Simulations can be saved and reloaded as JSON.

It is built with real-time performance in mind, and straightforward to embed in any C++ application. The GUI is powered by [Dear ImGui](https://github.com/ocornut/imgui), keeping it fast and lightweight.

---

## Requirements

- CMake 3.20+
- C++20 compiler (MSVC, GCC 11+, Clang 13+, or Apple Clang 13+)
- [vcpkg](https://github.com/microsoft/vcpkg) with `VCPKG_ROOT` set

Dependencies are installed automatically via vcpkg: `imgui`, `implot`, `imgui-node-editor`, `nlohmann-json`, `imgui-platform-kit`.

---

## Building

```bash
# Windows
build.bat

# Linux
./build.sh

# macOS (auto-detects Apple Silicon vs Intel)
./build_macos.sh
```

To install the library for use in another project:

```bash
# Windows
install.bat

# Linux / macOS
./install.sh
```

---

## Getting Started

Clone the repo and run the build script. Two pre-built executables are produced:

- **`dnf-composer-static`** — a single self-contained window with all panels docked in a fixed layout. Best for quickly building and running a simulation without any configuration.
- **`dnf-composer-dynamic`** — a fully dockable ImGui application. Windows can be rearranged, detached, and dragged to secondary monitors, giving you complete control over the layout at runtime.

You can also write your own executable, link against the library, and choose exactly which windows to render and which architecture to load or define in code. Creating a custom launcher is straightforward — see the **[Wiki](https://github.com/Jgocunha/dynamic-neural-field-composer/wiki/How-to-Create-and-Run-Your-Own-Example-Executable)** for a step-by-step guide.

Explore the `examples/` folder for ready-to-run architectures.

---

## Contributing

Bug fixes, new features, and documentation improvements are all welcome. Feel free to open an issue or submit a pull request.
