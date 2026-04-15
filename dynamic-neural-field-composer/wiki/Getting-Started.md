# Getting Started

## Prerequisites

| Requirement | Minimum version | Notes |
|---|---|---|
| C++ compiler | C++20 | MSVC 2022, GCC 11+, Clang 13+ |
| CMake | 3.20 | |
| vcpkg | Any recent | Set `VCPKG_ROOT` environment variable |

The `VCPKG_ROOT` environment variable must point to your vcpkg installation. The build script will fail with an explicit error message if it is not set.

**Linux only:** GCC 13 or later is recommended. If your system default is older:

```bash
sudo add-apt-repository ppa:ubuntu-toolchain-r/test
sudo apt update && sudo apt install gcc-13 g++-13
# Optionally set as default:
sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-13 100
sudo update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-13 100
```

### vcpkg dependencies

These are installed automatically by the build scripts:

| Package | Purpose |
|---|---|
| `gtest` | Unit testing framework |
| `imgui` | Immediate-mode GUI |
| `implot` | Real-time plotting |
| `unofficial-imgui-node-editor` | Visual node-graph editor |
| `nlohmann-json` | Simulation serialization |
| `imgui-platform-kit` | Platform/window abstraction |

---

## Building

### Windows — using the provided script

```bat
build.bat
```

This script:
1. Installs all vcpkg dependencies for `x64-windows`
2. Creates `build/x64-release/` and `build/x64-debug/`
3. Configures both with CMake (Visual Studio 17 2022 generator)
4. Builds both Release and Debug configurations

Binaries land in:
- `build/x64-release/Release/`
- `build/x64-debug/Debug/`

### Linux / macOS — using the provided script

```bash
chmod +x build.sh
./build.sh
```

The script installs vcpkg packages for Linux (using OpenGL + GLFW bindings instead of DirectX), then configures and builds with `make`.

### Manual CMake build

If you prefer to drive CMake directly:

```bash
# Configure (Release)
cmake -B build/release \
      -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_TOOLCHAIN_FILE="$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake"

# Build
cmake --build build/release --config Release

# Debug
cmake -B build/debug \
      -DCMAKE_BUILD_TYPE=Debug \
      -DCMAKE_TOOLCHAIN_FILE="$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake"
cmake --build build/debug --config Debug
```

### CMake build options

| Option | Default | Description |
|---|---|---|
| `DNF_COMPOSER_BUILD_TESTS` | `ON` | Build the Google Test suite |
| `CMAKE_BUILD_TYPE` | — | `Release` or `Debug` |

---

## Running the launcher

The `launcher` executable opens the full interactive GUI application.

```bash
# Windows (Release)
build\x64-release\Release\launcher.exe

# Linux
./build/launcher
```

### First steps in the GUI

1. Open a pre-built simulation: **File → Open** and navigate to `data/simulations/`
2. Use the **Node Graph** window to inspect the element topology
3. Use **Simulation Controls** to start, pause, single-step, or reset
4. Live plots appear in the **Plots** window
5. Select any element in the **Element Inspector** to modify parameters while running

---

## Running the examples

Eight example executables are built alongside the launcher:

```bash
# Windows
build\x64-release\Release\ex_field_couplings.exe
build\x64-release\Release\ex_complementary_action_selection.exe
# ... etc.

# Linux
./build/ex_field_couplings
./build/ex_complementary_action_selection
```

See [Examples](Examples) for a description of each.

---

## Installing the library

To install headers, the compiled library, and CMake config files into a local prefix:

```bash
# Windows
install.bat

# Linux / macOS
./install.sh
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
    app.addWindow<user_interface::MainWindow>();
    app.addWindow<user_interface::SimulationWindow>();
    app.addWindow<user_interface::PlotsWindow>();
    app.addWindow<user_interface::NodeGraphWindow>();

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
