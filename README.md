# dynamic-neural-field-composer

[![CI](https://github.com/Jgocunha/dynamic-neural-field-composer/actions/workflows/ci.yml/badge.svg)](https://github.com/Jgocunha/dynamic-neural-field-composer/actions/workflows/ci.yml)
[![Release](https://github.com/Jgocunha/dynamic-neural-field-composer/actions/workflows/release.yml/badge.svg)](https://github.com/Jgocunha/dynamic-neural-field-composer/actions/workflows/release.yml)
[![Latest Release](https://img.shields.io/github/v/release/Jgocunha/dynamic-neural-field-composer)](https://github.com/Jgocunha/dynamic-neural-field-composer/releases/latest)
[![codecov](https://codecov.io/gh/Jgocunha/dynamic-neural-field-composer/graph/badge.svg)](https://codecov.io/gh/Jgocunha/dynamic-neural-field-composer)

<img src="./dynamic-neural-field-composer/resources/images/logo.png" alt="logo" >

A C++20 library and interactive application for building and simulating **Dynamic Neural Field (DNF)** architectures.

---

## About

Dynamic Neural Fields model how neuron populations represent and transform information over continuous dimensions (position, direction, color). They produce emergent behaviours — working memory, winner-take-all selection, sequence generation — from local excitation and surround inhibition.

This library lets you design, connect, simulate, and visualize DNF architectures at runtime, either programmatically or through a visual node-graph editor. Simulations can be saved and reloaded as JSON.

---

## Requirements

- CMake 3.20+
- C++20 compiler (MSVC, GCC 13+, or Clang)
- [vcpkg](https://github.com/microsoft/vcpkg) with `VCPKG_ROOT` set

Dependencies are installed automatically via vcpkg: `imgui`, `implot`, `imgui-node-editor`, `nlohmann-json`, `imgui-platform-kit`.

---

## Building

```bash
# Windows
build.bat

# Linux / macOS
./build.sh
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

Clone the repo, run the build script, then explore the `examples/` folder to see the library in action.

For detailed setup, usage guides, and API reference, see the **[Wiki](https://github.com/Jgocunha/dynamic-neural-field-composer/wiki)**.


---

## Contributing

Bug fixes, new features, and documentation improvements are all welcome. Feel free to open an issue or submit a pull request.
