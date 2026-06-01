# Getting Started

Building from source was a deliberate design goal of this project. Academic codebases are notorious for being painful to compile, so dnf-composer keeps its external dependencies to a minimum and resolves them automatically and independently through [vcpkg](https://vcpkg.io/) — there are no libraries to hunt down and install by hand. In practice, building reduces to running a single setup script once, then a build script.

## Prerequisites

### What you must install manually

The setup scripts handle most dependencies automatically, but the following must be present on your machine before running them.

#### Windows

| Requirement | Notes | How to get it |
|---|---|---|
| Visual Studio 2022 | Enable the **"Desktop development with C++"** workload. This provides MSVC, CMake, and MSBuild. | [visualstudio.microsoft.com](https://visualstudio.microsoft.com/) |
| Git | Can be installed via the VS installer (optional components) or standalone. | [git-scm.com](https://git-scm.com/) |

#### Linux

| Requirement | Notes | How to get it |
|---|---|---|
| GCC 13+ | GCC 13 or later is required. | `sudo apt-get install gcc-13 g++-13` |
| CMake 3.20+ | | `sudo apt-get install cmake` |
| Git | | `sudo apt-get install git` |
| OpenGL + X11 dev libraries | Required by the GUI. | `sudo apt-get install libgl1-mesa-dev libglu1-mesa-dev libglfw3-dev libxrandr-dev libxinerama-dev libxcursor-dev libxi-dev libxext-dev` |

If GCC 13 is not yet your system default, set it:

```bash
sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-13 100
sudo update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-13 100
```

#### macOS

| Requirement | Notes | How to get it |
|---|---|---|
| Xcode Command Line Tools | Provides the Clang compiler and Git. | `xcode-select --install` |
| CMake 3.20+ | Not included with Xcode CLT — must be installed separately. | `brew install cmake` or [cmake.org](https://cmake.org/download/) |

---

### What the setup scripts install automatically

You do **not** need to install any of the following — the setup scripts handle them:

| Package | Source | Purpose |
|---|---|---|
| `vcpkg` | Cloned from GitHub | C++ package manager |
| `imgui` | vcpkg | Immediate-mode GUI |
| `implot` | vcpkg | Real-time plotting |
| `unofficial-imgui-node-editor` | vcpkg | Visual node-graph editor |
| `nlohmann-json` | vcpkg | Simulation serialization |
| `gtest` | vcpkg | Unit testing framework |
| `catch2` | vcpkg | Unit testing framework |
| `imgui-platform-kit` | Built from source (cloned to `deps/`) | Platform/window abstraction |

---

## Quick setup (recommended)

All scripts live in the [`scripts/`](scripts/) folder. See [`scripts/README.md`](scripts/README.md) for a full description of each script.

Run the setup script once on a fresh machine, then use the build script whenever you want to compile.

### Windows

```bat
scripts\setup.bat
scripts\build.bat
```

`setup.bat` installs vcpkg to `C:\tools\vcpkg` if `VCPKG_ROOT` is not already set and persists it via `setx`, installs all vcpkg packages, and builds `imgui-platform-kit` into `deps\ipk-install\`.

`build.bat` configures and builds both Release and Debug. Binaries land in `build\x64-release\Release\` and `build\x64-debug\Debug\`.

### Linux

```bash
chmod +x scripts/setup.sh scripts/build.sh
./scripts/setup.sh
./scripts/build.sh
```

`setup.sh` installs vcpkg to `$HOME/vcpkg` if not already present, installs all vcpkg packages for `x64-linux`, and builds `imgui-platform-kit`. After setup, `build.sh` configures and builds into `build/linux-release/`.

> **Note:** `setup.sh` exports `VCPKG_ROOT` for the current session only. It prints a one-liner to add to your `~/.bashrc` or `~/.zshrc` so that subsequent terminals pick it up automatically.

### macOS

```bash
chmod +x scripts/setup.sh scripts/build_macos.sh
./scripts/setup.sh
./scripts/build_macos.sh
```

`setup.sh` auto-detects your architecture (`arm64-osx` or `x64-osx`) and handles everything. `build_macos.sh` builds into `build/macos-release/`.

---

## Manual CMake build

If you already have all dependencies installed and want to drive CMake directly, pass `VCPKG_ROOT` as the toolchain and point `CMAKE_PREFIX_PATH` at your `imgui-platform-kit` install:

```bash
# Configure (Release)
cmake -B build/release \
      -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_TOOLCHAIN_FILE="$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake" \
      -DCMAKE_PREFIX_PATH="deps/ipk-install"

# Build
cmake --build build/release --config Release

# Debug
cmake -B build/debug \
      -DCMAKE_BUILD_TYPE=Debug \
      -DCMAKE_TOOLCHAIN_FILE="$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake" \
      -DCMAKE_PREFIX_PATH="deps/ipk-install"
cmake --build build/debug --config Debug
```

### CMake build options

| Option | Default | Description |
|---|---|---|
| `DNF_COMPOSER_BUILD_TESTS` | `ON` | Build the Google Test suite |
| `CMAKE_BUILD_TYPE` | — | `Release` or `Debug` |

---

## Running the application

Two pre-built executables are produced by the build:

### Static layout (`dnf-composer-static`)

A single self-contained window with all panels docked in a fixed layout. Best for quickly building and running a simulation without any configuration.

```bash
# Windows (Release)
build\x64-release\Release\dnf-composer-static.exe

# Linux
./build/dnf-composer-static

# macOS
./build/macos-release/dnf-composer-static
```

### Dynamic layout (`dnf-composer-dynamic`)

A fully dockable ImGui application. Windows can be rearranged, detached, and dragged to secondary monitors, giving you complete control over the layout at runtime.

```bash
# Windows (Release)
build\x64-release\Release\dnf-composer-dynamic.exe

# Linux
./build/dnf-composer-dynamic

# macOS
./build/macos-release/dnf-composer-dynamic
```

### First steps in the GUI

1. Open a pre-built simulation: **File → Open** and navigate to `data/` (simulation files are organized as `data/<sim_name>/<sim_name>.dnf`)
2. Use the **Node Graph** window to inspect the element topology
3. Use **Simulation Controls** to start, pause, single-step, or reset
4. Live plots appear in the **Plots** window
5. Select any element in the **Element Inspector** to modify parameters while running

---

## Running the examples

Seventeen example executables are built alongside the application:

```bash
# Windows
build\x64-release\Release\example_detection_instability.exe
build\x64-release\Release\example_memory_instability.exe
build\x64-release\Release\example_selection_instability.exe
build\x64-release\Release\example_multi_peak.exe
# ... etc.

# Linux
./build/example_detection_instability
./build/example_memory_instability
./build/example_selection_instability
./build/example_multi_peak

# macOS
./build/macos-release/example_detection_instability
./build/macos-release/example_memory_instability
./build/macos-release/example_selection_instability
./build/macos-release/example_multi_peak
```

See [Examples](Examples) for a description of each.

---

## Installing the library

To install headers, the compiled library, and CMake config files into a local prefix:

```bash
# Windows
scripts\install.bat

# Linux / macOS
./scripts/install.sh
```

This installs:
- Headers to `<prefix>/include/dnf_composer/`
- Library binary to `<prefix>/lib/`
- CMake package config to `<prefix>/share/dynamic-neural-field-composer/`

Downstream projects can then find and link the library with:

```cmake
find_package(dynamic-neural-field-composer REQUIRED)
target_link_libraries(your_target PRIVATE dynamic-neural-field-composer)
```

---

## Running the tests

See the dedicated [Testing](Testing) page for full details. The short version:

```bash
# CTest (from your build directory)
ctest --build-config Release --output-on-failure

# Or run the test executable directly
build\x64-release\Release\dnf_composer_tests.exe   # Windows
./build/dnf_composer_tests                          # Linux
./build/macos-release/dnf_composer_tests            # macOS
```

---

## Quick Start

The minimal code to create and run a simulation programmatically:

```cpp
#include "dynamic-neural-field-composer.h"

using namespace dnf_composer;
using namespace dnf_composer::element;

int main()
{
    // 1. Create simulation and visualization
    auto simulation  = std::make_shared<Simulation>("my sim", /*deltaT=*/1.0);
    auto visualization = std::make_shared<Visualization>(simulation);
    Application app{ simulation, visualization };

    // 2. Register UI windows
    app.addWindow<user_interface::MainMenuBar>();
    app.addWindow<user_interface::StaticLayoutWindow>(simulation, visualization);

    // 3. Create elements
    ElementFactory factory;
    const ElementDimensions dims{ 100, 1.0 };

    auto field = factory.createElement(NEURAL_FIELD,
        ElementCommonParameters{ ElementIdentifiers{"my field"}, dims },
        NeuralFieldParameters{ 25.0, -5.0, SigmoidFunction(0.0, 10.0) });

    auto kernel = factory.createElement(GAUSS_KERNEL,
        ElementCommonParameters{ ElementIdentifiers{"lateral interactions"}, dims },
        GaussKernelParameters{ 3.0, 3.0, -0.01 });

    auto stimulus = factory.createElement(GAUSS_STIMULUS,
        ElementCommonParameters{ ElementIdentifiers{"input stimulus"}, dims },
        GaussStimulusParameters{ 5.0, 15.0, 50.0 });

    // 4. Add elements to the simulation
    simulation->addElement(field);
    simulation->addElement(kernel);
    simulation->addElement(stimulus);

    // 5. Wire interactions
    field->addInput(kernel);    // recurrent lateral interactions
    kernel->addInput(field);
    field->addInput(stimulus);  // external drive

    // 6. Set up a plot
    visualization->plot(
        PlotCommonParameters{
            PlotType::LINE_PLOT,
            PlotDimensions{ 0.0, 100, -20.0, 20.0, 1.0, 1.0 },
            PlotAnnotations{ "Field dynamics", "Position", "Activation" }
        },
        LinePlotParameters{},
        { { field->getUniqueName(), "activation" },
          { field->getUniqueName(), "output" } });

    // 7. Application loop
    app.init();
    while (!app.hasGUIBeenClosed())
        app.step();
    app.close();
}
```

See the [Element Reference](Element-Reference) for all element types and their parameters.
