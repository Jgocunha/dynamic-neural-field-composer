# dynamic-neural-field-composer

A C++20 library and interactive application for building and simulating **Dynamic Neural Field (DNF)** architectures.

Dynamic Neural Fields are a mathematical framework from computational neuroscience that describes how populations of neurons represent, sustain, and transform information distributed over continuous feature dimensions — such as spatial position, direction, or color. A DNF is governed by a differential equation that produces rich emergent behaviors including self-sustained activation bumps (working memory), winner-take-all selection, and sequence generation, all arising from the interplay of local excitation and surround inhibition.

This library provides a complete toolkit for designing, connecting, simulating, and visualizing DNF architectures at runtime. Simulations are defined programmatically or via a visual node-graph editor, and can be saved and reloaded as JSON. Researchers use it to prototype neural field models; engineers use it to drive cognitive architectures for robotic tasks.

## Table of Contents

- [Overview](#overview)
- [Requirements](#requirements)
- [Building](#building)
- [Running](#running)
- [Testing](#testing)
- [Quick Start](#quick-start)
- [Core Concepts](#core-concepts)
  - [Simulation](#simulation)
  - [Elements](#elements)
  - [Interactions](#interactions)
  - [Visualization](#visualization)
  - [Application & UI](#application--ui)
- [Element Reference](#element-reference)
- [Examples](#examples)

---

## Overview

The library provides:

- A **Simulation** engine that manages elements and steps them forward in time
- A set of **Elements** (neural fields, kernels, stimuli, couplings, noise) that can be wired together
- A **Visualization** layer for real-time line plots and heatmaps
- An **ImGui-based GUI** with a node-graph editor, element inspector, field metrics panel, and plot controls
- JSON-based **save/load** for simulation configurations
- Learnable **field couplings** with Hebbian, Oja, and Delta rules

---

## Requirements

- **CMake** 3.20 or later
- **C++20** compiler (MSVC, GCC, or Clang)
- **vcpkg** (set `VCPKG_ROOT` environment variable)

vcpkg dependencies (installed automatically):

| Package | Purpose |
|---|---|
| `imgui` | Immediate-mode GUI |
| `implot` | Real-time plotting |
| `unofficial-imgui-node-editor` | Visual node graph |
| `nlohmann-json` | Simulation serialization |
| `imgui-platform-kit` | Platform/window abstraction |

---

## Building

```bash
# Windows
build.bat

# Linux / macOS
./build.sh
```

Or manually with CMake:

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

To install the library for use in another project:

```bash
# Windows
install.bat

# Linux / macOS
./install.sh
```

---

## Running

After building, the `launcher` executable opens the full interactive GUI. On Windows the binary is placed under `build/x64-release/Release/` or `build/x64-debug/Debug/`.

The GUI includes:
- **Node-graph editor** — visually wire elements and inspect the simulation topology
- **Element inspector** — edit parameters of any element while the simulation runs
- **Field metrics panel** — live bump detection with centroid, amplitude, width, and stability readout
- **Plots window** — real-time line plots and heatmaps of any element component
- **Simulation controls** — start, pause, single-step, reset, save, and load

Alternatively, load one of the pre-built simulation configurations from `data/simulations/` via `File → Open`.

---

## Testing

The test suite uses **Google Test** (installed automatically via vcpkg). Tests are built into the `dnf_composer_tests` executable when the `DNF_COMPOSER_BUILD_TESTS` CMake option is `ON` (the default).

### Build with tests

Tests are included in the default build. To explicitly enable or disable them:

```bash
# Enable (default)
cmake -B build -DDNF_COMPOSER_BUILD_TESTS=ON

# Disable
cmake -B build -DDNF_COMPOSER_BUILD_TESTS=OFF
```

### Run all tests via CTest

```bash
cd build/x64-release      # or your build directory
ctest --build-config Release --output-on-failure
```

### Run the test executable directly

```bash
# Windows
build\x64-release\Release\dnf_composer_tests.exe

# Linux
./build/dnf_composer_tests
```

### Run a specific test or suite

```bash
# All tests in a fixture group
dnf_composer_tests.exe --gtest_filter="NeuralFieldConstruction.*"

# A single test
dnf_composer_tests.exe --gtest_filter="SimulationLifecycle.StepAdvancesTime"

# List all available tests
dnf_composer_tests.exe --gtest_list_tests
```

## Quick Start

The minimal setup creates a `Simulation`, adds elements, wires them together, and runs the application loop:

```cpp
#include "dynamic-neural-field-composer.h"

using namespace dnf_composer;
using namespace dnf_composer::element;

int main()
{
    // 1. Create the simulation
    auto simulation = std::make_shared<Simulation>("my sim", /*deltaT=*/1.0);
    auto visualization = std::make_shared<Visualization>(simulation);
    Application app{ simulation, visualization };

    // 2. Add UI windows
    app.addWindow<user_interface::MainWindow>();
    app.addWindow<user_interface::SimulationWindow>();
    app.addWindow<user_interface::PlotsWindow>();
    app.addWindow<user_interface::NodeGraphWindow>();

    // 3. Create elements
    ElementFactory factory;
    const ElementDimensions dims{ 100, 1.0 };

    auto field  = factory.createElement(NEURAL_FIELD,
        ElementCommonParameters{ ElementIdentifiers{"my field"}, dims },
        NeuralFieldParameters{ /*tau=*/25.0, /*restingLevel=*/-5.0,
                               SigmoidFunction(0.0, 10.0) });

    auto kernel = factory.createElement(GAUSS_KERNEL,
        ElementCommonParameters{ ElementIdentifiers{"self-excitation"}, dims },
        GaussKernelParameters{ /*width=*/3.0, /*amplitude=*/3.0 });

    auto stimulus = factory.createElement(GAUSS_STIMULUS,
        ElementCommonParameters{ ElementIdentifiers{"input"}, dims },
        GaussStimulusParameters{ /*width=*/5.0, /*amplitude=*/15.0, /*position=*/50.0 });

    // 4. Register elements
    simulation->addElement(field);
    simulation->addElement(kernel);
    simulation->addElement(stimulus);

    // 5. Wire interactions
    field->addInput(kernel);    // kernel feeds into field
    kernel->addInput(field);    // field feeds back into kernel (recurrent)
    field->addInput(stimulus);  // external stimulus drives field

    // 6. Set up a plot
    visualization->plot(
        PlotCommonParameters{
            PlotType::LINE_PLOT,
            PlotDimensions{ 0.0, 100, -20.0, 20.0, 1.0, 1.0 },
            PlotAnnotations{ "Field activation", "Position", "Amplitude" }
        },
        LinePlotParameters{},
        { { field->getUniqueName(), "activation" },
          { field->getUniqueName(), "output" } });

    // 7. Run the application loop
    app.init();
    while (!app.hasGUIBeenClosed())
        app.step();
    app.close();
}
```

---

## Core Concepts

### Simulation

`Simulation` is the central object. It owns all elements and drives the time loop.

```cpp
// Create
auto sim = std::make_shared<Simulation>(
    "identifier",   // name
    1.0,            // deltaT (time step)
    0.0,            // tZero (start time)
    0.0             // t (current time)
);

// Manage elements
sim->addElement(element);
sim->removeElement("element name");
sim->createInteraction("source", "output", "target");

// Control
sim->init();
sim->step();          // advance one time step
sim->run(1000.0);     // run for N steps (headless)
sim->pause();
sim->resume();
sim->close();

// Persistence
sim->save("path/to/file.json");
sim->read("path/to/file.json");

// Inspect
auto el = sim->getElement("name");
auto data = sim->getComponent("name", "activation");
```

### Elements

All elements derive from the abstract base class `Element`. Each element has:

- **Common parameters** (`ElementCommonParameters`): a unique name, label, and spatial dimensions (`x_max`, `d_x`)
- **Specific parameters**: type-specific struct (e.g., `NeuralFieldParameters`)
- **Components**: named `std::vector<double>` buffers (e.g., `"activation"`, `"output"`, `"input"`)
- **Inputs / Outputs**: directed connections to other elements

```cpp
// Wire elements
elementA->addInput(elementB);                   // elementB's "output" -> elementA
elementA->addInput(elementB, "activation");     // use a specific component

// Inspect
auto data = element->getComponent("activation");
auto name = element->getUniqueName();
auto dims = element->getElementCommonParameters().dimensionParameters;
```

### Interactions

`createInteraction` is the `Simulation`-level shorthand for wiring elements:

```cpp
sim->createInteraction("stimulus name", "output", "field name");
```

Equivalent to calling `field->addInput(stimulus)`.

### Visualization

`Visualization` manages plots that render element components in real time.

```cpp
auto vis = std::make_shared<Visualization>(simulation);

// Line plot
vis->plot(
    PlotCommonParameters{
        PlotType::LINE_PLOT,
        PlotDimensions{ xMin, xMax, yMin, yMax, xStep, yStep },
        PlotAnnotations{ "title", "x label", "y label" }
    },
    LinePlotParameters{},
    { { "element name", "component" }, ... }
);

// Heatmap (e.g. for field coupling weights)
vis->plot(
    PlotCommonParameters{
        PlotType::HEATMAP,
        PlotDimensions{ 0.0, 280, 0.0, 280, 1.0, 1.0 },
        PlotAnnotations{ "coupling weights", "x", "y" }
    },
    HeatmapParameters{},
    { { "coupling name", "weights" } }
);
```

### Application & UI

`Application` wraps the simulation, visualization, and GUI. Windows are registered with `addWindow<T>()`. The constructor detects automatically whether a window needs a `Simulation` or `Visualization` handle.

Available UI windows:

| Window | Description |
|---|---|
| `user_interface::MainWindow` | Menu bar and layout control |
| `user_interface::SimulationWindow` | Start/stop/step/reset controls |
| `user_interface::ElementWindow` | Inspect and edit element parameters |
| `user_interface::FieldMetricsWindow` | Live bump detection, stability, min/max |
| `user_interface::PlotControlWindow` | Add/remove plot data series |
| `user_interface::PlotsWindow` | Rendered implot plots |
| `user_interface::NodeGraphWindow` | Visual node-graph editor |
| `imgui_kit::LogWindow` | Log output |

```cpp
app.init();
while (!app.hasGUIBeenClosed())
    app.step();
app.close();
```

---

## Element Reference

All elements are created through `ElementFactory::createElement(label, commonParams, specificParams)`.

### NeuralField

The core DNF element. Integrates inputs over time using a sigmoid activation function.

```cpp
NeuralFieldParameters params{
    /*tau=*/25.0,                      // time constant (ms)
    /*startingRestingLevel=*/-5.0,     // resting activation level
    SigmoidFunction(/*threshold=*/0.0, /*steepness=*/10.0)
};
```

**Components:** `activation`, `output`, `input`

**State:** tracks bumps (`NeuralFieldBump`) with centroid, amplitude, width, velocity, and acceleration; reports `isStable()`.

### GaussKernel

Lateral interaction kernel — typically used for local excitation (positive amplitude) and global inhibition (negative `amplitudeGlobal`).

```cpp
GaussKernelParameters params{
    /*width=*/3.0,
    /*amplitude=*/3.0,
    /*amplitudeGlobal=*/-0.01,
    /*circular=*/true,
    /*normalized=*/true
};
```

### MexicanHatKernel

Short-range excitation / long-range inhibition kernel. Common for producing stable localized bumps.

### OscillatoryKernel

Oscillatory lateral interaction pattern.

### AsymmetricGaussKernel

Asymmetric lateral interactions, useful for modeling directional or travelling-wave dynamics.

### GaussStimulus

A stationary Gaussian input to a field.

```cpp
GaussStimulusParameters params{
    /*width=*/5.0,
    /*amplitude=*/15.0,
    /*position=*/50.0,
    /*circular=*/true,
    /*normalized=*/false
};
```

### NormalNoise

Adds Gaussian white noise to a field.

```cpp
NormalNoiseParameters params{ /*amplitude=*/0.05 };
```

### FieldCoupling

A learned weight matrix connecting two fields of potentially different sizes. Supports three learning rules:

| Rule | Description |
|---|---|
| `LearningRule::HEBB` | Classic Hebbian learning |
| `LearningRule::OJA` | Oja's rule (normalized Hebbian) |
| `LearningRule::DELTA` | Delta / error-correcting rule |

```cpp
FieldCouplingParameters params{
    inputFieldDimensions,           // ElementDimensions of the source field
    LearningRule::HEBB,
    /*scalar=*/1.0,
    /*learningRate=*/0.01
};

// Toggle learning at runtime
coupling->setLearning(true);

// Persist weights
coupling->writeWeights();
coupling->readWeights();
coupling->clearWeights();
```

**Components:** `weights`, `output`

### GaussFieldCoupling

A field coupling with a fixed Gaussian coupling profile (no learning).

---

## Examples

Nine ready-to-run examples are included in [examples/](examples/):

| Executable | Source | Description |
|---|---|---|
| `ex_comp_act_selection` | [ex_complementary_action_selection.cpp](examples/ex_complementary_action_selection.cpp) | Complementary action selection via competing fields |
| `ex_asymmetric_gauss_kernel` | [ex_asymmetric_gauss_kernel.cpp](examples/ex_asymmetric_gauss_kernel.cpp) | Asymmetric kernel dynamics |
| `ex_two_robot_team` | [ex_two_robot_team.cpp](examples/ex_two_robot_team.cpp) | Two-robot team coordination |
| `ex_field_couplings` | [ex_field_couplings.cpp](examples/ex_field_couplings.cpp) | Three coupled fields (past / present / next) |
| `ex_gauss_and_field_couplings` | [ex_gauss_and_field_couplings.cpp](examples/ex_gauss_and_field_couplings.cpp) | Mixed Gaussian and learnable couplings |
| `ex_field_coupling_learning` | [ex_field_coupling_learning.cpp](examples/ex_field_coupling_learning.cpp) | Online Hebbian learning between fields |
| `ex_grand_architecture` | [ex_grand_architecture.cpp](examples/ex_grand_architecture.cpp) | Large multi-field architecture |
| `ex_packaging_task` | [ex_packaging_task.cpp](examples/ex_packaging_task.cpp) | DNF-based packaging/manipulation task |
| `ex_simulation_file_manager` | [ex_simulation_file_manager.cpp](examples/ex_simulation_file_manager.cpp) | Save and load simulations — explicit `SimulationFileManager` API and `sim->save` / `sim->read` convenience methods |

All examples follow the same pattern: create a `Simulation`, build elements with `ElementFactory`, wire them, set up plots, and run the application loop.